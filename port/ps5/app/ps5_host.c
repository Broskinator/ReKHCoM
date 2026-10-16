/* PS5 host: loads the game image below 4 GiB, maps the GBA regions and serves
 * the guest's bridge calls with the platform layer of the PS5 app base
 * (Vulkan presentation, audio and controllers). */
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ucontext.h>
#include <unistd.h>
#include <dirent.h>

#include "platform/platform.h"
#include "platform/ps5_crash.h"
#include "platform/vk/fg_bridge.h"
#include "platform/vk/vk_present.h"
#include <ps5platform/exec.h>

#include "bridge.h"

#define DATA_ROOT "/data/ReKHCom"
#define TITLE_ID "PPSA98511"

int sceSystemServiceLoadExec(const char *path, const char **arguments);
int ps5_klog_capture_stderr(const char *prefix); /* the platform layer: stderr lines go to klog (port 3232) */

/* ---- log ------------------------------------------------------------------ */

static FILE *sLog;

static void Log(const char *fmt, ...)
{
    char line[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);
    size_t n = strlen(line);
    while (n && (line[n - 1] == '\n' || line[n - 1] == '\r'))
        line[--n] = 0;
    fprintf(stderr, "[ReKHCoM] %s\n", line);
    if (sLog)
    {
        fprintf(sLog, "%s\n", line);
        fflush(sLog);
    }
}

/* ---- frame interpolation is not used: the presenter links against these ---- */

FgInterpolator *ssfg_create(PFN_vkVoidFunction (*get)(VkInstance, const char *), VkInstance i, VkPhysicalDevice g, VkDevice d,
                            uint32_t w, uint32_t h, VkFormat f, char *error, size_t error_size)
{
    (void)get, (void)i, (void)g, (void)d, (void)w, (void)h, (void)f;
    snprintf(error, error_size, "not in this build");
    return NULL;
}
void ssfg_destroy(FgInterpolator *fg) { (void)fg; }
void ssfg_prepare(FgInterpolator *fg, VkCommandBuffer cmd) { (void)fg, (void)cmd; }
VkImage ssfg_frame_image(FgInterpolator *fg) { (void)fg; return VK_NULL_HANDLE; }
VkImageView ssfg_frame_view(FgInterpolator *fg) { (void)fg; return VK_NULL_HANDLE; }
bool ssfg_record(FgInterpolator *fg, VkCommandBuffer cmd, bool reset) { (void)fg, (void)cmd, (void)reset; return false; }
VkImageView ssfg_output_view(FgInterpolator *fg) { (void)fg; return VK_NULL_HANDLE; }
bool ssfg_record_n(FgInterpolator *fg, VkCommandBuffer cmd, bool reset, uint32_t n) { (void)fg, (void)cmd, (void)reset, (void)n; return false; }
VkImageView ssfg_generated_view(FgInterpolator *fg, uint32_t i) { (void)fg, (void)i; return VK_NULL_HANDLE; }
void ssfg_advance(FgInterpolator *fg) { (void)fg; }
FgPacing *ssfg_pacing_create(void) { return NULL; }
void ssfg_pacing_destroy(FgPacing *p) { (void)p; }
FgDecision ssfg_pacing_frame(FgPacing *p, uint32_t a, uint32_t b, double c, double d, double e, double f, bool g, uint32_t h)
{
    (void)p, (void)a, (void)b, (void)c, (void)d, (void)e, (void)f, (void)g, (void)h;
    FgDecision none;
    memset(&none, 0, sizeof(none));
    return none;
}
const char *ssfg_pacing_state_name(int s) { (void)s; return ""; }
const char *ssfg_pacing_event(FgPacing *p) { (void)p; return NULL; }

/* The link wraps fclose and fflush (the platform layer's memory streams); the originals are kept under these names. */
int ps5_memstream_fclose(FILE *f);
int ps5_memstream_fflush(FILE *f);
int __wrap_fclose(FILE *f) { return ps5_memstream_fclose(f); }
int __wrap_fflush(FILE *f) { return ps5_memstream_fflush(f); }

/* ---- bridge: process, files ------------------------------------------------ */

static void H_log(const char *s) { Log("%s", s); }

static void H_exit(int code)
{
    Log("the game ended (%d)", code);
    plat_audio_close();
    if (sLog)
        fflush(sLog);
    const char *none[1] = {NULL};
    sceSystemServiceLoadExec("exit", none);
    _exit(code);
}

