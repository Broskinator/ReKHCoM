/**
 * card_deckexchange.c
 * Link Card Trade Screen
 */

#include "macros.h"
#include "registration_data.h"
#include "m4a_song.h"
#include "game_state.h"
#include "text.h"
#include "fade.h"
#include "obj_api.h"
#include "display.h"
#include "engine_math.h"
#include "listpool.h"
#include "anim.h"
#include "obj.h"
#include "taskpool.h"
#include "key.h"
#include "malloc.h"
#include "card.h"
#include "sprites_deck_menu.h"
#include "gba/keys.h"
#include "songs.h"
#include "card_api.h"
#include "card_def_data.h"
#include "card_types.h"
#include "gba/defines.h"
#include "player_progression_types.h"
#include "types.h"
#include "gba/macro.h"
#include <stddef.h>
#include "card_deckexchange.h"
#include "card_map_anim.h"
#include "card_deckmenu2.h"
#include "default_bg_map.h"
#include "sprite_palettes.h"
#include "card_ids.h"

#ifndef VERSION_EU
u16 gSioTradeCardId EWRAM_COMMON(4);
#endif

#ifndef VERSION_EU
static const s16 sDeckExchangeTabPointerX[3] = { 116, 116, 116 };

static const s16 sDeckExchangeTabPointerY[3] = { 56, 104, 148 };

const s16 gUnk_09041F10[5] = { 12, 28, 42, 56, 70 };

static const s16 sDeckExchangeFilterTabX[6] = { 172, 172, 188, 202, 216, 230 };

const s16 gUnk_09041F26[5] = { 64, 82, 100, 118, 136 };

static const s16 sDeckExchangeValueGridX[2] = { 80, 128 };

static const s16 sDeckExchangeValueGridY[5] = { 80, 88, 96, 104, 112 };

static const u16 sDeckExchangeRowY[4] = { 45, 93, 141, 30 };

void deckexchange_0(DeckExchangeWork* work, void* resultOut) {
    u16 n;

    CpuFill32(0, work, sizeof(DeckExchangeWork));
    work->tiles7 = NULL;
    work->tiles4 = NULL;
    work->tiles5 = NULL;
    work->tiles6 = NULL;
    work->palette2 = NULL;
    work->palette3 = NULL;
    work->tiles8 = NULL;
    work->palette6 = NULL;
    work->palette7 = NULL;
    work->palette4 = NULL;
    work->cursorCard = NULL;
    work->prevCursorCard = NULL;
    work->entries = NULL;
    work->resultOut = resultOut;
    SetBgMode0();
    SetBackdropColor(0, 0, 0);
    SetupBg(0, 3, 31, 0);
    SetupBg(1, 2, 23, 0);
    SetupBg(2, 1, 15, 0);
    SetupBg(3, 0, 30, 0);
    SetBgPriority(0, 0);
    SetBgPriority(1, 1);
    SetBgPriority(2, 2);
    FadeStartIn(FADE_MODE_BLACK, 16);
    ListPoolInit(&work->pool);
    TaskPoolInit(&work->tasks, 99);
    TaskPoolInit(&work->tasks2, 1);
    work->deckIndex = GetActiveDeckIndex();
    CreateDeckExchangeDeckGridCards(work, 0);
    work->tiles = AllocObjTiles(0x120, NULL);
    SetObjTileSource(work->tiles, gHandCursorTiles);
    AnimInit(&work->anim, gHandCursorAnims, gHandCursorFrames);
    AnimStart(&work->anim, 0, ANIM_FLAG_LOOP);
    work->gfx = AnimGetGfx(&work->anim);
    work->handX = sDeckExchangeTabPointerX[0] << 8;
    work->handY = sDeckExchangeTabPointerY[0] << 8;
    work->handFlags = 0;
    work->tiles3 = LoadObjTiles(gDeckScrollThumbTiles, sizeof(gDeckScrollThumbTiles));
    work->palette = LoadObjPalette(gDialogBoxPalette, sizeof(gDialogBoxPalette));
    work->tiles2 = AllocObjTiles(0x280, NULL);
    SetDeckExchangeFrameCursor(work, DECK_FRAME_CURSOR_CARD);
    work->palette4 = LoadObjPalette(gDeckMenuTextPalette, sizeof(gDeckMenuTextPalette));
    work->step = 0;
    work->cursorCol = 0;
    work->cursorRow = 0;
    work->prevCursorCol = 0;
    work->prevCursorRow = 0;
    work->timer = 4;
    work->commandCursor = 0;
    work->mode = DECK_MENU_MODE_NONE;
    work->view = DECK_MENU_VIEW_DECK_GRID;
    work->deckAttackCount = CountActiveDeckCardsOfCategory(CARD_CATEGORY_ATTACK);
    work->deckMagicCount = CountActiveDeckCardsOfCategory(CARD_CATEGORY_MAGIC);
    work->deckItemCount = CountActiveDeckCardsOfCategory(CARD_CATEGORY_ITEM);
    work->deckEnemyCount = CountActiveDeckCardsOfCategory(CARD_CATEGORY_ENEMY);
    work->categoryFilter = CATEGORY_FILTER_DECK_ALL;
    work->entryCount = 0;
    work->popupActive = 0;
    work->exitRequested = FALSE;
    work->barSlideTimer = 16;
    work->bannerSlideTimer = 16;
    work->topBarX = 0;
    work->topBarY = -0x800;
    work->bottomBarX = 0;
    work->bottomBarY = 0xA000;
    work->bannerX = -0x8000;
    work->holding = FALSE;
    work->descriptionX = 8;
    work->descriptionY = 113;
    work->textSlotCount5 = 0;
    work->unk_6C4 = 79;
    n = sDeckExchangeRowY[work->deckIndex];
    work->unk_6C6 = n;
    work->unk_6C8 = 225;
    n = sDeckExchangeRowY[work->deckIndex];
    work->unk_6CA = n;
    work->unk_70F = 0;
    work->inputDelay = 0;
    work->textSlotCount = 0;
    work->textSlotCount2 = 0;
    work->textSlotCount3 = 0;
    work->textSlotCount4 = 0;
    InitTextSlots(work->textSlots, ARRAY_COUNT(work->textSlots));
    InitTextSlots(work->textSlots2, ARRAY_COUNT(work->textSlots2));
    InitTextSlots(work->textSlots3, ARRAY_COUNT(work->textSlots3));
    InitTextSlots(work->textSlots4, ARRAY_COUNT(work->textSlots4));
    InitTextSlots(work->textSlots5, ARRAY_COUNT(work->textSlots5));
}

u8 deckexchange_1(DeckExchangeWork* work, void* task) {
    FadeStartIn(FADE_MODE_BLACK, 16);

    switch (work->step) {
    case 0:
        RequestDma3Clear(GetBgCharBase(0), 0x1000);
        break;
    case 1:
        RequestDma3Clear(GetBgCharBase(0) + 0x1000, 0x1000);
        break;
    case 2:
        RequestDma3Clear(GetBgCharBase(0) + 0x2000, 0x1000);
        break;
    case 3:
        RequestDma3Clear(GetBgCharBase(0) + 0x3000, 0x1000);
        break;
    case 4:
        RequestDma3Clear(GetBgCharBase(1), 0x1000);
        break;
    case 5:
        RequestDma3Clear(GetBgCharBase(1) + 0x1000, 0x1000);
        break;
    case 6:
        RequestDma3Clear(GetBgCharBase(1) + 0x2000, 0x1000);
        break;
    case 7:
        RequestDma3Clear(GetBgCharBase(1) + 0x3000, 0x1000);
        break;
    case 8:
        RequestDma3Clear(GetBgCharBase(2), 0x1000);
        break;
    case 9:
        RequestDma3Clear(GetBgCharBase(2) + 0x1000, 0x1000);
        break;
    case 10:
        RequestDma3Clear(GetBgCharBase(2) + 0x2000, 0x1000);
        break;
    case 11:
        RequestDma3Clear(GetBgCharBase(2) + 0x3000, 0x1000);
        break;
    case 12:
        RequestDma3Clear(GetBgCharBase(3), 0x1000);
        break;
    case 13:
        RequestDma3Clear(GetBgCharBase(3) + 0x1000, 0x1000);
        break;
    case 14:
        RequestDma3Clear(GetBgCharBase(3) + 0x2000, 0x1000);
        break;
    case 15:
        RequestDma3Clear(GetBgCharBase(3) + 0x3000, 0x1000);
        work->step = 0;
        SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeLoadBgs);
        return 1;
    }

    work->step++;
    // @bug No return value (the caller reads it).
}

u8 UpdateDeckExchangeLoadBgs(DeckExchangeWork* work, void* task) {
    FadeStartIn(FADE_MODE_BLACK, 16);

    switch (work->step) {
    case 0:
        LoadBgTiles(3, gDeckMenuTiles, 0x2000);
        break;
    case 1:
        RequestDma3Copy(&gDeckMenuTiles[0x2000],
                        (u8*)GetBgCharBase(3) + 0x2000, 0x2000);
        break;
    case 2:
        LoadBgPalette(3, gDeckMenuPalettes, sizeof(gDeckMenuPalettes));
        break;
    case 3:
        LoadBgTiles(0, gDeck1PanelTiles, sizeof(gDeck1PanelTiles));
        break;
    case 4:
        LoadBgMap(0, gDefaultBgMap, sizeof(gDefaultBgMap));
        break;
    case 5:
        LoadBgTiles(1, gDeck2PanelTiles, 0x2000);
        break;
    case 6:
        RequestDma3Copy(&gDeck2PanelTiles[0x2000],
                        (u8*)GetBgCharBase(1) + 0x2000, 0x1E20);
        break;
    case 7:
        LoadBgMap(1, gDefaultBgMap, sizeof(gDefaultBgMap));
        break;
    case 8:
        LoadBgTiles(2, gDeck3PanelTiles, 0x2000);
        break;
    case 9:
        RequestDma3Copy(&gDeck3PanelTiles[0x2000],
                        (u8*)GetBgCharBase(2) + 0x2000, 0x1E20);
        break;
    case 10:
        LoadBgMap(2, gDefaultBgMap, sizeof(gDefaultBgMap));
        break;
    case 12:
        work->step = 0;
        SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeLoadDeckInfo);
        return 1;
    }

    work->step++;
    SetBgScroll(0, (u16)-88, (u16)-16);
    SetBgScroll(1, (u16)-88, (u16)-64);
    SetBgScroll(2, (u16)-88, (u16)-112);
    return 1;
}

