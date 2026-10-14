/**
 * status.c
 * Status Screen Tasks
 */

#include "macros.h"
#include "registration_data.h"
#include "system_state.h"
#include "status_api.h"
#include "anim.h"
#include "status.h"
#include "gba/keys.h"
#include "sprites_fld.h"
#include "sprites_status.h"
#include "card_ids.h"
#include "player_progression.h"
#include "fade.h"
#include "songs.h"
#include "mode_status_api.h"
#include "card_api.h"
#include "card_def_data.h"
#include "card_label_data.h"
#include "card_types.h"
#include "card_ui_types.h"
#include "display.h"
#include "engine_math.h"
#include "game_state.h"
#include "key.h"
#include "m4a_song.h"
#include "obj.h"
#include "obj_api.h"
#include "player_progression_types.h"
#include "poo_api.h"
#include "taskpool.h"
#include "text.h"
#include "types.h"
#include <stddef.h>
#include "card_deckmenu2.h"
#include "card_stock_info.h"
#include "sprite_palettes.h"

TaskDesc gTaskDescStatus = {
    "task_status",
    (TaskInitFunc)task_status_0,
    (TaskUpdateFunc)task_status_1,
    (TaskDrawFunc)task_status_2,
    (TaskDestroyFunc)task_status_3,
    sizeof(StatusWork),
};

#ifdef VERSION_EU
static const u16 sStatusBarTileSizes[5] = {736, 608, 736, 640, 704};
#endif

#ifdef VERSION_EU
static void* sStatusBarTiles[5] = {
    gStatusBarTiles,
    gStatusBarFrenchTiles,
    gStatusBarTiles,
    gStatusBarItalianTiles,
    gStatusBarSpanishTiles,
};

static void** sStatusBarSprites[5] = {
    gStatusBarFrames,
    gStatusBarFrenchFrames,
    gStatusBarFrames,
    gStatusBarItalianFrames,
    gStatusBarSpanishFrames,
};
#endif

TaskDesc gTaskDescStatusBar = {
    "task_status_bar",
    (TaskInitFunc)task_status_bar_0,
    (TaskUpdateFunc)task_status_bar_1,
    (TaskDrawFunc)task_status_bar_2,
    (TaskDestroyFunc)task_status_bar_3,
    sizeof(StatusBarWork),
};

#ifdef VERSION_EU
static void* sStatusTabTiles[5] = {
    gStatusTabTiles,
    gStatusTabFrenchTiles,
    gStatusTabGermanSpanishTiles,
    gStatusTabItalianTiles,
    gStatusTabGermanSpanishTiles,
};

static void** sStatusTabSprites[5] = {
    gStatusTabFrames,
    gStatusTabFrenchFrames,
    gStatusTabGermanSpanishFrames,
    gStatusTabItalianFrames,
    gStatusTabGermanSpanishFrames,
};
#endif

static TaskDesc sTaskDescStatusTab = {
    "task_status_tab",
    (TaskInitFunc)task_status_tab_0,
    (TaskUpdateFunc)task_status_tab_1,
    (TaskDrawFunc)task_status_tab_2,
    (TaskDestroyFunc)task_status_tab_3,
    sizeof(StatusTabWork),
};

static TaskDesc sTaskDescStatusSora = {
    "task_status_sora",
    (TaskInitFunc)task_status_sora_0,
    (TaskUpdateFunc)task_status_sora_1,
    (TaskDrawFunc)task_status_sora_2,
    (TaskDestroyFunc)task_status_sora_3,
    sizeof(StatusSoraWork),
};

static TaskDesc sTaskDescStatusDeckname = {
    "task_status_deckname",
    (TaskInitFunc)task_status_deckname_0,
    (TaskUpdateFunc)task_status_deckname_1,
    (TaskDrawFunc)task_status_deckname_2,
    (TaskDestroyFunc)task_status_deckname_3,
    sizeof(StatusDecknameWork),
};

static const s32 sStatusTabCursorX[4] = {-1536, 2816, 6912, 10240};

static TaskDesc sTaskDescStatusCursor = {
    "task_status_cursor",
    (TaskInitFunc)task_status_cursor_0,
    (TaskUpdateFunc)task_status_cursor_1,
    (TaskDrawFunc)task_status_cursor_2,
    (TaskDestroyFunc)task_status_cursor_3,
    sizeof(StatusCursorWork),
};

#ifdef VERSION_EU
static const u16 sStatusNewMarkTileSizes[5] = {64, 128, 64, 128, 128};
#endif

#ifdef VERSION_EU
static void* sStatusNewMarkTiles[5] = {
    gStatusNewMarkEnglishTiles,
    gStatusNewMarkFrenchTiles,
    gStatusNewMarkGermanTiles,
    gStatusNewMarkItalianTiles,
    gStatusNewMarkSpanishTiles,
};

static void* sStatusNewMarkSprites[5] = {
    gStatusNewMarkEnglishFrame0,
    gStatusNewMarkFrenchFrame0,
    gStatusNewMarkGermanFrame0,
    gStatusNewMarkItalianFrame0,
    gStatusNewMarkSpanishFrame0,
};
#endif

static TaskDesc sTaskDescStatusStocklist = {
    "task_status_stocklist",
    (TaskInitFunc)task_status_stocklist_0,
    (TaskUpdateFunc)task_status_stocklist_1,
    (TaskDrawFunc)task_status_stocklist_2,
    (TaskDestroyFunc)task_status_stocklist_3,
    sizeof(StatusStocklistWork),
};

static TaskDesc sTaskDescStatusScrollcursor = {
    "task_status_scrollcursor",
    (TaskInitFunc)task_status_scrollcursor_0,
    (TaskUpdateFunc)task_status_scrollcursor_1,
    (TaskDrawFunc)task_status_scrollcursor_2,
    (TaskDestroyFunc)task_status_scrollcursor_3,
    sizeof(StatusScrollcursorWork),
};

static TaskDesc sTaskDescStatusMeswindow = {
    "task_status_meswindow",
    (TaskInitFunc)task_status_meswindow_0,
    (TaskUpdateFunc)task_status_meswindow_1,
    (TaskDrawFunc)task_status_meswindow_2,
    (TaskDestroyFunc)task_status_meswindow_3,
    sizeof(StatusMeswindowWork),
};