static uint64_t H_ticks_us(void) { return plat_ticks_us(); }
static void H_sleep_us(uint32_t us) { plat_sleep_us(us); }

static int H_getenv(const char *name, char *out, int cap)
{
    const char *v = getenv(name);
    if (!v)
        return -1;
    snprintf(out, (size_t)cap, "%s", v);
    return (int)strlen(out);
}

static FILE *sFiles[64];
static DIR *sDirs[16];

static int H_fopen(const char *path, int mode)
{
    static const char *const modes[] = {"rb", "wb", "ab", "r+b"};
    for (int i = 0; i < 64; ++i)
        if (!sFiles[i])
        {
            sFiles[i] = fopen(path, modes[mode & 3]);
            return sFiles[i] ? i : -1;
        }
    return -1;
}
static int H_fread(int h, void *buf, uint32_t n) { return (int)fread(buf, 1, n, sFiles[h]); }
static int H_fwrite(int h, const void *buf, uint32_t n) { return (int)fwrite(buf, 1, n, sFiles[h]); }
static int H_fseek(int h, int32_t off, int whence) { return fseek(sFiles[h], off, whence); }
static int H_ftell(int h) { return (int)ftell(sFiles[h]); }
static int H_fclose(int h)
{
    int r = fclose(sFiles[h]);
    sFiles[h] = NULL;
    return r;
}
static int H_fflush(int h) { return fflush(sFiles[h]); }
static int H_stat(const char *p)
{
    struct stat s;
    return stat(p, &s) ? 0 : S_ISDIR(s.st_mode) ? 2 : 1;
}
static int H_remove(const char *p) { return remove(p); }
static int H_rename(const char *a, const char *b) { return rename(a, b); }
static int H_mkdir(const char *p) { return mkdir(p, 0777); }
static int H_dir_open(const char *p)
{
    for (int i = 0; i < 16; ++i)
        if (!sDirs[i])
        {
            sDirs[i] = opendir(p);
            return sDirs[i] ? i : -1;
        }
    return -1;
}
static int H_dir_next(int d, char *name, int cap)
{
    struct dirent *e;
    while ((e = readdir(sDirs[d])))
    {
        if (e->d_name[0] == '.')
            continue;
        snprintf(name, (size_t)cap, "%s", e->d_name);
        return 1;
    }
    return 0;
}
static void H_dir_close(int d)
{
    closedir(sDirs[d]);
    sDirs[d] = NULL;
}

/* ---- bridge: video ---------------------------------------------------------- */

static void H_gfx_size(int *w, int *h)
{
    *w = plat_width();
    *h = plat_height();
}
static void H_gfx_begin(uint32_t clear) { plat_begin_frame(clear); }
static void H_gfx_end(void) { plat_end_frame(); }
static int H_tex_create(const void *rgba, int w, int h, int smooth)
{
    return vkp_ui_texture_create((const uint8_t *)rgba, w, h, smooth != 0, false, true);
}
static void H_tex_update(int id, const void *rgba) { vkp_ui_texture_update(id, (const uint8_t *)rgba); }
static void H_tex_free(int id) { vkp_ui_texture_free(id); }
static void H_gfx_draw(int id, const void *v, int count) { vkp_ui_draw(id, (const VkpUiVertex *)v, count); }
static void H_gfx_clip(int x, int y, int w, int h) { vkp_ui_clip(x, y, w, h); }
static void H_gfx_blend(int on) { vkp_ui_blend(on != 0); }

/* ---- bridge: audio and controllers ----------------------------------------- */

static int H_audio_open(int rate) { return plat_audio_open(rate) ? 0 : -1; }
static void H_audio_push(const int16_t *frames, uint32_t count) { plat_audio_push(frames, count); }
static uint32_t H_audio_queued(void) { return (uint32_t)plat_audio_queued_frames(); }
static void H_audio_clear(void) { plat_audio_clear(); }
static void H_audio_ratio(double ratio) { plat_audio_set_ratio(ratio); }

static bool sQuit;

