/* Experimental image-based HUD pilot. Decode once, draw cached GPU texture.
 * Enemy HP values and interpolation remain owned by the original game task.
 */
#include <SDL3/SDL.h>
#include <stdio.h>
#include "port/port.h"
#include "port/port_ui.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#include "../decompui/third_party/stb_image.h"

static SDL_Texture* sFrame;
static SDL_Texture* sWhiteFrame;
static SDL_Texture* sLabel;
static SDL_Texture* sWhiteLabel;
static SDL_Texture *sPlayerFrame, *sWhitePlayerFrame;
static SDL_FRect sPlayerSource;
int PortHdHudPlayerFrameReady(void) {
    /* Fitting probe only: keep native frame until interior-row alignment and
       full-health end caps are verified against the original. */
    return SDL_getenv("KHCOM_HDHUD_FRAME_PROBE") && gPortConfig.hdHud &&
           sPlayerFrame && sWhitePlayerFrame && gPortHudScale && !PortUiActive();
}
static void LoadPlayerFrame(SDL_Renderer* renderer) {
    const char* path = PortConfigPath("assets/hd-hud/pilot/player-double-frame-v1.png");
    int w, h, channels, x, y, l, t, r = -1, b = -1;
    unsigned char* pixels;
    SDL_Surface *src, *dst;
    SDL_ScaleMode sampling;
    if (!stbi_info(path, &w, &h, &channels) || channels != 4 || w < 1 || h < 1 ||
        w > 4096 || h > 4096 || (size_t)w * h > 4194304) return;
    pixels = stbi_load(path, &w, &h, &channels, 4);
    if (!pixels) return;
    l = w; t = h;
    for (y = 0; y < h; y++) for (x = 0; x < w; x++) {
        unsigned char* p = pixels + ((size_t)y * w + x) * 4;
        if (p[3] && (!x || !y || x == w - 1 || y == h - 1)) { stbi_image_free(pixels); return; }
        if (p[3] >= 32) { l = SDL_min(l, x); r = SDL_max(r, x); t = SDL_min(t, y); b = SDL_max(b, y); }
        if (!p[3]) p[0] = p[1] = p[2] = 0;
    }
    if (r < l) { stbi_image_free(pixels); return; }
    src = SDL_CreateSurfaceFrom(r - l + 1, b - t + 1, SDL_PIXELFORMAT_RGBA32,
                                pixels + ((size_t)t * w + l) * 4, w * 4);
    if (src) {
        int cw = SDL_min(src->w, 512), ch = SDL_max(1, src->h * cw / src->w);
        dst = SDL_CreateSurface(cw, ch, SDL_PIXELFORMAT_RGBA32);
        SDL_SetSurfaceBlendMode(src, SDL_BLENDMODE_NONE);
        if (dst && SDL_BlitSurfaceScaled(src, NULL, dst, NULL, SDL_SCALEMODE_LINEAR)) {
            sPlayerFrame = SDL_CreateTextureFromSurface(renderer, dst);
            for (y = 0; y < ch; y++) for (x = 0; x < cw; x++) {
                unsigned char* p = (unsigned char*)dst->pixels + y * dst->pitch + x * 4;
                p[0] = p[1] = p[2] = 255;
            }
            sWhitePlayerFrame = SDL_CreateTextureFromSurface(renderer, dst);
            sPlayerSource = (SDL_FRect){0, 0, cw, ch};
        }
        if (dst) SDL_DestroySurface(dst);
        SDL_DestroySurface(src);
    }
    stbi_image_free(pixels);
    sampling = SDL_strcmp(SDL_GetRendererName(renderer), "software") == 0 ? SDL_SCALEMODE_NEAREST : SDL_SCALEMODE_LINEAR;
    if (sPlayerFrame) { SDL_SetTextureBlendMode(sPlayerFrame, SDL_BLENDMODE_BLEND); SDL_SetTextureScaleMode(sPlayerFrame, sampling); }
    if (sWhitePlayerFrame) { SDL_SetTextureBlendMode(sWhitePlayerFrame, SDL_BLENDMODE_BLEND); SDL_SetTextureScaleMode(sWhitePlayerFrame, sampling); }
}
static SDL_FRect sLabelSource;
static SDL_FRect sLabelInk;
#ifdef VERSION_EU
extern uint32_t gLanguage;
#endif
static SDL_FRect sSource;
static int sAttempted;
static uint32_t* sExportPixels;
static uint64_t* sExportHashes;
static int sExportCount;
static int sLabelLanguage = -1;

