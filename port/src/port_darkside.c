/* Darkside: one authored island and one boss, surrounded by continued sky
 * and clean sand. Do not repeat the palms, rocks, grass or the boss layer. */
#include "port/port.h"
#include "port/port_battle_backdrop.h"

extern const uint16_t gBosDsdBgMap0[];
extern const uint16_t gBosDsdBgMap2[];
extern const uint16_t gDefaultBgMap[];
extern const uint8_t gBosDsdBgTiles[];

static int Repeat(int x, int origin, int size) {
    int n = (x - origin) % size;
    return origin + (n < 0 ? n + size : n);
}

static int SourceIndex(int x, int y) {
    const uint16_t* map = y < 256 ? gBosDsdBgMap0 : gBosDsdBgMap2;
    unsigned e = map[((y & 255) >> 3) * 32 + (x >> 3)];
    int px = x & 7, py = y & 7, ci;
    if (e & 1024) px = 7 - px;
    if (e & 2048) py = 7 - py;
    ci = (gBosDsdBgTiles[(e & 1023) * 32 + py * 4 + px / 2] >> ((px & 1) * 4)) & 15;
    return ci ? (int)(e >> 12) * 16 + ci : 0;
}

int PortDarksideBackdropMatches(const void* const* maps) {
    return maps && maps[0] == gBosDsdBgMap0 && maps[1] == gDefaultBgMap &&
           maps[2] == gBosDsdBgMap2 && maps[3] == gDefaultBgMap;
}

int PortDarksideBackdropPixel(int x, int y, uint16_t* color) {
    int sx, sy = y, index;
    if (x >= 0 && x < 256 && y >= 0 && y < 512)
        return 0;
    if (y < 0) {
        sx = 128; sy = 0;
    } else if (y < 312) {
        /* Bare sky/horizon/shore at the island's right edge. This strip
         * excludes the palm and the two unique rock formations. */
        sx = Repeat(x, 240, 16);
    } else if (y < 400) {
        /* Preserve the sand's distance-dependent shading, excluding grass. */
        sx = Repeat(x, 240, 16);
    } else {
        sx = Repeat(x, 128, 64);
        if (y >= 512) sy = Repeat(y, 448, 64);
    }
    index = SourceIndex(sx, sy);
    *color = index ? ((const uint16_t*)(uintptr_t)GBA_PLTT)[index] & 0x7FFF : 0x8000;
    return 1;
}
