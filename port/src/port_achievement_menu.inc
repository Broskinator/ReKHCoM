static int sAchievementGroup, sAchievementChoice, sAchievementScroll, sAchievementView;
/* Reuse cartridge pixel art and the launcher's offline public PNG cache.
 * Keep just the selected icon: no network, texture atlas or background worker. */
#include <stdio.h>
#include <stdlib.h>
#include "../decompui/third_party/stb_image.h"
#include "port_achievement_art.inc"
static unsigned char sAchievementBadgePixels[64 * 64 * 4];
static char sAchievementBadgeName[8];
static int sAchievementBadgeWidth, sAchievementBadgeHeight;
static void AchievementMenuBadge(const PortAchievementInfo* info, int index, int x, int y) {
    int px, py;
    if (!sAchievementGroup) {
#if defined(VERSION_US)
        const int region = 1;
#elif defined(VERSION_JP)
        const int region = 2;
#else
        const int region = 0;
#endif
        const unsigned char* rom = (const unsigned char*)(uintptr_t)GBA_ROM;
        unsigned tiles = sAchievementCardArt[index][region][0], pal = sAchievementCardArt[index][region][1];
        for (py = 0; py < 32; ++py) for (px = 0; px < 16; ++px) {
            int t = (py / 8) * 2 + px / 8;
            unsigned b = rom[tiles + t * 32 + (py % 8) * 4 + (px % 8) / 2];
            unsigned c = px & 1 ? b >> 4 : b & 15, rgb, r, g, blue;
            if (!c) continue;
            rgb = rom[pal + c * 2] | rom[pal + c * 2 + 1] << 8;
            r = (rgb & 31) * 255 / 31; g = ((rgb >> 5) & 31) * 255 / 31; blue = ((rgb >> 10) & 31) * 255 / 31;
            if (info->state != 1) r = g = blue = (r * 77 + g * 150 + blue * 29) >> 8;
            Fill(x + 16 + px * 2, y + py * 2, 2, 2, 0xFF000000u | r << 16 | g << 8 | blue);
        }
        return;
    }
    if (strcmp(sAchievementBadgeName, info->badge)) {
        FILE* file;
        char path[96];
        SDL_strlcpy(sAchievementBadgeName, info->badge, sizeof(sAchievementBadgeName));
        sAchievementBadgeWidth = sAchievementBadgeHeight = 0;
        SDL_snprintf(path, sizeof(path), "cache/achievement-badges/%s.png", info->badge);
        file = info->badge[0] ? fopen(PortConfigPath(path), "rb") : NULL;
        if (file) {
            long size;
            fseek(file, 0, SEEK_END); size = ftell(file); rewind(file);
            if (size > 0 && size <= (1 << 20)) {
                unsigned char* encoded = (unsigned char*)malloc((size_t)size);
                if (encoded && fread(encoded, 1, (size_t)size, file) == (size_t)size) {
                    int w, h, channels;
                    if (stbi_info_from_memory(encoded, (int)size, &w, &h, &channels) && w > 0 && h > 0 && w <= 256 && h <= 256) {
                        unsigned char* rgba = stbi_load_from_memory(encoded, (int)size, &w, &h, &channels, 4);
                        if (rgba) {
                            int step = (SDL_max(w, h) + 63) / 64;
                            sAchievementBadgeWidth = (w + step - 1) / step;
                            sAchievementBadgeHeight = (h + step - 1) / step;
                            for (py = 0; py < sAchievementBadgeHeight; ++py) for (px = 0; px < sAchievementBadgeWidth; ++px)
                                memcpy(sAchievementBadgePixels + (py * 64 + px) * 4, rgba + (py * step * w + px * step) * 4, 4);
                            stbi_image_free(rgba);
                        }
                    }
                }
                free(encoded);
            }
            fclose(file);
        }
    }
    if (!sAchievementBadgeWidth) {
        DrawText("?", x + 28, y + 24, MENU_DIM, 0xFFFFFFFFu);
        return;
    }
    for (py = 0; py < sAchievementBadgeHeight; ++py) for (px = 0; px < sAchievementBadgeWidth; ++px) {
        unsigned char* p = sAchievementBadgePixels + (py * 64 + px) * 4;
        unsigned r = p[0], g = p[1], b = p[2], a = p[3];
        if (info->state != 1) r = g = b = (r * 77 + g * 150 + b * 29) >> 8;
        r = (r * a + 255 * (255 - a)) / 255;
        g = (g * a + 255 * (255 - a)) / 255;
        b = (b * a + 255 * (255 - a)) / 255;
        Pixel(x + (64 - sAchievementBadgeWidth) / 2 + px, y + (64 - sAchievementBadgeHeight) / 2 + py,
            0xFF000000u | r << 16 | g << 8 | b);
    }
}
/* Read-only, portable list inside Options. Account controls stay in launcher. */
static int AchievementMenuRows(int* indices, int* earned) {
    PortAchievementInfo info;
    int i, count = 0;
    *earned = 0;
    for (i = 0; i < PortAchievementsListCount(sAchievementGroup); ++i) {
        if (!PortAchievementsListGet(sAchievementGroup, i, 0, &info)) continue;
        *earned += info.state == 1;
        if ((sAchievementView == 1 && info.state != 0) || (sAchievementView == 2 && info.state != 1)) continue;
        indices[count++] = i;
    }
    return count;
}
static void AchievementMenuOpen(void) {
    PortAchievementsListRefresh();
    sPage = PAGE_ACHIEVEMENTS;
    sAchievementGroup = sAchievementChoice = sAchievementScroll = sAchievementView = 0;
    sMenuLightStart = MenuLightNow();
    sAchievementBadgeName[0] = 0; sAchievementBadgeWidth = sAchievementBadgeHeight = 0;
}
static int AchievementMenuInput(const SDL_Event* ev) {
    int move = 0, filter = 0, back = 0, group = 0, indices[256], earned, count;
    if (ev->type == SDL_EVENT_KEY_DOWN) {
        switch (ev->key.key) {
        case SDLK_UP: case SDLK_W: move = -1; break;
        case SDLK_DOWN: case SDLK_S: move = 1; break;
        case SDLK_LEFT: case SDLK_A: filter = -1; break;
        case SDLK_RIGHT: case SDLK_D: filter = 1; break;
        case '\t': group = !ev->key.repeat; break;
        case SDLK_ESCAPE: case SDLK_BACKSPACE: case SDLK_F1: case SDLK_Z: case SDLK_J: back = !ev->key.repeat; break;
        }
    } else if (ev->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        switch (ev->gbutton.button) {
        case SDL_GAMEPAD_BUTTON_DPAD_UP: move = -1; break;
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN: move = 1; break;
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT: filter = -1; break;
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: filter = 1; break;
        case SDL_GAMEPAD_BUTTON_NORTH: group = 1; break;
        case SDL_GAMEPAD_BUTTON_EAST: back = 1; break;
        case SDL_GAMEPAD_BUTTON_START: case SDL_GAMEPAD_BUTTON_BACK: case SDL_GAMEPAD_BUTTON_GUIDE: Close(); return 1;
        }
    }
    if (back) { sPage = PAGE_MAIN; sCursor = ITEM_ACHIEVEMENTS_MENU; sScroll = 0; return 1; }
    if (group && PortAchievementsListCount(1)) { sAchievementGroup = !sAchievementGroup; sAchievementChoice = sAchievementScroll = 0; }
    if (filter) { sAchievementView = Wrap(sAchievementView + filter, 3); sAchievementChoice = sAchievementScroll = 0; }
    count = AchievementMenuRows(indices, &earned);
    if (move && count) sAchievementChoice = Wrap(sAchievementChoice + move, count);
    if (move || filter || group) sMenuLightStart = MenuLightNow();
    return 1;
}
static void AchievementMenuText(const char* text, int x, int y, int right, uint32_t colour) {
    while (*text) {
        int code = NextChar(&text), advance = GlyphWidth(code);
        if (x + advance > right - 10 && *text) { DrawText("...", x, y, colour, 0xFFD8D8D8u); return; }
        DrawGlyph(code, x, y, colour, 0xFFD8D8D8u); x += advance;
    }
}
static void DrawAchievementMenu(int l, int width, int height) {
    static const char* const filters[2][3] = {{"All", "Locked", "Unlocked"}, {"Todos", "Pendientes", "Conseguidos"}};
    int indices[256], earned, count = AchievementMenuRows(indices, &earned), es = l == LANG_ES;
    int x = 10, top = MENU_TOP + 23, rows = SDL_max(1, (height - top - 94) / 16), i, y;
    int right = width - 10;
    PortAchievementInfo info;
    char text[160];
    Fill(0, 0, width, height, 0xFFFFFFFFu);
    DrawText(es ? "Logros" : "Achievements", x, MENU_TITLE_Y, MENU_INK, 0xFFD8D8D8u);
    SDL_snprintf(text, sizeof(text), "%s  %d/%d    < %s >", sAchievementGroup ? "RetroAchievements EU" : es ? "Locales" : "Local",
        earned, PortAchievementsListCount(sAchievementGroup), filters[es][sAchievementView]);
    AchievementMenuText(text, x, MENU_TOP + 2, right, MENU_RED_DARK);
    if (sAchievementChoice >= count) sAchievementChoice = SDL_max(0, count - 1);
    if (sAchievementChoice < sAchievementScroll) sAchievementScroll = sAchievementChoice;
    if (sAchievementChoice >= sAchievementScroll + rows) sAchievementScroll = sAchievementChoice - rows + 1;
    for (i = sAchievementScroll, y = top; i < count && i < sAchievementScroll + rows; ++i, y += 16) {
        int selected = i == sAchievementChoice;
        PortAchievementsListGet(sAchievementGroup, indices[i], es, &info);
        MenuBar(x, y, right - x, selected);
        /* A filled red mark denotes earned; grey remains locked/unknown. */
        Fill(x + 2, y + 1, 7, 7, info.state == 1 ? MENU_RED : MENU_GREY);
        SDL_snprintf(text, sizeof(text), "%s", info.state < 0 ? (es ? "Sin consultar" : "Unknown") :
            info.state ? (es ? "Conseguido" : "Unlocked") : (es ? "Pendiente" : "Locked"));
        DrawText(text, right - TextWidth(text) - 4, y - 2, info.state == 1 ? MENU_RED_DARK : MENU_DIM, 0xFFD8D8D8u);
        AchievementMenuText(info.title, x + 16, y - 2, right - TextWidth(text) - 12, info.state == 1 ? MENU_INK : MENU_DIM);
    }
    if (!count) DrawText(es ? "Sin resultados" : "No matches", x + 16, top, MENU_DIM, 0xFFFFFFFFu);
    if (count) {
        const char* p;
        int dx = x, dy = height - 88;
        int descriptionRight = right - 76;
        PortAchievementsListGet(sAchievementGroup, indices[sAchievementChoice], es, &info);
        AchievementMenuBadge(&info, indices[sAchievementChoice], right - 64, height - 88);
        p = info.description;
        while (*p && dy < height - 25) {
            int code = NextChar(&p), advance = GlyphWidth(code);
            if (dx + advance > descriptionRight) { dx = x; dy += 11; }
            if (dy >= height - 25) break;
            DrawGlyph(code, dx, dy, MENU_INK, 0xFFD8D8D8u); dx += advance;
        }
    }
    AchievementMenuText(sAchievementGroup ? (es ? "Estado de la última consulta del launcher" : "Status from last launcher account query") :
        (es ? "Retos locales de esta región" : "Local challenges for this region"), x, height - 22, right, MENU_DIM);
    AchievementMenuText(PortAchievementsListCount(1) ? (es ? "Arriba/abajo: lista  Izq/der: filtro  Tab/Y: grupo  B: volver" : "Up/down: list  Left/right: filter  Tab/Y: group  B: back") :
        (es ? "Arriba/abajo: lista  Izq/der: filtro  B: volver" : "Up/down: list  Left/right: filter  B: back"), x, height - 11, right, MENU_DIM);
}
