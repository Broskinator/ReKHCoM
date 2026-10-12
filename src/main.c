/**
 * main.c
 * Main Loop and Interrupts
 */

#include "chara_api.h"
#include "intr.h"
#include "gba/syscall.h"
#include "key.h"
#include "malloc.h"
#include "m4a.h"
#include "pallet.h"
#include "sprite.h"
#include "sroll.h"
#include "key_state.h"
#include "sio.h"
#include "engine.h"
#include "main.h"
#include "mode.h"
#include "system_state.h"
#include "gba/io_reg.h"
#include "sroll_api.h"
#include "sio_api.h"
#include "save_api.h"
#include <stddef.h>
#include "engine_math.h"
#include "gba/defines.h"
#include "gba/macro.h"
#include "types.h"
#ifdef PLATFORM_PC
#include "game_state.h"
#include "registration_data.h"
#include "mode_movie.h"
extern int gPortIntroRequested;
extern int gPortStartupLanguage;
#endif

vu16 gFrameSyncFlags;
u16 gVBlankEndVCount;
u32 gUnk_03006C04[3];
u32 gDebugFlags;
IntrFunc* gIntrTableSerial;
u32 gSoftResetMarker[2];
IntrFunc gIntrTable[INTR_COUNT];
IntrFunc* gIntrTableVCount;
IntrFunc* gIntrTableVBlank;
IntrFunc* gIntrTableTimer3;
IntrFunc gHBlankCallback;
vu32 gVBlankCounter;
IntrFunc gVCountCallback;
IntrFunc gVBlankCallback;
IntrFunc* gIntrTableHBlank;
vu16 gSystemFlags;
u8 gUnk_03006C7A[6];
u8 gIntrHandler[0x800];
vu32 gFrameCounter;
#ifdef VERSION_EU
u32 gLanguage;
#endif
IntrFunc gVBlankHandlerOverride;

void* GetEwramHeapStart() {
    return gEwramHeapStart;
}

u32 GetEwramHeapSize() {
    return EWRAM_HEAP_SIZE;
}

void* GetIwramHeapStart() {
    return gIwramHeapStart;
}

u32 GetIwramHeapSize() {
    return IWRAM_HEAP_SIZE;
}

void EnableVBlankIntr() {
    REG_IME = 0;
    REG_IE |= INTR_FLAG_VBLANK;
    REG_DISPSTAT |= DISPSTAT_VBLANK_INTR;
    m4aSoundVSyncOn();
    REG_IME = 1;
}

void DisableVBlankIntr() {
    REG_IME = 0;
    REG_IE &= ~INTR_FLAG_VBLANK;
    REG_DISPSTAT &= ~DISPSTAT_VBLANK_INTR;
    m4aSoundVSyncOff();
    REG_IME = 1;
}

void EnableHBlankIntr() {
    REG_IME = 0;
    REG_IE |= INTR_FLAG_HBLANK;
    REG_DISPSTAT |= DISPSTAT_HBLANK_INTR;
    REG_IME = 1;
}

void DisableHBlankIntr() {
    REG_IME = 0;
    REG_IE &= ~INTR_FLAG_HBLANK;
    REG_DISPSTAT &= ~DISPSTAT_HBLANK_INTR;
    REG_IME = 1;
}

#ifdef VERSION_EU
void ClearSystemMemory() {
    RegisterRamReset(RESET_ALL);
    REG_WAITCNT = WAITCNT_SRAM_2 | WAITCNT_WS0_N_3 | WAITCNT_WS0_S_1 | WAITCNT_WS1_N_3 | WAITCNT_WS1_S_1 | WAITCNT_WS2_N_3 | WAITCNT_WS2_S_1 | WAITCNT_PREFETCH_ENABLE;
    CpuFill32(0, (void*)EWRAM_START, EWRAM_SIZE);
    CpuFill32(0, (void*)IWRAM_START, IWRAM_SIZE - 0x200);
    CpuFill32(0, (void*)VRAM, VRAM_SIZE);
}
#endif