static int HudLanguage(void) {
#ifdef VERSION_EU
    return (int)gLanguage;
#else
    return 0;
#endif
}

int PortHdHudLettersReady(void) { return sLabel != NULL; }

static void LoadLabel(SDL_Renderer* renderer) {
    const char* file = "assets/hd-hud/pilot/label-hp-v2.png";
    int w, h, channels, x, y, left, top, right = -1, bottom = -1;
    int inkLeft, inkTop, inkRight = -1, inkBottom = -1;
    unsigned char* pixels;
    SDL_Surface *source, *compact;
    sLabelLanguage = HudLanguage();
#ifdef VERSION_EU
    if (gLanguage == 4) file = "assets/hd-hud/pilot/label-vt-v2.png";
    else if (gLanguage == 2) file = "assets/hd-hud/pilot/label-lp-v1.png";
    else if (gLanguage == 3) file = "assets/hd-hud/pilot/label-pv-v3.png";
#endif
    if (!stbi_info(PortConfigPath(file), &w, &h, &channels) || channels != 4 ||
        w < 1 || h < 1 || w > 4096 || h > 4096 || (size_t)w * h > 4194304) return;
    pixels = stbi_load(PortConfigPath(file), &w, &h, &channels, 4);
    if (!pixels) return;
    left = w; top = h;
    inkLeft = w; inkTop = h;
    for (y = 0; y < h; y++) for (x = 0; x < w; x++) {
        unsigned char* p = pixels + ((size_t)y * w + x) * 4;
        int shade = SDL_max(p[0], SDL_max(p[1], p[2]));
        if (p[3] && (!x || !y || x == w - 1 || y == h - 1)) { stbi_image_free(pixels); return; }
        if (p[3] >= 32) {
            left = SDL_min(left, x); right = SDL_max(right, x);
            top = SDL_min(top, y); bottom = SDL_max(bottom, y);
        }
        if (p[3] >= 128 && shade >= 128) {
            inkLeft = SDL_min(inkLeft, x); inkRight = SDL_max(inkRight, x);
            inkTop = SDL_min(inkTop, y); inkBottom = SDL_max(inkBottom, y);
        }
        if (!p[3]) p[0] = p[1] = p[2] = 0;
    }
    if (right < left || inkRight < inkLeft) { stbi_image_free(pixels); return; }
    source = SDL_CreateSurfaceFrom(right - left + 1, bottom - top + 1, SDL_PIXELFORMAT_RGBA32,
                                  pixels + ((size_t)top * w + left) * 4, w * 4);
    if (source) {
        int cw = SDL_min(source->w, 256), ch = SDL_max(1, source->h * cw / source->w);
        compact = SDL_CreateSurface(cw, ch, SDL_PIXELFORMAT_RGBA32);
        SDL_SetSurfaceBlendMode(source, SDL_BLENDMODE_NONE);
        if (compact && SDL_BlitSurfaceScaled(source, NULL, compact, NULL, SDL_SCALEMODE_LINEAR)) {
            sLabel = SDL_CreateTextureFromSurface(renderer, compact);
            for (y = 0; y < ch; y++) for (x = 0; x < cw; x++) {
                unsigned char* p = (unsigned char*)compact->pixels + y * compact->pitch + x * 4;
                p[0] = p[1] = p[2] = 255;
            }
            sWhiteLabel = SDL_CreateTextureFromSurface(renderer, compact);
            sLabelSource = (SDL_FRect){0, 0, cw, ch};
            sLabelInk = (SDL_FRect){(inkLeft - left) * cw / (float)source->w,
                (inkTop - top) * ch / (float)source->h, (inkRight - inkLeft + 1) * cw / (float)source->w,
                (inkBottom - inkTop + 1) * ch / (float)source->h};
        }
        if (compact) SDL_DestroySurface(compact);
        SDL_DestroySurface(source);
    }
    stbi_image_free(pixels);
    if (sLabel) {
        SDL_SetTextureBlendMode(sLabel, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(sLabel, SDL_strcmp(SDL_GetRendererName(renderer), "software") == 0 ?
                                SDL_SCALEMODE_NEAREST : SDL_SCALEMODE_LINEAR);
        PortLog("HD HUD: cached label %s", file);
    }
    if (sWhiteLabel) {
        SDL_SetTextureBlendMode(sWhiteLabel, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(sWhiteLabel, SDL_strcmp(SDL_GetRendererName(renderer), "software") == 0 ?
                                SDL_SCALEMODE_NEAREST : SDL_SCALEMODE_LINEAR);
    }
}

/* Developer-only ROM-local references; never ship these extracted sprites. */
uint32_t* PortHdHudExportBuffer(void) {
    if (!SDL_getenv("KHCOM_HUD_EXPORT")) return NULL;
    if (!sExportPixels) sExportPixels = SDL_calloc(64 * 64, sizeof(uint32_t));
    return sExportPixels;
}

void PortHdHudExportSprite(int w, int h, int part, int ax, int ay) {
    uint64_t hash = 14695981039346656037ULL;
    SDL_Surface* surface;
    char name[160];
    int i;
    FILE* f;
    if (!sExportPixels) return;
    hash ^= (unsigned)w; hash *= 1099511628211ULL;
    hash ^= (unsigned)h; hash *= 1099511628211ULL;
    for (i = 0; i < w * h; i++) { hash ^= sExportPixels[i]; hash *= 1099511628211ULL; }
    if (!sExportHashes) sExportHashes = SDL_calloc(2048, sizeof(uint64_t));
    if (!sExportHashes) return;
    for (i = 0; i < sExportCount; i++) if (sExportHashes[i] == hash) return;
    if (sExportCount == 2048) return;
    if (!SDL_CreateDirectory(PortConfigPath("hud-source"))) return;
    SDL_snprintf(name, sizeof(name), "hud-source/%016llx.bmp", (unsigned long long)hash);
    surface = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_ARGB8888, sExportPixels, w * 4);
    if (!surface) return;
    if (!SDL_SaveBMP(surface, PortConfigPath(name))) { SDL_DestroySurface(surface); return; }
    SDL_DestroySurface(surface);
    sExportHashes[sExportCount++] = hash;
    f = fopen(PortConfigPath("hud-source/index.csv"), "a");
    if (f) {
        fprintf(f, "%016llx,%d,%d,%d,%d,%d\n", (unsigned long long)hash, w, h, part, ax, ay);
        fclose(f);
    }
}

void PortHdHudShutdown(void) {
    if (sPlayerFrame) SDL_DestroyTexture(sPlayerFrame);
    if (sWhitePlayerFrame) SDL_DestroyTexture(sWhitePlayerFrame);
    sPlayerFrame = sWhitePlayerFrame = NULL;
    if (sFrame) SDL_DestroyTexture(sFrame);
    if (sWhiteFrame) SDL_DestroyTexture(sWhiteFrame);
    if (sLabel) SDL_DestroyTexture(sLabel);
    if (sWhiteLabel) SDL_DestroyTexture(sWhiteLabel);
    sFrame = NULL;
    sWhiteFrame = NULL;
    sLabel = NULL;
    sWhiteLabel = NULL;
    SDL_free(sExportPixels); sExportPixels = NULL;
    SDL_free(sExportHashes); sExportHashes = NULL;
}

void PortHdHudPrepare(SDL_Renderer* renderer) {
    int w, h, channels, x, y, left, top, right = -1, bottom = -1;
    unsigned char* pixels;
    SDL_Surface* surface;
    SDL_Surface* compact;
    SDL_ScaleMode sampling;
    const char* path;
    if (!gPortConfig.hdHud) return;
    if (sAttempted) {
        if (sFrame && sLabelLanguage != HudLanguage()) {
            if (sLabel) SDL_DestroyTexture(sLabel);
            if (sWhiteLabel) SDL_DestroyTexture(sWhiteLabel);
            sLabel = NULL;
            sWhiteLabel = NULL;
            LoadLabel(renderer);
        }
        return;
    }
    sAttempted = 1;
    path = PortConfigPath("assets/hd-hud/pilot/enemy-hp-frame-v2.png");
    if (!stbi_info(path, &w, &h, &channels) || channels != 4 || w < 1 || h < 1 ||
        w > 4096 || h > 4096 || (size_t)w * h > 4194304) {
        PortLog("HD HUD: missing or invalid RGBA pilot; original HUD retained");
        return;
    }
    pixels = stbi_load(path, &w, &h, &channels, 4);
    if (!pixels) return;
    left = w; top = h;
    for (y = 0; y < h; y++) for (x = 0; x < w; x++) {
        unsigned char* p = pixels + ((size_t)y * w + x) * 4;
        if (!p[3]) { p[0] = p[1] = p[2] = 0; continue; }
        if (!x || !y || x == w - 1 || y == h - 1) {
            PortLog("HD HUD: exterior is not transparent; original HUD retained");
            stbi_image_free(pixels);
            return;
        }
        /* Tiny alpha noise outside the drawing must not change its layout.
           Keep antialiasing inside the crop; locate the shape with alpha 32+. */
        if (p[3] >= 32) {
            left = SDL_min(left, x); right = SDL_max(right, x);
            top = SDL_min(top, y); bottom = SDL_max(bottom, y);
        }
    }
    if (right < left) { stbi_image_free(pixels); return; }
    left = SDL_max(0, left - 1); top = SDL_max(0, top - 1);
    right = SDL_min(w - 1, right + 1); bottom = SDL_min(h - 1, bottom + 1);
    surface = SDL_CreateSurfaceFrom(right - left + 1, bottom - top + 1, SDL_PIXELFORMAT_RGBA32,
                                    pixels + ((size_t)top * w + left) * 4, w * 4);
    if (surface) {
        int cw = SDL_min(surface->w, 512);
        int ch = SDL_max(1, surface->h * cw / surface->w);
        compact = SDL_CreateSurface(cw, ch, SDL_PIXELFORMAT_RGBA32);
        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
        if (!compact || !SDL_BlitSurfaceScaled(surface, NULL, compact, NULL, SDL_SCALEMODE_LINEAR)) {
            if (compact) SDL_DestroySurface(compact);
            SDL_DestroySurface(surface);
            stbi_image_free(pixels);
            return;
        }
        sFrame = SDL_CreateTextureFromSurface(renderer, compact);
        /* Cache an alpha-matched white mask for the cartridge's white fades. */
        for (y = 0; y < ch; y++) for (x = 0; x < cw; x++) {
            unsigned char* p = (unsigned char*)compact->pixels + y * compact->pitch + x * 4;
            p[0] = p[1] = p[2] = 255;
        }
        sWhiteFrame = SDL_CreateTextureFromSurface(renderer, compact);
        sSource = (SDL_FRect){0, 0, cw, ch};
        SDL_DestroySurface(compact);
        SDL_DestroySurface(surface);
    }
    stbi_image_free(pixels);
    if (!sFrame || !sWhiteFrame) { PortHdHudShutdown(); return; }
    /* SDL 3.4.18 software linear minification crashes at this panel's small
       native dimensions. Its pixels are already antialiased at load time. */
    sampling = SDL_strcmp(SDL_GetRendererName(renderer), "software") == 0 ? SDL_SCALEMODE_NEAREST : SDL_SCALEMODE_LINEAR;
    SDL_SetTextureBlendMode(sWhiteFrame, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(sWhiteFrame, sampling);
    SDL_SetTextureBlendMode(sFrame, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(sFrame, sampling);
    PortLog("HD HUD: cached transparent enemy HP pilot (%dx%d crop)", right - left + 1, bottom - top + 1);
    LoadLabel(renderer);
    if (SDL_getenv("KHCOM_HDHUD_FRAME_PROBE")) LoadPlayerFrame(renderer);
}

int PortHdHudEnemy(int visible, int ratio, int layer) {
    (void)ratio; (void)layer;
    return visible && gPortConfig.hdHud && sFrame && gPortHudScale && !PortUiActive();
}

void PortHdHudDrawLabel(SDL_Renderer* renderer, int width, int height) {
    int rect[4];
    uint32_t color;
    float scale = (float)gPortHudScale;
    SDL_FRect box, shadow;
    uint16_t bld = *(volatile uint16_t*)(uintptr_t)(GBA_IO + 0x50);
    int effect = (bld >> 6) & 3, amount = SDL_min(*(volatile uint16_t*)(uintptr_t)(GBA_IO + 0x54), 16);
    int tint = (bld & 0x10) && effect == 3 ? 16 - amount : 16;
    Uint8 r, g, b;
    if (!gPortConfig.hdHud || !sLabel || !scale || PortUiActive() || !PortPpuEnemyHudLabel(rect, &color)) return;
    box = (SDL_FRect){(width - gPortHudW * scale) * 0.5f + rect[0] * scale,
                      (height - gPortHudH * scale) * 0.5f + rect[1] * scale,
                      (rect[2] - rect[0] + 1) * scale, (rect[3] - rect[1] + 1) * scale};
    /* Fit the letter faces to native ink; the modern bevel/shadow may extend
       into the same dark surround instead of shrinking the actual letters. */
    if (sLabelInk.w > 0 && sLabelInk.h > 0) {
        float sx = box.w / sLabelInk.w, sy = box.h / sLabelInk.h;
        box.x -= sLabelInk.x * sx; box.y -= sLabelInk.y * sy;
        box.w = sLabelSource.w * sx; box.h = sLabelSource.h * sy;
    }
    /* Keep the approved green faces/pale bevel/dark relief as authored;
       monochrome tinting would wash all three into the highlight colour. */
    r = g = b = 255 * tint / 16;
    shadow = box; shadow.x += scale * 0.5f; shadow.y += scale * 0.5f;
    SDL_SetTextureColorMod(sLabel, r / 5, g / 5, b / 5);
    SDL_RenderTexture(renderer, sLabel, &sLabelSource, &shadow);
    SDL_SetTextureColorMod(sLabel, r, g, b);
    SDL_RenderTexture(renderer, sLabel, &sLabelSource, &box);
    if (sWhiteLabel && (bld & 0x10) && effect == 2 && amount) {
        SDL_SetTextureAlphaMod(sWhiteLabel, amount * 255 / 16);
        SDL_RenderTexture(renderer, sWhiteLabel, &sLabelSource, &shadow);
        SDL_RenderTexture(renderer, sWhiteLabel, &sLabelSource, &box);
    }
}

/* Palette-matched HD fill: interpolate native highlight/body/shadow rows at
   output resolution, keeping the cartridge's exact visible shape and ratio. */
static void DrawVitalityBar(SDL_Renderer* renderer, int width, int height, int player) {
    int rect[4], line, lines;
    uint32_t rows[32];
    float scale = (float)gPortHudScale;
    float ox = (width - gPortHudW * scale) * 0.5f, oy = (height - gPortHudH * scale) * 0.5f;
    if (!gPortConfig.hdHud || !scale || PortUiActive() ||
        !(player ? PortPpuPlayerHudBar(rect, rows) : PortPpuEnemyHudBar(rect, rows))) return;
    if (player) {
        const char* trace = SDL_getenv("KHCOM_HUD_FULLHP_TRACE");
        while (trace && *trace) {
            if ((uint32_t)SDL_atoi(trace) == PortFrameCount())
                PortLog("HD HUD: fullhp frame=%u value=%d", PortFrameCount(), PortPpuPlayerHudFullHp());
            trace = SDL_strchr(trace, ',');
            if (trace) trace++;
        }
    }
    lines = (int)((rect[3] - rect[1] + 1) * scale);
    for (line = 0; line < lines; line++) {
        float y = rect[1] + line / scale;
        int ya = (int)y, yb = SDL_min(ya + 1, rect[3]), x = rect[0];
        float t = y - ya;
        uint32_t a = rows[ya], b = rows[yb];
        Uint8 r = ((a >> 16) & 255) * (1 - t) + ((b >> 16) & 255) * t;
        Uint8 g = ((a >> 8) & 255) * (1 - t) + ((b >> 8) & 255) * t;
        Uint8 blue = (a & 255) * (1 - t) + (b & 255) * t;
        SDL_SetRenderDrawColor(renderer, r, g, blue, 255);
        while (x <= rect[2]) {
            int start;
            while (x <= rect[2] && !(player ? PortPpuPlayerHudBarVisible(x, ya) : PortPpuEnemyHudBarVisible(x, ya))) x++;
            start = x;
            while (x <= rect[2] && (player ? PortPpuPlayerHudBarVisible(x, ya) : PortPpuEnemyHudBarVisible(x, ya))) x++;
            if (x > start) {
                SDL_FRect row = {ox + start * scale, oy + rect[1] * scale + line, (x - start) * scale, 1};
                SDL_RenderFillRect(renderer, &row);
            }
        }
    }
}

void PortHdHudDrawBar(SDL_Renderer* renderer, int width, int height) {
    DrawVitalityBar(renderer, width, height, 0);
    DrawVitalityBar(renderer, width, height, 1);
}

void PortHdHudDraw(SDL_Renderer* renderer, int width, int height) {
    float scale = (float)gPortHudScale;
    SDL_FRect frame;
    int rect[4];
    uint16_t bld = *(volatile uint16_t*)(uintptr_t)(GBA_IO + 0x50);
    int effect = (bld >> 6) & 3;
    int amount = SDL_min(*(volatile uint16_t*)(uintptr_t)(GBA_IO + 0x54), 16);
    Uint8 tint = (bld & 0x10) && effect == 3 ? 255 * (16 - amount) / 16 : 255;
    if (PortHdHudPlayerFrameReady() && PortPpuPlayerHudFrame(rect)) {
        frame = (SDL_FRect){(width - gPortHudW * scale) * 0.5f + rect[0] * scale,
            (height - gPortHudH * scale) * 0.5f + rect[1] * scale,
            (rect[2] - rect[0] + 1) * scale, (rect[3] - rect[1] + 1) * scale};
        SDL_SetTextureColorMod(sPlayerFrame, tint, tint, tint);
        SDL_RenderTexture(renderer, sPlayerFrame, &sPlayerSource, &frame);
        if ((bld & 0x10) && effect == 2 && amount) {
            SDL_SetTextureAlphaMod(sWhitePlayerFrame, amount * 255 / 16);
            SDL_RenderTexture(renderer, sWhitePlayerFrame, &sPlayerSource, &frame);
        }
    }
    if (!gPortConfig.hdHud || !sFrame || !scale || PortUiActive() || !PortPpuEnemyHudRect(rect)) return;
    frame.w = (rect[2] - rect[0] + 1) * scale;
    frame.h = (rect[3] - rect[1] + 1) * scale;
    frame.x = (width - gPortHudW * scale) * 0.5f + rect[0] * scale;
    frame.y = (height - gPortHudH * scale) * 0.5f + rect[1] * scale;
    if (SDL_getenv("KHCOM_HDHUDLOG")) PortLog("HD HUD rect: %d,%d,%d,%d -> %.1f,%.1f %.1fx%.1f src %.1fx%.1f", rect[0], rect[1], rect[2], rect[3], frame.x, frame.y, frame.w, frame.h, sSource.w, sSource.h);
    SDL_SetTextureColorMod(sFrame, tint, tint, tint);
    SDL_RenderTexture(renderer, sFrame, &sSource, &frame);
    if ((bld & 0x10) && effect == 2 && amount) {
        SDL_SetTextureAlphaMod(sWhiteFrame, amount * 255 / 16);
        SDL_RenderTexture(renderer, sWhiteFrame, &sSource, &frame);
    }
}
