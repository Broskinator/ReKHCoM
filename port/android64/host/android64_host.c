/* The 64-bit Android host (port/progress/ANDROID_64.md): loads the game image (port/android64/build_android64.py: AArch64
 * code with 32-bit pointers, a Mach-O executable linked at 0x01000000) from the APK, serves the bridge
 * (port/ps5/shared/bridge_list.h) with SDL3 and starts the game.
 *
 * The 32-bit app needs the GBA's hardware addresses, which some phones keep busy (issues #22, #34) and 64-bit-only
 * phones cannot run at all. Here the GBA space is the image's own memory (PORT_GBA_RELOC) and the image sits below
 * ART's heap (0x12c00000), claimed as soon as this library loads.
 *
 * Video: the game's quads go to the GPU with SDL_RenderGeometry. Sound: an SDL audio stream (the bridge's rate
 * ratio becomes the stream's frequency ratio). Input: SDL gamepads, the touch screen (touch_poll; the game draws its
 * own touch controls), the background state (app_state). Files: the data folder the activity passes (--data-dir);
 * "apk:/<name>" paths are the APK's assets. */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "bridge.h"

#define IMAGE_BASE 0x01000000u
#define IMAGE_LIMIT 0x0C000000u /* the reservation: the image (about 150 MB) ends well before ART's heap */

static int sReserved;

/* Before the graphics driver, the audio stack and the Java heap's growth can land in the range. */
__attribute__((constructor)) static void ReserveImageSpace(void) {
    void* p = mmap((void*)(uintptr_t)IMAGE_BASE, IMAGE_LIMIT - IMAGE_BASE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == (void*)(uintptr_t)IMAGE_BASE)
        sReserved = 1;
    else if (p != MAP_FAILED)
        munmap(p, IMAGE_LIMIT - IMAGE_BASE);
}

/* ---- log ------------------------------------------------------------------------------------------------------ */

static FILE* sLog;
static char sData[768] = ".";

static void Log(const char* fmt, ...) {
    char line[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);
    size_t n = strlen(line);
    while (n && (line[n - 1] == '\n' || line[n - 1] == '\r'))
        line[--n] = 0;
    SDL_Log("[ReKHCoM] %s", line);
    if (sLog) {
        fprintf(sLog, "%s\n", line);
        fflush(sLog);
    }
}

static void Fatal(const char* text) {
    Log("%s", text);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Kingdom Hearts: Chain of Memories", text, NULL);
    exit(1);
}

/* ---- bridge: process, files ----------------------------------------------------------------------------------- */

static int sQuit, sBackground;
static jmp_buf sExit; /* the game's exit returns to main: SDL's activity ends the process cleanly from there */
static int sExitCode;

static void H_log(const char* s) { Log("%s", s); }
static void H_exit(int code) {
    Log("the game ended (%d)", code);
    if (sLog)
        fflush(sLog);
    sExitCode = code;
    longjmp(sExit, 1);
}
static uint64_t H_ticks_us(void) { return SDL_GetTicksNS() / 1000; }
static void H_sleep_us(uint32_t us) { SDL_DelayNS((Uint64)us * 1000); }
static int H_getenv(const char* name, char* out, int cap) {
    const char* v = SDL_getenv(name);
    if (!v)
        return -1;
    SDL_strlcpy(out, v, (size_t)cap);
    return (int)strlen(out);
}