static TaskDesc sTaskDescStatusMessage = {
    "task_status_message",
    (TaskInitFunc)task_status_message_0,
    (TaskUpdateFunc)task_status_message_1,
    (TaskDrawFunc)task_status_message_2,
    (TaskDestroyFunc)task_status_message_3,
    sizeof(StatusMessageWork),
};

static TaskDesc sTaskDescStatusFriend = {
    "task_status_friend",
    (TaskInitFunc)task_status_friend_0,
    (TaskUpdateFunc)task_status_friend_1,
    (TaskDrawFunc)task_status_friend_2,
    (TaskDestroyFunc)task_status_friend_3,
    sizeof(StatusFriendWork),
};

static const StatusFriendTable sStatusFriendTable = {{
    {2, CARD_ID(CARD_DONALD_DUCK, 0)},
    {1, CARD_ID(CARD_GOOFY, 0)},
    {4, CARD_ID(CARD_ALADDIN, 0)},
    {8, CARD_ID(CARD_ARIEL, 0)},
    {16, CARD_ID(CARD_JACK, 0)},
    {32, CARD_ID(CARD_PETER_PAN, 0)},
    {64, CARD_ID(CARD_THE_BEAST, 0)},
    {128, CARD_ID(CARD_THE_KING, 0)},
}};

enum StatusBarState {
    STATUS_BAR_STATE_BARS_IN,
    STATUS_BAR_STATE_TITLE_IN,
    STATUS_BAR_STATE_IDLE,
    STATUS_BAR_STATE_TITLE_OUT,
    STATUS_BAR_STATE_BARS_OUT,
    STATUS_BAR_STATE_EXIT
};

static StatusWork* sStatusWork;
static u8 sStatusMesWindowOpen;
static s16 sStatusSelectedIndex;
static StatusStocklistWork* sStatusStocklistWork;

s32 gStatusBarState EWRAM_COMMON(4);

void task_status_0(StatusWork* work) {
    sStatusWork = work;
    work->tab = 0;
    sStatusMesWindowOpen = 0;
    work->cursor = 0;
    work->scroll = 0;
    TaskPoolInit(&work->pool, 9);
    TaskCreate(&work->pool, &sTaskDescStatusFriend, NULL);
    TaskCreate(&work->pool, &sTaskDescStatusSora, NULL);

    if (!(gGameState.flags & GAME_FLAG_RIKU)) {
        TaskCreate(&work->pool, &sTaskDescStatusTab, &work->tab);
        TaskCreate(&work->pool, &sTaskDescStatusDeckname, &sStatusMesWindowOpen);
    }

    TaskCreate(&work->pool, &sTaskDescStatusStocklist, &work->tab);
    TaskCreate(&work->pool, &sTaskDescStatusScrollcursor, &work->scroll);
    TaskCreate(&work->pool, &sTaskDescStatusMeswindow, &sStatusMesWindowOpen);

    if (GetStatusVisibleRowCount() == 0) {
        work->cursor = ~work->tab;
    } else {
        work->cursor = 0;
    }

    TaskCreate(&work->pool, &sTaskDescStatusCursor, &work->cursor);
    sStatusSelectedIndex = work->cursor + work->scroll;
}

void StatusHandleInput(StatusWork* work) {
    u16 keys;

    keys = GetKeysRepeat() & DPAD_UP;

    if (keys != 0) {
        if (work->cursor > 0) {
            work->cursor--;
            m4aSongNumStart(SONG_SYS_CLICKI04B);
        } else if (work->scroll > 0) {
            work->scroll--;
            StatusStocklistScrollUp();
            m4aSongNumStart(SONG_SYS_CLICKI04B);
        } else if (work->cursor == 0) {
            if (!(gGameState.flags & GAME_FLAG_RIKU)) {
                work->cursor = ~work->tab;
                m4aSongNumStart(SONG_SYS_CLICKI04B);
                sStatusMesWindowOpen = 0;
            }
        }
    } else if (GetKeysRepeat() & DPAD_DOWN) {
        if (work->cursor < GetStatusVisibleRowCount() - 1) {
            if (work->cursor >= 0) {
                work->cursor++;
                m4aSongNumStart(SONG_SYS_CLICKI04B);
            } else if (GetStatusVisibleRowCount() != 0) {
                work->cursor = 0;
                m4aSongNumStart(SONG_SYS_CLICKI04B);
            } else {
                m4aSongNumStart(SONG_SYS_BEEP);
            }
        } else if (work->scroll < GetStatusMaxScroll()) {
            work->scroll++;
            StatusStocklistScrollDown();
            m4aSongNumStart(SONG_SYS_CLICKI04B);
        }
    } else if ((GetKeysRepeat() & DPAD_LEFT) && !(gGameState.flags & GAME_FLAG_RIKU)) {
        if (work->tab != 0) {
            work->tab--;

            if (work->cursor < 0 || GetStatusVisibleRowCount() == 0) {
                work->cursor = ~work->tab;
                sStatusMesWindowOpen = 0;
            } else {
                work->cursor = 0;
            }

            work->scroll = 0;
            StatusStocklistLoadRows(0);
            m4aSongNumStart(SONG_SYS_CLICK);
        }
    } else if ((GetKeysRepeat() & DPAD_RIGHT) && !(gGameState.flags & GAME_FLAG_RIKU)) {
        if (work->tab <= 2) {
            work->tab++;

            if (work->cursor < 0 || GetStatusVisibleRowCount() == 0) {
                work->cursor = ~work->tab;
                sStatusMesWindowOpen = 0;
            } else {
                work->cursor = 0;
            }

            work->scroll = 0;
            StatusStocklistLoadRows(0);
            m4aSongNumStart(SONG_SYS_CLICK);
        }
    } else if (GetKeysPressed() & SELECT_BUTTON) {
        if (work->cursor >= 0) {
            work->cursor = ~work->tab;
            work->scroll = 0;
            StatusStocklistLoadRows(0);
            m4aSongNumStart(SONG_SYS_CLICKI04B);
#ifdef VERSION_EU
            sStatusMesWindowOpen = 0;
#endif
        }
    } else if ((GetKeysPressed() & A_BUTTON) && StatusTabHasItems() && sStatusMesWindowOpen == 0 && work->cursor >= 0) {
        sStatusMesWindowOpen = 1;
        m4aSongNumStart(SONG_SYS_KETTEI);
    } else if (sStatusMesWindowOpen != 0) {
        if ((GetKeysPressed() & B_BUTTON) || !StatusTabHasItems()) {
            sStatusMesWindowOpen = 0;
            m4aSongNumStart(SONG_SYS_CLOSE);
        }
    }
}

