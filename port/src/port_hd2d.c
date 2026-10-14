/*
 * HD-2D look (experimental option "HD-2D"; on by default only in the
 * alternative build, configure_pc.py --hd2d).
 *
 * A presentation-only pass in the manner of Octopath Traveler, applied to the
 * game picture of wide scenes (field and battles) before it is scaled to the
 * window. Nothing here feeds the game.
 *
 *   1. the picture is enlarged HD2D_SCALE times (nearest: the pixel art stays
 *      crisp) so the effects are smoother than the GBA pixels;
 *   2. tilt-shift depth of field: rows away from the focus band (where the
 *      characters usually stand) blur more, the far top most; sprites keep
 *      their own pixels so the characters stay sharp, and message windows
 *      are left as they are;
 *   3. bloom: what is brighter than a threshold glows, warm, less so in
 *      scenes that are bright all over (the white castle);
 *   4. grading: a filmic contrast curve, a little more colour, warm
 *      highlights and cool shadows, haze towards the top (the distance) and
 *      a vignette.
 */
#include <SDL3/SDL.h>
#include <string.h>

#include "port/port.h"
#include "port/port_hd2d.h"

#define HD2D_SCALE 3
#define HD2D_MAX_W (PORT_MAX_W * HD2D_SCALE)
#define HD2D_MAX_H (PORT_MAX_H * HD2D_SCALE)

/* Allocated on first use: large static arrays would move the executable's
   embedded GBA address space. */
static uint32_t* sBig;
static uint32_t* sTmp;
static uint32_t* sOut;
static uint8_t* sMaskBig;
/* prefix sums for the variable box blurs: one row / one column at a time */
static uint32_t (*sSum)[3];
/* bloom at the game's resolution */
static int32_t (*sBright)[3];
static int32_t (*sBloomTmp)[3];
static uint8_t sCurve[256];
static uint16_t* sVignette;
static int sVigW, sVigH;

int PortHd2dActive(void) {
    return gPortConfig.hd2d != 0;
}

static int Init(int W, int H) {
    int i, x, y;

    if (sBig == NULL) {
        size_t big = (size_t)HD2D_MAX_W * HD2D_MAX_H;
        sBig = SDL_malloc(big * 4);
        sTmp = SDL_malloc(big * 4);
        sOut = SDL_malloc(big * 4);
        sMaskBig = SDL_malloc(big);
        sVignette = SDL_malloc(big * 2);
        sSum = SDL_malloc(sizeof(*sSum) * ((HD2D_MAX_W > HD2D_MAX_H ? HD2D_MAX_W : HD2D_MAX_H) + 1));
        sBright = SDL_malloc(sizeof(*sBright) * PORT_MAX_W * PORT_MAX_H);
        sBloomTmp = SDL_malloc(sizeof(*sBloomTmp) * PORT_MAX_W * PORT_MAX_H);
        if (!sBig || !sTmp || !sOut || !sMaskBig || !sVignette || !sSum || !sBright || !sBloomTmp) {
            PortLog("hd2d: out of memory");
            return 0;
        }
    }

    if (sCurve[255] == 0) {
        /* filmic S-curve: deeper shadows, rolled-off highlights */
        for (i = 0; i < 256; i++) {
            float v = i / 255.0f;
            float s = v * v * (3.0f - 2.0f * v);
            float c = v * 0.30f + s * 0.70f;
            sCurve[i] = (uint8_t)SDL_clamp((int)(c * 255.0f + 0.5f), 0, 255);
        }
    }
    if (W != sVigW || H != sVigH) {
        sVigW = W;
        sVigH = H;
        for (y = 0; y < H; y++) {
            for (x = 0; x < W; x++) {
                float dx = (x - W * 0.5f) / (W * 0.5f), dy = (y - H * 0.52f) / (H * 0.5f);
                float d = dx * dx * 0.8f + dy * dy;
                float v = 1.0f - 0.38f * SDL_powf(SDL_min(d, 1.6f) / 1.6f, 1.6f);
                sVignette[y * W + x] = (uint16_t)(v * 256.0f);
            }
        }
    }
    return 1;
}