/* Files are SDL streams: a normal path is a file, "apk:/name" an asset of the APK (with or without "assets/"). */
static SDL_IOStream* sFiles[64];
static SDL_IOStream* OpenPath(const char* path, const char* mode) {
    if (strncmp(path, "apk:/", 5) == 0) {
        const char* name = path + 5;
        SDL_IOStream* io = SDL_IOFromFile(name, "rb");
        if (!io && strncmp(name, "assets/", 7) == 0)
            io = SDL_IOFromFile(name + 7, "rb");
        return io;
    }
    return SDL_IOFromFile(path, mode);
}
static int H_fopen(const char* path, int mode) {
    static const char* const modes[] = { "rb", "wb", "ab", "r+b" };
    for (int i = 0; i < 64; ++i)
        if (!sFiles[i]) {
            sFiles[i] = OpenPath(path, modes[mode & 3]);
            return sFiles[i] ? i : -1;
        }
    return -1;
}
static int H_fread(int h, void* buf, uint32_t n) { return (int)SDL_ReadIO(sFiles[h], buf, n); }
static int H_fwrite(int h, const void* buf, uint32_t n) { return (int)SDL_WriteIO(sFiles[h], buf, n); }
static int H_fseek(int h, int32_t off, int whence) {
    return SDL_SeekIO(sFiles[h], off, whence == 0 ? SDL_IO_SEEK_SET : whence == 1 ? SDL_IO_SEEK_CUR : SDL_IO_SEEK_END) < 0 ? -1 : 0;
}
static int H_ftell(int h) { return (int)SDL_TellIO(sFiles[h]); }
static int H_fclose(int h) {
    bool ok = SDL_CloseIO(sFiles[h]);
    sFiles[h] = NULL;
    return ok ? 0 : -1;
}
static int H_fflush(int h) { return SDL_FlushIO(sFiles[h]) ? 0 : -1; }
static int H_stat(const char* p) {
    struct stat s;
    if (strncmp(p, "apk:/", 5) == 0) {
        SDL_IOStream* io = OpenPath(p, "rb");
        if (!io)
            return 0;
        SDL_CloseIO(io);
        return 1;
    }
    return stat(p, &s) ? 0 : S_ISDIR(s.st_mode) ? 2 : 1;
}
static int H_remove(const char* p) { return remove(p); }
static int H_rename(const char* a, const char* b) { return rename(a, b); }
static int H_mkdir(const char* p) { return mkdir(p, 0777); }
static DIR* sDirs[16];
static int H_dir_open(const char* p) {
    for (int i = 0; i < 16; ++i)
        if (!sDirs[i]) {
            sDirs[i] = opendir(p);
            return sDirs[i] ? i : -1;
        }
    return -1;
}
static int H_dir_next(int d, char* name, int cap) {
    struct dirent* e;
    while ((e = readdir(sDirs[d]))) {
        if (e->d_name[0] == '.')
            continue;
        SDL_strlcpy(name, e->d_name, (size_t)cap);
        return 1;
    }
    return 0;
}
static void H_dir_close(int d) {
    closedir(sDirs[d]);
    sDirs[d] = NULL;
}

/* ---- video: the GPU ------------------------------------------------------------------------------------------- */

static SDL_Window* sWindow;
static SDL_Renderer* sRenderer;
static SDL_Texture* sTex[4096];
static int sTexW[4096], sTexH[4096], sTexSmooth[4096];
static int sBlend = 1;

static void H_gfx_size(int* w, int* h) { SDL_GetCurrentRenderOutputSize(sRenderer, w, h); }
static void H_gfx_begin(uint32_t clear) {
    SDL_SetRenderClipRect(sRenderer, NULL);
    SDL_SetRenderDrawColor(sRenderer, (clear >> 16) & 255, (clear >> 8) & 255, clear & 255, 255);
    if (clear >> 24)
        SDL_RenderClear(sRenderer);
}
static int H_tex_create(const void* rgba, int w, int h, int smooth) {
    for (int i = 0; i < 4096; i++)
        if (!sTex[i]) {
            sTex[i] = SDL_CreateTexture(sRenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, w, h);
            if (!sTex[i])
                return -1;
            SDL_UpdateTexture(sTex[i], NULL, rgba, w * 4);
            SDL_SetTextureBlendMode(sTex[i], SDL_BLENDMODE_BLEND);
            sTexW[i] = w;
            sTexH[i] = h;
            sTexSmooth[i] = smooth;
            return i;
        }
    return -1;
}
static void H_tex_update(int id, const void* rgba) { SDL_UpdateTexture(sTex[id], NULL, rgba, sTexW[id] * 4); }
static void H_tex_free(int id) {
    SDL_DestroyTexture(sTex[id]);
    sTex[id] = NULL;
}
static void H_gfx_clip(int x, int y, int w, int h) {
    SDL_Rect r = { x, y, w, h };
    SDL_SetRenderClipRect(sRenderer, w > 0 ? &r : NULL);
}
static void H_gfx_blend(int on) { sBlend = on; }