u8 task_status_1(StatusWork* work) {
    if (IsStatusBarIdle()) {
        StatusHandleInput(work);
    }

    sStatusSelectedIndex = work->cursor + work->scroll;
    TaskPoolUpdate(&work->pool);
    return 1;
}

void task_status_2(StatusWork* work) {
    TaskPoolDraw(&work->pool);
}

void task_status_3(StatusWork* work) {
    TaskPoolDestroy(&work->pool);
}

u8 IsStatusMesWindowOpen() {
    return sStatusMesWindowOpen;
}

s16 GetStatusSelectedIndex() {
    return sStatusSelectedIndex;
}

s16 GetStatusScroll() {
    return sStatusWork->scroll;
}

void StatusBarStartClose(StatusBarWork* work) {
    work->closing = TRUE;

    if (gStatusBarState == STATUS_BAR_STATE_BARS_IN) {
        gStatusBarState = STATUS_BAR_STATE_BARS_OUT;
    } else {
        gStatusBarState = STATUS_BAR_STATE_TITLE_OUT;
    }

    if (work->steps == 0) {
        work->steps = 16;
    }

    LoadBgMap(3, gStatusBgMap, sizeof(gStatusBgMap));
    work->targetY = -0x800;
    work->targetY2 = 0xA000;
    work->targetX = -0x8000;
}

void task_status_bar_0(StatusBarWork* work) {
#ifdef VERSION_EU
    work->tiles = LoadObjTiles(sStatusBarTiles[gLanguage], sStatusBarTileSizes[gLanguage]);
#else
    work->tiles = LoadObjTiles(gStatusBarTiles, sizeof(gStatusBarTiles));
#endif
    work->palette = LoadObjPalette(gStatusBarPalette, sizeof(gStatusBarPalette));
    work->steps = 16;
    gStatusBarState = STATUS_BAR_STATE_BARS_IN;
    work->y = -0x800;
    work->y2 = 0xA000;
    work->x = -0x8000;
    work->targetY = 0;
    work->targetY2 = 0x9800;
    work->targetX = 0;
    work->closing = FALSE;
    work->fadeStarted = FALSE;
}

u8 task_status_bar_1(StatusBarWork* work) {
    switch (gStatusBarState) {
    case STATUS_BAR_STATE_BARS_IN:
        ApproachValue(&work->y, work->targetY, work->steps);
        ApproachValue(&work->y2, work->targetY2, work->steps);
        work->steps--;

        if (work->steps == 0) {
            work->steps = 16;
            gStatusBarState = STATUS_BAR_STATE_TITLE_IN;
        }

        break;
    case STATUS_BAR_STATE_TITLE_IN:
        ApproachValue(&work->x, work->targetX, work->steps);
        work->steps--;

        if (work->steps == 0) {
            LoadBgMap(3, gStatusBarBgMap, sizeof(gStatusBarBgMap));
            gStatusBarState = STATUS_BAR_STATE_IDLE;
        }

        break;
    case STATUS_BAR_STATE_TITLE_OUT:
        ApproachValue(&work->x, work->targetX, work->steps);
        work->steps--;

        if (work->steps == 0) {
            work->steps = 16;
            gStatusBarState = STATUS_BAR_STATE_BARS_OUT;
        }

        break;
    case STATUS_BAR_STATE_BARS_OUT:
        if (!FadeIsActive() && !work->fadeStarted) {
            FadeStartOut(FADE_MODE_BLACK, 16);
            work->fadeStarted = TRUE;
        }

        ApproachValue(&work->y, work->targetY, work->steps);
        ApproachValue(&work->y2, work->targetY2, work->steps);
        work->steps--;

        if (work->steps == 0) {
            return 0;
        }

        break;
    case STATUS_BAR_STATE_IDLE:
        if (!work->closing) {
            if (GetKeysPressed() & START_BUTTON) {
                m4aSongNumStart(SONG_SYS_CLOSE);
                FadeStartOut(FADE_MODE_BLACK, 16);
                SetStatusReturnToMenu(FALSE);
                gStatusBarState = STATUS_BAR_STATE_EXIT;
            } else if (GetKeysPressed() & B_BUTTON) {
                if (!IsStatusMesWindowOpen()) {
                    m4aSongNumStart(SONG_SYS_CLOSE);
                    StatusBarStartClose(work);
                    SetStatusReturnToMenu(TRUE);
                }
            }
        }

        break;
    case STATUS_BAR_STATE_EXIT:
        if (!FadeIsActive()) {
            return 0;
        }

        break;
    }

    return 1;
}

void task_status_bar_2(StatusBarWork* work) {
#ifdef VERSION_EU
    DrawMenuHeader(work->x >> 8, 0, (sStatusBarSprites[gLanguage])[2], work->tiles,
        work->palette, NULL, SPRITE_PRIORITY(3), 29);
#else
    DrawMenuHeader(work->x >> 8, 0, gStatusBarFrame2, work->tiles, work->palette, NULL, SPRITE_PRIORITY(3), 29);
#endif

    if (gStatusBarState != STATUS_BAR_STATE_IDLE) {
#ifdef VERSION_EU
        DrawSprite(128, work->y >> 8, (sStatusBarSprites[gLanguage])[0], work->tiles,
            work->palette, NULL, SPRITE_PRIORITY(3), 30);
        DrawSprite(128, work->y2 >> 8, (sStatusBarSprites[gLanguage])[1], work->tiles,
            work->palette, NULL, SPRITE_PRIORITY(3), 31);
#else
        DrawSprite(128, work->y >> 8, gStatusBarFrame0, work->tiles, work->palette, NULL, SPRITE_PRIORITY(3), 30);
        DrawSprite(128, work->y2 >> 8, gStatusBarFrame1, work->tiles, work->palette, NULL, SPRITE_PRIORITY(3), 31);
#endif
    }
}

void task_status_bar_3(StatusBarWork* work) {
    ReleaseObjTiles(work->tiles);
    ReleaseObjPalette(work->palette);
}

u8 IsStatusBarIdle() {
    if (gStatusBarState == STATUS_BAR_STATE_IDLE) {
        return TRUE;
    }

    return FALSE;
}

