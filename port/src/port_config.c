/*
 * Settings, stored as key=value lines in kh_com.ini next to the executable.
 */
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

#include "port/port.h"

PortConfig gPortConfig = {
    PORT_VIEW_AUTO,     /* viewMode */
    2,                  /* zoom: x2, Enhanced */
    PORT_SCALE_INTEGER, /* scaleMode */
    PORT_FILTER_NEAREST, /* filter */
    0,                  /* fullscreen */
    1,                  /* vsync */
    1,                  /* hudSize */
    1,                  /* narrowFit */
    0,                  /* scanlines */
    0,                  /* colorCorrection */
    0,                  /* frameSkip */
    0,                  /* showFps */
    4,                  /* windowScale */
    0,                  /* fullscreenWidth: 0 = the desktop's resolution */
    0,                  /* fullscreenHeight */
    100,                /* masterVolume */
    100,                /* musicVolume */
    100,                /* sfxVolume */
    1,                  /* dodgeButton */
    1,                  /* stockButton */
    0,                  /* hdText */
    8,                  /* cardHudRaise */
    1,                  /* unstockButton */
    1,                  /* saveBank */
#ifdef PORT_HD2D
    1,                  /* hd2d: on in the alternative HD-2D build */
#else
    0,                  /* hd2d (experimental) */
#endif
    0,                  /* smoothAudio (experimental) */
    0,                  /* ps2Mode (experimental) */
    0,                  /* ps2Difficulty (experimental) */
    0,                  /* ps2FastText (experimental) */
    0,                  /* hdHud: pending, forced off (see PortConfigLoad) */
    0,                  /* aspect: fill the window */
    1,                  /* integerZoom */
    {
        { SDLK_X, SDLK_K },             /* A */
        { SDLK_Z, SDLK_J },             /* B */
        { SDLK_A, 0 },                  /* L */
        { SDLK_S, 0 },                  /* R */
        { SDLK_RETURN, 0 },             /* Start */
        { SDLK_BACKSPACE, SDLK_RSHIFT }, /* Select */
        { SDLK_UP, SDLK_W },            /* Up */
        { SDLK_DOWN, 0 },               /* Down */
        { SDLK_LEFT, 0 },               /* Left */
        { SDLK_RIGHT, 0 },              /* Right */
        { SDLK_D, 0 },                  /* stock (L + R) */
        { SDLK_C, 0 },                  /* dodge roll */
        { SDLK_V, 0 },                  /* return stocked cards */
    },
    {
        SDL_GAMEPAD_BUTTON_SOUTH, SDL_GAMEPAD_BUTTON_EAST, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
        SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, SDL_GAMEPAD_BUTTON_START, SDL_GAMEPAD_BUTTON_BACK,
        SDL_GAMEPAD_BUTTON_DPAD_UP, SDL_GAMEPAD_BUTTON_DPAD_DOWN, SDL_GAMEPAD_BUTTON_DPAD_LEFT,
        SDL_GAMEPAD_BUTTON_DPAD_RIGHT, SDL_GAMEPAD_BUTTON_NORTH, SDL_GAMEPAD_BUTTON_WEST,
        SDL_GAMEPAD_BUTTON_RIGHT_STICK,
    },
};

#define CONFIG_VERSION 8

static int sConfigVersion;
static char sDataPath[768];

void PortConfigSetDataPath(const char* path) {
    SDL_snprintf(sDataPath, sizeof(sDataPath), "%s/", path);
}

typedef struct Key {
    const char* name;
    int* value;
    int min;
    int max;
} Key;

