#ifndef PORT_SCENE061_TILES_H
#define PORT_SCENE061_TILES_H

/* Scene 061: accepted by the user, with visual polish pending (2026-10-18).
 * Coordinates and steps are in original 8x8 tiles. Call only for event 061,
 * outside the original map, on BG3 (floor) or BG2 (wall). Do not pass the result
 * through the generic decoration skipping algorithm: this distribution was
 * approved with direct source-tile lookup. See ESCENA061_RECUPERADA.md.
 */
static inline int PortScene061TileSource(int bg, int width, int height, int* x, int* y) {
    static const int floorSteps[4][2] = {{4, 2}, {-4, 2}, {-4, 2}, {-4, -2}};
    static const int wallSteps[4][2] = {{12, 6}, {-12, -6}, {12, 6}, {-4, -2}};
    const int (*steps)[2];
    int i, edge, dx, dy;
    if ((bg != 2 && bg != 3) || width <= 0 || height <= 0)
        return 0;
    steps = bg == 3 ? floorSteps : wallSteps;
    for (i = 0; i < 64 && (*x < 0 || *x >= width); ++i) {
        edge = *x < 0 ? 0 : 1;
        *x += steps[edge][0];
        *y += steps[edge][1];
    }
    for (i = 0; i < 64 && (*y < 0 || *y >= height); ++i) {
        edge = *y < 0 ? 2 : 3;
        dx = steps[edge][0];
        dy = steps[edge][1];
        if (*x + dx < 0 || *x + dx >= width)
            dx = -dx;
        *x += dx;
        *y += dy;
    }
    return *x >= 0 && *x < width && *y >= 0 && *y < height;
}

#endif
