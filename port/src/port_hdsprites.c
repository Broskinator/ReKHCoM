/*
 * HD sprites (HD mod, beta): sprites of the cartridge drawn with Re:CoM's HD
 * art where the two are the same picture (HUD_HD_RECOM.md). The first group
 * is the battle cards: illustration, category border and value digit.
 *
 * A sprite is recognised by the ROM tiles it shows (and, for the digits that
 * share one tile set, by its frame), so no game code is touched: engine.c
 * hands every OAM part its tiles, frame and box in the frame's coordinates
 * (PortOamSetHdPart). The importer (khhd_import.py, the launcher) writes one
 * PNG per sprite under hd/sprites/, covering the sprite's own box at a
 * higher resolution; each OAM part is drawn as a quad of its piece of that
 * image, with the part's own affine transform, so rotation, scaling and
 * flips follow the game exactly. Nothing is shipped with ReKHCoM.
 *
 * Missing files keep the cartridge's sprite.
 */
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

#include "port/port.h"
#include "port/port_hdsprites.h"
#include "../decompui/third_party/stb_image.h"

typedef struct HdSpriteDef {
    const void* tiles;
    const void* frame; /* NULL: any frame with these tiles */
    const char* name;
    int16_t x0, y0, w, h; /* the sprite's box in its frame's coordinates */
    uint8_t group;
} HdSpriteDef;

#include "port/hd_sprites.inc"

#define HD_SPRITE_COUNT ((int)(sizeof(kHdSprites) / sizeof(kHdSprites[0])))
#define MAX_DRAWS 128

typedef struct HdSpriteState {
    SDL_Texture *image, *white;
    uint32_t lastUsed;
    size_t bytes;
    uint8_t present; /* 0 not looked up yet, 1 missing, 2 on disk */
} HdSpriteState;

static HdSpriteState sState[HD_SPRITE_COUNT];
static size_t sBytes;
static SDL_Renderer* sRenderer;

typedef struct HdSpriteDraw {
    int id;
    float x[4], y[4]; /* HUD canvas corners: top-left, top-right, bottom-right, bottom-left of the part */
    float u0, v0, u1, v1;
    uint8_t alpha;
} HdSpriteDraw;
static HdSpriteDraw sDraws[MAX_DRAWS];
static int sDrawCount;

static int GroupOn(int group) {
    switch (group) {
    case 1: return gPortConfig.hdCards;
    default: return 0;
    }
}

int PortHdSpriteFind(const void* tiles, const void* frame) {
    int i;
    if (!tiles || !gPortConfig.hdCards)
        return 0;
    for (i = 0; i < HD_SPRITE_COUNT; i++) {
        const HdSpriteDef* d = &kHdSprites[i];
        if (d->tiles != tiles || (d->frame && d->frame != frame) || !GroupOn(d->group))
            continue;
        if (!sState[i].present) {
            char file[160];
            FILE* f;
            SDL_snprintf(file, sizeof(file), "hd/sprites/%s.png", d->name);
            f = fopen(PortConfigPath(file), "rb");
            sState[i].present = f ? 2 : 1;
            if (f) fclose(f);
        }
        return sState[i].present == 2 ? i + 1 : 0;
    }
    return 0;
}

void PortHdSpriteBox(int id, int* x0, int* y0, int* w, int* h) {
    const HdSpriteDef* d = &kHdSprites[id - 1];
    *x0 = d->x0; *y0 = d->y0; *w = d->w; *h = d->h;
}

static void Unload(int i) {
    if (sState[i].image) SDL_DestroyTexture(sState[i].image);
    if (sState[i].white) SDL_DestroyTexture(sState[i].white);
    sState[i].image = sState[i].white = NULL;
    sBytes -= sState[i].bytes;
    sState[i].bytes = 0;
}

