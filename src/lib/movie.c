#include "macros.h"
#include "snd_stream.h"
#include "movie.h"
#include "gba/defines.h"
#include "gba/io_reg.h"
#include <stddef.h>
#include "types.h"

#ifdef PLATFORM_PC
void PortMovieFrame(void);
void PortLog(const char*, ...);
#endif

#define MOVIE_TICKS_PER_FRAME 228
#define MOVIE_SECONDS_PER_TICK 0.000073433f

MoviePlayer* gMoviePlayer EWRAM_COMMON(4);
u8 gUnk_0203C7C8[8] EWRAM_COMMON(8);
MovieHeap gMovieHeap EWRAM_COMMON(16);

void MovieSetCallbacks(MovieAllocFunc a, MovieAllocFunc b, MovieFreeFunc c, MovieFreeFunc d) {
    MovieSetHeapCallbacks(a, b, c, d);
    SndStreamSetCallbacks(a, b, c, d);
}

s32 MovieStart(void* a) {
    void* dstA1;
    void* dstA2;
    void* dstB1;
    void* dstB2;
    u32 lenA1;
    u32 lenA2;
    u32 lenB1;
    u32 lenB2;
    s32 i;
    u32 channels;

#ifdef PLATFORM_PC
    dstB1 = dstB2 = NULL;
    lenB1 = lenB2 = 0;
#endif

    gMoviePlayer = MovieOpen(a);

    if (gMoviePlayer == NULL) {
        return 0;
    }

    channels = MovieGetChannels(gMoviePlayer);
#ifdef PLATFORM_PC
    PortLog("Movie: %ux%u, %.2f fps, %u frames, %u Hz, %u channels",
            gMoviePlayer->width, gMoviePlayer->height, gMoviePlayer->frameRate,
            gMoviePlayer->frameCount, gMoviePlayer->sampleRate, channels);
#endif

    if (channels != 0) {
        SndStreamInit(MovieGetSampleRate(gMoviePlayer), channels);

        for (i = 0; i < 4; i++) {
            if (channels == 1) {
                SndStreamLock(0, MovieGetAudioBlockSamples(gMoviePlayer), &dstA1, &lenA1, &dstA2, &lenA2);
                MovieDecodeAudioBlock(gMoviePlayer, dstA1, lenA1, dstA2, lenA2, dstB1, lenB1, dstB2, lenB2);
                SndStreamUnlock(0);
                MovieAdvanceAudioBlock(gMoviePlayer);
            } else {
                SndStreamLock(0, MovieGetAudioBlockSamples(gMoviePlayer), &dstA1, &lenA1, &dstA2, &lenA2);
                SndStreamLock(1, MovieGetAudioBlockSamples(gMoviePlayer), &dstB1, &lenB1, &dstB2, &lenB2);
                MovieDecodeAudioBlock(gMoviePlayer, dstA1, lenA1, dstA2, lenA2, dstB1, lenB1, dstB2, lenB2);
                SndStreamUnlock(0);
                SndStreamUnlock(1);
                MovieAdvanceAudioBlock(gMoviePlayer);
            }
        }
    }

    return 1;
}

void MoviePlay(s32 (*a)(s32), s32 b) {
    void* dstA1;
    void* dstA2;
    void* dstB1;
    void* dstB2;
    u32 lenA1;
    u32 lenA2;
    u32 lenB1;
    u32 lenB2;
    u32 channels;
    s32 ok;
    s32 w;
    s32 h;
    s32 x;
    s32 y;

#ifdef PLATFORM_PC
    dstB1 = dstB2 = NULL;
    lenB1 = lenB2 = 0;
#endif

    MovieGetSize(gMoviePlayer, &w, &h);
    x = (DISPLAY_WIDTH - w) >> 1;
    y = (DISPLAY_HEIGHT - h) >> 1;
    MovieDrawFrame(gMoviePlayer, (u16*)VRAM + (y * DISPLAY_WIDTH + x));
    MovieAdvanceFrame(gMoviePlayer);
    SndStreamStart();
    channels = MovieGetChannels(gMoviePlayer);

    if (channels != 0) {
        ok = 1;
    } else {
        ok = 0;
    }

    while (1) {
        while (!MovieSyncFrame(gMoviePlayer)) {
#ifdef PLATFORM_PC
            /* Hardware interrupts advance the cartridge during this wait. */
            PortMovieFrame();
#endif
        }

        MovieDrawFrame(gMoviePlayer, (u16*)VRAM + (y * DISPLAY_WIDTH + x));

        if (!MovieAdvanceFrame(gMoviePlayer)) {
            break;
        }

        if (ok) {
            if (channels == 1) {
                SndStreamLock(0, MovieGetAudioBlockSamples(gMoviePlayer), &dstA1, &lenA1, &dstA2, &lenA2);
                MovieDecodeAudioBlock(gMoviePlayer, dstA1, lenA1, dstA2, lenA2, dstB1, lenB1, dstB2, lenB2);
                SndStreamUnlock(0);

                if (!MovieAdvanceAudioBlock(gMoviePlayer)) {
                    ok = 0;
                }
            } else {
                SndStreamLock(0, MovieGetAudioBlockSamples(gMoviePlayer), &dstA1, &lenA1, &dstA2, &lenA2);
                SndStreamLock(1, MovieGetAudioBlockSamples(gMoviePlayer), &dstB1, &lenB1, &dstB2, &lenB2);
                MovieDecodeAudioBlock(gMoviePlayer, dstA1, lenA1, dstA2, lenA2, dstB1, lenB1, dstB2, lenB2);
                SndStreamUnlock(0);
                SndStreamUnlock(1);

                if (!MovieAdvanceAudioBlock(gMoviePlayer)) {
                    ok = 0;
                }
            }
        }

        if (a != NULL && a(b) != 0) {
            break;
        }
    }
#ifdef PLATFORM_PC
    PortLog("Movie: stopped at frame %u/%u", gMoviePlayer->frameIndex, gMoviePlayer->frameCount);
#endif
}

void MovieClose() {
#ifdef PLATFORM_PC
    if (gMoviePlayer == NULL)
        return;
#endif
    if (MovieGetChannels(gMoviePlayer)) {
        SndStreamClose();
    }

    MovieFree(gMoviePlayer);
#ifdef PLATFORM_PC
    gMoviePlayer = NULL;
#endif
}

void MovieUpdate() {
    SndStreamUpdate();
    MovieAdvanceTicks();
}

void MovieSetHeapCallbacks(MovieAllocFunc a, MovieAllocFunc b, MovieFreeFunc c, MovieFreeFunc d) {
    gMovieHeap.iwramAlloc = a;
    gMovieHeap.ewramAlloc = b;
    gMovieHeap.iwramFree = c;
    gMovieHeap.ewramFree = d;
    gMovieHeap.ticks = NULL;
}

void MovieAdvanceTicks() {
    gMovieHeap.ticks = gMovieHeap.ticks + MOVIE_TICKS_PER_FRAME;
}

u8* MovieGetTicks() {
    u8* t;
    u16 vc;

    REG_IME = 0;
    vc = REG_VCOUNT;

    if (vc > DISPLAY_HEIGHT - 1) {
        t = gMovieHeap.ticks + (vc - MOVIE_TICKS_PER_FRAME);
    } else {
        t = gMovieHeap.ticks + vc;
    }

    REG_IME = 1;
    return t;
}

float MovieTicksToSeconds(s32 a) {
    return a * MOVIE_SECONDS_PER_TICK;
}
