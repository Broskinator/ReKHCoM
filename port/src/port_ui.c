/*
 * In-game options overlay (graphics, display and sound settings).
 *
 * Drawn into the frame at the internal resolution with the game's own Latin
 * message font, so it matches the pixel art. Opened from the title screen
 * (SELECT), with F1 / Esc on the keyboard, or Back+Start / Guide on a
 * gamepad. The game is paused while it is open.
 */
#include <SDL3/SDL.h>
#include <string.h>

#include "port/port.h"
#include "port/port_ui.h"

extern void* gMsgLatinFontFrames[];
extern const uint8_t gMsgLatinFontTiles[];
#ifdef VERSION_EU
extern uint32_t gLanguage;
#endif
const uint16_t* PortLatinGlyphWidths(void);
void PortInputSuppressUntilRelease(void);

enum { LANG_EN, LANG_FR, LANG_DE, LANG_IT, LANG_ES, LANG_JA, LANG_COUNT };

enum {
    ITEM_VIEW,
    ITEM_ZOOM,
    ITEM_SCALE,
    ITEM_HUD,
    ITEM_NARROW,
    ITEM_FILTER,
    ITEM_FULLSCREEN,
    ITEM_FS_RES,
    ITEM_VSYNC,
    ITEM_SCANLINES,
    ITEM_LCD,
    ITEM_HDTEXT,
    ITEM_CARDS,
    ITEM_FPS,
    ITEM_DODGE,
    ITEM_STOCK,
    ITEM_MUSIC,
    ITEM_SFX,
    ITEM_MASTER,
    ITEM_CLOSE,
    ITEM_COUNT
};

/* Strings: UTF-8, one row per item, columns in LANG_* order. */
static const char* const sLabels[ITEM_COUNT][LANG_COUNT] = {
    { "View", "Vue", "Ansicht", "Visuale", "Vista", "表示" },
    { "Zoom", "Zoom", "Zoom", "Zoom", "Zoom", "ズーム" },
    { "Scaling", "Mise à l'échelle", "Skalierung", "Scala", "Escalado", "スケール" },
    { "HUD size", "Taille du HUD", "HUD-Größe", "Dimensione HUD", "Tamaño del HUD", "HUDサイズ" },
    { "GBA-size scenes", "Scènes format GBA", "Szenen im GBA-Format", "Scene formato GBA", "Escenas tamaño GBA", "GBAサイズの場面" },
    { "Filter", "Filtre", "Filter", "Filtro", "Filtro", "フィルター" },
    { "Fullscreen", "Plein écran", "Vollbild", "Schermo intero", "Pantalla completa", "フルスクリーン" },
    { "Fullscreen resolution", "Résolution plein écran", "Vollbild-Auflösung", "Risoluzione a schermo intero",
      "Resolución completa", "フルスクリーン解像度" },
    { "VSync", "VSync", "VSync", "VSync", "VSync", "VSync" },
    { "Scanlines", "Lignes de balayage", "Scanlines", "Linee di scansione", "Scanlines", "スキャンライン" },
    { "GBA LCD colours", "Couleurs LCD GBA", "GBA-LCD-Farben", "Colori LCD GBA", "Colores LCD GBA", "GBAカラー" },
    { "HD text", "Texte HD", "HD-Text", "Testo HD", "Texto HD", "HDテキスト" },
    { "Battle cards raised", "Cartes de combat relevées", "Kampfkarten angehoben", "Carte di battaglia rialzate",
      "Subir cartas en combate", "カードを上げる" },
    { "Show FPS", "Afficher FPS", "FPS anzeigen", "Mostra FPS", "Mostrar FPS", "FPS表示" },
    { "One-button dodge roll", "Roulade en un bouton", "Ausweichrolle mit einer Taste", "Capriola con un tasto",
      "Voltereta con un botón", "ワンボタンドッジ" },
    { "One-button stock (L+R)", "Réserve en un bouton (L+R)", "Vorrat mit einer Taste (L+R)",
      "Accumulo con un tasto (L+R)", "Acumular con un botón (L+R)", "ワンボタンストック (L+R)" },
    { "Music volume", "Volume musique", "Musiklautstärke", "Volume musica", "Volumen música", "音楽の音量" },
    { "Effects volume", "Volume effets", "Effektlautstärke", "Volume effetti", "Volumen efectos", "効果音の音量" },
    { "Master volume", "Volume général", "Gesamtlautstärke", "Volume generale", "Volumen general", "全体の音量" },
    { "Back", "Retour", "Zurück", "Indietro", "Volver", "戻る" },
};