static int H_pad_poll(HostPad *out, int count)
{
    PadState pads[HOSTPAD_MAX];
    bool quit = false;
    plat_poll(pads, &quit);
    if (quit)
        sQuit = true;
    int n = 0;
    for (int i = 0; i < HOSTPAD_MAX && i < count; ++i)
    {
        memset(&out[i], 0, sizeof(out[i]));
        if (!pads[i].connected)
            continue;
        out[i].connected = 1;
        out[i].buttons = pads[i].buttons;
        out[i].lx = pads[i].lx;
        out[i].ly = pads[i].ly;
        out[i].rx = pads[i].rx;
        out[i].ry = pads[i].ry;
        /* The triggers read 128 at rest on this driver (axis 0, not -32768): only what lies above the rest position counts,
         * and the digital L2 / R2 buttons count as fully pressed. */
        out[i].l2 = (pads[i].buttons & BIT(BTN_L2)) ? 255 : (pads[i].l2 > 128 ? (uint8_t)((pads[i].l2 - 128) * 2) : 0);
        out[i].r2 = (pads[i].buttons & BIT(BTN_R2)) ? 255 : (pads[i].r2 > 128 ? (uint8_t)((pads[i].r2 - 128) * 2) : 0);
        ++n;
    }
    return n;
}
static int H_quit_requested(void) { return sQuit; }
static void H_set_title(const char *s) { (void)s; }

/* ---- guest memory ------------------------------------------------------------- */

static bool MapAt(uintptr_t address, size_t bytes, const char *what)
{
    struct ps5_exec_request request = {.bytes = bytes, .address = address, .anchor = 0, .flags = PS5_EXEC_AT};
    struct ps5_exec_region region;
    memset(&region, 0, sizeof(region));
    ps5_crash_step(what);
    int rc = ps5_exec_alloc(&request, &region);
    Log("memory: %s at 0x%08lx, %zu bytes: rc 0x%08x, base %p, %zu bytes mapped", what, (unsigned long)address, bytes, (unsigned)rc,
        region.base, region.bytes);
    if (rc != 0 || (uintptr_t)region.base != address)
    {
        Log("memory: %s at 0x%08lx (%zu bytes) failed (0x%08x)", what, (unsigned long)address, bytes, (unsigned)rc);
        return false;
    }
    size_t clear = bytes < ((size_t)8 << 20) ? bytes : ((size_t)8 << 20);
    memset(region.base, 0, clear);
    Log("memory: %s cleared (%zu bytes)", what, clear);
    return true;
}

/* An integer division by zero returns 0 on the cartridge and traps on x86: the
 * handler zeroes the quotient and remainder and skips the instruction. */
static int DivLength(const uint8_t *p, int *bytes)
{
    const uint8_t *start = p;
    int size16 = 0;
    while (*p == 0x66 || *p == 0x2E || *p == 0x3E || *p == 0x26 || *p == 0x36 || *p == 0x64 || *p == 0x65 || (*p & 0xF0) == 0x40)
    {
        size16 |= *p == 0x66;
        ++p;
    }
    if (*p != 0xF6 && *p != 0xF7)
        return 0;
    *bytes = *p == 0xF6 ? 1 : (size16 ? 2 : 4);
    uint8_t modrm = p[1];
    if (((modrm >> 3) & 7) < 6)
        return 0;
    int mod = modrm >> 6, rm = modrm & 7;
    p += 2;
    if (mod != 3)
    {
        if (rm == 4)
        {
            uint8_t sib = *p++;
            if (mod == 0 && (sib & 7) == 5)
                p += 4;
        }
        else if (mod == 0 && rm == 5)
            p += 4;
        p += mod == 1 ? 1 : (mod == 2 ? 4 : 0);
    }
    return (int)(p - start);
}

static void OnFpe(int sig, siginfo_t *info, void *context)
{
    ucontext_t *uc = (ucontext_t *)context;
    int bytes = 0, len = DivLength((const uint8_t *)uc->uc_mcontext.mc_rip, &bytes);
    (void)info;
    if (len == 0)
    {
        signal(sig, SIG_DFL);
        raise(sig);
        return;
    }
    if (bytes == 1)
        uc->uc_mcontext.mc_rax &= ~0xFFFFull;
    else if (bytes == 2)
    {
        uc->uc_mcontext.mc_rax &= ~0xFFFFull;
        uc->uc_mcontext.mc_rdx &= ~0xFFFFull;
    }
    else
    {
        uc->uc_mcontext.mc_rax = 0;
        uc->uc_mcontext.mc_rdx = 0;
    }
    uc->uc_mcontext.mc_rip += (unsigned long)len;
}

