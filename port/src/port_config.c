/*
 * Settings, stored as key=value lines in kh_com.ini next to the executable.
 */
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

#include "port/port.h"

PortConfig gPortConfig = {
    PORT_VIEW_AUTO,     /* viewMode */
    1,                  /* zoom */
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
    1920,               /* fullscreenWidth */
    1080,               /* fullscreenHeight */
    100,                /* masterVolume */
    100,                /* musicVolume */
    100,                /* sfxVolume */
    1,                  /* dodgeButton */
    1,                  /* stockButton */
    0,                  /* hdText */
    8,                  /* cardHudRaise */
};

#define CONFIG_VERSION 2

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
    { "zoom", &gPortConfig.zoom, 0, 3 },
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

void PortConfigAutoView(int outW, int outH, int* w, int* h, int* scale) {
    static const int target[4] = { 160, 180, 216, 240 };
    int s = outH / target[SDL_clamp(gPortConfig.zoom, 0, 3)];

    if (s < 1)
        s = 1;
    /* Grow the scale until the canvas fits the renderer's limits. */
    while ((outW + s - 1) / s > PORT_MAX_W || (outH + s - 1) / s > PORT_MAX_H)
        s++;
    *w = (outW + s - 1) / s;
    *h = (outH + s - 1) / s;
    if (*w < GBA_SCREEN_W)
        *w = GBA_SCREEN_W;
    if (*h < GBA_SCREEN_H)
        *h = GBA_SCREEN_H;
    *scale = s;
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
    if (sConfigVersion < CONFIG_VERSION)
        gPortConfig.viewMode = PORT_VIEW_AUTO;
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
