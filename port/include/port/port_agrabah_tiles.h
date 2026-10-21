#ifndef PORT_AGRABAH_TILES_H
#define PORT_AGRABAH_TILES_H
#include "port_scene_wide_tiles.h"

/* Street shared by events 107/108. Coordinates are original 8x8 tiles.
 * Side donors contain no windows, pots, market stalls or building corners.
 * Keep the two independent diagonals: street NE/SW, roof fronts NW/SE. */
static inline int PortAgrabahTileSource(int bg, int* x, int* y) {
    int ox = *x, oy = *y, sx, sy;
    if (ox >= 0 && ox < 64 && oy >= 0 && oy < 64) return 1;
    if (ox < 0) {
        sx = PortSceneTileMod(ox, 4);
        sy = oy + (ox - sx) / 2;
    } else if (ox >= 64) {
        sx = 60 + PortSceneTileMod(ox, 4);
        if (2 * oy + ox >= 118 && 2 * oy - ox < 7) {
            if (bg == 2) return 0;
            *x = sx; *y = 30 + PortSceneTileMod(oy, 2);
            return 1; /* open roof between two independently sloping curbs */
        }
        sy = oy + (2 * oy - ox >= 7 ? sx - ox : ox - sx) / 2;
    } else {
        sx = ox;
        sy = oy;
    }
    if (sy >= 0 && sy < 64) { *x = sx; *y = sy; return 1; }
    if (bg == 2) return 0;
    /* Plain upper facade or foreground roof, from their own texture bands. */
    *x = PortSceneTileMod(ox, 4);
    *y = sy < 0 ? 12 + PortSceneTileMod(oy, 4) : 60 + PortSceneTileMod(oy, 4);
    return 1;
}
#endif
