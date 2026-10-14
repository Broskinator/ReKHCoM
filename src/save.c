/**
 * save.c
 * SRAM Save Storage
 */

#include "agb_sram.h"
#include "gba/syscall.h"
#include "save.h"
#include "gba/io_reg.h"
#include "gba/keys.h"
#include "sram_error_screen.h"
#include "gba/defines.h"
#include "gba/macro.h"
#include "malloc.h"
#include "save_types.h"
#ifdef PLATFORM_PC
/* port_ps2qol.c: how each file's game was started, kept beside the saves.
   Files: Sora's 0-1, Riku's 2-3, the quick save -1. */
void PortRecomSaved(int file);
void PortRecomLoaded(int file);
#define PORT_RECOM_SAVED(f) PortRecomSaved(f)
#define PORT_RECOM_LOADED(f) PortRecomLoaded(f)
#else
#define PORT_RECOM_SAVED(f) ((void)0)
#define PORT_RECOM_LOADED(f) ((void)0)
#endif
#include "system_state.h"
#include "types.h"
#include "save_data.h"

static u16 sRawKeys;
static u16 sRawKeysPrev;
static u8 sSramErrorTilemapBuf[0x800];

void ZeroFill(void* dst, s16 size) {
    if (size & 1) {
        ((u8*)dst)[size - 1] = 0;
    }

    DmaFill16(3, 0, dst, size);
}

void CopyBytes(const u8* src, u8* dst, s16 len) {
    s16 i;

    for (i = 0; i < len; i++) {
        dst[i] = src[i];
    }
}

u8 BytesEqual(const u8* lhs, const u8* rhs, s16 len) {
    s16 i;

    for (i = 0; i < len; i++) {
        if (lhs[i] != rhs[i]) {
            return FALSE;
        }
    }

    return TRUE;
}

u16 SaveChecksum(u16* data, int size) {
    u32 sum;
    s16 len;

    len = size;
    sum = 0;

    while (len > 1) {
        sum += *data++;
        len -= 2;
    }

    if (len > 0) {
        sum += *(u8*)data;
    }

    sum = (sum & 0xFFFF) + (sum >> 16);
    return ~(sum + (sum >> 16));
}

int SaveVerifyBlock(u8* sram, u8* hdr, u8* buf, s16 size) {
    int ret;

    ZeroFill(buf, size);
    ReadSramFast(sram, buf, size);

    if (BytesEqual(hdr, gSaveSignature, SAVE_SIGNATURE_SIZE)) {
        ret = (SaveChecksum((u16*)buf, size) == 0) ? SAVE_OK : SAVE_BAD_CHECKSUM;
    } else {
        ret = SAVE_BAD_SIGNATURE;
    }

    return ret;
}

void SaveInitSram() {
    SetSramFastFunc();
}

void SaveClearHeader() {
    u8* buf;
    s16 i;

    buf = EwramAlloc(SAVE_HEADER_SIZE);

    for (i = 0; i < SAVE_SLOTS; i++) {
        ZeroFill(buf, SAVE_HEADER_SIZE);
        WriteAndVerifySramFast(buf, SRAM_HEADER + i * SAVE_HEADER_SIZE, SAVE_HEADER_SIZE);
    }

    EwramFree(buf);
}

int SaveCheckHeaderSlot(s16 slot) {
    u8* buf;
    int ret;

    buf = EwramAlloc(SAVE_HEADER_SIZE);
    ret = SaveVerifyBlock(SRAM_HEADER + (s16)(u16)slot * SAVE_HEADER_SIZE, buf, buf,
                        SAVE_HEADER_SIZE);
    EwramFree(buf);
    return ret;
}

