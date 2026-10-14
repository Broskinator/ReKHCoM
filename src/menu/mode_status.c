/**
 * mode_status.c
 * Status Screen
 */

#include "registration_data.h"
#include "system_state.h"
#include "map_api.h"
#include "display.h"
#include "sprites_status.h"
#include "game_state.h"
#include "fade.h"
#include "mode.h"
#include <stddef.h>
#include "taskpool.h"
#include "types.h"
#include "copyright_screens.h"
#include "player_progression_types.h"

#ifdef PLATFORM_PC
#include <stdlib.h>
void PortRegisterBgTextTile(void* address, int code, int style, const void* source32);
void PortClearBgTextTiles(void);
void PortLog(const char* format, ...);
#endif

static TaskPool sStatusTaskPool;
static Task* sStatusBarTask;
static u8 sStatusReturnToMenu;

u8* CopyNumberTiles(u8* dst, s32 value, u16 digits) {
    u16 buf[6];
    s32 scale;
    s32 i;

    if (digits > 6) {
        digits = 6;
    }

    scale = 1;

    for (i = 0; i < digits; i++) {
        scale *= 10;
    }

    if (value > scale - 1) {
        value = scale - 1;
    } else if (value < 0) {
        value = 0;
    }

    for (i = 0; i < digits; i++) {
        buf[i] = (value % scale) / (scale / 10);
        scale /= 10;
    }

    for (i = 0; i < digits; i++) {
        RequestDma3Copy(&gStatusDigitTiles[buf[i] * 32], dst, 32);
#ifdef PLATFORM_PC
        PortRegisterBgTextTile(dst, '0' + buf[i], 2 | (1 << 8) | (7 << 12) | (8 << 16),
                               &gStatusDigitTiles[buf[i] * 32]);
        if (getenv("KHCOM_NUMERIC_TRACE"))
            PortLog("numeric-text: family=status byte=%u code=%u destination=%p size=32 style=%x",
                    (unsigned)buf[i] * 32, (unsigned)'0' + buf[i], dst,
                    2 | (1 << 8) | (7 << 12) | (8 << 16));
#endif
        dst += 32;
    }

    return dst;
}

void LoadStatusNumberTiles() {
    u8* dst;

    dst = GetBgCharBase(2) + 0x40;
    dst = CopyNumberTiles(dst, gGameState.progression.level, 2);
    dst = CopyNumberTiles(dst, gGameState.hp, 3);
    dst = CopyNumberTiles(dst, gGameState.progression.maxHp, 3);

    if (gGameState.flags & GAME_FLAG_RIKU) {
        dst += 0x80;
    } else {
        dst = CopyNumberTiles(dst, gGameState.progression.cp, 4);
    }

    dst = CopyNumberTiles(dst, gGameState.progression.exp, 6);
    dst = CopyNumberTiles(dst, gGameState.progression.nextExp, 6);

    if (gGameState.flags & GAME_FLAG_RIKU) {
        dst += 0xC0;
        dst = CopyNumberTiles(dst, gGameState.progression.ap, 2);
        CopyNumberTiles(dst, gGameState.progression.dp, 3);
    } else {
        dst += 0x20;
        CopyNumberTiles(dst, gGameState.progression.mooglePoints, 5);
    }
}

void mode_status_0() {
#ifdef PLATFORM_PC
    PortClearBgTextTiles();
#endif
    BgReset();
    SetBgMode0();
    SetupBg(0, 0, 0x1F, 0);
    SetupBg(2, 0, 0x1E, 0);
    SetupBg(3, 0, 0x1D, 0);
    SetBgPriority(0, 1);
    SetBgPriority(2, 2);
    SetBgPriority(3, 3);
#ifdef VERSION_EU
    LoadBgTiles(3, gStatusBgTiles, sizeof(gStatusBgTiles));

    switch (gLanguage) {
    case LANGUAGE_FRENCH:
        RequestDma3Copy(gStatusBgFrenchTiles, (u8*)GetBgCharBase(3) + 0x1800, sizeof(gStatusBgFrenchTiles));
        break;
    case LANGUAGE_GERMAN:
        // @bug Reads past the end of gStatusBgGermanTiles.
        RequestDma3Copy(gStatusBgGermanTiles, (u8*)GetBgCharBase(3) + 0x1800, 0xC00);
        break;
    case LANGUAGE_ITALIAN:
        RequestDma3Copy(gStatusBgItalianTiles, (u8*)GetBgCharBase(3) + 0x1800, sizeof(gStatusBgItalianTiles));
        break;
    case LANGUAGE_SPANISH:
        RequestDma3Copy(gStatusBgSpanishTiles, (u8*)GetBgCharBase(3) + 0x1800, sizeof(gStatusBgSpanishTiles));
        break;
    case LANGUAGE_ENGLISH:
    default:
        break;
    }
#else
    LoadBgTiles(3, gStatusBgTiles, sizeof(gStatusBgTiles));
#endif
    LoadBgPalette(3, gStatusBgPalette, sizeof(gStatusBgPalette));
    LoadBgMap(3, gStatusBgMap, sizeof(gStatusBgMap));

    if (gGameState.flags & GAME_FLAG_RIKU) {
        LoadBgMap(2, gStatusRikuBgMap, sizeof(gStatusRikuBgMap));
    } else {
        LoadBgMap(2, gStatusSoraBgMap, sizeof(gStatusSoraBgMap));
    }

    LoadBgMap(0, gStatusMesWindowMap, sizeof(gStatusMesWindowMap));
    DisableBg(0);
    LoadStatusNumberTiles();
    TaskPoolInit(&sStatusTaskPool, 4);
    sStatusBarTask = TaskCreate(&sStatusTaskPool, &gTaskDescStatusBar, NULL);
    TaskCreate(&sStatusTaskPool, &gTaskDescStatus, NULL);
    FadeStartIn(FADE_MODE_BLACK, 0x10);
}

void mode_status_1() {
    UpdatePlayTime();
    TaskPoolUpdate(&sStatusTaskPool);
    TaskPoolDraw(&sStatusTaskPool);

    if (!IsTaskActive(sStatusBarTask) && !FadeIsActive()) {
        ReturnToMap(sStatusReturnToMenu);
    }
}

void mode_status_2() {
#ifdef PLATFORM_PC
    PortClearBgTextTiles();
#endif
    TaskPoolDestroy(&sStatusTaskPool);
}

void SetStatusReturnToMenu(u8 returnToMenu) {
    sStatusReturnToMenu = returnToMenu;
}

Mode gModeStatus = {
    "mode_status",
    (ModeInitFunc)mode_status_0,
    mode_status_1,
    mode_status_2,
};
