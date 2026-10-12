/*
 * HD text: the dialogue glyphs the game draws as 16x16 sprites from its pixel
 * font are left out of the GBA picture (port_ppu.c) and drawn here with an
 * outline font at the window's resolution, at the same places and in the
 * same colours (palette fades included).
 *
 * Each word starts where the game puts it; when the outline font's word is
 * wider than the pixel font's, its letters are set closer to fit, so lines
 * keep the game's layout. Glyphs are rasterised with GDI and cached as
 * textures, drawn unscaled at whole pixels.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <SDL3/SDL.h>

#include "port/port.h"

/* The pixel font's capitals are 7 pixels tall, from row 4 to row 10 of the
   16x16 cell: the baseline is row 11. */
#define HD_TEXT_EM 10.5f
#define HD_TEXT_BASELINE 11
#define HD_TEXT_MAX_GLYPHS 256

const uint16_t* PortLatinGlyphWidths(void);

typedef struct HdGlyph {
    SDL_Texture* tex;
    int ready;
    int w, h;
    int left, top; /* bitmap position from the pen on the baseline */
    int advance;
} HdGlyph;

typedef struct HdFont {
    int px;
    HFONT font;
    HdGlyph glyphs[256];
} HdFont;

static HDC sDc;
static HdFont sFonts[2];
static int sNextFont;

static void FontRelease(HdFont* f) {
    int i;

    for (i = 0; i < 256; i++) {
        if (f->glyphs[i].tex != NULL)
            SDL_DestroyTexture(f->glyphs[i].tex);
    }
    if (f->font != NULL)
        DeleteObject(f->font);
    SDL_memset(f, 0, sizeof(*f));
}

static HdFont* FontForSize(int px) {
    HdFont* f;
    int i;

    for (i = 0; i < 2; i++) {
        if (sFonts[i].font != NULL && sFonts[i].px == px)
            return &sFonts[i];
    }
    if (sDc == NULL)
        sDc = CreateCompatibleDC(NULL);
    f = &sFonts[sNextFont];
    sNextFont ^= 1;
    FontRelease(f);
    f->px = px;
    f->font = CreateFontW(-px, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS,
                          CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH | FF_SWISS, L"Segoe UI");
    return f;
}

static HdGlyph* Glyph(SDL_Renderer* renderer, HdFont* f, int code) {
    static const MAT2 identity = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };
    HdGlyph* g = &f->glyphs[code & 255];
    GLYPHMETRICS gm;
    char c = (char)code;
    WCHAR wc;
    DWORD size;

    if (g->ready)
        return g;
    g->ready = 1;
    if (MultiByteToWideChar(1252, 0, &c, 1, &wc, 1) != 1)
        return g;
    SelectObject(sDc, f->font);
    size = GetGlyphOutlineW(sDc, wc, GGO_GRAY8_BITMAP, &gm, 0, NULL, &identity);
    if (size == GDI_ERROR)
        return g;
    g->advance = gm.gmCellIncX;
    g->left = gm.gmptGlyphOrigin.x;
    g->top = gm.gmptGlyphOrigin.y;
    if (size > 0) {
        uint8_t* bits = SDL_malloc(size);
        int pitch = ((int)gm.gmBlackBoxX + 3) & ~3;
        int w = (int)gm.gmBlackBoxX, h = (int)gm.gmBlackBoxY, x, y;
        uint32_t* argb = SDL_malloc((size_t)w * h * 4);

        if (bits != NULL && argb != NULL
            && GetGlyphOutlineW(sDc, wc, GGO_GRAY8_BITMAP, &gm, size, bits, &identity) != GDI_ERROR) {
            for (y = 0; y < h; y++) {
                for (x = 0; x < w; x++) {
                    uint32_t a = bits[y * pitch + x] * 255u / 64u;
                    argb[y * w + x] = (a << 24) | 0xFFFFFFu;
                }
            }
            g->tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
            if (g->tex != NULL) {
                SDL_UpdateTexture(g->tex, NULL, argb, w * 4);
                SDL_SetTextureBlendMode(g->tex, SDL_BLENDMODE_BLEND);
                SDL_SetTextureScaleMode(g->tex, SDL_SCALEMODE_NEAREST);
                g->w = w;
                g->h = h;
            }
        }
        SDL_free(bits);
        SDL_free(argb);
    }
    return g;
}

static int CompareGlyphs(const void* a, const void* b) {
    const PortTextGlyph* p = a;
    const PortTextGlyph* q = b;

    if (p->y != q->y)
        return p->y - q->y;
    return p->x - q->x;
}

static void DrawGlyph(SDL_Renderer* r, HdGlyph* g, float x, float y, uint32_t color, int alpha) {
    SDL_FRect d;

    if (g->tex == NULL)
        return;
    d.x = SDL_floorf(x + 0.5f) + g->left;
    d.y = SDL_floorf(y + 0.5f) - g->top;
    d.w = (float)g->w;
    d.h = (float)g->h;
    SDL_SetTextureColorMod(g->tex, (color >> 16) & 255, (color >> 8) & 255, color & 255);
    SDL_SetTextureAlphaMod(g->tex, (Uint8)alpha);
    SDL_RenderTexture(r, g->tex, NULL, &d);
}

void PortHdTextDraw(SDL_Renderer* renderer, int hudLayer, float originX, float originY, float scale) {
    static PortTextGlyph glyphs[HD_TEXT_MAX_GLYPHS];
    const uint16_t* widths = PortLatinGlyphWidths();
    HdFont* font;
    float shadow = SDL_max(1.0f, SDL_floorf(scale * 0.5f));
    int n, i, j, pass;

    if (!gPortConfig.hdText || scale <= 0.0f)
        return;
    n = PortPpuTextGlyphs(glyphs, HD_TEXT_MAX_GLYPHS, hudLayer);
    if (n == 0)
        return;
    font = FontForSize((int)(HD_TEXT_EM * scale + 0.5f));
    if (font->font == NULL)
        return;
    SDL_qsort(glyphs, n, sizeof(glyphs[0]), CompareGlyphs);

    /* Words: glyphs on one line, each where the previous one ends. */
    for (i = 0; i < n; i = j) {
        int engineW = widths[glyphs[i].code & 255], fontW = 0;
        float k = 1.0f, pen, baseline;

        fontW = Glyph(renderer, font, glyphs[i].code)->advance;
        for (j = i + 1; j < n; j++) {
            const PortTextGlyph* p = &glyphs[j - 1];
            if (glyphs[j].y != p->y || glyphs[j].x != p->x + widths[p->code & 255])
                break;
            engineW += widths[glyphs[j].code & 255];
            fontW += Glyph(renderer, font, glyphs[j].code)->advance;
        }
        if (fontW > engineW * scale)
            k = engineW * scale / fontW;
        baseline = originY + (glyphs[i].y + HD_TEXT_BASELINE) * scale;
        for (pass = 0; pass < 2; pass++) {
            int t;
            pen = originX + glyphs[i].x * scale;
            for (t = i; t < j; t++) {
                HdGlyph* g = Glyph(renderer, font, glyphs[t].code);
                int alpha = glyphs[t].semi ? 0xA0 : 0xFF;
                if (pass == 0)
                    DrawGlyph(renderer, g, pen + shadow, baseline + shadow, glyphs[t].shadow, alpha);
                else
                    DrawGlyph(renderer, g, pen, baseline, glyphs[t].face, alpha);
                pen += g->advance * k;
            }
        }
    }
}
