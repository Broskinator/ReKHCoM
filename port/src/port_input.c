/*
 * Keyboard and gamepad input, mapped to the GBA key bits.
 */
#include <SDL3/SDL.h>

#include "port/port.h"

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

/* Frames a dodge press waits for the battle code to take it. */
#define DODGE_PENDING_FRAMES 6

#define STICK_DEADZONE 12000
#define TRIGGER_THRESHOLD 16000

/* GBA key bit of each remappable action (port.h PORT_ACT_*). */
static const uint16_t sActionBits[PORT_ACT_COUNT] = {
    KEY_A,   KEY_B,    KEY_L,    KEY_R,     KEY_START, KEY_SELECT,
    KEY_UP,  KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_STOCK, KEY_DODGE,
};

static uint16_t sKeyboard;
static int sSuppress;
static int sDodgeHeld;
static int sDodgePending;

/* After closing an overlay, ignore buttons still held until released so the
   confirming press does not reach the game. */
void PortInputSuppressUntilRelease(void) {
    sSuppress = 1;
}

void PortInputInit(void) {
    SDL_SetGamepadEventsEnabled(true);
}

void PortInputHandleEvent(const void* p) {
    const SDL_Event* ev = (const SDL_Event*)p;
    size_t i;

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
            break;
    }
}

static uint16_t GamepadKeys(SDL_Gamepad* pad) {
    uint16_t keys = 0;
    int lx, ly, i;

    for (i = 0; i < PORT_ACT_COUNT; i++) {
        uint16_t bit = sActionBits[i];
        if (gPortConfig.pad[i] < 0 || !SDL_GetGamepadButton(pad, (SDL_GamepadButton)gPortConfig.pad[i]))
            continue;
        /* with the one-button options off, their buttons are B and A again */
        if (i == PORT_ACT_DODGE && !gPortConfig.dodgeButton)
            bit = KEY_B;
        if (i == PORT_ACT_STOCK && !gPortConfig.stockButton)
            bit = KEY_A;
        keys |= bit;
    }
    if (SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) > TRIGGER_THRESHOLD)
        keys |= KEY_L;
    if (SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > TRIGGER_THRESHOLD)
        keys |= KEY_R;

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
    return keys;
}

uint16_t PortInputKeys(void) {
    uint16_t keys = sKeyboard;
    SDL_JoystickID* ids;
    int count, i;

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
