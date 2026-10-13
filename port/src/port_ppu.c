/*
 * Software GBA PPU.
 *
 * Renders the picture described by the I/O registers, palette, VRAM and OAM
 * into a 32-bit frame buffer. The output can be larger than the GBA's
 * 240x160: GBA pixel (0,0) is placed at (originX, originY) and background
 * layers, sprites and full-screen windows continue past the original edges
 * when the current scene supports a wide view (gPortScreen.wide).
 *
 * Per-line state changes (HBlank interrupts and HBlank DMA) are honoured for
 * the 160 original lines; lines added above and below reuse the state of the
 * nearest original line.
 */
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "port/port.h"
#include "engine.h"
#include "port/port_state.h"

extern const void* gMsgwinMapBlocks[4];

PortScreen gPortScreen = { GBA_SCREEN_W, GBA_SCREEN_H, 0, 0, 0 };

/* Area the game should keep sprites alive in (see VIEW_* in gba/defines.h). */
int gPortViewLeft = 0;
int gPortViewTop = 0;
int gPortViewRight = GBA_SCREEN_W;
int gPortViewBottom = GBA_SCREEN_H;

/* Full-range sprite positions written alongside OAM by UpdateSpriteOam. */
typedef struct OamExt {
    int16_t x;
    int16_t y;
} OamExt;
static OamExt sOamExt[128];

/* Map-block backgrounds as they were when the scroll registers were last
   committed: lets the wide view draw the world beyond the VRAM window. */
typedef struct BgLatch {
    void* const* map;
    int width;
    int height;
    int x;
    int y;
    uint16_t hofs;
    uint16_t vofs;
    int valid;
} BgLatch;
static BgLatch sBgLatch[4];

/* Display offsets requested by the game for docking UI (see engine.h). */
int gPortSpriteShiftX;
typedef struct BgWorld {
    PortWorldTileFunc fn;
    int x;
    int y;
    uint16_t hofs;
    uint16_t vofs;
    int valid;
} BgWorld;
static BgWorld sBgWorld[4];
static BgWorld sBgWorldLatched[4];

void PortSetBgWorld(int bg, PortWorldTileFunc fn, int originX, int originY) {
    if (bg >= 0 && bg < 4) {
        sBgWorld[bg].fn = fn;
        sBgWorld[bg].x = originX;
        sBgWorld[bg].y = originY;
    }
}

static int sBgShift[4];
static int sBgShiftLatched[4];
static int sBgDocked[4];
static int sBgDockedLatched[4];

/* A docked layer is screen UI moved toward an edge: it shows only what the
   hardware would (no world-map continuation), so parked windows stay hidden. */
static int sBgSplit[4]; /* PortBgSplit, below */

void PortPpuVisitState(PortStateVisitor visit) {
    visit(sBgLatch, sizeof(sBgLatch));
    visit(sBgShift, sizeof(sBgShift));
    visit(sBgShiftLatched, sizeof(sBgShiftLatched));
    visit(sBgDocked, sizeof(sBgDocked));
    visit(sBgDockedLatched, sizeof(sBgDockedLatched));
    visit(sOamExt, sizeof(sOamExt));
    visit(sBgWorld, sizeof(sBgWorld));
    visit(sBgWorldLatched, sizeof(sBgWorldLatched));
    visit(sBgSplit, sizeof(sBgSplit));
}

void PortSetBgShiftX(int bg, int shift) {
    if (bg >= 0 && bg < 4) {
        sBgShift[bg] = shift;
        sBgDocked[bg] = 0;
    }
}

void PortDockBg(int bg, int shift) {
    if (bg >= 0 && bg < 4) {
        sBgShift[bg] = shift;
        sBgDocked[bg] = 1;
    }
}

int PortWideMarginX(void) {
    return gPortScreen.wide ? gPortScreen.originX : 0;
}

int gPortSpriteShiftY;

int PortWideMarginY(void) {
    return gPortScreen.wide ? gPortScreen.originY : 0;
}

/* Anchors nest (a HUD task may draw child tasks), so keep a small stack. */
static int sHudStack[16][4];
static int sHudDepth;
static int sHudAx, sHudAy;

void PortHudBegin(int ax, int ay) {
    if (sHudDepth < 16) {
        sHudStack[sHudDepth][0] = gPortSpriteShiftX;
        sHudStack[sHudDepth][1] = gPortSpriteShiftY;
        sHudStack[sHudDepth][2] = sHudAx;
        sHudStack[sHudDepth][3] = sHudAy;
    }
    sHudDepth++;
    sHudAx = ax;
    sHudAy = ay;
    gPortSpriteShiftX = ax * PortWideMarginX();
    gPortSpriteShiftY = ay * PortWideMarginY();
}

void PortHudEnd(void) {
    if (sHudDepth > 0)
        sHudDepth--;
    if (sHudDepth < 16) {
        gPortSpriteShiftX = sHudStack[sHudDepth][0];
        gPortSpriteShiftY = sHudStack[sHudDepth][1];
        sHudAx = sHudStack[sHudDepth][2];
        sHudAy = sHudStack[sHudDepth][3];
    }
}

/* Pointers/highlights that refer to HUD elements: anchor by screen zone,
   matching the corners the HUD itself is docked to. */
void PortHudBeginAt(int x, int y) {
    int ax = x < 80 ? -1 : (x >= 160 ? 1 : 0);
    int ay = y < 54 ? -1 : (y >= 80 ? 1 : 0);

    PortHudBegin(ax, ay);
}

/* Which sprite entries / OAM slots belong to the HUD, and their anchor. */
typedef struct SpriteTag {
    uint8_t hud;
    int8_t ax;
    int8_t ay;
    uint16_t text; /* HD text: pixel-font character, 0 for other sprites */
    uint8_t screen; /* drawn by a menu: goes on the screen layer */
} SpriteTag;
static SpriteTag sEntryTag[128];
static SpriteTag sOamTag[128];
static int sHudLayer;
static int sHdText;
int gPortTextChar;

/* ---- screen layer ---------------------------------------------------------------
 * Menus over the world (pause, save, world select, level up): their sprites
 * (drawn between PortScreenBegin/End) and the layers that are not world maps
 * are composed into gPortOverlay, gPortOverlayW x 160, which the presenter
 * scales to the window's height; the world keeps its own scale underneath.
 * The game marks every frame it draws such a menu.
 */
uint32_t gPortOverlay[PORT_MAX_W * GBA_SCREEN_H];
int gPortOverlayW = GBA_SCREEN_W;
int gPortScreenLayer; /* this frame uses the screen layer (set by the presenter) */
static int sScreenDepth, sScreenMark, sOverlay;
static int sScreenBg[4];
static uint16_t sScrLine[PORT_MAX_W];
static uint8_t sScrPrio[PORT_MAX_W];
static uint8_t sScrSemi[PORT_MAX_W];

void PortScreenBegin(void) {
    sScreenDepth++;
    sScreenMark = 1;
}

void PortScreenEnd(void) {
    if (sScreenDepth > 0)
        sScreenDepth--;
}

int PortTakeScreenMark(void) {
    int m = sScreenMark;
    sScreenMark = 0;
    return m;
}

/* Screen-layer BG split (as in the Vita port): the columns left of `column`
   move into the left margin, the rest into the right one. 0: off. */

void PortBgSplit(int bg, int column) {
    if (bg >= 0 && bg < 4)
        sBgSplit[bg] = column;
}

int PortScreenMarginX(void) {
    return (gPortOverlayW - GBA_SCREEN_W) / 2;
}

int gPortHudScale;
int gPortHudW;
int gPortHudH;

void PortSpriteTag(int entry) {
    if (entry >= 0 && entry < 128) {
        sEntryTag[entry].hud = sHudDepth > 0;
        sEntryTag[entry].ax = (int8_t)sHudAx;
        sEntryTag[entry].ay = (int8_t)sHudAy;
        sEntryTag[entry].text = (uint16_t)gPortTextChar;
        sEntryTag[entry].screen = sScreenDepth > 0;
    }
}

