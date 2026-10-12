/*
 * Entry point and frame loop.
 *
 * The game keeps its own main loop (AgbMain). Every time it waits for the
 * vertical blank, PortFrame() draws the frame, delivers the interrupts the
 * hardware would raise, presents the picture, mixes audio and paces the loop.
 */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "port/port.h"
#include "port/port_ui.h"
#include "port/port_state.h"

typedef volatile unsigned int vu32;

typedef void (*IntrFunc)(void);

extern void AgbMain(void);
extern IntrFunc gIntrTable[];

/* The game's mutable globals (see port/game_prefix.h). */
static char* sGameData;
static size_t sGameDataSize;
static char* sGameBss;
static size_t sGameBssSize;

static SDL_Window* sWindow;
static SDL_Renderer* sRenderer;
static SDL_Texture* sTexture;
static int sTexW, sTexH;
static jmp_buf sResetJmp;
static void* sDataSnapshot;
static size_t sDataSize;
static uint64_t sNextFrame;
static uint64_t sPerfFreq;
static uint32_t sFrameCount;
static int sInIrq;
static uint32_t sComposite[PORT_MAX_W * PORT_MAX_H];
static int HarnessListHas(const char* list, uint32_t frame);
static FILE* sLog;
static int sPacedByVsync;
static float sFps;
static uint64_t sFpsTime;
static uint32_t sFpsFrames;

void PortLog(const char* fmt, ...) {
    va_list ap;

    if (sLog == NULL)
        sLog = fopen(PortConfigPath("kh_com.log"), "w");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
    if (sLog != NULL) {
        va_start(ap, fmt);
        vfprintf(sLog, fmt, ap);
        fputc('\n', sLog);
        fflush(sLog);
        va_end(ap);
    }
}

