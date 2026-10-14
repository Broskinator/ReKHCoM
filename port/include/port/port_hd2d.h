#ifndef PORT_HD2D_H
#define PORT_HD2D_H

#include <stdint.h>

/* HD-2D look (port_hd2d.c), experimental: gPortConfig.hd2d. */
int PortHd2dActive(void);
/* The processed picture: w x h in, larger out. */
const uint32_t* PortHd2dProcess(const uint32_t* src, const uint8_t* spriteMask, int w, int h, int* outW,
                                int* outH);

#endif
