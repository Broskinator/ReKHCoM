/*
 * HD text: the dialogue glyphs the game draws as 16x16 sprites from its pixel
 * font are left out of the GBA picture (port_ppu.c) and drawn here with an
 * outline font at the window's resolution, at the same places and in the
 * same colours (palette fades included).
 *
 * Each word starts where the game puts it, or after the previous word when
 * the outline font set that one wider; letters keep their natural spacing.
 * Glyphs are rasterised with GDI and cached as textures, drawn unscaled at
 * whole pixels.
 *
 * The font is assets/fonts/ComicHearts.otf next to the executable (Segoe UI
 * when it is missing). Letters it lacks are built from it where possible:
 * accented letters are the base letter with the accent of Comic Sans MS
 * (close to its style), and the Spanish opening marks are ? and ! turned
 * over; anything else comes from Comic Sans MS.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <SDL3/SDL.h>

#include "port/port.h"

/* The pixel font's capitals are 7 pixels tall, from row 4 to row 10 of the
   16x16 cell: the baseline is row 11. */
#define HD_TEXT_CAP 7.0f
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
    int key;
    float cap;      /* capital height in output pixels */
    HFONT font;     /* the HD font */
    HFONT fallback; /* for what the HD font lacks: Comic Sans MS, same capital height */
    HdGlyph glyphs[256];
} HdFont;

/* A glyph bitmap in memory: 8-bit coverage, origin as GDI reports it. */
typedef struct Bitmap {
    uint8_t* a;
    int w, h, left, top, advance;
} Bitmap;

static HDC sDc;
static HdFont sFonts[2];
static int sNextFont;
static int sCustomTried;
static const WCHAR* sFace = L"Segoe UI";

static void LoadCustomFont(void) {
    char path[1024];
    WCHAR wpath[1024];
    const char* base = SDL_GetBasePath();

    if (sCustomTried)
        return;
    sCustomTried = 1;
    SDL_snprintf(path, sizeof(path), "%sassets/fonts/ComicHearts.otf", base ? base : "");
    if (MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, 1024) > 0 && AddFontResourceExW(wpath, FR_PRIVATE, NULL) > 0) {
        sFace = L"Comic Hearts";
        PortLog("HD text: font %s", path);
    }
}

static HFONT MakeFont(const WCHAR* face, int px) {
    return CreateFontW(-px, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS,
                       CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH | FF_SWISS, face);
}

/* Font of the face whose capital H is `cap` pixels tall. */
static HFONT FontWithCap(const WCHAR* face, float cap) {
    static const MAT2 identity = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };
    GLYPHMETRICS gm;
    int px = (int)(cap * 1.5f + 0.5f);
    HFONT f = MakeFont(face, px);

    SelectObject(sDc, f);
    if (GetGlyphOutlineW(sDc, L'H', GGO_METRICS, &gm, 0, NULL, &identity) != GDI_ERROR && gm.gmBlackBoxY > 0) {
        int fit = (int)(px * cap / (float)gm.gmBlackBoxY + 0.5f);
        if (fit != px && fit > 0) {
            DeleteObject(f);
            f = MakeFont(face, fit);
        }
    }
    return f;
}

static void FontRelease(HdFont* f) {
    int i;

    for (i = 0; i < 256; i++) {
        if (f->glyphs[i].tex != NULL)
            SDL_DestroyTexture(f->glyphs[i].tex);
    }
    if (f->fallback != NULL && f->fallback != f->font)
        DeleteObject(f->fallback);
    if (f->font != NULL)
        DeleteObject(f->font);
    SDL_memset(f, 0, sizeof(*f));
}

static HdFont* FontForScale(float scale) {
    int key = (int)(scale * 16.0f + 0.5f);
    HdFont* f;
    int i;

    for (i = 0; i < 2; i++) {
        if (sFonts[i].font != NULL && sFonts[i].key == key)
            return &sFonts[i];
    }
    if (sDc == NULL)
        sDc = CreateCompatibleDC(NULL);
    LoadCustomFont();
    f = &sFonts[sNextFont];
    sNextFont ^= 1;
    FontRelease(f);
    f->key = key;
    f->cap = HD_TEXT_CAP * scale;
    f->font = FontWithCap(sFace, f->cap);
    f->fallback = sFace[0] == L'S' ? f->font : FontWithCap(L"Comic Sans MS", f->cap);
    return f;
}

