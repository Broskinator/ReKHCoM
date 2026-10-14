/**
 * port_ps2qol.c
 * PC port: experimental options taken from Kingdom Hearts Re:Chain of
 * Memories (PS2), applied to the GBA game. All of them sit behind one switch,
 * "Re:CoM mode" (ps2_mode), off by default: while it is off nothing here
 * touches the game, which plays exactly as the cartridge (preservation).
 * The settings are in the options menu's Re:CoM page, the launcher and
 * kh_com.ini.
 */
#include <SDL3/SDL.h>
#include "port/port.h"

/* Re:Chain of Memories difficulty. Beginner: Sora takes half damage and
   deals a quarter more; Proud: he takes half again as much and deals less. */
static int DamagePercent(int player) {
    switch (gPortConfig.ps2Difficulty) {
    case 1: return player ? 50 : 125;
    case 2: return player ? 150 : 80;
    default: return 100;
    }
}

int PortScaleDamage(int damage, int player) {
    int scaled;
    /* mode off or Standard: the cartridge's own value, untouched */
    if (!gPortConfig.ps2Mode || gPortConfig.ps2Difficulty == 0)
        return damage;
    scaled = damage * DamagePercent(player) / 100;
    if (scaled < 1)
        scaled = 1;
    if (SDL_getenv("KHCOM_DAMAGELOG"))
        PortLog("damage: %s %d -> %d (difficulty %d)", player ? "player" : "enemy", damage, scaled,
                gPortConfig.ps2Difficulty);
    return scaled;
}

int PortFastText(void) {
    return gPortConfig.ps2Mode && gPortConfig.ps2FastText;
}