u8 UpdateDeckExchangeLoadDeckInfo(DeckExchangeWork* work, void* task) {
    s32 deckIndex;

    FadeStartIn(FADE_MODE_BLACK, 16);

    switch (work->step) {
    case 1:
        LoadBgMap(0, gDeck1PanelMap, 0x180);
        break;
    case 2:
        LoadBgMap(1, gDeck2PanelMap, 0x180);
        break;
    case 3:
        LoadBgMap(2, gDeck3PanelMap, 0x180);
        break;
    case 4:
        DrawDeckCategoryCount(work->deckAttackCount, CARD_CATEGORY_ATTACK);
        break;
    case 5:
        DrawDeckCategoryCount(work->deckMagicCount, CARD_CATEGORY_MAGIC);
        break;
    case 6:
        DrawDeckCategoryCount(work->deckItemCount, CARD_CATEGORY_ITEM);
        break;
    case 7:
        DrawDeckCategoryCount(work->deckEnemyCount, CARD_CATEGORY_ENEMY);
        break;
    case 8:
        HighlightDeckExchangeDeckTab(work, work->deckIndex);
        break;
    case 9:
        DrawDeckExchangeDeckCardCount(0);
        DrawDeckExchangeDeckCardCount(1);
        DrawDeckExchangeDeckCardCount(2);
        break;
    case 10:
        DrawDeckExchangeEquipMarker(GetActiveDeckIndex());
        DrawDeckExchangeCardTotals();
        work->thumbX = 0x4800;
        work->thumbY = 0x2800;
        deckIndex = work->deckIndex;
        work->cursorRow = deckIndex;
        ApproachValue(&work->handX, sDeckExchangeTabPointerX[work->cursorCol] << 8, work->timer);
        ApproachValue(&work->handY, sDeckExchangeTabPointerY[work->cursorRow] << 8, work->timer);
        SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeOpenCollection);
        work->view = DECK_MENU_VIEW_DECK_SELECT;
        SetDeckExchangeHandAnim(work);
        LoadDeckExchangeDeckNameTexts(work);
        break;
    }

    work->step++;
    return 1;
}

u8 UpdateDeckExchangeValueSelect(DeckExchangeWork* work, void* task) {
    s8 prevRow;
    s32 saved;

    if (work->popupActive != 0) {
        TaskPoolUpdate(&work->tasks);
        TaskPoolUpdate(&work->tasks2);

        if (GetKeysPressed() & START_BUTTON) {
            work->exitRequested = TRUE;
        }

        return 1;
    }

    if (work->exitRequested) {
        if ((u8)CheckDeckExchangeCpCost(work) && CheckDeckExchangeHasAttackCard(work)) {
            SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeFadeOut);
            FadeStartOut(FADE_MODE_BLACK, 4);
            m4aSongNumStart(SONG_SYS_CANSEL);
            return 1;
        }

        work->exitRequested = FALSE;
    }

    work->gfx = AnimUpdate(&work->anim);
    work->gfx2 = AnimUpdate(&work->anim2);

    switch (GetKeysRepeat()) {
    case DPAD_LEFT:
        if (work->cursorCol > 0) {
            work->cursorCol--;
            work->timer = 4;

            if ((u8)MoveDeckExchangeValueCursor(work, DPAD_LEFT)) {
                m4aSongNumStart(SONG_SYS_CLICK);
            }
        }

        DrawDeckExchangeValueCpCost(work);
        break;
    case DPAD_RIGHT:
        if (work->cursorCol <= 0) {
            work->cursorCol++;
            work->timer = 4;

            if ((u8)MoveDeckExchangeValueCursor(work, DPAD_RIGHT)) {
                m4aSongNumStart(SONG_SYS_CLICK);
            }
        }

        DrawDeckExchangeValueCpCost(work);
        break;
    case DPAD_UP:
        prevRow = work->cursorRow;

        if (work->cursorRow > 0) {
            work->cursorRow--;
        }

        work->timer = 4;
        MoveDeckExchangeValueCursor(work, DPAD_UP);

        if (prevRow != work->cursorRow) {
            m4aSongNumStart(SONG_SYS_CLICK);
        }

        DrawDeckExchangeValueCpCost(work);
        break;
    case DPAD_DOWN:
        prevRow = work->cursorRow;

        if (work->cursorRow <= 3) {
            work->cursorRow++;
        }

        work->timer = 4;
        MoveDeckExchangeValueCursor(work, DPAD_DOWN);

        if (prevRow != work->cursorRow) {
            m4aSongNumStart(SONG_SYS_CLICK);
        }

        DrawDeckExchangeValueCpCost(work);
        break;
    }

    switch (GetKeysPressed()) {
    case B_BUTTON:
        SetDeckExchangeFrameCursor(work, DECK_FRAME_CURSOR_CARD);
        saved = (s8)work->savedCol;
        work->cursorCol = saved;
        saved = (s8)work->savedRow;
        work->cursorRow = saved;
        work->handX = gCollectionGridColumnX[work->cursorCol] << 8;
        work->handY = gCollectionGridRowY[work->cursorRow] << 8;
        ShowDeckExchangeCardPreview(work);
        work->view = DECK_MENU_VIEW_DELETE_GRID;
        SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeGrid);
        return 1;
    case A_BUTTON:
        if (!(u8)TakeTradeCard(work)) {
            return 1;
        }

        DrawDeckExchangeCardTotals();
        CountCardsNotInDeckByCategory(DECK_ANY, work->collectionCategoryCounts);
        DrawCollectionCategoryCount(work->collectionCategoryCounts[CARD_CATEGORY_ATTACK], CARD_CATEGORY_ATTACK);
        DrawCollectionCategoryCount(work->collectionCategoryCounts[CARD_CATEGORY_MAGIC], CARD_CATEGORY_MAGIC);
        DrawCollectionCategoryCount(work->collectionCategoryCounts[CARD_CATEGORY_ITEM], CARD_CATEGORY_ITEM);
        DrawCollectionCategoryCount(work->collectionCategoryCounts[CARD_CATEGORY_ENEMY], CARD_CATEGORY_ENEMY);
        SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeClose);
        DrawDeckExchangeValueCpCost(work);
        work->timer = 4;
        break;
    case START_BUTTON:
        if ((u8)CheckDeckExchangeCpCost(work) && CheckDeckExchangeHasAttackCard(work)) {
            SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeFadeOut);
            FadeStartOut(FADE_MODE_BLACK, 4);
            m4aSongNumStart(SONG_SYS_CANSEL);
        }

        return 1;
    }

    if (work->timer != 0) {
        ApproachValue(&work->handX, sDeckExchangeValueGridX[work->cursorCol] << 8, work->timer);
        ApproachValue(&work->handY, (sDeckExchangeValueGridY[work->cursorRow] - 16) << 8, work->timer);
        work->timer--;
    }

    TaskPoolUpdate(&work->tasks);
    TaskPoolUpdate(&work->tasks2);
    return 1;
}

u8 UpdateDeckExchangeCollectionFilter(DeckExchangeWork* work, void* task) {
    s32 i;

    work->gfx = AnimUpdate(&work->anim);

    switch (GetKeysRepeat()) {
    case DPAD_LEFT:
        if (work->cursorCol > CATEGORY_FILTER_ATTACK) {
            work->cursorCol--;
            work->timer = 4;
            m4aSongNumStart(SONG_SYS_CLICK);
            work->categoryFilter = work->cursorCol;
            DrawDeckExchangeCollectionFilterTab(work->categoryFilter, work->mode);
            ClearDeckExchangeCardGrid(work);
            work->gridEntryCount = CreateDeckExchangeCollectionGridCards(work, work->categoryFilter, TRUE);
        }

        for (i = 0; i < 10; i++) {
            DrawValueCount(0, i);
        }

        break;
    case DPAD_RIGHT:
        if (work->cursorCol < CATEGORY_FILTER_COLLECTION_ALL) {
            work->cursorCol++;
            work->timer = 4;
            m4aSongNumStart(SONG_SYS_CLICK);
            work->categoryFilter = work->cursorCol;
            DrawDeckExchangeCollectionFilterTab(work->categoryFilter, work->mode);
            ClearDeckExchangeCardGrid(work);
            work->gridEntryCount = CreateDeckExchangeCollectionGridCards(work, work->categoryFilter, TRUE);
        }

        for (i = 0; i < 10; i++) {
            DrawValueCount(0, i);
        }

        break;
    case DPAD_DOWN:
        if (work->gridEntryCount != 0) {
            work->cursorCol = 0;
            work->cursorRow = 0;
            work->timer = 4;
            m4aSongNumStart(SONG_SYS_CLICKI04B);
            ShowDeckExchangeCardPreview(work);
            work->view = DECK_MENU_VIEW_DELETE_GRID;
            SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeGrid);
            work->thumbX = 0xA000;
            work->thumbY = 0x2800;
            work->scrollRowEnd = 4;
            return 1;
        }

        m4aSongNumStart(SONG_SYS_BEEP);
        break;
    case B_BUTTON:
        if (work->gridEntryCount != 0) {
            work->cursorCol = 0;
            work->cursorRow = 0;
            work->timer = 4;
            m4aSongNumStart(SONG_SYS_CLICKI04B);
            ShowDeckExchangeCardPreview(work);
            work->view = DECK_MENU_VIEW_DELETE_GRID;
            SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeGrid);
            work->thumbX = 0xA000;
            work->thumbY = 0x2800;
            work->scrollRowEnd = 4;
            return 1;
        }

        SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeClose);
        m4aSongNumStart(SONG_SYS_CLOSE);
        return 1;
    case START_BUTTON:
        if ((u8)CheckDeckExchangeCpCost(work) && CheckDeckExchangeHasAttackCard(work)) {
            SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeFadeOut);
            FadeStartOut(FADE_MODE_BLACK, 4);
            m4aSongNumStart(SONG_SYS_CANSEL);
        }

        return 1;
    }

    if (work->timer != 0) {
        ApproachValue(&work->handX, sDeckExchangeFilterTabX[work->cursorCol] << 8, work->timer);
        ApproachValue(&work->handY, 0x1E00, work->timer);
        work->timer--;
    }

    TaskPoolUpdate(&work->tasks);
    TaskPoolUpdate(&work->tasks2);
    return 1;
}

