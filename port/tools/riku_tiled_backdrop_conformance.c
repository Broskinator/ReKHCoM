/* Real renderer, synthetic ROM-free opaque scenery with empty author margins. */
#include <assert.h>
#define main PpuFixtureMain
#include "ppu_conformance.c"
#undef main
int main(void) {
    char* args[]={"tiles","backdrop","base.bin"};
    uint16_t* blocks; uint32_t core[240*160]; int x,y,i,ox,oy;
    assert(VirtualAlloc((void*)GBA_EWRAM,GBA_EWRAM_SIZE,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));
    assert(PpuFixtureMain(3,args)==0);
    blocks=(uint16_t*)(GBA_EWRAM+0x100);
    for(i=0;i<4;i++)((uint32_t*)GBA_EWRAM)[i]=(uint32_t)(uintptr_t)(blocks+1024*i);
    for(y=0;y<64;y++)for(x=0;x<64;x++)
        blocks[(y/32*2+x/32)*1024+(y%32)*32+x%32]=(x<56 && y>=13 && y<54)?1:0;
    memset((void*)GBA_VRAM,0,64);
    for(i=0;i<32;i++)((uint8_t*)GBA_VRAM)[32+i]=(i&1)?0x12:0x21;
    for(i=0;i<1024;i++)((uint16_t*)(GBA_VRAM+0x8000))[i]=1;
    ((uint16_t*)GBA_PLTT)[0]=0x7fff;
    ((uint16_t*)GBA_PLTT)[1]=0x1357;((uint16_t*)GBA_PLTT)[2]=0x2468;
    IO16(0)=0x800;IO16(14)=16<<8;
    IO16(0x1c)=97;IO16(0x1e)=214&255;
    memset(sBgLatch,0,sizeof(sBgLatch));
    sBgLatch[3].map=(void* const*)GBA_EWRAM;
    sBgLatch[3].width=sBgLatch[3].height=2;
    sBgLatch[3].x=97;sBgLatch[3].y=214;
    sBgLatch[3].hofs=97;sBgLatch[3].vofs=214&255;
    PortPpuSetResolution(480,270);ox=gPortScreen.originX;oy=gPortScreen.originY;
    sFixtureTiledBackdrop=-1;PortPpuRenderFrame();
    for(y=0;y<160;y++)memcpy(core+y*240,gPortFrame+(oy+y)*480+ox,240*4);
    sFixtureTiledBackdrop=3;PortPpuRenderFrame();assert(sTiledBackdropBg==3);
    for(y=0;y<160;y++)assert(!memcmp(core+y*240,gPortFrame+(oy+y)*480+ox,240*4));
    for(i=0;i<480*270;i++)assert(gPortFrame[i]==sColorLut[0x1357] || gPortFrame[i]==sColorLut[0x2468]);
    /* x1-size view: no holes, duplicate foreground or interpolated colours. */
    PortPpuSetResolution(1920,1080);PortPpuRenderFrame();
    for(i=0;i<1920*1080;i++)assert(gPortFrame[i]==sColorLut[0x1357] || gPortFrame[i]==sColorLut[0x2468]);
    IO16(0x50)=(3<<6)|(1<<3);IO16(0x54)=16;PortPpuRenderFrame();
    for(i=0;i<1920*1080;i++)assert(gPortFrame[i]==0xff000000u);
    IO16(0x50)=IO16(0x54)=0;
    /* A partial tile means this is scenery geometry, not a flat base plane. */
    ((uint8_t*)GBA_VRAM)[32]=0x10;PortPpuRenderFrame();assert(sTiledBackdropBg==-1);
    puts("Riku tiled scenery: 38400 native pixels exact; 480x270 and 1920x1080 palette-only; fade and partial-tile rejection PASS");
    return 0;
}
