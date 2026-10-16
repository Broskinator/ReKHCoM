/*
 * GBA address space for the PS5 build.
 *
 * The host maps every region at its hardware address before the guest starts
 * (host/ps5_host.c), so the game's register macros, VRAM/OAM pointers and the
 * pointers baked into ROM data are valid as-is.
 */
#include <stdio.h>
#include <string.h>

#include "port/port.h"
#include "port/rom_relocs.h"
#include "bridge_calls.h"

#define MAX_PATH 1024

/* Stand-in for the BIOS area behind BIOS_PTR(): reads see the open-bus value
   left by the BIOS (the opcode after the SWI return), writes are undone every
   frame. */
__attribute__((aligned(16))) uint8_t gPortBiosArea[0x10000];

void PortBiosAreaRefresh(void) {
    uint32_t* p = (uint32_t*)gPortBiosArea;
    size_t i;

    for (i = 0; i < sizeof(gPortBiosArea) / 4; i++)
        p[i] = 0xE3A02004;
}

static uint8_t sSramShadow[GBA_SRAM_SIZE];
static int sSramLoaded;

int PortMemoryInit(void) {
    /* The host maps the regions; a quick write test proves they are there. */
    static const uint32_t bases[] = { GBA_EWRAM, GBA_IWRAM, GBA_PLTT, GBA_VRAM, GBA_OAM, GBA_ROM, GBA_SRAM };
    size_t i;

    for (i = 0; i < sizeof(bases) / sizeof(bases[0]); i++) {
        volatile uint8_t* p = (volatile uint8_t*)(uintptr_t)bases[i];
        uint8_t keep = p[0];
        p[0] = (uint8_t)(keep ^ 0xA5);
        if (p[0] != (uint8_t)(keep ^ 0xA5)) {
            PortFatal("GBA region %08X is not mapped", bases[i]);
            return 0;
        }
        p[0] = keep;
    }
    return 1;
}

const char* gPortRomPath;

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
        || memcmp(header + 0xAC, PORT_ROM_CODE, 4) != 0) {
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
    static const char* const names[] = { PORT_ROM_CODE ".gba", "baserom.gba", "roms/" PORT_ROM_CODE ".gba",
                                         "../../roms/" PORT_ROM_CODE ".gba" /* development tree */ };
    size_t i;

    if (gPortRomPath != NULL && TryRom(gPortRomPath))
        return 1;
    for (i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        if (TryRom(PortConfigPath(names[i])))
            return 1;
    }
    /* Any dump of this cartridge version in the data folder. */
    {
        int d = h_dir_open(PortConfigPath(""));
        char name[256], path[MAX_PATH];
        size_t n;

        if (d >= 0) {
            while (h_dir_next(d, name, sizeof(name))) {
                n = strlen(name);
                if (n > 4 && strcasecmp(name + n - 4, ".gba") == 0) {
                    snprintf(path, sizeof(path), "%s", PortConfigPath(name));
                    if (TryRom(path)) {
                        h_dir_close(d);
                        return 1;
                    }
                }
            }
            h_dir_close(d);
        }
    }
    return 0;
}

int PortRomLoad(void) {
    const PortRomReloc* r;

    if (!FindRom()) {
        PortFatal("Kingdom Hearts: Chain of Memories (" PORT_ROM_REGION ") ROM not found.\n\n"
                  "Copy your cartridge dump (" PORT_ROM_CODE ", 32 MiB) to /data/ReKHCom/ "
                  "(any name ending in .gba).");
        return 0;
    }
    /* ROM words that referenced code or data now compiled natively. */
    for (r = gPortRomRelocs; r->romAddress != 0; r++)
        *(uint32_t*)(uintptr_t)r->romAddress = (uint32_t)(uintptr_t)r->target;
    return 1;
}

/* The bank the SRAM was loaded from (gPortConfig.saveBank can change first). */
static int sSramBank = 1;

static const char* BankName(int bank) {
    static char name[64];
    size_t len = strlen(PORT_SAVE_NAME) - 4; /* without ".sav" */

    if (bank <= 1)
        return PORT_SAVE_NAME;
    snprintf(name, sizeof(name), "%.*s_bank%d.sav", (int)len, PORT_SAVE_NAME, bank);
    return name;
}

const char* PortSaveName(void) {
    return BankName(sSramBank);
}

void PortSramSwitchBank(void) {
    if (gPortConfig.saveBank == sSramBank)
        return;
    PortSramSyncIfDirty();
    PortSramLoad();
}

void PortSramLoad(void) {
    FILE* f;
    sSramBank = gPortConfig.saveBank < 1 ? 1 : gPortConfig.saveBank > PORT_SAVE_BANKS ? PORT_SAVE_BANKS : gPortConfig.saveBank;
    f = fopen(PortConfigPath(BankName(sSramBank)), "rb");
    uint8_t* sram = (uint8_t*)(uintptr_t)GBA_SRAM;

    memset(sram, 0xFF, GBA_SRAM_SIZE);
    if (f != NULL) {
        size_t n = fread(sram, 1, GBA_SRAM_SIZE, f);
        fclose(f);
        PortLog("Save: loaded %u bytes from %s", (unsigned)n, BankName(sSramBank));
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
    snprintf(tmp, sizeof(tmp), "%s.tmp", PortConfigPath(BankName(sSramBank)));
    f = fopen(tmp, "wb");
    if (f == NULL)
        return;
    /* Always close exactly once, including failed writes / failed close. */
    int written = fwrite(sram, 1, GBA_SRAM_SIZE, f) == GBA_SRAM_SIZE;
    int closed = fclose(f) == 0;
    if (written && closed) {
        if (rename(tmp, PortConfigPath(BankName(sSramBank))) == 0)
            memcpy(sSramShadow, sram, GBA_SRAM_SIZE);
    }
}
