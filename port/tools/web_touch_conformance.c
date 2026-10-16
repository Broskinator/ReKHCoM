/* Exercise actual centered hit testing at multiple aspect ratios. */
#define PORT_TOUCH_CENTERED_LAYOUT
#include <assert.h>
#include <stdio.h>
#include "../android/native/touch_android.c"
int PortUiActive(void){return 0;}
void PortUiOpenOptions(void){}
int PortUiHandleEvent(const void* event){(void)event;return 0;}
static void point(int id,float x,float y) {
    SDL_Event event={0};event.type=SDL_EVENT_FINGER_DOWN;event.tfinger.fingerID=id;
    event.tfinger.x=x;event.tfinger.y=y;PortAndroidTouchEvent(&event);
}
int main(void) {
    int sizes[][2]={{1920,1080},{5120,1440},{900,1600},{360,640},{1052,592}},i;
    for(i=0;i<5;i++) {
        float w=sizes[i][0],h=sizes[i][1];CenterLayout((int)w,(int)h);
        assert(SDL_fabsf((layoutX+layoutW*.5f)-w*.5f)<.1f);
        assert(SDL_fabsf((layoutY+layoutH*.5f)-h*.5f)<.1f);
        assert(layoutX>=0 && layoutY>=0);
        point(1,(layoutX+layoutW*.91f)/w,(layoutY+layoutH*.62f)/h);
        assert(PortAndroidTouchKeys()==1);PortAndroidTouchClear();
        point(2,(layoutX+layoutW*.07f)/w,(layoutY+layoutH*.63f)/h);
        assert(PortAndroidTouchKeys()==(0x20|0x40));PortAndroidTouchClear();
    }
    puts("Web touch: centered geometry, A and diagonal hit testing at 5 aspect ratios PASS");return 0;
}
