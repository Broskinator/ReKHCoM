/* Portable outline rasterizer; the game's layout/occlusion is shared with
   Windows/Web. FreeType handles the existing CFF OpenType fonts directly. */
#include <SDL3/SDL.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "port/port.h"
#define HD_TEXT_CAP 7.0f
#define HD_TEXT_BASELINE 11
#define HD_TEXT_MAX_GLYPHS 256
#define HD_TEXT_JP_CELL 10
#define HD_TEXT_JP_CACHE 1024
typedef struct HdGlyph { SDL_Texture* tex; int ready,w,h,left,top,advance; } HdGlyph;
typedef struct HdFont {
    FT_Face font;
    int key,style;
    float cap,scale,condense;
    HdGlyph glyphs[256],jpGlyphs[HD_TEXT_JP_CACHE];
    uint16_t jpCodes[HD_TEXT_JP_CACHE];
} HdFont;
static FT_Library library;
static unsigned char* data[2];
static size_t sizes[2];
static HdFont fonts[3][2];
static int nextFont[3], tried, ready;
static FT_Face fallback, japanese;
const uint16_t* PortLatinGlyphWidths(void);
uint32_t PortFrameCount(void);
unsigned PortSjisToCodepoint(unsigned code);

int PortPortableHdReady(void) {
    static const char* names[2] = {"KHMenu.otf","ComicHearts.otf"};
    int i;
    if (tried) return ready;
    tried=1;
    if (FT_Init_FreeType(&library)) return 0;
    for (i=0;i<2;i++) {
        char path[1024]; FT_Face face;
        SDL_snprintf(path,sizeof(path),"assets/fonts/%s",names[i]);
        data[i]=SDL_LoadFile(PortConfigPath(path),&sizes[i]);
        if (!data[i]) {SDL_snprintf(path,sizeof(path),"fonts/%s",names[i]);data[i]=SDL_LoadFile(path,&sizes[i]);}
        if (!data[i] || FT_New_Memory_Face(library,data[i],sizes[i],0,&face)) return 0;
        if (!FT_Get_Char_Index(face,'H')) {FT_Done_Face(face);return 0;}
        FT_Done_Face(face);
    }
    /* Optional platform fallbacks; no platform fonts are redistributed. */
    FT_New_Face(library,"/system/fonts/Roboto-Regular.ttf",0,&fallback);
    if (!fallback) FT_New_Face(library,"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",0,&fallback);
    if (!fallback) FT_New_Face(library,"C:/Windows/Fonts/arial.ttf",0,&fallback);
    FT_New_Face(library,"/system/fonts/NotoSansCJK-Regular.ttc",0,&japanese);
    if (!japanese) FT_New_Face(library,"/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",0,&japanese);
#ifdef VERSION_JP
    /* Never hide Japanese SD glyphs without a usable Japanese outline face. */
    if (!japanese) return 0;
#endif
    ready=1;PortLog("HD text: portable KHMenu / Comic Hearts ready");return 1;
}
static void Release(HdFont* f) {
    int i;
    for(i=0;i<256;i++)SDL_DestroyTexture(f->glyphs[i].tex);
    for(i=0;i<HD_TEXT_JP_CACHE;i++)SDL_DestroyTexture(f->jpGlyphs[i].tex);
    if(f->font)FT_Done_Face(f->font);
    SDL_memset(f,0,sizeof(*f));
}
static HdFont* FontForScale(float scale,int style) {
    static const char sample[]="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const uint16_t* widths=PortLatinGlyphWidths();
    int key=(int)(scale*16+.5f),i,top,bottom,px;float natural=0,game=0;
    HdFont* f;
    for(i=0;i<2;i++)if(fonts[style][i].font && fonts[style][i].key==key)return &fonts[style][i];
    f=&fonts[style][nextFont[style]];nextFont[style]^=1;Release(f);
    if(!PortPortableHdReady())return f;
    if(FT_New_Memory_Face(library,data[style==1],sizes[style==1],0,&f->font))return f;
    f->key=key;f->style=style;f->scale=scale;f->cap=HD_TEXT_CAP*scale;
    FT_Load_Char(f->font,'H',FT_LOAD_NO_SCALE);
    top=(int)f->font->glyph->metrics.height;
    px=top>0?(int)(f->cap*f->font->units_per_EM/top+.5f):(int)(f->cap*1.5f+.5f);
    FT_Set_Pixel_Sizes(f->font,0,SDL_max(1,px));
    for(i=0;sample[i];i++) {
        if(!FT_Load_Char(f->font,sample[i],FT_LOAD_DEFAULT))natural+=f->font->glyph->advance.x/64.0f;
        game+=widths[(unsigned char)sample[i]]*scale;
    }
    f->condense=natural>game*1.02f?game/natural:1.0f;
    (void)bottom;return f;
}
static unsigned Unicode(int code,int jp) {
    static const uint16_t cp1252[32]={0x20AC,0x81,0x201A,0x192,0x201E,0x2026,0x2020,0x2021,
        0x2C6,0x2030,0x160,0x2039,0x152,0x8D,0x17D,0x8F,0x90,0x2018,0x2019,0x201C,
        0x201D,0x2022,0x2013,0x2014,0x2DC,0x2122,0x161,0x203A,0x153,0x9D,0x17E,0x178};
    return jp?PortSjisToCodepoint(code):code>=128&&code<160?cp1252[code-128]:(unsigned)code;
}
static void Raster(SDL_Renderer* renderer,HdFont* f,HdGlyph* g,int code,int jp) {
    FT_Face face=f->font;FT_Matrix matrix;FT_Bitmap* b;unsigned cp=Unicode(code,jp);uint32_t* rgba;int x,y;
    if(g->ready)return;g->ready=1;
    if(!face || !cp)return;
    if(jp)face=japanese;
    else if(!FT_Get_Char_Index(face,cp))face=fallback;
    if(!face || !FT_Get_Char_Index(face,cp)) {ready=0;return;}
    if(face!=f->font) {
        FT_Set_Pixel_Sizes(face,0,SDL_max(1,(int)(f->cap*(jp?11.0f/7.0f:1.4f)+.5f)));
    }
    matrix.xx=(FT_Fixed)((jp?1:f->condense)*65536);matrix.xy=matrix.yx=0;matrix.yy=65536;
    FT_Set_Transform(face,&matrix,NULL);
    if(FT_Load_Char(face,cp,FT_LOAD_RENDER|FT_LOAD_TARGET_NORMAL)){ready=0;return;}
    b=&face->glyph->bitmap;g->w=b->width;g->h=b->rows;g->left=face->glyph->bitmap_left;
    g->top=face->glyph->bitmap_top;g->advance=(face->glyph->advance.x+32)>>6;
    if(!g->w || !g->h)return;
    if(b->pixel_mode!=FT_PIXEL_MODE_GRAY || g->w>512 || g->h>512){ready=0;return;}
    rgba=SDL_malloc((size_t)g->w*g->h*4);if(!rgba){ready=0;return;}
    for(y=0;y<g->h;y++)for(x=0;x<g->w;x++) {
        const unsigned char* row=b->buffer+(b->pitch<0?(g->h-1-y)*-b->pitch:y*b->pitch);
        rgba[y*g->w+x]=((uint32_t)row[x]<<24)|0xFFFFFF;
    }
    g->tex=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STATIC,g->w,g->h);
    if(g->tex){SDL_UpdateTexture(g->tex,NULL,rgba,g->w*4);SDL_SetTextureBlendMode(g->tex,SDL_BLENDMODE_BLEND);SDL_SetTextureScaleMode(g->tex,SDL_SCALEMODE_NEAREST);}
    else ready=0;
    SDL_free(rgba);
}
static HdGlyph* Glyph(SDL_Renderer* r,HdFont* f,int code) {HdGlyph* g=&f->glyphs[code&255];Raster(r,f,g,code&255,0);return g;}
static HdGlyph* GlyphJp(SDL_Renderer* r,HdFont* f,uint16_t code) {
    static HdGlyph none;unsigned slot=(code*2654435761u)>>22,k;
    for(k=0;k<HD_TEXT_JP_CACHE;k++,slot=(slot+1)%HD_TEXT_JP_CACHE)
        if(!f->jpCodes[slot] || f->jpCodes[slot]==code){f->jpCodes[slot]=code;Raster(r,f,&f->jpGlyphs[slot],code,1);return &f->jpGlyphs[slot];}
    return &none;
}
#include "port/port_hdtext_layout.inc"
