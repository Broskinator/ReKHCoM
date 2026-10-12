/* Native versions of asm/movie_codec.s. No executable ARM code. */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "port/port_movie.h"

static uint16_t Read16(const uint8_t* p) { return p[0] | p[1] << 8; }
static uint32_t Read32(const uint8_t* p) {
    return (uint32_t)Read16(p) | (uint32_t)Read16(p + 2) << 16;
}

/* Preserve component rounding from the selected endpoint. */
static uint16_t Average(uint16_t a, uint16_t b, uint16_t rounding) {
    return (((a & 0x7BDF) + (b & 0x7BDF)) >> 1) | (rounding & 0x0420);
}

static uint32_t AveragePair(uint32_t a, uint32_t b, uint32_t rounding) {
    return (((a & 0x7BDF7BDFu) + (b & 0x7BDF7BDFu)) >> 1) | (rounding & 0x04200420u);
}
static uint32_t Pair(const uint16_t* p) { return p[0] | (uint32_t)p[1] << 16; }
static void PutPair(uint16_t* p, uint32_t value) { p[0] = value; p[1] = value >> 16; }

int PortMovieDecodeKey(uint16_t* dst, uint32_t w, uint32_t h,
                       const uint8_t* src, size_t size) {
    uint32_t bx, by, x, y, i;
    if (!w || !h || (w & 15) || (h & 15) || size < (size_t)w * h / 32) return 0;
    for (by = 0; by < h; by += 16) {
        for (bx = 0; bx < w; bx += 16, src += 8) {
            uint16_t colors[4] = { Read16(src), Read16(src + 2), 0, 0 };
            uint32_t selectors = Read32(src + 4);
            for (i = 0; i < 3; i++) {
                unsigned a = (colors[0] >> (i * 5)) & 31;
                unsigned b = (colors[1] >> (i * 5)) & 31;
                colors[2] |= ((2 * a + b + 1) / 3) << (i * 5);
                colors[3] |= ((a + 2 * b + 1) / 3) << (i * 5);
            }
            for (y = 0; y < 4; y++)
                for (x = 0; x < 4; x++, selectors >>= 2)
                    dst[(by + y * 4) * w + bx + x * 4] = colors[selectors & 3];
        }
    }
    for (y = 0; y < h; y += 4) {
        for (x = 0; x + 4 < w; x += 4) {
            uint16_t a = dst[y * w + x], b = dst[y * w + x + 4];
            uint16_t middle = Average(a, b, a);
            dst[y * w + x + 1] = Average(a, middle, a);
            dst[y * w + x + 2] = middle;
            dst[y * w + x + 3] = Average(middle, b, b);
        }
        for (i = 1; i < 4; i++) dst[y * w + w - 4 + i] = dst[y * w + w - 4];
    }
    /* The ARM vertical pass averages packed pairs. Its logical shift also
       transfers the high pixel's lowest bit into the low pixel's bit 15. */
    for (x = 0; x < w; x += 2) {
        for (y = 0; y + 4 < h; y += 4) {
            uint32_t a = Pair(dst + y * w + x), b = Pair(dst + (y + 4) * w + x);
            uint32_t middle = AveragePair(a, b, a);
            PutPair(dst + (y + 1) * w + x, AveragePair(a, middle, a));
            PutPair(dst + (y + 2) * w + x, middle);
            PutPair(dst + (y + 3) * w + x, AveragePair(middle, b, b));
        }
        for (i = 1; i < 4; i++) PutPair(dst + (h - 4 + i) * w + x, Pair(dst + (h - 4) * w + x));
    }
    return 1;
}

void PortMoviePostProcess(uint16_t* dst, uint32_t w, uint32_t h) {
    uint32_t x, y;
    for (y = 0; y < h; y++)
        for (x = 8; x < w; x += 8)
            dst[y * w + x] = Average(dst[y * w + x - 1], dst[y * w + x], dst[y * w + x]);
    for (y = 8; y < h; y += 8)
        for (x = 0; x < w; x += 2) {
            uint32_t b = Pair(dst + y * w + x);
            PutPair(dst + y * w + x, AveragePair(Pair(dst + (y - 1) * w + x), b, b));
        }
}