void PortOamTagsReset(void) {
    memset(sOamTag, 0, sizeof(sOamTag));
}

/* Characters the outline font draws: printable Windows-1252 codes. The codes
   below 0x20 are icons (arrows, button shapes) and stay pixel art. */
static int HdTextGlyph(int oamIndex) {
    int c = sOamTag[oamIndex].text;

    if (!sHdText || c <= 0x20 || c > 0xFF || c == 0x7F)
        return 0;
    return c != 0x81 && c != 0x8D && c != 0x8F && c != 0x90 && c != 0x9D;
}

void PortOamSetSource(int oamIndex, int entry) {
    if (oamIndex >= 0 && oamIndex < 128 && entry >= 0 && entry < 128)
        sOamTag[oamIndex] = sEntryTag[entry];
}

int PortPpuOverlayActive(void) {
    return sOverlay;
}

int PortPpuHudActive(void) {
    return sHudLayer;
}
uint32_t gPortFrame[PORT_MAX_W * PORT_MAX_H];

#define PLTT16 ((const uint16_t*)(uintptr_t)GBA_PLTT)
#define VRAM8 ((const uint8_t*)(uintptr_t)GBA_VRAM)
#define OAM16 ((const uint16_t*)(uintptr_t)GBA_OAM)

/* Line buffers hold BGR555 colours; bit 15 marks a transparent pixel. */
#define TRANSPARENT 0x8000

enum {
    LAYER_BG0,
    LAYER_BG1,
    LAYER_BG2,
    LAYER_BG3,
    LAYER_OBJ,
    LAYER_BD,
};

#define WIN_BG(n) (1 << (n))
#define WIN_OBJ 0x10
#define WIN_SFX 0x20

static uint16_t sBgLine[4][PORT_MAX_W];
static uint16_t sObjLine[PORT_MAX_W];
static uint8_t sObjPrio[PORT_MAX_W];
static uint8_t sObjIndex[PORT_MAX_W];
/* HUD layer: what covers each canvas pixel in the GBA's own compositing, so
   HUD sprites stay behind what the cartridge draws in front of them: the
   highest-priority opaque BG (4: none) and the winning other OBJ as
   priority * 128 + OAM index (0xFFFF: none). A window that hides OBJs there
   reads as 0 / 0. */
static uint8_t sOccBg[PORT_MAX_W * PORT_MAX_H];
static uint16_t sOccObj[PORT_MAX_W * PORT_MAX_H];
static uint8_t sObjSemi[PORT_MAX_W];
static uint8_t sObjWin[PORT_MAX_W];
static uint8_t sWinMask[PORT_MAX_W];
/* Wide view: 1 where some layer has world data (or inside the GBA picture);
   the backdrop shows black elsewhere, i.e. beyond the edge of the world. */
static uint8_t sInWorld[PORT_MAX_W];

/* Affine background reference points (internal registers). */
static int32_t sAffX[2], sAffY[2];
static uint32_t sAffLatchX[2], sAffLatchY[2];

static uint32_t sColorLut[0x8000];
static int sLutMode = -1;

/* Visible span of the current frame in GBA coordinates. */
static int sLeft, sRight, sTop, sBottom;

static int32_t Sext28(uint32_t v) {
    return (int32_t)(v << 4) >> 4;
}

static void BuildLut(void) {
    int c;

    if (sLutMode == gPortConfig.colorCorrection)
        return;
    sLutMode = gPortConfig.colorCorrection;
    for (c = 0; c < 0x8000; c++) {
        int r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
        int R, G, B;

        if (sLutMode) {
            /* Approximation of the GBA's darker, desaturated LCD. */
            double lr = r / 31.0, lg = g / 31.0, lb = b / 31.0;
            double gr = lr * lr * 0.84 + lg * lg * 0.18 + lb * lb * 0.0;
            double gg = lr * lr * 0.09 + lg * lg * 0.67 + lb * lb * 0.26;
            double gb = lr * lr * 0.15 + lg * lg * 0.10 + lb * lb * 0.73;
            R = (int)(255.0 * (gr > 1 ? 1 : gr) + 0.5);
            G = (int)(255.0 * (gg > 1 ? 1 : gg) + 0.5);
            B = (int)(255.0 * (gb > 1 ? 1 : gb) + 0.5);
            /* gamma back up so the result is not overly dark */
            {
                R = (int)(sqrt(R / 255.0) * 255.0 + 0.5);
                G = (int)(sqrt(G / 255.0) * 255.0 + 0.5);
                B = (int)(sqrt(B / 255.0) * 255.0 + 0.5);
            }
        } else {
            R = (r << 3) | (r >> 2);
            G = (g << 3) | (g >> 2);
            B = (b << 3) | (b >> 2);
        }
        sColorLut[c] = 0xFF000000u | ((uint32_t)R << 16) | ((uint32_t)G << 8) | (uint32_t)B;
    }
}

void PortPpuSetResolution(int w, int h) {
    if (w > PORT_MAX_W)
        w = PORT_MAX_W;
    if (h > PORT_MAX_H)
        h = PORT_MAX_H;
    if (w < GBA_SCREEN_W)
        w = GBA_SCREEN_W;
    if (h < GBA_SCREEN_H)
        h = GBA_SCREEN_H;
    gPortScreen.width = w;
    gPortScreen.height = h;
    gPortScreen.originX = (w - GBA_SCREEN_W) / 2;
    gPortScreen.originY = (h - GBA_SCREEN_H) / 2;
    gPortViewLeft = -gPortScreen.originX;
    gPortViewTop = -gPortScreen.originY;
    gPortViewRight = w - gPortScreen.originX;
    gPortViewBottom = h - gPortScreen.originY;
}

/* Development aid (KHCOM_OAMDUMP): log the sprites of the current frame. */
void PortPpuDumpOam(void) {
    int i;

    for (i = 0; i < 128; i++) {
        uint16_t a0 = OAM16[i * 4], a1 = OAM16[i * 4 + 1], a2 = OAM16[i * 4 + 2];
        if ((a0 & 0x300) == 0x200)
            continue;
        PortLog("oam %3d ext(%4d,%4d) raw(%3d,%3d) shape %d size %d prio %d pal %2d tile %4d mode %d", i,
                sOamExt[i].x, sOamExt[i].y, a1 & 0x1FF, a0 & 0xFF, a0 >> 14, a1 >> 14, (a2 >> 10) & 3, a2 >> 12,
                a2 & 0x3FF, (a0 >> 10) & 3);
    }
}

void PortOamSetPosition(int index, int x, int y) {
    if (index >= 0 && index < 128) {
        sOamExt[index].x = (int16_t)x;
        sOamExt[index].y = (int16_t)y;
    }
}

void PortLatchBgMaps(const BgEntry* entries) {
    int b;

    for (b = 0; b < 4; b++) {
        BgLatch* l = &sBgLatch[b];
        l->map = entries[b].map;
        l->width = entries[b].width;
        l->height = entries[b].height;
        l->x = entries[b].x;
        l->y = entries[b].y;
        l->hofs = IO16(0x10 + b * 4) & 0x1FF;
        l->vofs = IO16(0x12 + b * 4) & 0x1FF;
        sBgShiftLatched[b] = sBgShift[b];
        sBgWorldLatched[b] = sBgWorld[b];
        sBgWorldLatched[b].hofs = l->hofs;
        sBgWorldLatched[b].vofs = l->vofs;
        sBgDockedLatched[b] = sBgDocked[b];
    }
}

static int sBattleView;

/* Battle: the card hand sits partly below the GBA screen; the sprites are
   whole, so the bottom HUD can be lifted to show more of them. */
static int HudRaise(void) {
    int r = gPortConfig.cardHudRaise;

    return !sBattleView || r < 0 ? 0 : (r > 16 ? 16 : r);
}