static const char* const sTitle[LANG_COUNT] = { "Options", "Options", "Optionen", "Opzioni", "Opciones",
    "オプション",
};
static const char* const sHelp[LANG_COUNT] = {
    "\xE2\x86\x91\xE2\x86\x93 Select   \xE2\x86\x90\xE2\x86\x92 Change   B Back",
    "\xE2\x86\x91\xE2\x86\x93 Choisir   \xE2\x86\x90\xE2\x86\x92 Modifier   B Retour",
    "\xE2\x86\x91\xE2\x86\x93 Wählen   \xE2\x86\x90\xE2\x86\x92 Ändern   B Zurück",
    "\xE2\x86\x91\xE2\x86\x93 Scegli   \xE2\x86\x90\xE2\x86\x92 Cambia   B Indietro",
    "\xE2\x86\x91\xE2\x86\x93 Elegir   \xE2\x86\x90\xE2\x86\x92 Cambiar   B Volver",
    "\xE2\x86\x91\xE2\x86\x93 選択   \xE2\x86\x90\xE2\x86\x92 変更   B 戻る",
};
static const char* const sTitleHint[LANG_COUNT] = { "SELECT  Options", "SELECT  Options", "SELECT  Optionen",
                                                    "SELECT  Opzioni", "SELECT  Opciones",
    "SELECT  オプション",
};
static const char* const sOnOff[2][LANG_COUNT] = {
    { "Off", "Non", "Aus", "No", "No", "オフ" },
    { "On", "Oui", "An", "Sì", "Sí", "オン" },
};
static const char* const sViews[PORT_VIEW_COUNT][LANG_COUNT] = {
    { "Original 240x160", "Originale 240x160", "Original 240x160", "Originale 240x160", "Original 240x160", "オリジナル 240x160" },
    { "Wide 284x160", "Large 284x160", "Breit 284x160", "Panoramica 284x160", "Panorámica 284x160", "ワイド 284x160" },
    { "Extended 320x180", "Étendue 320x180", "Erweitert 320x180", "Estesa 320x180", "Ampliada 320x180", "拡張 320x180" },
    { "Maximum 384x216", "Maximale 384x216", "Maximal 384x216", "Massima 384x216", "Máxima 384x216", "最大 384x216" },
    { "Auto (fill window)", "Auto (remplir)", "Auto (füllen)", "Auto (riempi)", "Auto (llenar ventana)", "自動 (画面に合わせる)" },
};
static const char* const sZooms[4][LANG_COUNT] = {
    { "Near", "Proche", "Nah", "Vicino", "Cerca", "近い" },
    { "Normal", "Normal", "Normal", "Normale", "Normal", "ふつう" },
    { "Far", "Loin", "Weit", "Lontano", "Lejos", "遠い" },
    { "Very far", "Très loin", "Sehr weit", "Molto lontano", "Muy lejos", "とても遠い" },
};
static const char* const sScales[PORT_SCALE_COUNT][LANG_COUNT] = {
    { "Integer", "Entière", "Ganzzahlig", "Intera", "Entero", "ドットばい" },
    { "Fit", "Ajuster", "Einpassen", "Adatta", "Ajustar", "合わせる" },
    { "Stretch", "Étirer", "Strecken", "Allunga", "Estirar", "引き伸ばし" },
};
static const char* const sFilters[PORT_FILTER_COUNT][LANG_COUNT] = {
    { "Sharp", "Net", "Scharf", "Nitido", "Nítido", "シャープ" },
    { "Pixel art", "Pixel art", "Pixel-Art", "Pixel art", "Pixel art", "ドット絵" },
    { "Smooth", "Lisse", "Weich", "Morbido", "Suave", "なめらか" },
};
static const char* const sHudSizes[3][LANG_COUNT] = {
    { "Original", "Original", "Original", "Originale", "Original", "オリジナル" },
    { "Large", "Grand", "Groß", "Grande", "Grande", "大きい" },
    { "Larger", "Plus grand", "Größer", "Più grande", "Más grande", "もっと大きい" },
};
static const char* const sNarrow[2][LANG_COUNT] = {
    { "Exact pixels", "Pixels exacts", "Exakte Pixel", "Pixel esatti", "Píxel exacto", "等倍ピクセル" },
    { "Fill height", "Remplir la hauteur", "Höhe füllen", "Riempi altezza", "Llenar altura", "高さに合わせる" },
};
static const char* const sDesktop[LANG_COUNT] = { "Desktop", "Bureau", "Desktop", "Desktop", "Escritorio",
    "デスクトップ",
};
static const char* const sAutoOnly[LANG_COUNT] = { "(Auto view)", "(vue Auto)", "(Auto-Ansicht)", "(vista Auto)",
                                                   "(vista Auto)",
    "(自動表示のみ)",
};

