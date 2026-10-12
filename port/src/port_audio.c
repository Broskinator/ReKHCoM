/*
 * Sound output.
 *
 * Each frame combines the three GBA sound sources and queues the result on
 * an SDL audio stream:
 *   - DirectSound driven by the m4a mixer (read from gPortMix),
 *   - DirectSound fed by plain FIFO streaming (movies, staff roll),
 *   - the four PSG channels, emulated from the sound registers.
 */
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

#include "m4a.h"
#include "port/port.h"
#include "port/port_audio.h"
#include "port/port_state.h"

#define OUT_RATE 48000
#define FRAME_SAMPLES_MAX PORT_MIX_MAX

static SDL_AudioStream* sStream;
static int sStreamRate;
static float sOut[FRAME_SAMPLES_MAX * 2];
static float sPsgL[FRAME_SAMPLES_MAX], sPsgR[FRAME_SAMPLES_MAX];
static float sDsAR[FRAME_SAMPLES_MAX];

int PortPacedByVsync(void);

/* ---- PSG ------------------------------------------------------------------ */

typedef struct PsgChannel {
    int on;
    int length;
    int lengthEnable;
    int volume;
    int envDir;
    int envPeriod;
    int envTimer;
    double phase;
    uint16_t freq;
    uint16_t lastRegFreq;
    /* channel 1 sweep */
    int sweepTimer;
    int sweepEnabled;
    uint16_t sweepShadow;
    /* channel 4 */
    uint16_t lfsr;
} PsgChannel;

static PsgChannel sPsg[4];
static double sSeqPhase;
static int sSeqStep;

static const uint8_t sDuty[4][8] = {
    { 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 1, 1, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 0 },
};

static const uint8_t sNrx4[4] = { 0x65, 0x6D, 0x75, 0x7D };

static uint16_t RegFreq(int ch) {
    static const uint8_t lo[3] = { 0x64, 0x6C, 0x74 };
    return (uint16_t)(IO8(lo[ch]) | ((IO8(sNrx4[ch]) & 7) << 8));
}

static void LoadEnvelope(PsgChannel* c, uint8_t nrx2) {
    c->volume = nrx2 >> 4;
    c->envDir = (nrx2 >> 3) & 1;
    c->envPeriod = nrx2 & 7;
    c->envTimer = c->envPeriod ? c->envPeriod : 8;
}

static void Trigger(int ch) {
    PsgChannel* c = &sPsg[ch];

    c->on = 1;
    switch (ch) {
        case 0:
        case 1: {
            uint8_t nrx1 = IO8(ch == 0 ? 0x62 : 0x68);
            LoadEnvelope(c, IO8(ch == 0 ? 0x63 : 0x69));
            c->length = 64 - (nrx1 & 63);
            c->freq = c->lastRegFreq = RegFreq(ch);
            if (ch == 0) {
                uint8_t nr10 = IO8(0x60);
                int time = (nr10 >> 4) & 7;
                c->sweepShadow = c->freq;
                c->sweepTimer = time ? time : 8;
                c->sweepEnabled = time != 0 || (nr10 & 7) != 0;
            }
            if ((c->volume == 0 && !c->envDir))
                c->on = 0;
            break;
        }
        case 2:
            c->length = 256 - IO8(0x72);
            c->freq = c->lastRegFreq = RegFreq(2);
            c->phase = 0;
            if (!(IO8(0x70) & 0x80))
                c->on = 0;
            break;
        case 3:
            LoadEnvelope(c, IO8(0x79));
            c->length = 64 - (IO8(0x78) & 63);
            c->lfsr = 0x7FFF;
            if ((c->volume == 0 && !c->envDir))
                c->on = 0;
            break;
    }
    c->lengthEnable = (IO8(sNrx4[ch]) >> 6) & 1;
}

static void PsgFrameRegisters(void) {
    int ch;

    for (ch = 0; ch < 4; ch++) {
        uint8_t nrx4 = IO8(sNrx4[ch]);

        if (nrx4 & 0x80) {
            Trigger(ch);
            IO8(sNrx4[ch]) = nrx4 & 0x7F;
        } else {
            sPsg[ch].lengthEnable = (nrx4 >> 6) & 1;
        }
        if (ch < 3) {
            uint16_t f = RegFreq(ch);
            if (f != sPsg[ch].lastRegFreq) {
                sPsg[ch].lastRegFreq = f;
                sPsg[ch].freq = f;
                if (ch == 0)
                    sPsg[0].sweepShadow = f;
            }
        }
    }
    if (!(IO8(0x70) & 0x80))
        sPsg[2].on = 0;
}

