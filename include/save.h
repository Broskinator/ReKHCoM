#ifndef GUARD_SAVE_H
#define GUARD_SAVE_H

#include "types.h"
#include "save_types.h"

extern u8 gSramFileLarge[];

#if defined(PLATFORM_PC) && defined(PORT_GBA_RELOC)
#include "gba/gba_space.h"
#define SRAM_HEADER ((u8*)(GBA_SRAM_BASE + 0x10))
#define SRAM_SYSTEM ((u8*)(GBA_SRAM_BASE + 0x90))
#define SRAM_FILE_SMALL ((u8*)(GBA_SRAM_BASE + 0x1EC0))
#else
#define SRAM_HEADER ((u8*)0x0E000010)
#define SRAM_SYSTEM ((u8*)0x0E000090)
#define SRAM_FILE_SMALL ((u8*)0x0E001EC0)
#endif

#define SAVE_HEADER_SIZE 0x40
#define SAVE_SYSTEM_SIZE 0xF14
#define SAVE_FILE_SMALL_SIZE 0x418
#define SAVE_FILE_LARGE_SIZE 0xF14

#define SAVE_SLOTS 2
#define SAVE_SIGNATURE_SIZE 0x18

extern const u8 gSaveSignatureText[SAVE_SIGNATURE_SIZE + 1];
extern const u8* gSaveSignature;

typedef struct SaveBlockLarge {
    u8 signature[SAVE_SIGNATURE_SIZE];
    u16 checksum;
    SaveFileLarge data;
} SaveBlockLarge;

typedef struct SaveBlockSmall {
    u8 signature[SAVE_SIGNATURE_SIZE];
    u16 checksum;
    SaveFileSmall data;
} SaveBlockSmall;

typedef struct SaveHeader {
    u8 signature[SAVE_SIGNATURE_SIZE];
    u16 checksum;
    SaveHeaderData data;
} SaveHeader;

void ZeroFill(void* dst, s16 size);
void CopyBytes(const u8* src, u8* dst, s16 len);
u8 BytesEqual(const u8* lhs, const u8* rhs, s16 len);
u16 SaveChecksum(u16* data, int size);
int SaveVerifyBlock(u8* sram, u8* hdr, u8* buf, s16 size);

void SaveWriteSystem();

void SaveWriteFileSmall(u16 file);
void WaitSramErrorInput();
void ReadKeysRaw();

#endif /* GUARD_SAVE_H */