static const int sViewOrder[PORT_VIEW_COUNT] = { PORT_VIEW_AUTO, PORT_VIEW_ORIGINAL, PORT_VIEW_WIDE,
                                                 PORT_VIEW_EXTENDED, PORT_VIEW_MAXIMUM };

static int sOpen;
static int sCursor;
static int sScroll;
static int sTitleHintFrames;
static char sNotice[96];
static int sNoticeFrames;
static int sSkipPrompt;
static int sModeW[64], sModeH[64], sModeCount;
static uint32_t sPadBack;

/* ---- drawing ------------------------------------------------------------------ */

static uint32_t* sFb;
static int sFbW, sFbH;

static int Lang(void) {
#ifdef VERSION_EU
    return gLanguage < LANG_COUNT ? (int)gLanguage : LANG_EN;
#elif defined(VERSION_JP)
    return LANG_JA;
#else
    return LANG_EN;
#endif
}

#ifdef VERSION_JP
/* Japanese characters: drawn from the cartridge's Shift-JIS font, 10 pixels apart. */
#define JP_CHAR 0x1000000
#define JP_ADVANCE 10
int PortCodepointToSjis(unsigned cp);
void GetSjisGlyph(uint16_t code, uint16_t* glyph, uint8_t* bank);
extern void* gMsgFontBank0Frames[];
extern void* gMsgFontBank1Frames[];
extern void* gMsgFontBank2Frames[];
extern void* gMsgFontBank3Frames[];
extern const uint8_t gMsgFontBank0Tiles[];
extern const uint8_t gMsgFontBank1Tiles[];
extern const uint8_t gMsgFontBank2Tiles[];
extern const uint8_t gMsgFontBank3Tiles[];
#endif

/* Next character of a UTF-8 string as a code of the game's Latin font. */
static int NextChar(const char** s) {
    const unsigned char* p = (const unsigned char*)*s;
    uint32_t c = *p++;

    if (c >= 0xE0 && p[0] && p[1]) {
        c = ((c & 15) << 12) | ((p[0] & 63) << 6) | (p[1] & 63);
        p += 2;
    } else if (c >= 0xC0 && p[0]) {
        c = ((c & 31) << 6) | (p[0] & 63);
        p += 1;
    }
    *s = (const char*)p;
    switch (c) {
        case 0x2191:
            return 0x0A;
        case 0x2193:
            return 0x0B;
        case 0x2190:
            return 0x0C;
        case 0x2192:
            return 0x0D;
        case 0x25CB:
            return 0x0E;
    }
#ifdef VERSION_JP
    if (c >= 0x3000)
        return JP_CHAR | (int)c; /* kana, kanji and full-width forms */
#endif
    return c < 256 ? (int)c : '?';
}

static void Pixel(int x, int y, uint32_t c) {
    if (x >= 0 && y >= 0 && x < sFbW && y < sFbH)
        sFb[y * sFbW + x] = c;
}

static uint32_t Mix(uint32_t a, uint32_t b, int t) {
    uint32_t r = (((a >> 16) & 255) * (256 - t) + ((b >> 16) & 255) * t) >> 8;
    uint32_t g = (((a >> 8) & 255) * (256 - t) + ((b >> 8) & 255) * t) >> 8;
    uint32_t bl = ((a & 255) * (256 - t) + (b & 255) * t) >> 8;
    return 0xFF000000u | (r << 16) | (g << 8) | bl;
}

static void Shade(int x0, int y0, int w, int h, uint32_t color, int alpha) {
    int x, y;

    for (y = SDL_max(y0, 0); y < SDL_min(y0 + h, sFbH); y++)
        for (x = SDL_max(x0, 0); x < SDL_min(x0 + w, sFbW); x++)
            sFb[y * sFbW + x] = Mix(sFb[y * sFbW + x], color, alpha);
}

static void Rect(int x0, int y0, int w, int h, uint32_t c) {
    int i;

    for (i = 0; i < w; i++) {
        Pixel(x0 + i, y0, c);
        Pixel(x0 + i, y0 + h - 1, c);
    }
    for (i = 0; i < h; i++) {
        Pixel(x0, y0 + i, c);
        Pixel(x0 + w - 1, y0 + i, c);
    }
}

static int GlyphWidth(int code) {
#ifdef VERSION_JP
    if (code & JP_CHAR)
        return JP_ADVANCE;
#endif
    return PortLatinGlyphWidths()[code & 255];
}

