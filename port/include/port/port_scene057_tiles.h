#ifndef PORT_SCENE057_TILES_H
#define PORT_SCENE057_TILES_H

#include "port_scene_wide_tiles.h"

/* Event 057: layout and grass motif completion accepted. Use original source maps
 * and live palette. BG2 is intentionally empty to the right in this framing:
 * don't repeat the unique tree/vines from the original map.
 * Coordinates and dimensions are in 8x8 tiles, occupied map is 32x32.
 */
static inline int PortScene057TileSource(int bg, int* x, int* y) {
    if (bg != 2 && bg != 3)
        return 0;
    if (bg == 3 && *x >= 0 && *x <= 4 && *y >= 18 && *y <= 22) {
        /* Complete the half-grass crossing into the original map, under BG2. */
        *x += 8;
        *y -= 4;
        return 1;
    }
    if (bg == 3 && *x >= 32 && *x <= 35 && *y >= 5 && *y <= 11) {
        /* Nine original grass entries match the complete central counterpart. */
        *x -= 15;
        *y += 4;
        return 1;
    }
    if (*x < 0 || *x >= 32 || *y < 0 || *y >= 32)
        return PortSceneWideSource(57, bg, x, y);
    return 1;
}

#endif
