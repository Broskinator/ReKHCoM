#ifndef PORT_HDSPRITES_H
#define PORT_HDSPRITES_H

/* HD sprites (port_hdsprites.c): cartridge sprites drawn with imported Re:CoM art. */

/* 1-based id of the HD picture for these ROM tiles and frame, 0: none (option off or file missing). */
int PortHdSpriteFind(const void* tiles, const void* frame);
/* The sprite's box in its frame's coordinates (GBA pixels). */
void PortHdSpriteBox(int id, int* x0, int* y0, int* w, int* h);
/* A new picture of a layer (0 the game's picture, 1 the HUD canvas): forget the parts queued for the last one. */
void PortHdSpritesBegin(int layer);
/* Queue one OAM part: its four corners on the layer's canvas (top-left, top-right, bottom-right, bottom-left of the
   part's texture) and the part's box in the frame's coordinates. */
int PortHdSpriteQueue(int layer, int id, const float* x, const float* y, int px, int py, int pw, int ph, int alpha);
#ifdef SDL_h_
/* Draw a layer's queued parts, back to front, over its canvas placed at (ox, oy) with this scale; darken/brighten
   0..16 follow the game's fade. */
void PortHdSpritesDraw(SDL_Renderer* renderer, int layer, float ox, float oy, float scale, int darken, int brighten);
#endif
void PortHdSpritesShutdown(void);

#endif