static int TextWidth(const char* s) {
    int w = 0;

    while (*s)
        w += GlyphWidth(NextChar(&s));
    return w;
}

/* The font's glyphs occupy rows 3..12 of their 16x16 cell; y is the top of
   that band. Colour 1 is the face, colour 2 the shadow. */
static void DrawGlyph(int code, int x, int y, uint32_t face, uint32_t shadow) {
    const uint16_t* frame;
    const uint8_t* tiles;
    int px, py, top = 3, bottom = 13;

    if (code == ' ' || code <= 0)
        return;
#ifdef VERSION_JP
    if (code & JP_CHAR) {
        static void** const frames[4] = { gMsgFontBank0Frames, gMsgFontBank1Frames, gMsgFontBank2Frames,
                                          gMsgFontBank3Frames };
        static const uint8_t* const banks[4] = { gMsgFontBank0Tiles, gMsgFontBank1Tiles, gMsgFontBank2Tiles,
                                                 gMsgFontBank3Tiles };
        int sjis = PortCodepointToSjis((unsigned)(code & 0xFFFF));
        uint16_t glyph = 0;
        uint8_t bank = 0;

        if ((sjis >> 8) == 0x81) {
            /* punctuation: the table LayoutMsgGlyphsSjis uses (msg.c) */
            static const uint16_t row81[][2] = {
                { 0x41, 0xF5 }, { 0x42, 0xF6 }, { 0x43, 0xF8 }, { 0x44, 0xF7 }, { 0x45, 0xF9 }, { 0x48, 0xF1 },
                { 0x49, 0xF0 }, { 0x5B, 0xFD }, { 0x5C, 0xFC }, { 0x5E, 0xF3 }, { 0x60, 0xFE }, { 0x63, 0xFB },
                { 0x66, 0xFF }, { 0x69, 0xEC }, { 0x6A, 0xED }, { 0x75, 0xE8 }, { 0x76, 0xE9 }, { 0x77, 0xEA },
                { 0x78, 0xEB }, { 0x7B, 0xDF }, { 0x7C, 0xFC }, { 0x81, 0xEF }, { 0x93, 0xF2 }, { 0x96, 0xF4 },
            };
            size_t k;
            for (k = 0; k < sizeof(row81) / sizeof(row81[0]); k++)
                if (row81[k][0] == (sjis & 0xFF))
                    break;
            if (k == sizeof(row81) / sizeof(row81[0]))
                return;
            glyph = row81[k][1];
        } else {
            GetSjisGlyph((uint16_t)sjis, &glyph, &bank);
        }
        if (bank > 3)
            return;
        frame = (const uint16_t*)frames[bank][glyph];
        if (frame == NULL)
            return;
        tiles = banks[bank] + (frame[3] & 0x3FF) * 32;
        top = 0;
        bottom = 15;
    } else
#endif
    {
        frame = (const uint16_t*)gMsgLatinFontFrames[code & 255];
        if (frame == NULL || frame[0] == 0)
            return;
        tiles = gMsgLatinFontTiles + (frame[3] & 0x3FF) * 32;
    }
    /* Latin glyphs use rows 3..12 of the cell, Japanese ones nearly all of it */
    for (py = top; py < bottom; py++) {
        for (px = 0; px < 16; px++) {
            int t = (py >> 3) * 2 + (px >> 3);
            uint8_t b = tiles[t * 32 + (py & 7) * 4 + ((px & 7) >> 1)];
            int v = (px & 1) ? (b >> 4) : (b & 15);
            if (v == 1)
                Pixel(x + px, y + py - 3, face);
            else if (v >= 2)
                Pixel(x + px, y + py - 3, shadow);
        }
    }
}

static int DrawText(const char* s, int x, int y, uint32_t face, uint32_t shadow) {
    while (*s) {
        int c = NextChar(&s);
        DrawGlyph(c, x, y, face, shadow);
        x += GlyphWidth(c);
    }
    return x;
}

/* ---- values ------------------------------------------------------------------ */

static void RefreshModes(void) {
    sModeCount = PortVideoListModes(sModeW, sModeH, 64);
}

static int ViewIndex(void) {
    int i;

    for (i = 0; i < PORT_VIEW_COUNT; i++)
        if (sViewOrder[i] == gPortConfig.viewMode)
            return i;
    return 0;
}

static int ModeIndex(void) {
    int i;

    for (i = 0; i < sModeCount; i++)
        if (sModeW[i] == gPortConfig.fullscreenWidth && sModeH[i] == gPortConfig.fullscreenHeight)
            return i + 1;
    return 0;
}