static int HasGlyph(HFONT font, WCHAR wc) {
    WORD index = 0xFFFF;

    SelectObject(sDc, font);
    return GetGlyphIndicesW(sDc, &wc, 1, &index, GGI_MARK_NONEXISTING_GLYPHS) == 1 && index != 0xFFFF;
}

static int Render(HFONT font, WCHAR wc, Bitmap* out) {
    static const MAT2 identity = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };
    GLYPHMETRICS gm;
    DWORD size;

    SDL_memset(out, 0, sizeof(*out));
    SelectObject(sDc, font);
    size = GetGlyphOutlineW(sDc, wc, GGO_GRAY8_BITMAP, &gm, 0, NULL, &identity);
    if (size == GDI_ERROR)
        return 0;
    out->advance = gm.gmCellIncX;
    out->left = gm.gmptGlyphOrigin.x;
    out->top = gm.gmptGlyphOrigin.y;
    if (size > 0) {
        uint8_t* bits = SDL_malloc(size);
        int pitch = ((int)gm.gmBlackBoxX + 3) & ~3, x, y;

        out->w = (int)gm.gmBlackBoxX;
        out->h = (int)gm.gmBlackBoxY;
        out->a = SDL_calloc((size_t)out->w * out->h, 1);
        if (bits == NULL || out->a == NULL
            || GetGlyphOutlineW(sDc, wc, GGO_GRAY8_BITMAP, &gm, size, bits, &identity) == GDI_ERROR) {
            SDL_free(bits);
            SDL_free(out->a);
            out->a = NULL;
            out->w = out->h = 0;
            return 0;
        }
        for (y = 0; y < out->h; y++)
            for (x = 0; x < out->w; x++)
                out->a[y * out->w + x] = (uint8_t)(bits[y * pitch + x] * 255u / 64u);
        SDL_free(bits);
    }
    return 1;
}

/* Accented letters of Windows-1252: base letter and spacing accent. */
static int Decompose(WCHAR wc, WCHAR* base, WCHAR* accent) {
    static const struct {
        WCHAR c, base, accent;
    } table[] = {
        { 0xC0, 'A', 0x60 }, { 0xC1, 'A', 0xB4 }, { 0xC2, 'A', 0x2C6 }, { 0xC3, 'A', 0x2DC }, { 0xC4, 'A', 0xA8 },
        { 0xC5, 'A', 0x2DA }, { 0xC7, 'C', 0xB8 }, { 0xC8, 'E', 0x60 }, { 0xC9, 'E', 0xB4 }, { 0xCA, 'E', 0x2C6 },
        { 0xCB, 'E', 0xA8 }, { 0xCC, 'I', 0x60 }, { 0xCD, 'I', 0xB4 }, { 0xCE, 'I', 0x2C6 }, { 0xCF, 'I', 0xA8 },
        { 0xD1, 'N', 0x2DC }, { 0xD2, 'O', 0x60 }, { 0xD3, 'O', 0xB4 }, { 0xD4, 'O', 0x2C6 }, { 0xD5, 'O', 0x2DC },
        { 0xD6, 'O', 0xA8 }, { 0xD9, 'U', 0x60 }, { 0xDA, 'U', 0xB4 }, { 0xDB, 'U', 0x2C6 }, { 0xDC, 'U', 0xA8 },
        { 0xDD, 'Y', 0xB4 }, { 0xE0, 'a', 0x60 }, { 0xE1, 'a', 0xB4 }, { 0xE2, 'a', 0x2C6 }, { 0xE3, 'a', 0x2DC },
        { 0xE4, 'a', 0xA8 }, { 0xE5, 'a', 0x2DA }, { 0xE7, 'c', 0xB8 }, { 0xE8, 'e', 0x60 }, { 0xE9, 'e', 0xB4 },
        { 0xEA, 'e', 0x2C6 }, { 0xEB, 'e', 0xA8 }, { 0xEC, 0x131, 0x60 }, { 0xED, 0x131, 0xB4 },
        { 0xEE, 0x131, 0x2C6 }, { 0xEF, 0x131, 0xA8 }, { 0xF1, 'n', 0x2DC }, { 0xF2, 'o', 0x60 },
        { 0xF3, 'o', 0xB4 }, { 0xF4, 'o', 0x2C6 }, { 0xF5, 'o', 0x2DC }, { 0xF6, 'o', 0xA8 }, { 0xF9, 'u', 0x60 },
        { 0xFA, 'u', 0xB4 }, { 0xFB, 'u', 0x2C6 }, { 0xFC, 'u', 0xA8 }, { 0xFD, 'y', 0xB4 }, { 0xFF, 'y', 0xA8 },
    };
    size_t i;

    for (i = 0; i < sizeof(table) / sizeof(table[0]); i++) {
        if (table[i].c == wc) {
            *base = table[i].base;
            *accent = table[i].accent;
            return 1;
        }
    }
    return 0;
}

