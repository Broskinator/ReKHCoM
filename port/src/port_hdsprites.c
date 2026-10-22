/*
 * HD sprites (HD mod, beta): sprites of the cartridge drawn with Re:CoM's HD
 * art where the two are the same picture (HUD_HD_RECOM.md). The first group
 * is the battle cards (illustration, category border and value digit), the
 * second the title screen's logo.
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
    uint8_t alpha, dark, white;
} HdSpriteDraw;
/* per layer: 0 the game's picture, 1 the HUD canvas, 2 the screen layer (menus, the title's logo) */
#define HD_LAYERS 3
static HdSpriteDraw sDraws[HD_LAYERS][MAX_DRAWS];
static int sDrawCount[HD_LAYERS];

static int GroupOn(int group) {
    switch (group) {
    case 1: return gPortConfig.hdCards;
    case 2: return gPortConfig.hdTitle;
    case 3: return gPortConfig.hdCards; /* the battle HUD's faces and words go with the cards */
    case 4: return gPortConfig.hdCards; /* the world select's worlds */
    default: return 0;
    }
}

/* hd/sprites/<name>.png in the data folder (KHCOM_HD_SPRITES=<folder>: the test batteries' shared copy) */
static const char* SpritePath(char* file, size_t size, const char* name) {
    const char* dir = SDL_getenv("KHCOM_HD_SPRITES");
    if (dir) {
        SDL_snprintf(file, size, "%s/%s.png", dir, name);
        return file;
    }
    SDL_snprintf(file, size, "hd/sprites/%s.png", name);
    return PortConfigPath(file);
}

int PortHdSpriteFind(const void* tiles, const void* frame) {
    int i;
    if (!tiles || (!gPortConfig.hdCards && !gPortConfig.hdTitle))
        return 0;
    for (i = 0; i < HD_SPRITE_COUNT; i++) {
        const HdSpriteDef* d = &kHdSprites[i];
        if (d->tiles != tiles || (d->frame && d->frame != frame) || !GroupOn(d->group))
            continue;
        if (!sState[i].present) {
            char file[160];
            FILE* f;
            f = fopen(SpritePath(file, sizeof(file), d->name), "rb");
            sState[i].present = f ? 2 : 1;
            if (f) fclose(f);
        }
        return sState[i].present == 2 ? i + 1 : 0;
    }
    return 0;
}