int SaveRepairHeader() {
    int results[2];
    int good;
    int bad;
    s16 i;
    int ret;
    u8* buf;
    s64 off;

    good = -1;
    bad = -1;

    for (i = 0; i < SAVE_SLOTS; i++) {
        ret = results[i] = SaveCheckHeaderSlot(i);

        if (ret == SAVE_OK) {
            if (good < 0) {
                good = i;
            }
        } else {
            bad = i;
        }
    }

    if (good >= 0 && bad >= 0) {
        buf = EwramAlloc(SAVE_HEADER_SIZE);
        SaveVerifyBlock(SRAM_HEADER + good * SAVE_HEADER_SIZE, buf, buf, SAVE_HEADER_SIZE);

        for (i = 0; i < SAVE_SLOTS; i++) {
            if (results[i] != SAVE_OK) {
                off = i * SAVE_HEADER_SIZE;
                WriteAndVerifySramFast(buf, SRAM_HEADER + off, SAVE_HEADER_SIZE);
            }
        }

        EwramFree(buf);
        ret = SAVE_OK;
    }

    return ret;
}

int SaveLoadHeader() {
    u8* buf;
    int ret;
    s16 i;

    ret = 0;
    buf = EwramAlloc(SAVE_HEADER_SIZE);

    for (i = 0; i < SAVE_SLOTS; i++) {
        ret = SaveVerifyBlock(SRAM_HEADER + i * SAVE_HEADER_SIZE, buf, buf, SAVE_HEADER_SIZE);

        if (ret == SAVE_OK) {
            ApplySaveHeaderData(&((SaveHeader*)buf)->data);
            break;
        }
    }

    EwramFree(buf);
    return ret;
}

void SaveWriteHeader(s16 slot) {
    SaveHeader* hdr;
    s16 i;

    hdr = EwramAlloc(SAVE_HEADER_SIZE);
    ZeroFill(hdr, SAVE_HEADER_SIZE);
    MakeSaveHeaderData(&hdr->data, (u16)slot);
    CopyBytes(gSaveSignature, (u8*)hdr, SAVE_SIGNATURE_SIZE);
    hdr->checksum = 0;
    hdr->checksum = SaveChecksum((u16*)hdr, SAVE_HEADER_SIZE);

    for (i = 0; i < SAVE_SLOTS; i++) {
        WriteAndVerifySramFast((u8*)hdr, SRAM_HEADER + i * SAVE_HEADER_SIZE, SAVE_HEADER_SIZE);
    }

    EwramFree(hdr);
}

void SaveSetHeaderState(s16 slot, s16 state) {
    SaveHeader* hdr;

    hdr = EwramAlloc(SAVE_HEADER_SIZE);
    ZeroFill(hdr, SAVE_HEADER_SIZE);
    SaveVerifyBlock(SRAM_HEADER + slot * SAVE_HEADER_SIZE, (u8*)hdr, (u8*)hdr,
                    SAVE_HEADER_SIZE);

    switch (state) {
    case SAVE_BAD_SIGNATURE:
        hdr->signature[0] = 0;
        break;
    case SAVE_BAD_CHECKSUM:
        CopyBytes(gSaveSignature, (u8*)hdr, SAVE_SIGNATURE_SIZE);
        hdr->checksum = 0;
        hdr->checksum = SaveChecksum((u16*)hdr, SAVE_HEADER_SIZE) + 1;
        break;
    case SAVE_OK:
        CopyBytes(gSaveSignature, (u8*)hdr, SAVE_SIGNATURE_SIZE);
        hdr->checksum = 0;
        hdr->checksum = SaveChecksum((u16*)hdr, SAVE_HEADER_SIZE);
        break;
    }

    WriteAndVerifySramFast((u8*)hdr, SRAM_HEADER + slot * SAVE_HEADER_SIZE,
                           SAVE_HEADER_SIZE);
    EwramFree(hdr);
}

void SaveClearSystem() {
    u8* buf;
    s16 i;

    buf = EwramAlloc(SAVE_SYSTEM_SIZE);

    for (i = 0; i < SAVE_SLOTS; i++) {
        ZeroFill(buf, SAVE_SYSTEM_SIZE);
        WriteAndVerifySramFast(buf, SRAM_SYSTEM + i * SAVE_SYSTEM_SIZE, SAVE_SYSTEM_SIZE);
    }

    EwramFree(buf);
}

