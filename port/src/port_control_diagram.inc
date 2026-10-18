/* Native options menu: Ultimate 2C-inspired silhouette, 96 x 64 logical pixels.
 * All edges, labels and highlights share the framebuffer's integer pixel grid.
 * No texture filtering, device-specific remapping, or input side effects. */
static void ControlBindingName(int binding, int l, char* out, size_t size) {
    static const char* const names[] = {
        "A", "B", "X", "Y", "Back", "Guide", "Start", "LS", "RS", "LB", "RB"
    };
    static const char* const arrows[] = { "↑", "↓", "←", "→" };
    if (binding >= PORT_PAD_AXIS_BASE) {
        int axis = (binding - PORT_PAD_AXIS_BASE) / 2;
        int neg = (binding - PORT_PAD_AXIS_BASE) & 1;
        if (axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER || axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)
            SDL_strlcpy(out, axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER ? "LT" : "RT", size);
        else if (axis >= 0 && axis < 4)
            SDL_snprintf(out, size, "%s %s", axis < 2 ? "LS" : "RS", arrows[(axis & 1) ? !neg : (neg ? 2 : 3)]);
        else SDL_strlcpy(out, "?", size);
    } else if (binding >= 0 && binding < 11) SDL_strlcpy(out, names[binding], size);
    else if (binding >= SDL_GAMEPAD_BUTTON_DPAD_UP && binding <= SDL_GAMEPAD_BUTTON_DPAD_RIGHT)
        SDL_snprintf(out, size, "%s %s", l == LANG_ES ? "Cruceta" : "D-pad", arrows[binding - SDL_GAMEPAD_BUTTON_DPAD_UP]);
    else {
        const char* name = binding < 0 ? "-" : SDL_GetGamepadStringForButton((SDL_GamepadButton)binding);
        SDL_strlcpy(out, name ? name : "?", size);
    }
}

static int ControlDiagramRegion(int binding) {
    if (binding >= PORT_PAD_AXIS_BASE) {
        int axis = (binding - PORT_PAD_AXIS_BASE) / 2;
        if (axis < 4) return axis < 2 ? SDL_GAMEPAD_BUTTON_LEFT_STICK : SDL_GAMEPAD_BUTTON_RIGHT_STICK;
        if (axis < 6) return 15 + axis - 4;
        return -1;
    }
    return binding >= 0 && binding <= SDL_GAMEPAD_BUTTON_DPAD_RIGHT ? binding : -1;
}

static void ControlDiagramRect(int ox, int oy, int scale, int x, int y, int w, int h, uint32_t color) {
    Fill(ox + x * scale, oy + y * scale, w * scale, h * scale, color);
}

/* Deliberately tiny 3x5 lettering; remains on the same grid as the body. */
static void ControlDiagramLabel(int ox, int oy, int scale, int x, int y, const char* text, uint32_t color) {
    for (; *text; ++text, x += 4) {
        unsigned bits = 0;
        switch (*text) {
            case 'A': bits = 0x2BED; break; case 'B': bits = 0x6BAE; break;
            case 'X': bits = 0x5AAD; break; case 'Y': bits = 0x5A92; break;
            case 'L': bits = 0x4927; break; case 'R': bits = 0x6BAD; break;
            case 'T': bits = 0x7492; break; case 'S': bits = 0x788F; break;
            case '-': bits = 0x01C0; break; case '+': bits = 0x05D0; break;
        }
        for (int yy = 0; yy < 5; ++yy)
            for (int xx = 0; xx < 3; ++xx)
                if (bits & (1u << (14 - yy * 3 - xx)))
                    ControlDiagramRect(ox, oy, scale, x + xx, y + yy, 1, 1, color);
    }
}