u8 UpdateDeckExchangeOpenCollection(DeckExchangeWork* work, void* task) {
    FadeStartIn(FADE_MODE_BLACK, 4);
    SetupBg(3, 0, 30, 0);
    SetupBg(2, 0, 15, 0);
    SetupBg(1, 0, 23, 0);
    SetupBg(0, 0, 31, 0);
    SetBgScroll(0, 0, 0);
    SetBgScroll(1, 0, 0);
    SetBgScroll(2, 0, 16);
    LoadBgMap(3, gDeckExchangeGridMap, sizeof(gDeckExchangeGridMap));
    LoadBgMap(2, gDeckCollectionInfoMap, sizeof(gDeckCollectionInfoMap));
    LoadBgMap(1, gDeckCardsInUseMap, sizeof(gDeckCardsInUseMap));
    DisableBg(0);
    CountCardsNotInDeckByCategory(DECK_ANY, work->collectionCategoryCounts);
    DrawCollectionCategoryCount(work->collectionCategoryCounts[CARD_CATEGORY_ATTACK], CARD_CATEGORY_ATTACK);
    DrawCollectionCategoryCount(work->collectionCategoryCounts[CARD_CATEGORY_MAGIC], CARD_CATEGORY_MAGIC);
    DrawCollectionCategoryCount(work->collectionCategoryCounts[CARD_CATEGORY_ITEM], CARD_CATEGORY_ITEM);
    DrawCollectionCategoryCount(work->collectionCategoryCounts[CARD_CATEGORY_ENEMY], CARD_CATEGORY_ENEMY);
    work->view = DECK_MENU_VIEW_DELETE_GRID;
    SetDeckExchangeFrameCursor(work, DECK_FRAME_CURSOR_CARD);
    ClearDeckExchangeCardGrid(work);
    work->step = 0;
    SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeBuildList);
    work->unk_6C4 = 0xFFFE;
    work->unk_6C6 = 142;
    work->unk_6C8 = 142;
    work->unk_6CA = 142;
    TaskPoolUpdate(&work->tasks);
    TaskPoolUpdate(&work->tasks2);
    return 1;
}

u8 UpdateDeckExchangeBuildList(DeckExchangeWork* work, void* task) {
    u16 i;
    u16 j;
    u16 n;

    FadeStartIn(FADE_MODE_BLACK, 16);

    switch (work->step) {
    case 0:
        work->entryCount = 286;
        work->kindEntries = EwramAlloc(work->entryCount * sizeof(CardKindEntry));
        CpuFill32(0, work->kindEntries, work->entryCount * sizeof(CardKindEntry));
        break;
    case 1:
        CountCardsNotInDeckByKind(work->kindEntries, work->deckIndex, FALSE, work->entryCount, work->unk_4F4);
        break;
    case 2:
        work->entryCount = ListCardsNotInDeckByKind(work->kindEntries, work->deckIndex, FALSE, work->entryCount, work->unk_4F4);
        break;
    case 3:
        work->entries = EwramAlloc(work->entryCount * sizeof(CardKindEntry));

        for (i = 0, n = 0; i < 286; i++) {
            if (work->kindEntries[i].count != 0) {
                work->entries[n] = work->kindEntries[i];
                work->entries[n].indices = EwramAlloc(work->kindEntries[i].indexCount * sizeof(u16));

                for (j = 0; j < work->kindEntries[i].indexCount; j++) {
                    work->entries[n].indices[j] = work->kindEntries[i].indices[j];
                }

                n++;
            }
        }

        break;
    case 4:
        for (i = 0; i < 286; i++) {
            if (work->kindEntries[i].count != 0) {
                EwramFree(work->kindEntries[i].indices);
            }
        }

        EwramFree(work->kindEntries);
        break;
    case 5:
        work->categoryFilter = CATEGORY_FILTER_COLLECTION_ALL;
        work->gridEntryCount = CreateDeckExchangeCollectionGridCards(work, 5, TRUE);
        SetDeckExchangeHandAnim(work);
        work->handX = gCollectionGridColumnX[0] << 8;
        work->handY = gCollectionGridRowY[0] << 8;
        work->mode = DECK_MENU_MODE_ADD;
        work->cursorCol = 0;
        work->cursorRow = 0;
        ShowDeckExchangeCardPreview(work);

        if (work->gridEntryCount != 0) {
            SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeGrid);
        } else {
            work->cursorCol = work->categoryFilter;
            work->timer = 4;
            work->view = DECK_MENU_VIEW_DELETE_FILTER;
            SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeCollectionFilter);
        }

        break;
    }

    work->step++;
    TaskPoolUpdate(&work->tasks);
    TaskPoolUpdate(&work->tasks2);
    return 1;
}

u8 UpdateDeckExchangeGrid(DeckExchangeWork* work, void* task) {
    s32 i;

    work->gfx = AnimUpdate(&work->anim);
    work->gfx2 = AnimUpdate(&work->anim2);

    if (work->popupActive != 0) {
        TaskPoolUpdate(&work->tasks);
        TaskPoolUpdate(&work->tasks2);

        if (GetKeysPressed() & START_BUTTON) {
            work->exitRequested = TRUE;
        }

        return 1;
    }

    if (work->exitRequested) {
        if ((u8)CheckDeckExchangeCpCost(work) && CheckDeckExchangeHasAttackCard(work)) {
            SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeFadeOut);
            FadeStartOut(FADE_MODE_BLACK, 4);
            m4aSongNumStart(SONG_SYS_CANSEL);
            return 1;
        }

        work->exitRequested = FALSE;
    }

    switch (GetKeysRepeat()) {
    case DPAD_UP:
        if (work->cursorRow > 0) {
            if (IsDeckExchangeCardAt(work, work->cursorCol, work->cursorRow - 1)) {
                work->cursorRow--;
                work->timer = 4;
                m4aSongNumStart(SONG_SYS_CLICKI04B);
            }
        } else {
            if (!ScrollDeckExchangeGridUp(work)) {
                UpdateDeckExchangeGridScrollBar(work);
                work->cursorCol = work->categoryFilter;
                work->timer = 4;
                m4aSongNumStart(SONG_SYS_CLICKI04B);
                work->view = DECK_MENU_VIEW_DELETE_FILTER;

                for (i = 0; i < 10; i++) {
                    DrawValueCount(0, i);
                }

                SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeCollectionFilter);
                return 1;
            }

            UpdateDeckExchangeGridScrollBar(work);
        }

        ShowDeckExchangeCardPreview(work);
        break;
    case DPAD_DOWN:
        if (work->cursorRow < 3) {
            if (IsDeckExchangeCardAt(work, work->cursorCol, work->cursorRow + 1)) {
                work->cursorRow++;
                work->timer = 4;
                m4aSongNumStart(SONG_SYS_CLICKI04B);
            }
        } else if (IsDeckExchangeCardAt(work, work->cursorCol, work->cursorRow + 1)) {
            ScrollDeckExchangeGridDown(work);
            UpdateDeckExchangeGridScrollBar(work);
        }

        ShowDeckExchangeCardPreview(work);
        break;
    case DPAD_LEFT:
        if (work->cursorCol > 0 && IsDeckExchangeCardAt(work, work->cursorCol - 1, work->cursorRow)) {
            work->cursorCol--;
            work->timer = 4;
            m4aSongNumStart(SONG_SYS_CLICKI04B);
        }

        ShowDeckExchangeCardPreview(work);
        break;
    case DPAD_RIGHT:
        if (work->cursorCol > 1) {
            work->timer = 4;
            return 1;
        }

        if (IsDeckExchangeCardAt(work, work->cursorCol + 1, work->cursorRow)) {
            work->cursorCol++;
            work->timer = 4;
            m4aSongNumStart(SONG_SYS_CLICKI04B);
        }

        ShowDeckExchangeCardPreview(work);
        break;
    }

    switch (GetKeysPressed()) {
    case A_BUTTON:
        if (IsDeckExchangeCardAtCursor(work)) {
            work->savedCol = work->cursorCol;
            work->savedRow = work->cursorRow;
            work->cursorCol = 0;
            work->cursorRow = 0;
            SetDeckExchangeFrameCursor(work, DECK_FRAME_CURSOR_ROW);
            work->view = DECK_MENU_VIEW_DELETE_VALUE_SELECT;
            m4aSongNumStart(SONG_SYS_KETTEI);

            if ((u8)MoveDeckExchangeValueCursor(work, 0)) {
                DrawDeckExchangeValueCpCost(work);
                work->handX = sDeckExchangeValueGridX[work->cursorCol] << 8;
                work->handY = (sDeckExchangeValueGridY[work->cursorRow] - 16) << 8;
                SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeValueSelect);
                return 1;
            }

            work->cursorCol = (s8)work->savedCol;
            work->cursorRow = (s8)work->savedRow;
            SetDeckExchangeFrameCursor(work, DECK_FRAME_CURSOR_CARD);
            work->view = DECK_MENU_VIEW_DELETE_GRID;
            m4aSongNumStart(SONG_SYS_BEEP);
            return 1;
        }

        m4aSongNumStart(SONG_SYS_BEEP);
        return 1;
    case B_BUTTON:
        SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeClose);
        m4aSongNumStart(SONG_SYS_CLOSE);
        return 1;
    case START_BUTTON:
        if ((u8)CheckDeckExchangeCpCost(work) && CheckDeckExchangeHasAttackCard(work)) {
            SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeFadeOut);
            FadeStartOut(FADE_MODE_BLACK, 4);
            m4aSongNumStart(SONG_SYS_CANSEL);
        }

        return 1;
    }

    if (GetKeysPressed() & SELECT_BUTTON) {
        ResetDeckExchangeGridScroll(work);
        work->cursorCol = work->categoryFilter;
        work->timer = 4;
        work->thumbX = 0xA000;
        work->thumbY = 0x2800;
        work->scrollRowEnd = 4;
        m4aSongNumStart(SONG_SYS_CLICKI04B);
        work->view = DECK_MENU_VIEW_DELETE_FILTER;
        SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeCollectionFilter);
        return 1;
    }

    if (work->timer != 0) {
        ApproachValue(&work->handX, gCollectionGridColumnX[work->cursorCol] << 8, work->timer);
        ApproachValue(&work->handY, gCollectionGridRowY[work->cursorRow] << 8, work->timer);
        work->timer--;
    }

    TaskPoolUpdate(&work->tasks);
    TaskPoolUpdate(&work->tasks2);
    return 1;
}