int SaveCheckSystemSlot(s16 slot) {
    u8* buf;
    int ret;

    buf = EwramAlloc(SAVE_SYSTEM_SIZE);
    ret = SaveVerifyBlock(SRAM_SYSTEM + (s16)(u16)slot * SAVE_SYSTEM_SIZE, buf, buf,
                        SAVE_SYSTEM_SIZE);
    EwramFree(buf);
    return ret;
}

int SaveRepairSystem() {
    int results[2];
    int good;
    int bad;
    s16 i;
    int ret;
    u8* buf;

    good = -1;
    bad = -1;

    for (i = 0; i < SAVE_SLOTS; i++) {
        ret = results[i] = SaveCheckSystemSlot(i);

        if (ret == SAVE_OK) {
            if (good < 0) {
                good = i;
            }
        } else {
            bad = i;
        }
    }

    if (good >= 0 && bad >= 0) {
        buf = EwramAlloc(SAVE_SYSTEM_SIZE);
        SaveVerifyBlock(SRAM_SYSTEM + good * SAVE_SYSTEM_SIZE, buf, buf, SAVE_SYSTEM_SIZE);

        for (i = 0; i < SAVE_SLOTS; i++) {
            if (results[i] != SAVE_OK) {
                WriteAndVerifySramFast(buf, SRAM_SYSTEM + i * SAVE_SYSTEM_SIZE,
                                       SAVE_SYSTEM_SIZE);
            }
        }

        EwramFree(buf);
        ret = SAVE_OK;
    }

    return ret;
}

int SaveLoadSystem() {
    u8* buf;
    int ret;
    s16 i;
    s16 slot;

    ret = 0;
    buf = EwramAlloc(SAVE_SYSTEM_SIZE);

    for (i = 0; i < SAVE_SLOTS; i++) {
        slot = i;
        ret = SaveVerifyBlock(SRAM_SYSTEM + slot * SAVE_SYSTEM_SIZE, buf, buf, SAVE_SYSTEM_SIZE);

        if (ret == SAVE_OK) {
            ApplySaveSystem(&((SaveBlockLarge*)buf)->data);
            PORT_RECOM_LOADED(-1);
            break;
        }
    }

    EwramFree(buf);
    return ret;
}

void SaveWriteSystem() {
    SaveBlockLarge* blk;
    s16 i;

    blk = EwramAlloc(SAVE_SYSTEM_SIZE);
    ZeroFill(blk, SAVE_SYSTEM_SIZE);
    MakeSaveSystem(&blk->data);
    CopyBytes(gSaveSignature, (u8*)blk, SAVE_SIGNATURE_SIZE);
    blk->checksum = 0;
    blk->checksum = SaveChecksum((u16*)blk, SAVE_SYSTEM_SIZE);

    for (i = 0; i < SAVE_SLOTS; i++) {
        WriteAndVerifySramFast((u8*)blk, SRAM_SYSTEM + i * SAVE_SYSTEM_SIZE, SAVE_SYSTEM_SIZE);
    }

    EwramFree(blk);
    PORT_RECOM_SAVED(-1);
}

void SaveSetSystemState(s16 slot, s16 state) {
    SaveBlockLarge* blk;

    blk = EwramAlloc(SAVE_SYSTEM_SIZE);
    ZeroFill(blk, SAVE_SYSTEM_SIZE);
    SaveVerifyBlock(SRAM_SYSTEM + slot * SAVE_SYSTEM_SIZE, (u8*)blk, (u8*)blk,
                    SAVE_SYSTEM_SIZE);

    switch (state) {
    case SAVE_BAD_SIGNATURE:
        blk->signature[0] = 0;
        break;
    case SAVE_BAD_CHECKSUM:
        CopyBytes(gSaveSignature, (u8*)blk, SAVE_SIGNATURE_SIZE);
        blk->checksum = 0;
        blk->checksum = SaveChecksum((u16*)blk, SAVE_SYSTEM_SIZE) + 1;
        break;
    case SAVE_OK:
        CopyBytes(gSaveSignature, (u8*)blk, SAVE_SIGNATURE_SIZE);
        blk->checksum = 0;
        blk->checksum = SaveChecksum((u16*)blk, SAVE_SYSTEM_SIZE);
        break;
    }

    WriteAndVerifySramFast((u8*)blk, SRAM_SYSTEM + slot * SAVE_SYSTEM_SIZE,
                           SAVE_SYSTEM_SIZE);
    EwramFree(blk);
}

