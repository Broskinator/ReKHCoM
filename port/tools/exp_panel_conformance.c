/* ROM-free regression: replacing digit ink must refill its opaque panel. */
#define main PpuFixtureMain
#include "ppu_conformance.c"
#undef main

int main(void) {
    char* args[] = {"exp_panel", "backdrop", "base.bin"};
    uint16_t* p; uint16_t* o; uint8_t* tiles;
    uint32_t hud[240*160];
    int hd, effect, x, y, failures = 0, checks = 0;
    if (PpuFixtureMain(3,args)) return 2;
    p=(uint16_t*)(uintptr_t)GBA_PLTT; o=(uint16_t*)(uintptr_t)GBA_OAM;
    tiles=(uint8_t*)(uintptr_t)(GBA_VRAM+0x10000);
    p[0]=0x03e0; p[256+13]=0x4000; p[256+14]=0; p[256+15]=0x7fff;
    for (y=0;y<8;y++) for (x=0;x<8;x++) {
        int c = x==0 && y==0 ? 0 : x>=2 && x<6 && y>=2 && y<6 ? ((x+y)&1 ? 15 : 14) : 13;
        int off=y*4+x/2;
        tiles[off]=(tiles[off]&~(15<<((x&1)*4)))|(c<<((x&1)*4));
    }
    IO16(0)=0x1040; o[0]=o[1]=o[2]=0;
    PortOamSetText(0,'8',2);
    for (hd=0;hd<2;hd++) for (effect=0;effect<3;effect++) {
        gPortConfig.hdText=hd;
        IO16(0x50)=effect==0 ? 0 : 0x10 | ((effect+1)<<6);
        IO16(0x54)=5;
        sOamTag[0].hud=0;
        PortPpuRenderFrame();
        gPortHudScale=1; gPortHudW=240; gPortHudH=160; sHudLayer=1;
        sOamTag[0].hud=1;sOamTag[0].ax=-1;sOamTag[0].ay=-1;
        PortPpuRenderHud(hud);
        for (y=0;y<8;y++) for (x=0;x<8;x++) {
            int corner=x==0&&y==0;
            int ci=!hd && x>=2&&x<6&&y>=2&&y<6 ? ((x+y)&1 ? 15 : 14) : 13;
            uint16_t col=p[256+ci];
            if (effect==1) col=Brighten(col,5);
            if (effect==2) col=Darken(col,5);
            uint32_t want=sColorLut[col];
            failures += gPortFrame[y*240+x] != (corner?sColorLut[p[0]]:want);
            failures += hud[y*240+x] != (corner?0:want);
            checks+=2;
        }
    }
    printf("EXP panel: %d pixel assertions, %d failures (world/HUD, HD on/off, fades)\n",checks,failures);
    return failures != 0;
}
