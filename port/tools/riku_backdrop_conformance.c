/* Native PPU properties: one panel, palette fidelity, core pixels, fades. */
#include <assert.h>
#define main PpuFixtureMain
#include "ppu_conformance.c"
#undef main
int main(void) {
    char* args[]={"panel","backdrop","base.bin"};
    uint16_t *map, *hardware; uint32_t core[240*160];
    int i,x,y,c,ox,oy;
    assert(VirtualAlloc((void*)GBA_EWRAM,GBA_EWRAM_SIZE,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));
    assert(PpuFixtureMain(3,args)==0);
    map=(uint16_t*)(GBA_EWRAM+0x100);
    hardware=(uint16_t*)(GBA_VRAM+0x8000);
    for(i=0;i<4;i++)((uint32_t*)GBA_EWRAM)[i]=(uint32_t)(uintptr_t)(map+i*1024);
    for(c=1;c<16;c++) {
        memset((void*)(GBA_VRAM+c*32),c*17,32);
        ((uint16_t*)GBA_PLTT)[16+c]=(31-c+1)*0x421;
    }
    ((uint16_t*)GBA_PLTT)[1]=31; /* sentinel: other panel must not leak red */
    for(y=0;y<32;y++)for(x=0;x<32;x++) {
        int dx=x-16,dy=y-16,shade=15-(dx*dx+dy*dy*2)/14;
        if(shade<1)shade=1;
        hardware[y*32+x]=map[y*32+x]=0x1000|shade;
        map[1024+y*32+x]=1;
        map[2048+y*32+x]=map[3072+y*32+x]=0;
    }
    memset(&sBgLatch[3],0,sizeof(sBgLatch[3]));
    sBgLatch[3].map=(void* const*)GBA_EWRAM;
    sBgLatch[3].width=sBgLatch[3].height=2;
    sBgLatch[3].x=sBgLatch[3].hofs=8;
    sBgLatch[3].y=sBgLatch[3].vofs=81;
    IO16(0)=0x800;IO16(14)=16<<8;IO16(0x1c)=8;IO16(0x1e)=81;
    PortPpuSetResolution(512,288);ox=gPortScreen.originX;oy=gPortScreen.originY;
    sFixtureBackdropPanel=-1;PortPpuRenderFrame();
    for(y=0;y<160;y++)memcpy(core+y*240,gPortFrame+(oy+y)*512+ox,240*4);
    sFixtureBackdropPanel=3;PortPpuRenderFrame();assert(gPortScreen.wide);
    for(y=0;y<160;y++)assert(!memcmp(core+y*240,gPortFrame+(oy+y)*512+ox,240*4));
    for(i=0;i<512*288;i++) {
        int found=0;
        for(c=17;c<32;c++)found|=gPortFrame[i]==sColorLut[((uint16_t*)GBA_PLTT)[c]];
        assert(found); /* no invented colours, black holes or second panel */
    }
    assert(gPortFrame[0]==0xffffffffu && gPortFrame[512*288-1]==0xffffffffu);
    /* Added arcs fade outwards without a reflected second centre. */
    y=oy+128-81;
    for(i=0;i<2;i++) {
        int previous=0;
        for(x=0;x<64;x++) {
            int wx=i?256+x:-1-x,px=ox+wx-8;
            int shade=(gPortFrame[y*512+px]>>16)&255;
            assert(shade>=previous);previous=shade;
        }
    }
    ((uint16_t*)GBA_PLTT)[17]=0x7c00;PortPpuRenderFrame();
    assert(gPortFrame[0]==sColorLut[0x7c00]);
    IO16(0x50)=0xc8;IO16(0x54)=16;PortPpuRenderFrame();
    assert(gPortFrame[0]==0xff000000u);
    puts("Riku backdrop: 38400 native pixels exact; 147456 palette-only pixels; single centre, palette changes and fade PASS");
    return 0;
}