void PortSetBattleView(int on) {
    sBattleView = on;
}

/* Screen entry of the world map at world pixel (wx, wy); -1 outside it. */
static int WorldEntry(const BgLatch* l, int wx, int wy) {
    const uint16_t* block;

    /* battle stages continue sideways instead of ending in the void */
    if (sBattleView && l->width > 0 && (wx < 0 || wx >= l->width * 256))
        wx = ((wx % (l->width * 256)) + l->width * 256) % (l->width * 256);

    if (l->map == NULL || wx < 0 || wy < 0 || wx >= l->width * 256 || wy >= l->height * 256)
        return -1;
    block = (const uint16_t*)l->map[(wy >> 8) * l->width + (wx >> 8)];
    /* Blocks live in work RAM or the ROM; anything else is not a world map. */
    if ((uintptr_t)block < 0x02000000u || (uintptr_t)block >= 0x0A000000u)
        return -1;
    return block[((wy & 255) >> 3) * 32 + ((wx & 255) >> 3)];
}

static int VramEntry(int bg, int gx, int gy) {
    uint16_t cnt = IO16(0x08 + bg * 2);
    const uint16_t* screen = (const uint16_t*)(VRAM8 + ((cnt >> 8) & 31) * 0x800);
    int size = cnt >> 14;
    int px = (gx + (IO16(0x10 + bg * 4) & 0x1FF)) & ((size & 1) ? 511 : 255);
    int py = (gy + (IO16(0x12 + bg * 4) & 0x1FF)) & ((size & 2) ? 511 : 255);
    int tx = px >> 3, ty = py >> 3, block = 0;

    if (tx >= 32)
        block += 1;
    if (ty >= 32)
        block += (size == 3) ? 2 : 1;
    return screen[block * 0x400 + (ty & 31) * 32 + (tx & 31)];
}

/* A map-block BG can be extended when what VRAM shows is the world map. */
static int ValidateBgLatch(int bg, uint16_t dispcnt) {
    static const int xs[3] = { 8, 120, 232 }, ys[3] = { 8, 80, 152 };
    BgLatch* l = &sBgLatch[bg];
    int mode = dispcnt & 7, i, j, match = 0;

    if (!(dispcnt & (0x100 << bg)) || l->map == NULL || l->width == 0 || l->height == 0)
        return 0;
    /* Message window layers are screen UI, never part of the world. */
    for (i = 0; i < 4; i++)
        if ((const void*)l->map == gMsgwinMapBlocks[i])
            return 0;
    if (mode > 1 || (mode == 1 && bg == 2))
        return 0;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            int we = WorldEntry(l, l->x + xs[i], l->y + ys[j]);
            if (we >= 0 && we == VramEntry(bg, xs[i], ys[j]))
                match++;
        }
    }
    return match >= 7;
}

/* Same check for a game-provided world (cell rooms). */
static int ValidateBgWorld(int bg, uint16_t dispcnt) {
    static const int xs[3] = { 8, 120, 232 }, ys[3] = { 8, 80, 152 };
    BgWorld* w = &sBgWorldLatched[bg];
    int mode = dispcnt & 7, i, j, match = 0;

    if (!(dispcnt & (0x100 << bg)) || w->fn == NULL || mode > 1 || (mode == 1 && bg == 2))
        return 0;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            if (w->fn(bg, (w->x + xs[i]) >> 3, (w->y + ys[j]) >> 3) == VramEntry(bg, xs[i], ys[j]))
                match++;
    return match >= 7;
}

/* ---- backgrounds --------------------------------------------------------- */

static void DrawTextBg(int bg, int gy, uint16_t dispcnt) {
    uint16_t cnt = IO16(0x08 + bg * 2);
    int hofs = IO16(0x10 + bg * 4) & 0x1FF;
    int vofs = IO16(0x12 + bg * 4) & 0x1FF;
    const uint8_t* chars = VRAM8 + ((cnt >> 2) & 3) * 0x4000;
    const uint16_t* screen = (const uint16_t*)(VRAM8 + ((cnt >> 8) & 31) * 0x800);
    int size = cnt >> 14;
    int wmask = (size & 1) ? 511 : 255;
    int hmask = (size & 2) ? 511 : 255;
    int bpp8 = (cnt >> 7) & 1;
    int mosaic = (cnt >> 6) & 1;
    int mosH = mosaic ? (IO16(0x4C) & 15) + 1 : 1;
    int mosV = mosaic ? ((IO16(0x4C) >> 4) & 15) + 1 : 1;
    uint16_t* out = sBgLine[bg];
    int x, py, ty, fy, outside;
    int w = gPortScreen.width;

    (void)dispcnt;
    if (mosV > 1)
        gy -= ((gy % mosV) + mosV) % mosV;
    py = (gy + vofs) & hmask;
    ty = py >> 3;
    fy = py & 7;
    outside = gy < 0 || gy >= GBA_SCREEN_H;

    for (x = 0; x < w; x++) {
        int gx = x - gPortScreen.originX - sBgShiftLatched[bg];
        int px, tx, block, entry, tile, pal, fx, fyy, ci;
        const uint16_t* map;

        if (x - gPortScreen.originX < sLeft || x - gPortScreen.originX >= sRight) {
            out[x] = TRANSPARENT;
            continue;
        }
        if (mosH > 1)
            gx -= ((gx % mosH) + mosH) % mosH;
        if (outside || gx < 0 || gx >= GBA_SCREEN_W) {
            /* Beyond the hardware picture: only map-block layers continue. */
            const BgLatch* l = &sBgLatch[bg];
            int we;

            if (sBgDockedLatched[bg] || (!l->valid && !sBgWorldLatched[bg].valid)) {
                out[x] = TRANSPARENT;
                continue;
            }
            if (!l->valid) {
                const BgWorld* wd = &sBgWorldLatched[bg];
                int wx = wd->x + gx + (hofs - wd->hofs), wy = wd->y + gy + (vofs - wd->vofs);
                we = wd->fn(bg, wx >> 3, wy >> 3);
                if (we < 0) {
                    out[x] = TRANSPARENT;
                    continue;
                }
                sInWorld[x] = 1;
                entry = we;
                px = wx & 7;
                fy = wy & 7;
                tile = entry & 0x3FF;
                fx = (entry & 0x400) ? 7 - px : px;
                fyy = (entry & 0x800) ? 7 - fy : fy;
                fy = py & 7;
                goto pixel;
            }
            we = WorldEntry(l, l->x + gx + (hofs - l->hofs), l->y + gy + (vofs - l->vofs));
            if (we < 0) {
                out[x] = TRANSPARENT;
                continue;
            }
            sInWorld[x] = 1;
            entry = we;
            px = (l->x + gx + (hofs - l->hofs)) & 7;
            fy = (l->y + gy + (vofs - l->vofs)) & 7;
            tile = entry & 0x3FF;
            fx = (entry & 0x400) ? 7 - px : px;
            fyy = (entry & 0x800) ? 7 - fy : fy;
            fy = py & 7;
            goto pixel;
        }
        px = (gx + hofs) & wmask;
        tx = px >> 3;
        block = 0;
        if (tx >= 32)
            block += 1;
        if (ty >= 32)
            block += (size == 3) ? 2 : 1;
        map = screen + block * 0x400;
        entry = map[(ty & 31) * 32 + (tx & 31)];
        tile = entry & 0x3FF;
        fx = (entry & 0x400) ? 7 - (px & 7) : (px & 7);
        fyy = (entry & 0x800) ? 7 - fy : fy;
    pixel:
        if (bpp8) {
            uint32_t off = tile * 64 + fyy * 8 + fx;
            ci = (((cnt >> 2) & 3) * 0x4000 + off < 0x10000) ? chars[off] : 0;
            out[x] = ci ? (PLTT16[ci] & 0x7FFF) : TRANSPARENT;
        } else {
            uint32_t off = tile * 32 + fyy * 4 + (fx >> 1);
            uint8_t b = (((cnt >> 2) & 3) * 0x4000 + off < 0x10000) ? chars[off] : 0;
            ci = (fx & 1) ? (b >> 4) : (b & 15);
            pal = entry >> 12;
            out[x] = ci ? (PLTT16[pal * 16 + ci] & 0x7FFF) : TRANSPARENT;
        }
    }
}

