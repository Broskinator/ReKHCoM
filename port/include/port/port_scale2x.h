#ifndef PORT_SCALE2X_H
#define PORT_SCALE2X_H
#include <stdint.h>
/* Caller supplies 2*w by 2*h pixels. Clamp neighbours at image boundaries.
 * Exact colour comparisons preserve the original palette and alpha. */
static void PortScale2x(const uint32_t* src, int w, int h, uint32_t* dst) {
    int x, y;
    for (y = 0; y < h; ++y) for (x = 0; x < w; ++x) {
        uint32_t e = src[y*w+x];
        uint32_t b = src[(y ? y-1 : y)*w+x];
        uint32_t d = src[y*w+(x ? x-1 : x)];
        uint32_t f = src[y*w+(x+1 < w ? x+1 : x)];
        uint32_t hh = src[(y+1 < h ? y+1 : y)*w+x];
        uint32_t* o = dst + (y*2)*(w*2)+x*2;
        o[0] = o[1] = o[w*2] = o[w*2+1] = e;
        if (b != hh && d != f) {
            o[0] = d == b ? d : e;
            o[1] = b == f ? f : e;
            o[w*2] = d == hh ? d : e;
            o[w*2+1] = hh == f ? f : e;
        }
    }
}
#endif