static int ItemEnabled(int item) {
    switch (item) {
        case ITEM_ZOOM:
            return gPortConfig.viewMode == PORT_VIEW_AUTO;
        case ITEM_SCALE:
            return gPortConfig.viewMode != PORT_VIEW_AUTO;
        default:
            return 1;
    }
}

static void ValueText(int item, char* out, size_t size) {
    int l = Lang();

    switch (item) {
        case ITEM_VIEW:
            SDL_strlcpy(out, sViews[gPortConfig.viewMode][l], size);
            break;
        case ITEM_ZOOM:
            if (ItemEnabled(item))
                SDL_strlcpy(out, sZooms[gPortConfig.zoom][l], size);
            else
                SDL_strlcpy(out, sAutoOnly[l], size);
            break;
        case ITEM_SCALE:
            SDL_strlcpy(out, sScales[gPortConfig.scaleMode][l], size);
            break;
        case ITEM_HUD:
            SDL_strlcpy(out, sHudSizes[SDL_clamp(gPortConfig.hudSize, 0, 2)][l], size);
            break;
        case ITEM_NARROW:
            SDL_strlcpy(out, sNarrow[gPortConfig.narrowFit != 0][l], size);
            break;
        case ITEM_FILTER:
            SDL_strlcpy(out, sFilters[gPortConfig.filter][l], size);
            break;
        case ITEM_FULLSCREEN:
            SDL_strlcpy(out, sOnOff[gPortConfig.fullscreen != 0][l], size);
            break;
        case ITEM_FS_RES:
            if (gPortConfig.fullscreenWidth <= 0)
                SDL_strlcpy(out, sDesktop[l], size);
            else
                SDL_snprintf(out, size, "%dx%d", gPortConfig.fullscreenWidth, gPortConfig.fullscreenHeight);
            break;
        case ITEM_VSYNC:
            SDL_strlcpy(out, sOnOff[gPortConfig.vsync != 0][l], size);
            break;
        case ITEM_SCANLINES:
            if (gPortConfig.scanlines == 0)
                SDL_strlcpy(out, sOnOff[0][l], size);
            else
                SDL_snprintf(out, size, "%d%%", gPortConfig.scanlines);
            break;
        case ITEM_LCD:
            SDL_strlcpy(out, sOnOff[gPortConfig.colorCorrection != 0][l], size);
            break;
        case ITEM_HDTEXT:
            SDL_strlcpy(out, sOnOff[gPortConfig.hdText != 0][l], size);
            break;
        case ITEM_CARDS:
            if (gPortConfig.cardHudRaise == 0)
                SDL_strlcpy(out, sOnOff[0][l], size);
            else
                SDL_snprintf(out, size, "%d px", gPortConfig.cardHudRaise);
            break;
        case ITEM_FPS:
            SDL_strlcpy(out, sOnOff[gPortConfig.showFps != 0][l], size);
            break;
        case ITEM_DODGE:
            SDL_strlcpy(out, sOnOff[gPortConfig.dodgeButton != 0][l], size);
            break;
        case ITEM_STOCK:
            SDL_strlcpy(out, sOnOff[gPortConfig.stockButton != 0][l], size);
            break;
        case ITEM_MUSIC:
            SDL_snprintf(out, size, "%d%%", gPortConfig.musicVolume);
            break;
        case ITEM_SFX:
            SDL_snprintf(out, size, "%d%%", gPortConfig.sfxVolume);
            break;
        case ITEM_MASTER:
            SDL_snprintf(out, size, "%d%%", gPortConfig.masterVolume);
            break;
        default:
            out[0] = 0;
            break;
    }
}

static int Wrap(int v, int n) {
    return ((v % n) + n) % n;
}

