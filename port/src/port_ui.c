/*
 * In-game options overlay (graphics, display and sound settings).
 *
 * Drawn into the frame at the internal resolution with the game's own Latin
 * message font, so it matches the pixel art. Opened from the title screen
 * (SELECT), with F1 / Esc on the keyboard, or Start + Select (keyboard or
 * gamepad, as bound) / Guide anywhere. The game is paused while it is open.
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
    ITEM_ASPECT,
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
    ITEM_CARDS,
    ITEM_FPS,
    ITEM_DODGE,
    ITEM_STOCK,
    ITEM_UNSTOCK,
    ITEM_SAVEBANK,
    ITEM_MUSIC,
    ITEM_SFX,
    ITEM_MASTER,
    ITEM_EXPERIMENTAL, /* section heading, not selectable */
    ITEM_HDTEXT,
    ITEM_HDHUD,
    ITEM_HD2D,
    ITEM_SMOOTHAUDIO,
    ITEM_PS2_MODE,
    ITEM_PS2_MENU,     /* opens the Re:CoM page */
    ITEM_CLOSE,
    /* the Re:Chain of Memories page */
    ITEM_PS2_DIFFICULTY,
    ITEM_PS2_FASTTEXT,
    ITEM_PS2_BACK,
    /* the new game page: how to start it */
    ITEM_NG_VERSION,
    ITEM_NG_DIFFICULTY,
    ITEM_NG_HDTEXT,
    ITEM_NG_QOL,
    ITEM_NG_START,
    ITEM_COUNT
};

/* Two pages, each a run of items: the options and the Re:CoM settings. */
enum { PAGE_MAIN, PAGE_PS2, PAGE_NEWGAME };
static int sPage;
static int sNgRecom; /* new game page: the Re:CoM version chosen */
static int sNgQol;   /* the one-button controls as set before the new game page */
enum { WIZ_VERSION, WIZ_DIFFICULTY, WIZ_OPTIONS };
static int sWizStep, sWizCursor; /* the new game window's step and row */
static void WizKey(int key);
static int PageFirst(void) {
    return sPage == PAGE_PS2 ? ITEM_PS2_DIFFICULTY : sPage == PAGE_NEWGAME ? ITEM_NG_VERSION : 0;
}
static int PageEnd(void) {
    return sPage == PAGE_PS2 ? ITEM_PS2_BACK + 1 : sPage == PAGE_NEWGAME ? ITEM_COUNT : ITEM_CLOSE + 1;
}
/* rows shown only where they apply */
static int ItemVisible(int item) {
    switch (item) {
        /* the version and its difficulty are chosen at New Game and stay
           with the save, as in the original: no switch for them here */
        case ITEM_PS2_MODE:
        case ITEM_PS2_MENU: return 0;
        /* settings of our own, not the player's to tune */
        case ITEM_FILTER:
        case ITEM_CARDS:
        case ITEM_SAVEBANK: return 0;
        case ITEM_NG_VERSION: return 1;
        case ITEM_NG_DIFFICULTY: return sNgRecom;
        case ITEM_HDHUD: return 0; /* HD HUD pending: hidden and forced off (KHCOM_HD_HUD_DEV=1 for development) */
#ifndef PORT_HD2D
        case ITEM_HD2D: return 0; /* HD-2D pending: hidden and forced off (KHCOM_HD2D_DEV=1 for development) */
#endif
        default: return 1;
    }
}

/* Strings: UTF-8, one row per item, columns in LANG_* order. */
static const char* const sLabels[ITEM_COUNT][LANG_COUNT] = {
    { "View", "Vue", "Ansicht", "Visuale", "Vista", "表示" },
    { "Aspect ratio", "Format d'image", "Seitenverhältnis", "Proporzioni", "Relación de aspecto", "アスペクト比" },
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
    { "Battle cards raised", "Cartes de combat relevées", "Kampfkarten angehoben", "Carte di battaglia rialzate",
      "Subir cartas en combate", "カードを上げる" },
    { "Show FPS", "Afficher FPS", "FPS anzeigen", "Mostra FPS", "Mostrar FPS", "FPS表示" },
    { "One-button dodge roll", "Roulade en un bouton", "Ausweichrolle mit einer Taste", "Capriola con un tasto",
      "Voltereta con un botón", "ワンボタンドッジ" },
    { "One-button stock (L+R)", "Réserve en un bouton (L+R)", "Vorrat mit einer Taste (L+R)",
      "Accumulo con un tasto (L+R)", "Acumular con un botón (L+R)", "ワンボタンストック (L+R)" },
    { "Return stocked cards", "Rendre les cartes en réserve", "Vorratskarten zurücknehmen",
      "Riprendi carte accumulate", "Devolver cartas acumuladas", "ストックを手札に戻す" },
    { "Save bank", "Banque de sauvegarde", "Speicherbank", "Banco di salvataggio", "Banco de partidas",
      "セーブバンク" },
    { "Music volume", "Volume musique", "Musiklautstärke", "Volume musica", "Volumen música", "音楽の音量" },
    { "Effects volume", "Volume effets", "Effektlautstärke", "Volume effetti", "Volumen efectos", "効果音の音量" },
    { "Master volume", "Volume général", "Gesamtlautstärke", "Volume generale", "Volumen general", "全体の音量" },
    { "Experimental", "Expérimental", "Experimentell", "Sperimentale", "Experimentales", "実験的機能" },
    { "HD text", "Texte HD", "HD-Text", "Testo HD", "Texto HD", "HDテキスト" },
    { "HD HUD (pilot)", "HUD HD (pilote)", "HD-HUD (Pilot)", "HUD HD (pilota)", "HUD HD (piloto)", "HD HUD (試験)" },
    { "HD-2D look", "Style HD-2D", "HD-2D-Optik", "Stile HD-2D", "Estilo HD-2D", "HD-2D表示" },
    { "Audio filter", "Filtre audio", "Audiofilter", "Filtro audio", "Filtro de audio", "オーディオフィルター" },
    { "Re:CoM mode", "Mode Re:CoM", "Re:CoM-Modus", "Modalità Re:CoM", "Modo Re:CoM", "Re:CoMモード" },
    { "Re:CoM settings", "Réglages Re:CoM", "Re:CoM-Einstellungen", "Impostazioni Re:CoM", "Ajustes Re:CoM",
      "Re:CoM設定" },
    { "Back", "Retour", "Zurück", "Indietro", "Volver", "戻る" },
    { "Difficulty", "Difficulté", "Schwierigkeit", "Difficoltà", "Dificultad", "難易度" },
    { "Fast dialogue", "Dialogues rapides", "Schnelle Dialoge", "Dialoghi veloci", "Diálogos rápidos",
      "会話を早く" },
    { "Back", "Retour", "Zurück", "Indietro", "Volver", "戻る" },
    { "Version", "Version", "Version", "Versione", "Versión", "バージョン" },
    { "Difficulty", "Difficulté", "Schwierigkeit", "Difficoltà", "Dificultad", "難易度" },
    { "HD text", "Texte HD", "HD-Text", "Testo HD", "Texto HD", "HDテキスト" },
    { "One-button dodge and stock", "Roulade et réserve en un bouton", "Ausweichen und Vorrat mit einer Taste",
      "Capriola e accumulo con un tasto", "Rodar y acumular con un botón", "ワンボタン操作" },
    { "Start", "Commencer", "Beginnen", "Inizia", "Comenzar", "はじめる" },
};
static const char* const sNgTitle[LANG_COUNT] = { "New game", "Nouvelle partie", "Neues Spiel", "Nuova partita",
    "Nueva partida", "ニューゲーム" };