void PortFatal(const char* fmt, ...) {
    char msg[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
    PortLog("FATAL: %s", msg);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Kingdom Hearts: Chain of Memories", msg, sWindow);
    exit(1);
}

uint32_t PortTicks(void) {
    return (uint32_t)SDL_GetTicks();
}

/* ---- interrupts -------------------------------------------------------------- */

static int IrqIndex(uint16_t flag) {
    static const uint16_t order[14] = { 0x0080, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020,
                                        0x0040, 0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000 };
    int i;

    for (i = 0; i < 14; i++)
        if (order[i] == flag)
            return i;
    return -1;
}

void PortRaiseIrq(uint16_t flag) {
    uint16_t ie, ime;
    int index;

    IO16(0x202) |= flag;
    ime = IO16(0x208);
    ie = IO16(0x200);
    if (!(ime & 1) || !(ie & flag) || sInIrq)
        return;
    index = IrqIndex(flag);
    if (index < 0 || gIntrTable[index] == NULL)
        return;

    /* Same bookkeeping as the crt0 handler: acknowledge, mask, call. */
    IO16(0x202) &= ~flag;
    sInIrq = 1;
    gIntrTable[index]();
    sInIrq = 0;
    IO16(0x200) = ie;
    IO16(0x208) = ime;
}

/* ---- presentation --------------------------------------------------------------- */

/* Fullscreen uses the configured display mode (closest the monitor offers);
   0x0 selects the desktop resolution without a mode change. */
static void ApplyWindowMode(void) {
    SDL_DisplayID display = SDL_GetDisplayForWindow(sWindow);
    SDL_DisplayMode mode;

    if (gPortConfig.fullscreen && gPortConfig.fullscreenWidth > 0 && gPortConfig.fullscreenHeight > 0
        && SDL_GetClosestFullscreenDisplayMode(display, gPortConfig.fullscreenWidth, gPortConfig.fullscreenHeight,
                                               0.0f, true, &mode)) {
        SDL_SetWindowFullscreenMode(sWindow, &mode);
    } else {
        SDL_SetWindowFullscreenMode(sWindow, NULL);
    }
    SDL_SetWindowFullscreen(sWindow, gPortConfig.fullscreen != 0);
    SDL_SetRenderVSync(sRenderer, gPortConfig.vsync ? 1 : 0);
}

/* Distinct fullscreen resolutions of the window's display, largest first. */
int PortVideoListModes(int* w, int* h, int max) {
    SDL_DisplayMode** modes;
    int count = 0, n, i, j;

    modes = SDL_GetFullscreenDisplayModes(SDL_GetDisplayForWindow(sWindow), &n);
    if (modes == NULL)
        return 0;
    for (i = 0; i < n && count < max; i++) {
        for (j = 0; j < count; j++)
            if (w[j] == modes[i]->w && h[j] == modes[i]->h)
                break;
        if (j == count) {
            w[count] = modes[i]->w;
            h[count] = modes[i]->h;
            count++;
        }
    }
    SDL_free(modes);
    return count;
}

static void UpdateResolution(void);

void PortVideoApplyConfig(void) {
    ApplyWindowMode();
    UpdateResolution();
}

static void EnsureTexture(void) {
    SDL_ScaleMode mode;

    if (sTexture != NULL && sTexW == gPortScreen.width && sTexH == gPortScreen.height)
        goto filter;
    if (sTexture != NULL)
        SDL_DestroyTexture(sTexture);
    sTexW = gPortScreen.width;
    sTexH = gPortScreen.height;
    sTexture = SDL_CreateTexture(sRenderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, sTexW, sTexH);
    if (sTexture == NULL)
        PortFatal("Could not create the frame texture: %s", SDL_GetError());
filter:
    switch (gPortConfig.filter) {
        case PORT_FILTER_LINEAR:
            mode = SDL_SCALEMODE_LINEAR;
            break;
        case PORT_FILTER_SHARP:
            mode = SDL_SCALEMODE_PIXELART;
            break;
        default:
            mode = SDL_SCALEMODE_NEAREST;
            break;
    }
    SDL_SetTextureScaleMode(sTexture, mode);
}

static int sAutoScale;

/* Auto view: size the canvas to the window every frame. */
static void UpdateResolution(void) {
    int outW, outH, w, h;

    if (gPortConfig.viewMode != PORT_VIEW_AUTO) {
        sAutoScale = 0;
        PortConfigView(gPortConfig.viewMode, &w, &h);
    } else {
        if (!SDL_GetCurrentRenderOutputSize(sRenderer, &outW, &outH) || outW <= 0 || outH <= 0)
            return;
        PortConfigAutoView(outW, outH, &w, &h, &sAutoScale);
    }
    if (w != gPortScreen.width || h != gPortScreen.height)
        PortPpuSetResolution(w, h);
}

static void OutputRect(int outW, int outH, SDL_FRect* dst) {
    float sw = (float)gPortScreen.width, sh = (float)gPortScreen.height;
    float scale;

    if (sAutoScale > 0) {
        /* Whole-number scale; the canvas may overhang the window by < 1 pixel. */
        dst->w = sw * sAutoScale;
        dst->h = sh * sAutoScale;
        dst->x = (float)((outW - (int)dst->w) / 2);
        dst->y = (float)((outH - (int)dst->h) / 2);
        return;
    }

    switch (gPortConfig.scaleMode) {
        case PORT_SCALE_STRETCH:
            dst->x = 0;
            dst->y = 0;
            dst->w = (float)outW;
            dst->h = (float)outH;
            return;
        case PORT_SCALE_FIT:
            scale = SDL_min(outW / sw, outH / sh);
            break;
        default:
            scale = (float)(int)SDL_min(outW / sw, outH / sh);
            if (scale < 1.0f)
                scale = SDL_min(outW / sw, outH / sh);
            break;
    }
    dst->w = sw * scale;
    dst->h = sh * scale;
    dst->x = (float)(int)((outW - dst->w) / 2);
    dst->y = (float)(int)((outH - dst->h) / 2);
}

static void DrawScanlines(const SDL_FRect* dst) {
    float step = dst->h / gPortScreen.height;
    float y;

    if (gPortConfig.scanlines <= 0 || step < 2.0f)
        return;
    SDL_SetRenderDrawBlendMode(sRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sRenderer, 0, 0, 0, (Uint8)(gPortConfig.scanlines * 255 / 100 / 2));
    for (y = dst->y + step - SDL_max(1.0f, step / 3.0f); y < dst->y + dst->h; y += step) {
        SDL_FRect r = { dst->x, y, dst->w, SDL_max(1.0f, step / 3.0f) };
        SDL_RenderFillRect(sRenderer, &r);
    }
}


static void Present(void) {
    int outW, outH;
    SDL_FRect dst;

    EnsureTexture();
    /* Overlays go on a copy so the game picture survives while paused. */
    memcpy(sComposite, gPortFrame, (size_t)gPortScreen.width * gPortScreen.height * 4);
    PortUiDraw(sComposite, gPortScreen.width, gPortScreen.height);
    SDL_UpdateTexture(sTexture, NULL, sComposite, gPortScreen.width * 4);
    SDL_GetCurrentRenderOutputSize(sRenderer, &outW, &outH);
    SDL_SetRenderDrawColor(sRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sRenderer);
    if (!gPortScreen.wide && gPortConfig.narrowFit && !PortUiActive()
        && (gPortScreen.width > GBA_SCREEN_W || gPortScreen.height > GBA_SCREEN_H)) {
        /* Scene without a wide view: show the GBA picture at full height
           (bars only at the sides), sampled with the pixel-art filter. */
        SDL_FRect src = { (float)gPortScreen.originX, (float)gPortScreen.originY, GBA_SCREEN_W, GBA_SCREEN_H };
        float scale = SDL_min(outW / (float)GBA_SCREEN_W, outH / (float)GBA_SCREEN_H);
        dst.w = GBA_SCREEN_W * scale;
        dst.h = GBA_SCREEN_H * scale;
        dst.x = (float)(int)((outW - dst.w) / 2);
        dst.y = (float)(int)((outH - dst.h) / 2);
        SDL_SetTextureScaleMode(sTexture, gPortConfig.filter == PORT_FILTER_LINEAR ? SDL_SCALEMODE_LINEAR
                                                                                 : SDL_SCALEMODE_PIXELART);
        SDL_RenderTexture(sRenderer, sTexture, &src, &dst);
    } else {
        OutputRect(outW, outH, &dst);
        SDL_RenderTexture(sRenderer, sTexture, NULL, &dst);
    }
    DrawScanlines(&dst);
    if (gPortConfig.showFps) {
        char text[32];
        SDL_snprintf(text, sizeof(text), "%.1f FPS", sFps);
        SDL_SetRenderDrawColor(sRenderer, 255, 255, 0, 255);
        SDL_SetRenderScale(sRenderer, 2.0f, 2.0f);
        SDL_RenderDebugText(sRenderer, 4, 4, text);
        SDL_SetRenderScale(sRenderer, 1.0f, 1.0f);
    }
    {
        /* KHCOM_OUTPUT_SHOT=frames: save the presented window image. */
        const char* out = SDL_getenv("KHCOM_OUTPUT_SHOT");
        if (out && HarnessListHas(out, sFrameCount)) {
            SDL_Surface* s = SDL_RenderReadPixels(sRenderer, NULL);
            char name[64];
            SDL_snprintf(name, sizeof(name), "out_%05u.bmp", (unsigned)sFrameCount);
            if (s) {
                SDL_SaveBMP(s, PortConfigPath(name));
                SDL_DestroySurface(s);
            }
        }
    }
    SDL_RenderPresent(sRenderer);
}

static void Quit(void);

/* ---- test harness ------------------------------------------------------------------
 * KHCOM_SHOTS=60,120   save shot_<frame>.bmp at those frames
 * KHCOM_KEYS=100-105:START,200:A   hold GBA keys during frame ranges
 * KHCOM_QUIT=400       exit after that frame
 * KHCOM_FAST=1         do not pace frames (run as fast as possible)
 */
static int HarnessListHas(const char* list, uint32_t frame) {
    const char* p = list;

    while (p && *p) {
        if ((uint32_t)SDL_atoi(p) == frame)
            return 1;
        p = SDL_strchr(p, ',');
        if (p)
            p++;
    }
    return 0;
}

static uint16_t HarnessKeys(uint32_t frame) {
    static const struct { const char* name; uint16_t bit; } names[] = {
        { "A", 1 }, { "B", 2 }, { "SELECT", 4 }, { "START", 8 }, { "RIGHT", 16 },
        { "LEFT", 32 }, { "UP", 64 }, { "DOWN", 128 }, { "R", 256 }, { "L", 512 },
    };
    const char* p = SDL_getenv("KHCOM_KEYS");
    uint16_t keys = 0;

    while (p && *p) {
        int a = SDL_atoi(p), b = a;
        const char* dash = SDL_strchr(p, '-');
        const char* colon = SDL_strchr(p, ':');
        const char* comma = SDL_strchr(p, ',');
        size_t i;

        if (colon == NULL)
            break;
        if (dash && dash < colon)
            b = SDL_atoi(dash + 1);
        if (frame >= (uint32_t)a && frame <= (uint32_t)b) {
            for (i = 0; i < SDL_arraysize(names); i++) {
                size_t len = SDL_strlen(names[i].name);
                if (SDL_strncmp(colon + 1, names[i].name, len) == 0
                    && (colon[1 + len] == ',' || colon[1 + len] == '+' || colon[1 + len] == 0))
                    keys |= names[i].bit;
            }
        }
        p = comma ? comma + 1 : NULL;
    }
    return keys;
}

static void Present(void);

static void HarnessFrame(void) {
    const char* shots = SDL_getenv("KHCOM_SHOTS");
    const char* quit = SDL_getenv("KHCOM_QUIT");
    const char* ui = SDL_getenv("KHCOM_OPTIONS_SHOT");
    const char* oam = SDL_getenv("KHCOM_OAMDUMP");
    const char* saveAt = SDL_getenv("KHCOM_SAVESTATE");

    /* KHCOM_SAVESTATE=frame:path[,frame:path...] */
    while (saveAt && *saveAt) {
        const char* colon = SDL_strchr(saveAt, ':');
        const char* comma = SDL_strchr(saveAt, ',');
        if (colon && (uint32_t)SDL_atoi(saveAt) == sFrameCount) {
            char path[512];
            size_t n = comma ? (size_t)(comma - colon - 1) : SDL_strlen(colon + 1);
            SDL_strlcpy(path, colon + 1, SDL_min(n + 1, sizeof(path)));
            PortStateRequestSave(PortConfigPath(path));
        }
        saveAt = comma ? comma + 1 : NULL;
    }

    if (oam && HarnessListHas(oam, sFrameCount)) {
        extern void PortPpuDumpOam(void);
        PortLog("frame %u", (unsigned)sFrameCount);
        PortPpuDumpOam();
    }

    if (ui && HarnessListHas(ui, sFrameCount)) {
        extern void PortUiForceClose(void);
        SDL_Surface* s;
        PortUiOpenOptions();
        Present();
        s = SDL_CreateSurfaceFrom(gPortScreen.width, gPortScreen.height, SDL_PIXELFORMAT_XRGB8888, sComposite,
                                  gPortScreen.width * 4);
        if (s) {
            SDL_SaveBMP(s, PortConfigPath("shot_options.bmp"));
            SDL_DestroySurface(s);
        }
        PortUiForceClose();
    }

    if (shots && HarnessListHas(shots, sFrameCount)) {
        SDL_Surface* s = SDL_CreateSurfaceFrom(gPortScreen.width, gPortScreen.height, SDL_PIXELFORMAT_XRGB8888,
                                               gPortFrame, gPortScreen.width * 4);
        char name[64];
        SDL_snprintf(name, sizeof(name), "shot_%05u.bmp", (unsigned)sFrameCount);
        if (s) {
            SDL_SaveBMP(s, PortConfigPath(name));
            SDL_DestroySurface(s);
        }
    }
    if (quit && sFrameCount >= (uint32_t)SDL_atoi(quit))
        Quit();
}

/* ---- events / pacing --------------------------------------------------------------- */

static void Quit(void) {
    PortSramSyncIfDirty();
    PortConfigSave();
    PortAudioShutdown();
    SDL_Quit();
    exit(0);
}

static void PumpEvents(void) {
    SDL_Event ev;

    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_EVENT_QUIT:
                Quit();
                break;
            case SDL_EVENT_KEY_DOWN:
                if (!ev.key.repeat && ev.key.key == SDLK_F5) {
                    PortStateRequestSave(NULL);
                    continue;
                }
                if (!ev.key.repeat && ev.key.key == SDLK_F9) {
                    PortStateRequestLoad(NULL);
                    continue;
                }
                if (ev.key.key == SDLK_F6 || ev.key.key == SDLK_F7) {
                    char msg[32];
                    gPortStateSlot = (gPortStateSlot + (ev.key.key == SDLK_F7 ? 1 : 8)) % 9;
                    if (gPortStateSlot == 0)
                        gPortStateSlot = 9;
                    SDL_snprintf(msg, sizeof(msg), "Slot %d", gPortStateSlot);
                    PortUiNotice(msg);
                    continue;
                }
                if ((ev.key.key == SDLK_RETURN && (ev.key.mod & SDL_KMOD_ALT)) || ev.key.key == SDLK_F11) {
                    gPortConfig.fullscreen = !gPortConfig.fullscreen;
                    ApplyWindowMode();
                    PortConfigSave();
                    continue;
                }
                break;
        }
        if (PortUiHandleEvent(&ev))
            continue;
        PortInputHandleEvent(&ev);
    }
}