void InitSystem() {
#ifdef VERSION_EU
    u32 softReset;

    if (gSoftResetMarker[0] == SOFT_RESET_MAGIC) {
        ClearSystemMemory();
        softReset = TRUE;
    } else {
        ClearSystemMemory();
        softReset = FALSE;
    }
#else
    RegisterRamReset(RESET_ALL);
    REG_WAITCNT = WAITCNT_SRAM_2 | WAITCNT_WS0_N_3 | WAITCNT_WS0_S_1 | WAITCNT_WS1_N_3 | WAITCNT_WS1_S_1 | WAITCNT_WS2_N_3 | WAITCNT_WS2_S_1 | WAITCNT_PREFETCH_ENABLE;
    DmaFill32(3, 0, EWRAM_START, EWRAM_SIZE);
    DmaFill32(3, 0, IWRAM_START, IWRAM_SIZE - 0x200);
#endif
    gVBlankEndVCount = 0;
    gFrameSyncFlags = 0;
    gVBlankHandlerOverride = NULL;
#ifdef VERSION_EU
    gLanguage = LANGUAGE_ENGLISH;
#endif
    REG_IME = 0;
    DmaCopy32(3, IrqHandler, gIntrHandler, sizeof(gIntrHandler));
    INTR_VECTOR = gIntrHandler;
    REG_IE = INTR_FLAG_GAMEPAK;
    REG_IF = INTR_FLAG_GAMEPAK;
    REG_IME = 1;
    InitIntrTable();
    m4aSoundInit();
    m4aSoundVSyncOff();
    IwramHeapInit(GetIwramHeapStart(), GetIwramHeapSize());
    EwramHeapInit(GetEwramHeapStart(), GetEwramHeapSize());
    VTransInit();
    SpriteInit();
    BgInit();
    FadeInit();
    PalletInit();
    SioKeyInit();
    ResetPaletteEffect();
    ResetKeyState();
    SeedRandom(1234567);
    InitDisplayRegs();
    SaveInitSram();
    ScanlineDmaReset();
#ifdef VERSION_EU
    ModeInit(softReset);
#ifdef PLATFORM_PC
    if (gPortStartupLanguage >= 0)
        gLanguage = gPortStartupLanguage;
    if (gPortIntroRequested) {
        gPortIntroRequested = 0;
        SetupSoraNewGame();
        ModeRequestHeapReset(&gModeMovie, MOVIE_OPENING);
    }
#endif
#else
    ModeInit();
#endif
}

void AgbMain() {
    gFrameCounter = 0;
    gVBlankCounter = 0;
    gSystemFlags = 0;
    gDebugFlags = 0;
    gSioPlayerId = 0;
    gSioStatus = 0;
    InitSystem();
    EnableVBlankIntr();

    for (;;) {
        UpdateKeyState();

        if (gSystemFlags & SYSTEM_FLAG_LINK_ACTIVE) {
            SioLinkUpdate();

            if (!(gSioStatus & SIO_STAT_RECV_EMPTY)) {
                if (!(gFrameSyncFlags & FRAME_SYNC_FRAME_READY)) {
                    ModeUpdate();
                    gFrameSyncFlags |= FRAME_SYNC_FRAME_READY;
                }
            }
        } else {
            if (!(gFrameSyncFlags & FRAME_SYNC_FRAME_READY)) {
                ModeUpdate();
                gFrameSyncFlags |= FRAME_SYNC_FRAME_READY;
            }
        }

        ApplyIntrCallbacks();
        VBlankIntrWait();
        gFrameCounter++;
    }
}

void VBlankIntr() {
    if (gVBlankHandlerOverride != NULL) {
        gVBlankHandlerOverride();
        return;
    }

    if (gFrameSyncFlags & FRAME_SYNC_IN_VBLANK) {
        gFrameSyncFlags |= FRAME_SYNC_VBLANK_OVERRUN;
        return;
    }

    gFrameSyncFlags |= FRAME_SYNC_IN_VBLANK;

    if (!(gFrameSyncFlags & FRAME_SYNC_SOUND_BUSY)) {
        m4aSoundVSync();
    }

    gIntrCheck |= INTR_FLAG_VBLANK;

    if (gFrameSyncFlags & FRAME_SYNC_FRAME_READY) {
        ModeFlushDisplay();
    }

    ModeRunVBlankCallbacks();
    ScanlineDmaUpdate();
    gVBlankEndVCount = REG_VCOUNT;
    gFrameSyncFlags &= ~FRAME_SYNC_FRAME_READY;

    if (!(gFrameSyncFlags & FRAME_SYNC_SOUND_BUSY)) {
        gFrameSyncFlags |= FRAME_SYNC_SOUND_BUSY;
        m4aSoundMain();
        gFrameSyncFlags &= ~FRAME_SYNC_SOUND_BUSY;
    }

    gFrameSyncFlags &= ~FRAME_SYNC_IN_VBLANK;
    gVBlankCounter++;
}