void task_status_tab_0(StatusTabWork* work, s32* tab) {
    work->tab = tab;
#ifdef VERSION_EU
    work->tiles = AllocObjTiles(GetMaxSpriteTileBytes(sStatusTabSprites[gLanguage], 4),
        sStatusTabTiles[gLanguage]);
#else
    work->tiles = AllocObjTiles(GetMaxSpriteTileBytes(gStatusTabFrames, ARRAY_COUNT(gStatusTabFrames)), gStatusTabTiles);
#endif
    work->palette = LoadObjPalette(gStatusTabPalette, sizeof(gStatusTabPalette));
#ifdef VERSION_EU
    work->gfx = (sStatusTabSprites[gLanguage])[*work->tab];
#else
    work->gfx = gStatusTabFrames[*work->tab];
#endif
    work->tiles2 = AllocObjTiles(GetMaxSpriteTileBytes(gStatusTabRightFrames, ARRAY_COUNT(gStatusTabRightFrames)), gStatusTabRightTiles);
    work->palette2 = LoadObjPalette(gStatusTabRightPalette, sizeof(gStatusTabRightPalette));
    work->gfx2 = gStatusTabRightFrames[*work->tab];
}

u8 task_status_tab_1(StatusTabWork* work) {
#ifdef VERSION_EU
    work->gfx = (sStatusTabSprites[gLanguage])[*work->tab];
#else
    work->gfx = gStatusTabFrames[*work->tab];
#endif
    work->gfx2 = gStatusTabRightFrames[*work->tab];
    return 1;
}

void task_status_tab_2(StatusTabWork* work) {
    DrawMenuTab(0, 16, work->gfx, work->tiles, work->palette, NULL, SPRITE_PRIORITY(2), 10);
    DrawSprite(0, 16, work->gfx2, work->tiles2, work->palette2, NULL, SPRITE_PRIORITY(2), 11);
}

void task_status_tab_3(StatusTabWork* work) {
    ReleaseObjTiles(work->tiles);
    ReleaseObjPalette(work->palette);
    ReleaseObjTiles(work->tiles2);
    ReleaseObjPalette(work->palette2);
}

void task_status_sora_0(StatusSoraWork* work) {
    if (gGameState.flags & GAME_FLAG_RIKU) {
        work->tiles = AllocObjTiles(0x800, NULL);
        work->palette = LoadObjPalette(gRikuPalette, sizeof(gRikuPalette));
        SetObjTileSource(work->tiles, gRikuBt00Tiles);
        AnimInit(&work->anim, gRikuBt00Anims, gRikuBt00Frames);
    } else {
        work->tiles = AllocObjTiles(0x500, NULL);
        work->palette = LoadObjPalette(gSoraPalette, sizeof(gSoraPalette));
        SetObjTileSource(work->tiles, gSor1ll51Tiles);
        AnimInit(&work->anim, gSor1ll51Anims, gSor1ll51Frames);
    }

    AnimStart(&work->anim, 0, ANIM_FLAG_LOOP);
    work->gfx = AnimGetGfx(&work->anim);
}

u8 task_status_sora_1(StatusSoraWork* work) {
    work->gfx = AnimUpdate(&work->anim);
    return 1;
}

void task_status_sora_2(StatusSoraWork* work) {
    s16 x;
    s16 y;

    if (gGameState.flags & GAME_FLAG_RIKU) {
        x = 160;
        y = 65;
    } else {
        x = 140;
        y = 56;
    }

    DrawSprite(x, y, work->gfx, work->tiles, work->palette, NULL, SPRITE_PRIORITY(2), 12);
}

void task_status_sora_3(StatusSoraWork* work) {
    ReleaseObjTiles(work->tiles);
    ReleaseObjPalette(work->palette);
}

void task_status_deckname_0(StatusDecknameWork* work, u8* mesWindowOpen) {
    InitTextSlots(work->textSlots, ARRAY_COUNT(work->textSlots));
    work->mesWindowOpen = mesWindowOpen;
    work->textSlotCount = LoadTextSlots(GetDeckName(GetActiveDeckIndex()), work->textSlots);
    work->palette = LoadObjPalette(gStatusRowHighlightPalette, sizeof(gStatusRowHighlightPalette));
}

u8 task_status_deckname_1(StatusDecknameWork* work) {
    return 1;
}

void task_status_deckname_2(StatusDecknameWork* work) {
    if (*work->mesWindowOpen == 0) {
        DrawTextSlots(144, 142, work->textSlots, work->palette, 4, work->textSlotCount);
    }
}

void task_status_deckname_3(StatusDecknameWork* work) {
    FreeTextSlots(work->textSlots, ARRAY_COUNT(work->textSlots));
    ReleaseObjPalette(work->palette);
}

void task_status_cursor_0(StatusCursorWork* work, s16* cursor) {
    work->cursor = cursor;
    work->tiles = AllocObjTiles(GetMaxSpriteTileBytes(gStatusRowHighlightFrames, ARRAY_COUNT(gStatusRowHighlightFrames)), gStatusRowHighlightTiles);
    work->palette = LoadObjPalette(gStatusRowHighlightPalette, sizeof(gStatusRowHighlightPalette));
    AnimInit(&work->anim[0], gStatusRowHighlightAnims, gStatusRowHighlightFrames);
    AnimStart(&work->anim[0], 0, ANIM_FLAG_LOOP);
    work->gfx[0] = AnimGetGfx(&work->anim[0]);
    work->tiles2 = AllocObjTiles(GetMaxSpriteTileBytes(gStatusCursorFrames, 4), gStatusCursorTiles);
    work->palette2 = LoadObjPalette(gStatusCursorPalette, sizeof(gStatusCursorPalette));
    AnimInit(&work->anim[1], gStatusCursorAnims, gStatusCursorFrames);
    AnimStart(&work->anim[1], 0, ANIM_FLAG_LOOP);
    work->gfx[1] = AnimGetGfx(&work->anim[1]);
    work->lastCursor = *work->cursor;

    if (work->lastCursor < 0) {
        work->x = sStatusTabCursorX[~work->lastCursor];
        work->targetX = work->x;
        work->y = 0x1000;
        work->targetY = 0x1000;
    } else {
        work->x = 0x1800;
        work->targetX = 0x1800;
        work->y = *work->cursor * 3072 + 0x2400;
        work->targetY = work->y;
    }

    work->moveSteps = 0;
}