static void DrawAffineBg(int bg, uint16_t dispcnt) {
    int a = bg - 2;
    uint16_t cnt = IO16(0x08 + bg * 2);
    int16_t pa = (int16_t)IO16(0x20 + a * 0x10);
    int16_t pc = (int16_t)IO16(0x24 + a * 0x10);
    const uint8_t* chars = VRAM8 + ((cnt >> 2) & 3) * 0x4000;
    const uint8_t* screen = VRAM8 + ((cnt >> 8) & 31) * 0x800;
    int size = 128 << (cnt >> 14);
    int tiles = size >> 3;
    int wrap = (cnt >> 13) & 1;
    uint16_t* out = sBgLine[bg];
    int x, w = gPortScreen.width;
    int32_t rx = sAffX[a] + (int32_t)pa * (-gPortScreen.originX);
    int32_t ry = sAffY[a] + (int32_t)pc * (-gPortScreen.originX);

    (void)dispcnt;
    for (x = 0; x < w; x++, rx += pa, ry += pc) {
        int gx = x - gPortScreen.originX;
        int px = rx >> 8, py = ry >> 8;
        int tile, ci;

        if (gx < sLeft || gx >= sRight) {
            out[x] = TRANSPARENT;
            continue;
        }
        if (wrap) {
            px &= size - 1;
            py &= size - 1;
        } else if ((px < 0 || px >= size) && py >= 0 && py < size && size >= 512 && (gx < 0 || gx >= GBA_SCREEN_W)) {
            /* wide margins of a battle arena: the stage repeats sideways
               instead of ending in the void */
            px &= size - 1;
        } else if (px < 0 || py < 0 || px >= size || py >= size) {
            out[x] = TRANSPARENT;
            continue;
        }
        sInWorld[x] = 1;
        tile = screen[(py >> 3) * tiles + (px >> 3)];
        ci = chars[tile * 64 + (py & 7) * 8 + (px & 7)];
        out[x] = ci ? (PLTT16[ci] & 0x7FFF) : TRANSPARENT;
    }
}

static void DrawBitmapBg(int mode, uint16_t dispcnt) {
    int16_t pa = (int16_t)IO16(0x20);
    int16_t pc = (int16_t)IO16(0x24);
    uint16_t* out = sBgLine[2];
    int x, w = gPortScreen.width;
    int bw = (mode == 5) ? 160 : 240;
    int bh = (mode == 5) ? 128 : 160;
    uint32_t page = (mode != 3 && (dispcnt & 0x10)) ? 0xA000 : 0;
    int32_t rx = sAffX[0] + (int32_t)pa * (-gPortScreen.originX);
    int32_t ry = sAffY[0] + (int32_t)pc * (-gPortScreen.originX);

    for (x = 0; x < w; x++, rx += pa, ry += pc) {
        int gx = x - gPortScreen.originX;
        int px = rx >> 8, py = ry >> 8;

        if (gx < sLeft || gx >= sRight || px < 0 || py < 0 || px >= bw || py >= bh) {
            out[x] = TRANSPARENT;
            continue;
        }
        if (mode == 4) {
            int ci = VRAM8[page + py * 240 + px];
            out[x] = ci ? (PLTT16[ci] & 0x7FFF) : TRANSPARENT;
        } else {
            out[x] = *(const uint16_t*)(VRAM8 + page + (py * bw + px) * 2) & 0x7FFF;
        }
    }
}

/* ---- sprites --------------------------------------------------------------- */

static const uint8_t sObjSize[3][4][2] = {
    { { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 } },
    { { 16, 8 }, { 32, 8 }, { 32, 16 }, { 64, 32 } },
    { { 8, 16 }, { 8, 32 }, { 16, 32 }, { 32, 64 } },
};

static void DrawObjs(int gy, uint16_t dispcnt) {
    int i, w = gPortScreen.width;
    int mapping1d = (dispcnt >> 6) & 1;
    int bitmapMode = (dispcnt & 7) >= 3;
    uint16_t mosaic = IO16(0x4C);
    int mosH = ((mosaic >> 8) & 15) + 1;
    int mosV = ((mosaic >> 12) & 15) + 1;

    for (i = 0; i < w; i++) {
        sObjLine[i] = TRANSPARENT;
        sObjPrio[i] = 4;
        sObjSemi[i] = 0;
        sScrLine[i] = TRANSPARENT;
        sScrPrio[i] = 4;
        sObjWin[i] = 0;
    }
    if (!(dispcnt & 0x1000))
        return;

    for (i = 0; i < 128; i++) {
        uint16_t a0 = OAM16[i * 4 + 0];
        uint16_t a1 = OAM16[i * 4 + 1];
        uint16_t a2 = OAM16[i * 4 + 2];
        int affine = (a0 >> 8) & 1;
        int dbl = (a0 >> 9) & 1;
        int mode = (a0 >> 10) & 3;
        int shape = (a0 >> 14) & 3;
        int size = (a1 >> 14) & 3;
        int ow, oh, bw, bh, ox, oy, line, sx, x;
        int bpp8 = (a0 >> 13) & 1;
        int prio = (a2 >> 10) & 3;
        int tileBase = a2 & 0x3FF;
        int palBank = a2 >> 12;
        int16_t pa = 0x100, pb = 0, pc = 0, pd = 0x100;
        int objMosaic = (a0 >> 12) & 1;

        if (!affine && dbl)
            continue; /* disabled */
        if (shape == 3 || mode == 3)
            continue;
        if (sHudLayer && sOamTag[i].hud)
            continue; /* drawn by PortPpuRenderHud */
        if (HdTextGlyph(i))
            continue; /* drawn by port_hdtext.c */
        ow = sObjSize[shape][size][0];
        oh = sObjSize[shape][size][1];
        bw = ow << (affine && dbl);
        bh = oh << (affine && dbl);

        /* Decode the 8-bit Y and 9-bit X against the visible span. */
        oy = a0 & 0xFF;
        ox = a1 & 0x1FF;
        if ((sOamExt[i].x & 0x1FF) == ox && (sOamExt[i].y & 0xFF) == oy) {
            ox = sOamExt[i].x;
            oy = sOamExt[i].y;
        } else {
            if (oy >= sBottom)
                oy -= 256;
            if (ox >= sRight)
                ox -= 512;
        }
        if (ox >= sRight || ox + bw <= sLeft)
            continue;

        line = gy - oy;
        if (objMosaic && mosV > 1)
            line -= line % mosV;
        if (line < 0 || line >= bh)
            continue;
        if (bitmapMode && tileBase < 512)
            continue;

        if (affine) {
            const uint16_t* p = OAM16 + ((a1 >> 9) & 31) * 16;
            pa = (int16_t)p[3];
            pb = (int16_t)p[7];
            pc = (int16_t)p[11];
            pd = (int16_t)p[15];
        }

        for (sx = 0; sx < bw; sx++) {
            int gx = ox + sx;
            int fx = gx;
            int tx, ty, tile, ci, col;
            uint32_t off;

            if (gx < sLeft || gx >= sRight)
                continue;
            x = gx + gPortScreen.originX;
            if (x < 0 || x >= w)
                continue;
            if (objMosaic && mosH > 1)
                fx = gx - (((gx % mosH) + mosH) % mosH);
            fx -= ox;

            if (affine) {
                int cx = fx - bw / 2, cy = line - bh / 2;
                tx = ((pa * cx + pb * cy) >> 8) + ow / 2;
                ty = ((pc * cx + pd * cy) >> 8) + oh / 2;
                if (tx < 0 || ty < 0 || tx >= ow || ty >= oh)
                    continue;
            } else {
                tx = (a1 & 0x1000) ? ow - 1 - fx : fx;
                ty = (a1 & 0x2000) ? oh - 1 - line : line;
            }

            if (bpp8) {
                if (mapping1d)
                    tile = tileBase + ((ty >> 3) * (ow >> 3) + (tx >> 3)) * 2;
                else
                    tile = tileBase + (ty >> 3) * 32 + (tx >> 3) * 2;
                off = 0x10000 + (tile & 0x3FF) * 32 + (ty & 7) * 8 + (tx & 7);
                ci = VRAM8[off];
                col = ci ? (PLTT16[256 + ci] & 0x7FFF) : -1;
            } else {
                if (mapping1d)
                    tile = tileBase + (ty >> 3) * (ow >> 3) + (tx >> 3);
                else
                    tile = tileBase + (ty >> 3) * 32 + (tx >> 3);
                off = 0x10000 + (tile & 0x3FF) * 32 + (ty & 7) * 4 + ((tx & 7) >> 1);
                ci = (tx & 1) ? (VRAM8[off] >> 4) : (VRAM8[off] & 15);
                col = ci ? (PLTT16[256 + palBank * 16 + ci] & 0x7FFF) : -1;
            }
            if (col < 0)
                continue;
            if (mode == 2) {
                sObjWin[x] = 1;
                continue;
            }
            if (sOverlay && sOamTag[i].screen) {
                if (sScrLine[x] == TRANSPARENT || prio < sScrPrio[x]) {
                    sScrLine[x] = (uint16_t)col;
                    sScrPrio[x] = (uint8_t)prio;
                    sScrSemi[x] = (mode == 1);
                }
            } else if (sObjLine[x] == TRANSPARENT || prio < sObjPrio[x]) {
                sObjLine[x] = (uint16_t)col;
                sObjPrio[x] = (uint8_t)prio;
                sObjIndex[x] = (uint8_t)i;
                sObjSemi[x] = (mode == 1);
            }
        }
    }
}