/* Notices drawn when there is nothing to play (assets/<name>.rgba: w, h, then RGBA). */
typedef struct
{
    int tex, w, h;
} Notice;

static Notice LoadNotice(const char *name)
{
    static const char *const roots[] = {"/app0/assets", "/system_ex/app/" TITLE_ID "/assets", "/data/homebrew/" TITLE_ID "/assets"};
    Notice n = {-1, 0, 0};
    for (size_t i = 0; i < sizeof(roots) / sizeof(roots[0]) && n.tex < 0; ++i)
    {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s.rgba", roots[i], name);
        FILE *f = fopen(path, "rb");
        if (!f)
            continue;
        uint32_t size[2];
        if (fread(size, sizeof(size), 1, f) == 1 && size[0] && size[1] && size[0] <= 4096 && size[1] <= 4096)
        {
            size_t bytes = (size_t)size[0] * size[1] * 4;
            uint8_t *pixels = malloc(bytes);
            if (pixels && fread(pixels, 1, bytes, f) == bytes)
            {
                n.tex = vkp_ui_texture_create(pixels, (int)size[0], (int)size[1], true, false, true);
                n.w = (int)size[0];
                n.h = (int)size[1];
            }
            free(pixels);
        }
        fclose(f);
    }
    return n;
}

/* One frame of a notice, centred and scaled to the screen (1500 px wide at 1920 px). */
static void DrawNotice(const Notice *n)
{
    plat_begin_frame(0xFF0A0A18u);
    if (n->tex >= 0)
    {
        const uint32_t white = 0xFFFFFFFFu;
        float k = plat_width() / 1920.0f, dw = (float)n->w * k, dh = (float)n->h * k;
        float x0 = (plat_width() - dw) / 2, y0 = (plat_height() - dh) / 2;
        VkpUiVertex q[6] = {{x0, y0, 0, 0, white},      {x0 + dw, y0, 1, 0, white},      {x0, y0 + dh, 0, 1, white},
                            {x0 + dw, y0, 1, 0, white}, {x0, y0 + dh, 0, 1, white},      {x0 + dw, y0 + dh, 1, 1, white}};
        vkp_ui_blend(true);
        vkp_ui_draw(n->tex, q, 6);
    }
    plat_end_frame();
}

/* Shows a notice for `frames` screen refreshes, keeping the controllers and system events alive. */
static void ShowNotice(const Notice *n, int frames)
{
    for (int i = 0; i < frames; ++i)
    {
        PadState pads[HOSTPAD_MAX];
        bool quit = false;
        plat_poll(pads, &quit);
        DrawNotice(n);
    }
}

/* The version of the cartridge in the data folder: "eu", "us" or "jp". */
static const char *FindRegion(void)
{
    static const struct
    {
        const char *code, *region;
    } known[] = {{"B8CP", "eu"}, {"B8CE", "us"}, {"B8CJ", "jp"}};
    const char *wanted = getenv("KHCOM_REGION");
    DIR *d = opendir(DATA_ROOT);
    const char *found = NULL;
    while (d)
    {
        struct dirent *e = readdir(d);
        if (!e)
            break;
        size_t n = strlen(e->d_name);
        if (n < 5 || strcasecmp(e->d_name + n - 4, ".gba") != 0)
            continue;
        char path[512], header[0xB0];
        snprintf(path, sizeof(path), DATA_ROOT "/%s", e->d_name);
        FILE *f = fopen(path, "rb");
        if (!f)
            continue;
        size_t got = fread(header, 1, sizeof(header), f);
        fclose(f);
        if (got != sizeof(header))
            continue;
        for (size_t i = 0; i < 3; ++i)
            if (memcmp(header + 0xAC, known[i].code, 4) == 0 && (!wanted || strcmp(wanted, known[i].region) == 0))
            {
                found = known[i].region;
                Log("cartridge: %s (%s)", e->d_name, known[i].region);
                break;
            }
        if (found)
            break;
    }
    if (d)
        closedir(d);
    if (!found && !d)
    {
        /* a sandboxed process cannot list /data: try the usual names */
        for (size_t i = 0; i < 3 && !found; ++i)
        {
            char path[512];
            snprintf(path, sizeof(path), DATA_ROOT "/%s.gba", known[i].code);
            FILE *f = fopen(path, "rb");
            if (f && (!wanted || strcmp(wanted, known[i].region) == 0))
            {
                found = known[i].region;
                Log("cartridge: %s.gba (%s)", known[i].code, known[i].region);
            }
            if (f)
                fclose(f);
        }
    }
    return found;
}