static const char* const sVersions[2][LANG_COUNT] = {
    { "Original", "Originale", "Original", "Originale", "Original", "オリジナル" },
    { "Re:Chain of Memories", "Re:Chain of Memories", "Re:Chain of Memories", "Re:Chain of Memories",
      "Re:Chain of Memories", "Re:チェイン オブ メモリーズ" },
};
/* New game page: what the selected row means, under the list. */
static const char* const sNgDescOriginal[LANG_COUNT] = {
    "The original Game Boy Advance version, exactly as on the cartridge.",
    "La version Game Boy Advance originale, exactement comme sur la cartouche.",
    "Die originale Game-Boy-Advance-Version, genau wie auf dem Modul.",
    "La versione originale per Game Boy Advance, esattamente come sulla cartuccia.",
    "La versión original de Game Boy Advance, exactamente como en el cartucho.",
    "ゲームボーイアドバンス版そのままのオリジナル。",
};
static const char* const sNgDescRecom[LANG_COUNT] = {
    "With improvements from Re:Chain of Memories (PS2): difficulty levels and faster dialogue.",
    "Avec les améliorations de Re:Chain of Memories (PS2) : niveaux de difficulté et dialogues rapides.",
    "Mit Verbesserungen aus Re:Chain of Memories (PS2): Schwierigkeitsgrade und schnellere Dialoge.",
    "Con i miglioramenti di Re:Chain of Memories (PS2): livelli di difficoltà e dialoghi più rapidi.",
    "Con mejoras de Re:Chain of Memories (PS2): niveles de dificultad y diálogos más rápidos.",
    "Re:チェイン オブ メモリーズ (PS2) の改良入り：難易度の選択と会話の高速化。",
};
static const char* const sNgDescDifficulty[3][LANG_COUNT] = {
    { "As the original: the cartridge's damage.", "Comme l'original : les dégâts de la cartouche.",
      "Wie das Original: der Schaden des Moduls.", "Come l'originale: i danni della cartuccia.",
      "Como el original: el daño del cartucho.", "オリジナルと同じダメージ。" },
    { "Sora takes half damage and deals a quarter more.", "Sora subit moitié moins de dégâts et en inflige un quart de plus.",
      "Sora erleidet halben Schaden und teilt ein Viertel mehr aus.", "Sora subisce metà danni e ne infligge un quarto in più.",
      "Sora recibe la mitad de daño y causa un cuarto más.", "ソラの被ダメージ半分、与ダメージ1.25倍。" },
    { "Sora takes half again as much damage and deals less.", "Sora subit moitié plus de dégâts et en inflige moins.",
      "Sora erleidet anderthalbfachen Schaden und teilt weniger aus.", "Sora subisce una volta e mezza i danni e ne infligge meno.",
      "Sora recibe un 50 % más de daño y causa menos.", "ソラの被ダメージ1.5倍、与ダメージ減少。" },
};
static const char* const sNgDescHd[LANG_COUNT] = {
    "Dialogue drawn with a crisp font at the window's resolution.",
    "Dialogues tracés avec une police nette à la résolution de la fenêtre.",
    "Dialoge in einer scharfen Schrift in Fensterauflösung.",
    "Dialoghi disegnati con un carattere nitido alla risoluzione della finestra.",
    "Diálogos con una fuente nítida a la resolución de la ventana.",
    "会話をウィンドウ解像度のくっきりしたフォントで表示。",
};
static const char* const sNgDescQol[LANG_COUNT] = {
    "One button for the dodge roll and for stocking cards (L+R); hold it to return stocked cards.",
    "Un bouton pour la roulade et pour la réserve (L+R) ; maintenu, il rend les cartes en réserve.",
    "Eine Taste für die Ausweichrolle und den Vorrat (L+R); gehalten nimmt sie Vorratskarten zurück.",
    "Un tasto per la capriola e per l'accumulo (L+R); tenuto premuto riprende le carte accumulate.",
    "Un botón para rodar y para acumular cartas (L+R); mantenido devuelve las cartas acumuladas.",
    "回避とストック (L+R) をワンボタンで。長押しでストックを手札に戻す。",
};
static const char* const sNgDescStart[LANG_COUNT] = {
    "Everything here can be changed later with START + SELECT.",
    "Tout peut être changé plus tard avec START + SELECT.",
    "Alles lässt sich später mit START + SELECT ändern.",
    "Tutto si può cambiare in seguito con START + SELECT.",
    "Todo se puede cambiar después con START + SELECT.",
    "あとから START + SELECT で変更できます。",
};

static const char* NewGameDescription(int item, int l) {
    switch (item) {
        case ITEM_NG_VERSION: return sNgRecom ? sNgDescRecom[l] : sNgDescOriginal[l];
        case ITEM_NG_DIFFICULTY: return sNgDescDifficulty[SDL_clamp(gPortConfig.ps2Difficulty, 0, 2)][l];
        case ITEM_NG_HDTEXT: return sNgDescHd[l];
        case ITEM_NG_QOL: return sNgDescQol[l];
        default: return sNgDescStart[l];
    }
}