/* The frame timer always paces the game; vsync only removes tearing. On a
   display within ~1.5 Hz of the GBA's rate the game follows the display so
   every refresh shows exactly one frame (audio is resampled to match). */
static void Pace(void) {
    uint64_t now = SDL_GetPerformanceCounter();
    SDL_DisplayID display = SDL_GetDisplayForWindow(sWindow);
    const SDL_DisplayMode* dm = display ? SDL_GetCurrentDisplayMode(display) : NULL;
    double hz = GBA_FPS;
    uint64_t period;

    if (SDL_getenv("KHCOM_FAST")) {
        sNextFrame = 0;
        return;
    }
    (void)dm;
    sPacedByVsync = 0;
    period = (uint64_t)(sPerfFreq / hz);

    /* Exact GBA rate (59.7275 Hz) whatever the display does: sleep most of
       the wait, then spin for the last fraction of a millisecond. */
    if (sNextFrame == 0 || now > sNextFrame + period * 4)
        sNextFrame = now;
    sNextFrame += period;
    while ((now = SDL_GetPerformanceCounter()) < sNextFrame) {
        uint64_t leftNs = (sNextFrame - now) * 1000000000ull / sPerfFreq;
        if (leftNs > 1500000)
            SDL_DelayPrecise(leftNs - 1000000);
    }

    sFpsFrames++;
    if (now - sFpsTime >= sPerfFreq) {
        sFps = (float)(sFpsFrames * (double)sPerfFreq / (double)(now - sFpsTime));
        sFpsFrames = 0;
        sFpsTime = now;
    }
}