/* ---- windows --------------------------------------------------------------- */

static void WinSpan(uint16_t reg, int limit, int lo, int hi, int* a, int* b, int* wrapped) {
    int s = reg >> 8, e = reg & 0xFF;

    *wrapped = 0;
    if (e > limit || s > e) {
        if (s > e && e <= limit) {
            *wrapped = 1; /* covers [s, limit) and [0, e) */
        } else {
            e = limit;
        }
    }
    /* A window spanning the whole original screen spans the wide view too. */
    if (!*wrapped && s == 0 && e >= limit) {
        s = lo;
        e = hi;
    }
    *a = s;
    *b = e;
}

static int InWin(int v, int a, int b, int wrapped, int limit, int lo, int hi) {
    if (wrapped)
        return (v >= a && v < hi) || (v < b && v >= lo) || (a >= limit && v >= limit);
    return v >= a && v < b;
}

static void BuildWindows(int gy, uint16_t dispcnt) {
    int w = gPortScreen.width;
    int x;
    uint16_t winin = IO16(0x48), winout = IO16(0x4A);
    int win0 = (dispcnt >> 13) & 1, win1 = (dispcnt >> 14) & 1, winObj = (dispcnt >> 15) & 1;
    int in0 = 0, in1 = 0;
    int a0 = 0, b0 = 0, w0 = 0, a1 = 0, b1 = 0, w1 = 0;

    if (!win0 && !win1 && !winObj) {
        memset(sWinMask, 0x3F, w);
        return;
    }
    if (win0) {
        int ya, yb, yw;
        WinSpan(IO16(0x44), GBA_SCREEN_H, sTop, sBottom, &ya, &yb, &yw);
        in0 = InWin(gy, ya, yb, yw, GBA_SCREEN_H, sTop, sBottom);
        WinSpan(IO16(0x40), GBA_SCREEN_W, sLeft, sRight, &a0, &b0, &w0);
    }
    if (win1) {
        int ya, yb, yw;
        WinSpan(IO16(0x46), GBA_SCREEN_H, sTop, sBottom, &ya, &yb, &yw);
        in1 = InWin(gy, ya, yb, yw, GBA_SCREEN_H, sTop, sBottom);
        WinSpan(IO16(0x42), GBA_SCREEN_W, sLeft, sRight, &a1, &b1, &w1);
    }
    for (x = 0; x < w; x++) {
        int gx = x - gPortScreen.originX;
        uint8_t m = winout & 0x3F;

        if (winObj && sObjWin[x])
            m = (winout >> 8) & 0x3F;
        if (in1 && InWin(gx, a1, b1, w1, GBA_SCREEN_W, sLeft, sRight))
            m = (winin >> 8) & 0x3F;
        if (in0 && InWin(gx, a0, b0, w0, GBA_SCREEN_W, sLeft, sRight))
            m = winin & 0x3F;
        sWinMask[x] = m;
    }
}

/* ---- composition ----------------------------------------------------------- */

static uint16_t Blend(uint16_t a, uint16_t b, int ea, int eb) {
    int r = ((a & 31) * ea + (b & 31) * eb) >> 4;
    int g = (((a >> 5) & 31) * ea + ((b >> 5) & 31) * eb) >> 4;
    int bl = (((a >> 10) & 31) * ea + ((b >> 10) & 31) * eb) >> 4;

    if (r > 31)
        r = 31;
    if (g > 31)
        g = 31;
    if (bl > 31)
        bl = 31;
    return (uint16_t)(r | (g << 5) | (bl << 10));
}

static uint16_t Brighten(uint16_t c, int ey) {
    int r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;

    r += ((31 - r) * ey) >> 4;
    g += ((31 - g) * ey) >> 4;
    b += ((31 - b) * ey) >> 4;
    return (uint16_t)(r | (g << 5) | (b << 10));
}

static uint16_t Darken(uint16_t c, int ey) {
    int r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;

    r -= (r * ey) >> 4;
    g -= (g * ey) >> 4;
    b -= (b * ey) >> 4;
    return (uint16_t)(r | (g << 5) | (b << 10));
}