static void SequencerStep(void) {
    int ch;

    if ((sSeqStep & 1) == 0) {
        for (ch = 0; ch < 4; ch++) {
            PsgChannel* c = &sPsg[ch];
            if (c->lengthEnable && c->length > 0 && --c->length == 0)
                c->on = 0;
        }
    }
    if (sSeqStep == 2 || sSeqStep == 6) {
        PsgChannel* c = &sPsg[0];
        uint8_t nr10 = IO8(0x60);
        int time = (nr10 >> 4) & 7;

        if (--c->sweepTimer <= 0) {
            c->sweepTimer = time ? time : 8;
            if (c->sweepEnabled && time) {
                int shift = nr10 & 7;
                int delta = c->sweepShadow >> shift;
                int nf = (nr10 & 8) ? c->sweepShadow - delta : c->sweepShadow + delta;
                if (nf > 2047) {
                    c->on = 0;
                } else if (shift) {
                    c->sweepShadow = (uint16_t)nf;
                    c->freq = (uint16_t)nf;
                }
            }
        }
    }
    if (sSeqStep == 7) {
        static const int envCh[3] = { 0, 1, 3 };
        int i;
        for (i = 0; i < 3; i++) {
            PsgChannel* c = &sPsg[envCh[i]];
            if (c->envPeriod == 0)
                continue;
            if (--c->envTimer <= 0) {
                c->envTimer = c->envPeriod;
                if (c->envDir && c->volume < 15)
                    c->volume++;
                else if (!c->envDir && c->volume > 0)
                    c->volume--;
            }
        }
    }
    sSeqStep = (sSeqStep + 1) & 7;
}

static void PsgRender(int n, int rate) {
    uint8_t nr50 = IO8(0x80), nr51 = IO8(0x81);
    uint16_t cnth = IO16(0x82);
    int masterOn = (IO8(0x84) & 0x80) != 0;
    float ratio = (cnth & 3) == 2 ? 1.0f : ((cnth & 3) == 1 ? 0.5f : 0.25f);
    float volR = (float)((nr50 & 7) + 1);
    float volL = (float)(((nr50 >> 4) & 7) + 1);
    float gain[4];
    const uint8_t* wave = (const uint8_t*)(uintptr_t)(GBA_IO + 0x90);
    int i, ch;
    SoundInfo* info = gSoundInfoPtr;
    extern MusicPlayerInfo gMPlayInfo_BGM;

    for (ch = 0; ch < 4; ch++) {
        gain[ch] = gPortMix.sfxGain;
        if (info != NULL && info->cgbChans != NULL) {
            const MusicPlayerTrack* t = info->cgbChans[ch].track;
            if (t != NULL && gMPlayInfo_BGM.tracks != NULL && t >= gMPlayInfo_BGM.tracks
                && t < gMPlayInfo_BGM.tracks + MAX_MUSICPLAYER_TRACKS)
                gain[ch] = gPortMix.musicGain;
        }
    }

    PsgFrameRegisters();
    for (i = 0; i < n; i++) {
        float l = 0, r = 0;

        sSeqPhase += 512.0 / rate;
        while (sSeqPhase >= 1.0) {
            sSeqPhase -= 1.0;
            SequencerStep();
        }
        if (!masterOn) {
            sPsgL[i] = sPsgR[i] = 0;
            continue;
        }
        for (ch = 0; ch < 4; ch++) {
            PsgChannel* c = &sPsg[ch];
            float s = 0;

            if (!c->on)
                continue;
            switch (ch) {
                case 0:
                case 1: {
                    int duty = IO8(ch == 0 ? 0x62 : 0x68) >> 6;
                    double hz = 131072.0 / (2048 - c->freq);
                    c->phase += hz * 8.0 / rate;
                    if (c->phase >= 8.0)
                        c->phase -= 8.0 * (int)(c->phase / 8.0);
                    s = sDuty[duty][(int)c->phase] ? (float)c->volume : -(float)c->volume;
                    break;
                }
                case 2: {
                    uint8_t nr32 = IO8(0x73);
                    int idx, nib, shift;
                    double hz = 2097152.0 / (2048 - c->freq);
                    c->phase += hz / rate;
                    if (c->phase >= 32.0)
                        c->phase -= 32.0 * (int)(c->phase / 32.0);
                    idx = (int)c->phase;
                    nib = (idx & 1) ? (wave[idx >> 1] & 15) : (wave[idx >> 1] >> 4);
                    s = (float)(nib * 2 - 15);
                    if (nr32 & 0x80) {
                        s *= 0.75f;
                    } else {
                        shift = (nr32 >> 5) & 3;
                        s = shift == 0 ? 0 : s / (float)(1 << (shift - 1));
                    }
                    break;
                }
                case 3: {
                    uint8_t nr43 = IO8(0x7C);
                    double div = (nr43 & 7) ? (double)(nr43 & 7) : 0.5;
                    double hz = 524288.0 / div / (double)(2 << (nr43 >> 4));
                    c->phase += hz / rate;
                    while (c->phase >= 1.0) {
                        int bit = (c->lfsr ^ (c->lfsr >> 1)) & 1;
                        c->phase -= 1.0;
                        c->lfsr = (uint16_t)((c->lfsr >> 1) | (bit << 14));
                        if (nr43 & 8)
                            c->lfsr = (uint16_t)((c->lfsr & ~0x40) | (bit << 6));
                    }
                    s = (c->lfsr & 1) ? -(float)c->volume : (float)c->volume;
                    break;
                }
            }
            s *= gain[ch];
            if (nr51 & (0x10 << ch))
                l += s;
            if (nr51 & (0x01 << ch))
                r += s;
        }
        sPsgL[i] = l * volL * ratio;
        sPsgR[i] = r * volR * ratio;
    }

    /* Channel status bits read back by the game. */
    {
        uint8_t nr52 = IO8(0x84) & 0x80;
        for (ch = 0; ch < 4; ch++)
            if (sPsg[ch].on)
                nr52 |= 1 << ch;
        IO8(0x84) = nr52;
    }
}

