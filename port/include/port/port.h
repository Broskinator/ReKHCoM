#ifndef GUARD_PORT_PORT_H
#define GUARD_PORT_PORT_H

/*
 * Kingdom Hearts: Chain of Memories - native PC platform layer.
 *
 * The decompiled game code runs unmodified in a 32-bit process. The GBA
 * address space (work RAM, I/O, palette, VRAM, OAM, cartridge ROM and SRAM)
 * is mapped at its original addresses, so hardware registers and ROM pointers
 * keep working; this layer stands in for the BIOS, DMA, the PPU, the sound
 * hardware and the interrupt controller.
 */

#include <stdint.h>
#include <stddef.h>
#include "port/port_zoom.h"

#define GBA_EWRAM 0x02000000u
#define GBA_EWRAM_SIZE 0x40000u
#define GBA_IWRAM 0x03000000u
#define GBA_IWRAM_SIZE 0x8000u
#define GBA_IO 0x04000000u
#define GBA_IO_SIZE 0x400u
#define GBA_PLTT 0x05000000u
#define GBA_PLTT_SIZE 0x400u
#define GBA_VRAM 0x06000000u
#define GBA_VRAM_SIZE 0x18000u
#define GBA_OAM 0x07000000u
#define GBA_OAM_SIZE 0x400u
#define GBA_ROM 0x08000000u
#define GBA_ROM_SIZE 0x02000000u
#define GBA_SRAM 0x0E000000u
#define GBA_SRAM_SIZE 0x10000u

#define GBA_SCREEN_W 240
#define GBA_SCREEN_H 160
#define GBA_FPS 59.7275

/* Largest internal resolution the renderer supports. */
#define PORT_MAX_W 2560
#define PORT_MAX_H 1440

#define IO8(off) (*(volatile uint8_t*)(uintptr_t)(GBA_IO + (off)))
#define IO16(off) (*(volatile uint16_t*)(uintptr_t)(GBA_IO + (off)))
#define IO32(off) (*(volatile uint32_t*)(uintptr_t)(GBA_IO + (off)))

/* ---- configuration (port_config.c) ------------------------------------ */

enum PortViewMode {
    PORT_VIEW_ORIGINAL,   /* 240x160, the GBA picture */
    PORT_VIEW_WIDE,       /* 16:9 at the original height */
    PORT_VIEW_EXTENDED,   /* 16:9, more of the world in both directions */
    PORT_VIEW_MAXIMUM,    /* 16:9, as far out as the engine allows */
    PORT_VIEW_AUTO,       /* fill the window at a whole-number pixel scale */
    PORT_VIEW_COUNT
};

enum PortScaleMode {
    PORT_SCALE_INTEGER,   /* sharpest: largest whole-number multiple */
    PORT_SCALE_FIT,       /* fill the window, keep aspect ratio */
    PORT_SCALE_STRETCH,   /* fill the window */
    PORT_SCALE_COUNT
};

enum PortFilter {
    PORT_FILTER_NEAREST,
    PORT_FILTER_SHARP,    /* nearest at integer scale, then smooth to fit */
    PORT_FILTER_LINEAR,
    PORT_FILTER_COUNT
};

/* Remappable actions (kh_com.ini kb_<name>, kb_<name>_2, pad_<name>). */
enum {
    PORT_ACT_A,
    PORT_ACT_B,
    PORT_ACT_L,
    PORT_ACT_R,
    PORT_ACT_START,
    PORT_ACT_SELECT,
    PORT_ACT_UP,
    PORT_ACT_DOWN,
    PORT_ACT_LEFT,
    PORT_ACT_RIGHT,
    PORT_ACT_STOCK,
    PORT_ACT_DODGE,
    PORT_ACT_UNSTOCK,
    PORT_ACT_COUNT
};

/* pad_<action> values 0..BUTTON_COUNT-1 are buttons; this range encodes an
 * axis and sign so analog triggers can be captured and remapped. */
#define PORT_PAD_AXIS_BASE 100