static void Change(int item, int dir) {
    int i;

    if (!ItemEnabled(item))
        return;
    switch (item) {
        case ITEM_VIEW:
            gPortConfig.viewMode = sViewOrder[Wrap(ViewIndex() + dir, PORT_VIEW_COUNT)];
            break;
        case ITEM_ZOOM:
            gPortConfig.zoom = SDL_clamp(gPortConfig.zoom + dir, 0, 3);
            break;
        case ITEM_SCALE:
            gPortConfig.scaleMode = Wrap(gPortConfig.scaleMode + dir, PORT_SCALE_COUNT);
            break;
        case ITEM_HUD:
            gPortConfig.hudSize = Wrap(gPortConfig.hudSize + dir, 3);
            break;
        case ITEM_NARROW:
            gPortConfig.narrowFit = !gPortConfig.narrowFit;
            break;
        case ITEM_FILTER:
            gPortConfig.filter = Wrap(gPortConfig.filter + dir, PORT_FILTER_COUNT);
            break;
        case ITEM_FULLSCREEN:
            gPortConfig.fullscreen = !gPortConfig.fullscreen;
            break;
        case ITEM_FS_RES:
            i = Wrap(ModeIndex() + dir, sModeCount + 1);
            gPortConfig.fullscreenWidth = i ? sModeW[i - 1] : 0;
            gPortConfig.fullscreenHeight = i ? sModeH[i - 1] : 0;
            break;
        case ITEM_VSYNC:
            gPortConfig.vsync = !gPortConfig.vsync;
            break;
        case ITEM_SCANLINES:
            gPortConfig.scanlines = Wrap(gPortConfig.scanlines / 25 + dir, 5) * 25;
            break;
        case ITEM_LCD:
            gPortConfig.colorCorrection = !gPortConfig.colorCorrection;
            break;
        case ITEM_HDTEXT:
            gPortConfig.hdText = !gPortConfig.hdText;
            break;
        case ITEM_CARDS:
            gPortConfig.cardHudRaise = Wrap(gPortConfig.cardHudRaise / 4 + dir, 5) * 4;
            break;
        case ITEM_FPS:
            gPortConfig.showFps = !gPortConfig.showFps;
            break;
        case ITEM_DODGE:
            gPortConfig.dodgeButton = !gPortConfig.dodgeButton;
            break;
        case ITEM_STOCK:
            gPortConfig.stockButton = !gPortConfig.stockButton;
            break;
        case ITEM_MUSIC:
            gPortConfig.musicVolume = SDL_clamp(gPortConfig.musicVolume + dir * 10, 0, 100);
            break;
        case ITEM_SFX:
            gPortConfig.sfxVolume = SDL_clamp(gPortConfig.sfxVolume + dir * 10, 0, 100);
            break;
        case ITEM_MASTER:
            gPortConfig.masterVolume = SDL_clamp(gPortConfig.masterVolume + dir * 10, 0, 100);
            break;
    }
    PortVideoApplyConfig();
}

/* ---- public ------------------------------------------------------------------ */

void PortUiInit(void) {
}

int PortUiActive(void) {
    return sOpen;
}

void PortUiOpenOptions(void) {
    if (sOpen)
        return;
    sOpen = 1;
    sCursor = 0;
    sScroll = 0;
    if (SDL_getenv("KHCOM_OPTIONS_CURSOR")) /* test harness: shot of a lower row */
        sCursor = SDL_clamp(SDL_atoi(SDL_getenv("KHCOM_OPTIONS_CURSOR")), 0, ITEM_COUNT - 1);
    RefreshModes();
}

void PortUiForceClose(void) {
    sOpen = 0;
}

static void Close(void) {
    sOpen = 0;
    PortConfigSave();
    PortInputSuppressUntilRelease();
}

void PortUiNotice(const char* text) {
    static const char* const prefix[LANG_COUNT][4] = {
        { "State", "saved", "loaded", "Slot" },
        { "État", "sauvegardé", "chargé", "Emplacement" },
        { "Zustand", "gespeichert", "geladen", "Platz" },
        { "Stato", "salvato", "caricato", "Slot" },
        { "Estado", "guardado", "cargado", "Ranura" },
        { "ステート", "セーブ", "ロード", "スロット" },
    };
    int l = Lang(), n;

    /* Translate the few messages the platform layer emits. */
    if (SDL_sscanf(text, "State %d saved", &n) == 1 && SDL_strstr(text, "saved"))
        SDL_snprintf(sNotice, sizeof(sNotice), "%s %d %s", prefix[l][0], n, prefix[l][1]);
    else if (SDL_sscanf(text, "State %d loaded", &n) == 1 && SDL_strstr(text, "loaded"))
        SDL_snprintf(sNotice, sizeof(sNotice), "%s %d %s", prefix[l][0], n, prefix[l][2]);
    else if (SDL_sscanf(text, "Slot %d", &n) == 1)
        SDL_snprintf(sNotice, sizeof(sNotice), "%s %d", prefix[l][3], n);
    else
        SDL_strlcpy(sNotice, text, sizeof(sNotice));
    sNoticeFrames = 120;
}

void PortUiTitleHint(void) {
    sTitleHintFrames = 2;
}

static void Move(int dir) {
    do {
        sCursor = Wrap(sCursor + dir, ITEM_COUNT);
    } while (!ItemEnabled(sCursor));
}