void SaveClearFileLarge(u16 file) {
    u8* buf;
    u8* dst;
    s16 i;
    s32 off;

    buf = EwramAlloc(SAVE_FILE_LARGE_SIZE);
    i = 0;
    off = (s16)file * (SAVE_FILE_LARGE_SIZE * 2);

    for (; i < SAVE_SLOTS; i++) {
        ZeroFill(buf, SAVE_FILE_LARGE_SIZE);
        dst = gSramFileLarge + i * SAVE_FILE_LARGE_SIZE;
        WriteAndVerifySramFast(buf, dst + off, SAVE_FILE_LARGE_SIZE);
    }

    EwramFree(buf);
}

int SaveCheckFileLargeSlot(s16 file, s16 slot) {
    u8* buf;
    int ret;

    buf = EwramAlloc(SAVE_FILE_LARGE_SIZE);
    ret = SaveVerifyBlock(gSramFileLarge + (s16)(u16)file * (SAVE_FILE_LARGE_SIZE * 2)
                            + (s16)(u16)slot * SAVE_FILE_LARGE_SIZE,
                        buf, buf, SAVE_FILE_LARGE_SIZE);
    EwramFree(buf);
    return ret;
}

int SaveRepairFileLarge(u16 file) {
    int results[2];
    int good;
    int bad;
    s16 i;
    int ret;
    u8* buf;
    u8* src;
    u8* dst;
    s32 off;

    good = -1;
    bad = -1;

    for (i = 0; i < SAVE_SLOTS; i++) {
        ret = results[i] = SaveCheckFileLargeSlot(file, i);

        if (ret == SAVE_OK) {
            if (good < 0) {
                good = i;
            }
        } else {
            bad = i;
        }
    }

    if (good >= 0 && bad >= 0) {
        buf = EwramAlloc(SAVE_FILE_LARGE_SIZE);
        off = (s16)file * (SAVE_FILE_LARGE_SIZE * 2);
        src = gSramFileLarge + good * SAVE_FILE_LARGE_SIZE;
        SaveVerifyBlock((u8*)(off + (u32)src), buf, buf, SAVE_FILE_LARGE_SIZE);

        for (i = 0; i < SAVE_SLOTS; i++) {
            if (results[i] != SAVE_OK) {
                dst = gSramFileLarge + i * SAVE_FILE_LARGE_SIZE;
                WriteAndVerifySramFast(buf, (u8*)((s16)file * (SAVE_FILE_LARGE_SIZE * 2) + (u32)dst), SAVE_FILE_LARGE_SIZE);
            }
        }

        EwramFree(buf);
        ret = SAVE_OK;
    }

    return ret;
}

int SaveLoadFileLarge(u16 file) {
    u8* buf;
    u8* dst;
    int ret;
    s16 i;
    s32 off;

    ret = 0;
    buf = EwramAlloc(SAVE_FILE_LARGE_SIZE);

    for (i = 0; i < SAVE_SLOTS; i++) {
        dst = gSramFileLarge + i * SAVE_FILE_LARGE_SIZE;
        off = (s16)file * (SAVE_FILE_LARGE_SIZE * 2);
        ret = SaveVerifyBlock(off + dst, buf, buf, SAVE_FILE_LARGE_SIZE);

        if (ret == SAVE_OK) {
            ApplySaveFileLarge(&((SaveBlockLarge*)buf)->data);
            PORT_RECOM_LOADED(file);
            break;
        }
    }

    EwramFree(buf);
    return ret;
}