void HBlankIntrDummy() {
}

void VCountIntrDummy() {
}

void SerialIntrDummy() {
}

static IntrFunc sIntrTableTemplate[INTR_COUNT] = {
    SerialIntrDummy, VBlankIntr, HBlankIntrDummy, VCountIntrDummy, SerialIntrDummy, SerialIntrDummy, SerialIntrDummy,
    SerialIntrDummy, SerialIntrDummy, SerialIntrDummy, SerialIntrDummy, SerialIntrDummy, SerialIntrDummy, SerialIntrDummy,
};

void InitIntrTable() {
    s32 i;

    for (i = 0; i < INTR_COUNT; i++) {
        gIntrTable[i] = sIntrTableTemplate[i];
    }

    gIntrTableVBlank = &gIntrTable[1];
    gIntrTableVCount = &gIntrTable[3];
    gIntrTableHBlank = &gIntrTable[2];
    gIntrTableSerial = &gIntrTable[0];
    gIntrTableTimer3 = &gIntrTable[7];
    ResetVBlankCallback();
    ResetVCountCallback();
    ResetHBlankCallback();
    ResetSerialCallback();
    ResetTimer3Callback();
}

void ApplyIntrCallbacks() {
    *gIntrTableVBlank = gVBlankCallback;
    *gIntrTableVCount = gVCountCallback;
    *gIntrTableHBlank = gHBlankCallback;
}

void VBlankIntrSio() {
    if (gFrameSyncFlags & FRAME_SYNC_IN_VBLANK) {
        gFrameSyncFlags |= FRAME_SYNC_VBLANK_OVERRUN;
        return;
    }

    gFrameSyncFlags |= FRAME_SYNC_IN_VBLANK;
    SioVBlankUpdate();

    if (!(gFrameSyncFlags & FRAME_SYNC_SOUND_BUSY)) {
        m4aSoundVSync();
    }

    gIntrCheck |= INTR_FLAG_VBLANK;

    if (gFrameSyncFlags & FRAME_SYNC_FRAME_READY) {
        ModeFlushDisplay();
    }

    ModeRunVBlankCallbacks();
    gVBlankEndVCount = REG_VCOUNT;
    gFrameSyncFlags &= ~FRAME_SYNC_FRAME_READY;

    if (!(gFrameSyncFlags & FRAME_SYNC_SOUND_BUSY)) {
        gFrameSyncFlags |= FRAME_SYNC_SOUND_BUSY;
        m4aSoundMain();
        gFrameSyncFlags &= ~FRAME_SYNC_SOUND_BUSY;
    }

    gFrameSyncFlags &= ~FRAME_SYNC_IN_VBLANK;
    gVBlankCounter++;
}

void VBlankIntrBlockAudio() {
    if (gFrameSyncFlags & FRAME_SYNC_IN_VBLANK) {
        gFrameSyncFlags |= FRAME_SYNC_VBLANK_OVERRUN;
        return;
    }

    gFrameSyncFlags |= FRAME_SYNC_IN_VBLANK;
    REG_IME = 0;
    BlockAudioVBlank();
    REG_IME = 1;
    gIntrCheck |= INTR_FLAG_VBLANK;

    if (gFrameSyncFlags & FRAME_SYNC_FRAME_READY) {
        ModeFlushDisplay();
    }

    ModeRunVBlankCallbacks();
    ScanlineDmaUpdate();
    gVBlankEndVCount = REG_VCOUNT;
    gFrameSyncFlags &= ~FRAME_SYNC_FRAME_READY;
    gFrameSyncFlags &= ~FRAME_SYNC_IN_VBLANK;
    gVBlankCounter++;
}
