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

#define STICK_DEADZONE 12000
#define TRIGGER_THRESHOLD 16000

typedef struct KeyBind {
    SDL_Keycode key;
    uint16_t bit;
} KeyBind;

static const KeyBind sKeyBinds[] = {
    { SDLK_X, KEY_A },        { SDLK_Z, KEY_B },          { SDLK_A, KEY_L },
    { SDLK_S, KEY_R },        { SDLK_RETURN, KEY_START }, { SDLK_BACKSPACE, KEY_SELECT },
    { SDLK_RSHIFT, KEY_SELECT }, { SDLK_UP, KEY_UP },     { SDLK_DOWN, KEY_DOWN },
    { SDLK_LEFT, KEY_LEFT },  { SDLK_RIGHT, KEY_RIGHT },  { SDLK_K, KEY_A },
    { SDLK_J, KEY_B },        { SDLK_W, KEY_UP },
};

static uint16_t sKeyboard;

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
            for (i = 0; i < sizeof(sKeyBinds) / sizeof(sKeyBinds[0]); i++) {
                if (sKeyBinds[i].key != ev->key.key)
                    continue;
                if (ev->type == SDL_EVENT_KEY_DOWN)
                    sKeyboard |= sKeyBinds[i].bit;
                else
                    sKeyboard &= ~sKeyBinds[i].bit;
            }
            break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            sKeyboard = 0;
            break;
    }
}

static uint16_t GamepadKeys(SDL_Gamepad* pad) {
    uint16_t keys = 0;
    int lx, ly;

    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_SOUTH))
        keys |= KEY_A;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_EAST))
        keys |= KEY_B;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_WEST))
        keys |= KEY_B;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_NORTH))
        keys |= KEY_A;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_START))
        keys |= KEY_START;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_BACK))
        keys |= KEY_SELECT;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER))
        keys |= KEY_L;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER))
        keys |= KEY_R;
    if (SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) > TRIGGER_THRESHOLD)
        keys |= KEY_L;
    if (SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > TRIGGER_THRESHOLD)
        keys |= KEY_R;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_UP))
        keys |= KEY_UP;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_DOWN))
        keys |= KEY_DOWN;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_LEFT))
        keys |= KEY_LEFT;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT))
        keys |= KEY_RIGHT;

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
    return keys;
}
