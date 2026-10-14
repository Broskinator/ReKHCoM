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
#include "field_state.h"
#include "map.h"
#include "map_tasks.h"
#include "battle_work.h"
#include "battle_actor.h"
#include "card.h"
#include "taskpool.h"
#include "listpool.h"
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
#include "mode_pooh_api.h"
#include <stddef.h>
#include <string.h>
#include "engine_math.h"
#include "gba/defines.h"
#include "gba/macro.h"
#include "types.h"
#ifdef PLATFORM_PC
#include "game_state.h"
#include "map_api.h"
#include "registration_data.h"
#include "mode_movie.h"
#include "mode_chkbtl.h"
#include "battle.h"
#include "mode_mapchk.h"
#include "map_runtime.h"
#include "card_api.h"
#include "player_progression_types.h"
#include "map_check_worlds.h"
#include "btl4.h"
#include "sprites_btl_hud.h"
#ifdef PLATFORM_PC
#include "mode_mapchk.h"
#include "map_runtime.h"
#include "card_api.h"
#include "map_types.h"
#include "player_progression_types.h"
#include "map_check_worlds.h"
#endif
extern int gPortIntroRequested;
extern int gPortEventTest;
extern int gPortMovieTest;
extern int gPortBattleTest;
extern Mode gModeEventselect;
void PortEventSelectPreset(int list, int index);
extern int gPortStartupLanguage;
#endif

#ifdef PLATFORM_PC
/* Test harness (KHCOM_MODE): switch to one of these modes by name. */
extern const MapGmkDef gMapGmk04Def;
extern TaskDesc gTaskDescMapGmk04, gTaskDescMapSave;
void PortMapKeepGmkDesc(const TaskDesc* desc);
extern Mode gModeDeck, gModeStatus, gModeJiminy, gModeAllmap, gModeMapinspect, gModeWorldinspect,
    gModeWorldselect, gModeWORLDSELECT, gModeWorldwarp, gModeMsTop, gModePremire,
    gModeSioBtlOption, gModeDebug;

void PortHarnessSong(int song) {
    m4aSongNumStart((u16)song);
}

const char* PortHarnessModeName(void) {
    extern Mode* gCurrentMode;
    return gCurrentMode != NULL ? gCurrentMode->name : "none";
}

