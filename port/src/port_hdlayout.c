/*
 * Re:CoM 1.5's 2D layouts (LAD) and sequences (SEQD), drawn the way its engine does, for the HD battle HUD
 * (HUD_HD_RECOM.md): the player's own gauge.lad / Cockpit.lad (imported as hd/sprites/gHud*Lad.lad, with their
 * whole textures as PNGs) and the logic read from Re:CoM's executable. The formats and the drawing are OpenKh's
 * (OpenKh.Kh2/Layout.cs, Sequence.cs, OpenKh.Engine/Renders/SequenceRenderer.cs: KH2 shares the engine): an
 * animation group plays its animations at a frame; each animation moves, pivots, scales, turns and tints one sprite
 * group; each sprite of a group is a rectangle of the texture with a colour per corner. Our game keeps its own rules;
 * only the pictures follow Re:CoM.
 */
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

#include "port/port.h"
#include "port/port_hdlayout.h"
#include "../decompui/third_party/stb_image.h"

const char* PortHdSpritePath(char* file, size_t size, const char* name);

typedef struct HdLayout {
    unsigned char* data;
    int size;
} HdLayout;

typedef struct HdLayoutTexture {
    SDL_Texture* tex;
    int w, h, tried;
} HdLayoutTexture;

static HdLayout sLayouts[HD_LAYOUT_COUNT];
static int sLayoutTried[HD_LAYOUT_COUNT];
static HdLayoutTexture sTextures[HD_LAYOUT_TEXTURE_COUNT];
static const char* const kLayoutFiles[HD_LAYOUT_COUNT] = { "gHudGaugeLad", "gHudCockpitLad" };
static const char* const kTextureFiles[HD_LAYOUT_TEXTURE_COUNT] = { "gHudGaugeTex0", "gHudGaugeTex1",
                                                                     "gHudRikuGaugeTex0", "gHudCockpitTex" };

static int32_t I32(const unsigned char* p) { return (int32_t)(p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24); }
static int16_t I16(const unsigned char* p) { return (int16_t)(p[0] | p[1] << 8); }
static float F32(const unsigned char* p) { float f; uint32_t v = (uint32_t)I32(p); memcpy(&f, &v, 4); return f; }

static const HdLayout* Layout(int which) {
    if ((unsigned)which >= HD_LAYOUT_COUNT)
        return NULL;
    if (!sLayouts[which].data && !sLayoutTried[which]) {
        char file[160], name[64];
        size_t size = 0;
        void* data;
        sLayoutTried[which] = 1;
        SDL_snprintf(name, sizeof(name), "%s", kLayoutFiles[which]);
        PortHdSpritePath(file, sizeof(file), name);
        /* the path helper names .png: the layout is the same name with .lad */
        if (strlen(file) > 4 && SDL_strcmp(file + strlen(file) - 4, ".png") == 0)
            memcpy(file + strlen(file) - 4, ".lad", 4);
        data = SDL_LoadFile(file, &size);
        if (data && size >= 32 && I32((const unsigned char*)data) == 0x4459414C) {
            sLayouts[which].data = data;
            sLayouts[which].size = (int)size;
        } else if (data) {
            SDL_free(data);
        }
    }
    return sLayouts[which].data ? &sLayouts[which] : NULL;
}

static HdLayoutTexture* Texture(SDL_Renderer* renderer, int which) {
    HdLayoutTexture* t;
    if ((unsigned)which >= HD_LAYOUT_TEXTURE_COUNT)
        return NULL;
    t = &sTextures[which];
    if (!t->tex && !t->tried && renderer) {
        char file[160];
        int n;
        unsigned char* px;
        t->tried = 1;
        px = stbi_load(PortHdSpritePath(file, sizeof(file), kTextureFiles[which]), &t->w, &t->h, &n, 4);
        if (px) {
            SDL_Surface* s = SDL_CreateSurfaceFrom(t->w, t->h, SDL_PIXELFORMAT_RGBA32, px, t->w * 4);
            if (s) {
                t->tex = SDL_CreateTextureFromSurface(renderer, s);
                SDL_DestroySurface(s);
            }
            stbi_image_free(px);
            if (t->tex)
                SDL_SetTextureBlendMode(t->tex, SDL_BLENDMODE_BLEND);
        }
    }
    return t->tex ? t : NULL;
}

