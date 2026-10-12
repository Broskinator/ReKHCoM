/**
 * sprite.c
 * Sprite Management
 */

#include "display.h"
#include "listpool.h"
#include "malloc.h"
#include "sprite.h"
#include "gba/io_reg.h"
#include <stddef.h>
#include "gba/oam.h"
#include "engine.h"
#include "gba/macro.h"
#include "obj.h"
#include "types.h"

static const u8 sSpriteHeapName[8] = "SPRITE";

SpriteWork* gSpriteWork;

void SpriteInit() {
    SetIwramHeapName(sSpriteHeapName);
    gSpriteWork = IwramAlloc(sizeof(SpriteWork));
    CpuFill32(0, gSpriteWork, sizeof(SpriteWork));
}

void SpriteFree() {
    IwramFree(gSpriteWork);
}

u16 GetObjTileCount(u16 attr0, u16 attr1) {
    switch ((((u32)attr1 << 16) | attr0) & OAM_SHAPE_SIZE(OAM_SHAPE_MASK, 3)) {
    case OAM_SHAPE_SIZE(OAM_SHAPE_SQUARE, 0):
        return 1;
    case OAM_SHAPE_SIZE(OAM_SHAPE_SQUARE, 2):
        return 0x10;
    case OAM_SHAPE_SIZE(OAM_SHAPE_SQUARE, 3):
        return 0x40;
    case OAM_SHAPE_SIZE(OAM_SHAPE_HORIZONTAL, 0):
    case OAM_SHAPE_SIZE(OAM_SHAPE_VERTICAL, 0):
        return 2;
    case OAM_SHAPE_SIZE(OAM_SHAPE_SQUARE, 1):
    case OAM_SHAPE_SIZE(OAM_SHAPE_HORIZONTAL, 1):
    case OAM_SHAPE_SIZE(OAM_SHAPE_VERTICAL, 1):
        return 4;
    case OAM_SHAPE_SIZE(OAM_SHAPE_HORIZONTAL, 2):
    case OAM_SHAPE_SIZE(OAM_SHAPE_VERTICAL, 2):
        return 8;
    case OAM_SHAPE_SIZE(OAM_SHAPE_HORIZONTAL, 3):
    case OAM_SHAPE_SIZE(OAM_SHAPE_VERTICAL, 3):
        return 0x20;
    }

    return 0;
}

void SortSpriteEntries(SpriteEntry** arr, s32 lo, s32 hi) {
    SpriteEntry* entry;
    u16 pivot;
    s32 i;
    s32 j;

    pivot = arr[(lo + hi) >> 1]->priority;
    i = lo;
    j = hi;

    for (;;) {
        while (arr[i]->priority < pivot) {
            i++;
        }

        while (pivot < arr[j]->priority) {
            j--;
        }

        if (i >= j) {
            break;
        }

        entry = arr[i];
        arr[i] = arr[j];
        arr[j] = entry;
        i++;
        j--;
    }

    if (lo < i - 1) {
        SortSpriteEntries(arr, lo, i - 1);
    }

    if (j + 1 < hi) {
        SortSpriteEntries(arr, j + 1, hi);
    }
}

void EnableObj() {
    gDispCnt |= DISPCNT_OBJ_ON;
}

void DisableObj() {
    gDispCnt &= ~DISPCNT_OBJ_ON;
}

void SetObjTileRange(u16 start, u16 count) {
    s32 end;

    gSpriteWork->tilePool.rangeStart = start;
    end = start + count;
    gSpriteWork->tilePool.rangeEnd = end;

    if ((u16)end > 0x400) {
        gSpriteWork->tilePool.rangeEnd = 0x400;
    }
}

void SetObjPaletteRange(u16 start, u16 count) {
    s32 end;

    gSpriteWork->palettePool.rangeStart = start;
    end = start + count;
    gSpriteWork->palettePool.rangeEnd = end;

    if ((u16)end > 0x10) {
        gSpriteWork->palettePool.rangeEnd = 0x10;
    }
}

void SpriteReset() {
    s32 i;

    EnableObj();
    ListPoolInit(&gSpriteWork->tilePool);

    for (i = 0; i < 128; i++) {
        ListPoolAddFree(&gSpriteWork->tiles[i].node, &gSpriteWork->tilePool, &gSpriteWork->tiles[i]);
    }

    ListPoolInit(&gSpriteWork->palettePool);

    for (i = 0; i < 16; i++) {
        ListPoolAddFree(&gSpriteWork->palettes[i].node, &gSpriteWork->palettePool,
                      &gSpriteWork->palettes[i]);
    }

    gSpriteWork->entryCount = 0;
    gSpriteWork->affineCount = 0;
    gSpriteWork->sortLo = 0;
    SetObjMosaicSize(0, 0);
    gSpriteWork->oamUpdatesPaused = FALSE;
    gSpriteWork->mosaicEnabled = FALSE;
    SetObjTileRange(0, 0x400);
    SetObjPaletteRange(0, 0x10);
}

u8 DrawSpriteSharedTiles(s16 x, s16 y, void* sprite, void* obj, void* palette, ObjAffine* affine, u16 flags, u16 priority) {
    SpriteWork* work;

    if (palette != NULL && sprite != NULL) {
        work = gSpriteWork;
#ifdef PLATFORM_PC
        work->entries[work->entryCount].x = x + gPortSpriteShiftX;
#else
        work->entries[work->entryCount].x = x;
#endif
#ifdef PLATFORM_PC
        work->entries[work->entryCount].y = y + gPortSpriteShiftY;
#else
        work->entries[work->entryCount].y = y;
#endif
        work->entries[work->entryCount].tiles = obj;
        work->entries[work->entryCount].palette = palette;
        work->entries[work->entryCount].affine = affine;
        work->entries[work->entryCount].flags = flags;
        work->entries[work->entryCount].priority = priority;
        work->entries[work->entryCount].sprite = sprite;
        work->sortPtrs[work->entryCount] = &work->entries[work->entryCount];
        work->entryCount += 1;
        return TRUE;
    }

    return FALSE;
}
