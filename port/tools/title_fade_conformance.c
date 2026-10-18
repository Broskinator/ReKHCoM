/* Real PPU functions, synthetic live palette; no cartridge required. */
#define main fixture_main_unused
#include "ppu_conformance.c"
#undef main
#include <assert.h>
int main(void) {
    uint16_t* p; int i,x,y,changed=0;
    assert(VirtualAlloc((void*)GBA_IO,0x10000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));
    assert(VirtualAlloc((void*)GBA_PLTT,0x10000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));
    PortCanvasAlloc();PortPpuSetResolution(240,160);p=(uint16_t*)(uintptr_t)GBA_PLTT;
    for(i=0;i<256;i++)p[i]=i<32?(uint16_t)(i|(i<<5)|(i<<10)):0;
    PortTitleArtFade(1);PortPpuBeginFrame();
    for(y=0;y<160;y++) {
        for(x=0;x<240;x++){sBgLine[1][x]=p[4];sBgLine[0][x]=p[8];sObjLine[x]=p[16];}
        sBgLine[1][239]=TRANSPARENT;TitleArtFadeLine(y);
        assert(sBgLine[1][239]==TRANSPARENT);
        for(x=0;x<239;x++) {
            int c=sBgLine[1][x];
            assert(sBgLine[0][x]==p[8] && sObjLine[x]==p[16]);
            if(x<234 && y<154)assert(c==p[4]);
            else changed+=c!=p[4];
            for(i=0;i<256 && p[i]!=c;i++){}assert(i<256);
            if(y==159)assert(c==p[31]);
        }
    }
    assert(changed>0);
    /* A live fade to black invalidates every cached white result. */
    for(i=0;i<256;i++)p[i]=0;PortPpuBeginFrame();
    for(x=0;x<240;x++)sBgLine[1][x]=0;
    TitleArtFadeLine(159);for(x=0;x<240;x++)assert(sBgLine[1][x]==0);
    /* Exiting title cannot affect a following menu or battle. */
    PortTitleArtFade(0);for(x=0;x<240;x++)sBgLine[1][x]=123;
    TitleArtFadeLine(159);for(x=0;x<240;x++)assert(sBgLine[1][x]==123);
    puts("PASS: native grid, palette, protected area, transparency, UI layers, live fade, mode exit");
    return 0;
}
