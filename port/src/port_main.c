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

typedef void (*IntrFunc)(void);

extern void AgbMain(void);
extern IntrFunc gIntrTable[];

/* Bounds of the game's mutable globals (see port/game_prefix.h). */
extern char __kh_data_start[], __kh_data_end[], __kh_bss_start[], __kh_bss_end[];

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

static void ApplyWindowMode(void) {
    SDL_SetWindowFullscreen(sWindow, gPortConfig.fullscreen != 0);
    SDL_SetRenderVSync(sRenderer, gPortConfig.vsync ? 1 : 0);
}

void PortVideoApplyConfig(void) {
    int w, h;

    PortConfigView(gPortConfig.viewMode, &w, &h);
    PortPpuSetResolution(w, h);
    ApplyWindowMode();
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

static void OutputRect(int outW, int outH, SDL_FRect* dst) {
    float sw = (float)gPortScreen.width, sh = (float)gPortScreen.height;
    float scale;

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
    PortUiDraw(gPortFrame, gPortScreen.width, gPortScreen.height);
    SDL_UpdateTexture(sTexture, NULL, gPortFrame, gPortScreen.width * 4);
    SDL_GetCurrentRenderOutputSize(sRenderer, &outW, &outH);
    OutputRect(outW, outH, &dst);
    SDL_SetRenderDrawColor(sRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sRenderer);
    SDL_RenderTexture(sRenderer, sTexture, NULL, &dst);
    DrawScanlines(&dst);
    if (gPortConfig.showFps) {
        char text[32];
        SDL_snprintf(text, sizeof(text), "%.1f FPS", sFps);
        SDL_SetRenderDrawColor(sRenderer, 255, 255, 0, 255);
        SDL_SetRenderScale(sRenderer, 2.0f, 2.0f);
        SDL_RenderDebugText(sRenderer, 4, 4, text);
        SDL_SetRenderScale(sRenderer, 1.0f, 1.0f);
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

static void HarnessFrame(void) {
    const char* shots = SDL_getenv("KHCOM_SHOTS");
    const char* quit = SDL_getenv("KHCOM_QUIT");

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

static void Pace(void) {
    uint64_t now = SDL_GetPerformanceCounter();
    uint64_t period = (uint64_t)(sPerfFreq / GBA_FPS);
    SDL_DisplayID display = SDL_GetDisplayForWindow(sWindow);
    const SDL_DisplayMode* dm = display ? SDL_GetCurrentDisplayMode(display) : NULL;

    /* On a ~60 Hz display with vsync, presenting already paces the game. */
    if (SDL_getenv("KHCOM_FAST")) {
        sPacedByVsync = 0;
        sNextFrame = 0;
        return;
    }
    sPacedByVsync = gPortConfig.vsync && dm != NULL && dm->refresh_rate > 58.5f && dm->refresh_rate < 61.5f;
    if (sPacedByVsync) {
        sNextFrame = now;
    } else {
        if (sNextFrame == 0 || now > sNextFrame + period * 4)
            sNextFrame = now;
        sNextFrame += period;
        while ((now = SDL_GetPerformanceCounter()) < sNextFrame) {
            uint64_t left = (sNextFrame - now) * 1000 / sPerfFreq;
            if (left > 2)
                SDL_Delay((Uint32)(left - 1));
        }
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

void PortFrame(void) {
    *(volatile uint16_t*)(uintptr_t)0x03007FF8 &= ~1; /* BIOS IntrCheck */

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
    IO16(0x130) = (uint16_t)(~(PortInputKeys() | HarnessKeys(sFrameCount)) & 0x3FF);
    HarnessFrame();
    Pace();

    if (++sFrameCount % 60 == 0)
        PortSramSyncIfDirty();
}

/* ---- reset ----------------------------------------------------------------------- */

void PortSoftReset(void) {
    memcpy(__kh_data_start, sDataSnapshot, sDataSize);
    memset(__kh_bss_start, 0, (size_t)(__kh_bss_end - __kh_bss_start));
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

    sDataSize = (size_t)(__kh_data_end - __kh_data_start);
    sDataSnapshot = malloc(sDataSize ? sDataSize : 1);
    memcpy(sDataSnapshot, __kh_data_start, sDataSize);

    {
        extern void PortWatchdogStart(void);
        extern void PortCrashHandlerInstall(void);
        PortCrashHandlerInstall();
        PortWatchdogStart();
    }
    setjmp(sResetJmp);
    IO16(0x130) = 0x3FF;
    AgbMain();
    Quit();
    return 0;
}
