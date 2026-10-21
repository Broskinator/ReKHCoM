#ifndef PORT_AGRABAH109_TILES_H
#define PORT_AGRABAH109_TILES_H
#include "port_scene_wide_tiles.h"

/* The Genie conversation uses a separate 32x32 map. The rightmost eight
 * columns carry the diagonal wall footing without windows or pottery.
 * Continue that band, then use plain wall/sand past the donor's vertical
 * range. Return only original tile coordinates; keep the live palette. */
static inline int PortAgrabah109TileSource(int bg, int* x, int* y) {
    int ox = *x, oy = *y, sx, sy;
    if (ox >= 0 && ox < 32 && oy >= 0 && oy < 32) return 1;
    sx = 24 + PortSceneTileMod(ox, 8);
    sy = oy + (ox - sx) / 2;
    /* The top-right corner truncates part of this donor band. Continue
     * inward along the same diagonal to an intact footing, not plain wall. */
    while (sy < 0 && sx >= 24) { sx -= 8; sy += 4; }
    if (sy >= 0 && sy < 32) {
        *x = sx; *y = sy;
        return 1;
    }
    if (bg == 2) return 0;
    if (sy < 0) {
        *x = 8 + PortSceneTileMod(ox, 4);
        *y = PortSceneTileMod(oy, 4);
    } else {
        *x = 24 + PortSceneTileMod(ox, 8);
        *y = 24 + PortSceneTileMod(oy, 8);
    }
    return 1;
}
#endif