typedef struct Delta {
    const uint16_t* prev;
    uint16_t* dst;
    const uint8_t *bits, *bitsEnd, *colors, *colorsEnd, *motion, *end;
    uint32_t word, left, width, height;
    int ok;
} Delta;

static int Bit(Delta* d) {
    int bit;
    if (!d->left) {
        if (d->bitsEnd - d->bits < 4) { d->ok = 0; return 0; }
        d->word = Read32(d->bits); d->bits += 4; d->left = 32;
    }
    bit = d->word >> 31; d->word <<= 1; d->left--;
    return bit;
}

static uint16_t Color(Delta* d) {
    uint16_t value;
    if (d->colorsEnd - d->colors < 2) { d->ok = 0; return 0; }
    value = Read16(d->colors); d->colors += 2;
    return value;
}

static int Motion(Delta* d) {
    unsigned v;
    if (d->motion >= d->end) { d->ok = 0; return 0; }
    v = *d->motion++;
    return ((int)(v >> 4) - 8) * (int)d->width + (int)(v & 15) - 8;
}

/* 00 copies, 01 motion-copies, 100/101 split, 110 adds a packed
   color offset, 111 fills. One-dimensional blocks have one split direction.
   Two-pixel leaves use their final bit to choose fill or two raw pixels. */
static void Block(Delta* d, unsigned pos, unsigned w, unsigned h) {
    unsigned x, y;
    int offset = 0, mode = 0;
    uint16_t color = 0;
    if (!d->ok) return;
    if (!Bit(d)) {
        if (Bit(d)) offset = Motion(d);
    } else {
        if (w * h == 2) {
            if (!Bit(d)) { offset = Motion(d); color = Color(d); mode = 1; }
            else mode = Bit(d) ? 3 : 2;
        } else if (!Bit(d)) {
            int vertical = w == 1 ? 0 : (h == 1 ? 1 : Bit(d));
            if (vertical) {
                Block(d, pos, w / 2, h); Block(d, pos + w / 2, w / 2, h);
            } else {
                Block(d, pos, w, h / 2); Block(d, pos + d->width * (h / 2), w, h / 2);
            }
            return;
        } else if (!Bit(d)) {
            offset = Motion(d); color = Color(d); mode = 1;
        } else mode = 2;
        if (mode == 2) color = Color(d);
    }
    for (y = 0; y < h && d->ok; y++) {
        for (x = 0; x < w; x += (w > 1 ? 2 : 1)) {
            unsigned dest = pos + y * d->width + x;
            int source = (int)dest + offset;
            if (mode >= 2) {
                d->dst[dest] = mode == 3 ? Color(d) : color;
                if (w > 1) d->dst[dest + 1] = mode == 3 ? Color(d) : color;
            } else {
                uint32_t pair;
                if (source < 0 || (unsigned)source + (w > 1 ? 1 : 0) >= d->width * d->height) {
                    d->ok = 0; return;
                }
                pair = d->prev[source];
                if (w > 1) pair |= (uint32_t)d->prev[source + 1] << 16;
                /* Carry from the low pixel propagates to the high pixel. */
                if (mode == 1) pair += (uint32_t)color | (uint32_t)color << 16;
                d->dst[dest] = pair;
                if (w > 1) d->dst[dest + 1] = pair >> 16;
            }
        }
    }
}