void SaveWriteFileLarge(u16 file) {
    SaveBlockLarge* blk;
    u8* dst;
    s16 i;
    s32 off;

    blk = EwramAlloc(SAVE_FILE_LARGE_SIZE);
    ZeroFill(blk, SAVE_FILE_LARGE_SIZE);
    MakeSaveFileLarge(&blk->data);
    CopyBytes(gSaveSignature, (u8*)blk, SAVE_SIGNATURE_SIZE);
    blk->checksum = 0;
    blk->checksum = SaveChecksum((u16*)blk, SAVE_FILE_LARGE_SIZE);

    for (i = 0; i < SAVE_SLOTS; i++) {
        off = (s16)file * (SAVE_FILE_LARGE_SIZE * 2);
        dst = gSramFileLarge + i * SAVE_FILE_LARGE_SIZE;
        WriteAndVerifySramFast((u8*)blk, (u8*)(off + (u32)dst), SAVE_FILE_LARGE_SIZE);
    }

    EwramFree(blk);
    SaveWriteHeader(file);
    PORT_RECOM_SAVED(file);
}

void SaveSetFileLargeState(s16 file, s16 slot, s16 state) {
    SaveBlockLarge* blk;

    blk = EwramAlloc(SAVE_FILE_LARGE_SIZE);
    ZeroFill(blk, SAVE_FILE_LARGE_SIZE);
    SaveVerifyBlock(gSramFileLarge + file * (SAVE_FILE_LARGE_SIZE * 2)
                        + slot * SAVE_FILE_LARGE_SIZE,
                    (u8*)blk, (u8*)blk, SAVE_FILE_LARGE_SIZE);

    switch (state) {
    case SAVE_BAD_SIGNATURE:
        blk->signature[0] = 0;
        break;
    case SAVE_BAD_CHECKSUM:
        CopyBytes(gSaveSignature, (u8*)blk, SAVE_SIGNATURE_SIZE);
        blk->checksum = 0;
        blk->checksum = SaveChecksum((u16*)blk, SAVE_FILE_LARGE_SIZE) + 1;
        break;
    case SAVE_OK:
        CopyBytes(gSaveSignature, (u8*)blk, SAVE_SIGNATURE_SIZE);
        blk->checksum = 0;
        blk->checksum = SaveChecksum((u16*)blk, SAVE_FILE_LARGE_SIZE);
        break;
    }

    WriteAndVerifySramFast((u8*)blk, gSramFileLarge + file * (SAVE_FILE_LARGE_SIZE * 2)
                               + slot * SAVE_FILE_LARGE_SIZE,
                           SAVE_FILE_LARGE_SIZE);
    EwramFree(blk);
}

void SaveClearFileSmall(u16 file) {
    u8* buf;
    u8* dst;
    s16 i;
    s32 off;

    buf = EwramAlloc(SAVE_FILE_SMALL_SIZE);
    i = 0;
    off = (s16)file * (SAVE_FILE_SMALL_SIZE * 2);

    for (; i < SAVE_SLOTS; i++) {
        ZeroFill(buf, SAVE_FILE_SMALL_SIZE);
        dst = SRAM_FILE_SMALL + i * SAVE_FILE_SMALL_SIZE;
        WriteAndVerifySramFast(buf, dst + off, SAVE_FILE_SMALL_SIZE);
    }

    EwramFree(buf);
}

int SaveCheckFileSmallSlot(s16 file, s16 slot) {
    u8* buf;
    int ret;

    buf = EwramAlloc(SAVE_FILE_SMALL_SIZE);
    ret = SaveVerifyBlock(SRAM_FILE_SMALL + (s16)(u16)file * (SAVE_FILE_SMALL_SIZE * 2)
                            + (s16)(u16)slot * SAVE_FILE_SMALL_SIZE,
                        buf, buf, SAVE_FILE_SMALL_SIZE);
    EwramFree(buf);
    return ret;
}

