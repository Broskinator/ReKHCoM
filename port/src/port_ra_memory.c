#include <string.h>

#include "port/port.h"
void PortRaOverlayState(uint32_t address, uint8_t* buffer, uint32_t size);

/* rcheevos uses a flattened GBA memory map, not hardware bus addresses:
 * 00000-07fff IWRAM, 08000-47fff EWRAM, 48000-57fff SRAM.
 * Scalar GameState spans are projected by port_ra_state.c. Battle globals,
 * native pointer chains and the full official set still require validation. */
int PortRaReadMemory(uint32_t address, uint8_t* buffer, uint32_t size) {
    uint32_t remaining = size;

    if (buffer == NULL)
        return 0;

    if (address > 0x58000 || size > 0x58000 - address) {
        memset(buffer, 0, size);
        return 0;
    }
    while (remaining) {
        uint32_t base, offset, available, count;
        if (address < 0x8000) { base = GBA_IWRAM; offset = address; available = 0x8000 - address; }
        else if (address < 0x48000) { base = GBA_EWRAM; offset = address - 0x8000; available = 0x48000 - address; }
        else { base = GBA_SRAM; offset = address - 0x48000; available = 0x58000 - address; }
        count = remaining < available ? remaining : available;
        memcpy(buffer, (const void*)(uintptr_t)(base + offset), count);
        PortRaOverlayState(address, buffer, count);
        buffer += count; address += count; remaining -= count;
    }
    return 1;
}