typedef struct PortConfig {
    int viewMode;
    int zoom;             /* PORT_VIEW_AUTO: 1..6, the view as a multiple of the GBA's (1 Original, 2 Enhanced) */
    int scaleMode;
    int filter;
    int fullscreen;
    int vsync;
    int hudSize;          /* wide view: HUD drawn this many pixel-scale steps larger (0 = same) */
    int narrowFit;        /* scenes without a wide view: 1 fill the height, 0 exact pixels */
    int scanlines;        /* 0..100 % intensity */
    int colorCorrection;  /* emulate the GBA LCD colours */
    int frameSkip;        /* reserved */
    int showFps;
    int windowScale;
    int fullscreenWidth;  /* 0 = desktop resolution (borderless) */
    int fullscreenHeight;
    int masterVolume;     /* 0..100 */
    int musicVolume;
    int sfxVolume;
    int dodgeButton;      /* one button for the battle dodge roll */
    int stockButton;      /* one button for L + R (stock a card / sleight) */
    int hdText;           /* dialogue text drawn with an outline font at output resolution */
    int cardHudRaise;     /* battle: card hand lifted by this many GBA pixels (0..16) */
    int unstockButton;    /* experimental return-stocked-cards control; opt-in, off by default */
    int saveBank;         /* 1..PORT_SAVE_BANKS: which save file holds the game's two slots */
    int hd2d;             /* experimental HD-2D look (on by default in --hd2d builds) */
    int smoothAudio;      /* experimental: cubic sample interpolation and a gentle low-pass */
    int ps2Mode;          /* experimental Re:Chain of Memories mode; off: the game exactly as on the cartridge */
    int ps2Difficulty;    /* Re:CoM mode: 0 standard, 1 beginner, 2 proud */
    int ps2FastText;      /* experimental: event dialogue types one character a frame */
    int hdHud;           /* experimental HD HUD placeholder: enemy HP panel */
    int aspect;          /* picture shape: 0 fills the window, 1 at most 16:9, 2 at most 21:9, 3 force 21:9, 4 force 32:9, 5 exact 4:3, 6 exact GBA 3:2 */
    int integerZoom;     /* 1: select only whole pixel scales (v1.4 numbering) */
    int keys[PORT_ACT_COUNT][2]; /* SDL keycodes, 0 = none */
    int pad[PORT_ACT_COUNT];     /* SDL gamepad button or PORT_PAD_AXIS_BASE+axis*2+sign */
    int cheatUnlockRiku;         /* opt-in title access; never marks Sora cleared */
    int undub;                  /* Japanese voices; applied when loading the ROM */
    int savePointSlotSelect;    /* experimental: choose the save slot at save points */
    int hideTouchControls;      /* hide gameplay panels; keep access to options */
    int cheatOneHit;            /* cheat: enemies fall to one hit */
    int cheatInfiniteHp;        /* cheat: the player loses no HP */
    int moogleNotice;           /* experimental, off: show the moogle points picked up and the total */
    int gbaMapLogic;            /* cheats > debug mode, off: field enemies leave at the GBA screen's edges */
    int gameLanguage;          /* European game: 0 asks (once), 1..5 English, French, German, Italian, Spanish */
    int invertCardScroll;      /* off by default: reverse L/R and right-stick card scrolling in battle */
    int padLayout;             /* face buttons: 0 detect (Nintendo pads swap A/B, X/Y), 1 Xbox positions, 2 Nintendo */
} PortConfig;

extern PortConfig gPortConfig;

/* The cartridge version this executable is built for. */
#if defined(VERSION_US)
#define PORT_ROM_CODE "B8CE"
#define PORT_ROM_REGION "USA"
#define PORT_SAVE_NAME "kh_com_us.sav"
#elif defined(VERSION_JP)
#define PORT_ROM_CODE "B8CJ"
#define PORT_ROM_REGION "Japan"
#define PORT_SAVE_NAME "kh_com_jp.sav"
#else
#define PORT_ROM_CODE "B8CP"
#define PORT_ROM_REGION "Europe"
#define PORT_SAVE_NAME "kh_com.sav"
#endif

