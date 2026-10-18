/* ---- controller diagram for the Controls page (included by decompui.cpp) ------- *
 * An Xbox 360-style layout drawn as pixels of a fixed 64x36 grid at one whole
 * scale (no logo, no photograph): the binding of the focused action lights up,
 * and while a gamepad binding is captured, what is being pressed does too. */

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

/* which region a logical pixel belongs to (PR_NONE: body or outside) */
struct PadShape {
    unsigned char body[36][64]; /* 0 outside, 1 body, 2 outline */
    signed char region[36][64];
};

static const PadShape& PadLayout() {
    static PadShape s;
    static bool made;
    if (made)
        return s;
    made = true;
    std::memset(s.region, PR_NONE, sizeof(s.region));
    auto disc = [&](int cx, int cy, int r, int reg) {
        for (int y = cy - r; y <= cy + r; y++)
            for (int x = cx - r; x <= cx + r; x++)
                if ((x - cx) * (x - cx) + (y - cy) * (y - cy) <= r * r + r && y >= 0 && y < 36 && x >= 0 && x < 64)
                    s.region[y][x] = (signed char)reg;
    };
    auto rect = [&](int x0, int y0, int x1, int y1, int reg) {
        for (int y = y0; y <= y1; y++)
            for (int x = x0; x <= x1; x++)
                s.region[y][x] = (signed char)reg;
    };
    /* the body: wide top, two handles below */
    for (int y = 6; y < 35; y++) {
        int l, r;
        if (y < 8) { l = 9 - (y - 6) * 2; r = 54 + (y - 6) * 2; }
        else if (y < 22) { l = 4; r = 59; }
        else { l = 4 + (y - 22) / 2; r = 59 - (y - 22) / 2; }
        for (int x = l; x <= r; x++) {
            bool handleGap = y >= 26 && x > 22 + (y - 26) && x < 41 - (y - 26);
            if (!handleGap)
                s.body[y][x] = 1;
        }
    }
    for (int y = 0; y < 36; y++)
        for (int x = 0; x < 64; x++)
            if (s.body[y][x] && (y == 0 || x == 0 || y == 35 || x == 63 || !s.body[y - 1][x] || !s.body[y + 1][x] ||
                                 !s.body[y][x - 1] || !s.body[y][x + 1]))
                s.body[y][x] = 2;
    /* shoulders and triggers above the body */
    rect(10, 0, 18, 2, PR_LT);
    rect(45, 0, 53, 2, PR_RT);
    rect(8, 4, 20, 5, PR_LB);
    rect(43, 4, 55, 5, PR_RB);
    /* left stick high, d-pad and right stick low, face buttons high right */
    disc(14, 13, 4, PR_LS);
    rect(22, 20, 24, 21, PR_UP);
    rect(22, 25, 24, 26, PR_DOWN);
    rect(19, 22, 21, 24, PR_LEFT);
    rect(25, 22, 27, 24, PR_RIGHT);
    rect(22, 22, 24, 24, PR_NONE);
    disc(41, 23, 4, PR_RS);
    disc(49, 9, 2, PR_Y);
    disc(45, 13, 2, PR_X);
    disc(53, 13, 2, PR_B);
    disc(49, 17, 2, PR_A);
    rect(26, 13, 28, 14, PR_BACK);
    rect(35, 13, 37, 14, PR_START);
    disc(31, 10, 2, PR_GUIDE);
    return s;
}

/* what the pad's binding of this action lights; while capturing a gamepad
   binding, the controls being pressed on the first gamepad as well */
