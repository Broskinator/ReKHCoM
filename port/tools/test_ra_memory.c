#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "port/port.h"
#include "game_state.h"
GameState gGameState;
Deck gDecks[3];
uint16_t gCardCollection[999], gCardCount;
uint8_t gMapCardCounts[270];
uint8_t GetActiveDeckIndex(void) { return 0; }
uint16_t GetKeysHeld(void) { return 0; }
volatile uint32_t gVBlankCounter;
int32_t gPendingModeArg;

static int check(int ok, const char* label) {
    if (!ok) fprintf(stderr, "FAIL: %s\n", label);
    return ok;
}

int main(void) {
    uint8_t out[8];
    int ok = 1;
    void* ewram = VirtualAlloc((void*)(uintptr_t)GBA_EWRAM, GBA_EWRAM_SIZE,
                               MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    void* iwram = VirtualAlloc((void*)(uintptr_t)GBA_IWRAM, GBA_IWRAM_SIZE,
                               MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    void* sram = VirtualAlloc((void*)(uintptr_t)GBA_SRAM, GBA_SRAM_SIZE,
                              MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (ewram != (void*)(uintptr_t)GBA_EWRAM || iwram != (void*)(uintptr_t)GBA_IWRAM ||
        sram != (void*)(uintptr_t)GBA_SRAM) {
        fprintf(stderr, "Cannot reserve GBA test address windows\n");
        return 2;
    }

    memcpy((uint8_t*)ewram + GBA_EWRAM_SIZE - 4, "EWRM", 4);
    memcpy((uint8_t*)iwram, "IWRM", 4);
    ok &= check(PortRaReadMemory(0x47FFC, out, 4)
                && memcmp(out, "EWRM", 4) == 0, "EWRAM final bytes");
    ok &= check(PortRaReadMemory(0, out, 4)
                && memcmp(out, "IWRM", 4) == 0, "IWRAM initial bytes");
    memset(out, 0xA5, sizeof(out));
    memcpy((uint8_t*)iwram + GBA_IWRAM_SIZE - 2, "IW", 2);
    memcpy(ewram, "EW", 2);
    ok &= check(PortRaReadMemory(0x7FFE, out, 4) && memcmp(out, "IWEW", 4) == 0,
                "read across flattened IWRAM/EWRAM boundary");
    memcpy((uint8_t*)sram + GBA_SRAM_SIZE - 4, "SAVE", 4);
    ok &= check(PortRaReadMemory(0x57FFC, out, 4) && memcmp(out, "SAVE", 4) == 0,
                "SRAM final bytes");
    ok &= check(!PortRaReadMemory(0x57FFD, out, 4) && out[0] == 0 && out[3] == 0,
                "reject end overrun and clear output");
    memset(out, 0xA5, sizeof(out));
    ok &= check(!PortRaReadMemory(GBA_IO, out, 4)
                && out[0] == 0 && out[3] == 0, "reject MMIO");
    ok &= check(PortRaReadMemory(0x58000, out, 0), "empty read at mapped end");
    ok &= check(!PortRaReadMemory(UINT32_MAX - 2, out, 4), "reject integer wraparound");

    VirtualFree(iwram, 0, MEM_RELEASE);
    VirtualFree(ewram, 0, MEM_RELEASE);
    VirtualFree(sram, 0, MEM_RELEASE);
    if (!ok) return 1;
    puts("PASS: RetroAchievements GBA RAM adapter bounds");
    return 0;
}