int PortMovieDecodeDelta(const uint16_t* prev, uint16_t* dst, uint32_t w, uint32_t h,
                         const uint8_t* src, size_t size) {
    Delta d = {0};
    unsigned x, y, bitSize, colorSize;
    if (!w || !h || (w & 7) || (h & 7) || size < 4) return 0;
    bitSize = Read16(src); colorSize = Read16(src + 2);
    if ((bitSize & 3) || (colorSize & 1) || (size_t)bitSize + colorSize + 4 > size) return 0;
    d.prev = prev; d.dst = dst; d.width = w; d.height = h; d.ok = 1;
    d.bits = src + 4; d.bitsEnd = d.bits + bitSize;
    d.colors = d.bitsEnd; d.colorsEnd = d.colors + colorSize;
    d.motion = d.colorsEnd; d.end = src + size;
    for (y = 0; y < h && d.ok; y += 8)
        for (x = 0; x < w && d.ok; x += 8) Block(&d, y * w + x, 8, 8);
    return d.ok;
}

static const uint16_t sSteps[89] = {
    7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,
    73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,408,
    449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,
    2066,2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,
    7845,8630,9493,10442,11487,12635,13899,15289,16818,18500,20350,22358,
    24633,27086,29794,32767
};

int PortMovieDecodeAudio(const uint8_t* src, size_t size, int8_t* dst,
                         uint32_t bytes, unsigned codec) {
    unsigned i, count = bytes / 2;
    int index;
    uint32_t predictor;
    if (bytes & 1) return 0;
    if (codec == 0) {
        if (size < bytes) return 0;
        for (i = 0; i < count; i++) dst[i] = src[i * 2 + 1];
    } else if (codec == 1) memset(dst, 0, count);
    else if (codec == 2) {
        if (size < 4 + (count + 3) / 4) return 0;
        predictor = (int16_t)Read16(src); index = Read16(src + 2);
        if (index > 88) return 0;
        for (i = 0; i < count; i++) {
            unsigned code = (src[4 + i / 4] >> ((i & 3) * 2)) & 3;
            int step = (int16_t)((code & 1) * sSteps[index] + sSteps[index] / 2);
            if (code & 2) step = -step;
            predictor += (uint32_t)step; dst[i] = predictor >> 8;
            index += (code & 1) ? 1 : -1;
            if (index < 0) index = 0;
            if (index > 88) index = 88;
        }
    } else return 0;
    return 1;
}

#ifndef PORT_MOVIE_TEST
#include "movie.h"
#include "port/port.h"

static void KeyFrame(void* dst, u32 w, u32 h, void* src) {
    if (!PortMovieDecodeKey(dst, w, h, src, gMoviePlayer->frameSizes[gMoviePlayer->frameIndex]))
        PortFatal("Invalid movie key frame %u", gMoviePlayer->frameIndex);
}
static void PostProcess(void* dst, u32 w, u32 h) { PortMoviePostProcess(dst, w, h); }
static void DeltaFrame(void* prev, void* dst, void* src) {
    MoviePlayer* p = gMoviePlayer;
    if (!PortMovieDecodeDelta(prev, dst, p->width, p->height, src, p->frameSizes[p->frameIndex]))
        PortFatal("Invalid movie delta frame %u", p->frameIndex);
}
static void Audio(void* src, void* dst, s32 bytes) {
    MoviePlayer* p = gMoviePlayer;
    size_t size = p->audioBlockSizes[p->audioBlockIndex] - 4;
    if (p->channels == 2) size /= 2;
    if (bytes < 0 || bytes / 2 > 0x2000 || !PortMovieDecodeAudio(src, size, dst, bytes, p->audioCodecId))
        PortFatal("Invalid movie audio block %u", p->audioBlockIndex);
}
void PortMovieSetupVideo(MoviePlayer* p) {
    p->videoCodecCode = p->deltaCodecCode = NULL;
    p->decodeKeyFrame = KeyFrame; p->postProcessFrame = PostProcess; p->decodeDeltaFrame = DeltaFrame;
}
void PortMovieSetupAudio(MoviePlayer* p) { p->audioCodecCode = NULL; p->decodeAudio = Audio; }
#endif