static void PadDiagram(const char* focusKey) {
    const PadShape& s = PadLayout();
    bool lit[PR_COUNT] = {};
    int litDirection[PR_COUNT];
    std::fill(litDirection, litDirection + PR_COUNT, -1);
    if (focusKey != nullptr) {
        int direction, region = PadRegionOf(Ini(focusKey), &direction);
        if (region >= 0) {
            lit[region] = true;
            litDirection[region] = direction;
        }
    }
    if (!sCapture.empty() && sCapturePad) {
        int count = 0;
        SDL_JoystickID* ids = SDL_GetGamepads(&count);
        SDL_Gamepad* pad = count > 0 ? SDL_GetGamepadFromID(ids[0]) : nullptr;
        if (pad != nullptr) {
            for (int b = 0; b <= SDL_GAMEPAD_BUTTON_DPAD_RIGHT; b++)
                if (SDL_GetGamepadButton(pad, (SDL_GamepadButton)b))
                    lit[b] = true;
            for (int a = 0; a < SDL_GAMEPAD_AXIS_COUNT; a++) {
                int v = SDL_GetGamepadAxis(pad, (SDL_GamepadAxis)a), direction;
                if (std::abs(v) > 16000) {
                    int region = PadRegionOf(PORT_PAD_AXIS_BASE + a * 2 + (v < 0), &direction);
                    if (region >= 0) {
                        lit[region] = true;
                        litDirection[region] = direction;
                    }
                }
            }
        }
        SDL_free(ids);
    }

    /* one whole scale for the drawing; centred in the card */
    float avail = ImGui::GetContentRegionAvail().x;
    int scale = std::max(2, std::min((int)(avail / 64), (int)Px(7)));
    ImVec2 p = ImGui::GetCursorScreenPos();
    float ox = std::floor(p.x + (avail - 64 * scale) * 0.5f), oy = std::floor(p.y);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImU32 body = IM_COL32(214, 211, 206, 255), outline = IM_COL32(70, 68, 66, 255),
                part = IM_COL32(96, 94, 92, 255), light = ImGui::GetColorU32(kAccent);
    const ImU32 face[4] = { IM_COL32(72, 160, 64, 255), IM_COL32(200, 56, 48, 255), IM_COL32(56, 104, 200, 255),
                            IM_COL32(220, 176, 40, 255) };
    for (int y = 0; y < 36; y++)
        for (int x = 0; x < 64; x++) {
            int reg = s.region[y][x];
            ImU32 c;
            if (reg >= 0) {
                c = reg <= PR_Y ? face[reg] : part;
                bool on = lit[reg];
                /* a stick axis: only its side of the stick */
                if (on && (reg == PR_LS || reg == PR_RS) && litDirection[reg] >= 0) {
                    int cx = reg == PR_LS ? 14 : 41, cy = reg == PR_LS ? 13 : 23;
                    on = litDirection[reg] == 0 ? y < cy : litDirection[reg] == 1 ? y > cy
                       : litDirection[reg] == 2 ? x < cx : x > cx;
                }
                if (on) {
                    /* its own colour lightened, ringed with the accent */
                    bool edge = x == 0 || y == 0 || x == 63 || y == 35 || s.region[y - 1][x] != reg ||
                                s.region[y + 1][x] != reg || s.region[y][x - 1] != reg || s.region[y][x + 1] != reg;
                    ImVec4 v = ImGui::ColorConvertU32ToFloat4(c);
                    c = edge ? light : ImGui::GetColorU32(ImVec4(v.x + (1 - v.x) * 0.45f, v.y + (1 - v.y) * 0.45f,
                                                                  v.z + (1 - v.z) * 0.45f, 1));
                }
            } else if (s.body[y][x] == 2) {
                c = outline;
            } else if (s.body[y][x] == 1) {
                c = body;
            } else {
                continue;
            }
            dl->AddRectFilled(ImVec2(ox + x * scale, oy + y * scale), ImVec2(ox + (x + 1) * scale, oy + (y + 1) * scale), c);
        }
    /* letters on the face buttons and labels on shoulders and triggers */
    if (scale >= 4) {
        struct Label { int x, y; const char* text; };
        static const Label labels[] = { { 49, 17, "A" }, { 53, 13, "B" }, { 45, 13, "X" }, { 49, 9, "Y" },
                                        { 14, 1, "LT" }, { 49, 1, "RT" }, { 14, 4, "LB" }, { 49, 4, "RB" } };
        ImGui::PushFont(sFontSmall);
        for (const Label& lb : labels) {
            ImVec2 ts = ImGui::CalcTextSize(lb.text);
            dl->AddText(ImVec2(std::floor(ox + (lb.x + 0.5f) * scale - ts.x * 0.5f),
                               std::floor(oy + (lb.y + 0.5f) * scale - ts.y * 0.5f)),
                        IM_COL32(255, 255, 255, 255), lb.text);
        }
        ImGui::PopFont();
    }
    ImGui::Dummy(ImVec2(avail, 36.0f * scale + Px(6)));
}