int PortHdLayoutReady(int layout, int texture) {
    char file[160];
    FILE* f;
    if (!Layout(layout))
        return 0;
    if (sTextures[texture].tex)
        return 1;
    if (sTextures[texture].tried)
        return 0;
    f = fopen(PortHdSpritePath(file, sizeof(file), kTextureFiles[texture]), "rb");
    if (f)
        fclose(f);
    return f != NULL;
}

/* The layout's sequence n: its bytes and size. */
static const unsigned char* Sequence(const HdLayout* l, int n, int* size) {
    const unsigned char* b = l->data;
    int count = I32(b + 24), table = I32(b + 28), start, end;
    if (n < 0 || n >= count || table + 4 * count > l->size)
        return NULL;
    start = I32(b + table + 4 * n);
    end = n + 1 < count ? I32(b + table + 4 * (n + 1)) : l->size;
    if (start < 0 || end > l->size || end - start < 48 || I32(b + start) != 0x44514553)
        return NULL;
    *size = end - start;
    return b + start;
}

int PortHdLayoutGroupFrames(int layout, int sequence, int group) {
    const HdLayout* l = Layout(layout);
    const unsigned char *s, *ag;
    int size, i, end = 0, count;
    if (!l || !(s = Sequence(l, sequence, &size)) || group < 0 || group >= I32(s + 40))
        return 0;
    ag = s + I32(s + 44) + group * 36;
    for (i = 0, count = I16(ag + 2); i < count; i++) {
        const unsigned char* a = s + I32(s + 36) + (I16(ag) + i) * 144;
        if (I32(a + 12) > end) end = I32(a + 12);
    }
    return end;
}

typedef struct Ctx {
    float x, y, px, py, sx, sy, rx, ry, rz;
    float c[4];
} Ctx;

static float Lerp(float t, float a, float b) { return a * (1.0f - t) + b * t; }

static void Color(uint32_t c, float* out) {
    out[0] = (c & 255) / 128.0f;
    out[1] = ((c >> 8) & 255) / 128.0f;
    out[2] = ((c >> 16) & 255) / 128.0f;
    out[3] = ((c >> 24) & 255) / 128.0f;
}

static void DrawSprite(SDL_Renderer* r, const HdLayoutTexture* t, const unsigned char* s, const Ctx* c,
                       const unsigned char* part, float uvScale, float ox, float oy, float scale, int blend) {
    static const int kIndex[6] = { 0, 1, 2, 1, 3, 2 };
    const unsigned char* sp = s + I32(s + 12) + I32(part + 16) * 44; /* sprites: count at 8, offset at 12 */
    float L = (float)I32(part), T = (float)I32(part + 4), R = (float)I32(part + 8), B = (float)I32(part + 12);
    float cx[4] = { L, R, L, R }, cy[4] = { T, T, B, B };
    float u0 = I32(sp + 4) * uvScale / t->w, v0 = I32(sp + 8) * uvScale / t->h;
    float u1 = I32(sp + 12) * uvScale / t->w, v1 = I32(sp + 16) * uvScale / t->h;
    float tu[4] = { u0, u1, u0, u1 }, tv[4] = { v0, v0, v1, v1 };
    float cr = SDL_cosf(-c->rz), sr = SDL_sinf(-c->rz), crx = SDL_cosf(-c->rx), cry = SDL_cosf(-c->ry);
    SDL_Vertex v[4];
    int k;
    if (I32(sp + 12) == I32(sp + 4) || I32(sp + 16) == I32(sp + 8))
        return;
    for (k = 0; k < 4; k++) {
        float col[4], x = cx[k] + c->px, y = cy[k] + c->py, nx;
        x *= c->sx;
        y *= c->sy;
        y *= crx; /* a turn about X or Y, seen flat: the picture narrows */
        x *= cry;
        nx = x * cr - y * sr;
        y = x * sr + y * cr;
        x = nx + c->x;
        y += c->y;
        Color((uint32_t)I32(sp + 28 + 4 * k), col);
        v[k].position.x = ox + x * scale;
        v[k].position.y = oy + y * scale;
        v[k].color.r = SDL_min(1.0f, col[0] * c->c[0]);
        v[k].color.g = SDL_min(1.0f, col[1] * c->c[1]);
        v[k].color.b = SDL_min(1.0f, col[2] * c->c[2]);
        v[k].color.a = SDL_min(1.0f, col[3] * c->c[3]);
        v[k].tex_coord.x = tu[k];
        v[k].tex_coord.y = tv[k];
    }
    /* SDL's software renderer turns an axis-aligned quad into a copy that reads past the texture (port_hdsprites.c,
       Quad): a hundredth of a pixel keeps it on the triangle path */
    v[1].position.x += 0.01f;
    v[1].position.y += 0.01f;
    SDL_SetTextureBlendMode(t->tex, blend == 1 ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);
    SDL_RenderGeometry(r, t->tex, v, 4, kIndex, 6);
}

