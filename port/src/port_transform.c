/*
 * C version of asm/transform.s: the 8-point integer transform and square-law
 * quantiser used by the block audio codec (src/lib/audio_block_codec.c).
 * Arithmetic shifts and operation order follow the ARM code exactly.
 */
#include <stdint.h>

extern const uint8_t gUnk_09C43708[4096];

static const int sShifts[8] = { 8, 8, 9, 9, 10, 11, 12, 13 };

static int32_t Quantize(int32_t v, int shift) {
    int32_t q;

    if (v <= 0) {
        q = gUnk_09C43708[(-v) >> shift];
        return -q;
    }
    return gUnk_09C43708[v >> shift];
}

static int32_t Dequantize(int32_t q, int shift) {
    int32_t v = (int32_t)((uint32_t)(q * q) << shift);

    return q <= 0 ? -v : v;
}

static void ForwardTransform(const int32_t* in, int32_t* out) {
    int32_t x0 = in[0], x1 = in[1], x2 = in[2], x3 = in[3];
    int32_t x4 = in[4], x5 = in[5], x6 = in[6], x7 = in[7];
    int32_t a, b, c, d, e, f, g, h;

    a = x0 + x7;
    b = x3 + x4;
    c = x1 + x6;
    d = x2 + x5;
    e = a + b;
    f = a - b;
    g = c + d;
    h = c - d;
    e = e + g;
    g = (e >> 1) - g;
    h = h - ((3 * f) >> 3);
    f = f + ((3 * h) >> 3);
    out[0] = e;
    out[4] = g;
    out[2] = f;
    out[6] = h;

    a = x3 - x4;
    b = x2 - x5;
    c = x1 - x6;
    d = x0 - x7;
    c = c + ((3 * b) >> 3);
    b = ((5 * c) >> 3) - b;
    e = a + b;
    f = a - b;
    g = d - c;
    h = d + c;
    e = e - (h >> 3);
    f = f + ((7 * g) >> 3);
    g = g - (f >> 1);
    out[1] = h;
    out[3] = g;
    out[5] = f;
    out[7] = e;
}

static void InverseTransform(const int32_t* in, int32_t* out) {
    int32_t y0 = in[0], y1 = in[1], y2 = in[2], y3 = in[3];
    int32_t y4 = in[4], y5 = in[5], y6 = in[6], y7 = in[7];
    int32_t r3, r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, r2;

    r5 = y3 + (y5 >> 1);
    r7 = y5 - ((7 * r5) >> 3);
    r9 = y7 + (y1 >> 3);
    sl = r9 + r7;
    fp = r9 - r7;
    ip = y1 - r5;
    lr = y1 + r5;
    fp = ((5 * ip) >> 3) - fp;
    ip = ip - ((3 * fp) >> 3);
    r4 = y2 - ((3 * y6) >> 3);
    r8 = y6 + ((3 * r4) >> 3);
    r6 = (y0 >> 1) - y4;
    r2 = y0 - r6;
    r3 = r2 + r4;
    r5 = r6 + r8;
    r7 = r6 - r8;
    r9 = r2 - r4;
    out[0] = r3 + lr;
    out[1] = r5 + ip;
    out[2] = r7 + fp;
    out[3] = r9 + sl;
    out[4] = r9 - sl;
    out[5] = r7 - fp;
    out[6] = r5 - ip;
    out[7] = r3 - lr;
}

static void DivideBy4(int32_t* v) {
    int i;

    for (i = 0; i < 8; i++)
        v[i] = v[i] > 0 ? v[i] >> 2 : -((-v[i]) >> 2);
}

/* Forward: transform, then quantise with one round of error feedback. */
void func_081213CC(int32_t* src, int32_t* dst) {
    int i;

    ForwardTransform(src, dst);
    for (i = 0; i < 8; i++)
        dst[i] = (dst[i] << 1) - Dequantize(Quantize(dst[i], sShifts[i]), sShifts[i]);
    for (i = 0; i < 8; i++)
        dst[i] = Quantize(dst[i], sShifts[i]);
}

/* Inverse of residuals scaled by per-coefficient shifts. */
void func_081213C4(int32_t* src, int32_t* dst, const int32_t* shifts) {
    int32_t tmp[8];
    int i;

    for (i = 0; i < 8; i++)
        tmp[i] = (int32_t)((uint32_t)src[i] << shifts[i]);
    InverseTransform(tmp, dst);
    DivideBy4(dst);
}

/* Inverse of square-law quantised coefficients. */
void func_081213D4(int32_t* src, int32_t* dst) {
    int32_t tmp[8];
    int i;

    for (i = 0; i < 8; i++)
        tmp[i] = Dequantize(src[i], sShifts[i]);
    InverseTransform(tmp, dst);
    DivideBy4(dst);
}
