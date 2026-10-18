/*
 * Keyboard and gamepad input, mapped to the GBA key bits.
 */
#include <SDL3/SDL.h>

#include "port/port.h"
#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
#include "port/port_android.h"
#endif

#define KEY_A 0x001
#define KEY_B 0x002
#define KEY_SELECT 0x004
#define KEY_START 0x008
#define KEY_RIGHT 0x010
#define KEY_LEFT 0x020
#define KEY_UP 0x040
#define KEY_DOWN 0x080
#define KEY_R 0x100
#define KEY_L 0x200
#define KEY_STOCK PORT_KEY_STOCK
#define KEY_DODGE PORT_KEY_DODGE
#define KEY_UNSTOCK PORT_KEY_UNSTOCK

/* Frames a dodge press waits for the battle code to take it. */
#define DODGE_PENDING_FRAMES 6
/* Frames the return-stocked-cards button is held before it acts (0.5 s). */
#define UNSTOCK_HOLD_FRAMES 30

#define STICK_DEADZONE 12000
#define TRIGGER_THRESHOLD 16000

/* GBA key bit of each remappable action (port.h PORT_ACT_*). */
static const uint16_t sActionBits[PORT_ACT_COUNT] = {
    KEY_A,   KEY_B,    KEY_L,    KEY_R,     KEY_START, KEY_SELECT,
    KEY_UP,  KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_STOCK, KEY_DODGE, KEY_UNSTOCK,
};

static uint16_t sKeyboard;
static int sSuppress;
static int sDodgeHeld;
static int sDodgePending;
static int sUnstockHeld;
static int sUnstockPending;

/* After closing an overlay, ignore buttons still held until released so the
   confirming press does not reach the game. */
void PortInputSuppressUntilRelease(void) {
    sSuppress = 1;
    sDodgeHeld = sDodgePending = 0;
    sUnstockHeld = sUnstockPending = 0;
}

/* Test harness: KHCOM_VPAD="from-to:BUTTON,..." plugs in a virtual gamepad
   and holds its buttons on those frames (SOUTH, EAST, WEST, NORTH, START,
   BACK, LB, RB, LT, RT, UP, DOWN, LEFT, RIGHT, RS_LEFT, RS_RIGHT, RS_UP, RS_DOWN). */
static SDL_Joystick* sVirtualPad;

static void VirtualPadUpdate(void) {
    static const struct { const char* name; int button; int axis; int sign; } kNames[] = {
        { "SOUTH", SDL_GAMEPAD_BUTTON_SOUTH, -1, 0 }, { "EAST", SDL_GAMEPAD_BUTTON_EAST, -1, 0 },
        { "WEST", SDL_GAMEPAD_BUTTON_WEST, -1, 0 }, { "NORTH", SDL_GAMEPAD_BUTTON_NORTH, -1, 0 },
        { "START", SDL_GAMEPAD_BUTTON_START, -1, 0 }, { "BACK", SDL_GAMEPAD_BUTTON_BACK, -1, 0 },
        { "LB", SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, -1, 0 }, { "RB", SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, -1, 0 },
        { "LT", -1, SDL_GAMEPAD_AXIS_LEFT_TRIGGER, 1 }, { "RT", -1, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 1 },
        { "UP", SDL_GAMEPAD_BUTTON_DPAD_UP, -1, 0 }, { "DOWN", SDL_GAMEPAD_BUTTON_DPAD_DOWN, -1, 0 },
        { "LEFT", SDL_GAMEPAD_BUTTON_DPAD_LEFT, -1, 0 }, { "RIGHT", SDL_GAMEPAD_BUTTON_DPAD_RIGHT, -1, 0 },
        { "RS_LEFT", -1, SDL_GAMEPAD_AXIS_RIGHTX, -1 }, { "RS_RIGHT", -1, SDL_GAMEPAD_AXIS_RIGHTX, 1 },
        { "RS_UP", -1, SDL_GAMEPAD_AXIS_RIGHTY, -1 }, { "RS_DOWN", -1, SDL_GAMEPAD_AXIS_RIGHTY, 1 },
    };
    const char* spec = SDL_getenv("KHCOM_VPAD");
    int held[SDL_GAMEPAD_BUTTON_COUNT] = { 0 }, axes[SDL_GAMEPAD_AXIS_COUNT] = { 0 }, i;
    unsigned frame = PortFrameCount();
    if (sVirtualPad == NULL || spec == NULL)
        return;
    /* Joystick trigger axes use the full signed range; SDL maps their rest
       position to gamepad zero. Raw zero would mean a half-held trigger. */
    axes[SDL_GAMEPAD_AXIS_LEFT_TRIGGER] = -32768;
    axes[SDL_GAMEPAD_AXIS_RIGHT_TRIGGER] = -32768;
    while (*spec) {
        unsigned a = 0, b = 0;
        char name[16] = { 0 };
        int n = 0;
        if (SDL_sscanf(spec, "%u-%u:%15[A-Z_]%n", &a, &b, name, &n) < 3 || n == 0)
            break;
        for (i = 0; i < (int)SDL_arraysize(kNames); i++)
            if (SDL_strcmp(name, kNames[i].name) == 0 && frame >= a && frame <= b) {
                if (kNames[i].button >= 0) held[kNames[i].button] = 1;
                else axes[kNames[i].axis] = kNames[i].sign * 32767;
            }
        spec += n;
        if (*spec == ',') spec++;
    }
    for (i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; i++)
        SDL_SetJoystickVirtualButton(sVirtualPad, i, held[i]);
    for (i = 0; i < SDL_GAMEPAD_AXIS_COUNT; i++)
        SDL_SetJoystickVirtualAxis(sVirtualPad, i, (Sint16)axes[i]);
    SDL_UpdateJoysticks();
}