u8 UpdateDeckExchangeClose(DeckExchangeWork* work, void* task) {
    FadeStartOut(FADE_MODE_BLACK, 16);
    work->categoryFilter = CATEGORY_FILTER_DECK_ALL;
    SetTaskUpdate(task, (TaskUpdateFunc)UpdateDeckExchangeFadeOut);
    TaskPoolUpdate(&work->tasks);
    TaskPoolUpdate(&work->tasks2);
    return 1;
}

u8 UpdateDeckExchangeFadeOut(DeckExchangeWork* work) {
    if (!FadeIsActive()) {
        ClearDeckExchangeCardGrid(work);
        return 0;
    }

    TaskPoolUpdate(&work->tasks);
    TaskPoolUpdate(&work->tasks2);
    return 1;
}

void DrawDeckExchangeDeckNames(DeckExchangeWork* work, u8 selectedOnly) {
    if (!selectedOnly) {
        switch (work->deckIndex) {
        case 0:
            DrawTextSlots(work->deckNameX, work->deckNameY, work->textSlots, work->palette, 20, work->textSlotCount);
            DrawTextSlots(work->deckName2X, work->deckName2Y, work->textSlots2, work->palette4, 20, work->textSlotCount2);
            DrawTextSlots(work->deckName3X, work->deckName3Y, work->textSlots3, work->palette4, 20, work->textSlotCount3);
            break;
        case 1:
            DrawTextSlots(work->deckNameX, work->deckNameY, work->textSlots, work->palette4, 20, work->textSlotCount);
            DrawTextSlots(work->deckName2X, work->deckName2Y, work->textSlots2, work->palette, 20, work->textSlotCount2);
            DrawTextSlots(work->deckName3X, work->deckName3Y, work->textSlots3, work->palette4, 20, work->textSlotCount3);
            break;
        case 2:
            DrawTextSlots(work->deckNameX, work->deckNameY, work->textSlots, work->palette4, 20, work->textSlotCount);
            DrawTextSlots(work->deckName2X, work->deckName2Y, work->textSlots2, work->palette4, 20, work->textSlotCount2);
            DrawTextSlots(work->deckName3X, work->deckName3Y, work->textSlots3, work->palette, 20, work->textSlotCount3);
            break;
        }
    } else {
        switch (work->deckIndex) {
        case 0:
            DrawTextSlots(work->deckNameX, work->deckNameY, work->textSlots, work->palette, 20, work->textSlotCount);
            break;
        case 1:
            DrawTextSlots(work->deckName2X, work->deckName2Y, work->textSlots2, work->palette, 20, work->textSlotCount2);
            break;
        case 2:
            DrawTextSlots(work->deckName3X, work->deckName3Y, work->textSlots3, work->palette, 20, work->textSlotCount3);
            break;
        }
    }
}

void DrawDeckExchangeCardDescription(DeckExchangeWork* work) {
    DrawTextSlots(work->descriptionX, work->descriptionY, work->textSlots5, work->palette, 20, work->textSlotCount5);
}

void deckexchange_2(DeckExchangeWork* work) {
    if (work->popupActive == 0) {
        DrawSprite((work->handX >> 8) - 16, (work->handY >> 8) - 30, work->gfx, work->tiles, work->palette, NULL, work->handFlags, 3);
    }

    DrawSprite(work->thumbX >> 8, work->thumbY >> 8, gDeckScrollThumbFrames[0], work->tiles3, work->palette, NULL, SPRITE_PRIORITY(2), 10);

    switch (work->view) {
    case DECK_MENU_VIEW_DECK_GRID:
        if (work->holding) {
            DrawSprite((work->heldX >> 8) - 16, (work->heldY >> 8) - 20, work->gfx2, work->tiles2, work->palette4, NULL, 0, 8);
        }

        DrawSprite((work->handX >> 8) - 16, (work->handY >> 8) - 20, work->gfx2, work->tiles2, work->palette4, NULL, 0, 8);
    case DECK_MENU_VIEW_DECK_SELECT:
    case DECK_MENU_VIEW_DECK_FILTER:
    case DECK_MENU_VIEW_COMMANDS:
        DrawDeckExchangeDeckNames(work, FALSE);
        break;
    case DECK_MENU_VIEW_ADD_GRID:
        DrawDeckExchangeDeckNames(work, TRUE);
        DrawSprite((work->handX >> 8) - 16, (work->handY >> 8) - 20, work->gfx2, work->tiles2, work->palette4, NULL, 0, 8);

        if (work->tiles4 != NULL) {
            if (work->popupActive == 0) {
                DrawSprite((work->handX >> 8) - 16, (work->handY >> 8) - 20, work->gfx2, work->tiles2, work->palette4, NULL, 0, 8);
            }

            DrawSprite(24, 82, work->gfx3, work->tiles4, work->palette2, NULL, 0, 20);
            DrawSprite(24, 82, work->gfx4, work->tiles5, work->palette3, NULL, 0, 21);
            DrawTextSlots(10, 116, work->textSlots4, work->palette4, 20, work->textSlotCount4);
        }

        break;
    case DECK_MENU_VIEW_REMOVE_GRID:
        DrawDeckExchangeDeckNames(work, TRUE);
        DrawSprite((work->handX >> 8) - 16, (work->handY >> 8) - 20, work->gfx2, work->tiles2, work->palette4, NULL, 0, 8);

        if (work->tiles4 != NULL) {
            DrawSprite(164, 82, work->gfx3, work->tiles4, work->palette2, NULL, 0, 20);
            DrawSprite(164, 82, work->gfx4, work->tiles5, work->palette3, NULL, 0, 21);

            if (work->tiles6 != NULL) {
                DrawSprite(164, 82, work->gfx5, work->tiles6, work->palette2, NULL, 0, 19);
            }

            DrawTextSlots(100, 116, work->textSlots4, work->palette4, 20, work->textSlotCount4);
        }

        break;
    case DECK_MENU_VIEW_ADD_VALUE_SELECT:
        DrawSprite((work->handX >> 8) - 26, (work->handY >> 8) - 13, work->gfx2, work->tiles2, work->palette4, NULL, 0, 8);
        DrawDeckExchangeDeckNames(work, TRUE);

        if (work->tiles4 != NULL) {
            DrawSprite(24, 82, work->gfx3, work->tiles4, work->palette2, NULL, 0, 20);
            DrawSprite(24, 82, work->gfx4, work->tiles5, work->palette3, NULL, 0, 21);
            DrawTextSlots(10, 116, work->textSlots4, work->palette4, 20, work->textSlotCount4);
        }

        break;
    case DECK_MENU_VIEW_ADD_FILTER:
        DrawDeckExchangeDeckNames(work, TRUE);

        if (work->tiles4 != NULL) {
            DrawSprite(24, 82, work->gfx3, work->tiles4, work->palette2, NULL, 0, 20);
            DrawSprite(24, 82, work->gfx4, work->tiles5, work->palette3, NULL, 0, 21);
            DrawTextSlots(10, 116, work->textSlots4, work->palette4, 20, work->textSlotCount4);
        }

        break;
    case DECK_MENU_VIEW_REMOVE_FILTER:
        DrawDeckExchangeDeckNames(work, TRUE);

        if (work->tiles4 != NULL) {
            DrawSprite(164, 82, work->gfx3, work->tiles4, work->palette2, NULL, 0, 20);
            DrawSprite(164, 82, work->gfx4, work->tiles5, work->palette3, NULL, 0, 21);

            if (work->tiles6 != NULL) {
                DrawSprite(164, 82, work->gfx5, work->tiles6, work->palette2, NULL, 0, 19);
            }

            DrawTextSlots(100, 116, work->textSlots4, work->palette4, 20, work->textSlotCount4);
        }

        break;
    case DECK_MENU_VIEW_DELETE_GRID:
        DrawSprite((work->handX >> 8) - 16, (work->handY >> 8) - 20, work->gfx2, work->tiles2, work->palette4, NULL, 0, 8);
        DrawDeckExchangeCardDescription(work);

        if (work->tiles4 != NULL) {
            DrawSprite(24, 66, work->gfx3, work->tiles4, work->palette2, NULL, 0, 20);
            DrawSprite(24, 66, work->gfx4, work->tiles5, work->palette3, NULL, 0, 21);
            DrawTextSlots(10, 100, work->textSlots4, work->palette4, 20, work->textSlotCount4);
        }

        break;
    case DECK_MENU_VIEW_DELETE_VALUE_SELECT:
        DrawSprite((work->handX >> 8) - 26, (work->handY >> 8) - 13, work->gfx2, work->tiles2, work->palette4, NULL, 0, 8);
        DrawDeckExchangeCardDescription(work);

        if (work->tiles4 != NULL) {
            DrawSprite(24, 66, work->gfx3, work->tiles4, work->palette2, NULL, 0, 20);
            DrawSprite(24, 66, work->gfx4, work->tiles5, work->palette3, NULL, 0, 21);
            DrawTextSlots(10, 100, work->textSlots4, work->palette4, 20, work->textSlotCount4);
        }

        break;
    }

    TaskPoolDraw(&work->tasks);
    TaskPoolDraw(&work->tasks2);
}