static void ComposeLine(int y, uint16_t dispcnt, const int* bgOn, const int* bgPrio) {
    int w = gPortScreen.width;
    uint32_t* dst = gPortFrame + y * w;
    uint16_t bldcnt = IO16(0x50);
    uint16_t bldalpha = IO16(0x52);
    int eva = bldalpha & 31, evb = (bldalpha >> 8) & 31;
    int evy = IO16(0x54) & 31;
    int effect = (bldcnt >> 6) & 3;
    uint16_t backdrop = PLTT16[0] & 0x7FFF;
    int order[4], n = 0, p, b, x;

    if (eva > 16)
        eva = 16;
    if (evb > 16)
        evb = 16;
    if (evy > 16)
        evy = 16;

    /* Backgrounds sorted by priority, lower index first on ties. */
    for (p = 0; p < 4; p++)
        for (b = 0; b < 4; b++)
            if (bgOn[b] && !sScreenBg[b] && bgPrio[b] == p)
                order[n++] = b;

    for (x = 0; x < w; x++) {
        int gx = x - gPortScreen.originX;
        uint8_t mask = sWinMask[x];
        uint16_t bd = sInWorld[x] ? backdrop : 0;
        uint16_t top = bd, second = bd;
        int topLayer = LAYER_BD, secondLayer = LAYER_BD;
        int found = 0, i, objDone = 0;
        uint16_t col;

        if (gx < sLeft || gx >= sRight) {
            dst[x] = 0xFF000000u;
            continue;
        }
        if (sHudLayer) {
            int k, occ = 4, message = 0;
            for (k = 0; k < n; k++) {
                if ((mask & WIN_BG(order[k])) && sBgLine[order[k]][x] != TRANSPARENT) {
                    occ = bgPrio[order[k]];
                    break;
                }
            }
            /* message windows (docked layers) always stay in front of the HUD */
            for (k = 0; k < 4; k++)
                if (bgOn[k] && sBgDockedLatched[k] && (mask & WIN_BG(k)) && sBgLine[k][x] != TRANSPARENT)
                    message = 1;
            if (!(mask & WIN_OBJ) || message) {
                sOccBg[y * w + x] = 0;
                sOccObj[y * w + x] = 0;
            } else {
                sOccBg[y * w + x] = (uint8_t)occ;
                sOccObj[y * w + x] = sObjLine[x] != TRANSPARENT ? (uint16_t)(sObjPrio[x] * 128 + sObjIndex[x]) : 0xFFFFu;
            }
        }
        for (i = 0; i <= n && found < 2; i++) {
            int prio = (i < n) ? bgPrio[order[i]] : 4;

            if (!objDone && (mask & WIN_OBJ) && sObjLine[x] != TRANSPARENT && sObjPrio[x] <= prio) {
                objDone = 1;
                if (found == 0) {
                    top = sObjLine[x];
                    topLayer = LAYER_OBJ;
                } else {
                    second = sObjLine[x];
                    secondLayer = LAYER_OBJ;
                }
                found++;
                if (found == 2)
                    break;
            }
            if (i < n) {
                int bg = order[i];
                if ((mask & WIN_BG(bg)) && sBgLine[bg][x] != TRANSPARENT) {
                    if (found == 0) {
                        top = sBgLine[bg][x];
                        topLayer = bg;
                    } else {
                        second = sBgLine[bg][x];
                        secondLayer = bg;
                    }
                    found++;
                }
            }
        }

        col = top;
        if (topLayer == LAYER_OBJ && sObjSemi[x] && (bldcnt & (0x100 << secondLayer))) {
            col = Blend(top, second, eva, evb);
        } else if ((mask & WIN_SFX) && (bldcnt & (1 << topLayer))) {
            switch (effect) {
                case 1:
                    if (bldcnt & (0x100 << secondLayer))
                        col = Blend(top, second, eva, evb);
                    break;
                case 2:
                    col = Brighten(top, evy);
                    break;
                case 3:
                    col = Darken(top, evy);
                    break;
            }
        }
        dst[x] = sColorLut[col];
    }
}

/* ---- frame ------------------------------------------------------------------ */

static void LatchAffine(int force) {
    int a;

    for (a = 0; a < 2; a++) {
        uint32_t rx = IO32(0x28 + a * 0x10), ry = IO32(0x2C + a * 0x10);
        if (force || rx != sAffLatchX[a]) {
            sAffLatchX[a] = rx;
            sAffX[a] = Sext28(rx);
        }
        if (force || ry != sAffLatchY[a]) {
            sAffLatchY[a] = ry;
            sAffY[a] = Sext28(ry);
        }
    }
}

static void StepAffine(int lines) {
    int a;

    for (a = 0; a < 2; a++) {
        sAffX[a] += (int16_t)IO16(0x22 + a * 0x10) * lines;
        sAffY[a] += (int16_t)IO16(0x26 + a * 0x10) * lines;
    }
}

static void SplitBgLine(int b) {
    static uint16_t tmp[PORT_MAX_W];
    int w = gPortScreen.width, m = PortScreenMarginX(), split = sBgSplit[b], x;

    memcpy(tmp, sBgLine[b], (size_t)w * 2);
    for (x = 0; x < w; x++) {
        int gx = x - gPortScreen.originX, c;
        if (gx < split - m)
            c = gx + m;
        else if (gx >= split + m)
            c = gx - m;
        else {
            sBgLine[b][x] = TRANSPARENT;
            continue;
        }
        c += gPortScreen.originX;
        sBgLine[b][x] = (c >= 0 && c < w) ? tmp[c] : TRANSPARENT;
    }
}

/* The screen layer's line gy: menu layers only, transparent elsewhere. */
static void ComposeOverlayLine(int gy, uint16_t dispcnt, const int* bgOn, const int* bgPrio) {
    int wo = gPortOverlayW, w = gPortScreen.width;
    int originO = (wo - GBA_SCREEN_W) / 2;
    uint32_t* dst = gPortOverlay + gy * wo;
    uint16_t bldcnt = IO16(0x50), bldalpha = IO16(0x52);
    int eva = ((bldalpha & 31) > 16 ? 16 : (bldalpha & 31)), evb = (((bldalpha >> 8) & 31) > 16 ? 16 : ((bldalpha >> 8) & 31));
    int evy = ((IO16(0x54) & 31) > 16 ? 16 : (IO16(0x54) & 31)), effect = (bldcnt >> 6) & 3;
    int order[4], n = 0, p, b, ox;

    for (p = 0; p < 4; p++)
        for (b = 0; b < 4; b++)
            if (bgOn[b] && sScreenBg[b] && bgPrio[b] == p)
                order[n++] = b;
    for (ox = 0; ox < wo; ox++) {
        int gx = ox - originO, x = gx + gPortScreen.originX, i, found = 0, objDone = 0;
        int topLayer = LAYER_BD, secondLayer = LAYER_BD;
        uint16_t top = 0, second = 0, col;
        uint8_t mask;

        dst[ox] = 0;
        if (x < 0 || x >= w || gx < sLeft || gx >= sRight)
            continue;
        mask = sWinMask[x];
        for (i = 0; i <= n && found < 2; i++) {
            int prio = (i < n) ? bgPrio[order[i]] : 4;
            if (!objDone && (mask & WIN_OBJ) && sScrLine[x] != TRANSPARENT && sScrPrio[x] <= prio) {
                objDone = 1;
                if (found++ == 0) {
                    top = sScrLine[x];
                    topLayer = LAYER_OBJ;
                } else {
                    second = sScrLine[x];
                    secondLayer = LAYER_OBJ;
                }
                if (found == 2)
                    break;
            }
            if (i < n && (mask & WIN_BG(order[i])) && sBgLine[order[i]][x] != TRANSPARENT) {
                if (found++ == 0) {
                    top = sBgLine[order[i]][x];
                    topLayer = order[i];
                } else {
                    second = sBgLine[order[i]][x];
                    secondLayer = order[i];
                }
            }
        }
        if (found == 0)
            continue;
        col = top;
        {
            uint32_t alpha = 0xFF000000u;
            int blend = (topLayer == LAYER_OBJ && sScrSemi[x]) || ((mask & WIN_SFX) && effect == 1 && (bldcnt & (1 << topLayer)));
            if (blend) {
                if (found == 2 && (bldcnt & (0x100 << secondLayer)))
                    col = Blend(top, second, eva, evb);
                else
                    alpha = (uint32_t)(eva * 16 > 255 ? 255 : eva * 16) << 24; /* over the world below */
            } else if ((mask & WIN_SFX) && (bldcnt & (1 << topLayer))) {
                if (effect == 2)
                    col = Brighten(top, evy);
                else if (effect == 3)
                    col = Darken(top, evy);
            }
            dst[ox] = (sColorLut[col] & 0x00FFFFFFu) | alpha;
        }
    }
}