int PortPacedByVsync(void) {
    return sPacedByVsync;
}

/* One display frame: draw, vertical blank, present, input, pacing. */
static void VBlankCycle(void) {
    *(volatile uint16_t*)(uintptr_t)0x03007FF8 &= ~1; /* BIOS IntrCheck */
    PortBiosAreaRefresh();
    UpdateResolution();

    /* Visible lines (HBlank / VCount events in between). */
    PortPpuRenderFrame();

    /* Line 160: vertical blank. */
    IO16(0x06) = GBA_SCREEN_H;
    IO16(0x04) = (IO16(0x04) & ~6) | 1 | (((IO16(0x04) >> 8) == GBA_SCREEN_H) ? 4 : 0);
    PortDmaVBlank();
    if (IO16(0x04) & 0x08)
        PortRaiseIrq(0x0001);
    PortPpuVBlankLines();
    IO16(0x04) &= ~1;

    PortAudioFrame();
    Present();
    PumpEvents();

    /* Options overlay: the game is paused until it closes. */
    while (PortUiActive()) {
        int w = gPortScreen.width, h = gPortScreen.height;

        UpdateResolution();
        if (w != gPortScreen.width || h != gPortScreen.height)
            PortPpuRenderFrame();
        Present();
        PumpEvents();
        Pace();
    }
    IO16(0x130) = (uint16_t)(~(PortInputKeys() | HarnessKeys(sFrameCount)) & 0x3FF);
    HarnessFrame();
    Pace();

    if (++sFrameCount % 60 == 0)
        PortSramSyncIfDirty();
    if (SDL_getenv("KHCOM_TIMING") && sFrameCount % 300 == 0) {
        static uint64_t last;
        uint64_t t = SDL_GetPerformanceCounter();
        extern vu32 gVBlankCounter, gFrameCounter;
        {
            extern char gMPlayInfo_BGM[];
            extern void* gSoundInfoPtr;
            const uint8_t* m = (const uint8_t*)gMPlayInfo_BGM;
            const uint8_t* si = (const uint8_t*)gSoundInfoPtr;
            PortLog("bgm: song %08x clock %u tempoD %u tempoI %u | mix freq %d spv %d", *(uint32_t*)m,
                    *(uint32_t*)(m + 0x0C), *(uint16_t*)(m + 0x1C), *(uint16_t*)(m + 0x20),
                    si ? *(int32_t*)(si + 0x14) : 0, si ? *(int32_t*)(si + 0x10) : 0);
        }
        if (last)
            PortLog("timing: 300 frames in %.3f s (%.2f Hz), vblanks %u, game updates %u",
                    (double)(t - last) / sPerfFreq, 300.0 * sPerfFreq / (double)(t - last), (unsigned)gVBlankCounter,
                    (unsigned)gFrameCounter);
        last = t;
    }
}