/* ---- FIFO streaming ---------------------------------------------------------- */

typedef struct Fifo {
    uint32_t lastSad;
    uint32_t pos;
} Fifo;

static Fifo sFifo[2];
static double sStreamFrac;

static int FifoActive(int dma) {
    uint32_t cnt = IO32(0xB8 + dma * 12);
    return (cnt & 0x80000000u) && ((cnt >> 28) & 3) == 3;
}

static void FifoRead(int dma, float* out, int n, float scale) {
    Fifo* f = &sFifo[dma - 1];
    uint32_t sad = IO32(0xB0 + dma * 12);
    int i;

    if (sad != f->lastSad) {
        f->lastSad = sad;
        f->pos = sad;
    }
    for (i = 0; i < n; i++)
        out[i] = (float)*(const int8_t*)(uintptr_t)f->pos++ * scale;
}

static int InM4aBuffer(uint32_t addr) {
    SoundInfo* info = gSoundInfoPtr;
    uint32_t base;

    if (info == NULL)
        return 0;
    base = (uint32_t)(uintptr_t)info->pcmBuffer;
    return addr >= base && addr < base + sizeof(info->pcmBuffer);
}

/* ---- output -------------------------------------------------------------------- */

void PortAudioVisitState(PortStateVisitor visit) {
    visit(sPsg, sizeof(sPsg));
    visit(&sSeqPhase, sizeof(sSeqPhase));
    visit(&sSeqStep, sizeof(sSeqStep));
    visit(sFifo, sizeof(sFifo));
    visit(&sStreamFrac, sizeof(sStreamFrac));
}

void PortAudioReset(void) {
    if (sStream != NULL)
        SDL_ClearAudioStream(sStream);
}

int PortAudioInit(void) {
    SDL_AudioSpec out = { SDL_AUDIO_F32, 2, OUT_RATE };

    gPortMix.oversample = 4;
    gPortMix.musicGain = 1.0f;
    gPortMix.sfxGain = 1.0f;
    sStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &out, NULL, NULL);
    if (sStream == NULL) {
        PortLog("Audio: could not open the output device: %s", SDL_GetError());
        return 0;
    }
    SDL_ResumeAudioStreamDevice(sStream);
    return 1;
}

void PortAudioShutdown(void) {
    if (sStream != NULL)
        SDL_DestroyAudioStream(sStream);
    sStream = NULL;
}

