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
#define PORT_MAX_W 640
#define PORT_MAX_H 360

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

typedef struct PortConfig {
    int viewMode;
    int zoom;             /* PORT_VIEW_AUTO: 0 near .. 3 far */
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
    int unstockButton;    /* held button / right stick down returns the stocked cards (Sora) */
    int saveBank;         /* 1..PORT_SAVE_BANKS: which save file holds the game's two slots */
    int hd2d;             /* experimental HD-2D look (on by default in --hd2d builds) */
    int smoothAudio;      /* experimental: cubic sample interpolation and a gentle low-pass */
    int keys[PORT_ACT_COUNT][2]; /* SDL keycodes, 0 = none */
    int pad[PORT_ACT_COUNT];     /* SDL gamepad buttons, -1 = none */
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

void PortConfigLoad(void);
void PortConfigSetDataPath(const char* path);
void PortConfigSave(void);
void PortConfigView(int mode, int* w, int* h);
/* Internal size and pixel scale that fill an outW x outH output (auto view). */
void PortConfigAutoView(int outW, int outH, int* w, int* h, int* scale);
const char* PortConfigPath(const char* name);

/* ---- memory (port_memory.c) -------------------------------------------- */

int PortMemoryInit(void);
int PortRomLoad(void);
void PortSramLoad(void);
void PortSramSyncIfDirty(void);
/* Save banks: bank 1 is PORT_SAVE_NAME, bank n kh_com[_us|_jp]_bank<n>.sav. */
#define PORT_SAVE_BANKS 5
const char* PortSaveName(void);
/* Writes the current bank, then loads gPortConfig.saveBank. */
void PortSramSwitchBank(void);
void PortBiosAreaRefresh(void);
/* The game's globals: initialised data and zero-initialised data. */
void PortGameSections(char** data, size_t* dataSize, char** bss, size_t* bssSize);

/* ---- system / frame loop (port_main.c) --------------------------------- */

void PortFrame(void);
void PortVideoApplyConfig(void);
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
extern uint32_t gPortFrame[PORT_MAX_W * PORT_MAX_H];
extern uint32_t gPortMenuBackground[PORT_MAX_W * PORT_MAX_H];
uint32_t PortMenuHeaderPixel(int y, int phase);
uint32_t PortMenuBandPixel(int y, int phase);
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
    uint8_t style; /* 0 dialogue / menu font, 1 movie subtitles */
    uint32_t face, shadow;
} PortTextGlyph;
int PortPpuTextGlyphs(PortTextGlyph* out, int max, int hud);

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
