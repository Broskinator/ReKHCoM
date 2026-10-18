#ifndef GUARD_PORT_BATTLE_BACKDROP_H
#define GUARD_PORT_BATTLE_BACKDROP_H

#include <stdint.h>

/* Presentation-only continuation of Guard Armor's authored patio.
 * Called only outside the hardware picture. No game/collision map changes. */
int PortGuardArmorBackdropMatches(const void* const* maps);
int PortGuardArmorBackdropPixel(int x, int y, uint16_t* color);

int PortDarksideBackdropMatches(const void* const* maps);
int PortDarksideBackdropPixel(int x, int y, uint16_t* color);

#endif
