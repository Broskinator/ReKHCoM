#include <string.h>

#include "port/port.h"

/* rcheevos sets address the original GBA RAM bus. The native port already
 * maps these windows at their hardware addresses; keep the adapter deliberately
 * narrow so reads cannot wander into game code, save RAM, or MMIO. */
int PortRaReadMemory(uint32_t address, uint8_t* buffer, uint32_t size) {
    uint32_t offset;

    if (buffer == NULL)
        return 0;

    if (address >= GBA_EWRAM && address - GBA_EWRAM <= GBA_EWRAM_SIZE
        && size <= GBA_EWRAM_SIZE - (address - GBA_EWRAM)) {
        offset = address - GBA_EWRAM;
        memcpy(buffer, (const void*)(uintptr_t)(GBA_EWRAM + offset), size);
        return 1;
    }

    if (address >= GBA_IWRAM && address - GBA_IWRAM <= GBA_IWRAM_SIZE
        && size <= GBA_IWRAM_SIZE - (address - GBA_IWRAM)) {
        offset = address - GBA_IWRAM;
        memcpy(buffer, (const void*)(uintptr_t)(GBA_IWRAM + offset), size);
        return 1;
    }

    memset(buffer, 0, size);
    return 0;
}