/* Blur radius (output pixels) of a row: sharp in the focus band, growing
   towards the top (far, stronger) and the bottom (near). */
static int RowRadius(int y, int H) {
    float focus = H * 0.56f, band = H * 0.16f, ramp = H * 0.30f;
    float d = y - focus;
    float t;

    if (d < 0)
        t = (-d - band) / ramp;
    else
        t = (d - band * 1.2f) / (ramp * 1.3f);
    t = SDL_clamp(t, 0.0f, 1.0f);
    return (int)(t * t * (HD2D_SCALE * 2.6f) + 0.5f);
}

static void BlurRows(const uint32_t* src, uint32_t* dst, int W, int H) {
    int x, y, c;

    for (y = 0; y < H; y++) {
        const uint32_t* s = src + y * W;
        uint32_t* d = dst + y * W;
        int r = RowRadius(y, H);

        if (r == 0) {
            memcpy(d, s, W * 4);
            continue;
        }
        sSum[0][0] = sSum[0][1] = sSum[0][2] = 0;
        for (x = 0; x < W; x++) {
            sSum[x + 1][0] = sSum[x][0] + ((s[x] >> 16) & 255);
            sSum[x + 1][1] = sSum[x][1] + ((s[x] >> 8) & 255);
            sSum[x + 1][2] = sSum[x][2] + (s[x] & 255);
        }
        for (x = 0; x < W; x++) {
            int a = SDL_max(x - r, 0), b = SDL_min(x + r + 1, W), n = b - a;
            uint32_t v[3];
            for (c = 0; c < 3; c++)
                v[c] = (sSum[b][c] - sSum[a][c]) / n;
            d[x] = 0xFF000000u | v[0] << 16 | v[1] << 8 | v[2];
        }
    }
}

static void BlurColumns(const uint32_t* src, uint32_t* dst, int W, int H) {
    static int radius[HD2D_MAX_H];
    int x, y, c;

    for (y = 0; y < H; y++)
        radius[y] = RowRadius(y, H);
    for (x = 0; x < W; x++) {
        sSum[0][0] = sSum[0][1] = sSum[0][2] = 0;
        for (y = 0; y < H; y++) {
            uint32_t p = src[y * W + x];
            sSum[y + 1][0] = sSum[y][0] + ((p >> 16) & 255);
            sSum[y + 1][1] = sSum[y][1] + ((p >> 8) & 255);
            sSum[y + 1][2] = sSum[y][2] + (p & 255);
        }
        for (y = 0; y < H; y++) {
            int r = radius[y], a = SDL_max(y - r, 0), b = SDL_min(y + r + 1, H), n = b - a;
            uint32_t v[3];
            if (r == 0) {
                dst[y * W + x] = src[y * W + x];
                continue;
            }
            for (c = 0; c < 3; c++)
                v[c] = (sSum[b][c] - sSum[a][c]) / n;
            dst[y * W + x] = 0xFF000000u | v[0] << 16 | v[1] << 8 | v[2];
        }
    }
}

/* Separable box blur of the bloom buffer at the game's resolution. */
static void BloomBlur(int w, int h, int r) {
    int x, y, c, k;

    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++)
            for (c = 0; c < 3; c++) {
                int32_t s = 0;
                for (k = -r; k <= r; k++)
                    s += sBright[y * w + SDL_clamp(x + k, 0, w - 1)][c];
                sBloomTmp[y * w + x][c] = s / (2 * r + 1);
            }
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++)
            for (c = 0; c < 3; c++) {
                int32_t s = 0;
                for (k = -r; k <= r; k++)
                    s += sBloomTmp[SDL_clamp(y + k, 0, h - 1) * w + x][c];
                sBright[y * w + x][c] = s / (2 * r + 1);
            }
}