static int Load(int i) {
    char file[160];
    int w, h, n, k;
    unsigned char* px;
    SDL_Surface* s;
    size_t need;
    SDL_snprintf(file, sizeof(file), "hd/sprites/%s.png", kHdSprites[i].name);
    px = stbi_load(PortConfigPath(file), &w, &h, &n, 4);
    if (!px || w < 1 || h < 1 || w > 4096 || h > 4096) {
        if (px) stbi_image_free(px);
        sState[i].present = 1;
        PortLog("HD sprites: %s unreadable, the cartridge's sprite stays", file);
        return 0;
    }
    need = (size_t)w * h * 8;
    /* the cards on screen stay; the oldest others make room (32 MB) */
    while (sBytes + need > 32u * 1024 * 1024) {
        int victim = -1;
        for (k = 0; k < HD_SPRITE_COUNT; k++)
            if (sState[k].image && sState[k].lastUsed != PortFrameCount() &&
                (victim < 0 || sState[k].lastUsed < sState[victim].lastUsed))
                victim = k;
        if (victim < 0) break;
        Unload(victim);
    }
    s = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, px, w * 4);
    if (s) {
        sState[i].image = SDL_CreateTextureFromSurface(sRenderer, s);
        /* the white copy for the game's brightening fades */
        for (k = 0; k < w * h; k++) px[k * 4] = px[k * 4 + 1] = px[k * 4 + 2] = 255;
        sState[i].white = SDL_CreateTextureFromSurface(sRenderer, s);
        SDL_DestroySurface(s);
    }
    stbi_image_free(px);
    if (!sState[i].image || !sState[i].white) {
        Unload(i);
        sState[i].present = 1;
        return 0;
    }
    SDL_SetTextureBlendMode(sState[i].image, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(sState[i].white, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(sState[i].image, SDL_SCALEMODE_LINEAR);
    SDL_SetTextureScaleMode(sState[i].white, SDL_SCALEMODE_LINEAR);
    sState[i].bytes = need;
    sBytes += need;
    return 1;
}

void PortHdSpritesBegin(void) {
    sDrawCount = 0;
}

int PortHdSpriteQueue(int id, const float* x, const float* y, int px, int py, int pw, int ph, int alpha) {
    const HdSpriteDef* d;
    HdSpriteDraw* q;
    int k;
    if (id < 1 || id > HD_SPRITE_COUNT || sDrawCount == MAX_DRAWS)
        return 0;
    d = &kHdSprites[id - 1];
    /* a part outside the picture's box (another frame of these tiles) or a
       degenerate transform keeps the cartridge's pixels */
    if (px < d->x0 || py < d->y0 || px + pw > d->x0 + d->w || py + ph > d->y0 + d->h) {
        static int sLogged;
        if (sLogged++ < 8)
            PortLog("HD sprites: %s part %d,%d %dx%d outside its box", d->name, px, py, pw, ph);
        return 0;
    }
    for (k = 0; k < 4; k++)
        if (!(x[k] > -4096.0f && x[k] < 8192.0f && y[k] > -4096.0f && y[k] < 8192.0f))
            return 0;
    q = &sDraws[sDrawCount++];
    q->id = id - 1;
    memcpy(q->x, x, sizeof(q->x));
    memcpy(q->y, y, sizeof(q->y));
    q->u0 = (float)(px - d->x0) / d->w;
    q->v0 = (float)(py - d->y0) / d->h;
    q->u1 = (float)(px + pw - d->x0) / d->w;
    q->v1 = (float)(py + ph - d->y0) / d->h;
    q->alpha = (uint8_t)alpha;
    return 1;
}

static void Quad(SDL_Texture* t, const HdSpriteDraw* q, float ox, float oy, float scale, SDL_FColor c) {
    SDL_Vertex v[4];
    static const int kIndex[6] = { 0, 1, 2, 0, 2, 3 };
    float area;
    int k;
    for (k = 0; k < 4; k++) {
        v[k].position.x = ox + q->x[k] * scale;
        v[k].position.y = oy + q->y[k] * scale;
        v[k].color = c;
    }
    v[0].tex_coord.x = q->u0; v[0].tex_coord.y = q->v0;
    v[1].tex_coord.x = q->u1; v[1].tex_coord.y = q->v0;
    v[2].tex_coord.x = q->u1; v[2].tex_coord.y = q->v1;
    v[3].tex_coord.x = q->u0; v[3].tex_coord.y = q->v1;
    area = (v[1].position.x - v[0].position.x) * (v[3].position.y - v[0].position.y) -
           (v[1].position.y - v[0].position.y) * (v[3].position.x - v[0].position.x);
    if (area < 0.5f && area > -0.5f)
        return;
    /* SDL turns an axis-aligned quad into a texture copy; mirrored or turned
       a quarter (a card spinning), the software renderer's flipped/rotated
       copy reads outside the texture and crashes. A hundredth of a pixel
       keeps such a quad on the triangle path, which draws it correctly. */
    if (!(v[0].position.y == v[1].position.y && v[0].position.x == v[3].position.x &&
          v[1].position.x > v[0].position.x && v[3].position.y > v[0].position.y) &&
        (v[0].position.x == v[1].position.x || v[0].position.y == v[1].position.y)) {
        v[1].position.x += 0.01f;
        v[1].position.y += 0.01f;
    }
    if (SDL_getenv("KHCOM_HDSPRITES_LOG"))
        PortLog("hdsprite f=%u %s (%.1f,%.1f) (%.1f,%.1f) (%.1f,%.1f) (%.1f,%.1f) uv %.3f,%.3f-%.3f,%.3f", PortFrameCount(),
                kHdSprites[q->id].name, v[0].position.x, v[0].position.y, v[1].position.x, v[1].position.y,
                v[2].position.x, v[2].position.y, v[3].position.x, v[3].position.y, q->u0, q->v0, q->u1, q->v1);
    SDL_RenderGeometry(sRenderer, t, v, 4, kIndex, 6);
}

void PortHdSpritesDraw(SDL_Renderer* renderer, float ox, float oy, float scale, int darken, int brighten) {
    int i;
    sRenderer = renderer;
    for (i = 0; i < sDrawCount; i++) {
        const HdSpriteDraw* q = &sDraws[i];
        HdSpriteState* st = &sState[q->id];
        float shade = darken > 0 ? (16 - darken) / 16.0f : 1.0f;
        if (!st->image && !Load(q->id))
            continue;
        st->lastUsed = PortFrameCount();
        Quad(st->image, q, ox, oy, scale, (SDL_FColor){ shade, shade, shade, q->alpha / 255.0f });
        if (brighten > 0)
            Quad(st->white, q, ox, oy, scale, (SDL_FColor){ 1, 1, 1, q->alpha / 255.0f * brighten / 16.0f });
    }
}

/* Renderer gone (or the HD files re-imported): drop the textures and look the files up again. */
void PortHdSpritesShutdown(void) {
    int i;
    for (i = 0; i < HD_SPRITE_COUNT; i++) {
        Unload(i);
        sState[i].present = 0;
    }
}
