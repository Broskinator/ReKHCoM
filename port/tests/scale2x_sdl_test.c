#include <assert.h>
#include "port/port_scale2x_sdl.h"
int main(void) {
    PortUpscaleLayer layer = {0};
    uint32_t red = 0x80ff0000, clear = 0x0000ff00, resized[2] = {0xffffffff,0xffffffff};
    SDL_FRect src = {1,2,3,4}, scaled;
    SDL_Surface* surface = SDL_CreateSurface(8,8,SDL_PIXELFORMAT_ARGB8888);
    SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(surface);
    SDL_Texture* original;
    SDL_Surface* shot;
    uint32_t* row;
    assert(surface && renderer);
    original = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,1,1);
    assert(original);
    PortUpscaleUpload(&layer,renderer,&red,1,1,SDL_PIXELFORMAT_ARGB8888,1);
    assert(layer.ready && layer.pixels[0] == 0x80800000);
    assert(PortUpscaleSource(&layer,original,&src,&scaled,1) == layer.texture);
    assert(scaled.x==2 && scaled.y==4 && scaled.w==6 && scaled.h==8);
    SDL_SetRenderDrawColor(renderer,255,255,255,255);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer,layer.texture,NULL,NULL);
    shot=SDL_RenderReadPixels(renderer,NULL);
    assert(shot);
    row=(uint32_t*)shot->pixels;
    /* Half-transparent red over white: no dark edge from black padding. */
    assert(((row[0] >> 16)&255) >= 254);
    assert(((row[0] >> 8)&255) >= 126 && ((row[0] >> 8)&255) <= 128);
    SDL_DestroySurface(shot);
    PortUpscaleUpload(&layer,renderer,&clear,1,1,SDL_PIXELFORMAT_ARGB8888,1);
    assert(layer.ready && layer.pixels[0] == 0);
    PortUpscaleUpload(&layer,renderer,resized,2,1,SDL_PIXELFORMAT_XRGB8888,1);
    assert(layer.ready && layer.width==2 && layer.height==1);
    PortUpscaleUpload(&layer,renderer,NULL,0,0,SDL_PIXELFORMAT_ARGB8888,0);
    assert(!layer.ready && !layer.texture && !layer.pixels);
    assert(PortUpscaleSource(&layer,original,&src,&scaled,1)==original && scaled.x==src.x);
    /* Re-enable after releasing resources, then an invalid frame falls back. */
    PortUpscaleUpload(&layer,renderer,&red,1,1,SDL_PIXELFORMAT_ARGB8888,1);
    assert(layer.ready);
    PortUpscaleUpload(&layer,renderer,NULL,0,0,SDL_PIXELFORMAT_ARGB8888,1);
    assert(PortUpscaleSource(&layer,original,NULL,&scaled,1)==original);
    PortUpscaleFree(&layer);
    SDL_DestroyTexture(original);
    SDL_DestroyRenderer(renderer);
    SDL_DestroySurface(surface);
    SDL_Quit();
    return 0;
}