u8 task_status_cursor_1(StatusCursorWork* work) {
    s32 i;

    if (work->lastCursor != *work->cursor) {
        work->lastCursor = *work->cursor;
        work->moveSteps = 4;

        if (work->lastCursor < 0) {
            work->targetX = sStatusTabCursorX[~work->lastCursor];
            work->targetY = 0x1000;
        } else {
            work->targetX = 0x1800;
            work->targetY = *work->cursor * 3072 + 0x2400;
        }
    }

    ApproachValueHalf(&work->y, work->targetY);
    ApproachValueHalf(&work->x, work->targetX);

    for (i = 0; i < 2; i++) {
        work->gfx[i] = AnimUpdate(&work->anim[i]);
    }

    return 1;
}

void task_status_cursor_2(StatusCursorWork* work) {
    if (!FadeIsActive()) {
        if (!(gGameState.flags & GAME_FLAG_RIKU) || StatusTabHasItems()) {
            DrawSprite(work->x >> 8, (work->y >> 8) - 16, work->gfx[1], work->tiles2, work->palette2, NULL, 0, 0);

            if (work->lastCursor >= 0) {
                DrawSprite(1, (work->y >> 8) + 3, work->gfx[0], work->tiles, work->palette, NULL, 0, 1);
            }
        }
    }
}

void task_status_cursor_3(StatusCursorWork* work) {
    ReleaseObjTiles(work->tiles);
    ReleaseObjPalette(work->palette);
    ReleaseObjTiles(work->tiles2);
    ReleaseObjPalette(work->palette2);
}

void task_status_stocklist_0(StatusStocklistWork* work, s32* tab) {
    s32 i;
    StatusEntry* entry;

    sStatusStocklistWork = work;
    work->tab = tab;
    entry = work->entries;

    for (i = 0; i < 4; i++) {
        StatusEntryClear(entry);
        entry++;
    }

    if (gGameState.flags & GAME_FLAG_RIKU) {
        for (i = 66; i <= 69; i++) {
            if (IsStockLearned(i)) {
                StatusEntryAppend(work->entries, i);
            }
        }
    } else {
        for (i = 0; i <= 65; i++) {
            if (IsStockLearned(i)) {
                StatusEntryAppend(work->entries, i);
                StatusEntryAppend(&work->entries[GetStatusItemTab(i)], i);
            }
        }
    }

    for (i = 0; i < 8; i++) {
        work->tiles2[i] = NULL;
    }

    StatusStocklistLoadRows(0);
    work->palette = LoadObjPalette(gBStatesPalette, sizeof(gBStatesPalette));
#ifdef VERSION_EU
    work->tiles = LoadObjTiles(sStatusNewMarkTiles[gLanguage], sStatusNewMarkTileSizes[gLanguage]);
#else
    work->tiles = LoadObjTiles(gStatusNewMarkTiles, sizeof(gStatusNewMarkTiles));
#endif
    work->palette2 = LoadObjPalette(gStatusNewMarkPalette, sizeof(gStatusNewMarkPalette));
#ifdef VERSION_EU
    work->gfx = sStatusNewMarkSprites[gLanguage];
#else
    work->gfx = gStatusNewMarkFrame0;
#endif
    work->timer = 0;
    work->blink = FALSE;
}

u8 task_status_stocklist_1(StatusStocklistWork* work) {
    work->timer++;

    if (work->timer > 24) {
        work->blink = !work->blink ? TRUE : FALSE;
        work->timer = 0;
    }

    return 1;
}

void task_status_stocklist_2(StatusStocklistWork* work) {
    s32 i;
    s16 y;

    y = 36;

    for (i = 0; i < 8; i++) {
        if (work->tiles2[i] != NULL) {
            if (work->blink) {
                if (IsStockNew(GetStatusListItem(GetStatusScroll() + i))) {
                    DrawSprite(0, y, work->gfx, work->tiles, work->palette2, NULL, SPRITE_PRIORITY(2), i + 13);
                }
            }

            DrawSprite(1, y, NULL, work->tiles2[i], work->palette, NULL, SPRITE_PRIORITY(2), i + 21);
        }

        y += 12;
    }
}

void task_status_stocklist_3(StatusStocklistWork* work) {
    s32 i;

    for (i = 0; i < 8; i++) {
        if (work->tiles2[i] != NULL) {
            ReleaseObjTiles(work->tiles2[i]);
        }
    }

    ReleaseObjPalette(work->palette);
    ReleaseObjTiles(work->tiles);
    ReleaseObjPalette(work->palette2);
}

u16 GetStatusVisibleRowCount() {
    if (sStatusStocklistWork->entries[*sStatusStocklistWork->tab].count <= 7) {
        return sStatusStocklistWork->entries[*sStatusStocklistWork->tab].count;
    }

    return 8;
}

u16 GetStatusMaxScroll() {
    s16 maxScroll = sStatusStocklistWork->entries[*sStatusStocklistWork->tab].count - 8;

    if (maxScroll <= 0) {
        return 0;
    }

    return maxScroll;
}

u8 StatusTabHasItems() {
    if (sStatusStocklistWork->entries[*sStatusStocklistWork->tab].count == 0) {
        return FALSE;
    }

    return TRUE;
}

void StatusStocklistScrollDown() {
    s32 i;

    ReleaseObjTiles(sStatusStocklistWork->tiles2[0]);

    for (i = 0; i < 7; i++) {
        sStatusStocklistWork->tiles2[i] = sStatusStocklistWork->tiles2[i + 1];
    }

    sStatusStocklistWork->scroll++;
    sStatusStocklistWork->tiles2[7] = LoadStockNameTiles(GetStatusItemStockIndex(sStatusStocklistWork->entries[*sStatusStocklistWork->tab].items[sStatusStocklistWork->scroll + 7]));
}

void StatusStocklistScrollUp() {
    s32 i;

    ReleaseObjTiles(sStatusStocklistWork->tiles2[7]);

    for (i = 7; i > 0; i--) {
        sStatusStocklistWork->tiles2[i] = sStatusStocklistWork->tiles2[i - 1];
    }

    sStatusStocklistWork->scroll--;
    sStatusStocklistWork->tiles2[0] = LoadStockNameTiles(GetStatusItemStockIndex(sStatusStocklistWork->entries[*sStatusStocklistWork->tab].items[sStatusStocklistWork->scroll]));
}