static void RenderLine(int y, int gy) {
    uint16_t dispcnt = IO16(0x00);
    int mode = dispcnt & 7;
    int bgOn[4] = { 0, 0, 0, 0 }, bgPrio[4];
    int b, w = gPortScreen.width;

    if (dispcnt & 0x80) {
        uint32_t* dst = gPortFrame + y * w;
        int x;
        for (x = 0; x < w; x++) {
            int gx = x - gPortScreen.originX;
            dst[x] = (gx < sLeft || gx >= sRight) ? 0xFF000000u : 0xFFFFFFFFu;
        }
        return;
    }

    {
        int x, inside = !gPortScreen.wide || (gy >= 0 && gy < GBA_SCREEN_H);
        for (x = 0; x < w; x++) {
            int gx = x - gPortScreen.originX;
            sInWorld[x] = inside && (!gPortScreen.wide || (gx >= 0 && gx < GBA_SCREEN_W));
        }
    }

    for (b = 0; b < 4; b++) {
        bgPrio[b] = IO16(0x08 + b * 2) & 3;
        if (!(dispcnt & (0x100 << b)))
            continue;
        switch (mode) {
            case 0:
                bgOn[b] = 1;
                break;
            case 1:
                bgOn[b] = b <= 2;
                break;
            case 2:
                bgOn[b] = b >= 2;
                break;
            default:
                bgOn[b] = b == 2;
                break;
        }
    }

    for (b = 0; b < 4; b++) {
        if (!bgOn[b])
            continue;
        if (mode >= 3)
            DrawBitmapBg(mode, dispcnt);
        else if (b >= 2 && (mode == 2 || (mode == 1 && b == 2)))
            DrawAffineBg(b, dispcnt);
        else
            DrawTextBg(b, gy, dispcnt);
        if (sOverlay && sScreenBg[b] && sBgSplit[b])
            SplitBgLine(b);
    }
    DrawObjs(gy, dispcnt);
    BuildWindows(gy, dispcnt);
    ComposeLine(y, dispcnt, bgOn, bgPrio);
    if (sOverlay && gy >= 0 && gy < GBA_SCREEN_H)
        ComposeOverlayLine(gy, dispcnt, bgOn, bgPrio);
}

/* Start of a line: VCOUNT and the V-counter match flag. */
static void LineStart(int line) {
    uint16_t dispstat = IO16(0x04);

    IO16(0x06) = (uint16_t)line;
    IO16(0x04) = (dispstat & ~6) | (((dispstat >> 8) == line) ? 4 : 0);
}

/* End of a line's drawing period. The V-counter interrupt is delivered here
   rather than at the start of the line: handlers typically wait for HBlank
   anyway, and either way their writes take effect from the next line. */
static void HBlank(int line) {
    uint16_t dispstat;

    IO16(0x04) |= 2;
    dispstat = IO16(0x04);
    if ((dispstat & 4) && (dispstat & 0x20))
        PortRaiseIrq(0x0004);
    if (line < GBA_SCREEN_H)
        PortDmaHBlank();
    if (dispstat & 0x10)
        PortRaiseIrq(0x0002);
    IO16(0x04) &= ~2;
}

void PortPpuBeginFrame(void) {
    uint16_t dispcnt = IO16(0x00);
    int b, any = 0;

    BuildLut();
    for (b = 0; b < 4; b++) {
        sBgLatch[b].valid = ValidateBgLatch(b, dispcnt);
        sBgWorldLatched[b].valid = !sBgLatch[b].valid && ValidateBgWorld(b, dispcnt);
        any |= sBgLatch[b].valid | sBgWorldLatched[b].valid;
    }
    /* Large affine stages (battle arenas) are fully in VRAM and extend as is. */
    for (b = 2; b < 4; b++) {
        int mode = dispcnt & 7;
        if ((dispcnt & (0x100 << b)) && (mode == 2 || (mode == 1 && b == 2)) && (IO16(0x08 + b * 2) >> 14) >= 2)
            any = 1;
    }
    gPortScreen.wide = any && (gPortScreen.width > GBA_SCREEN_W || gPortScreen.height > GBA_SCREEN_H)
                       && !(dispcnt & 0x80);
    sHudLayer = gPortScreen.wide && gPortHudScale > 0 && gPortHudW >= GBA_SCREEN_W;
    if (sHudLayer) {
        memset(sOccBg, 4, (size_t)gPortScreen.width * gPortScreen.height);
        memset(sOccObj, 0xFF, (size_t)gPortScreen.width * gPortScreen.height * 2);
    }
    sHdText = gPortConfig.hdText;
    sOverlay = gPortScreenLayer;
    for (b = 0; b < 4; b++) {
        int m = dispcnt & 7;
        int arena = (m == 2 || (m == 1 && b == 2)) && b >= 2 && (IO16(0x08 + b * 2) >> 14) >= 2;
        sScreenBg[b] = sOverlay && !sBgLatch[b].valid && !sBgWorldLatched[b].valid && !arena;
    }
    if (sOverlay)
        memset(gPortOverlay, 0, (size_t)gPortOverlayW * GBA_SCREEN_H * 4);
    if (gPortScreen.wide) {
        sLeft = -gPortScreen.originX;
        sRight = gPortScreen.width - gPortScreen.originX;
        sTop = -gPortScreen.originY;
        sBottom = gPortScreen.height - gPortScreen.originY;
    } else {
        sLeft = 0;
        sRight = GBA_SCREEN_W;
        sTop = 0;
        sBottom = GBA_SCREEN_H;
    }
}

/* Draw the visible lines, running the per-line hardware events in between. */
void PortPpuRenderFrame(void) {
    int y, gy;
    int w = gPortScreen.width;

    PortPpuBeginFrame();
    LatchAffine(1);

    /* Lines above the original picture: state of line 0. */
    for (y = 0; y < gPortScreen.originY; y++) {
        gy = y - gPortScreen.originY;
        if (gy < sTop) {
            memset(gPortFrame + y * w, 0, w * 4);
            continue;
        }
        {
            int32_t sx[2] = { sAffX[0], sAffX[1] }, sy[2] = { sAffY[0], sAffY[1] };
            int a;
            for (a = 0; a < 2; a++) {
                sAffX[a] += (int16_t)IO16(0x22 + a * 0x10) * gy;
                sAffY[a] += (int16_t)IO16(0x26 + a * 0x10) * gy;
            }
            RenderLine(y, gy);
            for (a = 0; a < 2; a++) {
                sAffX[a] = sx[a];
                sAffY[a] = sy[a];
            }
        }
    }

    for (gy = 0; gy < GBA_SCREEN_H; gy++) {
        LineStart(gy);
        LatchAffine(0);
        RenderLine(gy + gPortScreen.originY, gy);
        StepAffine(1);
        HBlank(gy);
    }

    /* Lines below the original picture: state after line 159. */
    for (y = gPortScreen.originY + GBA_SCREEN_H; y < gPortScreen.height; y++) {
        gy = y - gPortScreen.originY;
        if (gy >= sBottom) {
            memset(gPortFrame + y * w, 0, w * 4);
            continue;
        }
        RenderLine(y, gy);
        StepAffine(1);
    }
}

/* Remaining lines of the frame (VBlank period) after the VBlank interrupt. */
void PortPpuVBlankLines(void) {
    int line;

    for (line = GBA_SCREEN_H + 1; line < 228; line++) {
        LineStart(line);
        HBlank(line);
    }
    IO16(0x06) = 0;
}

/* ---- HUD layer ----------------------------------------------------------------
 * HUD sprites (drawn between PortHudBegin/End) are rendered into their own
 * ARGB buffer, gPortHudW x gPortHudH, which the presenter scales by
 * gPortHudScale: larger than the world while staying pixel-exact. Each
 * sprite keeps its distance to the screen edge it is anchored to.
 */
static int ObjColor(uint16_t a0, uint16_t a2, int tx, int ty, int ow, int mapping1d) {
    int bpp8 = (a0 >> 13) & 1;
    int tileBase = a2 & 0x3FF;
    int tile, ci;
    uint32_t off;

    if (bpp8) {
        tile = mapping1d ? tileBase + ((ty >> 3) * (ow >> 3) + (tx >> 3)) * 2 : tileBase + (ty >> 3) * 32 + (tx >> 3) * 2;
        off = 0x10000 + (tile & 0x3FF) * 32 + (ty & 7) * 8 + (tx & 7);
        ci = VRAM8[off];
        return ci ? (PLTT16[256 + ci] & 0x7FFF) : -1;
    }
    tile = mapping1d ? tileBase + (ty >> 3) * (ow >> 3) + (tx >> 3) : tileBase + (ty >> 3) * 32 + (tx >> 3);
    off = 0x10000 + (tile & 0x3FF) * 32 + (ty & 7) * 4 + ((tx & 7) >> 1);
    ci = (tx & 1) ? (VRAM8[off] >> 4) : (VRAM8[off] & 15);
    return ci ? (PLTT16[256 + (a2 >> 12) * 16 + ci] & 0x7FFF) : -1;
}