/*
 * The cartridge's main loop updates the game once per VBlank only when the
 * update finishes within the frame. In play the CPU load makes it take two
 * frames, so logic and picture advance at ~30 fps while sound and the VBlank
 * handlers keep running every frame. "Original" speed reproduces that by
 * inserting a VBlank in which the new frame is not ready yet.
 */
extern volatile unsigned short gFrameSyncFlags;
#define FRAME_SYNC_FRAME_READY 0x4

void PortFrame(void) {
    PortStateService();
    if (gPortConfig.gameSpeed == 0 && (gFrameSyncFlags & FRAME_SYNC_FRAME_READY)) {
        gFrameSyncFlags &= ~FRAME_SYNC_FRAME_READY;
        VBlankCycle();
        gFrameSyncFlags |= FRAME_SYNC_FRAME_READY;
    }
    VBlankCycle();
}

/* Movies own their playback loop and run at the cartridge's frame rate,
   independently of the gameplay speed setting and main-loop save states. */
void PortMovieFrame(void) {
    VBlankCycle();
}

/* ---- reset ----------------------------------------------------------------------- */

void PortSoftReset(void) {
    memcpy(sGameData, sDataSnapshot, sDataSize);
    memset(sGameBss, 0, sGameBssSize);
    memset((void*)(uintptr_t)GBA_IO, 0, GBA_IO_SIZE);
    longjmp(sResetJmp, 1);
}

