/**
 * mode_lang.c
 * Language Select Screen
 */

#ifdef PLATFORM_PC
#include "engine.h"
#include "port/port.h"
#endif
#include "mode_lang.h"
#include "sprites_language_select.h"
#include "malloc.h"
#include "fade.h"
#include "songs.h"
#include "gba/keys.h"
#include "display.h"
#include "key.h"
#include "m4a_song.h"
#include "mode.h"
#include "obj_api.h"
#include "registration_data.h"
#include "save_api.h"
#include <stddef.h>
#include "system_state.h"
#include "types.h"
#include "sprite_palettes.h"

#ifdef VERSION_EU

enum LangState {
    LANG_STATE_FADE_IN,
    LANG_STATE_SELECT,
    LANG_STATE_CONFIRM,
    LANG_STATE_FADE_OUT
};

static LangWork* sLangWork;

void mode_lang_0(s32 arg) {
#ifdef PLATFORM_PC
    PortSideStyle(PORT_SIDE_TEXTURE); /* side bars continue the paper texture */
#endif
    sLangWork = EwramAlloc(sizeof(LangWork));
    SetBgMode0();
    SetupBg(0, 0, 29, 0);
    SetupBg(1, 0, 30, 0);
    SetBgPriority(0, 0);
    SetBgPriority(1, 1);
    LoadBgPalette(0, gLanguageSelectPalette, sizeof(gLanguageSelectPalette));
    LoadBgTilesLz77(0, gLanguageSelectTiles);
    LoadBgMapLz77(0, gLanguageSelectMenuMap);
    LoadBgMapLz77(1, gLanguageSelectBgMap);
    sLangWork->tiles = LoadObjTiles(gLanguageSelectCursorTiles, sizeof(gLanguageSelectCursorTiles));
    sLangWork->palette = LoadObjPalette(gLanguageSelectCursorPalette, sizeof(gLanguageSelectCursorPalette));
    sLangWork->timer = 0;
    sLangWork->state = LANG_STATE_FADE_IN;
    sLangWork->flags = 0;
    SaveLoadHeader();

    switch (gLanguage) {
    case LANGUAGE_ENGLISH:
        sLangWork->cursor = 0;
        break;
    case LANGUAGE_ITALIAN:
        sLangWork->cursor = 1;
        break;
    case LANGUAGE_FRENCH:
        sLangWork->cursor = 2;
        break;
    case LANGUAGE_SPANISH:
        sLangWork->cursor = 3;
        break;
    case LANGUAGE_GERMAN:
        sLangWork->cursor = 4;
        break;
    default:
        gLanguage = LANGUAGE_ENGLISH;
        sLangWork->cursor = 0;
        break;
    }

    sLangWork->language = gLanguage;
#ifdef PLATFORM_PC
    {
        /* the language is already known (--language from the launcher or the
           web page, or game_language, kept when the player first chose): the
           screen confirms it as the player would (the save header keeps it)
           and leaves through its own fade-out, without being shown */
        extern int gPortStartupLanguage;
        int known = gPortStartupLanguage >= LANGUAGE_ENGLISH ? gPortStartupLanguage : gPortConfig.gameLanguage - 1;
        if (known >= LANGUAGE_ENGLISH && known <= LANGUAGE_SPANISH) {
            gLanguage = known;
            if (sLangWork->language != gLanguage) {
                SaveWriteHeader(-1);
            }
            FadeToAmount(FADE_MODE_BLACK, 16, 1);
            DisableBg(0);
            DisableBg(1);
            sLangWork->flags |= LANG_FLAG_HIDE_CURSOR;
            sLangWork->state = LANG_STATE_FADE_OUT;
            sLangWork->timer = 1;
            return;
        }
    }
#endif
    FadeStartIn(FADE_MODE_BLACK, 16);
}