void PortInputInit(void) {
    SDL_SetGamepadEventsEnabled(true);
    if (SDL_getenv("KHCOM_VPAD")) {
        SDL_VirtualJoystickDesc desc;
        SDL_JoystickID id;
        SDL_INIT_INTERFACE(&desc);
        desc.type = SDL_JOYSTICK_TYPE_GAMEPAD;
        desc.naxes = SDL_GAMEPAD_AXIS_COUNT;
        desc.nbuttons = SDL_GAMEPAD_BUTTON_COUNT;
        desc.name = "KHCOM virtual pad";
        id = SDL_AttachVirtualJoystick(&desc);
        sVirtualPad = id ? SDL_OpenJoystick(id) : NULL;
        PortLog("vpad: %s", sVirtualPad ? "attached" : SDL_GetError());
    }
}

void PortInputHandleEvent(const void* p) {
    const SDL_Event* ev = (const SDL_Event*)p;
    size_t i;
#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
    PortAndroidTouchEvent(p);
#endif

    switch (ev->type) {
        case SDL_EVENT_GAMEPAD_ADDED:
            SDL_OpenGamepad(ev->gdevice.which);
            break;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            if (ev->key.repeat)
                break;
            for (i = 0; i < PORT_ACT_COUNT * 2; i++) {
                if (gPortConfig.keys[i / 2][i % 2] == 0 || (SDL_Keycode)gPortConfig.keys[i / 2][i % 2] != ev->key.key)
                    continue;
                if (ev->type == SDL_EVENT_KEY_DOWN)
                    sKeyboard |= sActionBits[i / 2];
                else
                    sKeyboard &= ~sActionBits[i / 2];
            }
            break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            sKeyboard = 0;
            sDodgeHeld = sDodgePending = 0;
            sUnstockHeld = sUnstockPending = 0;
            break;
    }
}

static uint16_t GamepadKeys(SDL_Gamepad* pad) {
    uint16_t keys = 0;
    int lx, ly, i;

    for (i = 0; i < PORT_ACT_COUNT; i++) {
        uint16_t bit = sActionBits[i];
        int binding = gPortConfig.pad[i];
        if (binding < 0) continue;
        if (binding >= PORT_PAD_AXIS_BASE) {
            int encoded = binding - PORT_PAD_AXIS_BASE;
            int axis = encoded / 2, negative = encoded & 1;
            int value;
            if (axis >= SDL_GAMEPAD_AXIS_COUNT) continue;
            value = SDL_GetGamepadAxis(pad, (SDL_GamepadAxis)axis);
            if ((!negative && value <= TRIGGER_THRESHOLD) || (negative && value >= -TRIGGER_THRESHOLD)) continue;
        } else if (!SDL_GetGamepadButton(pad, (SDL_GamepadButton)binding)) continue;
        /* with the one-button options off, their buttons are B and A again */
        if (i == PORT_ACT_DODGE && !gPortConfig.dodgeButton)
            bit = KEY_B;
        if (i == PORT_ACT_STOCK && !gPortConfig.stockButton)
            bit = KEY_A;
        keys |= bit;
    }
    lx = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTX);
    ly = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTY);
    if (lx < -STICK_DEADZONE)
        keys |= KEY_LEFT;
    if (lx > STICK_DEADZONE)
        keys |= KEY_RIGHT;
    if (ly < -STICK_DEADZONE)
        keys |= KEY_UP;
    if (ly > STICK_DEADZONE)
        keys |= KEY_DOWN;

    /* Right stick for the cards: left / right = L / R, up = L + R (stock). */
    lx = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHTX);
    ly = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHTY);
    if (lx < -STICK_DEADZONE * 2)
        keys |= KEY_L;
    if (lx > STICK_DEADZONE * 2)
        keys |= KEY_R;
    if (ly < -STICK_DEADZONE * 2)
        keys |= KEY_L | KEY_R;
    /* down held returns the stocked cards */
    if (ly > STICK_DEADZONE * 2)
        keys |= KEY_UNSTOCK;
    return keys;
}