static void Queue(int n, int rate) {
    if (sStream == NULL)
        return;
    if (rate != sStreamRate) {
        SDL_AudioSpec in = { SDL_AUDIO_F32, 2, rate };
        SDL_SetAudioStreamFormat(sStream, &in, NULL);
        sStreamRate = rate;
    }
    SDL_PutAudioStreamData(sStream, sOut, n * 2 * (int)sizeof(float));

    /* Keep ~4 frames queued. The fill level is smoothed and the playback rate
       corrected by at most 0.3% (about 5 cents), which is inaudible; only a
       large backlog (e.g. after a pause) is dropped outright. */
    {
        static float sQueuedAvg = -1.0f;
        float queued = (float)(SDL_GetAudioStreamQueued(sStream) / (2 * (int)sizeof(float)));
        float target = rate / (float)GBA_FPS * 4.0f;
        float ratio;

        if (queued > target * 6) {
            SDL_ClearAudioStream(sStream);
            queued = 0;
            sQueuedAvg = -1.0f;
        }
        if (sQueuedAvg < 0)
            sQueuedAvg = queued;
        sQueuedAvg += (queued - sQueuedAvg) * 0.02f;
        /* Production already matches consumption (59.7275 frames x samples per
           frame = sample rate), so normally play at exactly 1.0. Only a real
           drift outside the dead band is corrected, by at most 0.15%. */
        if (sQueuedAvg < target * 0.5f)
            ratio = 0.9985f;
        else if (sQueuedAvg > target * 3.0f)
            ratio = 1.0015f;
        else
            ratio = 1.0f;
        SDL_SetAudioStreamFrequencyRatio(sStream, ratio);
        if (SDL_getenv("KHCOM_AUDIOLOG")) {
            static int sTick;
            if (++sTick % 60 == 0) {
                SDL_AudioSpec src, dst;
                SDL_GetAudioStreamFormat(sStream, &src, &dst);
                PortLog("audio: queued %.0f (avg %.0f, target %.0f) ratio %.4f in %d Hz out %d Hz", queued, sQueuedAvg,
                        target, ratio, src.freq, dst.freq);
            }
        }
    }
}

void PortAudioFrame(void) {
    uint16_t cnth = IO16(0x82);
    float master = gPortConfig.masterVolume / 100.0f;
    float aScale = (cnth & 4) ? 4.0f : 2.0f;
    float bScale = (cnth & 8) ? 4.0f : 2.0f;
    int n, rate, i;
    int haveB = 0;
    static float sDsBL[FRAME_SAMPLES_MAX];

    gPortMix.musicGain = gPortConfig.musicVolume / 100.0f;
    gPortMix.sfxGain = gPortConfig.sfxVolume / 100.0f;

    if (gPortMix.ready && InM4aBuffer(IO32(0xBC))) {
        /* m4a: FIFO A carries the right mix, FIFO B the left one. */
        n = gPortMix.count;
        rate = gPortMix.rate;
        for (i = 0; i < n; i++) {
            sDsAR[i] = gPortMix.right[i] / 256.0f * aScale;
            sDsBL[i] = gPortMix.left[i] / 256.0f * bScale;
        }
        haveB = 1;
    } else if (FifoActive(1) && (IO16(0x102) & 0x80)) {
        uint32_t reload = IO16(0x100);
        double hz = 16777216.0 / (double)(0x10000 - reload);
        double want = hz / GBA_FPS + sStreamFrac;

        n = (int)want;
        sStreamFrac = want - n;
        if (n > FRAME_SAMPLES_MAX)
            n = FRAME_SAMPLES_MAX;
        rate = (int)(hz + 0.5);
        FifoRead(1, sDsAR, n, aScale);
        if (FifoActive(2)) {
            FifoRead(2, sDsBL, n, bScale);
            haveB = 1;
        }
    } else {
        rate = 32768 * 2;
        n = (int)(rate / GBA_FPS);
        memset(sDsAR, 0, n * sizeof(float));
    }
    gPortMix.ready = 0;

    PsgRender(n, rate);

    for (i = 0; i < n; i++) {
        float a = sDsAR[i], b = haveB ? sDsBL[i] : 0;
        float l = sPsgL[i], r = sPsgR[i];

        if (cnth & 0x100)
            r += a;
        if (cnth & 0x200)
            l += a;
        if (cnth & 0x1000)
            r += b;
        if (cnth & 0x2000)
            l += b;
        sOut[i * 2 + 0] = SDL_clamp(l / 1024.0f * master, -1.0f, 1.0f);
        sOut[i * 2 + 1] = SDL_clamp(r / 1024.0f * master, -1.0f, 1.0f);
    }
    Queue(n, rate);

    {
        /* KHCOM_WAVDUMP=file: raw float32 stereo of everything queued, with
           the rate of each block in a side file (development aid). */
        static FILE* sDump;
        static FILE* sDumpRates;
        const char* path = SDL_getenv("KHCOM_WAVDUMP");
        if (path != NULL) {
            if (sDump == NULL) {
                char rates[512];
                sDump = fopen(path, "wb");
                SDL_snprintf(rates, sizeof(rates), "%s.rates", path);
                sDumpRates = fopen(rates, "w");
            }
            if (sDump != NULL) {
                fwrite(sOut, sizeof(float), (size_t)n * 2, sDump);
                fprintf(sDumpRates, "%d %d\n", n, rate);
            }
        }
    }
}
