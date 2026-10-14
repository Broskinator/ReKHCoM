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
uint32_t PortFrameCount(void) {
    return (uint32_t)sFrameCount;
}
static float sFitScale; /* see PortFitScreen */
static int sInIrq;
static uint32_t sComposite[PORT_MAX_W * PORT_MAX_H];
static int HarnessListHas(const char* list, uint32_t frame);
static FILE* sLog;
static int sPacedByVsync;
static float sFps;
static float sLogicFps;      /* game main-loop updates per second */
static uint32_t sLogicLast;
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
            /* fractional fit scale: keep pixels even */
            mode = sFitScale > 0 ? SDL_SCALEMODE_PIXELART : SDL_SCALEMODE_NEAREST;
            break;
    }
    SDL_SetTextureScaleMode(sTexture, mode);
}

static int sAutoScale;
/* GBA-size screens (menus, battle start): the game marks each frame. The
   160 lines then fill the window's height at a fractional scale. */
static int sFitMark, sFit;
/* Screen layer (menus over the world), see port_ppu.c. */
extern uint32_t gPortOverlay[];
extern int gPortOverlayW, gPortScreenLayer;
int PortTakeScreenMark(void);
int PortPpuOverlayActive(void);
static SDL_Texture* sOverlayTex;
static int sOverlayTexW;
static float sOverlayScale;

void PortFitScreen(void) {
    sFitMark = 1;
}
static SDL_Texture* sHudTexture;
static int sHudTexW, sHudTexH;
static uint32_t sHudBuf[PORT_MAX_W * PORT_MAX_H];

int gPortIntroRequested;
int gPortEventTest = -1; /* KHCOM_EVENT=list:index (0 Sora, 1 Riku) */
int gPortBattleTest = -1; /* KHCOM_BATTLE=entry:stage[:riku] of the debug battle menu */
int gPortMovieTest = -1; /* KHCOM_MOVIE=id (mode_movie.h) */
int gPortStartupLanguage = -1;

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
    /* screen layer: the 160 GBA lines at the window's height */
    gPortOverlayW = GBA_SCREEN_W;
    sOverlayScale = 0;
    if (gPortConfig.viewMode == PORT_VIEW_AUTO) {
        float f = SDL_min(outH / (float)GBA_SCREEN_H, outW / (float)GBA_SCREEN_W);
        gPortOverlayW = SDL_max(GBA_SCREEN_W, SDL_min((int)SDL_ceilf(outW / f), PORT_MAX_W));
        sOverlayScale = f;
    }
    sFitScale = 0;
    if (sFit && gPortConfig.viewMode == PORT_VIEW_AUTO) {
        float f = SDL_min(outH / (float)GBA_SCREEN_H, outW / (float)GBA_SCREEN_W);
        w = SDL_min((int)SDL_ceilf(outW / f), PORT_MAX_W);
        h = GBA_SCREEN_H;
        sFitScale = f;
        sAutoScale = 0;
    }
    /* HUD layer: a larger whole-number scale, keeping at least 144 lines and
       the GBA width so corner-anchored elements never collide. */
    gPortHudScale = 0;
    if (gPortConfig.viewMode == PORT_VIEW_AUTO && sAutoScale > 0 && sFitScale == 0 && gPortConfig.hudSize > 0) {
        int hs = sAutoScale + gPortConfig.hudSize;
        while (hs > sAutoScale && (outH / hs < 144 || outW / hs < GBA_SCREEN_W))
            hs--;
        if (hs > sAutoScale) {
            gPortHudScale = hs;
            gPortHudW = SDL_min((outW + hs - 1) / hs, PORT_MAX_W);
            gPortHudH = SDL_min((outH + hs - 1) / hs, PORT_MAX_H);
        }
    }
    if (w != gPortScreen.width || h != gPortScreen.height)
        PortPpuSetResolution(w, h);
}