int PortUiHandleEvent(const void* p) {
    const SDL_Event* ev = (const SDL_Event*)p;

    if (!sOpen) {
        if (ev->type == SDL_EVENT_KEY_DOWN && !ev->key.repeat && (ev->key.key == SDLK_F1 || ev->key.key == SDLK_ESCAPE)) {
            PortUiOpenOptions();
            return 1;
        }
        if (ev->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
            if (ev->gbutton.button == SDL_GAMEPAD_BUTTON_GUIDE
                || (ev->gbutton.button == SDL_GAMEPAD_BUTTON_START && (sPadBack & 1))
                || (ev->gbutton.button == SDL_GAMEPAD_BUTTON_BACK && (sPadBack & 2))) {
                PortUiOpenOptions();
                return 1;
            }
            if (ev->gbutton.button == SDL_GAMEPAD_BUTTON_BACK)
                sPadBack |= 1;
            if (ev->gbutton.button == SDL_GAMEPAD_BUTTON_START)
                sPadBack |= 2;
        }
        if (ev->type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
            if (ev->gbutton.button == SDL_GAMEPAD_BUTTON_BACK)
                sPadBack &= ~1u;
            if (ev->gbutton.button == SDL_GAMEPAD_BUTTON_START)
                sPadBack &= ~2u;
        }
        return 0;
    }

    if (ev->type == SDL_EVENT_KEY_DOWN) {
        switch (ev->key.key) {
            case SDLK_UP:
            case SDLK_W:
                Move(-1);
                break;
            case SDLK_DOWN:
            case SDLK_S:
                Move(1);
                break;
            case SDLK_LEFT:
            case SDLK_A:
                Change(sCursor, -1);
                break;
            case SDLK_RIGHT:
            case SDLK_D:
                Change(sCursor, 1);
                break;
            case SDLK_RETURN:
            case SDLK_SPACE:
            case SDLK_X:
            case SDLK_K:
                if (sCursor == ITEM_CLOSE)
                    Close();
                else
                    Change(sCursor, 1);
                break;
            case SDLK_ESCAPE:
            case SDLK_BACKSPACE:
            case SDLK_F1:
            case SDLK_Z:
            case SDLK_J:
                if (!ev->key.repeat)
                    Close();
                break;
        }
        return 1;
    }
    if (ev->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        switch (ev->gbutton.button) {
            case SDL_GAMEPAD_BUTTON_DPAD_UP:
                Move(-1);
                break;
            case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
                Move(1);
                break;
            case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
                Change(sCursor, -1);
                break;
            case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
                Change(sCursor, 1);
                break;
            case SDL_GAMEPAD_BUTTON_SOUTH:
                if (sCursor == ITEM_CLOSE)
                    Close();
                else
                    Change(sCursor, 1);
                break;
            case SDL_GAMEPAD_BUTTON_EAST:
            case SDL_GAMEPAD_BUTTON_START:
            case SDL_GAMEPAD_BUTTON_BACK:
            case SDL_GAMEPAD_BUTTON_GUIDE:
                Close();
                break;
        }
        return 1;
    }
    if (ev->type == SDL_EVENT_KEY_UP || ev->type == SDL_EVENT_GAMEPAD_BUTTON_UP)
        return 1;
    return 0;
}

#define COL_TEXT 0xFFF0F4FFu
#define COL_SHADOW 0xFF202848u
#define COL_TITLE 0xFFF8D870u
#define COL_VALUE 0xFFA8D8FFu
#define COL_DIM 0xFF7880A0u
#define COL_PANEL 0xFF0C1030u
#define COL_BORDER 0xFFD8C080u
#define COL_SELECT 0xFF3050A8u

static const char* const sSkipText[LANG_COUNT] = {
    "Press again to skip the intro", "Appuyez encore pour passer l'intro", "Erneut drücken: Intro überspringen",
    "Premi di nuovo per saltare l'intro", "Pulsa otra vez para saltar la intro",
    "もう一度押すとイントロをスキップ",
};

void PortUiSkipPrompt(int visible) {
    sSkipPrompt = visible;
}

/* Bottom right of the GBA picture: an A button and the text, in the game font. */
static void DrawSkipPrompt(int l) {
    const char* text = sSkipText[l];
    int w = TextWidth(text) + 26, h = 15;
    int x = gPortScreen.originX + GBA_SCREEN_W - w - 4;
    int y = gPortScreen.originY + GBA_SCREEN_H - h - 4;
    int cx = x + 9, cy = y + 7, dx, dy;

    Shade(x, y, w, h, COL_PANEL, 210);
    Rect(x, y, w, h, COL_BORDER);
    for (dy = -5; dy <= 5; dy++)
        for (dx = -5; dx <= 5; dx++)
            if (dx * dx + dy * dy <= 26)
                Pixel(cx + dx, cy + dy, 0xFFE8ECF4u);
    DrawGlyph('A', cx - 2, cy - 4, 0xFF283050u, 0xFFE8ECF4u);
    DrawText(text, x + 19, y + 3, COL_TEXT, COL_SHADOW);
}