static int RowHasInk(const Bitmap* b, int y) {
    int x;

    for (x = 0; x < b->w; x++) {
        if (b->a[y * b->w + x] > 24)
            return 1;
    }
    return 0;
}

/* The base letter for an accent: a dotless i is the i without its dot. */
static int RenderBase(HFONT font, WCHAR base, Bitmap* out) {
    int y = 0;

    if (base != 0x131 || HasGlyph(font, base))
        return Render(font, base, out);
    if (!Render(font, L'i', out) || out->a == NULL)
        return 0;
    while (y < out->h && RowHasInk(out, y))
        y++; /* the dot */
    while (y < out->h && !RowHasInk(out, y))
        y++; /* the gap under it */
    if (y > 0 && y < out->h) {
        SDL_memmove(out->a, out->a + y * out->w, (size_t)(out->h - y) * out->w);
        out->h -= y;
        out->top -= y;
    }
    return 1;
}

/* base + accent in one bitmap; the accent sits centred above (cedilla below). */
static int Compose(HdFont* f, WCHAR wc, Bitmap* out) {
    WCHAR base, accent;
    Bitmap b, a;
    int gap = SDL_max(1, (int)(f->cap / 7.0f + 0.5f)), ax, atop, x0, y0, x1, y1, x, y;

    if (!Decompose(wc, &base, &accent) || !RenderBase(f->font, base, &b))
        return 0;
    if (b.a == NULL || !Render(f->fallback, accent, &a) || a.a == NULL) {
        *out = b;
        return 1;
    }
    ax = b.left + (b.w - a.w) / 2;
    atop = accent == 0xB8 ? b.top - b.h + gap / 2 : b.top + a.h + gap;
    x0 = SDL_min(b.left, ax);
    x1 = SDL_max(b.left + b.w, ax + a.w);
    y0 = SDL_max(b.top, atop);
    y1 = SDL_min(b.top - b.h, atop - a.h);
    out->w = x1 - x0;
    out->h = y0 - y1;
    out->left = x0;
    out->top = y0;
    out->advance = b.advance;
    out->a = SDL_calloc((size_t)out->w * out->h, 1);
    if (out->a != NULL) {
        for (y = 0; y < b.h; y++)
            for (x = 0; x < b.w; x++)
                out->a[(y0 - b.top + y) * out->w + (b.left - x0 + x)] = b.a[y * b.w + x];
        for (y = 0; y < a.h; y++) {
            for (x = 0; x < a.w; x++) {
                uint8_t* d = &out->a[(y0 - atop + y) * out->w + (ax - x0 + x)];
                *d = (uint8_t)SDL_max(*d, a.a[y * a.w + x]);
            }
        }
    }
    SDL_free(b.a);
    SDL_free(a.a);
    return out->a != NULL;
}

/* Spanish opening marks: the closing one turned over, hanging below the line
   from about the height of the small letters. */
static int Turned(HdFont* f, WCHAR wc, Bitmap* out) {
    WCHAR src = wc == 0xBF ? L'?' : L'!';
    int i, n;

    if (!HasGlyph(f->font, src) || !Render(f->font, src, out) || out->a == NULL)
        return 0;
    n = out->w * out->h;
    for (i = 0; i < n / 2; i++) {
        uint8_t t = out->a[i];
        out->a[i] = out->a[n - 1 - i];
        out->a[n - 1 - i] = t;
    }
    out->top = (int)(f->cap * 0.72f + 0.5f);
    return 1;
}