uint16_t PortInputKeys(void) {
    uint16_t keys = sKeyboard;
    SDL_JoystickID* ids;
    int count, i;
#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
    keys |= PortAndroidTouchKeys();
#endif

    VirtualPadUpdate();
    ids = SDL_GetGamepads(&count);
    if (ids != NULL) {
        for (i = 0; i < count; i++) {
            SDL_Gamepad* pad = SDL_GetGamepadFromID(ids[i]);
            if (pad == NULL)
                pad = SDL_OpenGamepad(ids[i]);
            if (pad != NULL)
                keys |= GamepadKeys(pad);
        }
        SDL_free(ids);
    }
    if (!gPortConfig.unstockButton)
        keys &= ~KEY_UNSTOCK;
    /* The hardware cannot report opposite directions at once. */
    if ((keys & (KEY_LEFT | KEY_RIGHT)) == (KEY_LEFT | KEY_RIGHT))
        keys &= ~(KEY_LEFT | KEY_RIGHT);
    if ((keys & (KEY_UP | KEY_DOWN)) == (KEY_UP | KEY_DOWN))
        keys &= ~(KEY_UP | KEY_DOWN);
    if (sSuppress) {
        if (keys != 0)
            return 0;
        sSuppress = 0;
    }
    return keys;
}

uint16_t PortInputResolve(uint16_t keys) {
    int dodge = (keys & KEY_DODGE) && gPortConfig.dodgeButton;

    /* Stock button: L and R together, without pressing both shoulders. */
    if ((keys & KEY_STOCK) && gPortConfig.stockButton)
        keys |= KEY_L | KEY_R;
    /* Dodge button: a press is kept for a few frames for the battle code. */
    if (dodge && !sDodgeHeld)
        sDodgePending = DODGE_PENDING_FRAMES;
    else if (sDodgePending > 0)
        sDodgePending--;
    sDodgeHeld = dodge;
    /* Return-stocked-cards button: acts once when held long enough. */
    if ((keys & KEY_UNSTOCK) && gPortConfig.unstockButton) {
        if (sUnstockHeld <= UNSTOCK_HOLD_FRAMES && ++sUnstockHeld == UNSTOCK_HOLD_FRAMES)
            sUnstockPending = DODGE_PENDING_FRAMES;
    } else {
        sUnstockHeld = 0;
        if (!gPortConfig.unstockButton)
            sUnstockPending = 0;
    }
    if (sUnstockPending > 0 && sUnstockHeld != UNSTOCK_HOLD_FRAMES)
        sUnstockPending--;
    return keys & 0x3FF;
}

/* Battle code: the dodge button acts as the second tap of a d-pad double tap
   toward the held direction, or the way the character faces. Returns that
   direction's key, or 0 without a pending press. */
int PortTakeDodge(int held, int facingLeft) {
    if (sDodgePending == 0)
        return 0;
    sDodgePending = 0;
    if (held & KEY_LEFT)
        return KEY_LEFT;
    if (held & KEY_RIGHT)
        return KEY_RIGHT;
    return facingLeft ? KEY_LEFT : KEY_RIGHT;
}

/* Battle code: 1 once after the return-stocked-cards button was held. */
int PortTakeUnstock(void) {
    int pending = gPortConfig.unstockButton && sUnstockPending > 0;

    sUnstockPending = 0;
    return pending;
}