void mode_lang_1() {
    switch (sLangWork->state) {
    case LANG_STATE_FADE_IN:
        if (!FadeIsActive()) {
            sLangWork->state = LANG_STATE_SELECT;
        }

        break;
    case LANG_STATE_SELECT:
        if (GetKeysRepeat() & DPAD_UP) {
            sLangWork->cursor--;

            if (sLangWork->cursor < 0) {
                sLangWork->cursor = 4;
            }

            m4aSongNumStart(SONG_SYS_CLICK);
        } else if (GetKeysRepeat() & DPAD_DOWN) {
            sLangWork->cursor++;

            if (sLangWork->cursor > 4) {
                sLangWork->cursor = 0;
            }

            m4aSongNumStart(SONG_SYS_CLICK);
        } else if (GetKeysPressed() & A_BUTTON) {
            sLangWork->timer = 0;
            sLangWork->state = LANG_STATE_CONFIRM;
            m4aSongNumStart(SONG_SYS_KETTEI);
        } else if (GetKeysPressed() & B_BUTTON) {
            sLangWork->state = LANG_STATE_FADE_OUT;
            m4aSongNumStart(SONG_SYS_CANSEL);
        }

        break;
    case LANG_STATE_CONFIRM:
        if (sLangWork->timer == 0) {
            switch (sLangWork->cursor) {
            case 0:
                gLanguage = LANGUAGE_ENGLISH;
                break;
            case 1:
                gLanguage = LANGUAGE_ITALIAN;
                break;
            case 2:
                gLanguage = LANGUAGE_FRENCH;
                break;
            case 3:
                gLanguage = LANGUAGE_SPANISH;
                break;
            case 4:
                gLanguage = LANGUAGE_GERMAN;
                break;
            default:
                gLanguage = LANGUAGE_ENGLISH;
                break;
            }

            if (sLangWork->language != gLanguage) {
                SaveWriteHeader(-1);
            }
#ifdef PLATFORM_PC
            /* remembered: the next start goes straight in (game_language) */
            gPortConfig.gameLanguage = (int)gLanguage + 1;
            PortConfigSave();
#endif
        }

        if (sLangWork->timer % 4 < 2) {
            sLangWork->flags &= ~LANG_FLAG_HIDE_CURSOR;
        } else {
            sLangWork->flags |= LANG_FLAG_HIDE_CURSOR;
        }

        if (sLangWork->timer > 29) {
            sLangWork->flags &= ~LANG_FLAG_HIDE_CURSOR;
            sLangWork->state = LANG_STATE_FADE_OUT;
            sLangWork->timer = 0;
        } else {
            sLangWork->timer++;
        }

        break;
    case LANG_STATE_FADE_OUT:
        if (sLangWork->timer == 0) {
            FadeStartOut(FADE_MODE_BLACK, 16);
        }

        if (!FadeIsActive()) {
            ModeRequest(&gModeCopyright1, 0);
        } else {
            sLangWork->timer++;
        }

        break;
    }

    if (!(sLangWork->flags & LANG_FLAG_HIDE_CURSOR)) {
        switch (sLangWork->cursor) {
        case 0:
            DrawSprite(0x60, 0x58, gLanguageSelectCursorFrame0, sLangWork->tiles, sLangWork->palette, NULL, 0, 0);
            break;
        case 1:
            DrawSprite(0x60, 0x68, gLanguageSelectCursorFrame1, sLangWork->tiles, sLangWork->palette, NULL, 0, 0);
            break;
        case 2:
            DrawSprite(0x60, 0x78, gLanguageSelectCursorFrame2, sLangWork->tiles, sLangWork->palette, NULL, 0, 0);
            break;
        case 3:
            DrawSprite(0x60, 0x88, gLanguageSelectCursorFrame3, sLangWork->tiles, sLangWork->palette, NULL, 0, 0);
            break;
        case 4:
            DrawSprite(0x60, 0x98, gLanguageSelectCursorFrame4, sLangWork->tiles, sLangWork->palette, NULL, 0, 0);
            break;
        }
    }
}

void mode_lang_2() {
    ReleaseObjTiles(sLangWork->tiles);
    ReleaseObjPalette(sLangWork->palette);
    EwramFree(sLangWork);
}

Mode gModeLang = { "mode_lang", mode_lang_0, mode_lang_1, mode_lang_2 };

#endif