/* HostVertex triangles; a u of 2 or more marks crisp (nearest) sampling, as the software hosts read it */
static void H_gfx_draw(int id, const void* verts, int count) {
    const HostVertex* v = verts;
    SDL_Texture* t = id >= 0 && id < 4096 ? sTex[id] : NULL;
    static SDL_Vertex out[8192];
    int crisp = 0;
    if (id >= 0 && !t)
        return;
    if (count > 8192)
        count = 8192;
    if (count > 0 && t) {
        crisp = v[0].u >= 1.75f;
        SDL_SetTextureScaleMode(t, crisp || !sTexSmooth[id] ? SDL_SCALEMODE_NEAREST : SDL_SCALEMODE_LINEAR);
        SDL_SetTextureBlendMode(t, sBlend ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
    }
    for (int i = 0; i < count; i++) {
        out[i].position.x = v[i].x;
        out[i].position.y = v[i].y;
        out[i].tex_coord.x = crisp ? v[i].u - 2.0f : v[i].u;
        out[i].tex_coord.y = v[i].v;
        out[i].color.r = ((v[i].argb >> 16) & 255) / 255.0f;
        out[i].color.g = ((v[i].argb >> 8) & 255) / 255.0f;
        out[i].color.b = (v[i].argb & 255) / 255.0f;
        out[i].color.a = (v[i].argb >> 24) / 255.0f;
    }
    SDL_SetRenderDrawBlendMode(sRenderer, sBlend ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
    SDL_RenderGeometry(sRenderer, t, out, count, NULL, 0);
}

/* notices: assets/<name>.rgba (w, h header) drawn over the game for five seconds */
static SDL_Texture* sToast;
static int sToastW, sToastH;
static Uint64 sToastUntil;
static SDL_Texture* LoadNotice(const char* name, int* w, int* h) {
    char path[160];
    size_t size = 0;
    SDL_snprintf(path, sizeof(path), "%s.rgba", name);
    uint8_t* d = SDL_LoadFile(path, &size);
    SDL_Texture* t = NULL;
    if (d && size > 8) {
        uint32_t ww, hh;
        memcpy(&ww, d, 4);
        memcpy(&hh, d + 4, 4);
        if ((size_t)ww * hh * 4 + 8 <= size) {
            t = SDL_CreateTexture(sRenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, (int)ww, (int)hh);
            SDL_UpdateTexture(t, NULL, d + 8, (int)ww * 4);
            SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
            *w = (int)ww;
            *h = (int)hh;
        }
    }
    SDL_free(d);
    return t;
}
static void H_notify(const char* name) {
    if (sToast)
        SDL_DestroyTexture(sToast);
    sToast = LoadNotice(name, &sToastW, &sToastH);
    sToastUntil = SDL_GetTicks() + 5000;
}
static void DrawBox(SDL_Texture* t, int w, int h, float y0, float alpha) {
    int ow, oh;
    SDL_GetCurrentRenderOutputSize(sRenderer, &ow, &oh);
    float k = ow / 1920.0f;
    SDL_FRect r = { (ow - w * k) / 2, y0 < 0 ? (oh - h * k) / 2 : y0, w * k, h * k };
    SDL_SetRenderClipRect(sRenderer, NULL);
    SDL_SetTextureAlphaModFloat(t, alpha);
    SDL_RenderTexture(sRenderer, t, NULL, &r);
}
static void H_gfx_end(void) {
    if (sToast) {
        Uint64 now = SDL_GetTicks();
        if (now >= sToastUntil) {
            SDL_DestroyTexture(sToast);
            sToast = NULL;
        } else {
            int ow, oh;
            SDL_GetCurrentRenderOutputSize(sRenderer, &ow, &oh);
            float k = ow / 1920.0f, left = (float)(sToastUntil - now);
            DrawBox(sToast, sToastW, sToastH, oh - sToastH * k - 90 * k, left < 400 ? left / 400 : 1);
        }
    }
    {
        /* test harness: KHCOM_HOST_SHOT=frame[,frame...] saves those frames as host_shot_<frame>.bmp in the data folder */
        static unsigned frame;
        const char* shots = SDL_getenv("KHCOM_HOST_SHOT");
        frame++;
        for (const char* p = shots; p && *p;) {
            if ((unsigned)SDL_atoi(p) == frame) {
                SDL_Surface* s = SDL_RenderReadPixels(sRenderer, NULL);
                if (s) {
                    char path[900];
                    SDL_snprintf(path, sizeof(path), "%s/host_shot_%u.bmp", sData, frame);
                    SDL_SaveBMP(s, path);
                    SDL_DestroySurface(s);
                    Log("frame %u saved", frame);
                }
            }
            p = SDL_strchr(p, ',');
            if (p)
                p++;
        }
    }
    SDL_RenderPresent(sRenderer);
}

/* ---- audio ---------------------------------------------------------------------------------------------------- */

static SDL_AudioStream* sAudio;
static int sGameRate = 48000; /* the rate the game opened the audio at */
static int H_audio_open(int rate) {
    sGameRate = rate;
    if (sAudio)
        return 0;
    SDL_AudioSpec spec = { SDL_AUDIO_S16, 2, rate };
    sAudio = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (!sAudio) {
        Log("audio: %s", SDL_GetError());
        return -1;
    }
    SDL_ResumeAudioStreamDevice(sAudio);
    Log("audio: %d Hz", rate);
    return 0;
}
static void GamePush(const int16_t* frames, uint32_t count) {
    if (sAudio && count)
        SDL_PutAudioStreamData(sAudio, frames, (int)count * 4);
}

/* HD music: the host decodes the track the game names and mixes it into the game's blocks (shared/hd_music_host.h) */
#define HD_MUSIC_LOG Log
#include "hd_music_host.h"
static void H_audio_push(const int16_t* frames, uint32_t count) { HdMusicPush(frames, count, sGameRate, GamePush); }
static uint32_t H_audio_queued(void) { return sAudio ? (uint32_t)SDL_GetAudioStreamQueued(sAudio) / 4 : 0; }
static void H_audio_clear(void) {
    if (sAudio)
        SDL_ClearAudioStream(sAudio);
}
static void H_audio_ratio(double ratio) {
    if (sAudio && ratio > 0.5 && ratio < 2.0)
        SDL_SetAudioStreamFrequencyRatio(sAudio, (float)ratio);
}

/* ---- input ---------------------------------------------------------------------------------------------------- */

static SDL_Gamepad* sPad;
static HostTouch sFingers[HOSTTOUCH_MAX];
static int sFingerCount;

static void TouchEvent(const SDL_TouchFingerEvent* e, int down) {
    int i;
    for (i = 0; i < sFingerCount && sFingers[i].id != (uint32_t)e->fingerID; i++) {
    }
    if (!down) {
        if (i < sFingerCount)
            sFingers[i] = sFingers[--sFingerCount];
        return;
    }
    if (i == sFingerCount) {
        if (sFingerCount == HOSTTOUCH_MAX)
            return;
        sFingerCount++;
    }
    sFingers[i].id = (uint32_t)e->fingerID;
    sFingers[i].x = e->x;
    sFingers[i].y = e->y;
}

static void Pump(void) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_EVENT_QUIT: sQuit = 1; break;
            case SDL_EVENT_WILL_ENTER_BACKGROUND:
            case SDL_EVENT_DID_ENTER_BACKGROUND: sBackground = 1; break;
            case SDL_EVENT_WILL_ENTER_FOREGROUND:
            case SDL_EVENT_DID_ENTER_FOREGROUND: sBackground = 0; break;
            case SDL_EVENT_GAMEPAD_ADDED:
                if (!sPad)
                    sPad = SDL_OpenGamepad(e.gdevice.which);
                break;
            case SDL_EVENT_GAMEPAD_REMOVED:
                if (sPad && SDL_GetGamepadID(sPad) == e.gdevice.which) {
                    SDL_CloseGamepad(sPad);
                    sPad = NULL;
                }
                break;
            case SDL_EVENT_FINGER_DOWN:
            case SDL_EVENT_FINGER_MOTION: TouchEvent(&e.tfinger, 1); break;
            case SDL_EVENT_FINGER_UP:
            case SDL_EVENT_FINGER_CANCELED: TouchEvent(&e.tfinger, 0); break;
            default: break;
        }
    }
}