/* ROM given on the command line (--rom), tried before the usual names. */
extern const char* gPortRomPath;

int PortRikuAvailable(int soraCleared);
int PortEventBackdropPanel(void); /* single authored backdrop panel, or -1 */
int PortEventTiledBackdrop(void); /* Riku scenery eligible for verified tile-band extension, or -1 */
int PortEventBackdropProfile(void); /* current story event, -1 outside events */
void PortConfigLoad(void);
void PortConfigSetDataPath(const char* path);
void PortConfigSave(void);
int PortSaveSlotSelectEnabled(void);
void PortConfigView(int mode, int* w, int* h);
/* Internal size and pixel scale that fill an outW x outH output (auto view). */
void PortConfigAutoView(int outW, int outH, int* w, int* h, int* scale);

int PortConfigZoomId(int level);
int PortConfigZoomNext(int level, int dir);
float PortConfigZoomScaleF(int level); /* pixel scale of a zoom level here */
int PortConfigZoomScale(int level);    /* ...rounded down (HUD, labels) */
int PortConfigZoomExact(int level);    /* the pixel scale is whole */
int PortConfigZoomHeavy(int zoom);     /* the step draws more than full speed allows */
const char* PortConfigPath(const char* name);

/* ---- memory (port_memory.c) -------------------------------------------- */

int PortMemoryInit(void);
int PortRomLoad(void);
int PortUndubAvailable(void);
void PortUndubApply(void);
void PortSramLoad(void);
void PortSramSyncIfDirty(void);
/* Save banks: bank 1 is PORT_SAVE_NAME, bank n kh_com[_us|_jp]_bank<n>.sav. */
#define PORT_SAVE_BANKS 5
const char* PortSaveName(void);
/* Writes the current bank, then loads gPortConfig.saveBank. */
void PortSramSwitchBank(void);
void PortBiosAreaRefresh(void);
/* RetroAchievements flattened GBA RAM/SRAM map (not hardware bus addresses).
 * Returns 1 on success; invalid ranges are zero-filled and return 0. */
int PortRaReadMemory(uint32_t address, uint8_t* buffer, uint32_t size);
void PortAchievementsInit(void);
void PortAchievementsRomLoaded(const uint8_t* rom, size_t size);
void PortAchievementsFrame(void);
void PortAchievementsInvalidate(const char* reason);
void PortAchievementsShutdown(void);
/* badgeIndex: position in the official set (its badges.png atlas); -1 = none */
void PortAchievementToast(const char* title, const char* detail, const char* label, int badgeIndex);
void PortAchievementToastDraw(void* renderer, int width, int height);
void PortAchievementToastShutdown(void);
/* The game's globals: initialised data and zero-initialised data. */
void PortGameSections(char** data, size_t* dataSize, char** bss, size_t* bssSize);

/* ---- system / frame loop (port_main.c) --------------------------------- */

void PortFrame(void);
void PortVideoApplyConfig(void);
/* port_ps2qol.c: the optional Re:Chain of Memories version of a game. */
int PortRecomActive(void);
void PortRecomSetActive(int active);
int PortHdHudEnemy(int visible, int ratio, int layer);
extern int gPortHdHudPart;
int PortPpuEnemyHudRect(int* rect);
uint32_t* PortHdHudExportBuffer(void);
void PortHdHudExportSprite(int w, int h, int part, int ax, int ay);
extern int gPortHudDigit; /* 0 none, 1..10 for digits 0..9 */
typedef struct PortHudDigit {
    int x, y, w, h, code, slot;
    uint32_t color;
} PortHudDigit;
int PortPpuHudDigits(PortHudDigit* out, int max);
int PortPpuHudDigitVisible(int digit, int x, int y);
int PortHdHudLettersReady(void);
int PortPpuEnemyHudLabel(int* rect, uint32_t* color);
int PortPpuEnemyHudBar(int* rect, uint32_t* rows);
int PortPpuEnemyHudBarVisible(int x, int y);
int PortPpuPlayerHudBar(int* rect, uint32_t* rows);
int PortPpuPlayerHudBarVisible(int x, int y);
int PortHdHudPlayerFrameReady(void);
int PortPpuPlayerHudFrame(int* rect);
int PortPpuPlayerHudFullHp(void);
void PortHdHudSpriteBegin(void);
int PortHdHudSpriteRegister(const uint32_t* pixels, int w, int h, int slot, int x, int y, int flip, int alpha);
int PortPpuHudSpriteVisible(int slot, int x, int y);
int PortQolButtons(void);
void PortSetQolButtons(int on);
int PortVideoListModes(int* w, int* h, int max);
void PortRaiseIrq(uint16_t flag);
void PortFatal(const char* fmt, ...);
void PortLog(const char* fmt, ...);
uint32_t PortTicks(void);

