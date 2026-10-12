/*
 * GBA address space for the native build.
 *
 * Every region is committed at its hardware address, so the game's register
 * macros, VRAM/OAM pointers and the pointers baked into ROM data are valid
 * as-is. This requires a 32-bit process with a fixed image base, which keeps
 * the 0x02000000-0x0E010000 range free when the program starts.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "port/port.h"
#include "port/rom_relocs.h"

/*
 * The GBA address range is part of the executable image: an uninitialised
 * section placed after the program's own data (".bss$z" sorts last) and large
 * enough to span 0x02000000-0x0E010000. Windows maps the image before any DLL,
 * heap or system view, so the range is guaranteed to be ours, already
 * committed and zero-filled.
 */
#define GBA_SPACE_END 0x0E010000u
#define GBA_SPACE_SIZE (GBA_SPACE_END - 0x00400000u)

__attribute__((section(".bss$zgba"), used, aligned(4096))) static uint8_t sGbaSpace[GBA_SPACE_SIZE];

static uint8_t sSramShadow[GBA_SRAM_SIZE];
static int sSramLoaded;

int PortMemoryInit(void) {
    uint32_t start = (uint32_t)(uintptr_t)sGbaSpace;
    uint32_t end = start + GBA_SPACE_SIZE;

    if (start > GBA_EWRAM || end < GBA_SPACE_END) {
        PortFatal("The executable's GBA memory section is at 0x%08X-0x%08X and does not cover "
                  "0x%08X-0x%08X. The program was linked incorrectly.",
                  start, end, GBA_EWRAM, GBA_SPACE_END);
        return 0;
    }
    return 1;
}

static int TryRom(const char* path) {
    FILE* f = fopen(path, "rb");
    uint8_t header[0xC0];
    long size;

    if (f == NULL)
        return 0;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size != (long)GBA_ROM_SIZE || fread(header, 1, sizeof(header), f) != sizeof(header)
        || memcmp(header + 0xAC, "B8CP", 4) != 0) {
        fclose(f);
        return 0;
    }
    fseek(f, 0, SEEK_SET);
    if (fread((void*)(uintptr_t)GBA_ROM, 1, GBA_ROM_SIZE, f) != GBA_ROM_SIZE) {
        fclose(f);
        return 0;
    }
    fclose(f);
    PortLog("ROM: %s", path);
    return 1;
}

static int FindRom(void) {
    WIN32_FIND_DATAA fd;
    HANDLE h;
    char path[MAX_PATH];
    static const char* const names[] = { "baserom.gba", "roms/B8CP.gba", "kh_com_eu.gba",
                                                 "../../roms/B8CP.gba" /* development tree */ };
    size_t i;

    for (i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        if (TryRom(PortConfigPath(names[i])))
            return 1;
    }
    /* Any dump of the European cartridge next to the executable. */
    h = FindFirstFileA(PortConfigPath("*.gba"), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            snprintf(path, sizeof(path), "%s", PortConfigPath(fd.cFileName));
            if (TryRom(path)) {
                FindClose(h);
                return 1;
            }
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
    return 0;
}

int PortRomLoad(void) {
    const PortRomReloc* r;

    if (!FindRom()) {
        PortFatal("Kingdom Hearts: Chain of Memories (Europe) ROM not found.\n\n"
                  "Place your cartridge dump (B8CP, 32 MiB) next to the executable "
                  "as \"baserom.gba\".");
        return 0;
    }
    /* ROM words that referenced code or data now compiled natively. */
    for (r = gPortRomRelocs; r->romAddress != 0; r++)
        *(uint32_t*)(uintptr_t)r->romAddress = (uint32_t)(uintptr_t)r->target;
    return 1;
}

void PortSramLoad(void) {
    FILE* f = fopen(PortConfigPath("kh_com.sav"), "rb");
    uint8_t* sram = (uint8_t*)(uintptr_t)GBA_SRAM;

    memset(sram, 0xFF, GBA_SRAM_SIZE);
    if (f != NULL) {
        size_t n = fread(sram, 1, GBA_SRAM_SIZE, f);
        fclose(f);
        PortLog("Save: loaded %u bytes", (unsigned)n);
    }
    memcpy(sSramShadow, sram, GBA_SRAM_SIZE);
    sSramLoaded = 1;
}

void PortSramSyncIfDirty(void) {
    const uint8_t* sram = (const uint8_t*)(uintptr_t)GBA_SRAM;
    char tmp[MAX_PATH];
    FILE* f;

    if (!sSramLoaded || memcmp(sSramShadow, sram, GBA_SRAM_SIZE) == 0)
        return;
    snprintf(tmp, sizeof(tmp), "%s.tmp", PortConfigPath("kh_com.sav"));
    f = fopen(tmp, "wb");
    if (f == NULL)
        return;
    if (fwrite(sram, 1, GBA_SRAM_SIZE, f) == GBA_SRAM_SIZE && fclose(f) == 0) {
        if (MoveFileExA(tmp, PortConfigPath("kh_com.sav"), MOVEFILE_REPLACE_EXISTING))
            memcpy(sSramShadow, sram, GBA_SRAM_SIZE);
    } else {
        fclose(f);
    }
}
