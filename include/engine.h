#ifndef GUARD_ENGINE_H
#define GUARD_ENGINE_H

#include "types.h"
#include "obj.h"
#include "macros.h"

struct SpriteWork;

typedef struct ObjTileListEntry {
    u16 attr0;
    u16 attr1;
    u16 tile;
} ObjTileListEntry;

typedef struct Dma3Request {
    const void* src;
    void* dst;
    u16 size;
} Dma3Request;

#define BG_ENTRY_COUNT 4
typedef struct BgEntry {
    u8 dirty;
    void* const* map;
    u8 width;
    u8 height;
    u16 x;
    u16 y;
#ifdef VERSION_EU
    void** decompressedMap;
#endif
} BgEntry;

#ifdef PLATFORM_PC
void PortLatchBgMaps(const BgEntry* entries);
void PortOamSetPosition(int index, int x, int y);
/* Wide view: how far the picture extends left/right of the GBA screen. */
int PortWideMarginX(void);
/* Horizontal display offset for a background / for sprites being drawn,
   used to dock screen-edge UI (message windows) to the wide view's edges. */
void PortSetBgShiftX(int bg, int shift);
void PortDockBg(int bg, int shift);
extern int gPortSpriteShiftX;
extern int gPortSpriteShiftY;
int PortWideMarginY(void);
/* Anchor what is drawn until PortHudEnd() to a side of the wide view:
   ax/ay are -1 (left/top), 0 (stay) or 1 (right/bottom). */
void PortHudBegin(int ax, int ay);
/* Same, choosing the side from where (x, y) lies on the GBA screen: for
   pointers and highlights that refer to an anchored HUD element. */
void PortHudBeginAt(int x, int y);
/* Bookkeeping for the HUD layer: the sprite entry just queued, and which
   entry an OAM slot was built from. */
void PortSpriteTag(int entry);
void PortOamSetSource(int oamIndex, int entry);
/* Start of a new OAM list: forget which entry each slot came from. */
void PortOamTagsReset(void);
/* HD text: the pixel-font character of the sprites being drawn (0: none). */
extern int gPortTextChar;
void PortHudEnd(void);
/* One-button dodge roll: the d-pad direction to roll toward, or 0. */
int PortTakeDodge(int held, int facingLeft);
/* Wide view for tilemaps the game streams into VRAM from its own world data:
   fn returns the screen entry of world tile (tx, ty) of a background whose
   scroll registers show world pixel (originX, originY) at the screen's
   top-left. NULL clears it. */
typedef int (*PortWorldTileFunc)(int bg, int tx, int ty);
void PortSetBgWorld(int bg, PortWorldTileFunc fn, int originX, int originY);
#endif

typedef struct BgWork {
    BgEntry entries[BG_ENTRY_COUNT];
} BgWork;

typedef struct Dma3Pending {
    void* dst;
    u16 size;
} Dma3Pending;

typedef struct Dma3Blit {
    void* src;
    void* dst;
    u8 srcX;
    u8 srcY;
    u8 dstX;
    u8 dstY;
    u8 width;
    u8 height;
} Dma3Blit;

typedef struct Dma3Fill {
    void* src;
    void* dst;
    u8 x;
    u8 y;
    u8 vertical;
} Dma3Fill;

typedef struct Dma3Queue {
    Dma3Request requests[256];
    Dma3Blit blits[64];
    Dma3Fill fills[8];
    void (*callbacks[8])();
    Dma3Pending pending[4];
#ifdef VERSION_EU
    Dma3Request lz77Requests[32];
#endif
    vu16 requestCount;
    vu16 blitCount;
    vu16 fillCount;
    vu16 callbackCount;
    vu16 count;
    vu16 lz77RequestCount;
    u32 transferredBytes;
} Dma3Queue;

typedef struct SpriteWork SpriteWork;
extern SpriteWork* gSpriteWork;
extern Dma3Queue* gDma3Requests;

enum FadeFlag {
    FADE_FLAG_ACTIVE = 0x1,
    FADE_FLAG_LOCKED = 0x2,
    FADE_FLAG_PAUSED = 0x4
};

typedef struct FadeWork {
    PaletteSlot slots[32];
    u32 amount;
    u32 target;
    u32 lastAmount;
    u16 timer;
    u32 mode;
    u16 flags;
} FadeWork;

STATIC_ASSERT(sizeof(FadeWork) == 0x598, FadeWorkSize);
STATIC_ASSERT(sizeof(PaletteSlot) == 0x2C, PaletteSlotSize);

typedef struct Spline2D {
    s16 pointCount;
    s32* intervals;
    s32* scratch;
    s32* knots;
    s32* xCoefficients;
    s32* yCoefficients;
    s32* xValues;
    s32* yValues;
} Spline2D;

STATIC_ASSERT(sizeof(Spline2D) == 0x20, Spline2DSize);

void SplineBuildAxisCoefficients(Spline2D* spline, s32* knots, s32* values, s32* coefficients);
s32 SplineEvaluateAxis(s16* pointCount, s32 position, s32* knots, s32* values, s32* coefficients);
void SplineInit2D(Spline2D* spline, s32* xValues, s32* yValues, s16 pointCount);
void SplineEvaluate2D(Spline2D* spline, s32 position, s32* outX, s32* outY);
void SplineFreeBuffers(Spline2D* spline);

u8 DrawSpriteAllocatedTiles(s16 x, s16 y, void* sprite, void* obj, void* palette, ObjAffine* affine, u16 flags, u16 priority);
u8 DrawSpriteFrameTiles(s16 x, s16 y, void* obj, void* palette, ObjAffine* affine, u16 flags, u16 priority);
void ReleaseSharedObjTiles(ObjTiles* tiles);
void ReleaseAllocatedObjTiles(ObjTiles* tiles);
void ReleaseSpriteFrameTiles(ObjTiles* tiles);
void ReleaseObjPaletteRef(ObjPalette* palette);
void SetSpriteMosaicEnabled(u8 enabled);
void VTransFree();
void BgFree();
void SetBgMosaic(s32 bg, u8 on);
void FadeFree();
u8 GetBgScrollX(u32 bg);
u8 GetBgScrollY(u32 bg);
void InitDynamicObjTilesAtSlot(ObjTiles* tiles, u16 slot, u16 size, void* src);
void* GetBgMapBlock(BgEntry* entry, u16 x, u16 y);
void SetSpriteOamUpdatesPaused(u8 paused);
void SetBgMosaicSize(u8 x, u8 y);
s32 Lerp8(s32 from, s32 to, s32 weight);

void VTransInit();
void BgInit();
void FadeInit();
void InitDisplayRegs();

#endif /* GUARD_ENGINE_H */
