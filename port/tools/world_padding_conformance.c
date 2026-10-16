/* Real PPU fixture for map exterior padding. Optional private state replay
 * loads drawing memory only; it never restores game code or player saves. */
#define main PpuFixtureMain
#include "ppu_conformance.c"
#undef main
static FILE* snapshot;
static void read_state(void* p, size_t n) { if (fread(p,1,n,snapshot)!=n) abort(); }
int main(int argc,char** argv) {
    char* base[] = {"padding", "backdrop", "base.bin"};
    uint16_t* map; int i, pattern, failures=0;
    if (!VirtualAlloc((void*)GBA_EWRAM,GBA_EWRAM_SIZE,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE) ||
        !VirtualAlloc((void*)GBA_IWRAM,GBA_IWRAM_SIZE,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE) ||
        !VirtualAlloc((void*)GBA_ROM,GBA_ROM_SIZE,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE)) return 3;
    if(PpuFixtureMain(3,base)) return 4;
    if(argc==6) {
        FILE* rom=fopen(argv[2],"rb"); uint32_t header[3];
        static const uint32_t addresses[]={GBA_EWRAM,GBA_IWRAM,GBA_IO,GBA_PLTT,GBA_VRAM,GBA_OAM};
        static const size_t sizes[]={GBA_EWRAM_SIZE,GBA_IWRAM_SIZE,GBA_IO_SIZE,GBA_PLTT_SIZE,GBA_VRAM_SIZE,GBA_OAM_SIZE};
        if(!rom) return 5;fread((void*)GBA_ROM,1,GBA_ROM_SIZE,rom);fclose(rom);
        snapshot=fopen(argv[1],"rb");if(!snapshot)return 5;
        read_state(header,sizeof(header));if(header[0]!=0x5453484B || header[1]!=1 || header[2]!=0xA6CDD77B)return 6;
        for(i=0;i<6;i++)read_state((void*)(uintptr_t)addresses[i],sizes[i]);
        /* Recorded v1.7.1 EU layout. Only drawing state is replayed. */
        fseek(snapshot,210116+50282+96,SEEK_CUR);PortPpuVisitState(read_state);fclose(snapshot);
        for(i=0;i<4;i++)if((uintptr_t)sBgLatch[i].map<GBA_EWRAM || (uintptr_t)sBgLatch[i].map>=0x0A000000u)sBgLatch[i].map=NULL;
        memset(sBgWorld,0,sizeof(sBgWorld));memset(sBgWorldLatched,0,sizeof(sBgWorldLatched));
        memset(&sMenuShown,0,sizeof(sMenuShown));memset(&sMenuPend,0,sizeof(sMenuPend));
        sMenuShown.tabBg[0]=sMenuShown.tabBg[1]=-1;
        for(i=0;i<128;i++)((uint16_t*)GBA_OAM)[i*4]=0x200;
        PortPpuSetResolution(atoi(argv[4]),atoi(argv[5]));PortPpuRenderFrame();
        FILE* f=fopen(argv[3],"wb");if(!f)return 5;
        fwrite(gPortFrame,4,gPortScreen.width*gPortScreen.height,f);fclose(f);return 0;
    }
    map=(uint16_t*)(GBA_EWRAM+0x100);
    *(uint32_t*)GBA_EWRAM=(uint32_t)(uintptr_t)map;
    for(i=0;i<1024;i++)map[i]=((uint16_t*)(GBA_VRAM+0x8000))[i]=1;
    IO16(0)=0x100;IO16(8)=16<<8;IO16(0x10)=IO16(0x12)=0;
    ((uint16_t*)GBA_PLTT)[13]=0x43;((uint16_t*)GBA_PLTT)[14]=0x7FFF;
    memset(&sBgLatch[0],0,sizeof(sBgLatch[0]));
    sBgLatch[0].map=(void* const*)GBA_EWRAM;sBgLatch[0].width=sBgLatch[0].height=1;
    PortPpuSetResolution(480,270);
    for(pattern=0;pattern<3;pattern++) {
        memset((void*)(GBA_VRAM+32),pattern==2?0:0xDD,32);
        if(pattern==1)((uint8_t*)GBA_VRAM)[32]=0xED; /* nonuniform artwork is never extended */
        PortPpuRenderFrame();
        uint32_t want=pattern==0?sColorLut[0x43]:0xFF000000u;
        for(i=0;i<gPortScreen.height;i++) failures+=gPortFrame[i*480]!=want;
    }
    printf("World padding: 810 exterior pixels, %d failures (uniform, patterned, transparent corners)\n",failures);
    return failures!=0;
}