static const Key sKeys[] = {
    { "config_version", &sConfigVersion, 0, 1000 },
    { "view_mode", &gPortConfig.viewMode, 0, PORT_VIEW_COUNT - 1 },
    { "zoom", &gPortConfig.zoom, 1, PORT_ZOOM_LEVELS },
    { "integer_zoom", &gPortConfig.integerZoom, 0, 1 },
    { "scale_mode", &gPortConfig.scaleMode, 0, PORT_SCALE_COUNT - 1 },
    { "filter", &gPortConfig.filter, 0, PORT_FILTER_COUNT - 1 },
    { "fullscreen", &gPortConfig.fullscreen, 0, 1 },
    { "vsync", &gPortConfig.vsync, 0, 1 },
    { "hud_size", &gPortConfig.hudSize, 0, 2 },
    { "narrow_fit", &gPortConfig.narrowFit, 0, 1 },
    { "scanlines", &gPortConfig.scanlines, 0, 100 },
    { "color_correction", &gPortConfig.colorCorrection, 0, 1 },
    { "show_fps", &gPortConfig.showFps, 0, 1 },
    { "window_scale", &gPortConfig.windowScale, 1, 8 },
    { "fullscreen_width", &gPortConfig.fullscreenWidth, 0, 16384 },
    { "fullscreen_height", &gPortConfig.fullscreenHeight, 0, 16384 },
    { "master_volume", &gPortConfig.masterVolume, 0, 100 },
    { "music_volume", &gPortConfig.musicVolume, 0, 100 },
    { "sfx_volume", &gPortConfig.sfxVolume, 0, 100 },
    { "dodge_button", &gPortConfig.dodgeButton, 0, 1 },
    { "stock_button", &gPortConfig.stockButton, 0, 1 },
    { "hd_text", &gPortConfig.hdText, 0, 1 },
    { "card_hud_raise", &gPortConfig.cardHudRaise, 0, 16 },
    { "unstock_button", &gPortConfig.unstockButton, 0, 1 },
    { "save_bank", &gPortConfig.saveBank, 1, PORT_SAVE_BANKS },
    { "hd2d", &gPortConfig.hd2d, 0, 1 },
    { "smooth_audio", &gPortConfig.smoothAudio, 0, 1 },
    { "ps2_mode", &gPortConfig.ps2Mode, 0, 1 },
    { "ps2_difficulty", &gPortConfig.ps2Difficulty, 0, 2 },
    { "ps2_fast_text", &gPortConfig.ps2FastText, 0, 1 },
    { "hd_hud", &gPortConfig.hdHud, 0, 1 },
    { "aspect", &gPortConfig.aspect, 0, 4 },
#define BIND(name, act)                                                           { "kb_" name, &gPortConfig.keys[act][0], 0, 0x7FFFFFFF },                     { "kb_" name "_2", &gPortConfig.keys[act][1], 0, 0x7FFFFFFF },                { "pad_" name, &gPortConfig.pad[act], -1, SDL_GAMEPAD_BUTTON_COUNT - 1 }
    BIND("a", PORT_ACT_A), BIND("b", PORT_ACT_B), BIND("l", PORT_ACT_L), BIND("r", PORT_ACT_R),
    BIND("start", PORT_ACT_START), BIND("select", PORT_ACT_SELECT), BIND("up", PORT_ACT_UP),
    BIND("down", PORT_ACT_DOWN), BIND("left", PORT_ACT_LEFT), BIND("right", PORT_ACT_RIGHT),
    BIND("stock", PORT_ACT_STOCK), BIND("dodge", PORT_ACT_DODGE),
    BIND("unstock", PORT_ACT_UNSTOCK),
#undef BIND
};

static char* Trim(char* s) {
    char* e;

    while (SDL_isspace((unsigned char)*s))
        s++;
    e = s + SDL_strlen(s);
    while (e > s && SDL_isspace((unsigned char)e[-1]))
        *--e = 0;
    return s;
}

const char* PortConfigPath(const char* name) {
    static char paths[4][1024];
    static int next;
    const char* base = sDataPath[0] ? sDataPath : SDL_GetBasePath();
    char* out = paths[next++ & 3];

    SDL_snprintf(out, 1024, "%s%s", base ? base : "", name);
    return out;
}

void PortConfigView(int mode, int* w, int* h) {
    switch (mode) {
        case PORT_VIEW_ORIGINAL:
            *w = 240;
            *h = 160;
            break;
        case PORT_VIEW_WIDE:
            *w = 284;
            *h = 160;
            break;
        case PORT_VIEW_MAXIMUM:
            *w = 384;
            *h = 216;
            break;
        default:
            *w = 320;
            *h = 180;
            break;
    }
}

/* Auto view's whole-number pixel scale for each zoom step at a window
   height. Normal (the default) shows about 360 lines: x3 on a 1080-line
   screen. Near is the largest scale that still shows the GBA's 160 lines;
   Far, Very far and Farthest are one scale smaller each. A step only exists
   where it gives its own scale (no two steps alike) within the canvas's
   limits; an unavailable step falls back to the nearest one towards
   Normal. */
static int sZoomOutW = 1920, sZoomOutH = 1080;

/* The smallest scale whose canvas the software PPU still draws at full
   speed: it costs about 40 ns a pixel, so the canvas is kept near 640x360
   (about 10 ms a frame). Larger canvases (x2 at 1080p: 960x540, 22 ms)
   dropped below 60 fps. */
#define ZOOM_PIXEL_BUDGET (640 * 360 * 5 / 4)

/* the smallest scale the canvas's buffers allow */
static int ZoomLeast(void) {
    return SDL_max(SDL_max((sZoomOutW + PORT_MAX_W - 1) / PORT_MAX_W, (sZoomOutH + PORT_MAX_H - 1) / PORT_MAX_H), 1);
}

/* ...and the smallest one drawn at full speed (the default never goes
   below it; the steps beyond it stay available, marked as demanding) */
static int ZoomFast(void) {
    int s = ZoomLeast();
    while (((sZoomOutW + s - 1) / s) * ((sZoomOutH + s - 1) / s) > ZOOM_PIXEL_BUDGET)
        s++;
    return s;
}



/* Zoom (Auto view): level N shows N times the GBA's view (160*N lines):
   x1 is the cartridge's own picture at full height, x2 (Enhanced) the
   default. With Integer scale on, the pixel scale is rounded down to a
   whole number (sharp, may show a little more); off, it is exact. */