static HdGlyph* Glyph(SDL_Renderer* renderer, HdFont* f, int code) {
    HdGlyph* g = &f->glyphs[code & 255];
    char c = (char)code;
    WCHAR wc;
    Bitmap bm;
    int ok;

    if (g->ready)
        return g;
    g->ready = 1;
    if (MultiByteToWideChar(1252, 0, &c, 1, &wc, 1) != 1)
        return g;
    if (HasGlyph(f->font, wc))
        ok = Render(f->font, wc, &bm);
    else if (wc == 0xBF || wc == 0xA1)
        ok = Turned(f, wc, &bm) || Render(f->fallback, wc, &bm);
    else
        ok = Compose(f, wc, &bm) || Render(f->fallback, wc, &bm);
    if (!ok)
        return g;
    g->advance = bm.advance;
    g->left = bm.left;
    g->top = bm.top;
    if (bm.a != NULL && bm.w > 0 && bm.h > 0) {
        uint32_t* argb = SDL_malloc((size_t)bm.w * bm.h * 4);
        int i;

        if (argb != NULL) {
            for (i = 0; i < bm.w * bm.h; i++)
                argb[i] = ((uint32_t)bm.a[i] << 24) | 0xFFFFFFu;
            g->tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, bm.w, bm.h);
            if (g->tex != NULL) {
                SDL_UpdateTexture(g->tex, NULL, argb, bm.w * 4);
                SDL_SetTextureBlendMode(g->tex, SDL_BLENDMODE_BLEND);
                SDL_SetTextureScaleMode(g->tex, SDL_SCALEMODE_NEAREST);
                g->w = bm.w;
                g->h = bm.h;
            }
            SDL_free(argb);
        }
    }
    SDL_free(bm.a);
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
    int n, i, j, pass, lineY, lineGameEnd = 0;
    float space, lineEnd;

    if (!gPortConfig.hdText || scale <= 0.0f)
        return;
    n = PortPpuTextGlyphs(glyphs, HD_TEXT_MAX_GLYPHS, hudLayer);
    if (n == 0)
        return;
    font = FontForScale(scale);
    if (font->font == NULL)
        return;
    SDL_qsort(glyphs, n, sizeof(glyphs[0]), CompareGlyphs);

    /* Words: glyphs on one line, each where the previous one ends. */
    space = (float)Glyph(renderer, font, ' ')->advance;
    lineY = -1;
    lineEnd = 0;
    for (i = 0; i < n; i = j) {
        float start, pen, baseline;

        for (j = i + 1; j < n; j++) {
            const PortTextGlyph* p = &glyphs[j - 1];
            if (glyphs[j].y != p->y || glyphs[j].x != p->x + widths[p->code & 255])
                break;
        }
        start = originX + glyphs[i].x * scale;
        if (glyphs[i].y == lineY) {
            /* Words of a sentence follow each other with the font's own
               space; a wider gap is a column and keeps its place. */
            int gap = glyphs[i].x - lineGameEnd, gameSpace = SDL_max(1, widths[' ']);
            if (gap <= gameSpace * 2)
                start = lineEnd + space * SDL_max(1, (gap + gameSpace / 2) / gameSpace);
            else if (start < lineEnd + space)
                start = lineEnd + space;
        }
        baseline = originY + (glyphs[i].y + HD_TEXT_BASELINE) * scale;
        for (pass = 0; pass < 2; pass++) {
            int t;
            pen = start;
            for (t = i; t < j; t++) {
                HdGlyph* g = Glyph(renderer, font, glyphs[t].code);
                int alpha = glyphs[t].semi ? 0xA0 : 0xFF;
                if (pass == 0)
                    DrawGlyph(renderer, g, pen + shadow, baseline + shadow, glyphs[t].shadow, alpha);
                else
                    DrawGlyph(renderer, g, pen, baseline, glyphs[t].face, alpha);
                pen += g->advance;
            }
        }
        lineY = glyphs[i].y;
        lineEnd = pen;
        lineGameEnd = glyphs[j - 1].x + widths[glyphs[j - 1].code & 255];
    }
}
