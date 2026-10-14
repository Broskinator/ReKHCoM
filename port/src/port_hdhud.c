/* Experimental image-based HUD pilot. Decode once, draw cached GPU texture.
 * Enemy HP values and interpolation remain owned by the original game task.
 */
#include <SDL3/SDL.h>
#include "port/port.h"
#include "port/port_ui.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#include "../decompui/third_party/stb_image.h"

static SDL_Texture* sFrame;
static SDL_Texture* sWhiteFrame;
static SDL_FRect sSource;
static int sAttempted;

void PortHdHudShutdown(void) {
    if (sFrame) SDL_DestroyTexture(sFrame);
    if (sWhiteFrame) SDL_DestroyTexture(sWhiteFrame);
    sFrame = NULL;
    sWhiteFrame = NULL;
}

void PortHdHudPrepare(SDL_Renderer* renderer) {
    int w, h, channels, x, y, left, top, right = -1, bottom = -1;
    unsigned char* pixels;
    SDL_Surface* surface;
    SDL_Surface* compact;
    SDL_ScaleMode sampling;
    const char* path;
    if (!gPortConfig.hdHud || sAttempted) return;
    sAttempted = 1;
    path = PortConfigPath("assets/hd-hud/pilot/enemy-hp-frame-v2.png");
    if (!stbi_info(path, &w, &h, &channels) || channels != 4 || w < 1 || h < 1 ||
        w > 4096 || h > 4096 || (size_t)w * h > 4194304) {
        PortLog("HD HUD: missing or invalid RGBA pilot; original HUD retained");
        return;
    }
    pixels = stbi_load(path, &w, &h, &channels, 4);
    if (!pixels) return;
    left = w; top = h;
    for (y = 0; y < h; y++) for (x = 0; x < w; x++) {
        unsigned char* p = pixels + ((size_t)y * w + x) * 4;
        if (!p[3]) { p[0] = p[1] = p[2] = 0; continue; }
        if (!x || !y || x == w - 1 || y == h - 1) {
            PortLog("HD HUD: exterior is not transparent; original HUD retained");
            stbi_image_free(pixels);
            return;
        }
        /* Tiny alpha noise outside the drawing must not change its layout.
           Keep antialiasing inside the crop; locate the shape with alpha 32+. */
        if (p[3] >= 32) {
            left = SDL_min(left, x); right = SDL_max(right, x);
            top = SDL_min(top, y); bottom = SDL_max(bottom, y);
        }
    }
    if (right < left) { stbi_image_free(pixels); return; }
    left = SDL_max(0, left - 1); top = SDL_max(0, top - 1);
    right = SDL_min(w - 1, right + 1); bottom = SDL_min(h - 1, bottom + 1);
    surface = SDL_CreateSurfaceFrom(right - left + 1, bottom - top + 1, SDL_PIXELFORMAT_RGBA32,
                                    pixels + ((size_t)top * w + left) * 4, w * 4);
    if (surface) {
        int cw = SDL_min(surface->w, 512);
        int ch = SDL_max(1, surface->h * cw / surface->w);
        compact = SDL_CreateSurface(cw, ch, SDL_PIXELFORMAT_RGBA32);
        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
        if (!compact || !SDL_BlitSurfaceScaled(surface, NULL, compact, NULL, SDL_SCALEMODE_LINEAR)) {
            if (compact) SDL_DestroySurface(compact);
            SDL_DestroySurface(surface);
            stbi_image_free(pixels);
            return;
        }
        sFrame = SDL_CreateTextureFromSurface(renderer, compact);
        /* Cache an alpha-matched white mask for the cartridge's white fades. */
        for (y = 0; y < ch; y++) for (x = 0; x < cw; x++) {
            unsigned char* p = (unsigned char*)compact->pixels + y * compact->pitch + x * 4;
            p[0] = p[1] = p[2] = 255;
        }
        sWhiteFrame = SDL_CreateTextureFromSurface(renderer, compact);
        sSource = (SDL_FRect){0, 0, cw, ch};
        SDL_DestroySurface(compact);
        SDL_DestroySurface(surface);
    }
    stbi_image_free(pixels);
    if (!sFrame || !sWhiteFrame) { PortHdHudShutdown(); return; }
    /* SDL 3.4.18 software linear minification crashes at this panel's small
       native dimensions. Its pixels are already antialiased at load time. */
    sampling = SDL_strcmp(SDL_GetRendererName(renderer), "software") == 0 ? SDL_SCALEMODE_NEAREST : SDL_SCALEMODE_LINEAR;
    SDL_SetTextureBlendMode(sWhiteFrame, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(sWhiteFrame, sampling);
    SDL_SetTextureBlendMode(sFrame, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(sFrame, sampling);
    PortLog("HD HUD: cached transparent enemy HP pilot (%dx%d crop)", right - left + 1, bottom - top + 1);
}

int PortHdHudEnemy(int visible, int ratio, int layer) {
    (void)ratio; (void)layer;
    return visible && gPortConfig.hdHud && sFrame && gPortHudScale && !PortUiActive();
}

void PortHdHudDraw(SDL_Renderer* renderer, int width, int height) {
    float scale = (float)gPortHudScale;
    SDL_FRect frame;
    int rect[4];
    uint16_t bld = *(volatile uint16_t*)(uintptr_t)(GBA_IO + 0x50);
    int effect = (bld >> 6) & 3;
    int amount = SDL_min(*(volatile uint16_t*)(uintptr_t)(GBA_IO + 0x54), 16);
    Uint8 tint = (bld & 0x10) && effect == 3 ? 255 * (16 - amount) / 16 : 255;
    if (!gPortConfig.hdHud || !sFrame || !scale || PortUiActive() || !PortPpuEnemyHudRect(rect)) return;
    frame.w = (rect[2] - rect[0] + 1) * scale;
    frame.h = (rect[3] - rect[1] + 1) * scale;
    frame.x = (width - gPortHudW * scale) * 0.5f + rect[0] * scale;
    frame.y = (height - gPortHudH * scale) * 0.5f + rect[1] * scale;
    if (SDL_getenv("KHCOM_HDHUDLOG")) PortLog("HD HUD rect: %d,%d,%d,%d -> %.1f,%.1f %.1fx%.1f src %.1fx%.1f", rect[0], rect[1], rect[2], rect[3], frame.x, frame.y, frame.w, frame.h, sSource.w, sSource.h);
    SDL_SetTextureColorMod(sFrame, tint, tint, tint);
    SDL_RenderTexture(renderer, sFrame, &sSource, &frame);
    if ((bld & 0x10) && effect == 2 && amount) {
        SDL_SetTextureAlphaMod(sWhiteFrame, amount * 255 / 16);
        SDL_RenderTexture(renderer, sWhiteFrame, &sSource, &frame);
    }
}