void StatusStocklistLoadRows(u16 scroll) {
    s32 i;

    for (i = 0; i <= 7; i++) {
        if (sStatusStocklistWork->tiles2[i] != NULL) {
            ReleaseObjTiles(sStatusStocklistWork->tiles2[i]);
            sStatusStocklistWork->tiles2[i] = NULL;
        }
    }

    sStatusStocklistWork->scroll = scroll;

    for (i = 0; i < sStatusStocklistWork->entries[*sStatusStocklistWork->tab].count - scroll && i <= 7; i++) {
        sStatusStocklistWork->tiles2[i] = LoadStockNameTiles(GetStatusItemStockIndex(sStatusStocklistWork->entries[*sStatusStocklistWork->tab].items[scroll + i]));
    }
}

s32 GetStatusListItem(s16 index) {
    return sStatusStocklistWork->entries[*sStatusStocklistWork->tab].items[index];
}

void StatusEntryClear(StatusEntry* entry) {
    entry->count = 0;
}

void StatusEntryAppend(StatusEntry* entry, s32 item) {
    entry->items[entry->count] = item;
    entry->count++;
}

s32 GetStatusItemTab(u32 item) {
    if (item <= 8) {
        return 1;
    }

    if (item >= 9 && item <= 46) {
        return 2;
    }

    return 3;
}

void* LoadStockNameTiles(u16 stock) {
    const SpriteFrameResourceDef* def;
    void* tiles;

    def = &gStockNameSprites[stock];
    tiles = AllocSpriteFrameTiles(def->tilesSize);
#ifdef VERSION_EU
    UpdateSpriteFrameTiles(tiles, def->sprites[gLanguage][def->spriteIndex],
        def->tiles[gLanguage]);
#else
    UpdateSpriteFrameTiles(tiles, def->sprites[def->spriteIndex], def->tiles);
#endif
    return tiles;
}

s32 GetStatusItemStockIndex(s32 item) {
    switch (item) {
    case LEARNED_STOCK_BLITZ:
        return STOCK_BLITZ;
    case LEARNED_STOCK_SONIC_BLADE:
        return STOCK_SONIC_BLADE;
    case LEARNED_STOCK_ARS_ARCANUM:
        return STOCK_ARS_ARCANUM;
    case LEARNED_STOCK_STRIKE_RAID:
        return STOCK_STRIKE_RAID;
    case LEARNED_STOCK_RAGNAROK:
        return STOCK_RAGNAROK;
    case LEARNED_STOCK_TRINITY_LIMIT:
        return STOCK_TRINITY_LIMIT;
    case LEARNED_STOCK_SLIDING_DASH:
        return STOCK_SLIDING_DASH;
    case LEARNED_STOCK_STUN_IMPACT:
        return STOCK_STUN_IMPACT;
    case LEARNED_STOCK_ZANTETSUKEN:
        return STOCK_ZANTETSUKEN;
    case LEARNED_STOCK_WARP:
        return STOCK_WARP;
    case LEARNED_STOCK_WARPINATOR:
        return STOCK_WARPINATOR;
    case LEARNED_STOCK_TERROR:
        return STOCK_TERROR;
    case LEARNED_STOCK_CONFUSE:
        return STOCK_CONFUSE;
    case LEARNED_STOCK_SLEIGHT_57:
        return STOCK_SLEIGHT_57;
    case LEARNED_STOCK_STOP_RAID:
        return STOCK_STOP_RAID;
    case LEARNED_STOCK_JUDGMENT:
        return STOCK_JUDGMENT;
    case LEARNED_STOCK_REFLECT_RAID:
        return STOCK_REFLECT_RAID;
    case LEARNED_STOCK_FIRE_RAID:
        return STOCK_FIRE_RAID;
    case LEARNED_STOCK_BLIZZARD_RAID:
        return STOCK_BLIZZARD_RAID;
    case LEARNED_STOCK_THUNDER_RAID:
        return STOCK_THUNDER_RAID;
    case LEARNED_STOCK_GRAVITY_RAID:
        return STOCK_GRAVITY_RAID;
    case LEARNED_STOCK_AQUA_SPLASH:
        return STOCK_AQUA_SPLASH;
    case LEARNED_STOCK_HOLY:
        return STOCK_HOLY;
    case LEARNED_STOCK_BLAZING_DONALD:
        return STOCK_BLAZING_DONALD;
    case LEARNED_STOCK_SLEIGHT_68:
        return STOCK_SLEIGHT_68;
    case LEARNED_STOCK_GIFTED_MIRACLE:
        return STOCK_GIFTED_MIRACLE;
    case LEARNED_STOCK_MEGA_FLARE:
        return STOCK_MEGA_FLARE;
    case LEARNED_STOCK_FIRAGA_BREAK:
        return STOCK_FIRAGA_BREAK;
    case LEARNED_STOCK_SHOCK_IMPACT:
        return STOCK_SHOCK_IMPACT;
    case LEARNED_STOCK_IDYLL_ROMP:
        return STOCK_IDYLL_ROMP;
    case LEARNED_STOCK_CROSS_SLASH_PLUS:
        return STOCK_CROSS_SLASH_PLUS;
    case LEARNED_STOCK_HOMING_FIRA:
        return STOCK_HOMING_FIRA;
    case LEARNED_STOCK_HOMING_BLIZZARA:
        return STOCK_HOMING_BLIZZARA;
    case LEARNED_STOCK_SYNCHRO:
        return STOCK_SYNCHRO;
    case LEARNED_STOCK_BIND:
        return STOCK_BIND;
    case LEARNED_STOCK_TORNADO:
        return STOCK_TORNADO;
    case LEARNED_STOCK_QUAKE:
        return STOCK_QUAKE;
    case LEARNED_STOCK_TELEPORT:
        return STOCK_TELEPORT;
    case LEARNED_STOCK_FIRA:
        return STOCK_FIRA;
    case LEARNED_STOCK_BLIZZARA:
        return STOCK_BLIZZARA;
    case LEARNED_STOCK_THUNDARA:
        return STOCK_THUNDARA;
    case LEARNED_STOCK_CURA:
        return STOCK_CURA;
    case LEARNED_STOCK_GRAVIRA:
        return STOCK_GRAVIRA;
    case LEARNED_STOCK_STOPRA:
        return STOCK_STOPRA;
    case LEARNED_STOCK_AERORA:
        return STOCK_AERORA;
    case LEARNED_STOCK_FIRAGA:
        return STOCK_FIRAGA;
    case LEARNED_STOCK_BLIZZAGA:
        return STOCK_BLIZZAGA;
    case LEARNED_STOCK_THUNDAGA:
        return STOCK_THUNDAGA;
    case LEARNED_STOCK_CURAGA:
        return STOCK_CURAGA;
    case LEARNED_STOCK_GRAVIGA:
        return STOCK_GRAVIGA;
    case LEARNED_STOCK_STOPGA:
        return STOCK_STOPGA;
    case LEARNED_STOCK_AEROGA:
        return STOCK_AEROGA;
    case LEARNED_STOCK_PROUD_ROAR:
        return STOCK_PROUD_ROAR;
    case LEARNED_STOCK_SHOWTIME:
        return STOCK_SHOWTIME;
    case LEARNED_STOCK_TWINKLE:
        return STOCK_TWINKLE;
    case LEARNED_STOCK_FLARE_BREATH:
        return STOCK_FLARE_BREATH;
    case LEARNED_STOCK_OMNISLASH:
        return STOCK_OMNISLASH;
    case LEARNED_STOCK_PARADISE:
        return STOCK_PARADISE;
    case LEARNED_STOCK_SPLASH:
        return STOCK_SPLASH;
    case LEARNED_STOCK_MAGIC:
        return STOCK_MAGIC;
    case LEARNED_STOCK_GOOFY_CHARGE:
        return STOCK_GOOFY_CHARGE;
    case LEARNED_STOCK_GOOFY_TORNADO:
        return STOCK_GOOFY_TORNADO;
    case LEARNED_STOCK_SANDSTORM:
        return STOCK_SANDSTORM;
    case LEARNED_STOCK_SURPRISE:
        return STOCK_SURPRISE;
    case LEARNED_STOCK_SPIRAL_WAVE:
        return STOCK_SPIRAL_WAVE;
    case LEARNED_STOCK_HUMMINGBIRD:
        return STOCK_HUMMINGBIRD;
    case LEARNED_STOCK_FEROCIOUS_LUNGE:
        return STOCK_FEROCIOUS_LUNGE;
    case LEARNED_STOCK_DARK_BREAK:
        return STOCK_DARK_BREAK;
    case LEARNED_STOCK_DARK_FIRAGA:
        return STOCK_DARK_FIRAGA;
    case LEARNED_STOCK_DARK_AURA:
        return STOCK_DARK_AURA;
    case LEARNED_STOCK_CROSS_SLASH:
        return STOCK_CROSS_SLASH;
    case LEARNED_STOCK_MM_MIRACLE:
        return STOCK_MM_MIRACLE;
    case 72:
        return 0xFFFF;
    }
}