float PortConfigZoomScaleF(int level) {
    float f = sZoomOutH / (float)(GBA_SCREEN_H * SDL_clamp(level, 1, PORT_ZOOM_LEVELS));
    float least = (float)ZoomLeast();
    if (gPortConfig.integerZoom)
        f = SDL_floorf(f);
    return SDL_max(f, SDL_max(least, 1.0f));
}

int PortConfigZoomScale(int level) {
    return (int)PortConfigZoomScaleF(level);
}

int PortConfigZoomExact(int level) {
    return sZoomOutH % (GBA_SCREEN_H * SDL_clamp(level, 1, PORT_ZOOM_LEVELS)) == 0;
}

int PortConfigZoomHeavy(int level) {
    float f = PortConfigZoomScaleF(level);
    return (sZoomOutW / f) * (sZoomOutH / f) > ZOOM_PIXEL_BUDGET;
}

float gPortAutoScaleF; /* the Auto view's exact pixel scale (may be fractional) */

void PortConfigAutoView(int outW, int outH, int* w, int* h, int* scale) {
    float f;
    extern int PortUiActive(void);

    sZoomOutW = outW;
    sZoomOutH = outH;
    /* the options menu is drawn on the canvas: while it is open the canvas
       keeps the Enhanced size, so its text stays readable at any zoom */
    f = PortConfigZoomScaleF(PortUiActive() ? 2 : gPortConfig.zoom);
    while (SDL_ceilf(outW / f) > PORT_MAX_W || SDL_ceilf(outH / f) > PORT_MAX_H)
        f += 0.25f;
    *w = SDL_max((int)SDL_ceilf(outW / f), GBA_SCREEN_W);
    *h = SDL_max((int)SDL_ceilf(outH / f), GBA_SCREEN_H);
    *scale = SDL_max((int)f, 1);
    gPortAutoScaleF = f;
}

void PortConfigLoad(void) {
    FILE* f = fopen(PortConfigPath("kh_com.ini"), "r");
    char line[256];

    if (f == NULL)
        return;
    while (fgets(line, sizeof(line), f)) {
        char* eq = strchr(line, '=');
        size_t i;

        if (line[0] == '#' || line[0] == ';' || eq == NULL)
            continue;
        *eq = '\0';
        for (i = 0; i < sizeof(sKeys) / sizeof(sKeys[0]); i++) {
            if (SDL_strcmp(Trim(line), sKeys[i].name) == 0) {
                int v = SDL_atoi(eq + 1);
                *sKeys[i].value = SDL_clamp(v, sKeys[i].min, sKeys[i].max);
            }
        }
    }
    fclose(f);
    /* Settings files from before the auto view switch to it once. */
    if (sConfigVersion < 2)
        gPortConfig.viewMode = PORT_VIEW_AUTO;
    /* ...and from before v1.0 to filling the height in GBA-size scenes,
       whose margins are now drawn on the picture's own pixel grid. */
    if (sConfigVersion < 3)
        gPortConfig.narrowFit = 1;
    /* ...and from before v1.5 (fullscreen 1920x1080 by default) to the
       monitor's own resolution, so ultrawide monitors fill at 32:9. */
    /* ...and from before v1.6.2 back to the Normal zoom (x3 at 1080p): the
       steps were renumbered while testing */
    if (sConfigVersion < 5)
        gPortConfig.zoom = 1;
    /* ...and to filling the window (the aspect limit was left on by tests;
       it is an opt-in for very wide monitors) */
    if (sConfigVersion < 6)
        gPortConfig.aspect = 0;
    /* ...and from the named zoom steps to Enhanced (0) or a scale (xN) */
    if (sConfigVersion < 8) {
        gPortConfig.zoom = 2; /* x2, Enhanced */
        gPortConfig.integerZoom = 1;
    }
    if (sConfigVersion < 4 && gPortConfig.fullscreenWidth == 1920 && gPortConfig.fullscreenHeight == 1080) {
        gPortConfig.fullscreenWidth = 0;
        gPortConfig.fullscreenHeight = 0;
    }
    /* HD HUD: pending, off for players until it is finished, whatever an
       older kh_com.ini says. Development builds turn it on with
       KHCOM_HD_HUD_DEV=1 (then kh_com.ini's hd_hud applies). */
    if (!SDL_getenv("KHCOM_HD_HUD_DEV"))
        gPortConfig.hdHud = 0;
#ifndef PORT_HD2D
    /* HD-2D look: pending as well (too many scenes still wrong); forced off
       outside its own build unless KHCOM_HD2D_DEV=1. */
    if (!SDL_getenv("KHCOM_HD2D_DEV"))
        gPortConfig.hd2d = 0;
#endif
}

void PortConfigSave(void) {
    FILE* f = fopen(PortConfigPath("kh_com.ini"), "w");
    size_t i;

    if (f == NULL)
        return;
    fprintf(f, "# Kingdom Hearts: Chain of Memories (PC) settings\n");
    sConfigVersion = CONFIG_VERSION;
    for (i = 0; i < sizeof(sKeys) / sizeof(sKeys[0]); i++)
        fprintf(f, "%s=%d\n", sKeys[i].name, *sKeys[i].value);
    fclose(f);
}