static int H_pad_poll(HostPad* out, int count) {
    memset(out, 0, sizeof(HostPad) * (size_t)count);
    Pump();
    if (count < 1 || !sPad)
        return 0;
    /* RetroPad numbering by position (bridge.h) */
    static const struct { SDL_GamepadButton b; int bit; } map[] = {
        { SDL_GAMEPAD_BUTTON_SOUTH, 0 }, { SDL_GAMEPAD_BUTTON_WEST, 1 }, { SDL_GAMEPAD_BUTTON_BACK, 2 },
        { SDL_GAMEPAD_BUTTON_START, 3 }, { SDL_GAMEPAD_BUTTON_DPAD_UP, 4 }, { SDL_GAMEPAD_BUTTON_DPAD_DOWN, 5 },
        { SDL_GAMEPAD_BUTTON_DPAD_LEFT, 6 }, { SDL_GAMEPAD_BUTTON_DPAD_RIGHT, 7 }, { SDL_GAMEPAD_BUTTON_EAST, 8 },
        { SDL_GAMEPAD_BUTTON_NORTH, 9 }, { SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, 10 },
        { SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, 11 }, { SDL_GAMEPAD_BUTTON_LEFT_STICK, 14 },
        { SDL_GAMEPAD_BUTTON_RIGHT_STICK, 15 }, { SDL_GAMEPAD_BUTTON_TOUCHPAD, 16 },
    };
    out[0].connected = 1;
    for (size_t i = 0; i < sizeof(map) / sizeof(map[0]); i++)
        if (SDL_GetGamepadButton(sPad, map[i].b))
            out[0].buttons |= 1u << map[i].bit;
    out[0].lx = SDL_GetGamepadAxis(sPad, SDL_GAMEPAD_AXIS_LEFTX);
    out[0].ly = SDL_GetGamepadAxis(sPad, SDL_GAMEPAD_AXIS_LEFTY);
    out[0].rx = SDL_GetGamepadAxis(sPad, SDL_GAMEPAD_AXIS_RIGHTX);
    out[0].ry = SDL_GetGamepadAxis(sPad, SDL_GAMEPAD_AXIS_RIGHTY);
    out[0].l2 = (uint8_t)(SDL_GetGamepadAxis(sPad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) >> 7);
    out[0].r2 = (uint8_t)(SDL_GetGamepadAxis(sPad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) >> 7);
    if (out[0].l2 > 64)
        out[0].buttons |= 1u << 12;
    if (out[0].r2 > 64)
        out[0].buttons |= 1u << 13;
    return 1;
}
static int H_touch_poll(HostTouch* fingers, int max) {
    int n = sFingerCount < max ? sFingerCount : max;
    memcpy(fingers, sFingers, sizeof(HostTouch) * (size_t)n);
    return n;
}
static int H_app_state(void) { return sBackground; }
static int H_quit_requested(void) { return sQuit; }
static void H_set_title(const char* s) { (void)s; }