s16 GetStatusScrollcursorY(StatusScrollcursorWork* work) {
    if (GetStatusMaxScroll() == 0) {
        return 40;
    }

    return *work->scroll * 84 / GetStatusMaxScroll() + 40;
}

void task_status_scrollcursor_0(StatusScrollcursorWork* work, u16* scroll) {
    work->scroll = scroll;
    work->tiles = AllocObjTiles(GetMaxSpriteTileBytes(gStatusCursorFrames, 4), gStatusCursorTiles);
    work->palette = LoadObjPalette(gStatusCursorPalette, sizeof(gStatusCursorPalette));
    work->gfx = gStatusCursorFrames[4];
    work->y = GetStatusScrollcursorY(work);
}

u8 task_status_scrollcursor_1(StatusScrollcursorWork* work) {
    work->y = GetStatusScrollcursorY(work);
    return 1;
}

void task_status_scrollcursor_2(StatusScrollcursorWork* work) {
    if (StatusTabHasItems()) {
        DrawSprite(84, work->y, work->gfx, work->tiles, work->palette, NULL, SPRITE_PRIORITY(2), 6);
    }
}

void task_status_scrollcursor_3(StatusScrollcursorWork* work) {
    ReleaseObjTiles(work->tiles);
    ReleaseObjPalette(work->palette);
}

void task_status_meswindow_0(StatusMeswindowWork* work, u8* open) {
    work->open = open;
    work->item = 72;
    TaskPoolInit(&work->pool, 2);
    work->task = NULL;
    work->textIndex = 0;
}

u8 task_status_meswindow_1(StatusMeswindowWork* work) {
    s32 item;
    s16 idx;

    if (*work->open != 0) {
        idx = GetStatusSelectedIndex();

        if (idx >= 0) {
            item = GetStatusListItem(GetStatusSelectedIndex());

            if (work->item != item) {
                work->item = item;

                if (work->task != NULL) {
                    work->textIndex = GetStockMesDispTextIndex(work->task);
                    TaskKill(&work->pool, work->task);
                }

                work->task = CreateStockMesDispTask(&work->pool, GetStatusItemStockIndex(work->item), work->textIndex, 88, 98);
                ClearStockNew(work->item);
            }

            TaskPoolUpdate(&work->pool);
        }
    }

    return 1;
}

void task_status_meswindow_2(StatusMeswindowWork* work) {
    if (*work->open == 0) {
        DisableBg(0);
    } else {
        EnableBg(0);
        TaskPoolDraw(&work->pool);
    }
}

void task_status_meswindow_3(StatusMeswindowWork* work) {
    TaskPoolDestroy(&work->pool);
}

void task_status_message_0(StatusMessageWork* work, StatusMessageParam* arg) {
    InitTextSlots(work->textSlots, ARRAY_COUNT(work->textSlots));
    work->param = *arg;
    work->textSlotCount = LoadTextSlots(work->param.text, work->textSlots);
    work->palette = LoadObjPalette(gStatusBarPalette, sizeof(gStatusBarPalette));
}

u8 task_status_message_1(StatusMessageWork* work) {
    return 1;
}

void task_status_message_2(StatusMessageWork* work) {
    DrawTextSlots(work->param.x, work->param.y, work->textSlots, work->palette, 3, work->textSlotCount);
}

void task_status_message_3(StatusMessageWork* work) {
    FreeTextSlots(work->textSlots, ARRAY_COUNT(work->textSlots));
    ReleaseObjPalette(work->palette);
}

Task* CreateStatusMessageTask(void* pool, s16 x, s16 y, void* text) {
    StatusMessageParam param;

    param.x = x;
    param.y = y;
    param.text = text;
    return TaskCreate(pool, &sTaskDescStatusMessage, &param);
}

