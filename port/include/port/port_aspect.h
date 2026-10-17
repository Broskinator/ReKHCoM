#ifndef PORT_ASPECT_H
#define PORT_ASPECT_H

/* Persisted IDs 0..4 retain their original meanings. */
enum { PORT_ASPECT_FILL, PORT_ASPECT_16_9, PORT_ASPECT_21_9,
       PORT_ASPECT_FORCE_21_9, PORT_ASPECT_FORCE_32_9,
       PORT_ASPECT_4_3, PORT_ASPECT_GBA, PORT_ASPECT_COUNT };
typedef struct PortAspectBounds { int x, y, w, h; } PortAspectBounds;
static inline PortAspectBounds PortAspectFit(int width, int height, int mode) {
    PortAspectBounds r = {0, 0, width, height};
    double ratio = 0;
    int exact = 0;
    switch (mode) {
        case PORT_ASPECT_16_9: ratio = 16.0/9.0; break;
        case PORT_ASPECT_21_9: ratio = 21.0/9.0; break;
        case PORT_ASPECT_FORCE_21_9: ratio = 21.0/9.0; exact = 1; break;
        case PORT_ASPECT_FORCE_32_9: ratio = 32.0/9.0; exact = 1; break;
        case PORT_ASPECT_4_3: ratio = 4.0/3.0; exact = 1; break;
        case PORT_ASPECT_GBA: ratio = 3.0/2.0; exact = 1; break;
    }
    if (width <= 0 || height <= 0 || ratio == 0) return r;
    if (width > height * ratio) {
        r.w = (int)(height * ratio + .5);
        if (r.w < 1) r.w = 1;
        r.x = (width - r.w) / 2;
    } else if (exact) {
        r.h = (int)(width / ratio + .5);
        if (r.h < 1) r.h = 1;
        r.y = (height - r.h) / 2;
    }
    return r;
}
#endif