static void DrawControlDiagram(int x, int y, int w, int h) {
    static unsigned char body[64][96];
    static signed char region[64][96];
    static int ready;
    static const struct { int x, y, radius, region; } discs[] = {
        {23, 28, 9, 7}, {61, 44, 9, 8},
        {75, 22, 5, 3}, {66, 31, 5, 2}, {84, 31, 5, 1}, {75, 40, 5, 0},
        {42, 27, 3, 4}, {53, 27, 3, 6}, {48, 17, 3, 5}
    };
    int scale = SDL_min(w / 96, h / 64), focus = -1, binding = -1;
    int lit[17] = {0};
    if (scale < 1) return;
    if (!ready) {
        SDL_memset(region, -1, sizeof(region));
        /* Rounded face and long tapered grips. Scanlines are symmetric;
         * the rising inner gap separates the handles from the central bridge. */
        static const unsigned char contour[] = {
            24,19,16,13,11,9,8,7,6,5,4,4,3,3,2,2,2,2,2,2,2,2,2,2,2,
            2,2,2,3,3,3,3,4,4,4,4,5,5,5,6,6,7,7,8,8,10,12,15,18,22,26,29
        };
        for (int yy = 10; yy < 62; ++yy) {
            int left = contour[yy - 10];
            for (int xx = left; xx <= 95 - left; ++xx) {
                int gap = yy >= 45 ? 47 - (yy - 45) : 48;
                if (yy >= 45 && xx > gap && xx < 95 - gap) continue;
                body[yy][xx] = 1;
            }
        }
        for (unsigned i = 0; i < sizeof(discs) / sizeof(discs[0]); ++i)
            for (int yy = -discs[i].radius; yy <= discs[i].radius; ++yy)
                for (int xx = -discs[i].radius; xx <= discs[i].radius; ++xx)
                    if (xx * xx + yy * yy <= discs[i].radius * discs[i].radius + 1)
                        region[discs[i].y + yy][discs[i].x + xx] = (signed char)discs[i].region;
        /* Separate rear triggers and front shoulders, with stepped end caps. */
        for (int yy = 0; yy < 13; ++yy)
            for (int xx = 14; xx < 35; ++xx) {
                if ((yy == 0 || yy == 7) && (xx < 17 || xx > 32)) continue;
                if (yy == 6) continue;
                region[yy][xx] = yy < 6 ? 15 : 9;
                region[yy][95 - xx] = yy < 6 ? 16 : 10;
            }
        for (int yy = 38; yy <= 52; ++yy)
            for (int xx = 29; xx <= 43; ++xx) {
                if (xx >= 34 && xx <= 38) region[yy][xx] = yy < 43 ? 11 : yy > 47 ? 12 : 17;
                else if (yy >= 43 && yy <= 47) region[yy][xx] = xx < 34 ? 13 : 14;
            }
        ready = 1;
    }
    x += (w - 96 * scale) / 2;
    y += (h - 64 * scale) / 2;
    if (sCursor >= ITEM_CONTROL_A && sCursor <= ITEM_CONTROL_UNSTOCK) {
        binding = gPortConfig.pad[sCursor - ITEM_CONTROL_A];
        focus = ControlDiagramRegion(binding);
        if (focus >= 0) lit[focus] = 1;
    }
    /* During capture also show real current input; querying an already-open
     * handle preserves ownership in port_input and never changes bindings. */
    if (sControlCapture) {
        int count = 0;
        SDL_JoystickID* ids = SDL_GetGamepads(&count);
        SDL_Gamepad* pad = count ? SDL_GetGamepadFromID(ids[0]) : NULL;
        if (pad) {
            for (int b = 0; b < 15; ++b) if (SDL_GetGamepadButton(pad, (SDL_GamepadButton)b)) lit[b] = 1;
            for (int a = 0; a < 6; ++a) {
                int v = SDL_GetGamepadAxis(pad, (SDL_GamepadAxis)a);
                if (SDL_abs(v) > 16000) lit[ControlDiagramRegion(PORT_PAD_AXIS_BASE + a * 2 + (v < 0))] = 1;
            }
        }
        SDL_free(ids);
    }
    for (int yy = 0; yy < 64; ++yy)
        for (int xx = 0; xx < 96; ++xx) {
            int r = region[yy][xx], edge;
            uint32_t color;
            if (r >= 0) {
                edge = !xx || !yy || xx == 95 || yy == 63 || region[yy-1][xx] != r ||
                       region[yy+1][xx] != r || region[yy][xx-1] != r || region[yy][xx+1] != r;
                int active = r < 17 && lit[r];
                /* A focused stick axis highlights only its corresponding half. */
                if (active && r == focus && binding >= PORT_PAD_AXIS_BASE && r < 15) {
                    int a = (binding - PORT_PAD_AXIS_BASE) / 2, neg = (binding - PORT_PAD_AXIS_BASE) & 1;
                    int delta = (a & 1) ? yy - (r == 7 ? 28 : 44) : xx - (r == 7 ? 23 : 61);
                    active = neg ? delta <= 0 : delta >= 0;
                }
                static const uint32_t face[] = { 0xFF46814Du, 0xFFAB414Du, 0xFF456DA1u, 0xFFB98937u };
                color = edge ? 0xFF292535u : r < 4 ? face[r] : 0xFF5D536Eu;
                /* Stick wells and raised caps, with a single upper-left glint. */
                if (r == 7 || r == 8) {
                    int dx = xx - (r == 7 ? 23 : 61), dy = yy - (r == 7 ? 28 : 44);
                    int distance = dx * dx + dy * dy;
                    color = distance > 53 ? 0xFF292535u : distance > 37 ? 0xFF9D8EADu :
                            dy < -3 ? 0xFF82738Eu : 0xFF494153u;
                }
                if (active) color = edge ? MENU_RED_DARK : 0xFFF77558u;
            } else if (body[yy][xx]) {
                edge = !xx || !yy || xx == 95 || yy == 63 || !body[yy-1][xx] ||
                       !body[yy+1][xx] || !body[yy][xx-1] || !body[yy][xx+1];
                /* Cel shading: broad lit face, one shadow plane on the right
                 * grip and underside, a short top-left rim; no noisy dithering. */
                color = 0xFFC9B7DCu;
                if (xx > 80 - (yy - 30) / 3 || yy > 52) color = 0xFF9583B3u;
                if (yy < 20 && xx < 67) color = 0xFFE5D9F1u;
                if (yy > 39 && xx < 29 && xx > 22 - (yy - 39) / 2) color = 0xFFB4A0CCu;
                if (yy > 11 && yy < 29 && xx > 3 && !body[yy-2][xx]) color = 0xFFF9EEFFu;
                if (edge) color = 0xFF373044u;
            } else continue;
            ControlDiagramRect(x, y, scale, xx, yy, 1, 1, color);
        }
    ControlDiagramLabel(x,y,scale,74,38,"A",0xFFFFFFFFu);
    ControlDiagramLabel(x,y,scale,83,29,"B",0xFFFFFFFFu);
    ControlDiagramLabel(x,y,scale,65,29,"X",0xFFFFFFFFu);
    ControlDiagramLabel(x,y,scale,74,20,"Y",0xFFFFFFFFu);
    ControlDiagramLabel(x,y,scale,21,0,"LT",0xFFFFFFFFu);
    ControlDiagramLabel(x,y,scale,68,0,"RT",0xFFFFFFFFu);
    ControlDiagramLabel(x,y,scale,21,7,"LB",0xFFFFFFFFu);
    ControlDiagramLabel(x,y,scale,68,7,"RB",0xFFFFFFFFu);
    ControlDiagramLabel(x,y,scale,41,25,"-",0xFFFFFFFFu);
    ControlDiagramLabel(x,y,scale,52,25,"+",0xFFFFFFFFu);
}