int PortHdSpriteGroup(int id) {
    return id >= 1 && id <= HD_SPRITE_COUNT ? kHdSprites[id - 1].group : 0;
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

/* Group 4 (the world images): Re:CoM's picture stands on an opaque glow, so it keeps only the cartridge sprite's
   silhouette, read from its frame and tiles and sampled bilinearly (a soft edge one GBA pixel wide). */
static void MaskBySprite(unsigned char* px, int w, int h, const HdSpriteDef* d) {
    static const unsigned char kSize[3][4][2] = { { { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 } },
                                                  { { 16, 8 }, { 32, 8 }, { 32, 16 }, { 64, 32 } },
                                                  { { 8, 16 }, { 8, 32 }, { 16, 32 }, { 32, 64 } } };
    const unsigned char* frame = d->frame;
    const unsigned char* tiles = d->tiles;
    unsigned char* mask;
    int n, k, x, y;
    if (!frame || d->w < 1 || d->h < 1 || !(mask = SDL_calloc((size_t)d->w * d->h, 1)))
        return;
    n = frame[0] | frame[1] << 8;
    for (k = 0; k < n && k < 64; k++) {
        const unsigned char* a = frame + 2 + k * 6;
        int a0 = a[0] | a[1] << 8, a1 = a[2] | a[3] << 8, a2 = a[4] | a[5] << 8;
        int shape = a0 >> 14, pw, ph, ox = a1 & 0x1FF, oy = a0 & 0xFF, tx, ty;
        if (shape > 2)
            continue;
        pw = kSize[shape][a1 >> 14][0];
        ph = kSize[shape][a1 >> 14][1];
        if (ox & 0x100) ox -= 512;
        if (oy & 0x80) oy -= 256;
        for (ty = 0; ty < ph; ty++)
            for (tx = 0; tx < pw; tx++) {
                int sx = (a1 & 0x1000) ? pw - 1 - tx : tx, sy = (a1 & 0x2000) ? ph - 1 - ty : ty;
                int tile = (a2 & 0x3FF) + (sy >> 3) * (pw >> 3) + (sx >> 3);
                int b = tiles[tile * 32 + (sy & 7) * 4 + ((sx & 7) >> 1)], v = (sx & 1) ? b >> 4 : b & 15;
                int mx = ox + tx - d->x0, my = oy + ty - d->y0;
                if (v && mx >= 0 && my >= 0 && mx < d->w && my < d->h)
                    mask[my * d->w + mx] = 255;
            }
    }
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++) {
            /* the mask at this pixel's centre, in GBA pixels */
            float fx = (x + 0.5f) * d->w / w - 0.5f, fy = (y + 0.5f) * d->h / h - 0.5f;
            int x0 = (int)SDL_floorf(fx), y0 = (int)SDL_floorf(fy), x1 = x0 + 1, y1 = y0 + 1;
            float u = fx - x0, v = fy - y0, m;
#define M(xx, yy) ((xx) < 0 || (yy) < 0 || (xx) >= d->w || (yy) >= d->h ? 0.0f : mask[(yy) * d->w + (xx)] / 255.0f)
            m = (M(x0, y0) * (1 - u) + M(x1, y0) * u) * (1 - v) + (M(x0, y1) * (1 - u) + M(x1, y1) * u) * v;
#undef M
            px[(y * w + x) * 4 + 3] = (unsigned char)(px[(y * w + x) * 4 + 3] * m + 0.5f);
        }
    SDL_free(mask);
}