void deckexchange_3(DeckExchangeWork* work) {
    ObjPalette** palette4;

    if (work->tiles8 != NULL) {
        ReleaseObjTiles(work->tiles8);
    }

    if (work->palette5 != NULL) {
        ReleaseObjPalette(work->palette5);
    }

    if (work->tiles9 != NULL) {
        ReleaseObjTiles(work->tiles9);
    }

    if (work->palette6 != NULL) {
        ReleaseObjPalette(work->palette6);
    }

    if (work->palette7 != NULL) {
        ReleaseObjPalette(work->palette7);
    }

    palette4 = &work->palette4;

    if (*palette4 != NULL) {
        ReleaseObjPalette(*palette4);
    }

    ReleaseDeckExchangeCardPreview(work);
    ReleaseObjTiles(work->tiles);
    ReleaseObjTiles(work->tiles2);
    ReleaseObjTiles(work->tiles3);
    ReleaseObjPalette(work->palette);
    FreeTextSlots(work->textSlots, ARRAY_COUNT(work->textSlots));
    FreeTextSlots(work->textSlots2, ARRAY_COUNT(work->textSlots2));
    FreeTextSlots(work->textSlots3, ARRAY_COUNT(work->textSlots3));
    FreeTextSlots(work->textSlots4, ARRAY_COUNT(work->textSlots4));
    FreeTextSlots(work->textSlots5, ARRAY_COUNT(work->textSlots5));
    // @bug Releases palette4 a second time.
    ReleaseObjPalette(*palette4);
    TaskPoolDestroy(&work->tasks);
    TaskPoolDestroy(&work->tasks2);
    FreeDeckExchangeCollectionEntries(work);
    *work->resultOut = DECK_MENU_RESULT_CLOSED;
}

void CreateDeckExchangeDeckGridCards(DeckExchangeWork* work, u8 categoryFilter) {
    DeckCard2Args args;
    u16* cards;
    u8 i;
    s8 x;
    s8 y;

    cards = GetDeck(work->deckIndex)->cards;
    x = 0;
    y = 0;

    for (i = 0; i < DECK_SIZE; i++) {
        if (cards[i] != CARD_NONE) {
            if (categoryFilter == CATEGORY_FILTER_DECK_ALL) {
                args.pool = &work->pool;
                args.cardId = gCardCollection[cards[i]] & (CARD_ID_MASK | CARD_FLAG_PREMIUM);
                args.col = x;
                args.row = y;
                args.panel = DECK_CARD2_PANEL_DECK;
                args.slot = &cards[i];
                TaskCreate(&work->tasks, &gTaskDescDeckCard2, &args);
                x++;
            } else if (gCardDefs[gCardCollection[cards[i]] & CARD_ID_MASK].category == categoryFilter - 1) {
                args.pool = &work->pool;
                args.cardId = gCardCollection[cards[i]] & (CARD_ID_MASK | CARD_FLAG_PREMIUM);
                args.col = x;
                args.row = y;
                args.panel = DECK_CARD2_PANEL_DECK;
                args.slot = &cards[i];
                TaskCreate(&work->tasks, &gTaskDescDeckCard2, &args);
                x++;
            }

            if (x > 2) {
                x = 0;
                y++;
            }
        }
    }

    work->thumbX = 0x4800;
    work->thumbY = 0x2800;
    work->scrollRowEnd = 4;
    SetDeckExchangeGridRowCount(work, y * 3 + x);
}

s32 CreateDeckExchangeCollectionGridCards(DeckExchangeWork* work, u8 categoryFilter, u8 excludeBossCards) {
    DeckCard2Args args;
    u16 i;
    s8 x;
    s8 y;

    x = 0;
    y = 0;

    for (i = 0; i < work->entryCount; i++) {
        if (categoryFilter == CATEGORY_FILTER_COLLECTION_ALL) {
            if (work->entries[i].kind <= CARD_KIND_CRESCENDO) {
                args.pool = &work->pool;
                args.cardId = GetCardIdForKind(work->entries[i].kind);
                args.col = x;
                args.row = y;
                args.panel = DECK_CARD2_PANEL_COLLECTION;
                args.slot = NULL;
                TaskCreate(&work->tasks, &gTaskDescDeckCard2, &args);
                x++;
            }
        } else {
            args.pool = &work->pool;
            args.cardId = GetCardIdForKind(work->entries[i].kind);

            if (gCardDefs[args.cardId].category == categoryFilter - 1 && work->entries[i].kind <= CARD_KIND_CRESCENDO) {
                args.col = x;
                args.row = y;
                args.panel = DECK_CARD2_PANEL_COLLECTION;
                args.slot = NULL;
                TaskCreate(&work->tasks, &gTaskDescDeckCard2, &args);
                x++;
            }
        }

        if (x > 2) {
            x = 0;
            y++;
        }
    }

    work->thumbX = 0xA000;
    work->thumbY = 0x2800;
    work->scrollRowEnd = 4;
    SetDeckExchangeGridRowCount(work, y * 3 + x);
    // @bug No return value (the caller reads it).
}

s32 GetCardIdForKind(s32 kind) {
    u32 i;

    for (i = 0; i < 950; i++) {
        if (gCardDefs[i].kind == kind) {
            return i;
        }
    }
}

void ClearDeckExchangeCardGrid(DeckExchangeWork* work) {
    DeckCard2Work* node;

    node = ListPoolFirst(&work->pool);

    while (node != NULL) {
        node->done = TRUE;
        node = ListPoolNext(&node->node);
    }

    TaskPoolUpdate(&work->tasks);
}

void ScrollDeckExchangeGridDown(DeckExchangeWork* work) {
    DeckCard2Work* node;

    node = ListPoolFirst(&work->pool);

    if (work->scrollRowEnd == 33) {
        return;
    }

    while (node != NULL) {
        node->args.row--;

        if (node->args.row < 0) {
            node->y = 0x20000;
            DeckCard2ReleaseGfx(node);
        }

        node = ListPoolNext(&node->node);
    }

    m4aSongNumStart(SONG_SYS_CLICKI04B);
    work->scrollRowEnd++;
    work->thumbY += 0x300;

    if (work->thumbY > 0x7C00) {
        work->thumbY = 0x7C00;
    }

    if (work->holding) {
        work->heldRow--;
    }
}

u8 ScrollDeckExchangeGridUp(DeckExchangeWork* work) {
    DeckCard2Work* node;

    node = ListPoolFirst(&work->pool);

    if (node == NULL) {
        work->thumbY -= 0x300;

        if (work->thumbY < 0x2800) {
            work->thumbY = 0x2800;
            return FALSE;
        }

        return TRUE;
    }

    if (node->args.row == 0) {
        return FALSE;
    }

    do {
        node->args.row++;

        if (node->args.row > 3) {
            node->y = 0x20000;
            DeckCard2ReleaseGfx(node);
        }

        node = ListPoolNext(&node->node);
    } while (node != NULL);

    m4aSongNumStart(SONG_SYS_CLICKI04B);
    work->scrollRowEnd--;
    work->thumbY -= 0x300;

    if (work->thumbY < 0x2800) {
        work->thumbY = 0x2800;
    }

    return TRUE;
}

void SetDeckExchangeHandAnim(DeckExchangeWork* work) {
    u16 handFlags;

    switch (work->view) {
    case DECK_MENU_VIEW_DECK_GRID:
    case DECK_MENU_VIEW_DECK_FILTER:
    case DECK_MENU_VIEW_ADD_GRID:
    case DECK_MENU_VIEW_ADD_VALUE_SELECT:
    case DECK_MENU_VIEW_ADD_FILTER:
    case DECK_MENU_VIEW_REMOVE_GRID:
    case DECK_MENU_VIEW_REMOVE_FILTER:
    case DECK_MENU_VIEW_DELETE_GRID:
    case DECK_MENU_VIEW_DELETE_FILTER:
    case DECK_MENU_VIEW_DELETE_VALUE_SELECT:
        AnimStart(&work->anim, 0, ANIM_FLAG_LOOP);
        work->handFlags &= ~SPRITE_FLAG_HFLIP;
        break;
    case DECK_MENU_VIEW_DECK_SELECT:
    case DECK_MENU_VIEW_COMMANDS:
        AnimStart(&work->anim, 2, ANIM_FLAG_LOOP);
        handFlags = work->handFlags | SPRITE_FLAG_HFLIP;
        work->handFlags = handFlags;
        break;
    }
}