int PortHarnessMode(const char* name, int arg) {
    static Mode* const modes[] = {
        &gModeDeck, &gModeStatus, &gModeJiminy, &gModeAllmap, &gModeMapinspect, &gModeWorldinspect,
        &gModeWorldselect, &gModeWORLDSELECT, &gModeWorldwarp, &gModeMsTop, &gModePremire,
        &gModeSioBtlOption, &gModeDebug, &gModePooh,
    };
    u32 i;

    /* Isolated HUD regression: change actual actor HP, letting the original
       task perform damage/healing interpolation and expression changes. */
    if (strcmp(name, "hudhp") == 0 && gBtlWork != NULL && gBtlWork->actor != NULL) {
        extern void PortLog(const char* format, ...);
        int hp;
        if (arg < 0 || arg > 100) return 0;
        hp = gBtlWork->actor->maxHp * arg / 100;
        if (hp < 1) hp = 1; /* avoid ending the battle during the render probe */
        gBtlWork->actor->hp = hp;
        gBtlWork->actor->invincibleTimer = 3000;
        PortLog("harness: hudhp %d/%d", hp, gBtlWork->actor->maxHp);
        return 1;
    }

    /* Small numeric text fixture: use the existing battle task and its
       original DrawSprite path. This does not award experience or a level. */
    if (strcmp(name, "textdigits") == 0 && gBtlWork != NULL) {
        Task* task = ListPoolFirst(&gBtlWork->taskPools[1].head);
        BtlExpWork* work;
        extern void PortLog(const char* format, ...);
        while (task != NULL && task->desc != &gTaskDescBtlExp)
            task = ListPoolNext(&task->node);
        if (task == NULL)
            task = TaskCreate(&gBtlWork->taskPools[1], &gTaskDescBtlExp, NULL);
        if (task == NULL)
            return 0;
        work = task->work;
        work->level = gGameState.progression.level;
        work->lastExp = gGameState.progression.exp;
        work->timer = 0;
        work->state = 1; /* BTL_EXP_STATE_GAIN, local to btl_exp.c */
#ifdef VERSION_JP
        work->gfx = gBtlExpNextFrame2;
#else
        work->gfx = gBtlExpNextLvFrame2;
#endif
        BtlExpSetNumber(work, (u32)arg % 100000);
        PortLog("harness: textdigits %d", arg % 100000);
        return 1;
    }

    if (strcmp(name, "progress") == 0) {
        extern void PortLog(const char* format, ...);
        PortLog("harness: progress flags=%u floor=%u world=%u room=%u story=%u level=%u mode=%s",
                (unsigned)gGameState.flags, (unsigned)gGameState.floor,
                (unsigned)gMapFloorState.world, (unsigned)gMapFloorState.room,
                (unsigned)gMapFloorState.progress, (unsigned)gGameState.progression.level,
                PortHarnessModeName());
        return 1;
    }
    if (strcmp(name, "roomgraph") == 0 || strcmp(name, "roomgoto") == 0) {
        u8 seen[32] = {0}, queue[32], sides[32] = {0};
        int head = 0, count = 1, side, room, neighbor;
        extern void PortLog(const char* format, ...);
        if (gFieldState == NULL || gGameState.floor >= ((gGameState.flags & GAME_FLAG_RIKU) ? 12 : 13))
            return 0;
        queue[0] = GetMapFloorDef(gGameState.floor)->entryRoom;
        if (queue[0] >= 32) return 0;
        seen[queue[0]] = 1;
        /* The final Riku boss room is disconnected from the ordinary links,
           but is explicitly part of the floor definition. Include its graph. */
        neighbor = GetMapFloorDef(gGameState.floor)->exitRoom;
        if (neighbor < 32 && !seen[neighbor]) {
            seen[neighbor] = 1;
            queue[count++] = neighbor;
        }
        while (head < count) {
            room = queue[head++];
            for (side = 0; side < 4; side++) {
                neighbor = GetMapRoomLink(room, side);
                if (neighbor < 32 && !seen[neighbor]) {
                    seen[neighbor] = 1;
                    sides[neighbor] = side;
                    queue[count++] = neighbor;
                }
            }
            PortLog("harness: roomgraph floor=%u world=%u room=%d", (unsigned)gGameState.floor,
                    (unsigned)gMapFloorState.world, room);
        }
        if (strcmp(name, "roomgoto") == 0) {
            if (arg < 0 || arg >= 32 || !seen[arg]) return 0;
            CreateMapRoom(arg, NULL);
            SetCurrentMapRoom(arg, sides[arg]);
            RequestMapMode();
        }
        return 1;
    }

    /* Reproduce the MAP debug menu's entry, without bypassing map setup.
       arg = mode*10000 + world*100 + floor; roomriku selects Reverse/Rebirth.
       These are fixtures, not proof of a completed story playthrough. */
    if (strcmp(name, "room") == 0 || strcmp(name, "roomriku") == 0) {
        int mode = arg / 10000, world = (arg / 100) % 100, floor = arg % 100;
        int riku = strcmp(name, "roomriku") == 0;
        extern u8 gMapChkUseParams;
        extern void PortLog(const char* format, ...);
        if (arg < 0 || mode < 1 || mode > 3 || world > 12 || floor >= (riku ? 12 : 13))
            return 0;
        ResetGameState();
        if (riku) SetupRikuNewGame();
        else SetupSoraNewGame();
        InitMapCardInventory();
        gMapChkUseParams = 0;
        gGameState.progression.tutorialFlags |= TUTORIAL_FLAG_MAP_SELECT | TUTORIAL_FLAG_SAVE_POINT |
            TUTORIAL_FLAG_MAP | TUTORIAL_FLAG_MAP_SELECT_EVENT_DOOR | TUTORIAL_FLAG_WARP_POINT |
            TUTORIAL_FLAG_ROOM_CREATE | TUTORIAL_FLAG_MAP_STAIR;
        gGameState.floors[floor].world = mode == 2 ? 0 : gMapChkWorldNames[world][0];
        MapChkSetFloorProgress(floor, mode == 3);
        GoToFloor(floor);
        if (mode == 1) {
            SetFloorWorld(gMapChkWorldNames[world][0]);
            gMapFloorState.flags |= FLOOR_FLAG_LOGO_SHOWN;
            EnterFloorWorld();
        } else {
            gMapFloorState.room = mode == 3 ? MAP_ROOM_EXIT_HALL : MAP_ROOM_ENTRANCE_HALL;
            gMapFloorState.entrySide = MAP_DOOR_SIDE_NONE;
        }
        PortLog("harness: room fixture actor=%s mode=%d world=%d floor=%d room=%d",
                riku ? "riku" : "sora", mode, world, floor, gMapFloorState.room);
        RequestMapMode();
        return 1;
    }

    if (strcmp(name, "castle") == 0) {
        ResetGameState();
        SetupSoraNewGame();
        gMapFloorState.room = MAP_ROOM_ENTRANCE_HALL;
        ModeRequestHeapReset(&gModeMapFix, 0);
        return 1;
    }
    if (strcmp(name, "spotlight") == 0 && gFieldState != NULL) {
        SetBgPriority(0, 2);
        SetBgPriority(1, 2);
        TaskCreate(&gFieldState->tasks, &gTaskDescSpotLight, NULL);
        return 1;
    }
    if ((strcmp(name, "gmkcount") == 0 || strcmp(name, "crystalview") == 0) && gFieldState != NULL) {
        extern void PortLog(const char* format, ...);
        Task* task = ListPoolFirst(&gFieldState->tasks.head);
        int count = 0;
        while (task != NULL) {
            count += task->desc == &gTaskDescMapGmk04;
            if (task->desc == &gTaskDescMapGmk04 && strcmp(name, "crystalview") == 0) {
                MapGmk04Work* crystal = task->work;
                gFieldState->actor.fieldPosition = crystal->obj.fieldPosition;
                gFieldState->actor.fieldPosition.x -= 0x1200;
                gFieldState->actor.fieldPosition.y += 0x1C00;
                MapSetCameraTarget(gFieldState->actor.fieldPosition.x, gFieldState->actor.fieldPosition.y);
            }
            task = ListPoolNext(&task->node);
        }
        PortLog("harness: save crystals %d", count);
        return 1;
    }
    if (strcmp(name, "doorselect") == 0 && gFieldState != NULL) {
        static u8 status;
        gFieldState->flags |= FIELD_FLAG_ROOM_CREATE | FIELD_FLAG_FREEZE_PLAYER;
        MapFreezeBg1();
        CreateMapCardSelection(&gFieldState->tasks, &status);
        SetBgPriority(1, 1);
        return 1;
    }

    /* a save point beside Sora */
    if (strcmp(name, "crystal") == 0 && gFieldState != NULL) {
        static MapGmkPlacement sCrystal;
        sCrystal.pos = gFieldState->actor.fieldPosition;
        sCrystal.pos.x += (arg ? arg : 40) << 8;
        sCrystal.def = &gMapGmk04Def;
        PortMapKeepGmkDesc(&gTaskDescMapGmk04); /* kept like the room's own */
        TaskCreate(&gFieldState->tasks, &gTaskDescMapGmk04, &sCrystal);
        return 1;
    }
    /* the level-up screen, as the end of a battle opens it */
    /* the card-break mosaic over every layer (battle_runtime.c) */
    if (strcmp(name, "mosaic") == 0) {
        MosaicStartIn(16, 15);
        return 1;
    }
    /* battlewin: every enemy leaves as Oogie Boogie's helpers do, so the game
       ends the battle by its own route (white fade, rewards, back to the room) */
    if (strcmp(name, "battlewin") == 0 && gBtlWork != NULL
        && strcmp(PortHarnessModeName(), "mode_battle") == 0) {
        BtlObj* obj = ListPoolFirst(&gBtlWork->pool);
        gBtlWork->pendingEnemies = 0;
        while (obj != NULL) {
            obj->flags |= BTLOBJ_FLAG_WARP_PENDING;
            obj->hitFlags = 0;
            obj = ListPoolNext(&obj->node);
        }
        return 1;
    }
    /* battleend: the same closing sequence without a level up, for the exit */
    if ((strcmp(name, "levelup") == 0 || strcmp(name, "battleend") == 0) && gBtlWork != NULL) {
        /* The debug battle menu maxes every stat; restore selectable reward
           options for this fixture instead of indexing a nonexistent option. */
        gGameState.progression.maxHp = 80;
        gGameState.progression.cp = 275;
        gGameState.progression.levelMilestone = 0;
        gBtlWork->pendingLevelUps = strcmp(name, "levelup") == 0;
        /* Use the battle's closing sequence, so cards/camera and effects
           stop before their tile allocations are reused by the menu. */
        gBtlWork->phase = BTL_PHASE_END;
        gBtlWork->phaseStep = 0; /* BTL_END_STEP_CLOSE_CARDS, local to battle_runtime */
        gBtlWork->prizeCount = 0;
        /* Debug encounters still have live actors; prevent their animation
           uploads from touching tile allocations released for the reward UI. */
        gBtlWork->hitStop = 32767;
        gBtlWork->flags |= BTL_FLAG_BGFX_PAUSED;
        return 1;
    }
    /* the save point's menu, opened as talking to it does */
    if (strcmp(name, "mapsave") == 0 && gFieldState != NULL) {
        gFieldState->flags |= FIELD_FLAG_FREEZE_PLAYER;
        TaskCreate(&gFieldState->tasks, &gTaskDescMapSave, NULL);
        return 1;
    }
    for (i = 0; i < sizeof(modes) / sizeof(modes[0]); i++) {
        if (strcmp(modes[i]->name, name) == 0) {
            ModeRequest(modes[i], arg);
            return 1;
        }
    }
    return 0;
}
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
#else
    ModeInit();
