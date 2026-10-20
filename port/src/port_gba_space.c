/*
 * The relocatable GBA address space (PORT_GBA_RELOC, see include/gba/gba_space.h): the regions are arrays of the
 * program (port_gba_space.s, rom_symbols_reloc.s) and this file translates bus addresses into them and moves the
 * pointers stored inside the ROM once it is loaded. Without the flag the regions are at their bus addresses and
 * nothing here is built.
 */
#ifdef PORT_GBA_RELOC
#include <stdint.h>
#include <string.h>

#include "port/port.h"

extern uint8_t gPortBiosArea[0x10000];

/* The bus address `address` decodes to, as the GBA does it: mirrors folded, BIOS and open bus to the stand-in. */
void* GbaSpacePtr(unsigned int address) {
    uint32_t o;

    switch ((address >> 24) & 0xF) {
        case 0x2: return gGbaEwram + (address & 0x3FFFFu);
        case 0x3: return gGbaIwram + (address & 0x7FFFu);
        case 0x4: return (address & 0xFFFFFFu) < 0x400u ? gGbaIo + (address & 0x3FFu) : NULL;
        case 0x5: return gGbaPltt + (address & 0x3FFu);
        case 0x6:
            o = address & 0x1FFFFu;
            return gGbaVram + (o >= 0x18000u ? o - 0x8000u : o);
        case 0x7: return gGbaOam + (address & 0x3FFu);
        case 0x8: case 0x9: case 0xA: case 0xB: case 0xC: case 0xD: return gGbaRom + (address & 0x1FFFFFFu);
        case 0xE: case 0xF: return gGbaSram + (address & 0xFFFFu);
        default: return NULL;
    }
}

/* A program address back to its bus address (0 outside the regions): for what reports GBA addresses (logs,
   achievements). */
unsigned int GbaSpaceBusAddress(const void* p) {
    uintptr_t a = (uintptr_t)p;
    static const struct { uint8_t* base; uint32_t size, bus; } regions[] = {
        { gGbaEwram, GBA_EWRAM_SIZE, 0x02000000u }, { gGbaIwram, GBA_IWRAM_SIZE, 0x03000000u },
        { gGbaIo, GBA_IO_SIZE, 0x04000000u },       { gGbaPltt, GBA_PLTT_SIZE, 0x05000000u },
        { gGbaVram, GBA_VRAM_SIZE, 0x06000000u },   { gGbaOam, GBA_OAM_SIZE, 0x07000000u },
        { gGbaRom, GBA_ROM_SIZE, 0x08000000u },     { gGbaSram, GBA_SRAM_SIZE, 0x0E000000u },
    };
    size_t i;
    for (i = 0; i < sizeof(regions) / sizeof(regions[0]); i++)
        if (a >= (uintptr_t)regions[i].base && a < (uintptr_t)regions[i].base + regions[i].size)
            return regions[i].bus + (uint32_t)(a - (uintptr_t)regions[i].base);
    return 0;
}

/* Every word of the ROM that holds a pointer (the GBA build's ABS32 relocations, port/data/rom_pointers_*.inc)
   now points into the arrays. Run once, right after the ROM is read and before the native relocations. */
static const uint32_t kRomPointers[] = {
#if defined(VERSION_EU)
#include "../data/rom_pointers_eu.inc"
#elif defined(VERSION_US)
#include "../data/rom_pointers_us.inc"
#else
#include "../data/rom_pointers_jp.inc"
#endif
};

int GbaSpaceMoveRomPointers(void) {
    size_t i;
    int moved = 0;
    for (i = 0; i < sizeof(kRomPointers) / sizeof(kRomPointers[0]); i++) {
        uint32_t* word = (uint32_t*)(gGbaRom + kRomPointers[i]);
        uint32_t value;
        memcpy(&value, word, 4);
        if (value >= 0x02000000u && value < 0x0F000000u) {
            void* p = GbaSpacePtr(value);
            if (p) {
                value = (uint32_t)(uintptr_t)p;
                memcpy(word, &value, 4);
                moved++;
            }
        }
    }
    return moved;
}
#endif