/* ---- the game image ------------------------------------------------------------------------------------------- */

typedef struct {
    uint32_t magic, cputype, cpusubtype, filetype, ncmds, sizeofcmds, flags;
} MachHeader;
typedef struct {
    uint32_t cmd, cmdsize;
    char segname[16];
    uint32_t vmaddr, vmsize, fileoff, filesize, maxprot, initprot, nsects, flags;
} MachSegment;

static int LoadImage(const char* region, Ps5GuestHeader* header) {
    char name[64];
    size_t size = 0;
    SDL_snprintf(name, sizeof(name), "guest-%s.macho", region);
    uint8_t* file = SDL_LoadFile(name, &size);
    const MachHeader* mh = (const MachHeader*)file;
    uint32_t headerAt = 0, end = 0;
    if (!file || size < sizeof(MachHeader) || mh->magic != 0xFEEDFACEu) {
        Log("the game image %s is missing from the APK", name);
        SDL_free(file);
        return 0;
    }
    const uint8_t* lc = file + sizeof(MachHeader);
    for (uint32_t i = 0; i < mh->ncmds; i++) {
        const MachSegment* seg = (const MachSegment*)lc;
        if (seg->cmd == 1 && seg->vmsize && strncmp(seg->segname, "__PAGEZERO", 16) && strncmp(seg->segname, "__LINKEDIT", 16)) {
            size_t page = (size_t)sysconf(_SC_PAGESIZE); /* 4 or 16 KiB; the image's segments are 16 KiB aligned */
            size_t len = (seg->vmsize + page - 1) & ~(page - 1);
            void* p;
            if (seg->vmaddr < IMAGE_BASE || seg->vmaddr + len > IMAGE_LIMIT) {
                Log("segment %.16s at 0x%08x is outside the reserved space", seg->segname, seg->vmaddr);
                SDL_free(file);
                return 0;
            }
            /* over this library's own reservation (MAP_FIXED is safe here: the range is ours) */
            p = mmap((void*)(uintptr_t)seg->vmaddr, len, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | (sReserved ? MAP_FIXED : MAP_FIXED_NOREPLACE), -1, 0);
            if (p != (void*)(uintptr_t)seg->vmaddr) {
                Log("segment %.16s at 0x%08x: %s", seg->segname, seg->vmaddr, strerror(errno));
                SDL_free(file);
                return 0;
            }
            if (seg->filesize)
                memcpy(p, file + seg->fileoff, seg->filesize);
            if (seg->initprot & 4) {
                __builtin___clear_cache((char*)p, (char*)p + len);
                mprotect(p, len, PROT_READ | PROT_EXEC);
            }
            if (!strncmp(seg->segname, "__RKHDR", 16))
                headerAt = seg->vmaddr;
            if (seg->vmaddr + seg->vmsize > end)
                end = seg->vmaddr + seg->vmsize;
        }
        lc += seg->cmdsize;
    }
    SDL_free(file);
    if (!headerAt)
        return 0;
    memcpy(header, (const void*)(uintptr_t)headerAt, sizeof(*header));
    header->imageEnd = header->regionEnd = end;
    Log("game image: %s, %s reservation, ends at 0x%08x", name, sReserved ? "early" : "late", end);
    return header->magic == PS5G_MAGIC;
}