int main(int argc, char** argv) {
    int winW, winH;

    (void)argc;
    (void)argv;

    /* Claim the GBA address range before anything else can land there. */
    if (!PortMemoryInit())
        return 1;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD | SDL_INIT_EVENTS))
        PortFatal("SDL_Init failed: %s", SDL_GetError());
    PortConfigLoad();
    PortLog("KHCoM PC - SDL %d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);

    {
        int w, h;
        PortConfigView(gPortConfig.viewMode, &w, &h);
        PortPpuSetResolution(w, h);
    }
    winW = gPortScreen.width * gPortConfig.windowScale;
    winH = gPortScreen.height * gPortConfig.windowScale;
    sWindow = SDL_CreateWindow("Kingdom Hearts: Chain of Memories", winW, winH,
                               SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (sWindow == NULL)
        PortFatal("Could not create the window: %s", SDL_GetError());
    SDL_SetWindowMinimumSize(sWindow, GBA_SCREEN_W, GBA_SCREEN_H);
    sRenderer = SDL_CreateRenderer(sWindow, NULL);
    if (sRenderer == NULL)
        PortFatal("Could not create the renderer: %s", SDL_GetError());
    PortLog("Renderer: %s", SDL_GetRendererName(sRenderer));
    ApplyWindowMode();
    sPerfFreq = SDL_GetPerformanceFrequency();
    sFpsTime = SDL_GetPerformanceCounter();

    if (!PortRomLoad())
        return 1;
    PortSramLoad();
    PortInputInit();
    PortAudioInit();
    PortUiInit();

    PortBiosAreaRefresh();
    PortGameSections(&sGameData, &sGameDataSize, &sGameBss, &sGameBssSize);
    sDataSize = sGameDataSize;
    sDataSnapshot = malloc(sDataSize ? sDataSize : 1);
    memcpy(sDataSnapshot, sGameData, sDataSize);
    PortLog("Game globals: data %u bytes, bss %u bytes", (unsigned)sGameDataSize, (unsigned)sGameBssSize);

    {
        extern void PortWatchdogStart(void);
        extern void PortCrashHandlerInstall(void);
        PortCrashHandlerInstall();
        PortWatchdogStart();
    }
    if (SDL_getenv("KHCOM_LOADSTATE"))
        PortStateRequestLoad(PortConfigPath(SDL_getenv("KHCOM_LOADSTATE")));
    setjmp(sResetJmp);
    IO16(0x130) = 0x3FF;
    AgbMain();
    Quit();
    return 0;
}
