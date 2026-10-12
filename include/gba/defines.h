#ifndef GUARD_GBA_DEFINES_H
#define GUARD_GBA_DEFINES_H

#define INTR_VECTOR (*(void**)0x03007FFC)

#define EWRAM_START 0x02000000
#define EWRAM_SIZE 0x40000

#define IWRAM_START 0x03000000
#define IWRAM_SIZE 0x8000

#define PLTT 0x05000000
#define PLTT_SIZE 0x400
#define BG_PLTT PLTT
#define BG_PLTT_SIZE 0x200
#define OBJ_PLTT (PLTT + 0x200)

#define VRAM 0x06000000
#define VRAM_SIZE 0x18000
#define BG_VRAM VRAM
#define BG_CHAR_SIZE 0x4000
#define BG_CHAR_ADDR(n) (BG_VRAM + BG_CHAR_SIZE * (n))
#define OBJ_VRAM0 (VRAM + 0x10000)

#define OAM 0x07000000

#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 160

#define TILE_SIZE_4BPP 32
#define PLTT_SIZE_4BPP 32

/*
 * Visible area in screen coordinates. On hardware it is the 240x160 LCD; the
 * PC build can show more around it (wide view), so sprite culling uses these.
 */
#ifdef PLATFORM_PC
extern int gPortViewLeft;
extern int gPortViewTop;
extern int gPortViewRight;
extern int gPortViewBottom;
#define VIEW_LEFT gPortViewLeft
#define VIEW_TOP gPortViewTop
#define VIEW_RIGHT gPortViewRight
#define VIEW_BOTTOM gPortViewBottom
#else
#define VIEW_LEFT 0
#define VIEW_TOP 0
#define VIEW_RIGHT DISPLAY_WIDTH
#define VIEW_BOTTOM DISPLAY_HEIGHT
#endif

#endif
