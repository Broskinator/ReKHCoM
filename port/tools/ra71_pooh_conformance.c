/* Actual downloaded condition supplied privately at runtime; no network or account. */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define main ExistingProjectionTests
#include "test_ra_state.c"
#undef main
#include "../third_party/rcheevos/include/rc_runtime_types.h"
#include <stdlib.h>
static unsigned char original[0x58000];
int PortRaReadMemory(uint32_t, uint8_t*, uint32_t);
static uint32_t Peek(uint32_t address, uint32_t size, void* ud) {
    unsigned char b[4]; uint32_t value=0; unsigned i;
    assert(size <= 4 && address + size <= sizeof(original));
    if (ud) memcpy(b,original+address,size);
    else assert(PortRaReadMemory(address,b,size));
    for(i=0;i<size;i++) value |= (uint32_t)b[i] << (i*8);
    return value;
}
int main(int argc, char** argv) {
    FILE* f; char definition[8192]; int size,scenario,step,successes[3]={0};
    assert(argc==2);f=fopen(argv[1],"rb");assert(f);
    size=(int)fread(definition,1,sizeof(definition)-1,f);definition[size]=0;fclose(f);
    size=rc_trigger_size(definition);assert(size>0);
    assert(VirtualAlloc((void*)0x02000000,0x40000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE)==(void*)0x02000000);
    assert(VirtualAlloc((void*)0x03000000,0x8000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE)==(void*)0x03000000);
    for(scenario=0;scenario<3;scenario++) {
        void* a=malloc(size);void* b=malloc(size);
        rc_trigger_t* native=rc_parse_trigger(a,definition,NULL,0);
        rc_trigger_t* ref=rc_parse_trigger(b,definition,NULL,0);
        assert(native&&ref);
        for(step=0;step<100;step++) {
            unsigned char* state=(unsigned char*)&gGameState;
            int ns,rs;
            memset(&gGameState,0,sizeof(gGameState));
            gGameState.world=step<10||step>=80?0:13;
            state[0x186]=10;state[0x146]=step>=40?2:0;
            gPoohGauge=3;
            if(scenario==1&&step>=20) gPoohGauge=0;
            if(scenario==2&&step>=10) gPoohGauge=(step==20||step>=22)?2:1;
            memcpy(original+0x421c0,&gGameState,sizeof(gGameState));
            original[0x449d4]=(unsigned char)gPoohGauge;
            original[0x449d5]=(unsigned char)(gPoohGauge>>8);
            ns=rc_evaluate_trigger(native,Peek,NULL,NULL);
            rs=rc_evaluate_trigger(ref,Peek,(void*)1,NULL);
            assert(ns==rs);
            if(ns==RC_TRIGGER_STATE_TRIGGERED) ++successes[scenario];
        }
        printf("scenario %d: matches real condition graph, triggers=%d\n",scenario,successes[scenario]);
        free(a);free(b);
    }
    assert(successes[0] == 1 && successes[1] == 0 && successes[2] == 0);
    puts("PASS native projected RAM matches the official Pooh graph: completion, empty gauge, two refills");
    return 0;
}