int SaveRepairFileSmall(u16 file) {
    int results[2];
    int good;
    int bad;
    s16 i;
    int ret;
    u8* buf;
    u8* src;
    u8* dst;
    s32 off;

    good = -1;
    bad = -1;

    for (i = 0; i < SAVE_SLOTS; i++) {
        ret = results[i] = SaveCheckFileSmallSlot(file, i);

        if (ret == SAVE_OK) {
            if (good < 0) {
                good = i;
            }
        } else {
            bad = i;
        }
    }

    if (good >= 0 && bad >= 0) {
        buf = EwramAlloc(SAVE_FILE_SMALL_SIZE);
        off = (s16)file * (SAVE_FILE_SMALL_SIZE * 2);
        src = SRAM_FILE_SMALL + good * SAVE_FILE_SMALL_SIZE;
        SaveVerifyBlock((u8*)(off + (u32)src), buf, buf, SAVE_FILE_SMALL_SIZE);

        for (i = 0; i < SAVE_SLOTS; i++) {
            if (results[i] != SAVE_OK) {
                dst = SRAM_FILE_SMALL + i * SAVE_FILE_SMALL_SIZE;
                WriteAndVerifySramFast(buf, (u8*)((s16)file * (SAVE_FILE_SMALL_SIZE * 2) + (u32)dst), SAVE_FILE_SMALL_SIZE);
            }
        }

        EwramFree(buf);
        ret = SAVE_OK;
    }

    return ret;
}

int SaveLoadFileSmall(u16 file) {
    u8* buf;
    u8* dst;
    int ret;
    s16 i;

    buf = EwramAlloc(SAVE_FILE_SMALL_SIZE);

    for (i = 0; i < SAVE_SLOTS; i++) {
        dst = SRAM_FILE_SMALL + i * SAVE_FILE_SMALL_SIZE;
        ret = SaveVerifyBlock((u8*)((s16)file * (SAVE_FILE_SMALL_SIZE * 2) + (u32)dst), buf,
                              buf, SAVE_FILE_SMALL_SIZE);

        if (ret == SAVE_OK) {
            ApplySaveFileSmall(&((SaveBlockSmall*)buf)->data);
            PORT_RECOM_LOADED(file + 2);
            break;
        }
    }

    EwramFree(buf);
    return ret;
}

void SaveWriteFileSmall(u16 file) {
    SaveBlockSmall* blk;
    u8* dst;
    s16 i;
    s32 off;
    s16 fileIndex;

    fileIndex = file;
    blk = EwramAlloc(SAVE_FILE_SMALL_SIZE);
    ZeroFill(blk, SAVE_FILE_SMALL_SIZE);
    MakeSaveFileSmall(&blk->data);
    CopyBytes(gSaveSignature, (u8*)blk, SAVE_SIGNATURE_SIZE);
    blk->checksum = 0;
    blk->checksum = SaveChecksum((u16*)blk, SAVE_FILE_SMALL_SIZE);

    for (i = 0; i < SAVE_SLOTS; i++) {
        off = fileIndex * (SAVE_FILE_SMALL_SIZE * 2);
        dst = SRAM_FILE_SMALL + i * SAVE_FILE_SMALL_SIZE;
        WriteAndVerifySramFast((u8*)blk, (u8*)(off + (u32)dst), SAVE_FILE_SMALL_SIZE);
    }

    EwramFree(blk);
    SaveWriteHeader(fileIndex + 2);
    PORT_RECOM_SAVED(fileIndex + 2);
}