void PortUiDraw(uint32_t* frame, int width, int height) {
    int l = Lang();
    int rowH = 11, panelW, panelH, px, py, rows, i, y, labelW = 0, valueW = 0;
    char value[64];

    sFb = frame;
    sFbW = width;
    sFbH = height;

    if (sNoticeFrames > 0) {
        int w = TextWidth(sNotice) + 12;
        sNoticeFrames--;
        Shade(4, 4, w, 14, COL_PANEL, 200);
        Rect(4, 4, w, 14, COL_BORDER);
        DrawText(sNotice, 10, 6, COL_TEXT, COL_SHADOW);
    }
    if (!sOpen) {
        if (sSkipPrompt)
            DrawSkipPrompt(l);
        if (sTitleHintFrames > 0) {
            const char* hint = sTitleHint[l];
            int gx = gPortScreen.originX + GBA_SCREEN_W - TextWidth(hint) - 6;
            int gy = gPortScreen.originY + 4;
            sTitleHintFrames--;
            DrawText(hint, gx, gy, 0xFF404858u, 0xFFC8D0E0u);
        }
        return;
    }

    for (i = 0; i < ITEM_COUNT; i++) {
        int w = TextWidth(sLabels[i][l]);
        labelW = SDL_max(labelW, w);
        ValueText(i, value, sizeof(value));
        valueW = SDL_max(valueW, TextWidth(value));
    }
    valueW = SDL_max(valueW, 92);
    panelW = SDL_min(width - 8, labelW + valueW + 52);
    rows = SDL_min(ITEM_COUNT, (height - 44) / rowH);
    panelH = rows * rowH + 38;
    px = (width - panelW) / 2;
    py = (height - panelH) / 2;

    if (sCursor < sScroll)
        sScroll = sCursor;
    if (sCursor >= sScroll + rows)
        sScroll = sCursor - rows + 1;

    Shade(0, 0, width, height, 0xFF000010u, 120);
    Shade(px, py, panelW, panelH, COL_PANEL, 220);
    Rect(px, py, panelW, panelH, COL_BORDER);
    Rect(px + 2, py + 2, panelW - 4, panelH - 4, 0xFF505878u);

    DrawText(sTitle[l], px + (panelW - TextWidth(sTitle[l])) / 2, py + 5, COL_TITLE, 0xFF604010u);
    for (i = 8; i < panelW - 8; i++)
        Pixel(px + i, py + 17, Mix(COL_BORDER, COL_PANEL, i < 24 ? (24 - i) * 10 : (i > panelW - 24 ? (i - (panelW - 24)) * 10 : 0)));

    y = py + 21;
    for (i = sScroll; i < sScroll + rows; i++, y += rowH) {
        int on = ItemEnabled(i);
        uint32_t face = on ? COL_TEXT : COL_DIM;

        if (i == sCursor)
            Shade(px + 4, y - 1, panelW - 8, rowH, COL_SELECT, 200);
        if (i == ITEM_CLOSE) {
            DrawText(sLabels[i][l], px + (panelW - TextWidth(sLabels[i][l])) / 2, y, face, COL_SHADOW);
            continue;
        }
        DrawText(sLabels[i][l], px + 10, y, face, COL_SHADOW);
        ValueText(i, value, sizeof(value));
        if (i == sCursor && on) {
            int vw = TextWidth(value);
            int vx = px + panelW - 20 - vw;
            DrawText("\xE2\x86\x90", vx - 10, y, COL_TITLE, COL_SHADOW);
            DrawText(value, vx, y, COL_VALUE, COL_SHADOW);
            DrawText("\xE2\x86\x92", vx + vw + 2, y, COL_TITLE, COL_SHADOW);
        } else {
            DrawText(value, px + panelW - 20 - TextWidth(value), y, on ? COL_VALUE : COL_DIM, COL_SHADOW);
        }
    }
    if (sScroll > 0)
        DrawText("\xE2\x86\x91", px + panelW - 12, py + 6, COL_TITLE, COL_SHADOW);
    if (sScroll + rows < ITEM_COUNT)
        DrawText("\xE2\x86\x93", px + panelW - 12, py + panelH - 13, COL_TITLE, COL_SHADOW);
    DrawText(sHelp[l], px + (panelW - TextWidth(sHelp[l])) / 2, py + panelH - 13, COL_DIM, COL_SHADOW);
}
