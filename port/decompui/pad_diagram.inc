/* ---- controller diagram for the Controls page (included by decompui.cpp) ------- *
 * The game's own Ultimate 2C sprite (port_controller_art.inc), with the same
 * regions, lighting and LT/LB/RB/RT tabs as its Options menu
 * (port_control_diagram.inc) and the web page, at one whole scale: the binding
 * of the focused action lights up, and while a gamepad binding is captured,
 * what is being pressed does too. */

enum PadRegion {
    PR_A, PR_B, PR_X, PR_Y, PR_BACK, PR_GUIDE, PR_START, PR_LS, PR_RS, PR_LB, PR_RB,
    PR_UP, PR_DOWN, PR_LEFT, PR_RIGHT, PR_LT, PR_RT, PR_COUNT, PR_NONE = -1
};

/* SDL button numbers 0..14 are in PadRegion order; axes: sticks and triggers */
static int PadRegionOf(int binding, int* direction) {
    *direction = -1; /* 0 up, 1 down, 2 left, 3 right on a stick */
    if (binding < 0)
        return PR_NONE;
    if (binding >= PORT_PAD_AXIS_BASE) {
        int encoded = binding - PORT_PAD_AXIS_BASE, axis = encoded / 2, negative = encoded & 1;
        switch (axis) {
            case SDL_GAMEPAD_AXIS_LEFTX: *direction = negative ? 2 : 3; return PR_LS;
            case SDL_GAMEPAD_AXIS_LEFTY: *direction = negative ? 0 : 1; return PR_LS;
            case SDL_GAMEPAD_AXIS_RIGHTX: *direction = negative ? 2 : 3; return PR_RS;
            case SDL_GAMEPAD_AXIS_RIGHTY: *direction = negative ? 0 : 1; return PR_RS;
            case SDL_GAMEPAD_AXIS_LEFT_TRIGGER: return PR_LT;
            case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER: return PR_RT;
        }
        return PR_NONE;
    }
    return binding <= SDL_GAMEPAD_BUTTON_DPAD_RIGHT ? binding : PR_NONE;
}

/* the Xbox name of a gamepad binding, as the list shows it */
static std::string PadSymbol(int b) {
    static const char* const buttons[] = { "A", "B", "X", "Y", "Back", "Guide", "Start", "LS (click)", "RS (click)",
                                           "LB", "RB" };
    static const char* const arrows[] = { "\xE2\x86\x91", "\xE2\x86\x93", "\xE2\x86\x90", "\xE2\x86\x92" };
    int direction, region = PadRegionOf(b, &direction);
    if (b < 0)
        return "-";
    if (region == PR_LT) return "LT";
    if (region == PR_RT) return "RT";
    if ((region == PR_LS || region == PR_RS) && direction >= 0)
        return std::string(region == PR_LS ? "LS " : "RS ") + arrows[direction];
    if (region >= PR_UP && region <= PR_RIGHT)
        return std::string(T("Cruceta ", "D-pad ")) + arrows[region - PR_UP];
    if (region >= 0 && region <= PR_RB) {
        if (region == PR_LS || region == PR_RS)
            return T(region == PR_LS ? "LS (pulsar)" : "RS (pulsar)", buttons[region]);
        return buttons[region];
    }
    return PadName(b);
}

#include <cstdint>
#include "../src/port_controller_art.inc"

enum { PAD_TOP = 7, PAD_H = CONTROL_ART_H + PAD_TOP }; /* the tabs sit above the art */

/* which region each pixel lights as (PR_NONE: shell or outside); PR_* follow
   SDL's button numbers, so these are the game's own discs and masks */
static const signed char (&PadLayout())[PAD_H][CONTROL_ART_W] {
    static signed char region[PAD_H][CONTROL_ART_W];
    static bool made;
    if (made)
        return region;
    made = true;
    std::memset(region, PR_NONE, sizeof(region));
    static const struct { int x, y, radius, id; } discs[] = {
        { 20, 27, 7, PR_LS }, { 68, 44, 7, PR_RS }, { 83, 20, 4, PR_Y }, { 75, 28, 4, PR_X },
        { 90, 28, 4, PR_B },  { 83, 36, 4, PR_A },  { 37, 18, 3, PR_BACK }, { 67, 18, 3, PR_START },
        { 52, 18, 4, PR_GUIDE }
    };
    for (const auto& d : discs)
        for (int dy = -d.radius; dy <= d.radius; dy++)
            for (int dx = -d.radius; dx <= d.radius; dx++)
                if (dx * dx + dy * dy <= d.radius * d.radius + 1)
                    region[d.y + dy][d.x + dx] = (signed char)d.id;
    /* four arms of the cross; the middle is neutral */
    for (int y = 35; y <= 54; y++)
        for (int x = 28; x <= 46; x++) {
            if (x >= 34 && x <= 39)
                region[y][x] = y < 42 ? PR_UP : y > 47 ? PR_DOWN : PR_NONE;
            else if (y >= 42 && y <= 47)
                region[y][x] = x < 34 ? PR_LEFT : PR_RIGHT;
        }
    /* the front shoulder caps; the triggers sit behind the shell: only their tabs light */
    for (int y = 7; y <= 10; y++)
        for (int x = 16; x <= 30; x++) {
            region[y][x] = PR_LB;
            region[y][CONTROL_ART_W - 1 - x] = PR_RB;
        }
    return region;
}

