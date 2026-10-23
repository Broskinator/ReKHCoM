#ifndef PORT_SCALE2X_SDL_H
#define PORT_SCALE2X_SDL_H
#include <SDL3/SDL.h>
#include "port/port_scale2x.h"

typedef struct PortUpscaleLayer {
    SDL_Texture* texture;
    uint32_t* pixels;
    int width, height, ready;
    SDL_PixelFormat format;
} PortUpscaleLayer;

static void PortUpscaleFree(PortUpscaleLayer* layer) {
    if (layer->texture) SDL_DestroyTexture(layer->texture);
    SDL_free(layer->pixels);
    SDL_zero(*layer);
}

/* ARGB layers are premultiplied before linear sampling: transparent padding
 * cannot introduce black/coloured fringes around HUD sprites and menus. */
static void PortUpscaleUpload(PortUpscaleLayer* layer, SDL_Renderer* renderer,
        const uint32_t* src, int w, int h, SDL_PixelFormat format, int enabled) {
    size_t i, count;
    layer->ready = 0;
    if (!enabled || w <= 0 || h <= 0) {
        PortUpscaleFree(layer);
        return;
    }
    if (!layer->texture || !layer->pixels || w != layer->width || h != layer->height || format != layer->format) {
        PortUpscaleFree(layer);
        layer->texture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, w*2, h*2);
        layer->pixels = SDL_malloc((size_t)w*h*16);
        layer->width = w; layer->height = h; layer->format = format;
        if (!layer->texture || !layer->pixels) {
            PortUpscaleFree(layer);
            return; /* original texture is the fallback */
        }
    }
    if (!SDL_SetTextureBlendMode(layer->texture, format == SDL_PIXELFORMAT_ARGB8888 ?
            SDL_BLENDMODE_BLEND_PREMULTIPLIED : SDL_BLENDMODE_NONE) ||
        !SDL_SetTextureScaleMode(layer->texture, SDL_SCALEMODE_LINEAR)) return;
    PortScale2x(src, w, h, layer->pixels);
    if (format == SDL_PIXELFORMAT_ARGB8888) {
        count = (size_t)w*h*4;
        for (i = 0; i < count; ++i) {
            uint32_t c = layer->pixels[i], a = c >> 24;
            uint32_t r = (((c >> 16) & 255)*a + 127)/255;
            uint32_t g = (((c >> 8) & 255)*a + 127)/255;
            uint32_t b = ((c & 255)*a + 127)/255;
            layer->pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }
    layer->ready = SDL_UpdateTexture(layer->texture, NULL, layer->pixels, w*8);
}

static SDL_Texture* PortUpscaleSource(const PortUpscaleLayer* layer, SDL_Texture* original,
        const SDL_FRect* src, SDL_FRect* scaled, int enabled) {
    if (src) *scaled = *src;
    if (enabled && layer->ready) {
        if (src) { scaled->x *= 2; scaled->y *= 2; scaled->w *= 2; scaled->h *= 2; }
        return layer->texture;
    }
    return original;
}
#endif