/* The backdrop colour (palette 0), as displayed. */
uint32_t PortPpuBackdrop(void) {
    return sColorLut[PLTT16[0] & 0x7FFF];
}

void PortPpuRenderHud(uint32_t* buf) {
    uint16_t dispcnt = IO16(0x00);
    uint16_t bldcnt = IO16(0x50);
    int effect = (bldcnt >> 6) & 3, evy = IO16(0x54) & 31;
    int mapping1d = (dispcnt >> 6) & 1;
    int w = gPortHudW, h = gPortHudH;
    int i;

    memset(buf, 0, (size_t)w * h * 4);
    if (!sHudLayer || (dispcnt & 0x80) || !(dispcnt & 0x1000))
        return;
    if (evy > 16)
        evy = 16;

    for (i = 127; i >= 0; i--) {
        uint16_t a0 = OAM16[i * 4 + 0], a1 = OAM16[i * 4 + 1], a2 = OAM16[i * 4 + 2];
        int affine = (a0 >> 8) & 1, dbl = (a0 >> 9) & 1, mode = (a0 >> 10) & 3;
        int shape = (a0 >> 14) & 3, size = (a1 >> 14) & 3;
        int ow, oh, bw, bh, ox, oy, xg, yg, hx, hy, sx, sy;
        int16_t pa = 0x100, pb = 0, pc = 0, pd = 0x100;
        const SpriteTag* tag = &sOamTag[i];

        if (!tag->hud || (!affine && dbl) || shape == 3 || mode >= 2 || HdTextGlyph(i))
            continue;
        ow = sObjSize[shape][size][0];
        oh = sObjSize[shape][size][1];
        bw = ow << (affine && dbl);
        bh = oh << (affine && dbl);
        oy = a0 & 0xFF;
        ox = a1 & 0x1FF;
        if ((sOamExt[i].x & 0x1FF) == ox && (sOamExt[i].y & 0xFF) == oy) {
            ox = sOamExt[i].x;
            oy = sOamExt[i].y;
        }
        /* back to GBA-screen coordinates, then onto the HUD canvas */
        xg = ox - tag->ax * gPortScreen.originX;
        yg = oy - tag->ay * gPortScreen.originY;
        hx = tag->ax < 0 ? xg : (tag->ax > 0 ? w - (GBA_SCREEN_W - xg) : xg + (w - GBA_SCREEN_W) / 2);
        hy = tag->ay < 0 ? yg : (tag->ay > 0 ? h - (GBA_SCREEN_H - yg) : yg + (h - GBA_SCREEN_H) / 2);
        if (tag->ay > 0)
            hy -= HudRaise();
        if (affine) {
            const uint16_t* p = OAM16 + ((a1 >> 9) & 31) * 16;
            pa = (int16_t)p[3];
            pb = (int16_t)p[7];
            pc = (int16_t)p[11];
            pd = (int16_t)p[15];
        }
        for (sy = 0; sy < bh; sy++) {
            int py = hy + sy;
            if (py < 0 || py >= h)
                continue;
            for (sx = 0; sx < bw; sx++) {
                int px = hx + sx, tx, ty, col;
                uint32_t argb;
                if (px < 0 || px >= w)
                    continue;
                if (affine) {
                    int cx = sx - bw / 2, cy = sy - bh / 2;
                    tx = ((pa * cx + pb * cy) >> 8) + ow / 2;
                    ty = ((pc * cx + pd * cy) >> 8) + oh / 2;
                    if (tx < 0 || ty < 0 || tx >= ow || ty >= oh)
                        continue;
                } else {
                    tx = (a1 & 0x1000) ? ow - 1 - sx : sx;
                    ty = (a1 & 0x2000) ? oh - 1 - sy : sy;
                }
                col = ObjColor(a0, a2, tx, ty, ow, mapping1d);
                if (col < 0)
                    continue;
                {
                    /* hidden where the cartridge draws something in front */
                    int cx = ox + sx + gPortScreen.originX, cy = oy + sy + gPortScreen.originY;
                    int prio = (a2 >> 10) & 3;
                    if (cx >= 0 && cy >= 0 && cx < gPortScreen.width && cy < gPortScreen.height) {
                        int k = cy * gPortScreen.width + cx;
                        if (sOccBg[k] < prio || sOccObj[k] < prio * 128 + i)
                            continue;
                    }
                }
                if (bldcnt & 0x10) {
                    if (effect == 2)
                        col = Brighten((uint16_t)col, evy);
                    else if (effect == 3)
                        col = Darken((uint16_t)col, evy);
                }
                argb = sColorLut[col];
                if (mode == 1)
                    argb = (argb & 0x00FFFFFFu) | 0xA0000000u;
                buf[py * w + px] = argb;
            }
        }
    }
}

/* HD text glyphs of one layer (world canvas or HUD canvas), front last. */
int PortPpuTextGlyphs(PortTextGlyph* out, int max, int hud) {
    uint16_t dispcnt = IO16(0x00);
    uint16_t bldcnt = IO16(0x50);
    int effect = (bldcnt >> 6) & 3, evy = IO16(0x54) & 31;
    int n = 0, i;

    if (!sHdText || (dispcnt & 0x80) || !(dispcnt & 0x1000))
        return 0;
    if (evy > 16)
        evy = 16;
    for (i = 127; i >= 0 && n < max; i--) {
        uint16_t a0 = OAM16[i * 4 + 0], a1 = OAM16[i * 4 + 1], a2 = OAM16[i * 4 + 2];
        const SpriteTag* tag = &sOamTag[i];
        int onHud = sHudLayer && tag->hud;
        int ox, oy, bank, k;
        uint16_t col[2];

        if (!HdTextGlyph(i) || onHud != hud || (a0 & 0x300) == 0x200 || ((a0 >> 10) & 3) >= 2)
            continue;
        oy = a0 & 0xFF;
        ox = a1 & 0x1FF;
        if ((sOamExt[i].x & 0x1FF) == ox && (sOamExt[i].y & 0xFF) == oy) {
            ox = sOamExt[i].x;
            oy = sOamExt[i].y;
        } else {
            if (oy >= sBottom)
                oy -= 256;
            if (ox >= sRight)
                ox -= 512;
        }
        if (onHud) {
            int w = gPortHudW, h = gPortHudH;
            int xg = ox - tag->ax * gPortScreen.originX, yg = oy - tag->ay * gPortScreen.originY;
            out[n].x = tag->ax < 0 ? xg : (tag->ax > 0 ? w - (GBA_SCREEN_W - xg) : xg + (w - GBA_SCREEN_W) / 2);
            out[n].y = tag->ay < 0 ? yg : (tag->ay > 0 ? h - (GBA_SCREEN_H - yg) - HudRaise() : yg + (h - GBA_SCREEN_H) / 2);
        } else {
            if (ox + 16 <= sLeft || ox >= sRight || oy + 16 <= sTop || oy >= sBottom)
                continue;
            out[n].x = ox + gPortScreen.originX;
            out[n].y = oy + gPortScreen.originY;
        }
        /* colour 1 is the face, colour 2 the shadow, as the game fades them */
        bank = a2 >> 12;
        for (k = 0; k < 2; k++) {
            col[k] = PLTT16[256 + bank * 16 + 1 + k] & 0x7FFF;
            if (bldcnt & 0x10) {
                if (effect == 2)
                    col[k] = Brighten(col[k], evy);
                else if (effect == 3)
                    col[k] = Darken(col[k], evy);
            }
        }
        out[n].code = tag->text;
        out[n].semi = ((a0 >> 10) & 3) == 1;
        out[n].face = sColorLut[col[0]];
        out[n].shadow = sColorLut[col[1]];
        n++;
    }
    return n;
}