void HighlightDeckExchangeDeckTab(DeckExchangeWork* work, u8 deckIndex) {
    u16* pal;

    switch (deckIndex) {
    case 0:
        pal = (u16*)(BG_PLTT + 8 * PLTT_SIZE_4BPP);
        LoadPalette(gDeckTabHighlightPalette, pal, sizeof(gDeckTabHighlightPalette));
        pal = (u16*)(BG_PLTT + 9 * PLTT_SIZE_4BPP);
        LoadPalette(gDeckMenuPalettes[9], pal, sizeof(gDeckMenuPalettes[0]));
        pal = (u16*)(BG_PLTT + 10 * PLTT_SIZE_4BPP);
        LoadPalette(gDeckMenuPalettes[10], pal, sizeof(gDeckMenuPalettes[0]));
        LoadBgMap(0, gDeck1PanelMap + 0xC0, 0x180);
        LoadBgMap(1, gDeck2PanelMap, 0x180);
        LoadBgMap(2, gDeck3PanelMap, 0x180);
        SetBgScroll(0, (u16)-76, (u16)-14);
        SetBgScroll(1, (u16)-88, (u16)-64);
        SetBgScroll(2, (u16)-88, (u16)-112);
        work->deckNameX = 100;
        work->deckNameY = 25;
        work->deckName2X = 102;
        work->deckName2Y = 75;
        work->deckName3X = 102;
        work->deckName3Y = 122;
        break;
    case 1:
        pal = (u16*)(BG_PLTT + 9 * PLTT_SIZE_4BPP);
        LoadPalette(gDeckTabHighlightPalette, pal, sizeof(gDeckTabHighlightPalette));
        pal = (u16*)(BG_PLTT + 8 * PLTT_SIZE_4BPP);
        LoadPalette(gDeckMenuPalettes[8], pal, sizeof(gDeckMenuPalettes[0]));
        pal = (u16*)(BG_PLTT + 10 * PLTT_SIZE_4BPP);
        LoadPalette(gDeckMenuPalettes[10], pal, sizeof(gDeckMenuPalettes[0]));
        LoadBgMap(0, gDeck1PanelMap, 0x180);
        LoadBgMap(1, gDeck2PanelMap + 0xC0, 0x180);
        LoadBgMap(2, gDeck3PanelMap, 0x180);
        SetBgScroll(0, (u16)-88, (u16)-16);
        SetBgScroll(1, (u16)-76, (u16)-62);
        SetBgScroll(2, (u16)-88, (u16)-112);
        work->deckNameX = 102;
        work->deckNameY = 27;
        work->deckName2X = 100;
        work->deckName2Y = 73;
        work->deckName3X = 102;
        work->deckName3Y = 122;
        break;
    case 2:
        pal = (u16*)(BG_PLTT + 10 * PLTT_SIZE_4BPP);
        LoadPalette(gDeckTabHighlightPalette, pal, sizeof(gDeckTabHighlightPalette));
        pal = (u16*)(BG_PLTT + 8 * PLTT_SIZE_4BPP);
        LoadPalette(gDeckMenuPalettes[8], pal, sizeof(gDeckMenuPalettes[0]));
        pal = (u16*)(BG_PLTT + 9 * PLTT_SIZE_4BPP);
        LoadPalette(gDeckMenuPalettes[9], pal, sizeof(gDeckMenuPalettes[0]));
        LoadBgMap(0, gDeck1PanelMap, 0x180);
        LoadBgMap(1, gDeck2PanelMap, 0x180);
        LoadBgMap(2, gDeck3PanelMap + 0xC0, 0x180);
        SetBgScroll(0, (u16)-88, (u16)-16);
        SetBgScroll(1, (u16)-88, (u16)-64);
        SetBgScroll(2, (u16)-76, (u16)-110);
        work->deckNameX = 102;
        work->deckNameY = 27;
        work->deckName2X = 102;
        work->deckName2Y = 75;
        work->deckName3X = 100;
        work->deckName3Y = 121;
        break;
    }
}

void DrawDeckExchangeDeckCardCount(u8 deck) {
    u8 countDigits[2];
    u8 maxDigits[2];
    u8* base;
    u16 n;

    base = NULL;
    n = GetDeckCardCount(deck);
    countDigits[0] = n / 10;
    countDigits[1] = n - (u16)(n / 10) * 10;
    maxDigits[0] = 9;
    maxDigits[1] = 9;

    switch (deck) {
    case 0:
        base = GetBgCharBase(0);
        break;
    case 1:
        base = GetBgCharBase(1);
        break;
    case 2:
        base = GetBgCharBase(2);
        break;
    }

    RequestDma3Copy(&gDeckCountDigitTiles[(countDigits[0] + 1) * 32], base + 0x20, 32);
    RequestDma3Copy(&gDeckCountDigitTiles[(countDigits[1] + 1) * 32], base + 0x40, 32);
    RequestDma3Copy(&gDeckCountDigitTiles[(maxDigits[0] + 1) * 32], base + 0x60, 32);
    RequestDma3Copy(&gDeckCountDigitTiles[(maxDigits[1] + 1) * 32], base + 0x80, 32);
}

void DrawDeckExchangeEquipMarker(u8 deck) {
    u8* base0;
    u8* base1;
    u8* base2;

    base0 = GetBgCharBase(0);
    base1 = GetBgCharBase(1);
    base2 = GetBgCharBase(2);

    switch (deck) {
    case 0:
        RequestDma3Copy(gDeckEquipMarkerTiles + 0x20, base0 + 0x1A0, 0x1E0);
        RequestDma3Copy(gDeckEquipMarkerTiles + 0x420, base1 + 0x1A0, 0x1E0);
        RequestDma3Copy(gDeckEquipMarkerTiles + 0x420, base2 + 0x1A0, 0x1E0);
        break;
    case 1:
        RequestDma3Copy(gDeckEquipMarkerTiles + 0x420, base0 + 0x1A0, 0x1E0);
        RequestDma3Copy(gDeckEquipMarkerTiles + 0x20, base1 + 0x1A0, 0x1E0);
        RequestDma3Copy(gDeckEquipMarkerTiles + 0x420, base2 + 0x1A0, 0x1E0);
        break;
    case 2:
        RequestDma3Copy(gDeckEquipMarkerTiles + 0x420, base0 + 0x1A0, 0x1E0);
        RequestDma3Copy(gDeckEquipMarkerTiles + 0x420, base1 + 0x1A0, 0x1E0);
        RequestDma3Copy(gDeckEquipMarkerTiles + 0x20, base2 + 0x1A0, 0x1E0);
        break;
    }
}

void DrawDeckExchangeDeckCpCost(u8 deck) {
    u8 costDigits[3];
    u8 cpDigits[3];
    u8* base;
    u16 n;
    u8* out;

    base = NULL;
    n = GetDeckCpCost(deck);
    costDigits[0] = n / 100;
    costDigits[1] = n / 10 - costDigits[0] * 10;
    costDigits[2] = n - costDigits[0] * 100 - costDigits[1] * 10;
    out = cpDigits;
    out[0] = gGameState.progression.cp / 100;
    out[1] = gGameState.progression.cp / 10 - out[0] * 10;
    out[2] = gGameState.progression.cp - out[0] * 100 - out[1] * 10;

    switch (deck) {
    case 0:
        base = GetBgCharBase(0);
        break;
    case 1:
        base = GetBgCharBase(1);
        break;
    case 2:
        base = GetBgCharBase(2);
        break;
    }

    RequestDma3Copy(&gDeckCountDigitTiles[(costDigits[0] + 1) * 32], base + 0xA0, 32);
    RequestDma3Copy(&gDeckCountDigitTiles[(costDigits[1] + 1) * 32], base + 0xC0, 32);
    RequestDma3Copy(&gDeckCountDigitTiles[(costDigits[2] + 1) * 32], base + 0xE0, 32);
    RequestDma3Copy(&gDeckCountDigitTiles[(cpDigits[0] + 1) * 32], base + 0x100, 32);
    RequestDma3Copy(&gDeckCountDigitTiles[(cpDigits[1] + 1) * 32], base + 0x120, 32);
    RequestDma3Copy(&gDeckCountDigitTiles[(cpDigits[2] + 1) * 32], base + 0x140, 32);
}

void DrawDeckExchangeCollectionFilterTab(u8 categoryFilter, u8 mode) {
    u8* dst;

    dst = (u8*)GetBgScreenBase(3) + 0xA8;

    switch (categoryFilter) {
    case CATEGORY_FILTER_COLLECTION_ALL:
        RequestDma3Copy(gDeckFilterTabMap + mode * 128, dst, 20);
        RequestDma3Copy(gDeckFilterTabMap + 0x20 + mode * 128, dst + 0x40, 20);
        break;
    case CATEGORY_FILTER_ENEMY:
        RequestDma3Copy(gDeckFilterTabMap + 0xA + mode * 128, dst, 20);
        RequestDma3Copy(gDeckFilterTabMap + 0xA + 0x20 + mode * 128, dst + 0x40, 20);
        break;
    case CATEGORY_FILTER_ITEM:
        RequestDma3Copy(gDeckFilterTabMap + 0x14 + mode * 128, dst, 20);
        RequestDma3Copy(gDeckFilterTabMap + 0x14 + 0x20 + mode * 128, dst + 0x40, 20);
        break;
    case CATEGORY_FILTER_MAGIC:
        RequestDma3Copy(gDeckFilterTabMap + 0x40 + mode * 128, dst, 20);
        RequestDma3Copy(gDeckFilterTabMap + 0x40 + 0x20 + mode * 128, dst + 0x40, 20);
        break;
    case CATEGORY_FILTER_ATTACK:
        RequestDma3Copy(gDeckFilterTabMap + 0x4A + mode * 128, dst, 20);
        RequestDma3Copy(gDeckFilterTabMap + 0x4A + 0x20 + mode * 128, dst + 0x40, 20);
        break;
    }
}