const uint32_t* PortHd2dProcess(const uint32_t* src, const uint8_t* spriteMask, int w, int h, int* outW,
                                int* outH) {
    int W = w * HD2D_SCALE, H = h * HD2D_SCALE;
    int x, y, i, mean, bloomGain, hazeGain;

    if (!Init(W, H)) {
        *outW = w;
        *outH = h;
        return src;
    }
    /* 1. enlarge */
    for (y = 0; y < H; y++) {
        const uint32_t* s = src + (y / HD2D_SCALE) * w;
        const uint8_t* m = spriteMask + (y / HD2D_SCALE) * w;
        for (x = 0; x < W; x++) {
            sBig[y * W + x] = s[x / HD2D_SCALE];
            sMaskBig[y * W + x] = m[x / HD2D_SCALE];
        }
    }
    /* 2. tilt-shift: two passes of each for a softer, rounder blur */
    BlurRows(sBig, sTmp, W, H);
    BlurColumns(sTmp, sOut, W, H);
    BlurRows(sOut, sTmp, W, H);
    BlurColumns(sTmp, sOut, W, H);
    for (i = 0; i < W * H; i++)
        if (sMaskBig[i])
            sOut[i] = sBig[i];

    /* 3. bloom from the bright parts of the game picture, weaker (and no
       haze) when the whole scene is bright */
    {
        uint32_t sum = 0;
        for (i = 0; i < w * h; i += 7) {
            uint32_t p = src[i];
            sum += (((p >> 16) & 255) * 77 + ((p >> 8) & 255) * 150 + (p & 255) * 29) >> 8;
        }
        mean = (int)(sum / ((w * h + 6) / 7));
    }
    bloomGain = SDL_clamp((215 - mean) * 256 / 90, 40, 256);
    hazeGain = SDL_clamp((200 - mean) * 256 / 80, 0, 256);
    for (i = 0; i < w * h; i++) {
        uint32_t p = src[i];
        int r = (p >> 16) & 255, g = (p >> 8) & 255, b = p & 255;
        int lum = (r * 77 + g * 150 + b * 29) >> 8;
        int k = lum > 200 ? (lum - 200) * 3 * bloomGain >> 8 : 0;
        sBright[i][0] = r * k >> 8;
        sBright[i][1] = g * k >> 8;
        sBright[i][2] = b * k >> 8;
    }
    BloomBlur(w, h, 4);
    BloomBlur(w, h, 4);

    /* 4. bloom in, grading, haze and vignette */
    for (y = 0; y < H; y++) {
        int by = SDL_min(y / HD2D_SCALE, h - 1);
        /* haze: the top quarter (the distance) fades towards a warm mist */
        int haze = y < H / 3 ? (H / 3 - y) * 26 / (H / 3) * hazeGain >> 8 : 0;
        for (x = 0; x < W; x++) {
            uint32_t p = sOut[y * W + x];
            if (sMaskBig[y * W + x] == 2) { /* message windows: untouched */
                sOut[y * W + x] = sBig[y * W + x];
                continue;
            }
            const int32_t* bl = sBright[by * w + SDL_min(x / HD2D_SCALE, w - 1)];
            int r = ((p >> 16) & 255) + (bl[0] * 6 >> 3);
            int g = ((p >> 8) & 255) + (bl[1] * 5 >> 3);
            int b = (p & 255) + (bl[2] * 3 >> 3);
            int lum, v = sVignette[y * W + x];

            r = sCurve[SDL_min(r, 255)];
            g = sCurve[SDL_min(g, 255)];
            b = sCurve[SDL_min(b, 255)];
            /* a little more colour; warm highlights, cool shadows */
            lum = (r * 77 + g * 150 + b * 29) >> 8;
            r = lum + (r - lum) * 300 / 256;
            g = lum + (g - lum) * 300 / 256;
            b = lum + (b - lum) * 300 / 256;
            r += (lum - 128) * 14 / 256;
            b -= (lum - 128) * 18 / 256;
            if (haze) {
                r += (236 - r) * haze / 256;
                g += (226 - g) * haze / 256;
                b += (206 - b) * haze / 256;
            }
            r = SDL_clamp(r * v >> 8, 0, 255);
            g = SDL_clamp(g * v >> 8, 0, 255);
            b = SDL_clamp(b * v >> 8, 0, 255);
            sOut[y * W + x] = 0xFF000000u | (uint32_t)r << 16 | (uint32_t)g << 8 | (uint32_t)b;
        }
    }
    *outW = W;
    *outH = H;
    return sOut;
}
