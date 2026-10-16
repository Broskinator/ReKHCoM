#include <SDL3/SDL.h>
#include <math.h>
#include "port/port.h"
#include "port/port_android.h"
#include "touch_icons.h"

typedef struct Finger { SDL_FingerID id; float x,y; int active; } Finger;
typedef struct Button { float x,y; uint16_t bits; const char* text; int icon; } Button;
static Finger fingers[10];
static float aspect=16.0f/9.0f;
static const Button buttons[]={
    {.13f,.34f,0x200,"L",0},{.87f,.34f,0x100,"R",0},
    {.91f,.62f,1,"A",0},{.83f,.79f,2,"B",0},
    {.73f,.71f,PORT_KEY_DODGE,"",1},{.81f,.49f,PORT_KEY_STOCK,"",2},
    {.45f,.93f,4,"SELECT",0},{.55f,.93f,8,"START",0},
    {.95f,.17f,12,"...",0}
};
static SDL_Texture* icons[2];
void PortAndroidTouchClear(void) { SDL_memset(fingers,0,sizeof(fingers)); }
void PortAndroidTouchEvent(const void* event) {
    const SDL_Event* e=event;
    int i, freeSlot=-1;
    if(e->type==SDL_EVENT_WINDOW_FOCUS_LOST || e->type==SDL_EVENT_WILL_ENTER_BACKGROUND) {
        PortAndroidTouchClear(); return;
    }
    if(e->type!=SDL_EVENT_FINGER_DOWN && e->type!=SDL_EVENT_FINGER_MOTION &&
       e->type!=SDL_EVENT_FINGER_UP && e->type!=SDL_EVENT_FINGER_CANCELED) return;
    for(i=0;i<10;i++) {
        if(!fingers[i].active) { if(freeSlot<0) freeSlot=i; continue; }
        if(fingers[i].id!=e->tfinger.fingerID) continue;
        fingers[i].x=e->tfinger.x; fingers[i].y=e->tfinger.y;
        if(e->type==SDL_EVENT_FINGER_UP || e->type==SDL_EVENT_FINGER_CANCELED) fingers[i].active=0;
        return;
    }
    if(e->type==SDL_EVENT_FINGER_DOWN && freeSlot>=0) {
        fingers[freeSlot]=(Finger){e->tfinger.fingerID,e->tfinger.x,e->tfinger.y,1};
    }
}
uint16_t PortAndroidTouchKeys(void) {
    uint16_t keys=0;
    int i,b;
    for(i=0;i<10;i++) if(fingers[i].active) {
        float dx=(fingers[i].x-.13f)*aspect,dy=fingers[i].y-.73f;
        if(SDL_fabsf(dx)<.17f && SDL_fabsf(dy)<.17f) {
            if(dx<-.035f) keys|=0x20;
            if(dx>.035f) keys|=0x10;
            if(dy<-.035f) keys|=0x40;
            if(dy>.035f) keys|=0x80;
            continue;
        }
        for(b=0;b<(int)SDL_arraysize(buttons);b++) {
            float x=(fingers[i].x-buttons[b].x)*aspect,y=fingers[i].y-buttons[b].y;
            float radius=b==6 || b==7 ? .055f : .075f;
            if(x*x+y*y<=radius*radius) { keys|=buttons[b].bits; break; }
        }
    }
    return keys;
}
static void Circle(SDL_Renderer* r,float x,float y,float radius,SDL_FColor color) {
    SDL_Vertex v[18]; int indices[48],i;
    v[0]=(SDL_Vertex){{x,y},color,{0,0}};
    for(i=0;i<=16;i++) {
        float a=i*6.28318530718f/16;
        v[i+1]=(SDL_Vertex){{x+SDL_cosf(a)*radius,y+SDL_sinf(a)*radius},color,{0,0}};
        if(i<16) {indices[i*3]=0;indices[i*3+1]=i+1;indices[i*3+2]=i+2;}
    }
    SDL_RenderGeometry(r,NULL,v,18,indices,48);
}
static void DrawButton(SDL_Renderer* r,int w,int h,float x,float y,float radius,const char* text,int icon,int active) {
    float px=x*w,py=y*h,rr=radius*h;
    SDL_FColor edge={1,1,1,active?.9f:.5f},inside={.04f,.06f,.11f,active?.75f:.25f};
    Circle(r,px,py,rr,edge); Circle(r,px,py,rr-2,inside);
    if(icon) {
        SDL_FRect rect={px-rr*.66f,py-rr*.66f,rr*1.32f,rr*1.32f};
        SDL_SetTextureAlphaMod(icons[icon-1],active?240:150);
        SDL_RenderTexture(r,icons[icon-1],NULL,&rect);
    } else {
        float scale=SDL_max(1.0f,(radius*h)/(SDL_strlen(text)>2?25.0f:10.0f));
        float oldX,oldY; SDL_GetRenderScale(r,&oldX,&oldY);
        SDL_SetRenderScale(r,scale,scale);SDL_SetRenderDrawColor(r,255,255,255,active?240:150);
        SDL_RenderDebugText(r,px/scale-SDL_strlen(text)*4,py/scale-4,text);
        SDL_SetRenderScale(r,oldX,oldY);
    }
}
void PortAndroidTouchDraw(SDL_Renderer* r) {
    int w,h,b; uint16_t keys=PortAndroidTouchKeys();
    SDL_GetCurrentRenderOutputSize(r,&w,&h); if(w<=0 || h<=0)return;
    aspect=(float)w/h;
    if(!icons[0]) for(b=0;b<2;b++) {
        icons[b]=SDL_CreateTexture(r,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_STATIC,32,32);
        if(!icons[b])return;
        SDL_UpdateTexture(icons[b],NULL,b?touch_stock:touch_roll,32*4);
        SDL_SetTextureBlendMode(icons[b],SDL_BLENDMODE_BLEND);
    }
    SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
    for(b=0;b<(int)SDL_arraysize(buttons);b++) DrawButton(r,w,h,buttons[b].x,buttons[b].y,
        b==6 || b==7?.05f:.066f,buttons[b].text,buttons[b].icon,(keys&buttons[b].bits)==buttons[b].bits);
    DrawButton(r,w,h,.13f,.73f-.10f,.063f,"^",0,keys&0x40);
    DrawButton(r,w,h,.13f,.73f+.10f,.063f,"v",0,keys&0x80);
    DrawButton(r,w,h,.13f-.10f/aspect,.73f,.063f,"<",0,keys&0x20);
    DrawButton(r,w,h,.13f+.10f/aspect,.73f,.063f,">",0,keys&0x10);
}
