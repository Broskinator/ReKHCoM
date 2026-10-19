/* Production frame guard with a transport-free RA spy. No account or server. */
#define main ExistingLocalConformanceMain
#include "native_achievements_conformance.c"
#undef main
#include "../achievements/ra_client.h"
struct KhRa { int invalid; };
static struct KhRa fake;
static int frames, evaluations, invalidations;
KhRa* KhRaCreate(const char* path, KhRaRead read, void* data, KhRaTransport transport) {
    (void)path; (void)data; assert(read && !transport); return &fake;
}
void KhRaStartOfficial(KhRa* ra, const uint32_t* ids, uint32_t count) {
    assert(ra == &fake && ids && count > 0);
}
void KhRaSetRom(KhRa* ra, const uint8_t* rom, size_t size) { (void)ra; (void)rom; (void)size; }
void KhRaDestroy(KhRa* ra) { assert(ra == &fake); }
void KhRaInvalidate(KhRa* ra, const char* reason) { assert(ra == &fake && reason); ++invalidations; fake.invalid = 1; }
const char* KhRaTakeNotice(KhRa* ra) { (void)ra; return NULL; }
void KhRaPump(KhRa* ra, int evaluate) {
    assert(ra == &fake); ++frames;
    if (evaluate) {
        assert(!gPortConfig.cheatOneHit && !gPortConfig.cheatInfiniteHp &&
               !gPortConfig.cheatUnlockRiku && !gPortConfig.ps2Mode && !fake.invalid);
        ++evaluations;
    }
}
int PortRaReadMemory(uint32_t address, uint8_t* out, uint32_t size) {
    (void)address; memset(out,0,size); return 1;
}
int main(int argc, char** argv) {
    int which = argc > 1 ? atoi(argv[1]) : 0;
    assert(SDL_Init(0));
    PortAchievementsInit(); gCurrentMode = &gModeMapFld;
    gGameState.progression.level = 1; gGameState.progression.maxHp = 80;
    PortAchievementsFrame(); assert(frames == 1 && evaluations == 1 && invalidations == 0);
    switch (which) {
    case 0: gPortConfig.cheatOneHit = 1; break;
    case 1: gPortConfig.cheatInfiniteHp = 1; break;
    case 2: gPortConfig.cheatUnlockRiku = 1; break;
    case 3: gPortConfig.ps2Mode = 1; break;
    default: assert(0);
    }
    PortAchievementsFrame(); assert(frames == 2 && evaluations == 1 && invalidations == 1);
    gPortConfig.cheatOneHit = gPortConfig.cheatInfiniteHp = gPortConfig.cheatUnlockRiku = gPortConfig.ps2Mode = 0;
    PortAchievementsFrame(); assert(frames == 3 && evaluations == 1 && invalidations == 1);
    PortAchievementsShutdown(); SDL_Quit();
    puts("PASS official frame guard: clean frame evaluates; first modified frame and later frames stay blocked");
    return 0;
}