int PortHdLayoutDraw(SDL_Renderer* renderer, int layout, int texture, int sequence, int group, int frame, float x,
                     float y, float scale, float alpha, const float* tint) {
    const HdLayout* l = Layout(layout);
    HdLayoutTexture* t = Texture(renderer, texture);
    const unsigned char *s, *ag;
    int size, i, count, loopStart, loopEnd;
    if (!l || !t || !(s = Sequence(l, sequence, &size)) || group < 0 || group >= I32(s + 40))
        return 0;
    ag = s + I32(s + 44) + group * 36;
    count = I16(ag + 2);
    loopStart = I32(ag + 8);
    loopEnd = I32(ag + 12);
    if (I16(ag + 4) == 0) { /* looping: from LoopStart to LoopEnd (or the last frame) */
        if (loopEnd == 0)
            loopEnd = PortHdLayoutGroupFrames(layout, sequence, group);
        if (loopEnd > loopStart && frame >= loopEnd)
            frame = loopStart + (frame - loopStart) % (loopEnd - loopStart);
    }
    for (i = 0; i < count; i++) {
        const unsigned char* a = s + I32(s + 36) + (I16(ag) + i) * 144;
        int flags = I32(a), f0 = I32(a + 8), f1 = I32(a + 12), sg = I32(a + 4), p;
        const unsigned char* g;
        float delta, tt, c0[4], c1[4];
        Ctx c;
        if (frame < f0 || frame > f1)
            continue;
        delta = f1 > f0 ? (float)(frame - f0) / (f1 - f0) : 0.0f;
        tt = (flags & 1) ? delta : (SDL_sinf(delta * SDL_PI_F - SDL_PI_F / 2) + 1.0f) / 2.0f;
        memset(&c, 0, sizeof(c));
        c.x = (flags & 0x1000) ? (float)I32(a + 16) : Lerp(tt, (float)I32(a + 16), (float)I32(a + 20));
        c.y = (flags & 0x1000) ? (float)I32(a + 24) : Lerp(tt, (float)I32(a + 24), (float)I32(a + 28));
        if (!(flags & 0x40)) {
            float sc = Lerp(tt, F32(a + 72), F32(a + 76));
            c.sx = sc * Lerp(tt, F32(a + 80), F32(a + 84));
            c.sy = sc * Lerp(tt, F32(a + 88), F32(a + 92));
        } else {
            c.sx = c.sy = 1.0f;
        }
        c.c[0] = c.c[1] = c.c[2] = 1.0f;
        c.c[3] = alpha;
        if (tint)
            for (p = 0; p < 4; p++) c.c[p] *= tint[p];
        if (!(flags & 0x400)) {
            Color((uint32_t)I32(a + 136), c0);
            Color((uint32_t)I32(a + 140), c1);
            for (p = 0; p < 4; p++)
                c.c[p] *= (flags & 0x80) ? c0[p] : Lerp(tt, c0[p], c1[p]);
        }
        if (!(flags & 0x20)) {
            c.rx = Lerp(tt, F32(a + 48), F32(a + 52));
            c.ry = Lerp(tt, F32(a + 56), F32(a + 60));
            c.rz = Lerp(tt, F32(a + 64), F32(a + 68));
        }
        if (!(flags & 0x4000)) {
            c.px = Lerp(tt, (float)I32(a + 32), (float)I32(a + 36));
            c.py = Lerp(tt, (float)I32(a + 40), (float)I32(a + 44));
        }
        if (!(flags & 0x10)) {
            float bx = SDL_sinf(delta * I16(a + 128) * SDL_PI_F), by = SDL_sinf(delta * I16(a + 130) * SDL_PI_F);
            c.x += bx * Lerp(tt, F32(a + 112), F32(a + 116));
            c.y += by * Lerp(tt, F32(a + 120), F32(a + 124));
        }
        if (sg < 0 || sg >= I32(s + 24))
            continue;
        g = s + I32(s + 28) + sg * 4;
        for (p = 0; p < I16(g + 2); p++)
            DrawSprite(renderer, t, s, &c, s + I32(s + 20) + (I16(g) + p) * 20, 2.0f, x, y, scale, I32(a + 132));
    }
    SDL_SetTextureBlendMode(t->tex, SDL_BLENDMODE_BLEND);
    return 1;
}

