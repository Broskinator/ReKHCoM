/* Production input path with a real SDL virtual gamepad; no cartridge needed. */
#include <assert.h>
#include <stdio.h>
#include "../src/port_input.c"
PortConfig gPortConfig = {
    .dodgeButton=1,.stockButton=1,.unstockButton=1,
    .keys={{SDLK_X,SDLK_K},{SDLK_Z,SDLK_J},{SDLK_A,0},{SDLK_S,0},
           {SDLK_RETURN,0},{SDLK_BACKSPACE,0},{SDLK_UP,SDLK_W},{SDLK_DOWN,0},
           {SDLK_LEFT,0},{SDLK_RIGHT,0},{SDLK_D,0},{SDLK_C,0},{SDLK_V,0}},
    .pad={SDL_GAMEPAD_BUTTON_SOUTH,SDL_GAMEPAD_BUTTON_EAST,
          SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
          SDL_GAMEPAD_BUTTON_START,SDL_GAMEPAD_BUTTON_BACK,
          SDL_GAMEPAD_BUTTON_DPAD_UP,SDL_GAMEPAD_BUTTON_DPAD_DOWN,
          SDL_GAMEPAD_BUTTON_DPAD_LEFT,SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
          SDL_GAMEPAD_BUTTON_NORTH,SDL_GAMEPAD_BUTTON_WEST,SDL_GAMEPAD_BUTTON_RIGHT_STICK}
};
uint32_t PortFrameCount(void){return 0;}
void PortLog(const char* fmt,...){(void)fmt;}
static void key(SDL_Keycode code,int down){
    SDL_Event e={0};e.type=down?SDL_EVENT_KEY_DOWN:SDL_EVENT_KEY_UP;
    e.key.key=code;PortInputHandleEvent(&e);
}
int main(void){
    assert(SDL_Init(SDL_INIT_GAMEPAD|SDL_INIT_EVENTS));
    const SDL_Keycode codes[]={SDLK_X,SDLK_Z,SDLK_A,SDLK_S,SDLK_RETURN,
        SDLK_BACKSPACE,SDLK_UP,SDLK_DOWN,SDLK_LEFT,SDLK_RIGHT,SDLK_D,SDLK_C,SDLK_V};
    const uint16_t bits[]={1,2,0x200,0x100,8,4,0x40,0x80,0x20,0x10,
        PORT_KEY_STOCK,PORT_KEY_DODGE,PORT_KEY_UNSTOCK};
    for(int i=0;i<12;i++){
        key(codes[i],1);assert(PortInputKeys()==bits[i]);
        key(codes[i],0);assert(!PortInputKeys());
    }
    key(SDLK_V,1);assert(!PortInputKeys());key(SDLK_V,0); /* legacy binding ignored */
    key(SDLK_LEFT,1);key(SDLK_RIGHT,1);assert(!PortInputKeys());
    key(SDLK_RIGHT,0);assert(PortInputKeys()==0x20);key(SDLK_LEFT,0);
    assert((PortInputResolve(PORT_KEY_STOCK)&0x300)==0x300);
    PortInputResolve(PORT_KEY_DODGE);
    assert(PortTakeDodge(KEY_LEFT,0)==KEY_LEFT);
    PortInputResolve(0);PortInputResolve(PORT_KEY_DODGE);
    for(int i=0;i<30;i++)assert(!PortInputResolve(PORT_KEY_UNSTOCK));
    key(SDLK_X,1);
    SDL_Event lost={0};lost.type=SDL_EVENT_WINDOW_FOCUS_LOST;PortInputHandleEvent(&lost);
    assert(!PortInputKeys() && !PortTakeDodge(0,0));
    SDL_VirtualJoystickDesc desc;SDL_INIT_INTERFACE(&desc);
    desc.type=SDL_JOYSTICK_TYPE_GAMEPAD;desc.naxes=SDL_GAMEPAD_AXIS_COUNT;
    desc.nbuttons=SDL_GAMEPAD_BUTTON_COUNT;desc.name="ReKHCoM controls test";
    SDL_JoystickID id=SDL_AttachVirtualJoystick(&desc);assert(id);
    SDL_Joystick* joystick=SDL_OpenJoystick(id);assert(joystick);
    SDL_Gamepad* pad=SDL_OpenGamepad(id);assert(pad);
    assert(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_SOUTH,true));
    SDL_UpdateJoysticks();key(SDLK_LEFT,1);assert(PortInputKeys()==(1|0x20));
    assert(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_SOUTH,false));
    SDL_UpdateJoysticks();key(SDLK_LEFT,0);assert(!PortInputKeys());
    assert(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_RIGHT_STICK,true));
    assert(SDL_SetJoystickVirtualAxis(joystick,SDL_GAMEPAD_AXIS_RIGHTY,32767));
    SDL_UpdateJoysticks();assert(!PortInputKeys()); /* retired button and stick direction */
    SDL_CloseGamepad(pad);SDL_CloseJoystick(joystick);SDL_DetachVirtualJoystick(id);
    SDL_Quit();puts("Input: 12 keys, retired action ignored, opposite directions, stock, dodge, focus reset, SDL virtual gamepad PASS");
    return 0;
}