void DrawDeckExchangeCardTotals() {
    u8 inDeckDigits[3];
    u8 collectionDigits[3];
    u16 inDeckCount;
    u16 collectionCount;

    u8* base;

    inDeckCount = CountCardsInDecks();
    collectionCount = CountCollectionCards();

    inDeckDigits[0] = inDeckCount / 100;
    inDeckDigits[1] = inDeckCount / 10 - inDeckDigits[0] * 10;
    inDeckDigits[2] = inDeckCount - inDeckDigits[0] * 100 - inDeckDigits[1] * 10;
    collectionDigits[0] = collectionCount / 100;
    collectionDigits[1] = collectionCount / 10 - collectionDigits[0] * 10;
    collectionDigits[2] = collectionCount - collectionDigits[0] * 100 - collectionDigits[1] * 10;
    base = GetBgCharBase(3);
    RequestDma3Copy(&gDeckCountDigitTiles[(inDeckDigits[0] + 1) * 32], base + 0x2A0, 32);
    RequestDma3Copy(&gDeckCountDigitTiles[(inDeckDigits[1] + 1) * 32], base + 0x2C0, 32);
    RequestDma3Copy(&gDeckCountDigitTiles[(inDeckDigits[2] + 1) * 32], base + 0x2E0, 32);
    RequestDma3Copy(&gDeckCountDigitTiles[(collectionDigits[0] + 1) * 32], base + 0x300, 32);
    RequestDma3Copy(&gDeckCountDigitTiles[(collectionDigits[1] + 1) * 32], base + 0x320, 32);
    RequestDma3Copy(&gDeckCountDigitTiles[(collectionDigits[2] + 1) * 32], base + 0x340, 32);
}

void LoadDeckExchangeDeckNameTexts(DeckExchangeWork* work) {
    InitTextSlots(work->textSlots, ARRAY_COUNT(work->textSlots));
    InitTextSlots(work->textSlots2, ARRAY_COUNT(work->textSlots2));
    InitTextSlots(work->textSlots3, ARRAY_COUNT(work->textSlots3));
    work->textSlotCount = LoadTextSlots(GetDeckName(0), work->textSlots);
    work->textSlotCount2 = LoadTextSlots(GetDeckName(1), work->textSlots2);
    work->textSlotCount3 = LoadTextSlots(GetDeckName(2), work->textSlots3);
}

void LoadDeckExchangeCardNameText(DeckExchangeWork* work, s32 id) {
    const CardDef* def;

    def = &gCardDefs[id];
    work->textSlotCount4 = LoadTextSlots(def->name, work->textSlots4);

    switch (def->category) {
    case CARD_CATEGORY_ATTACK:
        LoadPalette(gDeckMenuTextRedPalette, (void*)(work->palette4->index * 32 + OBJ_PLTT),
                    work->palette4->count << 5);
        break;
    case CARD_CATEGORY_MAGIC:
        LoadPalette(gDeckMenuTextBluePalette, (void*)(work->palette4->index * 32 + OBJ_PLTT),
                    work->palette4->count << 5);
        break;
    case CARD_CATEGORY_ITEM:
        LoadPalette(gDeckMenuTextGreenPalette, (void*)(work->palette4->index * 32 + OBJ_PLTT),
                    work->palette4->count << 5);
        break;
    case CARD_CATEGORY_ENEMY:
        LoadPalette(gDeckMenuTextGrayPalette, (void*)(work->palette4->index * 32 + OBJ_PLTT),
                    work->palette4->count << 5);
        break;
    }
}

void ShowDeckExchangeCardPreview(DeckExchangeWork* work) {
    DeckCard2Work* node;
    const CardDef* def;
    s32 id;
    u8 i;
    u8 j;
    void* dst;

    id = CARD_ID_NONE;
    node = ListPoolFirst(&work->pool);

    while (node != NULL) {
        if (node->args.row == work->cursorRow && node->args.col == work->cursorCol) {
            id = node->args.cardId;
            break;
        }

        node = ListPoolNext(&node->node);
    }

    ReleaseDeckExchangeCardPreview(work);

    if (id != CARD_ID_NONE) {
        def = &gCardDefs[id & CARD_ID_MASK];
        work->tiles4 = LoadObjTiles(gCardBacks[def->category].tiles, 0x300);
        work->tiles5 = LoadObjTiles(def->tiles, 0x200);
        work->palette3 = LoadObjPalette(def->palette, 32);
        work->palette2 = LoadObjPalette(gCard00Palette, sizeof(gCard00Palette));
        work->gfx3 = gCardBacks[def->category].gfx;
        work->gfx4 = def->gfx;

        for (i = 0; i < work->entryCount; i++) {
            if (work->entries[i].kind == def->kind) {
                break;
            }
        }

        work->entryIndex = i;
        dst = (void*)(BG_PLTT + 11 * PLTT_SIZE_4BPP);
        LoadPalette(&gCardCategoryPalettes[def->category * 16], dst, 32);

        for (j = 0; j < ARRAY_COUNT(work->entries[i].valueCounts); j++) {
            DrawValueCount(work->entries[i].valueCounts[j], j);
        }

        LoadDeckExchangeCardNameText(work, id);
        LoadDeckExchangeCardDescriptionText(work, id);

        if (def->kind > CARD_KIND_THE_KING) {
            LoadBgMap(2, gDeckCollectionEnemyInfoMap, sizeof(gDeckCollectionEnemyInfoMap));
            DrawDeckExchangeCpCost(0);
        } else {
            LoadBgMap(2, gDeckCollectionInfoMap, sizeof(gDeckCollectionInfoMap));
            DrawDeckExchangeCpCost(0);
        }
    } else {
        for (j = 0; j < 10; j++) {
            DrawValueCount(0, j);
        }

        DrawDeckExchangeCpCost(0);
    }
}

void ReleaseDeckExchangeCardPreview(DeckExchangeWork* work) {
    if (work->tiles7 != NULL) {
        ReleaseObjTiles(work->tiles7);
        work->tiles7 = NULL;
    }

    if (work->tiles4 != NULL) {
        ReleaseObjTiles(work->tiles4);
        ReleaseObjPalette(work->palette2);
        ReleaseObjTiles(work->tiles5);
        ReleaseObjPalette(work->palette3);

        if (work->tiles6 != NULL) {
            ReleaseObjTiles(work->tiles6);
            work->tiles6 = NULL;
        }

        work->tiles4 = NULL;
        work->palette2 = NULL;
        work->tiles5 = NULL;
        work->palette3 = NULL;
    }
}

void DrawDeckExchangeValueCpCost(DeckExchangeWork* work) {
    DrawDeckExchangeCpCost(GetCardCpCost(
        GetCardIdForKind(work->entries[work->entryIndex].kind) +
        work->cursorCol * 5 + (u16)work->cursorRow));
}

void DrawDeckExchangeCpCost(u8 cpCost) {
    u8 digits[2];
    u8* base;

    base = GetBgCharBase(3);

    if (cpCost != 0) {
        digits[0] = cpCost / 10;
        digits[1] = cpCost - digits[0] * 10;
        RequestDma3Copy(&gDeckValueDigitTiles[(digits[0] + 3) * 32], base + 0xCE0, 32);
        RequestDma3Copy(&gDeckValueDigitTiles[(digits[1] + 3) * 32], base + 0xD00, 32);
    } else {
        RequestDma3Copy(gDeckValueZeroTiles, base + 0xCE0, 32);
        RequestDma3Copy(gDeckValueZeroTiles, base + 0xD00, 32);
    }
}

u32 SumDeckExchangeValueCounts(u16* data) {
    u32 sum;
    u16* count;
    s32 i;

    sum = 0;
    count = data;
    i = 9;

    do {
        sum += *count++;
    } while (--i >= 0);

    return sum;
}