void PortHdLayoutShutdown(void) {
    int i;
    for (i = 0; i < HD_LAYOUT_TEXTURE_COUNT; i++) {
        if (sTextures[i].tex)
            SDL_DestroyTexture(sTextures[i].tex);
        memset(&sTextures[i], 0, sizeof(sTextures[i]));
    }
}

/* ---- the player's HP gauge (Gauge::CPlayerGauge, read from Re:CoM's executable, HUD_HD_RECOM.md) ---------------
   gauge.lad's group "gauge:01" (layout group 1, at 60,60) has five places: 0 the gauge's state (14 appearing, 15
   normal, 10 low HP, 11..13 a hit), 1 the track (group 7), 2 the red trail (8), 3 the green HP (9), and "gauge:00"'s
   face (sequence 0: 3 normal, 1 hurt, 0 low, 2 down). The lengths are frames: value / 560 x 470. A hit drops the
   green at once; the trail waits 1.5 s, then closes a tenth of the gap and one more each step; healing moves both.
   Low HP is 25 % of the maximum. Only the picture is Re:CoM's: the HP and its rules are the game's own. */
#define GAUGE_LEN(v) ((int)((float)SDL_clamp((v), 0, 560) / 560.0f * 470.0f))
static struct {
    int active, frame, hp, maxHp, trail, delayMs, hurtMs, riku, started;
    int stateAnim, stateFrame, faceAnim, faceFrame;
} sGauge;

static void GaugeSet(int* anim, int* frame, int value) {
    if (*anim != value) {
        *anim = value;
        *frame = 0;
    }
}