static bool LoadImage(const char *region, Ps5GuestHeader *header)
{
    static const char *const roots[] = {"/app0/assets", "/system_ex/app/" TITLE_ID "/assets", "/data/homebrew/" TITLE_ID "/assets"};
    FILE *f = NULL;
    char path[512];
    for (size_t i = 0; i < sizeof(roots) / sizeof(roots[0]) && !f; ++i)
    {
        snprintf(path, sizeof(path), "%s/guest-%s.bin", roots[i], region);
        f = fopen(path, "rb");
        if (f)
            setenv("KHCOM_ASSETS", roots[i], 1); /* the game's fonts are there too */
    }
    if (!f)
    {
        Log("the game image for %s was not found", region);
        return false;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    Ps5GuestHeader h;
    if (fread(&h, 1, sizeof(h), f) != sizeof(h) || h.magic != PS5G_MAGIC || h.imageEnd < (uint32_t)size)
    {
        fclose(f);
        Log("%s is not a game image", path);
        return false;
    }
    fseek(f, 0, SEEK_SET);
    static const struct
    {
        uintptr_t base;
        size_t bytes;
        const char *name;
    } gba[] = {{0x02000000u, 0x40000, "EWRAM"}, {0x03000000u, 0x10000, "IWRAM"}, {0x04000000u, 0x10000, "IO"},
               {0x05000000u, 0x10000, "palette"}, {0x06000000u, 0x20000, "VRAM"}, {0x07000000u, 0x10000, "OAM"},
               {0x08000000u, 0x02000000, "ROM"}, {0x0E000000u, 0x10000, "SRAM"}};
    for (size_t i = 0; i < sizeof(gba) / sizeof(gba[0]); ++i)
        if (!MapAt(gba[i].base, gba[i].bytes, gba[i].name))
        {
            fclose(f);
            return false;
        }
    if (!MapAt(PS5G_BASE, h.regionEnd - PS5G_BASE, "the game"))
    {
        fclose(f);
        return false;
    }
    ps5_crash_step("read the image");
    bool ok = fread((void *)(uintptr_t)PS5G_BASE, 1, (size_t)size, f) == (size_t)size;
    fclose(f);
    Log("image read: %s", ok ? "ok" : "short");
    if (ok)
    {
        ps5_crash_step("clear the image tail");
        /* the image's bss: everything between the file and imageEnd must read as zero (the first 8 MB were cleared on mapping) */
        for (uint32_t at = PS5G_BASE + (uint32_t)size; at < h.imageEnd;)
        {
            uint32_t n = h.imageEnd - at < (1u << 20) ? h.imageEnd - at : (1u << 20);
            Log("clearing 0x%08x + 0x%x", at, n);
            memset((void *)(uintptr_t)at, 0, n);
            at += n;
        }
        *header = h;
        Log("game image: %s (%ld bytes, ends at 0x%08x)", path, size, h.imageEnd);
        ps5_crash_step("image ready");
    }
    return ok;
}

int main(void)
{
    mkdir(DATA_ROOT, 0777);
    mkdir(DATA_ROOT "/logs", 0777);

    char error[200] = "";
    bool unlocked = plat_prepare_storage(error, sizeof(error));
    plat_want_output(2);
    plat_want_high_refresh(false);
    if (!plat_init())
    {
        char msg[300];
        snprintf(msg, sizeof(msg), "ReKHCoM: %s", plat_init_error());
        plat_notify(msg);
        for (;;)
            plat_sleep_us(1000000);
    }
    mkdir(DATA_ROOT, 0777);
    mkdir(DATA_ROOT "/logs", 0777);
    char log_path[256] = DATA_ROOT "/logs/ReKHCoM.log";
    sLog = fopen(log_path, "w");
    ps5_crash_install(log_path);
    ps5_klog_capture_stderr("");
    Log("ReKHCoM starting; /data %s (%s)", unlocked ? "unlocked" : "sandboxed", error);
    Log("screen: %s", plat_screen_info());

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = OnFpe;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGFPE, &sa, NULL);

    /* Without a cartridge the app stays open and says what is needed; it starts once the file is there. */
    const char *region = FindRegion();
    if (!region)
    {
        Notice need = LoadNotice("need-rom");
        Log("no cartridge found in " DATA_ROOT ": waiting for one");
        while (!(region = FindRegion()))
            ShowNotice(&need, 60);
    }
    Ps5GuestHeader header;
    if (!LoadImage(region, &header))
    {
        Notice failed = LoadNotice("load-error");
        for (;;)
            ShowNotice(&failed, 600);
    }

    Log("image loaded; filling the bridge table at 0x%08x", header.hostTable);
    ps5_crash_step("bridge table");
    uint64_t *table = (uint64_t *)(uintptr_t)header.hostTable;
    uint64_t t[HF_COUNT] = {
        [HF_log] = (uintptr_t)H_log, [HF_exit] = (uintptr_t)H_exit, [HF_ticks_us] = (uintptr_t)H_ticks_us,
        [HF_sleep_us] = (uintptr_t)H_sleep_us, [HF_getenv] = (uintptr_t)H_getenv, [HF_fopen] = (uintptr_t)H_fopen,
        [HF_fread] = (uintptr_t)H_fread, [HF_fwrite] = (uintptr_t)H_fwrite, [HF_fseek] = (uintptr_t)H_fseek,
        [HF_ftell] = (uintptr_t)H_ftell, [HF_fclose] = (uintptr_t)H_fclose, [HF_fflush] = (uintptr_t)H_fflush,
        [HF_stat] = (uintptr_t)H_stat, [HF_remove] = (uintptr_t)H_remove, [HF_rename] = (uintptr_t)H_rename,
        [HF_mkdir] = (uintptr_t)H_mkdir, [HF_dir_open] = (uintptr_t)H_dir_open, [HF_dir_next] = (uintptr_t)H_dir_next,
        [HF_dir_close] = (uintptr_t)H_dir_close, [HF_gfx_size] = (uintptr_t)H_gfx_size,
        [HF_gfx_begin] = (uintptr_t)H_gfx_begin, [HF_gfx_end] = (uintptr_t)H_gfx_end,
        [HF_tex_create] = (uintptr_t)H_tex_create, [HF_tex_update] = (uintptr_t)H_tex_update,
        [HF_tex_free] = (uintptr_t)H_tex_free, [HF_gfx_draw] = (uintptr_t)H_gfx_draw, [HF_gfx_clip] = (uintptr_t)H_gfx_clip,
        [HF_gfx_blend] = (uintptr_t)H_gfx_blend, [HF_audio_open] = (uintptr_t)H_audio_open,
        [HF_audio_push] = (uintptr_t)H_audio_push, [HF_audio_queued] = (uintptr_t)H_audio_queued,
        [HF_audio_clear] = (uintptr_t)H_audio_clear, [HF_audio_ratio] = (uintptr_t)H_audio_ratio,
        [HF_pad_poll] = (uintptr_t)H_pad_poll, [HF_quit_requested] = (uintptr_t)H_quit_requested,
        [HF_sin] = (uintptr_t)(double (*)(double))sin, [HF_cos] = (uintptr_t)(double (*)(double))cos,
        [HF_tan] = (uintptr_t)(double (*)(double))tan, [HF_atan2] = (uintptr_t)(double (*)(double, double))atan2,
        [HF_pow] = (uintptr_t)(double (*)(double, double))pow, [HF_exp] = (uintptr_t)(double (*)(double))exp,
        [HF_log_e] = (uintptr_t)(double (*)(double))log, [HF_fmod] = (uintptr_t)(double (*)(double, double))fmod,
        [HF_sinf] = (uintptr_t)(float (*)(float))sinf, [HF_cosf] = (uintptr_t)(float (*)(float))cosf,
        [HF_powf] = (uintptr_t)(float (*)(float, float))powf, [HF_set_title] = (uintptr_t)H_set_title,
    };
    for (int i = 0; i < HF_COUNT; ++i)
        if (!t[i])
        {
            Log("bridge entry %d has no host function", i);
            return 1;
        }
    memcpy(table, t, sizeof(t));
    Log("bridge table filled (%d entries)", HF_COUNT);

    int (*boot)(uint32_t) = (int (*)(uint32_t))(uintptr_t)header.boot;
    Log("starting the game: boot %p, stack 0x%08x, table %p", (void *)boot, header.stackTop, (void *)table);
    ps5_crash_step("the game");
    int rc = boot(header.stackTop);
    Log("the game returned %d", rc);
    H_exit(rc);
    return rc;
}