static void OutputRect(int outW, int outH, SDL_FRect* dst) {
    float sw = (float)gPortScreen.width, sh = (float)gPortScreen.height;
    float scale;

    if (sFitScale > 0) {
        dst->w = sw * sFitScale;
        dst->h = sh * sFitScale;
        dst->x = (float)(int)((outW - dst->w) / 2);
        dst->y = (float)(int)((outH - dst->h) / 2);
        return;
    }
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


void PortHdTextDraw(SDL_Renderer* renderer, int hudLayer, float originX, float originY, float scale);

/* Side bars of GBA-size scenes, filled the way the screen chooses
   (PortSideStyle): its edge columns stretched (default), its background
   colour, or its paper texture repeated mirrored from clean bands. */
#define SIDE_STRIP 16
static SDL_Texture* sMenuSideTexture;
static int sSideStyle; /* engine.h: stretch, colour, texture, blur, menu, pattern */
static int sMenuPanelSide = -1;
void PortMenuPanelSide(int side) { sMenuPanelSide = side; }

void PortSideStyle(int style) {
    sSideStyle = style;
}

static uint32_t StripPixel(int sx, int sy, int x, int y) {
    return sComposite[(sy + y) * gPortScreen.width + sx + x];
}

static void SideBar(int sx, int sy, int side, float y, float scale, float x0, float x1, int edgeX) {
    float w = SIDE_STRIP * scale;
    int band;

    if (x1 - x0 <= 0)
        return;
    SDL_SetTextureScaleMode(sTexture, SDL_SCALEMODE_NEAREST);
    if (sSideStyle == 4 || sSideStyle == 5) {
        /* Extend background tiles, never the foreground labels or cursor.
           The title band keeps its existing edge treatment. */
        uint32_t strip[SIDE_STRIP * GBA_SCREEN_H];
        int yy, xx;
        if (!sMenuSideTexture)
            sMenuSideTexture = SDL_CreateTexture(sRenderer, SDL_PIXELFORMAT_XRGB8888,
                SDL_TEXTUREACCESS_STREAMING, SIDE_STRIP, GBA_SCREEN_H);
        if (!sMenuSideTexture)
            return;
        for (yy = 0; yy < GBA_SCREEN_H; yy++) {
            uint32_t tabColor = 0;
            if (sSideStyle == 4 && side == sMenuPanelSide && yy >= 16 && yy < 32) {
                int a, b, best = 0;
                /* The tab is a sprite. Use its row's dominant colour so
                   the eye and an overlapping hand cannot smear outward. */
                for (a = 0; a < 24; a++) {
                    int tabX = side < 0 ? sx : edgeX - 23;
                    uint32_t color = sComposite[(sy + yy) * gPortScreen.width + tabX + a];
                    int count = 0;
                    for (b = 0; b < 24; b++)
                        count += sComposite[(sy + yy) * gPortScreen.width + tabX + b] == color;
                    if (count > best) {
                        best = count;
                        tabColor = color;
                    }
                }
            }
            for (xx = 0; xx < SIDE_STRIP; xx++) {
                int sample = edgeX;
                /* The outside gutter is clear of the grid silhouettes.
                   Repeat its stipple on either side when the panels swap. */
                if (sSideStyle == 4 && side == sMenuPanelSide && yy >= 32 && yy < 132)
                    sample = (side < 0 ? sx : edgeX - 3) + (xx & 3);
                /* Only the world's ornamental bands repeat horizontally;
                   its floor badge and text retain the plain edge fill. */
                if (sSideStyle == 5 && ((yy >= 16 && yy < 24) || (yy >= 136 && yy < (side > 0 ? 154 : 145))))
                    sample = sx + xx;
                strip[yy * SIDE_STRIP + xx] = yy < 16 ?
                    sComposite[(sy + yy) * gPortScreen.width + edgeX] :
                    gPortMenuBackground[(sy + yy) * gPortScreen.width + sample];
                if (tabColor)
                    strip[yy * SIDE_STRIP + xx] = tabColor;
            }
        }
        SDL_UpdateTexture(sMenuSideTexture, NULL, strip, SIDE_STRIP * 4);
        SDL_SetTextureScaleMode(sMenuSideTexture, SDL_SCALEMODE_NEAREST);
        for (float pos = x0; pos < x1; pos += w) {
            float width = SDL_min(w, x1 - pos);
            SDL_FRect src = {0, 0, width / scale, GBA_SCREEN_H};
            SDL_FRect dst = {pos, y, width, GBA_SCREEN_H * scale};
            SDL_RenderTexture(sRenderer, sMenuSideTexture, &src, &dst);
        }
        return;
    }
    if (sSideStyle == 1) {
        /* the strip's most common colour: the screen's background */
        static uint16_t count[4096];
        uint32_t bd = 0;
        int i, best = 0;
        memset(count, 0, sizeof(count));
        for (i = 0; i < GBA_SCREEN_H * SIDE_STRIP; i++) {
            uint32_t c = StripPixel(sx, sy, i % SIDE_STRIP, i / SIDE_STRIP);
            int q = ((c >> 20) & 15) << 8 | ((c >> 12) & 15) << 4 | ((c >> 4) & 15);
            if (++count[q] > best) {
                best = count[q];
                bd = c;
            }
        }
        SDL_FRect r = { x0, y, x1 - x0, GBA_SCREEN_H * scale };
        SDL_SetRenderDrawColor(sRenderer, (bd >> 16) & 255, (bd >> 8) & 255, bd & 255, 255);
        SDL_RenderFillRect(sRenderer, &r);
        return;
    }
    if (sSideStyle != 2) {
        /* the picture's edge column, stretched to the window's edge */
        SDL_FRect es = { (float)edgeX, (float)sy, 1, GBA_SCREEN_H }, ed = { x0, y, x1 - x0, GBA_SCREEN_H * scale };
        SDL_RenderTexture(sRenderer, sTexture, &es, &ed);
        return;
    }
    {
        /* texture: 8-line bands; a band with drawing in it (an emblem)
           takes the nearest clean band above (or below) instead */
        int clean[GBA_SCREEN_H / 8], n = GBA_SCREEN_H / 8, b;
        for (b = 0; b < n; b++) {
            int x, yy, lo = 255, hi = 0, colorful = 0;
            for (yy = b * 8; yy < b * 8 + 8; yy++) {
                for (x = 0; x < SIDE_STRIP; x++) {
                    uint32_t c = StripPixel(sx, sy, x, yy);
                    int r = (c >> 16) & 255, g = (c >> 8) & 255, bl = c & 255;
                    int l = (r * 3 + g * 6 + bl) / 10;
                    lo = SDL_min(lo, l);
                    hi = SDL_max(hi, l);
                    colorful |= SDL_max(r, SDL_max(g, bl)) - SDL_min(r, SDL_min(g, bl)) > 40;
                }
            }
            clean[b] = !colorful && hi - lo <= 64;
        }
        for (band = 0; band < n; band++) {
            int srcBand = band, k;
            float by = y + band * 8 * scale, pos;
            int flip = 1;
            for (k = 0; !clean[srcBand] && k < n; k++) {
                if (band - k >= 0 && clean[band - k]) {
                    srcBand = band - k;
                    break;
                }
                if (band + k < n && clean[band + k]) {
                    srcBand = band + k;
                    break;
                }
            }
            for (pos = side < 0 ? x1 : x0; side < 0 ? pos > x0 : pos < x1; pos += side * w, flip ^= 1) {
                SDL_FRect src = { (float)sx, (float)(sy + srcBand * 8), SIDE_STRIP, 8 };
                SDL_FRect d = { side < 0 ? pos - w : pos, by, w, 8 * scale };
                SDL_RenderTextureRotated(sRenderer, sTexture, &src, &d, 0, NULL, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
            }
        }
    }
}

/* Side bars of style 3: the picture itself, cropped to the window's shape,
   shrunk to a few pixels and enlarged again (a blurred reflection), dimmed. */
static SDL_Texture* sBlurTex;
#define BLUR_W 24
#define BLUR_H 14
#define BLUR_OUT_W 320
#define BLUR_OUT_H 180

static void DrawBlurredReflection(const SDL_FRect* src, int outW, int outH) {
    static uint32_t cell[BLUR_W * BLUR_H], big[BLUR_OUT_W * BLUR_OUT_H];
    float aspect = outW / (float)outH, cw = src->w, ch = src->h;
    int x0, y0, w, h, cx, cy;

    if (sBlurTex == NULL) {
        sBlurTex = SDL_CreateTexture(sRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, BLUR_OUT_W,
                                     BLUR_OUT_H);
        if (sBlurTex == NULL)
            return;
        SDL_SetTextureScaleMode(sBlurTex, SDL_SCALEMODE_NEAREST);
    }
    if (aspect >= cw / ch)
        ch = cw / aspect; /* wider window: keep the full width, crop rows */
    else
        cw = ch * aspect;
    w = (int)cw;
    h = (int)ch;
    x0 = (int)src->x + ((int)src->w - w) / 2;
    y0 = (int)src->y + ((int)src->h - h) / 2;
    /* mean colour of each cell of the cropped picture, dimmed */
    for (cy = 0; cy < BLUR_H; cy++) {
        for (cx = 0; cx < BLUR_W; cx++) {
            int xa = x0 + cx * w / BLUR_W, xb = x0 + (cx + 1) * w / BLUR_W;
            int ya = y0 + cy * h / BLUR_H, yb = y0 + (cy + 1) * h / BLUR_H, x, y, n = 0;
            unsigned r = 0, g = 0, b = 0;
            for (y = ya; y < yb; y++) {
                for (x = xa; x < xb; x++) {
                    uint32_t c = sComposite[y * gPortScreen.width + x];
                    r += (c >> 16) & 255;
                    g += (c >> 8) & 255;
                    b += c & 255;
                    n++;
                }
            }
            if (n == 0)
                n = 1;
            cell[cy * BLUR_W + cx] = 0xFF000000u | ((r * 110 / 255 / n) << 16) | ((g * 110 / 255 / n) << 8) | (b * 110 / 255 / n);
        }
    }
    /* bilinear enlargement in code: smooth, and nothing but a plain copy
       is left for the renderer */
    for (cy = 0; cy < BLUR_OUT_H; cy++) {
        float fy = (cy + 0.5f) * BLUR_H / BLUR_OUT_H - 0.5f;
        int iy = (int)SDL_floorf(fy), jy;
        float ty = fy - iy;

        jy = SDL_clamp(iy + 1, 0, BLUR_H - 1);
        iy = SDL_clamp(iy, 0, BLUR_H - 1);
        for (cx = 0; cx < BLUR_OUT_W; cx++) {
            float fx = (cx + 0.5f) * BLUR_W / BLUR_OUT_W - 0.5f;
            int ix = (int)SDL_floorf(fx), jx, sh;
            float tx = fx - ix;
            uint32_t out = 0xFF000000u;

            jx = SDL_clamp(ix + 1, 0, BLUR_W - 1);
            ix = SDL_clamp(ix, 0, BLUR_W - 1);
            for (sh = 0; sh < 24; sh += 8) {
                float a = (float)((cell[iy * BLUR_W + ix] >> sh) & 255);
                float b = (float)((cell[iy * BLUR_W + jx] >> sh) & 255);
                float c = (float)((cell[jy * BLUR_W + ix] >> sh) & 255);
                float d = (float)((cell[jy * BLUR_W + jx] >> sh) & 255);
                float v = (a * (1 - tx) + b * tx) * (1 - ty) + (c * (1 - tx) + d * tx) * ty;

                out |= (uint32_t)(v + 0.5f) << sh;
            }
            big[cy * BLUR_OUT_W + cx] = out;
        }
    }
    SDL_UpdateTexture(sBlurTex, NULL, big, BLUR_OUT_W * 4);
    SDL_RenderTexture(sRenderer, sBlurTex, NULL, NULL);
}

static void Present(void) {
    int outW, outH;
    SDL_FRect dst;
    float worldX, worldY, worldScale;

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
        if (sSideStyle == 3 && !PortUiActive())
            DrawBlurredReflection(&src, outW, outH);
        SDL_SetTextureScaleMode(sTexture, gPortConfig.filter == PORT_FILTER_LINEAR ? SDL_SCALEMODE_LINEAR
                                                                                 : SDL_SCALEMODE_PIXELART);
        SDL_RenderTexture(sRenderer, sTexture, &src, &dst);
        if (!PortUiActive() && sSideStyle != 3) {
            SideBar((int)src.x, (int)src.y, -1, dst.y, scale, 0.0f, dst.x, (int)src.x);
            SideBar((int)(src.x + src.w) - SIDE_STRIP, (int)src.y, 1, dst.y, scale, dst.x + dst.w, (float)outW,
                    (int)(src.x + src.w) - 1);
        }
        worldScale = scale;
        worldX = dst.x - src.x * scale;
        worldY = dst.y - src.y * scale;
    } else {
        OutputRect(outW, outH, &dst);
        SDL_RenderTexture(sRenderer, sTexture, NULL, &dst);
        worldScale = dst.w / gPortScreen.width;
        worldX = dst.x;
        worldY = dst.y;
    }
    if (!PortUiActive())
        PortHdTextDraw(sRenderer, 0, worldX, worldY, worldScale);
    if (PortPpuOverlayActive() && sOverlayScale > 0) {
        SDL_FRect od;
        if (sOverlayTex == NULL || sOverlayTexW != gPortOverlayW) {
            if (sOverlayTex != NULL)
                SDL_DestroyTexture(sOverlayTex);
            sOverlayTexW = gPortOverlayW;
            sOverlayTex = SDL_CreateTexture(sRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                            sOverlayTexW, GBA_SCREEN_H);
            SDL_SetTextureBlendMode(sOverlayTex, SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(sOverlayTex, SDL_SCALEMODE_PIXELART);
        }
        SDL_UpdateTexture(sOverlayTex, NULL, gPortOverlay, sOverlayTexW * 4);
        od.w = sOverlayTexW * sOverlayScale;
        od.h = GBA_SCREEN_H * sOverlayScale;
        od.x = (float)(int)((outW - od.w) / 2);
        od.y = (float)(int)((outH - od.h) / 2);
        SDL_RenderTexture(sRenderer, sOverlayTex, NULL, &od);
        if (!PortUiActive())
            PortHdTextDraw(sRenderer, 2, od.x, od.y, sOverlayScale);
    }
    if (PortPpuHudActive()) {
        SDL_FRect hd;
        if (sHudTexture == NULL || sHudTexW != gPortHudW || sHudTexH != gPortHudH) {
            if (sHudTexture != NULL)
                SDL_DestroyTexture(sHudTexture);
            sHudTexW = gPortHudW;
            sHudTexH = gPortHudH;
            sHudTexture = SDL_CreateTexture(sRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                            sHudTexW, sHudTexH);
            SDL_SetTextureBlendMode(sHudTexture, SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(sHudTexture, SDL_SCALEMODE_NEAREST);
        }
        PortPpuRenderHud(sHudBuf);
        SDL_UpdateTexture(sHudTexture, NULL, sHudBuf, sHudTexW * 4);
        hd.w = (float)(sHudTexW * gPortHudScale);
        hd.h = (float)(sHudTexH * gPortHudScale);
        hd.x = (float)((outW - (int)hd.w) / 2);
        hd.y = (float)((outH - (int)hd.h) / 2);
        SDL_RenderTexture(sRenderer, sHudTexture, NULL, &hd);
        PortHdTextDraw(sRenderer, 1, hd.x, hd.y, (float)gPortHudScale);
    }
    DrawScanlines(&dst);
    {
        /* state / slot notices: topmost, at the window's scale */
        static uint32_t sNoticeBuf[256 * 14];
        static SDL_Texture* sNoticeTex;
        int nw;

        memset(sNoticeBuf, 0, sizeof(sNoticeBuf));
        nw = PortUiDrawNotice(sNoticeBuf, 256, 14);
        if (nw > 0) {
            float f = SDL_max(1.0f, SDL_min(outH / (float)GBA_SCREEN_H, outW / (float)GBA_SCREEN_W));
            SDL_FRect src = { 0, 0, (float)nw, 14 };
            SDL_FRect nd = { 4 * f, 4 * f, nw * f, 14 * f };

            if (sNoticeTex == NULL) {
                sNoticeTex = SDL_CreateTexture(sRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 14);
                if (sNoticeTex != NULL) {
                    SDL_SetTextureBlendMode(sNoticeTex, SDL_BLENDMODE_BLEND);
                    SDL_SetTextureScaleMode(sNoticeTex, SDL_SCALEMODE_PIXELART);
                }
            }
            if (sNoticeTex != NULL) {
                SDL_UpdateTexture(sNoticeTex, NULL, sNoticeBuf, 256 * 4);
                SDL_RenderTexture(sRenderer, sNoticeTex, &src, &nd);
            }
        }
    }
    if (gPortConfig.showFps) {
        char text[48];
        SDL_snprintf(text, sizeof(text), "%.1f FPS  logic %.1f", sFps, sLogicFps);
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
 * KHCOM_BATTLE=10:4[:1] debug battle menu preset to entry 10, stage 4, as Riku with :1 (A starts it)
 * KHCOM_MODE=30:Mode_Deck[:arg]   at frame 30 switch to that game mode
 * KHCOM_SONG=30:29    at frame 30 start song / sound effect 29
 * KHCOM_FAST=1         do not pace frames (run as fast as possible)
 * KHCOM_VBM=movie.vbm  play a VBA-rr input movie (power-on start, controller 1);
 *                      KHCOM_VBM_OFFSET=n shifts it by n frames
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

/* Input of a VBA-rr movie for one frame (0 past its end). */
static uint16_t MovieKeys(uint32_t frame) {
    static uint8_t* data;
    static size_t frames;
    static int loaded;
    const char* path = SDL_getenv("KHCOM_VBM");
    long index;

    if (path == NULL)
        return 0;
    if (!loaded) {
        size_t size = 0;
        loaded = 1;
        data = SDL_LoadFile(path, &size);
        if (data != NULL && size >= 0x100 && SDL_memcmp(data, "VBM", 4) == 0) {
            uint32_t start = data[0x3C] | data[0x3D] << 8 | data[0x3E] << 16 | (uint32_t)data[0x3F] << 24;
            frames = start < size ? (size - start) / 2 : 0;
            SDL_memmove(data, data + start, frames * 2);
            PortLog("vbm: %s, %u frames", path, (unsigned)frames);
        } else {
            PortLog("vbm: cannot read %s", path);
            frames = 0;
        }
    }
    index = (long)frame + (SDL_getenv("KHCOM_VBM_OFFSET") ? SDL_atoi(SDL_getenv("KHCOM_VBM_OFFSET")) : 0);
    if (index < 0 || (size_t)index >= frames)
        return 0;
    return (uint16_t)((data[index * 2] | data[index * 2 + 1] << 8) & 0x3FF);
}

static uint16_t HarnessKeys(uint32_t frame) {
    static const struct { const char* name; uint16_t bit; } names[] = {
        { "A", 1 }, { "B", 2 }, { "SELECT", 4 }, { "START", 8 }, { "RIGHT", 16 },
        { "LEFT", 32 }, { "UP", 64 }, { "DOWN", 128 }, { "R", 256 }, { "L", 512 },
        { "STOCK", PORT_KEY_STOCK }, { "DODGE", PORT_KEY_DODGE }, { "UNSTOCK", PORT_KEY_UNSTOCK },
    };
    const char* p = SDL_getenv("KHCOM_KEYS");
    uint16_t keys = MovieKeys(frame);

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

    if (SDL_getenv("KHCOM_TRACE_MODES")) {
        extern const char* PortHarnessModeName(void);
        static const char* previous;
        const char* name = PortHarnessModeName();
        if (name != previous) {
            PortLog("harness: frame %u mode %s", (unsigned)sFrameCount, name);
            previous = name;
        }
    }

    /* KHCOM_MODE=frame:name[:arg][,frame:name...]: switch to a game mode
       (menus, screens) or run a harness action (main.c PortHarnessMode). */
    {
        const char* m = SDL_getenv("KHCOM_MODE");
        while (m && *m) {
            const char* comma = SDL_strchr(m, ',');
            const char* colon = SDL_strchr(m, ':');
            if (colon && (!comma || colon < comma) && (uint32_t)SDL_atoi(m) == sFrameCount) {
                extern int PortHarnessMode(const char* name, int arg);
                char item[96], *name = item, *arg;
                size_t n = comma ? (size_t)(comma - colon - 1) : SDL_strlen(colon + 1);
                SDL_strlcpy(item, colon + 1, SDL_min(n + 1, sizeof(item)));
                arg = SDL_strchr(item, ':');
                if (arg)
                    *arg++ = 0;
                if (!PortHarnessMode(name, arg ? SDL_atoi(arg) : 0))
                    PortLog("harness: no mode %s", name);
            }
            m = comma ? comma + 1 : NULL;
        }
    }

    /* KHCOM_SONG=frame:id: start a song or sound effect. */
    {
        const char* m = SDL_getenv("KHCOM_SONG");
        const char* colon = m ? SDL_strchr(m, ':') : NULL;
        if (colon && (uint32_t)SDL_atoi(m) == sFrameCount) {
            extern void PortHarnessSong(int song);
            PortHarnessSong(SDL_atoi(colon + 1));
        }
    }

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
        extern volatile uint32_t gFrameCounter; /* main-loop updates (main.c) */
        uint32_t logic = gFrameCounter;
        double secs = (double)(now - sFpsTime) / (double)sPerfFreq;

        sFps = (float)(sFpsFrames / secs);
        sLogicFps = (float)((uint32_t)(logic - sLogicLast) / secs);
        sLogicLast = logic;
        sFpsFrames = 0;
        sFpsTime = now;
        if (SDL_getenv("KHCOM_FPSLOG"))
            PortLog("fps: display %.2f logic %.2f", sFps, sLogicFps);
    }
}

int PortPacedByVsync(void) {
    return sPacedByVsync;
}

/* One display frame: draw, vertical blank, present, input, pacing. */
static void VBlankCycle(void) {
    *(volatile uint16_t*)(uintptr_t)0x03007FF8 &= ~1; /* BIOS IntrCheck */
    PortBiosAreaRefresh();
    sFit = sFitMark;
    sFitMark = 0;
    {
        /* a menu's sprites reach OAM one frame after it marks the frame:
           keep the layer one frame longer so they never land in the world */
        static int sPrevMark;
        int mark = PortTakeScreenMark();
        gPortScreenLayer = (mark || sPrevMark) && gPortConfig.viewMode == PORT_VIEW_AUTO;
        sPrevMark = mark;
    }
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
    IO16(0x130) = (uint16_t)(~PortInputResolve(PortInputKeys() | HarnessKeys(sFrameCount)) & 0x3FF);
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
            PortLog("bgm: song %08x clock %u tempoD %u tempoI %u status %08x ident %08x | sound ident %08x mix freq %d"
                    " spv %d | dma1 %08x cnt %08x tm0 %04x",
                    *(uint32_t*)m, *(uint32_t*)(m + 0x0C), *(uint16_t*)(m + 0x1C), *(uint16_t*)(m + 0x20),
                    *(uint32_t*)(m + 0x04), *(uint32_t*)(m + 0x34), si ? *(uint32_t*)si : 0,
                    si ? *(int32_t*)(si + 0x14) : 0, si ? *(int32_t*)(si + 0x10) : 0, IO32(0xBC), IO32(0xC4),
                    IO16(0x102));
        }
        if (last)
            PortLog("timing: 300 frames in %.3f s (%.2f Hz), vblanks %u, game updates %u",
                    (double)(t - last) / sPerfFreq, 300.0 * sPerfFreq / (double)(t - last), (unsigned)gVBlankCounter,
                    (unsigned)gFrameCounter);
        last = t;
    }
}

/*
 * One frame per vertical blank, like the cartridge: measured on hardware-
 * accurate timing (port/tools/bizhawk_timing.lua), the game's
 * main loop makes one update per VBlank in every scene, at 59.7275 Hz.
 */
void PortFrame(void) {
    PortStateService();
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

    {
        int i;
        for (i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--intro") == 0)
                gPortIntroRequested = 1;
            else if (strcmp(argv[i], "--data-dir") == 0 && i + 1 < argc)
                PortConfigSetDataPath(argv[++i]);
            else if (strcmp(argv[i], "--rom") == 0 && i + 1 < argc)
                gPortRomPath = argv[++i];
            else if (strcmp(argv[i], "--language") == 0 && i + 1 < argc) {
                const char* language = argv[++i];
                if (strcmp(language, "es") == 0) gPortStartupLanguage = 4;
                else if (strcmp(language, "en") == 0) gPortStartupLanguage = 0;
                else if (strcmp(language, "fr") == 0) gPortStartupLanguage = 1;
                else if (strcmp(language, "de") == 0) gPortStartupLanguage = 2;
                else if (strcmp(language, "it") == 0) gPortStartupLanguage = 3;
            }
        }
    }

    if (SDL_getenv("KHCOM_MOVIE"))
        gPortMovieTest = SDL_atoi(SDL_getenv("KHCOM_MOVIE"));
    if (SDL_getenv("KHCOM_BATTLE")) {
        const char* b = SDL_getenv("KHCOM_BATTLE");
        const char* colon = SDL_strchr(b, ':');
        const char* riku = colon ? SDL_strchr(colon + 1, ':') : NULL;
        gPortBattleTest = (SDL_atoi(b) & 0xFF) | ((colon ? SDL_atoi(colon + 1) & 0xFF : 0) << 8)
                          | ((riku && SDL_atoi(riku + 1)) ? 0x10000 : 0);
    }
    if (SDL_getenv("KHCOM_EVENT")) {
        const char* e = SDL_getenv("KHCOM_EVENT");
        const char* colon = SDL_strchr(e, ':');
        if (colon)
            gPortEventTest = (SDL_atoi(e) << 8) | (SDL_atoi(colon + 1) & 0xFF);
    }

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
        extern void PortBiosFaultInstall(void);
        PortCrashHandlerInstall();
        PortBiosFaultInstall();
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
