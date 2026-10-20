/**
 * mode_copyright2.c
 * Second Copyright Screen
 */

#ifdef PLATFORM_PC
#include "engine.h"
#endif
#include "registration_data.h"
#include "copyright_screens.h"
#include "fade.h"
#include "display.h"
#include "mode.h"
#include "types.h"
#ifdef PLATFORM_PC
#include "key.h"
#include "gba/keys.h"
#include "port/port_ui.h"
static u8 sCopyright2Skipping;
#endif

static u16 sCopyright2Timer;

void mode_copyright2_0(s32 arg) {
#ifdef PLATFORM_PC
    PortSideStyle(PORT_SIDE_COLOR); /* side bars in the screen's background colour */
#endif
    SetBgMode0();
    SetupBg(0, 0, 0x1F, 0);
    SetBgPriority(0, 3);
    LoadBgTiles(0, gCopyright2Tiles, sizeof(gCopyright2Tiles));
    LoadBgPalette(0, gCopyright2Palette, sizeof(gCopyright2Palette));
    LoadBgMap(0, gCopyright2Map, sizeof(gCopyright2Map));

    if (arg == 0) {
        FadeStartIn(FADE_MODE_BLACK, 0x43);
    } else {
        FadeStartIn(FADE_MODE_WHITE, 0x43);
    }

    sCopyright2Timer = 60;
}

void mode_copyright2_1() {
#ifdef PLATFORM_PC
    /* #24: A twice skips to the title (see mode_copyright1.c) */
    if (!sCopyright2Skipping && PortUiIntroSkip(GetKeysPressed() & A_BUTTON)) {
        sCopyright2Skipping = TRUE;
        FadeStartOut(FADE_MODE_BLACK, 0x10);
    }
    if (sCopyright2Skipping) {
        if (!FadeIsActive()) {
            sCopyright2Skipping = FALSE;
            ModeRequest(&gModeTitle, 0);
        }
        return;
    }
#endif
    if (!FadeIsActive()) {
        if (sCopyright2Timer != 0) {
            if (--sCopyright2Timer == 0) {
                FadeStartOut(FADE_MODE_BLACK, 0x43);
            }
        } else {
            ModeRequest(&gModeTitle, 0);
        }
    }
}

void mode_copyright2_2() {
}

Mode gModeCopyright2 = {
    "mode_copyright2",
    mode_copyright2_0,
    mode_copyright2_1,
    mode_copyright2_2,
};