static const char* RegionOf(const char* rom) {
    static const struct { const char* code; const char* region; } known[] = { { "B8CP", "eu" }, { "B8CE", "us" }, { "B8CJ", "jp" } };
    char hdr[0xB0];
    SDL_IOStream* io = SDL_IOFromFile(rom, "rb");
    size_t got = io ? SDL_ReadIO(io, hdr, sizeof(hdr)) : 0;
    if (io)
        SDL_CloseIO(io);
    if (got != sizeof(hdr))
        return NULL;
    for (size_t i = 0; i < 3; i++)
        if (memcmp(hdr + 0xAC, known[i].code, 4) == 0)
            return known[i].region;
    return NULL;
}

int main(int argc, char** argv) {
    const char* rom = NULL;
    char logPath[900];
    for (int i = 1; i + 1 < argc; i++) {
        if (!strcmp(argv[i], "--data-dir"))
            SDL_strlcpy(sData, argv[++i], sizeof(sData));
        else if (!strcmp(argv[i], "--rom"))
            rom = argv[++i];
    }
    SDL_snprintf(logPath, sizeof(logPath), "%s/kh_com_host.log", sData);
    sLog = fopen(logPath, "w");
    Log("ReKHCoM (Android 64-bit) starting");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD))
        Fatal(SDL_GetError());
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
    sWindow = SDL_CreateWindow("Kingdom Hearts: Chain of Memories", 1280, 720, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    sRenderer = sWindow ? SDL_CreateRenderer(sWindow, NULL) : NULL;
    if (!sRenderer)
        Fatal(SDL_GetError());
    SDL_SetRenderVSync(sRenderer, 1);
    Log("renderer: %s", SDL_GetRendererName(sRenderer));

    const char* region = rom ? RegionOf(rom) : NULL;
    if (!region)
        Fatal("Kingdom Hearts: Chain of Memories: the ROM could not be read.");
    SDL_setenv_unsafe("KHCOM_DATA", sData, 1);
    SDL_setenv_unsafe("KHCOM_ASSETS", "apk:", 1);
    SDL_setenv_unsafe("KHCOM_ROM", rom, 1);
    Ps5GuestHeader header;
    if (!LoadImage(region, &header))
        Fatal("The game image could not be loaded. Please send kh_com_host.log (Settings > share log).");

    uint64_t* table = (uint64_t*)(uintptr_t)header.hostTable;
#define HF(name) [HF_##name] = (uint64_t)(uintptr_t)H_##name
#define HM(name, fn) [HF_##name] = (uint64_t)(uintptr_t)(fn)
    uint64_t t[HF_COUNT] = {
        HF(log), HF(exit), HF(ticks_us), HF(sleep_us), HF(getenv), HF(fopen), HF(fread), HF(fwrite), HF(fseek), HF(ftell),
        HF(fclose), HF(fflush), HF(stat), HF(remove), HF(rename), HF(mkdir), HF(dir_open), HF(dir_next), HF(dir_close),
        HF(gfx_size), HF(gfx_begin), HF(gfx_end), HF(tex_create), HF(tex_update), HF(tex_free), HF(gfx_draw), HF(gfx_clip),
        HF(gfx_blend), HF(audio_open), HF(audio_push), HF(audio_queued), HF(audio_clear), HF(audio_ratio), HF(pad_poll),
        HF(quit_requested),
        HM(sin, (double (*)(double))sin), HM(cos, (double (*)(double))cos), HM(tan, (double (*)(double))tan),
        HM(atan2, (double (*)(double, double))atan2), HM(pow, (double (*)(double, double))pow), HM(exp, (double (*)(double))exp),
        HM(log_e, (double (*)(double))log), HM(fmod, (double (*)(double, double))fmod), HM(sinf, (float (*)(float))sinf),
        HM(cosf, (float (*)(float))cosf), HM(powf, (float (*)(float, float))powf), HF(set_title), HF(notify),
        HF(touch_poll), HF(app_state), HF(hd_music),
    };
    for (int i = 0; i < HF_COUNT; ++i)
        if (!t[i]) {
            Log("bridge entry %d has no host function", i);
            return 1;
        }
    memcpy(table, t, sizeof(t));
    Log("starting the game (%d bridge entries)", HF_COUNT);
    int (*boot)(uint32_t) = (int (*)(uint32_t))(uintptr_t)header.boot;
    if (setjmp(sExit) == 0)
        sExitCode = boot(header.stackTop);
    if (sAudio)
        SDL_DestroyAudioStream(sAudio);
    SDL_DestroyRenderer(sRenderer);
    SDL_DestroyWindow(sWindow);
    SDL_Quit();
    return sExitCode;
}