#endif
#ifdef PLATFORM_PC
#ifdef VERSION_EU
    if (gPortStartupLanguage >= 0)
        gLanguage = gPortStartupLanguage;
#endif
    if (gPortIntroRequested) {
        gPortIntroRequested = 0;
        ResetGameState(); /* as the title screen does before a new game */
        SetupSoraNewGame();
        ModeRequestHeapReset(&gModeMovie, MOVIE_OPENING);
    }
    /* Test harness: play one of the ROM's movies (then what follows it). */
    if (gPortMovieTest >= 0) {
        ResetGameState();
        SetupSoraNewGame();
        ModeRequestHeapReset(&gModeMovie, gPortMovieTest);
        gPortMovieTest = -1;
    }
    /* Test harness: the debug battle menu, preset to one battle and stage
       (A starts it). */
    if (gPortBattleTest >= 0) {
        ResetGameState();
        if (gPortBattleTest & 0x10000)
            SetupRikuNewGame();
        else
            SetupSoraNewGame();
        gChkBtlWork->enemy = gPortBattleTest & 0xFF;
        gChkBtlWork->bg = (gPortBattleTest >> 8) & 0xFF;
        gChkBtlWork->hp = 560; /* the menu's maximum: tests reach the end of the battle */
        ModeRequestHeapReset(&gModeChkbtl, 0);
        gPortBattleTest = -1;
    }
    /* Test harness: the debug event menu, preset to one event (A starts it). */
    if (gPortEventTest >= 0) {
        ResetGameState();
        PortEventSelectPreset(gPortEventTest >> 8, gPortEventTest & 0xFF);
        ModeRequestHeapReset(&gModeEventselect, 0);
        gPortEventTest = -1;
    }
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