void task_status_friend_0(StatusFriendWork* work) {
    work->count = LoadFriendCardSprites(work->tiles, work->palette, work->gfx);
}

u8 task_status_friend_1(StatusFriendWork* work) {
    return 1;
}

void task_status_friend_2(StatusFriendWork* work) {
    s32 i;
    s16 x;

    x = (gGameState.flags & GAME_FLAG_RIKU) ? 216 : 186;

    for (i = 0; i < work->count; i++) {
        DrawSprite(x, 45, work->gfx[i], work->tiles[i], work->palette[i], NULL, SPRITE_PRIORITY(2), i + 7);
        x += 20;
    }
}

void task_status_friend_3(StatusFriendWork* work) {
    s32 i;

    for (i = 0; i < work->count; i++) {
        ReleaseObjTiles(work->tiles[i]);
        ReleaseObjPalette(work->palette[i]);
    }
}

u16 LoadFriendCardSprites(void** tiles, void** palettes, void** gfx) {
    StatusFriendTable table;
    const CardDef* card;
    const void* data;
    u16 count;
    u16 index;
    u16 limit;
    const void* source;

    source = &sStatusFriendTable;
    table = *(const StatusFriendTable*)source;

    data = &gGameState;

    if (((const GameState*)data)->flags & GAME_FLAG_RIKU) {
        limit = 1;
    } else {
        limit = 3;
    }

    count = 0;

    for (index = 0; index < ARRAY_COUNT(table.entries); index++) {
        data = &table.entries[index];
        source = &gGameState.progression.friendFlags;

        if (*(const u16*)source & ((const StatusFriendEntry*)data)->flag) {
            card = &gCardDefs[((const StatusFriendEntry*)data)->cardId];
            tiles[count] = LoadObjTiles(card->tiles2, 0x100);
            palettes[count] = LoadObjPalette(card->palette2, 0x20);
            gfx[count] = card->gfx2;
            count++;

            if (count >= limit) {
                break;
            }
        }
    }

    return count;
}

void stock_mes_disp_0(StockMesDispWork* work, StatusMesParam* arg) {
    gStockMesDispWork = work;
    *(StatusMesParam*)&work->x = *arg;
    work->textCount = GetCardHelpTextCount(work->helpIndex);

    if (work->textIndex >= work->textCount - 1) {
        work->textIndex = work->textCount - 1;
    }

    work->tiles = LoadStockNameTiles(work->helpIndex);
    work->palette = LoadObjPalette(gBStatesPalette, sizeof(gBStatesPalette));
    TaskPoolInit(&work->tasks, 1);
    work->task = CreateStatusMessageTask(&work->tasks, work->x + 6, work->y + 16,
                                        GetCardHelpText(work->helpIndex, work->textIndex));
    work->tiles2 = AllocObjTiles(GetMaxSpriteTileBytes(gStockMesDispArrowFrames, ARRAY_COUNT(gStockMesDispArrowFrames)), gStockMesDispArrowTiles);
    work->palette2 = LoadObjPalette(gStockMesDispArrowPalette, sizeof(gStockMesDispArrowPalette));
    work->gfx = gStockMesDispArrowFrames[0];
    work->tiles3 = AllocObjTiles(GetMaxSpriteTileBytes(gStockMesDispArrowFrames, ARRAY_COUNT(gStockMesDispArrowFrames)), gStockMesDispArrowTiles);
    work->palette3 = LoadObjPalette(gStockMesDispArrowPalette, sizeof(gStockMesDispArrowPalette));
    work->gfx2 = gStockMesDispArrowFrames[1];
    work->frame = 0;
}

u8 stock_mes_disp_1(StockMesDispWork* work) {
    u8 changed = FALSE;

    if (GetKeysPressed() & R_BUTTON) {
        if (work->textIndex < work->textCount - 1) {
            work->textIndex++;
            changed = TRUE;
        }
    } else if (GetKeysPressed() & L_BUTTON) {
        if (work->textIndex != 0) {
            work->textIndex--;
            changed = TRUE;
        }
    }

    if (changed) {
        m4aSongNumStart(SONG_SYS_CANSEL);
        TaskKill(&work->tasks, work->task);
        work->task = CreateStatusMessageTask(&work->tasks, work->x + 6, work->y + 16, GetCardHelpText(work->helpIndex, work->textIndex));
    }

    TaskPoolUpdate(&work->tasks);
    return 1;
}

void stock_mes_disp_2(StockMesDispWork* work) {
    DrawSprite(work->x + 14, work->y - 4, NULL, work->tiles, work->palette, NULL, 0, 5);

    if (work->textIndex != 0) {
        DrawSprite(work->x - (work->frame / 8) % 4, work->y, work->gfx, work->tiles2, work->palette2, NULL, 0, 2);
    }

    if (work->textIndex < work->textCount - 1) {
        DrawSprite(work->x + ((work->frame / 8) % 4 + 136), work->y, work->gfx2, work->tiles3, work->palette3, NULL, 0, 3);
    }

    TaskPoolDraw(&work->tasks);
    work->frame++;
}

void stock_mes_disp_3(StockMesDispWork* work) {
    ReleaseObjTiles(work->tiles);
    ReleaseObjPalette(work->palette);
    ReleaseObjTiles(work->tiles2);
    ReleaseObjPalette(work->palette2);
    ReleaseObjTiles(work->tiles3);
    ReleaseObjPalette(work->palette3);
    TaskPoolDestroy(&work->tasks);
}

void* CreateStockMesDispTask(void* pool, u16 helpIndex, u8 textIndex, u16 x, s32 y) {
    StatusMesParam param;

    param.helpIndex = helpIndex;
    param.textIndex = textIndex;
    param.x = x;
    param.y = y;
    return TaskCreate(pool, &gTaskDescStockMesDisp, &param);
}

u8 GetStockMesDispTextIndex(void* task) {
    return ((StockMesDispWork*)gStockMesDispWork)->textIndex;
}

TaskDesc gTaskDescStockMesDisp = {
    "stock_mes_disp",
    (TaskInitFunc)stock_mes_disp_0,
    (TaskUpdateFunc)stock_mes_disp_1,
    (TaskDrawFunc)stock_mes_disp_2,
    (TaskDestroyFunc)stock_mes_disp_3,
    sizeof(StockMesDispWork),
};
