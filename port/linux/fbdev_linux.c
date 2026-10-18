/*
 * Direct framebuffer output for handhelds (Anbernic and similar).
 *
 * Their GPU drivers are 64-bit only, so a 32-bit process cannot use SDL's
 * KMSDRM backend (it needs EGL/GBM). With KHCOM_FBDEV=/dev/fb0 the game
 * renders with SDL's offscreen driver and software renderer, and each frame
 * is copied to the Linux framebuffer here. Audio (ALSA) and the built-in
 * controls (evdev) still go through SDL.
 */
#include <SDL3/SDL.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "port/port.h"

static uint8_t* sFb;
static size_t sFbSize;
static struct fb_var_screeninfo sVar;
static struct fb_fix_screeninfo sFix;

/* Opens the framebuffer named by KHCOM_FBDEV; returns 0 when not in use. */
int PortFbInit(int* w, int* h) {
    const char* path = SDL_getenv("KHCOM_FBDEV");
    int fd;

    if (path == NULL || *path == '\0')
        return 0;
    fd = open(path, O_RDWR);
    if (fd < 0 || ioctl(fd, FBIOGET_VSCREENINFO, &sVar) != 0 || ioctl(fd, FBIOGET_FSCREENINFO, &sFix) != 0) {
        PortLog("fbdev: cannot open %s", path);
        if (fd >= 0)
            close(fd);
        return 0;
    }
    if (sVar.bits_per_pixel != 32 && sVar.bits_per_pixel != 16) {
        PortLog("fbdev: unsupported %u bits per pixel", sVar.bits_per_pixel);
        close(fd);
        return 0;
    }
    sFbSize = (size_t)sFix.line_length * sVar.yres_virtual;
    sFb = mmap(NULL, sFbSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (sFb == MAP_FAILED) {
        sFb = NULL;
        PortLog("fbdev: cannot map %s", path);
        return 0;
    }
    *w = (int)sVar.xres;
    *h = (int)sVar.yres;
    PortLog("fbdev: %s %ux%u, %u bpp (red at bit %u)", path, sVar.xres, sVar.yres, sVar.bits_per_pixel,
            sVar.red.offset);
    return 1;
}

int PortFbActive(void) {
    return sFb != NULL;
}

/* Copies the renderer's frame to the framebuffer (call before presenting). */
void PortFbPresent(SDL_Renderer* renderer) {
    SDL_Surface* frame;
    SDL_Surface* conv;
    SDL_PixelFormat format;
    uint8_t* dst;
    int y, w, h, bpp;

    if (sFb == NULL)
        return;
    frame = SDL_RenderReadPixels(renderer, NULL);
    if (frame == NULL)
        return;
    if (sVar.bits_per_pixel == 16)
        format = SDL_PIXELFORMAT_RGB565;
    else
        format = sVar.red.offset == 0 ? SDL_PIXELFORMAT_XBGR8888 : SDL_PIXELFORMAT_XRGB8888;
    conv = SDL_ConvertSurface(frame, format);
    SDL_DestroySurface(frame);
    if (conv == NULL)
        return;
    bpp = sVar.bits_per_pixel / 8;
    w = SDL_min(conv->w, (int)sVar.xres);
    h = SDL_min(conv->h, (int)sVar.yres);
    dst = sFb + (size_t)sVar.yoffset * sFix.line_length + (size_t)sVar.xoffset * bpp;
    for (y = 0; y < h; y++)
        memcpy(dst + (size_t)y * sFix.line_length, (uint8_t*)conv->pixels + (size_t)y * conv->pitch, (size_t)w * bpp);
    SDL_DestroySurface(conv);
}
