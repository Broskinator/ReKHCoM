#ifndef PORT_SCENE074_TILES_H
#define PORT_SCENE074_TILES_H

/* Event 074: layout and completed lower ornament accepted by user.
 * Apply to BG3, BG2 AND BG1, with the current hardware palette/effects.
 * The declared 64x64 map has only one occupied 32x32 block: call this lookup
 * also for coordinates inside default blocks, not only outside the declaration.
 * The authored void inside the occupied block is part of the scene and stays.
 * A successful lookup returns ORIGINAL source coordinates, never mutated tiles.
 */
static inline int PortScene074TileSource(int bg, int* x, int* y) {
    int i, dx, dy;
    if ((bg < 1 || bg > 3) || *x < 0 || *y < 0)
        return 0;
    if (bg == 2 && *x >= 32 && *x <= 34 && *y >= 17 && *y <= 21) {
        /* Nineteen existing entries match the original left counterpart.
         * Continue its tail instead of repeating its beginning diagonally. */
        *x -= 20;
        *y += 1;
        return 1;
    }
    for (i = 0; i < 64 && *x >= 32; ++i) {
        *x -= 4;
        *y -= 2;
    }
    for (i = 0; i < 64 && (*y < 0 || *y >= 32); ++i) {
        if (*y < 0) { dx = 2; dy = 1; }
        else if (bg == 3) { dx = 4; dy = -2; }
        else if (bg == 2) { dx = -2; dy = -1; }
        else { dx = -4; dy = -2; }
        if (*x + dx < 0 || *x + dx >= 32)
            dx = -dx;
        *x += dx;
        *y += dy;
    }
    return *x >= 0 && *x < 32 && *y >= 0 && *y < 32;
}

#endif
