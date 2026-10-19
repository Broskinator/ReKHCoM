/* Read-only, portable list inside Options. Account controls stay in launcher. */
static int sAchievementGroup, sAchievementChoice, sAchievementScroll, sAchievementView;
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
    int x = 10, top = MENU_TOP + 23, rows = SDL_max(1, (height - top - 57) / 16), i, y;
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
        int dx = x, dy = height - 48;
        PortAchievementsListGet(sAchievementGroup, indices[sAchievementChoice], es, &info);
        p = info.description;
        while (*p && dy < height - 22) {
            int code = NextChar(&p), advance = GlyphWidth(code);
            if (dx + advance > right) { dx = x; dy += 11; }
            if (dy >= height - 22) break;
            DrawGlyph(code, dx, dy, MENU_INK, 0xFFD8D8D8u); dx += advance;
        }
    }
    AchievementMenuText(sAchievementGroup ? (es ? "Estado de la última consulta del launcher" : "Status from last launcher account query") :
        (es ? "Retos locales de esta región" : "Local challenges for this region"), x, height - 22, right, MENU_DIM);
    AchievementMenuText(PortAchievementsListCount(1) ? (es ? "Arriba/abajo: lista  Izq/der: filtro  Tab/Y: grupo  B: volver" : "Up/down: list  Left/right: filter  Tab/Y: group  B: back") :
        (es ? "Arriba/abajo: lista  Izq/der: filtro  B: volver" : "Up/down: list  Left/right: filter  B: back"), x, height - 11, right, MENU_DIM);
}