s32 MoveDeckExchangeValueCursor(DeckExchangeWork* work, u16 key) {
    u8* entry;
    u8 idx;
    u8 startCol;
    u8 startRow;
    u16 cursorCol;
    s32 sum;
    s32 i;
    s8 delta;
    s8 n;
    s32 ofs;
    s32 k;
    u8* cursorRow;

    idx = work->cursorCol * 5 + (u8)work->cursorRow;
    entry = (u8*)&work->entries[work->entryIndex];
    cursorCol = work->cursorCol;
    startCol = work->cursorCol;
    startRow = work->cursorRow;

    if (*(u16*)&entry[idx << 1] != 0) {
        return TRUE;
    }

    switch (key) {
    case DPAD_UP:
        do {
            if (work->cursorRow > 0) {
                work->cursorRow = work->cursorRow - 1;
            } else {
                work->cursorRow = 4;
            }

            idx = work->cursorCol * 5 + (u8)work->cursorRow;

            if (work->cursorCol == startCol && work->cursorRow == startRow) {
                return FALSE;
            }
        } while (*(u16*)&entry[idx << 1] == 0);

        break;
    case DPAD_DOWN:
        do {
            if (work->cursorRow <= 3) {
                work->cursorRow = work->cursorRow + 1;
            } else {
                work->cursorRow = 0;
            }

            idx = work->cursorCol * 5 + (u8)work->cursorRow;

            if (work->cursorCol == startCol && work->cursorRow == startRow) {
                return FALSE;
            }
        } while (*(u16*)&entry[idx << 1] == 0);

        break;
    case DPAD_LEFT:
        if (*(u16*)&entry[work->cursorRow << 1] != 0) {
            if ((s16)cursorCol > 0) {
                work->cursorCol = cursorCol - 1;
            }

            return TRUE;
        }

        sum = 0;

        for (i = 0; i < 5; i++) {
            sum += *(u16*)&entry[i * 2];
        }

        if (sum == 0) {
            work->cursorCol = 1;
            return FALSE;
        }

        cursorRow = (u8*)&work->cursorRow;
        delta = -1;
        k = *cursorRow + delta;

        for (;;) {
            n = k;

            if (n < 0) {
                n = 0;
            }

            if (n > 4) {
                n = 4;
            }

            ofs = n;

            if (*(u16*)&entry[ofs *= 2] != 0) {
                break;
            }

            if (delta < 0) {
                delta = -delta;
            } else {
                delta++;
                delta = -delta;
            }

            k = *cursorRow + delta;
        }

        work->cursorRow = n;
        break;
    case DPAD_RIGHT:
        if (*(u16*)&entry[(work->cursorRow + 5) << 1] != 0) {
            if ((s16)cursorCol <= 0) {
                work->cursorCol = cursorCol + 1;
            }

            return TRUE;
        }

        sum = 0;

        for (i = 5; i < 10; i++) {
            sum += *(u16*)&entry[i * 2];
        }

        if (sum == 0) {
            work->cursorCol = 0;
            return FALSE;
        }

        cursorRow = (u8*)&work->cursorRow;
        delta = -1;
        k = *cursorRow + delta;

        for (;;) {
            n = k;

            if (n < 0) {
                n = 0;
            }

            if (n > 4) {
                n = 4;
            }

            ofs = n;
            ofs *= 2;

            if (*(u16*)&entry[ofs += 10] != 0) {
                break;
            }

            if (delta < 0) {
                delta = -delta;
            } else {
                delta++;
                delta = -delta;
            }

            k = *cursorRow + delta;
        }

        work->cursorRow = n;
        break;
    case 0:
        do {
            if (work->cursorRow <= 3) {
                work->cursorRow = work->cursorRow + 1;
            } else {
                work->cursorRow = 0;
            }

            idx = work->cursorCol * 5 + (u8)work->cursorRow;

            if (work->cursorCol == startCol && work->cursorRow == startRow) {
                if (work->cursorCol <= 0) {
                    work->cursorCol = work->cursorCol + 1;
                } else {
                    work->cursorCol = 0;
                }

                if (SumDeckExchangeValueCounts((u16*)entry) == 0) {
                    return FALSE;
                }
            }
        } while (*(u16*)&entry[idx << 1] == 0);

        break;
    }

    return TRUE;
}

void FreeDeckExchangeCollectionEntries(DeckExchangeWork* work) {
    u16 i;

    if (work->entries != NULL) {
        for (i = 0; i < work->entryCount; i++) {
            EwramFree(work->entries[i].indices);
        }

        EwramFree(work->entries);
        work->entries = NULL;
    }
}

void SetDeckExchangeFrameCursor(DeckExchangeWork* work, u8 frameCursor) {
    switch (frameCursor) {
    case DECK_FRAME_CURSOR_CARD:
        SetObjTileSource(work->tiles2, gDeckCardCursorTiles);
        AnimInit(&work->anim2, gDeckCardCursorAnims, gDeckCardCursorFrames);
        AnimStart(&work->anim2, 0, ANIM_FLAG_LOOP);
        work->gfx2 = AnimGetGfx(&work->anim2);
        break;
    case DECK_FRAME_CURSOR_ROW:
        SetObjTileSource(work->tiles2, gDeckRowCursorTiles);
        AnimInit(&work->anim2, gDeckRowCursorAnims, gDeckRowCursorFrames);
        AnimStart(&work->anim2, 0, ANIM_FLAG_LOOP);
        work->gfx2 = AnimGetGfx(&work->anim2);
        break;
    }
}

s32 TakeTradeCard(DeckExchangeWork* work) {
    u16 idx;
    CardKindEntry* entry;
    u16 i;
    s32 card;
    u16 id;
    const CardDef* def;
    u16 value;

    idx = work->cursorCol * 5 + work->cursorRow;
    entry = &work->entries[work->entryIndex];

    if (entry->valueCounts[idx] == 0) {
        m4aSongNumStart(SONG_SYS_BEEP);
        return TRUE;
    }

    for (i = 0; i < entry->count; i++) {
        card = entry->indices[i];

        if (card != CARD_NONE) {
            id = gCardCollection[card] & CARD_ID_MASK;
            def = &gCardDefs[id];

            if (id > CARD_ID_LAST_VALUED) {
                if (idx == 0) {
                    gSioTradeCardId = gCardCollection[card] & CARD_ID_MASK;
                    ClearCardCollectionSlot(&gCardCollection[card]);
                    entry->indices[i] = CARD_NONE;
                    entry->valueCounts[0]--;
                    DrawValueCount(entry->valueCounts[0], 0);
                    m4aSongNumStart(SONG_SYS_KETTEI);
                    return TRUE;
                }
            } else {
                value = def->value;

                if (value == idx) {
                    gSioTradeCardId = gCardCollection[card] & CARD_ID_MASK;
                    ClearCardCollectionSlot(&gCardCollection[card]);
                    entry->indices[i] = CARD_NONE;
                    entry->valueCounts[value]--;
                    DrawValueCount(entry->valueCounts[value], value);
                    m4aSongNumStart(SONG_SYS_KETTEI);
                    return TRUE;
                }
            }
        }
    }

    m4aSongNumStart(SONG_SYS_BEEP);
    return TRUE;
}

s32 CheckDeckExchangeCpCost(DeckExchangeWork* work) {
    if (GetDeckCpCost(GetActiveDeckIndex()) > gGameState.progression.cp) {
        TaskCreate(&work->tasks2, &gTaskDescDeckErrorCp, &work->popupActive);
        m4aSongNumStart(SONG_SYS_BEEP);

        return FALSE;
    }

    return TRUE;
}

u8 CheckDeckExchangeHasAttackCard(DeckExchangeWork* work) {
    if (CountActiveDeckCardsOfCategory(CARD_CATEGORY_ATTACK) == 0) {
        m4aSongNumStart(SONG_SYS_BEEP);
        TaskCreate(&work->tasks2, &gTaskDescDeckErrorNoAttackCard, &work->popupActive);
        return FALSE;
    }

    return TRUE;
}

void ResetDeckExchangeGridScroll(DeckExchangeWork* work) {
    DeckCard2Work* node;
    s16 x;
    s16 y;

    node = ListPoolFirst(&work->pool);
    x = 0;
    y = 0;

    while (node != NULL) {
        node->args.col = x;
        node->args.row = y;
        x++;

        if (x > 2) {
            x = 0;
            y++;
        }

        node = ListPoolNext(&node->node);
    }

    work->thumbY = 0x2800;
    work->scrollRowEnd = 4;
}

u8 IsDeckExchangeCardAtCursor(DeckExchangeWork* work) {
    DeckCard2Work* node;

    node = ListPoolFirst(&work->pool);

    while (node != NULL) {
        if (node->args.col == work->cursorCol &&
            node->args.row == work->cursorRow) {
            return TRUE;
        }

        node = ListPoolNext(&node->node);
    }

    return FALSE;
}

u8 IsDeckExchangeCardAt(DeckExchangeWork* work, s16 x, s16 y) {
    DeckCard2Work* node;

    node = ListPoolFirst(&work->pool);

    while (node != NULL) {
        if (node->args.col == x && node->args.row == y) {
            return TRUE;
        }

        node = ListPoolNext(&node->node);
    }

    return FALSE;
}

u8 FindDeckExchangeCardInDirection(DeckExchangeWork* work, s16 x, s16 y, u16 dir) {
    DeckCard2Work* node;

    for (node = ListPoolFirst(&work->pool); node != NULL; node = ListPoolNext(&node->node)) {
        if (node->args.col == x && node->args.row == y) {
            return TRUE;
        }
    }

    switch (dir) {
    case DPAD_UP:
        return FindDeckExchangeCardInDirection(work, x, y - 1, DPAD_UP);
    case DPAD_DOWN:
        return FindDeckExchangeCardInDirection(work, x, y + 1, DPAD_DOWN);
    case DPAD_LEFT:
        return FindDeckExchangeCardInDirection(work, x - 1, y, DPAD_LEFT);
    case DPAD_RIGHT:
        return FindDeckExchangeCardInDirection(work, x + 1, y, DPAD_RIGHT);
    }

    return FALSE;
}

void LoadDeckExchangeCardDescriptionText(DeckExchangeWork* work, u16 index) {
    const CardDef* def;

    def = &gCardDefs[index];
    work->textSlotCount5 = LoadTextSlots((void*)gCardKindDescriptions[def->kind], work->textSlots5);
}

void SetDeckExchangeGridRowCount(DeckExchangeWork* work, s16 cardCount) {
    work->rowCount = cardCount / 3;

    if (cardCount % 3 != 0) {
        work->rowCount = cardCount / 3 + 1;
    }
}

void UpdateDeckExchangeGridScrollBar(DeckExchangeWork* work) {
    s32 rowStep;

    rowStep = 0x5400 / (work->rowCount - 4);
    work->thumbY = rowStep * (work->scrollRowEnd - 4) + 0x2800;

    if (work->thumbY > 0x7C00) {
        work->thumbY = 0x7C00;
    }

    if (work->thumbY < 0x2800) {
        work->thumbY = 0x2800;
    }
}
#endif

#ifndef VERSION_EU
TaskDesc gTaskDescDeckexchange = {
    "deckexchange",
    (TaskInitFunc)deckexchange_0,
    (TaskUpdateFunc)deckexchange_1,
    (TaskDrawFunc)deckexchange_2,
    (TaskDestroyFunc)deckexchange_3,
    sizeof(DeckExchangeWork),
};
#endif