static const char* const sPs2Title[LANG_COUNT] = { "Re:Chain of Memories", "Re:Chain of Memories",
    "Re:Chain of Memories", "Re:Chain of Memories", "Re:Chain of Memories", "Re:チェイン オブ メモリーズ" };

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
static const char* const sHelpNg[LANG_COUNT] = {
    "ââ Select   ââ Change   A Start",
    "ââ Choisir   ââ Modifier   A Commencer",
    "ââ Wählen   ââ Ändern   A Beginnen",
    "ââ Scegli   ââ Cambia   A Inizia",
    "ââ Elegir   ââ Cambiar   A Comenzar",
    "ââ 選択   ââ 変更   A はじめる",
};
static const char* const sTitleHint[LANG_COUNT] = { "SELECT  Options", "SELECT  Options", "SELECT  Optionen",
                                                    "SELECT  Opzioni", "SELECT  Opciones",
    "SELECT  オプション",
};
static const char* const sDifficulty[3][LANG_COUNT] = {
    { "Standard", "Standard", "Standard", "Standard", "Estándar", "スタンダード" },
    { "Beginner", "Débutant", "Anfänger", "Principiante", "Principiante", "ビギナー" },
    { "Proud", "Fier", "Stolz", "Orgoglioso", "Orgulloso", "プラウド" },
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
static const char* const sZooms[PORT_ZOOM_COUNT][LANG_COUNT] = {
    /* the closest step shows the GBA's own 160 lines at full height, as an
       emulator stretched to the screen: the original view */
    { "Original (GBA)", "Original (GBA)", "Original (GBA)", "Originale (GBA)", "Original (GBA)", "オリジナル (GBA)" },
    { "Normal", "Normal", "Normal", "Normale", "Normal", "ふつう" },
    { "Far", "Loin", "Weit", "Lontano", "Lejos", "遠い" },
    { "Very far", "Très loin", "Sehr weit", "Molto lontano", "Muy lejos", "とても遠い" },
    { "Farthest", "Le plus loin", "Am weitesten", "Lontanissimo", "Lejísimos", "いちばん遠い" },
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
static const char* const sAspect[3][LANG_COUNT] = {
    { "Fill window", "Remplir la fenêtre", "Fenster füllen", "Riempi finestra", "Llenar ventana", "ウィンドウに合わせる" },
    { "16:9", "16:9", "16:9", "16:9", "16:9", "16:9" },
    { "21:9", "21:9", "21:9", "21:9", "21:9", "21:9" },
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
        case ITEM_PS2_MENU:
            return gPortConfig.ps2Mode != 0;
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
            if (ItemEnabled(item)) /* the step and the pixel scale it gives here */
                SDL_snprintf(out, size, "%s  (pixel x%d)", sZooms[PortConfigZoomEffective(gPortConfig.zoom)][l],
                             PortConfigZoomScale(gPortConfig.zoom));
            else
                SDL_strlcpy(out, sAutoOnly[l], size);
            break;
        case ITEM_SCALE:
            SDL_strlcpy(out, sScales[gPortConfig.scaleMode][l], size);
            break;
        case ITEM_HUD:
            SDL_strlcpy(out, sHudSizes[SDL_clamp(gPortConfig.hudSize, 0, 2)][l], size);
            break;
        case ITEM_ASPECT:
            SDL_strlcpy(out, sAspect[SDL_clamp(gPortConfig.aspect, 0, 2)][l], size);
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
        case ITEM_UNSTOCK:
            SDL_strlcpy(out, sOnOff[gPortConfig.unstockButton != 0][l], size);
            break;
        case ITEM_SAVEBANK:
            SDL_snprintf(out, size, "%d / %d", gPortConfig.saveBank, PORT_SAVE_BANKS);
            break;
        case ITEM_HD2D:
            SDL_strlcpy(out, sOnOff[gPortConfig.hd2d != 0][l], size);
            break;
        case ITEM_HDHUD:
            SDL_strlcpy(out, sOnOff[gPortConfig.hdHud != 0][l], size);
            break;
        case ITEM_SMOOTHAUDIO:
            SDL_strlcpy(out, sOnOff[gPortConfig.smoothAudio != 0][l], size);
            break;
        case ITEM_PS2_MODE:
            SDL_strlcpy(out, sOnOff[gPortConfig.ps2Mode != 0][l], size);
            break;
        case ITEM_PS2_MENU:
            SDL_strlcpy(out, "â", size);
            break;
        case ITEM_PS2_DIFFICULTY:
        case ITEM_NG_DIFFICULTY:
            SDL_strlcpy(out, sDifficulty[SDL_clamp(gPortConfig.ps2Difficulty, 0, 2)][l], size);
            break;
        case ITEM_NG_VERSION:
            SDL_strlcpy(out, sVersions[sNgRecom != 0][l], size);
            break;
        case ITEM_NG_HDTEXT:
            SDL_strlcpy(out, sOnOff[gPortConfig.hdText != 0][l], size);
            break;
        case ITEM_NG_QOL:
            SDL_strlcpy(out, sOnOff[PortQolButtons() != 0][l], size);
            break;
        case ITEM_PS2_FASTTEXT:
            SDL_strlcpy(out, sOnOff[gPortConfig.ps2FastText != 0][l], size);
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

static void Close(void);

static void Change(int item, int dir) {
    int i;

    if (!ItemEnabled(item))
        return;
    switch (item) {
        case ITEM_VIEW:
            gPortConfig.viewMode = sViewOrder[Wrap(ViewIndex() + dir, PORT_VIEW_COUNT)];
            break;
        case ITEM_ZOOM:
            /* the next step that gives a scale of its own at this size */
            i = PortConfigZoomEffective(gPortConfig.zoom) + dir;
            while (i >= 0 && i < PORT_ZOOM_COUNT && !PortConfigZoomValid(i))
                i += dir;
            if (i >= 0 && i < PORT_ZOOM_COUNT)
                gPortConfig.zoom = i;
            break;
        case ITEM_SCALE:
            gPortConfig.scaleMode = Wrap(gPortConfig.scaleMode + dir, PORT_SCALE_COUNT);
            break;
        case ITEM_HUD:
            gPortConfig.hudSize = Wrap(gPortConfig.hudSize + dir, 3);
            break;
        case ITEM_ASPECT:
            gPortConfig.aspect = Wrap(gPortConfig.aspect + dir, 3);
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
        case ITEM_UNSTOCK:
            gPortConfig.unstockButton = !gPortConfig.unstockButton;
            break;
        case ITEM_SAVEBANK:
            gPortConfig.saveBank = Wrap(gPortConfig.saveBank - 1 + dir, PORT_SAVE_BANKS) + 1;
            PortSramSwitchBank();
            break;
        case ITEM_HD2D:
            gPortConfig.hd2d = !gPortConfig.hd2d;
            break;
        case ITEM_HDHUD:
            gPortConfig.hdHud = !gPortConfig.hdHud;
            break;
        case ITEM_SMOOTHAUDIO:
            gPortConfig.smoothAudio = !gPortConfig.smoothAudio;
            break;
        case ITEM_PS2_MODE:
            gPortConfig.ps2Mode = !gPortConfig.ps2Mode;
            break;
        case ITEM_PS2_MENU:
            if (dir > 0 && gPortConfig.ps2Mode) {
                sPage = PAGE_PS2;
                sCursor = ITEM_PS2_DIFFICULTY;
                sScroll = ITEM_PS2_DIFFICULTY;
            }
            break;
        case ITEM_PS2_BACK:
            sPage = PAGE_MAIN;
            sCursor = ITEM_PS2_MENU;
            break;
        case ITEM_NG_VERSION:
            /* each version comes with its presets: the original as on the
               cartridge (the player's controls), Re:CoM with HD text and the
               one-button controls */
            sNgRecom = !sNgRecom;
            gPortConfig.hdText = sNgRecom;
            PortSetQolButtons(sNgRecom || sNgQol);
            break;
        case ITEM_NG_HDTEXT:
            gPortConfig.hdText = !gPortConfig.hdText;
            break;
        case ITEM_NG_QOL:
            PortSetQolButtons(!PortQolButtons());
            break;
        case ITEM_NG_START:
            PortRecomSetActive(sNgRecom);
            Close();
            break;
        case ITEM_NG_DIFFICULTY:
        case ITEM_PS2_DIFFICULTY:
            gPortConfig.ps2Difficulty = Wrap(gPortConfig.ps2Difficulty + dir, 3);
            break;
        case ITEM_PS2_FASTTEXT:
            gPortConfig.ps2FastText = !gPortConfig.ps2FastText;
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

/* Fades: the picture goes to black, the menu comes in from black, and the
   reverse on closing (MENU_FADE frames each way). */
#define MENU_FADE 8
#define MENU_FADE_COLOR 0xFF000000u
static int sFade;    /* frames since the menu opened */
static int sClosing; /* frames left of the menu fading to black */
static int sReturn;  /* frames left of the black fading back to the game */
static int sHarnessTick;

void PortUiOpenOptions(void) {
    if (sOpen)
        return;
    sOpen = 1;
    sFade = SDL_getenv("KHCOM_OPTIONS_SHOT") ? MENU_FADE * 2 : 0; /* test shots: no fade */
    sClosing = 0;
    sReturn = 0;
    sCursor = 0;
    sScroll = 0;
    sPage = PAGE_MAIN;
    sHarnessTick = 0;
    if (SDL_getenv("KHCOM_OPTIONS_CURSOR")) /* test harness: shot of a lower row */
        sCursor = SDL_clamp(SDL_atoi(SDL_getenv("KHCOM_OPTIONS_CURSOR")), 0, ITEM_COUNT - 1);
    RefreshModes();
}

/* map.c: a new game was confirmed. Original preselected, with its presets
   (HD text and the one-button controls off, as the cartridge plays). */
void PortUiOpenNewGame(void) {
    /* unattended runs (KHCOM_FAST batteries, the bot, replays) start as the
       original unless a test drives this page */
    if ((SDL_getenv("KHCOM_FAST") || SDL_getenv("KHCOM_BOT")) && !SDL_getenv("KHCOM_OPTIONS_ACTIONS") && !SDL_getenv("KHCOM_WIZARD_KEYS")) {
        PortRecomSetActive(0);
        return;
    }
    PortUiOpenOptions();
    sPage = PAGE_NEWGAME;
    sNgRecom = 0;
    gPortConfig.hdText = 0;
    /* the one-button controls stay as the player set them (on by default,
       as they always were); only Re:CoM turns them on */
    sNgQol = PortQolButtons();
    /* the version is always the first question */
    sWizStep = WIZ_VERSION;
    sWizCursor = 0;
    PortVideoApplyConfig();
}

void PortUiForceClose(void) {
    sOpen = 0;
    sClosing = 0;
    sReturn = 0;
}

static void Close(void);

void PortUiHarnessToggle(void) {
    if (!sOpen)
        PortUiOpenOptions();
    else
        Close();
}

/* The game stays paused (sOpen) until the menu has faded out. */
static void Close(void) {
    if (sClosing > 0)
        return;
    sClosing = MENU_FADE;
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
        int first = PageFirst(), n = PageEnd() - first;
        sCursor = first + Wrap(sCursor - first + dir, n);
        if (sCursor == ITEM_EXPERIMENTAL)
            sCursor = first + Wrap(sCursor - first + dir, n);
    } while (!ItemEnabled(sCursor) || !ItemVisible(sCursor));
}

int PortUiHandleEvent(const void* p) {
    const SDL_Event* ev = (const SDL_Event*)p;

    if (sOpen && (sClosing > 0 || sFade < MENU_FADE)) /* fading: the menu takes no input */
        return ev->type == SDL_EVENT_KEY_DOWN || ev->type == SDL_EVENT_KEY_UP ||
               ev->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN || ev->type == SDL_EVENT_GAMEPAD_BUTTON_UP;
    if (!sOpen) {
        if (ev->type == SDL_EVENT_KEY_DOWN && !ev->key.repeat && (ev->key.key == SDLK_F1 || ev->key.key == SDLK_ESCAPE)) {
            PortUiOpenOptions();
            return 1;
        }
        /* Start + Select anywhere (the bound buttons and keys): the other one
           is read as it is now, so a release the overlay swallowed never
           leaves a half combo behind. */
        if (ev->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
            SDL_Gamepad* pad = SDL_GetGamepadFromID(ev->gbutton.which);
            int start = gPortConfig.pad[PORT_ACT_START], select = gPortConfig.pad[PORT_ACT_SELECT];
            int b = ev->gbutton.button;
            if (b == SDL_GAMEPAD_BUTTON_GUIDE
                || (pad != NULL && b == start && select >= 0 && SDL_GetGamepadButton(pad, (SDL_GamepadButton)select))
                || (pad != NULL && b == select && start >= 0 && SDL_GetGamepadButton(pad, (SDL_GamepadButton)start))) {
                PortUiOpenOptions();
                return 1;
            }
        }
        if (ev->type == SDL_EVENT_KEY_DOWN && !ev->key.repeat) {
            const bool* held = SDL_GetKeyboardState(NULL);
            int i, isStart = 0, isSelect = 0, startHeld = 0, selectHeld = 0;
            for (i = 0; i < 2; i++) {
                SDL_Keycode ks = (SDL_Keycode)gPortConfig.keys[PORT_ACT_START][i];
                SDL_Keycode kl = (SDL_Keycode)gPortConfig.keys[PORT_ACT_SELECT][i];
                if (ks != 0) {
                    isStart |= ev->key.key == ks;
                    startHeld |= held[SDL_GetScancodeFromKey(ks, NULL)];
                }
                if (kl != 0) {
                    isSelect |= ev->key.key == kl;
                    selectHeld |= held[SDL_GetScancodeFromKey(kl, NULL)];
                }
            }
            if ((isStart && selectHeld) || (isSelect && startHeld)) {
                PortUiOpenOptions();
                return 1;
            }
        }
        return 0;
    }

    if (sPage == PAGE_NEWGAME && (ev->type == SDL_EVENT_KEY_DOWN || ev->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN)) {
        int key = -1;
        if (ev->type == SDL_EVENT_KEY_DOWN) {
            switch (ev->key.key) {
                case SDLK_UP: case SDLK_W: key = 0; break;
                case SDLK_DOWN: case SDLK_S: key = 1; break;
                case SDLK_LEFT: case SDLK_RIGHT: case SDLK_A: case SDLK_D: key = 2; break;
                case SDLK_RETURN: case SDLK_SPACE: case SDLK_X: case SDLK_K: key = 3; break;
                case SDLK_ESCAPE: case SDLK_BACKSPACE: case SDLK_Z: case SDLK_J: key = ev->key.repeat ? -1 : 4; break;
            }
        } else {
            switch (ev->gbutton.button) {
                case SDL_GAMEPAD_BUTTON_DPAD_UP: key = 0; break;
                case SDL_GAMEPAD_BUTTON_DPAD_DOWN: key = 1; break;
                case SDL_GAMEPAD_BUTTON_DPAD_LEFT: case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: key = 2; break;
                case SDL_GAMEPAD_BUTTON_SOUTH: case SDL_GAMEPAD_BUTTON_START: key = 3; break;
                case SDL_GAMEPAD_BUTTON_EAST: key = 4; break;
            }
        }
        if (key >= 0)
            WizKey(key);
        return 1;
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
                if (!ev->key.repeat) {
                    if (sPage == PAGE_NEWGAME)
                        Change(ITEM_NG_START, 1);
                    else if (sPage == PAGE_PS2)
                        Change(ITEM_PS2_BACK, 1);
                    else
                        Close();
                }
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
                if (sPage == PAGE_NEWGAME) {
                    Change(ITEM_NG_START, 1);
                    break;
                }
                if (sPage == PAGE_PS2) {
                    Change(ITEM_PS2_BACK, 1);
                    break;
                }
                Close();
                break;
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

static void MenuButton(int x, int y, const char* text, int key);

/* Bottom right of the GBA picture: a debug-menu button with the text. */
static void DrawSkipPrompt(int l) {
    const char* text = sSkipText[l];
    int x = gPortScreen.originX + GBA_SCREEN_W - TextWidth(text) - 38;
    int y = gPortScreen.originY + GBA_SCREEN_H - 15;

    MenuButton(x, y, text, 'A');
}

static int sSkipOwnLayer;

void PortUiSkipOwnLayer(int on) {
    sSkipOwnLayer = on;
}

int PortUiDrawSkipLayer(uint32_t* frame, int width, int height, int rect[4]) {
    const char* text;
    int l = Lang();

    if (!sSkipPrompt || sOpen)
        return 0;
    text = sSkipText[l];
    sFb = frame;
    sFbW = width;
    sFbH = height;
    DrawSkipPrompt(l);
    /* MenuButton's shade: from x - 3, y - 2, the text and the A key */
    rect[0] = gPortScreen.originX + GBA_SCREEN_W - TextWidth(text) - 41;
    rect[1] = gPortScreen.originY + GBA_SCREEN_H - 17;
    rect[2] = TextWidth(text) + 26 + GlyphWidth('A') + 4;
    rect[3] = 14;
    return 1;
}

/* The state / slot notice, on a layer of its own above everything else (the
   presenter draws it last, at the window's scale): like the app's own HUD.
   Returns the width in GBA pixels, 0 when there is no notice. */
static char sFpsText[48];

void PortUiSetFps(float fps, float logic) {
    SDL_snprintf(sFpsText, sizeof(sFpsText), "%.1f FPS   %.1f Hz", fps, logic);
}

int PortUiDrawNotice(uint32_t* frame, int width, int height) {
    const char* text = sNotice;

    if (sNoticeFrames > 0)
        sNoticeFrames--;
    else if (gPortConfig.showFps)
        text = sFpsText;
    else
        return 0;
    sFb = frame;
    sFbW = width;
    sFbH = height;
    MenuButton(3, 2, text, 0);
    return SDL_min(TextWidth(text) + 26, width);
}

/* ---- the options screen, in the style of the cartridge's debug menu --------
   White, a list of bars on the left (a grey square and an underline; the
   current one black with a red square), and the debug menu's own picture of
   Sora (gDebugMenuBg*, decoded once from the ROM) at the bottom right. */

extern uint8_t gDebugMenuBgTiles[];
extern uint16_t gDebugMenuBgMap[];
extern uint16_t gDebugMenuBgPalette[];
void LZ77UnCompWram(const void* src, void* dst);

#define MENU_ART_X 114 /* the picture starts right of the debug menu's bars */
#define MENU_INK 0xFF080808u
#define MENU_GREY 0xFFB5ADADu
#define MENU_SHADE 0xFFB5B5B5u
#define MENU_RED 0xFFFF1000u
#define MENU_RED_DARK 0xFF9C1000u
#define MENU_DIM 0xFF9C9494u
/* Japanese glyphs fill the whole 16-pixel cell (Latin ones 10 rows): the
   title and the list start a little lower. */
#ifdef VERSION_JP
#define MENU_TITLE_Y 5
#define MENU_TOP 18
#else
#define MENU_TITLE_Y 2
#define MENU_TOP 14
#endif

static uint32_t sMenuArt[GBA_SCREEN_W * GBA_SCREEN_H];
static int sMenuArtReady;

static void DecodeMenuArt(void) {
    static uint8_t tiles[32768];
    static uint16_t map[1024];
    int x, y;

#ifdef VERSION_EU
    LZ77UnCompWram(gDebugMenuBgTiles, tiles);
    LZ77UnCompWram(gDebugMenuBgMap, map);
#else
    memcpy(tiles, gDebugMenuBgTiles, 23360);
    memcpy(map, gDebugMenuBgMap, sizeof(map));
#endif
    for (y = 0; y < GBA_SCREEN_H; y++) {
        for (x = 0; x < GBA_SCREEN_W; x++) {
            uint16_t e = map[(y >> 3) * 32 + (x >> 3)];
            int tx = (e & 0x400) ? 7 - (x & 7) : (x & 7);
            int ty = (e & 0x800) ? 7 - (y & 7) : (y & 7);
            uint8_t v = tiles[(e & 0x3FF) * 64 + ty * 8 + tx];
            uint16_t c = gDebugMenuBgPalette[v];
            uint32_t r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
            sMenuArt[y * GBA_SCREEN_W + x] = v == 0 ? 0xFFFFFFFFu
                : 0xFF000000u | ((r << 3 | r >> 2) << 16) | ((g << 3 | g >> 2) << 8) | (b << 3 | b >> 2);
        }
    }
    sMenuArtReady = 1;
}

static void Fill(int x0, int y0, int w, int h, uint32_t c) {
    int x, y;

    for (y = y0; y < y0 + h; y++)
        for (x = x0; x < x0 + w; x++)
            Pixel(x, y, c);
}

/* One row: t is the top of its square, the bar runs from x to x + w. */
static void MenuBar(int x, int t, int w, int selected) {
    if (!selected) {
        Fill(x, t, 10, 10, MENU_GREY);
        Fill(x, t + 8, w, 2, MENU_GREY);
        return;
    }
    Fill(x, t, 11, 10, MENU_INK);
    Fill(x + 11, t, 1, 10, MENU_SHADE);
    Fill(x + 3, t + 3, 5, 5, MENU_RED);
    Fill(x + 3, t + 3, 1, 5, MENU_RED_DARK);
    Fill(x + 3, t + 3, 5, 1, MENU_RED_DARK);
    Fill(x, t + 8, w, 2, MENU_INK);
    Fill(x + 1, t + 10, w, 1, MENU_SHADE);
}

/* One selected row on its own (the skip prompt, notices, hints): the bar
   under the text on a white plate, as the debug menu shows it. */
static void MenuButton(int x, int y, const char* text, int key) {
    int keyW = key ? GlyphWidth(key) + 4 : 0;
    int w = TextWidth(text) + 20 + keyW;

    Shade(x - 3, y - 2, w + 6, 14, 0xFFFFFFFFu, 224);
    MenuBar(x, y, w, 1);
    if (key)
        DrawGlyph(key, x + 15, y - 2, MENU_RED_DARK, 0xFFD8D8D8u);
    DrawText(text, x + 16 + keyW, y - 2, MENU_INK, 0xFFD8D8D8u);
}

/* A value without its explanation in brackets, for the narrow list. */
static void ShortValue(int item, char* out, size_t size) {
    char* paren;

    ValueText(item, out, size);
    paren = SDL_strstr(out, " (");
    if (paren != NULL && paren != out)
        *paren = 0;
}

/* ---- the new game window: a few steps, as modern games ask on first start - */

static const char* const sWizAsk[3][LANG_COUNT] = {
    { "Which version do you want to play?", "Quelle version veux-tu jouer ?", "Welche Version möchtest du spielen?",
      "Quale versione vuoi giocare?", "¿Qué versión quieres jugar?", "どのバージョンで遊びますか？" },
    { "Difficulty", "Difficulté", "Schwierigkeit", "Difficoltà", "Dificultad", "難易度" },
    { "Before you start", "Avant de commencer", "Bevor es losgeht", "Prima di iniziare", "Antes de empezar",
      "はじめる前に" },
};
static const char* const sWizConfirm[LANG_COUNT] = { "Confirm", "Confirmer", "Bestätigen", "Conferma", "Confirmar",
    "決定" };
static const char* const sWizHelp[LANG_COUNT] = {
    "\xE2\x86\x91\xE2\x86\x93 Select   A OK   B Back",
    "\xE2\x86\x91\xE2\x86\x93 Choisir   A OK   B Retour",
    "\xE2\x86\x91\xE2\x86\x93 Wählen   A OK   B Zurück",
    "\xE2\x86\x91\xE2\x86\x93 Scegli   A OK   B Indietro",
    "\xE2\x86\x91\xE2\x86\x93 Elegir   A Aceptar   B Atrás",
    "\xE2\x86\x91\xE2\x86\x93 選択   A 決定   B 戻る",
};

/* The first question has nothing before it: no "B Back". */
static const char* const sWizHelpFirst[LANG_COUNT] = {
    "\xE2\x86\x91\xE2\x86\x93 Select   A OK",
    "\xE2\x86\x91\xE2\x86\x93 Choisir   A OK",
    "\xE2\x86\x91\xE2\x86\x93 Wählen   A OK",
    "\xE2\x86\x91\xE2\x86\x93 Scegli   A OK",
    "\xE2\x86\x91\xE2\x86\x93 Elegir   A Aceptar",
    "\xE2\x86\x91\xE2\x86\x93 選択   A 決定",
};

static int WizRows(void) {
    return sWizStep == WIZ_VERSION ? 2 : sWizStep == WIZ_DIFFICULTY ? 3 : 3;
}

static void WizFinish(void) {
    PortRecomSetActive(sNgRecom);
    Close();
}

/* key: 0 up, 1 down, 2 left/right (toggle), 3 A, 4 B */
static void WizKey(int key) {
    int rows = WizRows();
    switch (key) {
        case 0: sWizCursor = (sWizCursor + rows - 1) % rows; return;
        case 1: sWizCursor = (sWizCursor + 1) % rows; return;
        case 4:
            if (sWizStep == WIZ_OPTIONS) {
                sWizStep = sNgRecom ? WIZ_DIFFICULTY : WIZ_VERSION;
                sWizCursor = sNgRecom ? (gPortConfig.ps2Difficulty == 1 ? 0 : gPortConfig.ps2Difficulty == 0 ? 1 : 2) : 0;
            } else if (sWizStep == WIZ_DIFFICULTY) {
                sWizStep = WIZ_VERSION;
                sWizCursor = 1;
            }
            return;
        default:
            break;
    }
    if (sWizStep == WIZ_VERSION && key == 3) {
        /* each version comes with its presets: the original as on the
           cartridge, Re:CoM with HD text and the one-button controls */
        sNgRecom = sWizCursor == 1;
        /* choosing Re:CoM here is what turns the Re:CoM rules on */
        if (sNgRecom)
            gPortConfig.ps2Mode = 1;
        gPortConfig.hdText = sNgRecom;
        PortSetQolButtons(sNgRecom || sNgQol);
        PortVideoApplyConfig();
        sWizStep = sNgRecom ? WIZ_DIFFICULTY : WIZ_OPTIONS;
        sWizCursor = sNgRecom ? 1 : 0; /* Standard first */
    } else if (sWizStep == WIZ_DIFFICULTY && key == 3) {
        static const int pick[3] = { 1, 0, 2 }; /* rows: Beginner, Standard, Proud */
        gPortConfig.ps2Difficulty = pick[sWizCursor];
        sWizStep = WIZ_OPTIONS;
        sWizCursor = 0;
    } else if (sWizStep == WIZ_OPTIONS) {
        if (sWizCursor == 0 && (key == 2 || key == 3)) {
            gPortConfig.hdText = !gPortConfig.hdText;
            PortVideoApplyConfig();
        } else if (sWizCursor == 1 && (key == 2 || key == 3)) {
            PortSetQolButtons(!PortQolButtons());
        } else if (sWizCursor == 2 && key == 3) {
            WizFinish();
        }
    }
}

/* Text wrapped to maxW; returns the lines drawn (draw = 0: only measures). */
static int DrawWrapped(const char* text, int x, int y, int maxW, uint32_t color, int draw) {
    char line[200];
    const char* p = text;
    int lines = 0;
    while (*p) {
        const char* best = NULL;
        const char* q = p;
        int n;
        while (1) {
            const char* e = q;
            while (*e && *e != ' ') e++;
            n = (int)(e - p);
            if (n >= (int)sizeof(line)) break;
            SDL_memcpy(line, p, (size_t)n);
            line[n] = 0;
            if (TextWidth(line) > maxW && best != NULL) break;
            best = e;
            if (!*e) break;
            q = e + 1;
        }
        if (best == NULL) best = p + SDL_strlen(p);
        n = SDL_min((int)(best - p), (int)sizeof(line) - 1);
        SDL_memcpy(line, p, (size_t)n);
        line[n] = 0;
        if (draw) DrawText(line, x, y + lines * 11, color, 0xFFD8D8D8u);
        lines++;
        p = *best ? best + 1 : best;
    }
    return lines;
}

static void DrawWizard(int l, int width, int height) {
    const char* labels[3];
    const char* subs[3];
    const char* values[3] = { NULL, NULL, NULL };
    int rows = WizRows(), i, panelW = SDL_min(width - 24, 250), x0, y0, h, y, textW;
    static const int order[3] = { 1, 0, 2 };

    if (sWizStep == WIZ_VERSION) {
        labels[0] = sVersions[0][l]; subs[0] = sNgDescOriginal[l];
        labels[1] = sVersions[1][l]; subs[1] = sNgDescRecom[l];
    } else if (sWizStep == WIZ_DIFFICULTY) {
        for (i = 0; i < 3; i++) {
            labels[i] = sDifficulty[order[i]][l];
            subs[i] = sNgDescDifficulty[order[i]][l];
        }
    } else {
        labels[0] = sLabels[ITEM_NG_HDTEXT][l]; subs[0] = sNgDescHd[l];
        values[0] = sOnOff[gPortConfig.hdText != 0][l];
        labels[1] = sLabels[ITEM_NG_QOL][l]; subs[1] = sNgDescQol[l];
        values[1] = sOnOff[PortQolButtons() != 0][l];
        labels[2] = sWizConfirm[l]; subs[2] = sNgDescStart[l];
    }
    textW = panelW - 30;
    /* measure, then a white window in the middle of a darkened screen */
    h = 22;
    for (i = 0; i < rows; i++)
        h += 14 + DrawWrapped(subs[i], 0, 0, textW, 0, 0) * 11 + 4;
    h += 14;
    x0 = (width - panelW) / 2;
    y0 = SDL_max(2, (height - h) / 2);
    Fill(0, 0, width, height, 0xFF000000u);
    Fill(x0 - 2, y0 - 2, panelW + 4, h + 4, MENU_GREY);
    Fill(x0, y0, panelW, h, 0xFFFFFFFFu);
    DrawText(sWizAsk[sWizStep][l], x0 + 8, y0 + 4, MENU_RED_DARK, 0xFFD8D8D8u);
    Fill(x0 + 8, y0 + 17, panelW - 16, 1, MENU_GREY);
    y = y0 + 22;
    for (i = 0; i < rows; i++) {
        int sel = i == sWizCursor;
        MenuBar(x0 + 8, y + 2, panelW - 16, sel);
        DrawText(labels[i], x0 + 24, y, MENU_INK, 0xFFD8D8D8u);
        if (values[i]) {
            int vx = x0 + panelW - 12 - TextWidth(values[i]);
            if (sel) {
                vx -= 8;
                DrawText("\xE2\x86\x90", vx - 10, y, MENU_RED_DARK, 0xFFD8D8D8u);
                DrawText("\xE2\x86\x92", vx + TextWidth(values[i]) + 2, y, MENU_RED_DARK, 0xFFD8D8D8u);
            }
            DrawText(values[i], vx, y, MENU_INK, 0xFFD8D8D8u);
        }
        y += 14;
        y += DrawWrapped(subs[i], x0 + 24, y, textW, sel ? MENU_INK : MENU_DIM, 1) * 11 + 4;
    }
    DrawText(sWizStep == WIZ_VERSION ? sWizHelpFirst[l] : sWizHelp[l], x0 + 8, y0 + h - 12, MENU_DIM, 0xFFFFFFFFu);
}

static void DrawMenu(int l, int width, int height) {
    int rows, i, x, y, labelW = 0, valueW = 0, barX = 10, barW, top = MENU_TOP, artX;
    char value[64];

    if (sPage == PAGE_NEWGAME) {
        DrawWizard(l, width, height);
        return;
    }

    if (!sMenuArtReady)
        DecodeMenuArt();
    for (i = PageFirst(); i < PageEnd(); i++) {
        labelW = SDL_max(labelW, TextWidth(sLabels[i][l]));
        ShortValue(i, value, sizeof(value));
        valueW = SDL_max(valueW, TextWidth(value));
    }
    barW = SDL_min(labelW + valueW + 40, width - barX - 6);
    rows = SDL_min(PageEnd() - PageFirst(), (height - top - 14) / 16);
    if (sScroll < PageFirst() || sScroll >= PageEnd())
        sScroll = PageFirst();
    if (sCursor < sScroll)
        sScroll = sCursor;
    if (sCursor >= sScroll + rows)
        sScroll = sCursor - rows + 1;

    /* white, the picture at the bottom right */
    Fill(0, 0, width, height, 0xFFFFFFFFu);
    /* the picture as tall as the screen (it was a fixed 160 lines, small on
       a large window), sampled pixel by pixel; artX/artY stay the left/top
       of an unscaled picture so the list's shading below still lines up */
    {
        int ah = height, aw = (GBA_SCREEN_W - MENU_ART_X) * height / GBA_SCREEN_H;
        int ax0 = width - aw;
        for (y = 0; y < ah; y++)
            for (x = 0; x < aw; x++)
                Pixel(ax0 + x, y, sMenuArt[(y * GBA_SCREEN_H / ah) * GBA_SCREEN_W + MENU_ART_X +
                                           x * GBA_SCREEN_H / ah]);
        artX = ax0 - MENU_ART_X;
    }
    /* keep the list readable where it runs over the picture */
    for (x = barX + barW - 24; x < barX + barW + 16; x++)
        if (x >= artX + MENU_ART_X)
            Shade(x, 0, 1, top + rows * 16, 0xFFFFFFFFu,
                  SDL_clamp(256 - (x - (barX + barW)) * 16, 0, 230));
    if (barX + barW - 24 > artX + MENU_ART_X)
        Shade(artX + MENU_ART_X, 0, barX + barW - 24 - (artX + MENU_ART_X), top + rows * 16, 0xFFFFFFFFu, 230);

    DrawText(sPage == PAGE_PS2 ? sPs2Title[l] : sPage == PAGE_NEWGAME ? sNgTitle[l] : sTitle[l], barX, MENU_TITLE_Y, MENU_INK, 0xFFD8D8D8u);
    if (sScroll > PageFirst())
        DrawText("\xE2\x86\x91", barX + barW - 8, 2, MENU_INK, 0xFFD8D8D8u);
    for (i = sScroll, y = top; i < sScroll + rows && i < PageEnd(); i++, y += 16) {
        int on = ItemEnabled(i), sel = i == sCursor;
        if (!ItemVisible(i)) {
            y -= 16;
            continue;
        }
        uint32_t face = on ? MENU_INK : MENU_DIM;

        if (i == ITEM_EXPERIMENTAL) {
            /* a section heading: red text and a thin line, no square */
            int end = DrawText(sLabels[i][l], barX, y - 1, MENU_RED_DARK, 0xFFD8D8D8u);
            Fill(end + 6, y + 4, barX + barW - end - 6, 1, MENU_GREY);
            continue;
        }
        MenuBar(barX, y, barW, sel);
        DrawText(sLabels[i][l], barX + 16, y - 2, face, 0xFFD8D8D8u);
        if (i == ITEM_CLOSE || i == ITEM_PS2_BACK || i == ITEM_NG_START)
            continue;
        ShortValue(i, value, sizeof(value));
        x = barX + barW - 4 - TextWidth(value);
        if (sel && on) {
            x -= 8;
            DrawText("\xE2\x86\x90", x - 10, y - 2, MENU_RED_DARK, 0xFFD8D8D8u);
            DrawText("\xE2\x86\x92", x + TextWidth(value) + 2, y - 2, MENU_RED_DARK, 0xFFD8D8D8u);
        }
        DrawText(value, x, y - 2, face, 0xFFD8D8D8u);
    }
    if (sScroll + rows < PageEnd())
        DrawText("\xE2\x86\x93", barX + barW - 8, top + rows * 16 - 2, MENU_INK, 0xFFD8D8D8u);
    DrawText(sPage == PAGE_NEWGAME ? sHelpNg[l] : sHelp[l], barX, height - 13, MENU_DIM, 0xFFFFFFFFu);
}

/* Test harness: KHCOM_OPTIONS_HOLD=n closes the menu n frames after it has
   faded in (the game is paused meanwhile, so frame-based keys cannot);
   KHCOM_FADE_SHOTS saves every frame of the fades as fade_<n>.bmp. */
static void HarnessFadeShot(int width, int height) {
    static int n;
    char name[32];
    SDL_Surface* s;

    if (!SDL_getenv("KHCOM_FADE_SHOTS"))
        return;
    s = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_XRGB8888, sFb, width * 4);
    SDL_snprintf(name, sizeof(name), "fade_%03d.bmp", n++);
    if (s) {
        SDL_SaveBMP(s, PortConfigPath(name));
        SDL_DestroySurface(s);
    }
}

/* Opt-in regression route, evaluated while the game is paused by this menu.
   KHCOM_OPTIONS_ACTIONS=tick:item:direction,... uses the same Change() as
   keyboard/controller input. Each action records its translated value and a
   rendered menu BMP; KHCOM_OPTIONS_HOLD supplies the ordinary close route.
   Tests must run in an isolated config/save directory (save bank is real). */
static int HarnessOptionsAction(void) {
    const char* route = SDL_getenv("KHCOM_OPTIONS_ACTIONS");
    int changed = 0;
    if (sPage == PAGE_NEWGAME && SDL_getenv("KHCOM_WIZARD_KEYS")) {
        const char* w = SDL_getenv("KHCOM_WIZARD_KEYS");
        sHarnessTick++;
        while (w && *w) {
            const char* comma = SDL_strchr(w, ',');
            const char* colon = SDL_strchr(w, ':');
            if (colon && (!comma || colon < comma) && SDL_atoi(w) == sHarnessTick) {
                PortLog("wizard-test: tick=%d step=%d cursor=%d key=%d", sHarnessTick, sWizStep, sWizCursor,
                        SDL_atoi(colon + 1));
                WizKey(SDL_atoi(colon + 1));
                changed = 1;
            }
            w = comma ? comma + 1 : NULL;
        }
        return changed;
    }
    if (!route)
        return 0;
    sHarnessTick++;
    while (route && *route) {
        const char* comma = SDL_strchr(route, ',');
        const char* item = SDL_strchr(route, ':');
        const char* dir = item ? SDL_strchr(item + 1, ':') : NULL;
        if (item && dir && (!comma || dir < comma) && SDL_atoi(route) == sHarnessTick) {
            int row = SDL_atoi(item + 1), direction = SDL_atoi(dir + 1);
            if (row >= 0 && row < ITEM_COUNT && row != ITEM_EXPERIMENTAL && row != ITEM_CLOSE) {
                char before[128], after[128];
                ValueText(row, before, sizeof(before));
                sCursor = row;
                Change(row, direction < 0 ? -1 : 1);
                ValueText(row, after, sizeof(after));
                PortLog("options-test: tick=%d item=%d enabled=%d before=[%s] after=[%s]",
                        sHarnessTick, row, ItemEnabled(row), before, after);
                if (row == ITEM_FULLSCREEN || row == ITEM_FS_RES || row == ITEM_VSYNC) {
                    int count = 0, vsync = -999, winW = 0, winH = 0;
                    SDL_Window** windows = SDL_GetWindows(&count);
                    if (windows && count > 0) {
                        SDL_SyncWindow(windows[0]);
                        const SDL_DisplayMode* mode = SDL_GetWindowFullscreenMode(windows[0]);
                        SDL_Renderer* renderer = SDL_GetRenderer(windows[0]);
                        SDL_GetWindowSizeInPixels(windows[0], &winW, &winH);
                        if (renderer)
                            SDL_GetRenderVSync(renderer, &vsync);
                        PortLog("options-video: tick=%d fullscreen=%d size=%dx%d mode=%dx%d vsync=%d driver=%s",
                                sHarnessTick, (SDL_GetWindowFlags(windows[0]) & SDL_WINDOW_FULLSCREEN) != 0,
                                winW, winH, mode ? mode->w : 0, mode ? mode->h : 0, vsync,
                                SDL_GetCurrentVideoDriver());
                    }
                    SDL_free(windows);
                }
                changed = 1;
            }
        }
        route = comma ? comma + 1 : NULL;
    }
    return changed;
}

static void HarnessOptionsShot(uint32_t* frame, int width, int height) {
    char name[64];
    SDL_Surface* surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_XRGB8888, frame, width * 4);
    if (!surface)
        return;
    SDL_snprintf(name, sizeof(name), "options_%04d_%02d.bmp", sHarnessTick, sCursor);
    SDL_SaveBMP(surface, PortConfigPath(name));
    SDL_DestroySurface(surface);
}

void PortUiDraw(uint32_t* frame, int width, int height) {
    int l = Lang();
    int harnessShot = 0;

    sFb = frame;
    sFbW = width;
    sFbH = height;

    if (!sOpen && sReturn > 0) {
        Shade(0, 0, width, height, MENU_FADE_COLOR, SDL_min(256, sReturn * 256 / MENU_FADE));
        sReturn--;
        HarnessFadeShot(width, height);
    }
    if (!sOpen) {
        if (sSkipPrompt && !sSkipOwnLayer)
            DrawSkipPrompt(l);
        if (sTitleHintFrames > 0) {
            const char* hint = sTitleHint[l];
            int gx = gPortScreen.originX + GBA_SCREEN_W - TextWidth(hint) - 6;
            int gy = gPortScreen.originY + 4;
            sTitleHintFrames--;
            MenuButton(gx - 18, gy + 2, hint, 0);
        }
        return;
    }

    if (sClosing > 0) {
        DrawMenu(l, width, height);
        Shade(0, 0, width, height, MENU_FADE_COLOR, SDL_min(256, (MENU_FADE - sClosing + 1) * 256 / MENU_FADE));
        HarnessFadeShot(width, height);
        if (--sClosing == 0) {
            sOpen = 0;
            sReturn = MENU_FADE;
        }
        return;
    }
    if (sFade < MENU_FADE) { /* the game's picture to black */
        Shade(0, 0, width, height, MENU_FADE_COLOR, SDL_min(256, (sFade + 1) * 256 / MENU_FADE));
        sFade++;
        HarnessFadeShot(width, height);
        return;
    }
    if (sFade >= MENU_FADE * 2)
        harnessShot = HarnessOptionsAction();
    DrawMenu(l, width, height);
    if (harnessShot)
        HarnessOptionsShot(frame, width, height);
    if (sFade < MENU_FADE * 2) { /* the menu in from black */
        Shade(0, 0, width, height, MENU_FADE_COLOR, (MENU_FADE * 2 - sFade) * 256 / MENU_FADE - 32);
        sFade++;
        HarnessFadeShot(width, height);
    } else if (SDL_getenv("KHCOM_OPTIONS_HOLD") && ++sFade >= MENU_FADE * 2 + SDL_atoi(SDL_getenv("KHCOM_OPTIONS_HOLD"))) {
        Close();
    }
}
