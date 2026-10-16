/* PS5 platform glue. Cartridge saves work as on the other platforms;
 * pointer-based quick states and the GDI HD text are not available here. */
#include <SDL3/SDL.h>
#include <stdint.h>
#include "port/port.h"
#include "port/port_state.h"
#include "port/port_ui.h"
#include "bridge_calls.h"

extern char __start_khdata[], __stop_khdata[], __start_khbss[], __stop_khbss[];
void PortGameSections(char** data, size_t* size, char** bss, size_t* bsize) {
    *data = __start_khdata;
    *size = (size_t)(__stop_khdata - __start_khdata);
    *bss = __start_khbss;
    *bsize = (size_t)(__stop_khbss - __start_khbss);
}

int gPortInMainLoop, gPortStateSlot = 1;
int gPortMmpx; /* the MMPX 3.2 filter on the game picture (Options, saved as mmpx) */
const char* PortStatePath(int slot) { (void)slot; return ""; }
void PortStateRequestSave(const char* path) { (void)path; PortUiNotice("Quick states are not available on PS5"); }
void PortStateRequestLoad(const char* path) { (void)path; PortUiNotice("Quick states are not available on PS5"); }
void PortStateService(void) {}
void PortWatchdogStart(void) {}
void PortCrashHandlerInstall(void) {}
void PortBiosFaultInstall(void) { PortLog("PS5: explicit BIOS_PTR helpers; the host recovers integer divisions by zero"); }
int PortCodepointToSjis(unsigned cp) { return cp < 128 ? (int)cp : 0; }

FILE* popen(const char* cmd, const char* mode) { (void)cmd; (void)mode; return NULL; }
int pclose(FILE* f) { (void)f; return -1; }

int main(int argc, char** argv);

/* Without a settings file the game starts with HD text and the audio filter on (HD text needs the Latin fonts: the
 * Japanese version keeps the cartridge's text). */
static void WriteDefaultSettings(void) {
    const char* path = PortConfigPath("kh_com.ini");
    FILE* f;

    if (h_stat(path) != 0)
        return;
    f = fopen(path, "w");
    if (f == NULL)
        return;
#ifdef VERSION_JP
    fputs("hd_text=0\n", f);
#else
    fputs("hd_text=1\n", f);
#endif
    fputs("smooth_audio=1\n", f);
    fclose(f);
}

/* The Japanese version has no HD text (no CJK font); called every frame in case the settings file says otherwise. */
void PortPs5LockSettings(void) {
#ifdef VERSION_JP
    gPortConfig.hdText = 0;
#endif
}

/* Called on the guest stack by ps5_boot. */
int guest_main(void) {
    static char name[] = "ReKHCoM";
    static char* argv[] = { name, NULL };
    WriteDefaultSettings();
    return main(1, argv);
}