void SaveSetFileSmallState(s16 file, s16 slot, s16 state) {
    SaveBlockSmall* blk;

    blk = EwramAlloc(SAVE_FILE_SMALL_SIZE);
    ZeroFill(blk, SAVE_FILE_SMALL_SIZE);
    SaveVerifyBlock(SRAM_FILE_SMALL + file * (SAVE_FILE_SMALL_SIZE * 2)
                        + slot * SAVE_FILE_SMALL_SIZE,
                    (u8*)blk, (u8*)blk, SAVE_FILE_SMALL_SIZE);

    switch (state) {
    case SAVE_BAD_SIGNATURE:
        blk->signature[0] = 0;
        break;
    case SAVE_BAD_CHECKSUM:
        CopyBytes(gSaveSignature, (u8*)blk, SAVE_SIGNATURE_SIZE);
        blk->checksum = 0;
        blk->checksum = SaveChecksum((u16*)blk, SAVE_FILE_SMALL_SIZE) + 1;
        break;
    case SAVE_OK:
        CopyBytes(gSaveSignature, (u8*)blk, SAVE_SIGNATURE_SIZE);
        blk->checksum = 0;
        blk->checksum = SaveChecksum((u16*)blk, SAVE_FILE_SMALL_SIZE);
        break;
    }

    WriteAndVerifySramFast((u8*)blk, SRAM_FILE_SMALL + file * (SAVE_FILE_SMALL_SIZE * 2)
                               + slot * SAVE_FILE_SMALL_SIZE,
                           SAVE_FILE_SMALL_SIZE);
    EwramFree(blk);
}

void ShowSramErrorScreen() {
    REG_IME = 0;
    REG_IE |= INTR_FLAG_VBLANK;
    REG_DISPSTAT |= DISPSTAT_VBLANK_INTR;
    REG_IME = 1;
    REG_BG0CNT = 0x88;
    REG_BLDCNT = 0x3FBF;
    REG_BLDY = 0x10;
    REG_DISPCNT = (DISPCNT_BG0_ON | DISPCNT_OBJ_ON);
    VBlankIntrWait();
    DmaCopy16(3, gSramErrorTiles, BG_CHAR_ADDR(2), sizeof(gSramErrorTiles));
    DmaCopy16(3, gSramErrorPalette, BG_PLTT, BG_PLTT_SIZE);
    DmaCopy16(3, gSramErrorTilemap, sSramErrorTilemapBuf, sizeof(gSramErrorTilemap));
    DmaCopy16(3, sSramErrorTilemapBuf, VRAM, sizeof(sSramErrorTilemapBuf));
    WaitSramErrorInput();
    REG_IME = 0;
    REG_IE &= ~INTR_FLAG_VBLANK;
    REG_DISPSTAT &= ~DISPSTAT_VBLANK_INTR;
    REG_IME = 1;
    REG_DISPCNT = 0;
}

void WaitSramErrorInput() {
    vu16* bldy;
    vu16* bldy2;
    s32 i;
    u32 j;
    s32 prev;
    s32 cur;
    s32 ok;

    i = 0;
    ok = FALSE;
    cur = 0;
    prev = 0;
    j = 0;
    bldy = (vu16*)REG_ADDR_BLDY;

    do {
        VBlankIntrWait();
        *bldy = 16 - j;
        j++;
    } while (j <= 16);

    if (i <= 19) {
        do {
            ReadKeysRaw();

            if ((((sRawKeysPrev ^ sRawKeys) & sRawKeys) & DPAD_ANY) == DPAD_ANY) {
                prev = cur;
                cur = i;
            }

            if (prev != 0 && cur - prev <= 3) {
                ok = TRUE;
            }

            i++;
            VBlankIntrWait();
            DmaCopy32(3, sSramErrorTilemapBuf, VRAM, sizeof(sSramErrorTilemapBuf));
        } while (i <= 19);
    }

    j = 0;
    bldy2 = (vu16*)REG_ADDR_BLDY;

    do {
        VBlankIntrWait();
        *bldy2 = j;
        j++;
    } while (j <= 16);

    if (ok) {
        gSystemFlags |= 4;
    } else {
        gSystemFlags &= 0xFFFB;
    }
}

void ReadKeysRaw() {
    u16 keys = KEYS_MASK ^ REG_KEYINPUT;

    sRawKeysPrev = sRawKeys;
    sRawKeys = keys;
}