static int Load(int i) {
    char file[160];
    int w, h, n, k;
    unsigned char* px;
    SDL_Surface* s;
    size_t need;
    px = stbi_load(SpritePath(file, sizeof(file), kHdSprites[i].name), &w, &h, &n, 4);
    if (!px || w < 1 || h < 1 || w > 4096 || h > 4096) {
        if (px) stbi_image_free(px);
        sState[i].present = 1;
        PortLog("HD sprites: %s unreadable, the cartridge's sprite stays", file);
        return 0;
    }
    if (kHdSprites[i].group == 4)
        MaskBySprite(px, w, h, &kHdSprites[i]);
    need = (size_t)w * h * 8;
    /* the cards on screen stay; the oldest others make room (32 MB; 12 MB on Switch and PS Vita, whose memory is
       shared with the game's image and the host's own copy of each texture) */
#ifdef PORT_GBA_RELOC
    const size_t budget = 12u * 1024 * 1024;
#else
    const size_t budget = 32u * 1024 * 1024;
#endif
    while (sBytes + need > budget) {
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

void PortHdSpritesBegin(int layer) {
    if ((unsigned)layer < HD_LAYERS)
        sDrawCount[layer] = 0;
}

int PortHdSpriteQueue(int layer, int id, const float* x, const float* y, int px, int py, int pw, int ph, int alpha,
                      int dark, int white) {
    const HdSpriteDef* d;
    HdSpriteDraw* q;
    int k;
    if ((unsigned)layer >= HD_LAYERS)
        return 0;
    if (id < 1 || id > HD_SPRITE_COUNT || sDrawCount[layer] == MAX_DRAWS)
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
    q = &sDraws[layer][sDrawCount[layer]++];
    q->id = id - 1;
    memcpy(q->x, x, sizeof(q->x));
    memcpy(q->y, y, sizeof(q->y));
    q->u0 = (float)(px - d->x0) / d->w;
    q->v0 = (float)(py - d->y0) / d->h;
    q->u1 = (float)(px + pw - d->x0) / d->w;
    q->v1 = (float)(py + ph - d->y0) / d->h;
    q->alpha = (uint8_t)alpha;
    q->dark = (uint8_t)SDL_clamp(dark, 0, 255);
    q->white = (uint8_t)SDL_clamp(white, 0, 255);
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
    /* An axis-aligned quad is cut to the window first (texture coordinates in proportion): SDL's software
       renderer drops a triangle that reaches far past the canvas (the title's logo in its intro zoom was not
       drawn at all), and it saves the work of what cannot be seen. */
    if (v[0].position.y == v[1].position.y && v[0].position.x == v[3].position.x) {
        int ow = 0, oh = 0;
        if (SDL_GetCurrentRenderOutputSize(sRenderer, &ow, &oh) && ow > 0 && oh > 0) {
            float x0 = v[0].position.x, x1 = v[1].position.x, y0 = v[0].position.y, y1 = v[3].position.y;
            float u0 = q->u0, u1 = q->u1, w0 = q->v0, w1 = q->v1;
            float cx0 = SDL_clamp(x0, 0, (float)ow), cx1 = SDL_clamp(x1, 0, (float)ow);
            float cy0 = SDL_clamp(y0, 0, (float)oh), cy1 = SDL_clamp(y1, 0, (float)oh);
            if (cx0 == cx1 || cy0 == cy1)
                return; /* entirely outside */
            v[0].position.x = v[3].position.x = cx0;
            v[1].position.x = v[2].position.x = cx1;
            v[0].position.y = v[1].position.y = cy0;
            v[2].position.y = v[3].position.y = cy1;
            v[0].tex_coord.x = v[3].tex_coord.x = u0 + (u1 - u0) * (cx0 - x0) / (x1 - x0);
            v[1].tex_coord.x = v[2].tex_coord.x = u0 + (u1 - u0) * (cx1 - x0) / (x1 - x0);
            v[0].tex_coord.y = v[1].tex_coord.y = w0 + (w1 - w0) * (cy0 - y0) / (y1 - y0);
            v[2].tex_coord.y = v[3].tex_coord.y = w0 + (w1 - w0) * (cy1 - y0) / (y1 - y0);
        }
    }
    /* SDL turns an axis-aligned quad into a texture copy, and the software
       renderer's scaled copy reads outside the texture when the quad is
       mirrored, turned a quarter (a card spinning) or partly off the canvas (a
       face at the HUD's edge): it crashed. A hundredth of a pixel keeps every
       such quad on the triangle path, which clips and maps it correctly. */
    if (v[0].position.x == v[1].position.x || v[0].position.y == v[1].position.y) {
        v[1].position.x += 0.01f;
        v[1].position.y += 0.01f;
    }
    if (SDL_getenv("KHCOM_HDSPRITES_LOG"))
        PortLog("hdsprite f=%u %s (%.1f,%.1f) (%.1f,%.1f) (%.1f,%.1f) (%.1f,%.1f) uv %.3f,%.3f-%.3f,%.3f", PortFrameCount(),
                kHdSprites[q->id].name, v[0].position.x, v[0].position.y, v[1].position.x, v[1].position.y,
                v[2].position.x, v[2].position.y, v[3].position.x, v[3].position.y, q->u0, q->v0, q->u1, q->v1);
    {
        /* A large quad goes as a grid of smaller ones: SDL's software renderer draws nothing of a triangle
           this big (the title's logo, 1700 pixels wide in the intro). Corners and texture coordinates are
           interpolated, exact for any affine transform. */
        float span = SDL_max(SDL_fabsf(v[2].position.x - v[0].position.x), SDL_fabsf(v[2].position.y - v[0].position.y));
        int n = SDL_clamp((int)(span / 256.0f) + 1, 1, 8), i, j;
        SDL_Vertex grid[81];
        int index[6 * 64], ni = 0;
        if (n == 1) {
            SDL_RenderGeometry(sRenderer, t, v, 4, kIndex, 6);
            return;
        }
        for (j = 0; j <= n; j++)
            for (i = 0; i <= n; i++) {
                float a = (float)i / n, b = (float)j / n;
                SDL_Vertex* g = &grid[j * (n + 1) + i];
#define LERP2(f) ((1 - b) * ((1 - a) * v[0].f + a * v[1].f) + b * ((1 - a) * v[3].f + a * v[2].f))
                g->position.x = LERP2(position.x);
                g->position.y = LERP2(position.y);
                g->tex_coord.x = LERP2(tex_coord.x);
                g->tex_coord.y = LERP2(tex_coord.y);
#undef LERP2
                g->color = c;
            }
        for (j = 0; j < n; j++)
            for (i = 0; i < n; i++) {
                int p = j * (n + 1) + i;
                index[ni++] = p; index[ni++] = p + 1; index[ni++] = p + n + 2;
                index[ni++] = p; index[ni++] = p + n + 2; index[ni++] = p + n + 1;
            }
        SDL_RenderGeometry(sRenderer, t, grid, (n + 1) * (n + 1), index, ni);
    }
}

void PortHdSpritesDraw(SDL_Renderer* renderer, int layer, float ox, float oy, float scale, int darken, int brighten) {
    int i;
    sRenderer = renderer;
    if ((unsigned)layer >= HD_LAYERS)
        return;
    for (i = 0; i < sDrawCount[layer]; i++) {
        const HdSpriteDraw* q = &sDraws[layer][i];
        HdSpriteState* st = &sState[q->id];
        /* the game's brightness fade (BLDY) and the part's palette fade, whichever is further */
        float shade = 1.0f - SDL_max(darken > 0 ? darken / 16.0f : 0.0f, q->dark / 255.0f);
        float white = SDL_max(brighten > 0 ? brighten / 16.0f : 0.0f, q->white / 255.0f);
        if (!st->image && !Load(q->id))
            continue;
        st->lastUsed = PortFrameCount();
        Quad(st->image, q, ox, oy, scale, (SDL_FColor){ shade, shade, shade, q->alpha / 255.0f });
        if (white > 0)
            Quad(st->white, q, ox, oy, scale, (SDL_FColor){ 1, 1, 1, q->alpha / 255.0f * white });
    }
}

void PortPaletteFade(const uint16_t* original, const volatile uint16_t* current, int count, int* dark, int* white) {
    int i, so = 0, sc = 0, room = 0, err = 0, k;
    *dark = *white = 0;
    if (!original || count < 2)
        return;
    for (i = 1; i < count; i++) /* entry 0 is transparent */
        for (k = 0; k < 15; k += 5) {
            int o = (original[i] >> k) & 31, c = (current[i] >> k) & 31;
            so += o;
            sc += c;
            room += 31 - o;
        }
    if (sc < so && so > 0) {
        int f = 256 * (so - sc) / so; /* c = o (1 - f) */
        for (i = 1; i < count; i++)
            for (k = 0; k < 15; k += 5) {
                int o = (original[i] >> k) & 31, c = (current[i] >> k) & 31;
                err += SDL_abs(c - o * (256 - f) / 256);
            }
        if (err <= 2 * 3 * (count - 1) && f > 6)
            *dark = SDL_min(f, 255);
    } else if (sc > so && room > 0) {
        int f = 256 * (sc - so) / room; /* c = o + (31 - o) f */
        for (i = 1; i < count; i++)
            for (k = 0; k < 15; k += 5) {
                int o = (original[i] >> k) & 31, c = (current[i] >> k) & 31;
                err += SDL_abs(c - (o + (31 - o) * f / 256));
            }
        if (err <= 2 * 3 * (count - 1) && f > 6)
            *white = SDL_min(f, 255);
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