/* ---- DMA (port_dma.c) ---------------------------------------------------- */

void PortDmaHBlank(void);
void PortDmaVBlank(void);

/* ---- video (port_ppu.c) -------------------------------------------------- */

typedef struct PortScreen {
    int width;            /* internal resolution */
    int height;
    int originX;          /* where GBA pixel (0,0) lands in the frame */
    int originY;
    int wide;             /* the current scene draws beyond 240x160 */
} PortScreen;

extern PortScreen gPortScreen;
extern uint32_t* gPortFrame;          /* Active canvas capacity, on the heap (PortCanvasAlloc) */
extern uint32_t* gPortMenuBackground;
void PortCanvasAlloc(void);
uint32_t PortMenuHeaderPixel(int y, int phase);
uint32_t PortMenuHeaderPixelAt(int y, int distance); /* follows the header sprite's slide */
uint32_t PortMenuBandPixel(int y, int phase);
uint32_t PortTitleBandPixel(int side, int y);
int PortSpotlightActive(void);
uint32_t PortMenuTabPixel(int y, int phase, int side);

void PortPpuSetResolution(int w, int h);
void PortPpuRenderFrame(void);
void PortPpuVBlankLines(void);
/* HUD layer: sprites anchored with PortHudBegin, drawn at their own scale. */
extern int gPortHudScale;   /* output pixels per HUD pixel; 0 = no separate layer */
extern int gPortHudW;
extern int gPortHudH;
int PortPpuHudActive(void);
void PortPpuRenderHud(uint32_t* buf);
/* HD text: a dialogue glyph the PPU left out of the picture, at its place on
   the world canvas (hud 0) or the HUD canvas (hud 1), with its colours. */
typedef struct PortTextGlyph {
    int x, y;
    uint16_t code;
    uint8_t semi; /* 0 opaque, else the alpha (0..255) of a blended sprite */
    uint8_t style; /* 0 dialogue / menu font, 1 movie subtitles, 2 numbers in cells */
    uint8_t cap, base; /* style 2: digit height and baseline row in the cell (0: 5 and 7) */
    uint8_t clip0, clip1; /* visible columns [clip0, clip1) of the 8-pixel cell; 0,0: all */
    uint32_t face, shadow;
} PortTextGlyph;
int PortPpuTextGlyphs(PortTextGlyph* out, int max, int hud);
/* Frames presented since start (port_main.c). */
uint32_t PortFrameCount(void);

/* ---- audio (port_audio.c) ------------------------------------------------ */

int PortAudioInit(void);
void PortAudioFrame(void);
void PortAudioShutdown(void);

/* ---- input (port_input.c) ------------------------------------------------ */

void PortInputInit(void);
uint16_t PortInputKeys(void);
/* The port's extra buttons, beyond the GBA's ten key bits. */
#define PORT_KEY_STOCK 0x400
#define PORT_KEY_DODGE 0x800
#define PORT_KEY_UNSTOCK 0x1000
/* Turns the extra buttons into GBA keys and requests; returns the GBA key
   bits. Called once per frame. */
uint16_t PortInputResolve(uint16_t keys);
void PortInputHandleEvent(const void* sdlEvent);

#endif
