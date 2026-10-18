#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "port/port.h"

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
    if (ewram != (void*)(uintptr_t)GBA_EWRAM || iwram != (void*)(uintptr_t)GBA_IWRAM) {
        fprintf(stderr, "Cannot reserve GBA test address windows\n");
        return 2;
    }

    memcpy((uint8_t*)ewram + GBA_EWRAM_SIZE - 4, "EWRM", 4);
    memcpy((uint8_t*)iwram, "IWRM", 4);
    ok &= check(PortRaReadMemory(GBA_EWRAM + GBA_EWRAM_SIZE - 4, out, 4)
                && memcmp(out, "EWRM", 4) == 0, "EWRAM final bytes");
    ok &= check(PortRaReadMemory(GBA_IWRAM, out, 4)
                && memcmp(out, "IWRM", 4) == 0, "IWRAM initial bytes");
    memset(out, 0xA5, sizeof(out));
    ok &= check(!PortRaReadMemory(GBA_EWRAM + GBA_EWRAM_SIZE - 3, out, 4)
                && out[0] == 0 && out[3] == 0, "reject EWRAM overrun and clear output");
    memset(out, 0xA5, sizeof(out));
    ok &= check(!PortRaReadMemory(GBA_IO, out, 4)
                && out[0] == 0 && out[3] == 0, "reject MMIO");
    ok &= check(PortRaReadMemory(GBA_EWRAM + GBA_EWRAM_SIZE, out, 0),
                "allow empty read at EWRAM end");

    VirtualFree(iwram, 0, MEM_RELEASE);
    VirtualFree(ewram, 0, MEM_RELEASE);
    if (!ok) return 1;
    puts("PASS: RetroAchievements GBA RAM adapter bounds");
    return 0;
}