/* lit[r]: direction bits up 1, down 2, left 4, right 8 (a stick axis lights its side) */
static bool PadPixelLit(int r, int x, int y, const unsigned* lit) {
    if (r < 0 || r >= PR_COUNT || !lit[r])
        return false;
    if (r != PR_LS && r != PR_RS)
        return true;
    int dx = x - (r == PR_LS ? 20 : 68), dy = y - (r == PR_LS ? 27 : 44);
    return ((lit[r] & 1) && dy <= 0) || ((lit[r] & 2) && dy >= 0) || ((lit[r] & 4) && dx <= 0) ||
           ((lit[r] & 8) && dx >= 0);
}

/* what the pad's binding of this action lights; while capturing a gamepad
   binding, the controls being pressed on the first gamepad as well */
static void PadDiagram(const char* focusKey) {
    const auto& region = PadLayout();
    unsigned lit[PR_COUNT] = {};
    auto light = [&](int binding) {
        int direction, r = PadRegionOf(binding, &direction);
        if (r >= 0)
            lit[r] |= direction >= 0 ? 1u << direction : 15u;
    };
    if (focusKey != nullptr)
        light(Ini(focusKey));
    if (!sCapture.empty() && sCapturePad) {
        int count = 0;
        SDL_JoystickID* ids = SDL_GetGamepads(&count);
        SDL_Gamepad* pad = count > 0 ? SDL_GetGamepadFromID(ids[0]) : nullptr;
        if (pad != nullptr) {
            for (int b = 0; b <= SDL_GAMEPAD_BUTTON_DPAD_RIGHT; b++)
                if (SDL_GetGamepadButton(pad, (SDL_GamepadButton)b))
                    lit[b] = 15;
            for (int a = 0; a < SDL_GAMEPAD_AXIS_COUNT; a++) {
                int v = SDL_GetGamepadAxis(pad, (SDL_GamepadAxis)a);
                if (std::abs(v) > 16000)
                    light(PORT_PAD_AXIS_BASE + a * 2 + (v < 0));
            }
        }
        SDL_free(ids);
    }

    /* one whole scale for the sprite; centred in the card */
    float avail = ImGui::GetContentRegionAvail().x;
    int scale = std::max(1, std::min((int)(avail / CONTROL_ART_W), (int)Px(4)));
    ImVec2 p = ImGui::GetCursorScreenPos();
    float ox = std::floor(p.x + (avail - CONTROL_ART_W * scale) * 0.5f), oy = std::floor(p.y);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    auto pixel = [&](int x, int y, int w, int h, uint32_t argb) {
        dl->AddRectFilled(ImVec2(ox + x * scale, oy + y * scale), ImVec2(ox + (x + w) * scale, oy + (y + h) * scale),
                          IM_COL32((argb >> 16) & 255, (argb >> 8) & 255, argb & 255, 255));
    };
    for (int y = PAD_TOP; y < PAD_H; y++)
        for (int x = 0; x < CONTROL_ART_W; x++) {
            unsigned index = sControlArt[y - PAD_TOP][x];
            if (!index)
                continue; /* transparent: no plate behind the sprite */
            uint32_t c = sControlArtPalette[index];
            int r = region[y][x];
            if (PadPixelLit(r, x, y, lit)) {
                bool edge = !x || y == PAD_TOP || x == CONTROL_ART_W - 1 || y == PAD_H - 1 || region[y - 1][x] != r ||
                            region[y + 1][x] != r || region[y][x - 1] != r || region[y][x + 1] != r;
                unsigned red = (c >> 16) & 255, green = (c >> 8) & 255, blue = c & 255;
                if (edge)
                    c = 0xFF9C1000u; /* the menu's dark red ring */
                else if (red + green + blue > 180) /* warm tint; dark lettering stays legible */
                    c = 0xFF000000u | (((red + 255) / 2) << 16) | (((green + 158) / 2) << 8) | ((blue + 52) / 2);
            } else if ((x > 0 && PadPixelLit(region[y][x - 1], x - 1, y, lit)) ||
                       (x + 1 < CONTROL_ART_W && PadPixelLit(region[y][x + 1], x + 1, y, lit)) ||
                       (y > PAD_TOP && PadPixelLit(region[y - 1][x], x, y - 1, lit)) ||
                       (y + 1 < PAD_H && PadPixelLit(region[y + 1][x], x, y + 1, lit))) {
                c = 0xFFFFE68Eu; /* one-pixel light rim */
            }
            pixel(x, y, 1, 1, c);
        }
    /* LT LB RB RT tabs with the game's 3x5 lettering */
    static const struct { int x, id; const char* name; } tabs[] = {
        { 4, PR_LT, "LT" }, { 22, PR_LB, "LB" }, { 65, PR_RB, "RB" }, { 83, PR_RT, "RT" }
    };
    for (const auto& t : tabs) {
        bool on = lit[t.id] != 0;
        pixel(t.x, 0, 17, 7, on ? 0xFF9C1000u : 0xFF43343Cu);
        pixel(t.x + 1, 1, 15, 5, on ? 0xFFCB563Du : 0xFF78626Bu);
        for (int c = 0; t.name[c]; c++) {
            unsigned bits = t.name[c] == 'L' ? 0x4927 : t.name[c] == 'R' ? 0x6BAD : t.name[c] == 'T' ? 0x7492 : 0x6BAE;
            for (int yy = 0; yy < 5; yy++)
                for (int xx = 0; xx < 3; xx++)
                    if (bits & (1u << (14 - yy * 3 - xx)))
                        pixel(t.x + 5 + c * 4 + xx, 1 + yy, 1, 1, 0xFFFFF2CDu);
        }
    }
    ImGui::Dummy(ImVec2(avail, (float)PAD_H * scale + Px(6)));
}