int PortHdGauge(int hp, int maxHp, int riku) {
    int msPerFrame = 17;
    if (!gPortConfig.hdCards || !PortHdLayoutReady(HD_LAYOUT_GAUGE, HD_LAYOUT_TEX_GAUGE) ||
        !PortHdLayoutReady(HD_LAYOUT_GAUGE, riku ? HD_LAYOUT_TEX_RIKU_FACES : HD_LAYOUT_TEX_GAUGE_FACES))
        return 0;
    hp = SDL_clamp(hp, 0, 560);
    maxHp = SDL_clamp(maxHp, 1, 560);
    if (!sGauge.started || sGauge.frame + 1 != (int)PortFrameCount() || sGauge.riku != riku) {
        /* a new battle (or the gauge was away): it appears, full as the game has it */
        memset(&sGauge, 0, sizeof(sGauge));
        sGauge.started = 1;
        sGauge.riku = riku;
        sGauge.hp = sGauge.trail = hp;
        sGauge.stateAnim = 14;
        sGauge.faceAnim = 3;
    }
    sGauge.frame = (int)PortFrameCount();
    sGauge.maxHp = maxHp;
    if (hp != sGauge.hp) {
        if (hp < sGauge.hp) { /* a hit: the trail waits, the gauge and the face react */
            sGauge.delayMs = 1500;
            sGauge.hurtMs = 3000;
            GaugeSet(&sGauge.stateAnim, &sGauge.stateFrame, 13);
            GaugeSet(&sGauge.faceAnim, &sGauge.faceFrame, 1);
        }
        sGauge.hp = hp;
        if (sGauge.trail < hp) {
            sGauge.trail = hp;
            sGauge.delayMs = 0;
        }
    }
    if (sGauge.trail > sGauge.hp) {
        if (sGauge.delayMs <= 0) {
            sGauge.trail -= (sGauge.trail - sGauge.hp) / 10 + 1;
            if (sGauge.trail < sGauge.hp)
                sGauge.trail = sGauge.hp;
        }
        sGauge.delayMs -= msPerFrame;
    }
    if (sGauge.hurtMs > 0) {
        sGauge.hurtMs -= msPerFrame;
    } else if (sGauge.stateAnim != 14 || sGauge.stateFrame >= PortHdLayoutGroupFrames(HD_LAYOUT_GAUGE, 1, 14)) {
        if (hp == 0) {
            GaugeSet(&sGauge.stateAnim, &sGauge.stateFrame, 15);
            GaugeSet(&sGauge.faceAnim, &sGauge.faceFrame, 2);
        } else if (hp <= maxHp * 25 / 100) {
            GaugeSet(&sGauge.stateAnim, &sGauge.stateFrame, 10);
            GaugeSet(&sGauge.faceAnim, &sGauge.faceFrame, 0);
        } else {
            GaugeSet(&sGauge.stateAnim, &sGauge.stateFrame, 15);
            GaugeSet(&sGauge.faceAnim, &sGauge.faceFrame, 3);
        }
    }
    sGauge.stateFrame++;
    sGauge.faceFrame++;
    sGauge.active = 1;
    return 1;
}

void PortHdGaugeDraw(SDL_Renderer* renderer, float ox, float oy, float scale, float shade) {
    /* gauge:01 sits at (60, 60) of Re:CoM's 640-wide screen: the GBA HUD's 240 gives 0.375 of a GBA pixel a unit */
    const float unit = 240.0f / 640.0f;
    float x = ox + 60.0f * unit * scale, y = oy + 60.0f * unit * scale, s = unit * scale;
    float tint[4] = { shade, shade, shade, 1.0f };
    if (!sGauge.active || sGauge.frame != (int)PortFrameCount())
        return;
    PortHdLayoutDraw(renderer, HD_LAYOUT_GAUGE, HD_LAYOUT_TEX_GAUGE, 1, sGauge.stateAnim, sGauge.stateFrame, x, y, s, 1.0f, tint);
    PortHdLayoutDraw(renderer, HD_LAYOUT_GAUGE, HD_LAYOUT_TEX_GAUGE, 1, 7, GAUGE_LEN(sGauge.maxHp), x, y, s, 1.0f, tint);
    PortHdLayoutDraw(renderer, HD_LAYOUT_GAUGE, HD_LAYOUT_TEX_GAUGE, 1, 8, GAUGE_LEN(sGauge.trail), x, y, s, 1.0f, tint);
    PortHdLayoutDraw(renderer, HD_LAYOUT_GAUGE, HD_LAYOUT_TEX_GAUGE, 1, 9, GAUGE_LEN(sGauge.hp), x, y, s, 1.0f, tint);
    PortHdLayoutDraw(renderer, HD_LAYOUT_GAUGE, sGauge.riku ? HD_LAYOUT_TEX_RIKU_FACES : HD_LAYOUT_TEX_GAUGE_FACES, 0,
                     sGauge.faceAnim, sGauge.faceFrame, x, y, s, 1.0f, tint);
}
