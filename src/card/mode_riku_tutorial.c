/**
 * mode_riku_tutorial.c
 * Sleight Combos and Riku Tutorial Modes
 */

#include "macros.h"
#include "registration_data.h"
#include "card_battle.h"
#include "player_progression.h"
#include "game_state.h"
#include <string.h>
#include "fade.h"
#include "taskpool.h"
#include "card.h"
#include "battle_work.h"
#include "card_api.h"
#include "card_types.h"
#include "mode.h"
#include "mode_battle_data.h"
#include "player_progression_types.h"
#include "types.h"
#include <stddef.h>
#include "mode_riku_tutorial.h"
#include "battle.h"
#include "btl.h"
#include "card_ids.h"
#include "card_label_data.h"
#include "card_message_data.h"

static TaskPool sRikuTutorialTasks;

static s32 sRikuTutorialModeArg;

u8 gRikuDeckTutorialState EWRAM_COMMON(4);

enum RikuDeckTutorialState {
    RIKU_DECK_TUTORIAL_STATE_SHOW_MESSAGE,
    RIKU_DECK_TUTORIAL_STATE_WAIT_MESSAGE
};

void RikuTutorialModeInit(s32 arg) {
    FadeStartIn(FADE_MODE_BLACK, 16);
    sRikuTutorialModeArg = arg;
    TaskPoolInit(&sRikuTutorialTasks, 1);
    gRikuDeckTutorialState = RIKU_DECK_TUTORIAL_STATE_SHOW_MESSAGE;
}

void Mode_riku_btlTutorial_1() {
    u16 tutorialFlags;

    tutorialFlags = gGameState.progression.tutorialFlags | TUTORIAL_FLAG_RIKU_BATTLE;
    gGameState.progression.tutorialFlags = tutorialFlags;
    ModeRequest(&gModeBattle, sRikuTutorialModeArg);
    TaskPoolUpdate(&sRikuTutorialTasks);
    TaskPoolDraw(&sRikuTutorialTasks);
}

void Mode_riku_deckTutorial_1() {
    if (!FadeIsActive()) {
        switch (gRikuDeckTutorialState) {
        case RIKU_DECK_TUTORIAL_STATE_SHOW_MESSAGE:
            if (!IsMessageWindowOpen() && gRikuDeckTutorialState == RIKU_DECK_TUTORIAL_STATE_SHOW_MESSAGE) {
                CreateSysmsgwinTask(&sRikuTutorialTasks, CARD_MSG_RIKU_DECK_TUTORIAL);
                gRikuDeckTutorialState = RIKU_DECK_TUTORIAL_STATE_WAIT_MESSAGE;
            }

            break;
        case RIKU_DECK_TUTORIAL_STATE_WAIT_MESSAGE:
            if (!IsMessageWindowOpen()) {
                gGameState.progression.tutorialFlags |= TUTORIAL_FLAG_RIKU_DECK;
                ModeRequest(&gModeDeck, sRikuTutorialModeArg);
            }

            break;
        }
    }

    TaskPoolUpdate(&sRikuTutorialTasks);
    TaskPoolDraw(&sRikuTutorialTasks);
}

void RikuTutorialModeDestroy() {
    TaskPoolDestroy(&sRikuTutorialTasks);
}

s32 ResolveActiveCardsMove(s32* out) {
    StockKeys stockKeys;
    u8 found[6];
    u8 flag;
    u8 i;
    s32 stockName;
    BtlWork* btl;
#ifdef VERSION_EU
    u8 soraStockActive;
    s32 j;
#endif

    stockKeys = gTutorialEmptyKeys;
    flag = 0;
    memset(found, 0, sizeof(found));

#ifdef VERSION_EU
    soraStockActive = FALSE;
    out[0] = -1;
    out[1] = -1;
    out[2] = -1;
    out[3] = -1;
    out[4] = -1;
    out[5] = -1;
#endif

    for (i = 0; i < gCardBattleState->activeCardCount; i++) {
        stockKeys.keys[i] = gCardBattleState->activeCards[i]->cardDef->catalogNumber;
#ifdef VERSION_EU
        out[i] = gCardBattleState->activeCards[i]->cardDef->move;
#else

        if (out != NULL) {
            out[i] = gCardBattleState->activeCards[i]->cardDef->move;
        }
#endif
    }

#ifdef VERSION_EU
    if (gCardBattleState->activeCardCount == 1 && (soraStockActive = gCardBattleState->soraStockActive) == 0) {
#else
    if (gCardBattleState->activeCardCount == 1) {
#endif
        gCardBattleState->stockMoveCount = 1;

        if ((gGameState.flags & GAME_FLAG_RIKU) && gBtlWork->soraOwnsPlay == TRUE && !(gBtlWork->flags & BTL_FLAG_DARK_MODE)) {
            if (gCardBattleState->darkModeReady == TRUE || gBtlWork->darkPoints > 29) {
                gCardBattleState->darkModeReady = FALSE;
                return MOVE_DARK_MODE;
            }
        }

        return gCardBattleState->activeCards[0]->cardDef->move;
#ifdef VERSION_EU
    } else if (gCardBattleState->activeCardCount == 1 && (btl = gBtlWork)->soraOwnsPlay == TRUE) {
        if (btl->hcEffect == HC_EFFECT_DOUBLE_SLEIGHT) {
            out[0] = out[1] = gCardBattleState->activeCards[0]->cardDef->move;
            gCardBattleState->stockMoveCount = 2;
            return MOVE_STOCK_SEQUENCE;
        } else {
            return gCardBattleState->activeCards[0]->cardDef->move;
        }
#endif
    } else if (gCardBattleState->activeCardCount == 0) {
        gCardBattleState->stockMoveCount = 0;
        return -1;
    } else {
        stockName = LookupStockName(gCardBattleState->activeCards, gCardBattleState->activeCardCount, gCardBattleState->activeValue, &stockKeys, &flag);

        switch (stockName) {
        case STOCK_DARK_MODE:
            gCardBattleState->stockMoveCount = 1;
            return MOVE_DARK_MODE;
        default:
            btl = gBtlWork;

            if (btl->soraOwnsPlay == TRUE) {
                if (btl->hcEffect == HC_EFFECT_DOUBLE_SLEIGHT) {
                    out[0] = out[1] = GetStockMove(stockName);
                    gCardBattleState->stockMoveCount = 2;
                    return MOVE_STOCK_SEQUENCE;
                } else {
                    return GetStockMove(stockName);
                }
            } else {
                if (gRikuBtlWork->hcEffect == HC_EFFECT_DOUBLE_SLEIGHT) {
                    out[0] = out[1] = GetStockMove(stockName);
                    gCardBattleState->stockMoveCount = 2;
                    return MOVE_STOCK_SEQUENCE;
                } else {
                    return GetStockMove(stockName);
                }
            }
        case STOCK_NONE:
#ifndef VERSION_EU
            if (out != NULL) {
#endif
                FindStockPairsInCombo(stockKeys.keys, found);

                if (found[0] == 1) {
                    out[0] = stockKeys.keys[0];
#ifdef VERSION_EU
                    out[1] = MOVE_STOCK_SEQUENCE;
#else
                    out[1] = -1;
#endif
                    out[3] = out[0];
                    out[4] = out[1];
                    out[5] = out[2];
                } else if (found[1] == 1) {
#ifdef VERSION_EU
                    out[1] = MOVE_STOCK_SEQUENCE;
#else
                    out[1] = -1;
#endif
                    out[2] = stockKeys.keys[1];
                    out[3] = out[0];
                    out[4] = out[1];
                    out[5] = out[2];
                } else {
                    out[3] = out[0];
                    out[4] = out[1];
                    out[5] = out[2];

#ifdef VERSION_EU
                    for (j = 0; j < 5; j++) {
                        if (out[j] == -1) {
                            out[j] = out[j + 1];
                            out[j + 1] = -1;
                        }
                    }
#endif
                }

                if (gBtlWork->soraOwnsPlay == TRUE) {
                    if (gBtlWork->hcEffect == HC_EFFECT_DOUBLE_SLEIGHT) {
                        gCardBattleState->stockMoveCount = gCardBattleState->activeCardCount * 2;
                    } else {
                        gCardBattleState->stockMoveCount = gCardBattleState->activeCardCount;
                    }
                } else {
                    if (gRikuBtlWork->hcEffect == HC_EFFECT_DOUBLE_SLEIGHT) {
                        gCardBattleState->stockMoveCount = gCardBattleState->activeCardCount * 2;
                    } else {
                        gCardBattleState->stockMoveCount = gCardBattleState->activeCardCount;
                    }
                }

#ifndef VERSION_EU
            }
#endif

            return MOVE_STOCK_SEQUENCE;
        }
    }
}

s32 ResolveLinkActiveCardsMove(s32* out, s32 side) {
    StockKeys stockKeys;
    u8 found[6];
    u8 flag;
    u8 i;
    s32 stockName;
#ifdef VERSION_EU
    u8 stockActive;
    s32 j;
    s32 move;
#endif

    memset(&stockKeys, 0, sizeof(stockKeys));
    flag = 0;
    memset(found, 0, sizeof(found));

#ifdef VERSION_EU
    stockActive = FALSE;
    out[0] = -1;
    out[1] = -1;
    out[2] = -1;
    out[3] = -1;
    out[4] = -1;
    out[5] = -1;
#endif

    for (i = 0; i < gCardBattleState->activeCardCount; i++) {
        stockKeys.keys[i] = gCardBattleState->activeCards[i]->cardDef->catalogNumber;
#ifdef VERSION_EU
        out[i] = gCardBattleState->activeCards[i]->cardDef->move;
#else

        if (out != NULL) {
            out[i] = gCardBattleState->activeCards[i]->cardDef->move;
        }
#endif
    }

    if (gCardBattleState->activeCardCount == 1) {
#ifdef VERSION_EU
        switch (side) {
        case LINK_SIDE_SELF:
            stockActive = gCardBattleState->soraStockActive;
            break;
        case LINK_SIDE_PARTNER:
            stockActive = gCardBattleState->rikuStockActive;
            break;
        }

        if (!stockActive) {
#endif
        gCardBattleState->stockMoveCount = 1;

        if ((gGameState.flags & GAME_FLAG_RIKU) && gBtlWork->soraOwnsPlay && !(gBtlWork->flags & BTL_FLAG_DARK_MODE)) {
            if (gCardBattleState->darkModeReady == TRUE || gBtlWork->darkPoints > 29) {
                gCardBattleState->darkModeReady = FALSE;
                return MOVE_DARK_MODE;
            }
        }

        return gCardBattleState->activeCards[0]->cardDef->move;
#ifdef VERSION_EU
        } else {
            if (gBtlWork->soraOwnsPlay == TRUE) {
                if (gBtlWork->hcEffect == HC_EFFECT_DOUBLE_SLEIGHT) {
                    out[0] = out[1] = gCardBattleState->activeCards[0]->cardDef->move;
                    gCardBattleState->stockMoveCount = 2;
                    return MOVE_STOCK_SEQUENCE;
                } else {
                    return gCardBattleState->activeCards[0]->cardDef->move;
                }
            } else {
                if (gRikuBtlWork->hcEffect == HC_EFFECT_DOUBLE_SLEIGHT) {
                    out[0] = out[1] = gCardBattleState->activeCards[0]->cardDef->move;
                    gCardBattleState->stockMoveCount = 2;
                    return MOVE_STOCK_SEQUENCE;
                } else {
                    return gCardBattleState->activeCards[0]->cardDef->move;
                }
            }
        }
#endif
    } else if (gCardBattleState->activeCardCount == 0) {
        gCardBattleState->stockMoveCount = 0;
        return -1;
    } else {
        stockName = LookupLinkStockName(gCardBattleState->activeCards, gCardBattleState->activeCardCount, gCardBattleState->activeValue, &stockKeys, &flag, side);

        switch (stockName) {
        case STOCK_DARK_MODE:
            gCardBattleState->stockMoveCount = 1;
            return MOVE_DARK_MODE;
        default:
            if (gBtlWork->soraOwnsPlay == TRUE) {
                if (gBtlWork->hcEffect == HC_EFFECT_DOUBLE_SLEIGHT) {
                    out[0] = out[1] = GetStockMove(stockName);
                    gCardBattleState->stockMoveCount = 2;
                    return MOVE_STOCK_SEQUENCE;
                } else {
                    return GetStockMove(stockName);
                }
            } else {
                if (gRikuBtlWork->hcEffect == HC_EFFECT_DOUBLE_SLEIGHT) {
                    out[0] = out[1] = GetStockMove(stockName);
                    gCardBattleState->stockMoveCount = 2;
                    return MOVE_STOCK_SEQUENCE;
                } else {
                    return GetStockMove(stockName);
                }
            }
        case STOCK_NONE:
#ifndef VERSION_EU
            if (out != NULL) {
#endif
                FindStockPairsInCombo(stockKeys.keys, found);

                if (found[0] == 1) {
                    out[0] = stockKeys.keys[0];
#ifdef VERSION_EU
                    out[1] = MOVE_STOCK_SEQUENCE;
#else
                    out[1] = -1;
#endif
                    out[3] = out[0];
                    out[4] = out[1];
                    out[5] = out[2];
                } else if (found[1] == 1) {
#ifdef VERSION_EU
                    out[1] = MOVE_STOCK_SEQUENCE;
#else
                    out[1] = -1;
#endif
                    out[2] = stockKeys.keys[1];
                    out[3] = out[0];
                    out[4] = out[1];
                    out[5] = out[2];
                } else {
                    out[3] = out[0];
                    out[4] = out[1];
                    out[5] = out[2];

#ifdef VERSION_EU
                    for (j = 0; j < 5; j++) {
                        if (out[j] == -1) {
                            move = out[j];
                            out[j] = out[j + 1];
                            out[j + 1] = move;
                        }
                    }
#endif
                }

                if (gBtlWork->soraOwnsPlay == TRUE) {
                    if (gBtlWork->hcEffect == HC_EFFECT_DOUBLE_SLEIGHT) {
                        gCardBattleState->stockMoveCount = gCardBattleState->activeCardCount * 2;
                    } else {
                        gCardBattleState->stockMoveCount = gCardBattleState->activeCardCount;
                    }
                } else {
                    if (gRikuBtlWork->hcEffect == HC_EFFECT_DOUBLE_SLEIGHT) {
                        gCardBattleState->stockMoveCount = gCardBattleState->activeCardCount * 2;
                    } else {
                        gCardBattleState->stockMoveCount = gCardBattleState->activeCardCount;
                    }
                }

#ifndef VERSION_EU
            }
#endif

            return MOVE_STOCK_SEQUENCE;
        }
    }
}

u8 GetStockMoveCount() {
    // @bug Still called after the card battle frees gCardBattleState (NULL read).
    if (BIOS_PTR(gCardBattleState)->soraStockActive || BIOS_PTR(gCardBattleState)->rikuStockActive) {
        return BIOS_PTR(gCardBattleState)->stockMoveCount;
    }

    return 0;
}

#define SIO_PAIR_CASE(v) \
    case v: \
        keys[i] = key; \
        found[i] = 1; \
        break;
u8 FindStockPairsInCombo(s32* keys, u8* found) {
    u32 catalogNumbers[6];
    u32 key;
    s32 i = 0;

    if (gCardBattleState->activeCardCount <= 2) {
        return 0;
    }

    catalogNumbers[0] = keys[0];
    catalogNumbers[1] = keys[1];
    catalogNumbers[2] = keys[2];
    key = (catalogNumbers[1] << 10) | catalogNumbers[0] | 0x80000000;

    switch (key) {
    SIO_PAIR_CASE(0x8002ACAB)
    SIO_PAIR_CASE(0x8002D4B5)
    SIO_PAIR_CASE(0x8002FCBF)
    SIO_PAIR_CASE(0x800324C9)
    SIO_PAIR_CASE(0x80034CD3)
    SIO_PAIR_CASE(0x800374DD)
    SIO_PAIR_CASE(0x80039CE7)
    SIO_PAIR_CASE(0x8003C4F1)
    SIO_PAIR_CASE(0x8003ECFB)
    SIO_PAIR_CASE(0x80041505)
    SIO_PAIR_CASE(0x80043D0F)
    SIO_PAIR_CASE(0x80046519)
    SIO_PAIR_CASE(0x80048D23)
    SIO_PAIR_CASE(0x8004B52D)
    SIO_PAIR_CASE(0x8004DD37)
    SIO_PAIR_CASE(0x80050541)
    SIO_PAIR_CASE(0x80052D4B)
    SIO_PAIR_CASE(0x80055555)
    SIO_PAIR_CASE(0x80057D5F)
    SIO_PAIR_CASE(0x8005A569)
    SIO_PAIR_CASE(0x8005CD73)
    SIO_PAIR_CASE(0x800A7E9F)
    }

    i++;
    key = catalogNumbers[1] | (catalogNumbers[2] << 10) | 0x80000000;

    switch (key) {
    SIO_PAIR_CASE(0x8002ACAB)
    SIO_PAIR_CASE(0x8002D4B5)
    SIO_PAIR_CASE(0x8002FCBF)
    SIO_PAIR_CASE(0x800324C9)
    SIO_PAIR_CASE(0x80034CD3)
    SIO_PAIR_CASE(0x800374DD)
    SIO_PAIR_CASE(0x80039CE7)
    SIO_PAIR_CASE(0x8003C4F1)
    SIO_PAIR_CASE(0x8003ECFB)
    SIO_PAIR_CASE(0x80041505)
    SIO_PAIR_CASE(0x80043D0F)
    SIO_PAIR_CASE(0x80046519)
    SIO_PAIR_CASE(0x80048D23)
    SIO_PAIR_CASE(0x8004B52D)
    SIO_PAIR_CASE(0x8004DD37)
    SIO_PAIR_CASE(0x80050541)
    SIO_PAIR_CASE(0x80052D4B)
    SIO_PAIR_CASE(0x80055555)
    SIO_PAIR_CASE(0x80057D5F)
    SIO_PAIR_CASE(0x8005A569)
    SIO_PAIR_CASE(0x8005CD73)
    SIO_PAIR_CASE(0x800A7E9F)
    }
}

s32 GetStockMove(s32 stock) {
    // @bug No case for STOCK_SLEIGHT_68 (latent: no lookup returns it).
    switch (stock) {
    case STOCK_FIRA:
        return MOVE_FIRA;
    case STOCK_BLIZZARA:
        return MOVE_BLIZZARA;
    case STOCK_THUNDARA:
        return MOVE_THUNDARA;
    case STOCK_CURA:
        return MOVE_CURA;
    case STOCK_GRAVIRA:
        return MOVE_GRAVIRA;
    case STOCK_STOPRA:
        return MOVE_STOPRA;
    case STOCK_SONIC_BLADE:
        return MOVE_SONIC_BLADE;
    case STOCK_STRIKE_RAID:
        return MOVE_STRIKE_RAID;
    case STOCK_FIRAGA:
        return MOVE_FIRAGA;
    case STOCK_BLIZZAGA:
        return MOVE_BLIZZAGA;
    case STOCK_THUNDAGA:
        return MOVE_THUNDAGA;
    case STOCK_CURAGA:
        return MOVE_CURAGA;
    case STOCK_STOPGA:
        return MOVE_STOPGA;
    case STOCK_GRAVIGA:
        return MOVE_GRAVIGA;
    case STOCK_GOOFY_TORNADO:
        return MOVE_GOOFY_TORNADO;
    case STOCK_GOOFY_CHARGE:
        return MOVE_GOOFY_CHARGE;
    case STOCK_MAGIC:
        return MOVE_MAGIC;
    case STOCK_MAGIC_PAIR:
        return MOVE_MAGIC_PAIR;
    case STOCK_PROUD_ROAR:
        return MOVE_PROUD_ROAR;
    case STOCK_PROUD_ROAR_PAIR:
        return MOVE_PROUD_ROAR_PAIR;
    case STOCK_SHOWTIME:
        return MOVE_SHOWTIME;
    case STOCK_SHOWTIME_PAIR:
        return MOVE_SHOWTIME_PAIR;
    case STOCK_PARADISE:
        return MOVE_PARADISE;
    case STOCK_PARADISE_PAIR:
        return MOVE_PARADISE_PAIR;
    case STOCK_SPLASH:
        return MOVE_SPLASH;
    case STOCK_SPLASH_PAIR:
        return MOVE_SPLASH_PAIR;
    case STOCK_TWINKLE:
        return MOVE_TWINKLE;
    case STOCK_TWINKLE_PAIR:
        return MOVE_TWINKLE_PAIR;
    case STOCK_FLARE_BREATH:
        return MOVE_FLARE_BREATH;
    case STOCK_FLARE_BREATH_PAIR:
        return MOVE_FLARE_BREATH_PAIR;
    case STOCK_OMNISLASH:
        return MOVE_OMNISLASH;
    case STOCK_CROSS_SLASH:
        return MOVE_CROSS_SLASH;
    case STOCK_SANDSTORM:
        return MOVE_SANDSTORM;
    case STOCK_SANDSTORM_PAIR:
        return MOVE_SANDSTORM_PAIR;
    case STOCK_SPIRAL_WAVE:
        return MOVE_SPIRAL_WAVE;
    case STOCK_SPIRAL_WAVE_PAIR:
        return MOVE_SPIRAL_WAVE_PAIR;
    case STOCK_SURPRISE:
        return MOVE_SURPRISE;
    case STOCK_SURPRISE_PAIR:
        return MOVE_SURPRISE_PAIR;
    case STOCK_HUMMINGBIRD:
        return MOVE_HUMMINGBIRD;
    case STOCK_HUMMINGBIRD_PAIR:
        return MOVE_HUMMINGBIRD_PAIR;
    case STOCK_FEROCIOUS_LUNGE:
        return MOVE_FEROCIOUS_LUNGE;
    case STOCK_FEROCIOUS_LUNGE_PAIR:
        return MOVE_FEROCIOUS_LUNGE_PAIR;
    case STOCK_MM_MIRACLE:
        return MOVE_MM_MIRACLE;
    case STOCK_MM_MIRACLE_PAIR:
        return MOVE_MM_MIRACLE_PAIR;
    case STOCK_AERORA:
        return MOVE_AERORA;
    case STOCK_AEROGA:
        return MOVE_AEROGA;
    case STOCK_BLITZ:
        return MOVE_BLITZ;
    case STOCK_ARS_ARCANUM:
        return MOVE_ARS_ARCANUM;
    case STOCK_RAGNAROK:
        return MOVE_RAGNAROK;
    case STOCK_TRINITY_LIMIT:
        return MOVE_TRINITY_LIMIT;
    case STOCK_SLIDING_DASH:
        return MOVE_SLIDING_DASH;
    case STOCK_STUN_IMPACT:
        return MOVE_STUN_IMPACT;
    case STOCK_ZANTETSUKEN:
        return MOVE_ZANTETSUKEN;
    case STOCK_WARP:
        return MOVE_WARP;
    case STOCK_WARPINATOR:
        return MOVE_WARPINATOR;
    case STOCK_TERROR:
        return MOVE_TERROR;
    case STOCK_CONFUSE:
        return MOVE_CONFUSE;
    case STOCK_SLEIGHT_57:
        return MOVE_SLEIGHT_57;
    case STOCK_STOP_RAID:
        return MOVE_STOP_RAID;
    case STOCK_JUDGMENT:
        return MOVE_JUDGMENT;
    case STOCK_REFLECT_RAID:
        return MOVE_REFLECT_RAID;
    case STOCK_FIRE_RAID:
        return MOVE_FIRE_RAID;
    case STOCK_BLIZZARD_RAID:
        return MOVE_BLIZZARD_RAID;
    case STOCK_THUNDER_RAID:
        return MOVE_THUNDER_RAID;
    case STOCK_GRAVITY_RAID:
        return MOVE_GRAVITY_RAID;
    case STOCK_AQUA_SPLASH:
        return MOVE_AQUA_SPLASH;
    case STOCK_HOLY:
        return MOVE_HOLY;
    case STOCK_BLAZING_DONALD:
        return MOVE_BLAZING_DONALD;
    case STOCK_GIFTED_MIRACLE:
        return MOVE_GIFTED_MIRACLE;
    case STOCK_MEGA_FLARE:
        return MOVE_MEGA_FLARE;
    case STOCK_FIRAGA_BREAK:
        return MOVE_FIRAGA_BREAK;
    case STOCK_SHOCK_IMPACT:
        return MOVE_SHOCK_IMPACT;
    case STOCK_IDYLL_ROMP:
        return MOVE_IDYLL_ROMP;
    case STOCK_CROSS_SLASH_PLUS:
        return MOVE_CROSS_SLASH_PLUS;
    case STOCK_HOMING_FIRA:
        return MOVE_HOMING_FIRA;
    case STOCK_HOMING_BLIZZARA:
        return MOVE_HOMING_BLIZZARA;
    case STOCK_SYNCHRO:
        return MOVE_SYNCHRO;
    case STOCK_BIND:
        return MOVE_BIND;
    case STOCK_TORNADO:
        return MOVE_TORNADO;
    case STOCK_QUAKE:
        return MOVE_QUAKE;
    case STOCK_TELEPORT:
        return MOVE_TELEPORT;
    case STOCK_DARK_BREAK:
        return MOVE_DARK_BREAK;
    case STOCK_DARK_FIRAGA:
        return MOVE_DARK_FIRAGA;
    case STOCK_DARK_AURA:
        return MOVE_DARK_AURA;
    case STOCK_DARK_MODE:
        return MOVE_DARK_MODE;
    case STOCK_FIRE_WALL:
        return MOVE_FIRE_WALL;
    case STOCK_CROSS_SLASH_2:
        return MOVE_CROSS_SLASH_2;
    case STOCK_OMNISLASH_2:
        return MOVE_OMNISLASH_2;
    case STOCK_TEMPER_FLARE:
        return MOVE_TEMPER_FLARE;
    case STOCK_FIRAGA_BALL:
        return MOVE_FIRAGA_BALL;
    case STOCK_LIGHTNING_BOLT:
        return MOVE_LIGHTNING_BOLT;
    case STOCK_COMBO_PRESENT:
        return MOVE_COMBO_PRESENT;
    case STOCK_RUSH_PRESENT:
        return MOVE_RUSH_PRESENT;
    case STOCK_DARK_FIRAGA_2:
        return MOVE_DARK_FIRAGA_2;
    case STOCK_FREEZE:
        return MOVE_FREEZE;
    case STOCK_DIAMOND_DUST:
        return MOVE_DIAMOND_DUST_SLEIGHT;
    case STOCK_ICEBURN:
        return MOVE_ICEBURN;
    case STOCK_DARK_AURA_2:
        return MOVE_DARK_AURA_2;
    case STOCK_TELEPORT_RUSH:
        return MOVE_TELEPORT_RUSH;
    case STOCK_FIRETOOTH:
        return MOVE_FIRETOOTH;
    case STOCK_BLOSSOM_SHOWER:
        return MOVE_BLOSSOM_SHOWER;
    case STOCK_DEATHSCYTHE:
        return MOVE_DEATHSCYTHE;
    case STOCK_ROCKSHATTER:
        return MOVE_ROCKSHATTER;
    case STOCK_DARK_SHADOW:
        return MOVE_DARK_SHADOW;
    case STOCK_ROCKSHATTER_2:
        return MOVE_ROCKSHATTER_2;
    case STOCK_ICE_NEEDLES:
        return MOVE_ICE_NEEDLES;
    }
}

u8 AreThreeCardValuesEqual(CardDisplayWork** cards, u8 count) {
    u8 x;
    u8 y;
    u8 z;

    if (count != 3) {
        return FALSE;
    }

    x = cards[0]->value;
    y = cards[1]->value;
    z = cards[2]->value;

    if (x != y) {
        return FALSE;
    }

    if (x != z) {
        return FALSE;
    }

    return TRUE;
}

s32 LookupStockName(CardDisplayWork** cards, u8 count, u8 value, StockKeys* stockKeys, u8* flag) {
    s32 catalogNumbers[3];
    u32 key;
    u8 matchCount;
    u8 i;

    memset(catalogNumbers, 0, sizeof(catalogNumbers));
    matchCount = 0;

    for (i = 0; i < count; i++) {
        catalogNumbers[i] = (*(i + cards))->cardDef->catalogNumber;
    }

    if ((gGameState.flags & GAME_FLAG_RIKU) && gBtlWork->soraOwnsPlay && !(gBtlWork->flags & BTL_FLAG_DARK_MODE)) {
        if (gCardBattleState->darkModeReady == TRUE || gBtlWork->darkPoints > 29) {
            gCardBattleState->darkModeReady = FALSE;
            return STOCK_DARK_MODE;
        }
    }

    key = catalogNumbers[0] | (catalogNumbers[1] << 10) | (catalogNumbers[2] << 20) | (count << 30);

    switch (key) {
    case 0x8002ACAB:
        return STOCK_FIRA;
    case 0x8002FCBF:
        return STOCK_THUNDARA;
    case 0x8002D4B5:
        return STOCK_BLIZZARA;
    case 0x800324C9:
        return STOCK_CURA;
    case 0x80034CD3:
        return STOCK_GRAVIRA;
    case 0x800374DD:
        return STOCK_STOPRA;
    case 0x80039CE7:
        return STOCK_AERORA;
    case 0x8003ECFB:
        return STOCK_GOOFY_CHARGE;
    case 0x8003C4F1:
        return STOCK_MAGIC_PAIR;
    case 0x80041505:
        return STOCK_PROUD_ROAR_PAIR;
    case 0x80043D0F:
        return STOCK_SHOWTIME_PAIR;
    case 0x80046519:
        return STOCK_PARADISE_PAIR;
    case 0x80048D23:
        return STOCK_SPLASH_PAIR;
    case 0x8004B52D:
        return STOCK_TWINKLE_PAIR;
    case 0x8004DD37:
        return STOCK_FLARE_BREATH_PAIR;
    case 0x80050541:
        return STOCK_CROSS_SLASH;
    case 0x80052D4B:
        return STOCK_SANDSTORM_PAIR;
    case 0x80055555:
        return STOCK_SPIRAL_WAVE_PAIR;
    case 0x80057D5F:
        return STOCK_SURPRISE_PAIR;
    case 0x8005A569:
        return STOCK_HUMMINGBIRD_PAIR;
    case 0x8005CD73:
        return STOCK_FEROCIOUS_LUNGE_PAIR;
    case 0x800A7E9F:
        return STOCK_MM_MIRACLE_PAIR;
    case 0xC0100401:
    case 0xC0B02C0B:
    case 0xC1505415:
    case 0xC1F07C1F:
    case 0xC290A429:
    case 0xC330CC33:
    case 0xC3D0F43D:
    case 0xC4711C47:
    case 0xC5114451:
    case 0xC5B16C5B:
    case 0xC6519465:
    case 0xC6F1BC6F:
    case 0xC791E479:
    case 0xC8320C83:
    case 0xC8D2348D:
    case 0xC9725C97:
    case 0xCA1284A1:
        if ((u16)(value - 10) <= 5 && IsStockLearned(LEARNED_STOCK_SLIDING_DASH)) {
            stockKeys->keys[0] = STOCK_SLIDING_DASH;
            return STOCK_SLIDING_DASH;
        }

        if ((u16)(value - 20) <= 3 && IsStockLearned(LEARNED_STOCK_STUN_IMPACT)) {
            stockKeys->keys[0] = STOCK_STUN_IMPACT;
            return STOCK_STUN_IMPACT;
        }

        break;
    case 0xCAB2ACAB:
        return STOCK_FIRAGA;
    case 0xCB52D4B5:
        return STOCK_BLIZZAGA;
    case 0xCBF2FCBF:
        return STOCK_THUNDAGA;
    case 0xCC9324C9:
        return STOCK_CURAGA;
    case 0xCD334CD3:
        return STOCK_GRAVIGA;
    case 0xCDD374DD:
        return STOCK_STOPGA;
    case 0xCE739CE7:
        return STOCK_AEROGA;
    case 0xCFB3ECFB:
        return STOCK_GOOFY_TORNADO;
    case 0xCF13C4F1:
        return STOCK_MAGIC;
    case 0xD0541505:
        return STOCK_PROUD_ROAR;
    case 0xD0F43D0F:
        return STOCK_SHOWTIME;
    case 0xD1946519:
        return STOCK_PARADISE;
    case 0xD2348D23:
        return STOCK_SPLASH;
    case 0xD2D4B52D:
        return STOCK_TWINKLE;
    case 0xD374DD37:
        return STOCK_FLARE_BREATH;
    case 0xD4150541:
        return STOCK_OMNISLASH;
    case 0xD4B52D4B:
        return STOCK_SANDSTORM;
    case 0xD5555555:
        return STOCK_SPIRAL_WAVE;
    case 0xD5F57D5F:
        return STOCK_SURPRISE;
    case 0xD695A569:
        return STOCK_HUMMINGBIRD;
    case 0xD735CD73:
        return STOCK_FEROCIOUS_LUNGE;
    case 0xE9FA7E9F:
        return STOCK_MM_MIRACLE;
    case 0xCE739CDD:
        if (IsStockLearned(LEARNED_STOCK_WARP)) {
            return STOCK_WARP;
        }

        break;
    case 0xCE734CDD:
        if (IsStockLearned(LEARNED_STOCK_WARPINATOR)) {
            return STOCK_WARPINATOR;
        }

        break;
    case 0xCE72ACB5:
        if (IsStockLearned(LEARNED_STOCK_AQUA_SPLASH)) {
            return STOCK_AQUA_SPLASH;
        }

        break;
    case 0xCAB2AD37:
        if (IsStockLearned(LEARNED_STOCK_MEGA_FLARE)) {
            return STOCK_MEGA_FLARE;
        }

        break;
    case 0xCE734CC9:
        if (IsStockLearned(LEARNED_STOCK_SYNCHRO)) {
            return STOCK_SYNCHRO;
        }

        break;
    case 0xE95A5695:
        if (gBtlWork->flags & BTL_FLAG_DARK_MODE) {
            if ((u16)(value - 5) <= 10) {
                return STOCK_DARK_BREAK;
            }

            if ((u16)(value - 16) <= 9) {
                return STOCK_DARK_FIRAGA;
            }

            if (value > 26) {
                return STOCK_DARK_AURA;
            }
        }

        break;
    case 0xF21C8721:
        return STOCK_FIRE_WALL;
    case 0xEB3ACEB3:
        return STOCK_CROSS_SLASH_2;
    case 0xEB3AA6B3:
        return STOCK_OMNISLASH_2;
    case 0xEE5B96E5:
        return STOCK_TEMPER_FLARE;
    case 0xEEFB96EF:
        return STOCK_FIRAGA_BALL;
    case 0xF49D2735:
        return STOCK_LIGHTNING_BOLT;
    case 0xED1AF6BD:
        return STOCK_COMBO_PRESENT;
    case 0xED1B1EC7:
        return STOCK_RUSH_PRESENT;
    case 0xF0DBE6F9:
        return STOCK_DARK_FIRAGA_2;
    case 0xF53D7753:
        return STOCK_FREEZE;
    case 0xF53D4F5D:
        return STOCK_DIAMOND_DUST;
    case 0xF5DD4F53:
        return STOCK_ICEBURN;
    case 0xF17C0F03:
        return STOCK_DARK_AURA_2;
    case 0xF35CFF3F:
        return STOCK_TELEPORT_RUSH;
    case 0xF21CAF21:
        return STOCK_FIRETOOTH;
    case 0xF71D9F71:
        return STOCK_BLOSSOM_SHOWER;
    case 0xF7BDC767:
        return STOCK_DEATHSCYTHE;
    case 0xFADEB7A3:
        return STOCK_ROCKSHATTER;
    case 0xFA3EB7A3:
        return STOCK_DARK_SHADOW;
    case 0xF85E3F85:
        return STOCK_ROCKSHATTER_2;
    case 0xF53D4F53:
        return STOCK_ICE_NEEDLES;
    }

    if ((u8)IsTwoSummonsThenKind(cards, count, CARD_KIND_JACK) || (u8)IsSimbaMushuItem(cards, count)) {
        if (IsStockLearned(LEARNED_STOCK_TERROR)) {
            return STOCK_TERROR;
        }
    }

    if ((u8)IsGenieTinkerBellSummon(cards, count) && IsStockLearned(LEARNED_STOCK_CONFUSE)) {
        return STOCK_CONFUSE;
    }

    if ((u8)IsKindThenTwoAttackCards(cards, CARD_KIND_STOP, count) && IsStockLearned(LEARNED_STOCK_STOP_RAID)) {
        return STOCK_STOP_RAID;
    }

    if ((u8)IsKindThenTwoAttackCards(cards, CARD_KIND_AERO, count) && IsStockLearned(LEARNED_STOCK_JUDGMENT)) {
        return STOCK_JUDGMENT;
    }

    if ((u8)IsKindThenTwoAttackCards(cards, CARD_KIND_CLOUD, count) && IsStockLearned(LEARNED_STOCK_REFLECT_RAID)) {
        return STOCK_REFLECT_RAID;
    }

    if ((u8)IsKindThenTwoAttackCards(cards, CARD_KIND_FIRE, count) && IsStockLearned(LEARNED_STOCK_FIRE_RAID)) {
        return STOCK_FIRE_RAID;
    }

    if ((u8)IsKindThenTwoAttackCards(cards, CARD_KIND_BLIZZARD, count) && IsStockLearned(LEARNED_STOCK_BLIZZARD_RAID)) {
        return STOCK_BLIZZARD_RAID;
    }

    if ((u8)IsKindThenTwoAttackCards(cards, CARD_KIND_THUNDER, count) && IsStockLearned(LEARNED_STOCK_THUNDER_RAID)) {
        return STOCK_THUNDER_RAID;
    }

    if ((u8)IsKindThenTwoAttackCards(cards, CARD_KIND_GRAVITY, count) && IsStockLearned(LEARNED_STOCK_GRAVITY_RAID)) {
        return STOCK_GRAVITY_RAID;
    }

    if ((u8)IsMegaEtherMegalixirItem(cards, count) && IsStockLearned(LEARNED_STOCK_HOLY)) {
        return STOCK_HOLY;
    }

    if ((u8)IsFireDonaldMagic(cards, count) && IsStockLearned(LEARNED_STOCK_BLAZING_DONALD)) {
        return STOCK_BLAZING_DONALD;
    }

    if ((u8)IsSummonMagicJackOrBambiBlizzardItem(cards, count) && IsStockLearned(LEARNED_STOCK_GIFTED_MIRACLE)) {
        return STOCK_GIFTED_MIRACLE;
    }

    if ((u8)IsFireMushuAttack(cards, count) && IsStockLearned(LEARNED_STOCK_FIRAGA_BREAK)) {
        return STOCK_FIRAGA_BREAK;
    }

    if ((u8)IsKindThenTwoOfCategory(cards, CARD_KIND_SIMBA, CARD_CATEGORY_ATTACK, count) && IsStockLearned(LEARNED_STOCK_SHOCK_IMPACT)) {
        return STOCK_SHOCK_IMPACT;
    }

    if ((u8)IsKindThenTwoOfCategory(cards, CARD_KIND_BAMBI, CARD_CATEGORY_ATTACK, count) && IsStockLearned(LEARNED_STOCK_IDYLL_ROMP)) {
        return STOCK_IDYLL_ROMP;
    }

    if ((u8)IsCloudStopAttack(cards, count) && IsStockLearned(LEARNED_STOCK_CROSS_SLASH_PLUS)) {
        return STOCK_CROSS_SLASH_PLUS;
    }

    if ((u8)IsAeroFireMagic(cards, count) && IsStockLearned(LEARNED_STOCK_HOMING_FIRA)) {
        return STOCK_HOMING_FIRA;
    }

    if ((u8)IsAeroBlizzardMagic(cards, count) && IsStockLearned(LEARNED_STOCK_HOMING_BLIZZARA)) {
        return STOCK_HOMING_BLIZZARA;
    }

    if ((u8)IsKindPairThenNonSummonOfCategory(cards, count, CARD_KIND_GRAVITY, CARD_KIND_STOP, CARD_CATEGORY_MAGIC) && IsStockLearned(LEARNED_STOCK_BIND)) {
        return STOCK_BIND;
    }

    if ((u8)IsKindPairThenSummon(cards, count, CARD_KIND_AERO, CARD_KIND_GRAVITY) && IsStockLearned(LEARNED_STOCK_TORNADO)) {
        return STOCK_TORNADO;
    }

    if ((u8)IsKindPairThenNonSummonOfCategory(cards, count, CARD_KIND_GRAVITY, CARD_KIND_SIMBA, CARD_CATEGORY_MAGIC) && IsStockLearned(LEARNED_STOCK_QUAKE)) {
        return STOCK_QUAKE;
    }

    if ((u8)IsTwoMagicThenPeterPan(cards, count) && IsStockLearned(LEARNED_STOCK_TELEPORT)) {
        return STOCK_TELEPORT;
    }

    if ((u8)IsThreeDistinctAttackCards(cards, count)) {
        if ((u16)(value - 10) <= 5 && IsStockLearned(LEARNED_STOCK_BLITZ)) {
            stockKeys->keys[0] = STOCK_BLITZ;
            return STOCK_BLITZ;
        }

        if ((u16)(value - 20) <= 3 && IsStockLearned(LEARNED_STOCK_SONIC_BLADE)) {
            stockKeys->keys[0] = STOCK_SONIC_BLADE;
            return STOCK_SONIC_BLADE;
        }
    }

    if ((u8)IsThreeAttackCardsNoSoulEater(cards, count)) {
        if ((u16)(value - 1) <= 5 && IsStockLearned(LEARNED_STOCK_ARS_ARCANUM)) {
            stockKeys->keys[0] = STOCK_ARS_ARCANUM;
            return STOCK_ARS_ARCANUM;
        }

        if ((u16)(value - 24) <= 2 && IsStockLearned(LEARNED_STOCK_STRIKE_RAID)) {
            stockKeys->keys[0] = STOCK_STRIKE_RAID;
            return STOCK_STRIKE_RAID;
        }

        if ((u16)(value - 7) <= 2 && IsStockLearned(LEARNED_STOCK_RAGNAROK)) {
            stockKeys->keys[0] = STOCK_RAGNAROK;
            return STOCK_RAGNAROK;
        }

        if (value == 0 || value == 27) {
            if (IsStockLearned(LEARNED_STOCK_ZANTETSUKEN)) {
                stockKeys->keys[0] = STOCK_ZANTETSUKEN;
                return STOCK_ZANTETSUKEN;
            }
        }
    }

    if ((u8)IsAttackDonaldGoofyAnyOrder(cards, count) && IsStockLearned(LEARNED_STOCK_TRINITY_LIMIT)) {
        stockKeys->keys[0] = STOCK_TRINITY_LIMIT;
        return STOCK_TRINITY_LIMIT;
    }

    *flag = matchCount;

    if (matchCount == 0) {
        return STOCK_NONE;
    }

    if (matchCount == 1) {
        return stockKeys->keys[0];
    }

    return STOCK_MULTIPLE;
}

s32 LookupLinkStockName(CardDisplayWork** cards, u8 count, u8 value, StockKeys* stockKeys, u8* flag, s32 side) {
    s32 catalogNumbers[3];
    u32 key;
    u8 matchCount;
    u8 i;

    memset(catalogNumbers, 0, sizeof(catalogNumbers));
    matchCount = 0;

    for (i = 0; i < count; i++) {
        catalogNumbers[i] = (*(i + cards))->cardDef->catalogNumber;
    }

    if ((gGameState.flags & GAME_FLAG_RIKU) && gBtlWork->soraOwnsPlay && !(gBtlWork->flags & BTL_FLAG_DARK_MODE)) {
        if (gCardBattleState->darkModeReady == TRUE || gBtlWork->darkPoints > 29) {
            gCardBattleState->darkModeReady = FALSE;
            return STOCK_DARK_MODE;
        }
    }

    key = catalogNumbers[0] | (catalogNumbers[1] << 10) | (catalogNumbers[2] << 20) | (count << 30);

    switch (key) {
    case 0x8002ACAB:
        return STOCK_FIRA;
    case 0x8002FCBF:
        return STOCK_THUNDARA;
    case 0x8002D4B5:
        return STOCK_BLIZZARA;
    case 0x800324C9:
        return STOCK_CURA;
    case 0x80034CD3:
        return STOCK_GRAVIRA;
    case 0x800374DD:
        return STOCK_STOPRA;
    case 0x80039CE7:
        return STOCK_AERORA;
    case 0x8003ECFB:
        return STOCK_GOOFY_CHARGE;
    case 0x8003C4F1:
        return STOCK_MAGIC_PAIR;
    case 0x80041505:
        return STOCK_PROUD_ROAR_PAIR;
    case 0x80043D0F:
        return STOCK_SHOWTIME_PAIR;
    case 0x80046519:
        return STOCK_PARADISE_PAIR;
    case 0x80048D23:
        return STOCK_SPLASH_PAIR;
    case 0x8004B52D:
        return STOCK_TWINKLE_PAIR;
    case 0x8004DD37:
        return STOCK_FLARE_BREATH_PAIR;
    case 0x80050541:
        return STOCK_CROSS_SLASH;
    case 0x80052D4B:
        return STOCK_SANDSTORM_PAIR;
    case 0x80055555:
        return STOCK_SPIRAL_WAVE_PAIR;
    case 0x80057D5F:
        return STOCK_SURPRISE_PAIR;
    case 0x8005A569:
        return STOCK_HUMMINGBIRD_PAIR;
    case 0x8005CD73:
        return STOCK_FEROCIOUS_LUNGE_PAIR;
    case 0x800A7E9F:
        return STOCK_MM_MIRACLE_PAIR;
    case 0xC0100401:
    case 0xC0B02C0B:
    case 0xC1505415:
    case 0xC1F07C1F:
    case 0xC290A429:
    case 0xC330CC33:
    case 0xC3D0F43D:
    case 0xC4711C47:
    case 0xC5114451:
    case 0xC5B16C5B:
    case 0xC6519465:
    case 0xC6F1BC6F:
    case 0xC791E479:
    case 0xC8320C83:
    case 0xC8D2348D:
    case 0xC9725C97:
    case 0xCA1284A1:
        if ((u16)(value - 10) <= 5 && IsLinkSideStockLearned(LEARNED_STOCK_SLIDING_DASH, side)) {
            stockKeys->keys[0] = STOCK_SLIDING_DASH;
            return STOCK_SLIDING_DASH;
        }

        if ((u16)(value - 20) <= 3 && IsLinkSideStockLearned(LEARNED_STOCK_STUN_IMPACT, side)) {
            stockKeys->keys[0] = STOCK_STUN_IMPACT;
            return STOCK_STUN_IMPACT;
        }

        break;
    case 0xCAB2ACAB:
        return STOCK_FIRAGA;
    case 0xCB52D4B5:
        return STOCK_BLIZZAGA;
    case 0xCBF2FCBF:
        return STOCK_THUNDAGA;
    case 0xCC9324C9:
        return STOCK_CURAGA;
    case 0xCD334CD3:
        return STOCK_GRAVIGA;
    case 0xCDD374DD:
        return STOCK_STOPGA;
    case 0xCE739CE7:
        return STOCK_AEROGA;
    case 0xCFB3ECFB:
        return STOCK_GOOFY_TORNADO;
    case 0xCF13C4F1:
        return STOCK_MAGIC;
    case 0xD0541505:
        return STOCK_PROUD_ROAR;
    case 0xD0F43D0F:
        return STOCK_SHOWTIME;
    case 0xD1946519:
        return STOCK_PARADISE;
    case 0xD2348D23:
        return STOCK_SPLASH;
    case 0xD2D4B52D:
        return STOCK_TWINKLE;
    case 0xD374DD37:
        return STOCK_FLARE_BREATH;
    case 0xD4150541:
        return STOCK_OMNISLASH;
    case 0xD4B52D4B:
        return STOCK_SANDSTORM;
    case 0xD5555555:
        return STOCK_SPIRAL_WAVE;
    case 0xD5F57D5F:
        return STOCK_SURPRISE;
    case 0xD695A569:
        return STOCK_HUMMINGBIRD;
    case 0xD735CD73:
        return STOCK_FEROCIOUS_LUNGE;
    case 0xE9FA7E9F:
        return STOCK_MM_MIRACLE;
    case 0xCE739CDD:
        if (IsLinkSideStockLearned(LEARNED_STOCK_WARP, side)) {
            return STOCK_WARP;
        }

        break;
    case 0xCE734CDD:
        if (IsLinkSideStockLearned(LEARNED_STOCK_WARPINATOR, side)) {
            return STOCK_WARPINATOR;
        }

        break;
    case 0xCE72ACB5:
        if (IsLinkSideStockLearned(LEARNED_STOCK_AQUA_SPLASH, side)) {
            return STOCK_AQUA_SPLASH;
        }

        break;
    case 0xCAB2AD37:
        if (IsLinkSideStockLearned(LEARNED_STOCK_MEGA_FLARE, side)) {
            return STOCK_MEGA_FLARE;
        }

        break;
    case 0xCE734CC9:
        if (IsLinkSideStockLearned(LEARNED_STOCK_SYNCHRO, side)) {
            return STOCK_SYNCHRO;
        }

        break;
    case 0xE95A5695:
        if (gBtlWork->flags & BTL_FLAG_DARK_MODE) {
            if ((u16)(value - 5) <= 10) {
                return STOCK_DARK_BREAK;
            }

            if ((u16)(value - 16) <= 9) {
                return STOCK_DARK_FIRAGA;
            }

            if (value > 26) {
                return STOCK_DARK_AURA;
            }
        }

        break;
    case 0xF21C8721:
        return STOCK_FIRE_WALL;
    case 0xEB3ACEB3:
        return STOCK_CROSS_SLASH_2;
    case 0xEB3AA6B3:
        return STOCK_OMNISLASH_2;
    case 0xEE5B96E5:
        return STOCK_TEMPER_FLARE;
    case 0xEEFB96EF:
        return STOCK_FIRAGA_BALL;
    case 0xF49D2735:
        return STOCK_LIGHTNING_BOLT;
    case 0xED1AF6BD:
        return STOCK_COMBO_PRESENT;
    case 0xED1B1EC7:
        return STOCK_RUSH_PRESENT;
    case 0xF0DBE6F9:
        return STOCK_DARK_FIRAGA_2;
    case 0xF53D7753:
        return STOCK_FREEZE;
    case 0xF53D4F5D:
        return STOCK_DIAMOND_DUST;
    case 0xF5DD4F53:
        return STOCK_ICEBURN;
    case 0xF17C0F03:
        return STOCK_DARK_AURA_2;
    case 0xF35CFF3F:
        return STOCK_TELEPORT_RUSH;
    case 0xF21CAF21:
        return STOCK_FIRETOOTH;
    case 0xF71D9F71:
        return STOCK_BLOSSOM_SHOWER;
    case 0xF7BDC767:
        return STOCK_DEATHSCYTHE;
    case 0xFADEB7A3:
        return STOCK_ROCKSHATTER;
    case 0xFA3EB7A3:
        return STOCK_DARK_SHADOW;
    case 0xF85E3F85:
        return STOCK_ROCKSHATTER_2;
    case 0xF53D4F53:
        return STOCK_ICE_NEEDLES;
    }

    if ((u8)IsTwoSummonsThenKind(cards, count, CARD_KIND_JACK) || (u8)IsSimbaMushuItem(cards, count)) {
        if (IsLinkSideStockLearned(LEARNED_STOCK_TERROR, side)) {
            return STOCK_TERROR;
        }
    }

    if ((u8)IsGenieTinkerBellSummon(cards, count) && IsLinkSideStockLearned(LEARNED_STOCK_CONFUSE, side)) {
        return STOCK_CONFUSE;
    }

    if ((u8)IsKindThenTwoAttackCards(cards, CARD_KIND_STOP, count) && IsLinkSideStockLearned(LEARNED_STOCK_STOP_RAID, side)) {
        return STOCK_STOP_RAID;
    }

    if ((u8)IsKindThenTwoAttackCards(cards, CARD_KIND_AERO, count) && IsLinkSideStockLearned(LEARNED_STOCK_JUDGMENT, side)) {
        return STOCK_JUDGMENT;
    }

    if ((u8)IsKindThenTwoAttackCards(cards, CARD_KIND_CLOUD, count) && IsLinkSideStockLearned(LEARNED_STOCK_REFLECT_RAID, side)) {
        return STOCK_REFLECT_RAID;
    }

    if ((u8)IsKindThenTwoAttackCards(cards, CARD_KIND_FIRE, count) && IsLinkSideStockLearned(LEARNED_STOCK_FIRE_RAID, side)) {
        return STOCK_FIRE_RAID;
    }

    if ((u8)IsKindThenTwoAttackCards(cards, CARD_KIND_BLIZZARD, count) && IsLinkSideStockLearned(LEARNED_STOCK_BLIZZARD_RAID, side)) {
        return STOCK_BLIZZARD_RAID;
    }

    if ((u8)IsKindThenTwoAttackCards(cards, CARD_KIND_THUNDER, count) && IsLinkSideStockLearned(LEARNED_STOCK_THUNDER_RAID, side)) {
        return STOCK_THUNDER_RAID;
    }

    if ((u8)IsKindThenTwoAttackCards(cards, CARD_KIND_GRAVITY, count) && IsLinkSideStockLearned(LEARNED_STOCK_GRAVITY_RAID, side)) {
        return STOCK_GRAVITY_RAID;
    }

    if ((u8)IsMegaEtherMegalixirItem(cards, count) && IsLinkSideStockLearned(LEARNED_STOCK_HOLY, side)) {
        return STOCK_HOLY;
    }

    if ((u8)IsFireDonaldMagic(cards, count) && IsLinkSideStockLearned(LEARNED_STOCK_BLAZING_DONALD, side)) {
        return STOCK_BLAZING_DONALD;
    }

    if ((u8)IsSummonMagicJackOrBambiBlizzardItem(cards, count) && IsLinkSideStockLearned(LEARNED_STOCK_GIFTED_MIRACLE, side)) {
        return STOCK_GIFTED_MIRACLE;
    }

    if ((u8)IsFireMushuAttack(cards, count) && IsLinkSideStockLearned(LEARNED_STOCK_FIRAGA_BREAK, side)) {
        return STOCK_FIRAGA_BREAK;
    }

    if ((u8)IsKindThenTwoOfCategory(cards, CARD_KIND_SIMBA, CARD_CATEGORY_ATTACK, count) && IsLinkSideStockLearned(LEARNED_STOCK_SHOCK_IMPACT, side)) {
        return STOCK_SHOCK_IMPACT;
    }

    if ((u8)IsKindThenTwoOfCategory(cards, CARD_KIND_BAMBI, CARD_CATEGORY_ATTACK, count) && IsLinkSideStockLearned(LEARNED_STOCK_IDYLL_ROMP, side)) {
        return STOCK_IDYLL_ROMP;
    }

    if ((u8)IsCloudStopAttack(cards, count) && IsLinkSideStockLearned(LEARNED_STOCK_CROSS_SLASH_PLUS, side)) {
        return STOCK_CROSS_SLASH_PLUS;
    }

    if ((u8)IsAeroFireMagic(cards, count) && IsLinkSideStockLearned(LEARNED_STOCK_HOMING_FIRA, side)) {
        return STOCK_HOMING_FIRA;
    }

    if ((u8)IsAeroBlizzardMagic(cards, count) && IsLinkSideStockLearned(LEARNED_STOCK_HOMING_BLIZZARA, side)) {
        return STOCK_HOMING_BLIZZARA;
    }

    if ((u8)IsKindPairThenNonSummonOfCategory(cards, count, CARD_KIND_GRAVITY, CARD_KIND_STOP, CARD_CATEGORY_MAGIC) && IsLinkSideStockLearned(LEARNED_STOCK_BIND, side)) {
        return STOCK_BIND;
    }

    if ((u8)IsKindPairThenSummon(cards, count, CARD_KIND_AERO, CARD_KIND_GRAVITY) && IsLinkSideStockLearned(LEARNED_STOCK_TORNADO, side)) {
        return STOCK_TORNADO;
    }

    if ((u8)IsKindPairThenNonSummonOfCategory(cards, count, CARD_KIND_GRAVITY, CARD_KIND_SIMBA, CARD_CATEGORY_MAGIC) && IsLinkSideStockLearned(LEARNED_STOCK_QUAKE, side)) {
        return STOCK_QUAKE;
    }

    if ((u8)IsTwoMagicThenPeterPan(cards, count) && IsLinkSideStockLearned(LEARNED_STOCK_TELEPORT, side)) {
        return STOCK_TELEPORT;
    }

    if ((u8)IsThreeDistinctAttackCards(cards, count)) {
        if ((u16)(value - 10) <= 5 && IsLinkSideStockLearned(LEARNED_STOCK_BLITZ, side)) {
            stockKeys->keys[0] = STOCK_BLITZ;
            return STOCK_BLITZ;
        }

        if ((u16)(value - 20) <= 3 && IsLinkSideStockLearned(LEARNED_STOCK_SONIC_BLADE, side)) {
            stockKeys->keys[0] = STOCK_SONIC_BLADE;
            return STOCK_SONIC_BLADE;
        }
    }

    if ((u8)IsThreeAttackCardsNoSoulEater(cards, count)) {
        if ((u16)(value - 1) <= 5 && IsLinkSideStockLearned(LEARNED_STOCK_ARS_ARCANUM, side)) {
            stockKeys->keys[0] = STOCK_ARS_ARCANUM;
            return STOCK_ARS_ARCANUM;
        }

        if ((u16)(value - 24) <= 2 && IsLinkSideStockLearned(LEARNED_STOCK_STRIKE_RAID, side)) {
            stockKeys->keys[0] = STOCK_STRIKE_RAID;
            return STOCK_STRIKE_RAID;
        }

        if ((u16)(value - 7) <= 2 && IsLinkSideStockLearned(LEARNED_STOCK_RAGNAROK, side)) {
            stockKeys->keys[0] = STOCK_RAGNAROK;
            return STOCK_RAGNAROK;
        }

        if (value == 0 || value == 27) {
            if (IsLinkSideStockLearned(LEARNED_STOCK_ZANTETSUKEN, side)) {
                stockKeys->keys[0] = STOCK_ZANTETSUKEN;
                return STOCK_ZANTETSUKEN;
            }
        }
    }

    if ((u8)IsAttackDonaldGoofyAnyOrder(cards, count) && IsLinkSideStockLearned(LEARNED_STOCK_TRINITY_LIMIT, side)) {
        stockKeys->keys[0] = STOCK_TRINITY_LIMIT;
        return STOCK_TRINITY_LIMIT;
    }

    *flag = matchCount;

    if (matchCount == 0) {
        return STOCK_NONE;
    }

    if (matchCount == 1) {
        return stockKeys->keys[0];
    }

    return STOCK_MULTIPLE;
}

s32 IsThreeDistinctAttackCards(CardDisplayWork** cards, u8 count) {
    const CardDef* firstDef;
    const CardDef* secondDef;
    const CardDef* thirdDef;
    u16 firstKind;
    u16 secondKind;
    u16 thirdKind;

    if (count == 3) {
        firstDef = cards[0]->cardDef;
        firstKind = firstDef->kind;
        secondDef = cards[1]->cardDef;
        secondKind = secondDef->kind;
        thirdDef = cards[2]->cardDef;
        thirdKind = thirdDef->kind;

        if (firstDef->category == CARD_CATEGORY_ATTACK && secondDef->category == CARD_CATEGORY_ATTACK && thirdDef->category == CARD_CATEGORY_ATTACK &&
            firstKind != secondKind && firstKind != thirdKind && secondKind != thirdKind) {
            return TRUE;
        }
    }

    return FALSE;
}

s32 IsThreeAttackCardsNoSoulEater(CardDisplayWork** cards, u8 count) {
    const CardDef* firstDef;
    const CardDef* secondDef;
    const CardDef* thirdDef;

    if (count == 3) {
        firstDef = cards[0]->cardDef;
        secondDef = cards[1]->cardDef;
        thirdDef = cards[2]->cardDef;

        if (firstDef->move != MOVE_SOUL_EATER && secondDef->move != MOVE_SOUL_EATER && thirdDef->move != MOVE_SOUL_EATER &&
            firstDef->category == CARD_CATEGORY_ATTACK && secondDef->category == CARD_CATEGORY_ATTACK && thirdDef->category == CARD_CATEGORY_ATTACK) {
            return TRUE;
        }
    }

    return FALSE;
}

s32 IsAttackDonaldGoofyAnyOrder(CardDisplayWork** cards, u8 count) {
    const CardDef* firstDef;
    u16 firstKind;
    u16 secondKind;
    u16 thirdKind;

    if (count == 3) {
        firstDef = cards[0]->cardDef;
        firstKind = firstDef->kind;
        secondKind = cards[1]->cardDef->kind;
        thirdKind = cards[2]->cardDef->kind;

        if (firstKind != secondKind && secondKind != thirdKind && thirdKind != firstKind) {
            if (firstDef->category == CARD_CATEGORY_ATTACK) {
                if (secondKind == CARD_KIND_DONALD_DUCK && thirdKind == CARD_KIND_GOOFY) {
                    return TRUE;
                }

                if (secondKind == CARD_KIND_GOOFY && thirdKind == CARD_KIND_DONALD_DUCK) {
                    return TRUE;
                }
            }

            if (cards[1]->cardDef->category == CARD_CATEGORY_ATTACK) {
                if (firstKind == CARD_KIND_DONALD_DUCK && thirdKind == CARD_KIND_GOOFY) {
                    return TRUE;
                }

                if (firstKind == CARD_KIND_GOOFY && thirdKind == CARD_KIND_DONALD_DUCK) {
                    return TRUE;
                }
            }

            if (cards[2]->cardDef->category == CARD_CATEGORY_ATTACK) {
                if (secondKind == CARD_KIND_DONALD_DUCK && firstKind == CARD_KIND_GOOFY) {
                    return TRUE;
                }

                if (secondKind == CARD_KIND_GOOFY && firstKind == CARD_KIND_DONALD_DUCK) {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

s32 IsKindPairThenNonSummonOfCategory(CardDisplayWork** cards, u8 count, u16 firstKind, u16 secondKind, u8 category) {
    // @bug Outside EU, reads three cards even when fewer are stocked (NULL read).
    if (
#ifdef VERSION_EU
        count == 3 &&
#endif
        cards[0]->cardDef->kind == firstKind && cards[1]->cardDef->kind == secondKind &&
        cards[2]->cardDef->category == category && !(cards[2]->cardDef->flags & CARD_DEF_FLAG_SUMMON)) {
        return TRUE;
    }

    return FALSE;
}

s32 IsKindPairThenSummon(CardDisplayWork** cards, u8 count, u16 firstKind, u16 secondKind) {
    // @bug Outside EU, reads three cards even when fewer are stocked (NULL read).
    if (
#ifdef VERSION_EU
        count == 3 &&
#endif
        cards[0]->cardDef->kind == firstKind && cards[1]->cardDef->kind == secondKind &&
        (cards[2]->cardDef->flags & CARD_DEF_FLAG_SUMMON)) {
        return TRUE;
    }

    return FALSE;
}

s32 IsKindThenTwoAttackCards(CardDisplayWork** cards, u16 kind, u8 count) {
    const CardDef* secondDef;
    const CardDef* thirdDef;
    u16 firstKind;
    u8 secondCategory;
    u8 thirdCategory;

    if (count == 3) {
        firstKind = cards[0]->cardDef->kind;
        secondDef = cards[1]->cardDef;
        thirdDef = cards[2]->cardDef;
        secondCategory = secondDef->category;
        thirdCategory = thirdDef->category;

        if (secondCategory == CARD_CATEGORY_ATTACK && thirdCategory == CARD_CATEGORY_ATTACK && firstKind == kind) {
            return TRUE;
        }
    }

    return FALSE;
}

s32 IsKindThenTwoOfCategory(CardDisplayWork** cards, u16 kind, u8 category, u8 count) {
    const CardDef* secondDef;
    const CardDef* thirdDef;
    u16 firstKind;
    u8 secondCategory;
    u8 thirdCategory;

    if (count == 3) {
        firstKind = cards[0]->cardDef->kind;
        secondDef = cards[1]->cardDef;
        thirdDef = cards[2]->cardDef;
        secondCategory = secondDef->category;
        thirdCategory = thirdDef->category;

        if (firstKind == kind && secondCategory == category && thirdCategory == secondCategory) {
            return TRUE;
        }
    }

    return FALSE;
}

#ifndef VERSION_EU
s32 IsAnyThenTwoGenie(CardDisplayWork** cards, u8 count) {
    u16 firstKind;
    u16 secondKind;
    u16 thirdKind;

    if (count == 3) {
        firstKind = cards[0]->cardDef->kind;
        secondKind = cards[1]->cardDef->kind;
        thirdKind = cards[2]->cardDef->kind;

        if (secondKind == thirdKind && secondKind == CARD_KIND_GENIE && firstKind != CARD_KIND_GENIE) {
            return TRUE;
        }
    }

    return FALSE;
}
#endif

s32 IsFireMushuAttack(CardDisplayWork** cards, u8 count) {
    u16 firstKind;
    u16 secondKind;

    if (count == 3) {
        firstKind = cards[0]->cardDef->kind;
        secondKind = cards[1]->cardDef->kind;

        if (firstKind == CARD_KIND_FIRE && secondKind == CARD_KIND_MUSHU && cards[2]->cardDef->category == CARD_CATEGORY_ATTACK) {
            return TRUE;
        }
    }

    return FALSE;
}

s32 IsTwoSummonsThenKind(CardDisplayWork** cards, u8 count, u16 kind) {
    u16 firstFlags;
    u16 secondFlags;

    if (count == 3) {
        firstFlags = cards[0]->cardDef->flags;
        secondFlags = cards[1]->cardDef->flags;

        if ((firstFlags & 4) && (secondFlags & 4) && cards[2]->cardDef->kind == kind) {
            return TRUE;
        }
    }

    return FALSE;
}

s32 IsSimbaMushuItem(CardDisplayWork** cards, u8 count) {
    const CardDef* thirdDef;
    u16 firstKind;
    u16 secondKind;

    if (count == 3) {
        firstKind = cards[0]->cardDef->kind;
        secondKind = cards[1]->cardDef->kind;

        if (firstKind == CARD_KIND_SIMBA && secondKind == CARD_KIND_MUSHU) {
            thirdDef = cards[2]->cardDef;

            if (thirdDef->category == CARD_CATEGORY_ITEM && !(thirdDef->flags & CARD_DEF_FLAG_FRIEND)) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

s32 IsSummonMagicJackOrBambiBlizzardItem(CardDisplayWork** cards, u8 count) {
    const CardDef* secondDef;
    u16 firstFlags;
    u16 secondFlags;

    if (count == 3) {
        firstFlags = cards[0]->cardDef->flags;
        secondDef = cards[1]->cardDef;
        secondFlags = secondDef->flags;

        if ((firstFlags & 4) && secondFlags == 0 && secondDef->category == CARD_CATEGORY_MAGIC &&
            cards[2]->cardDef->kind == CARD_KIND_JACK) {
            return TRUE;
        }

        if (cards[0]->cardDef->kind == CARD_KIND_BAMBI && cards[1]->cardDef->kind == CARD_KIND_BLIZZARD &&
            cards[2]->cardDef->category == CARD_CATEGORY_ITEM && !(cards[2]->cardDef->flags & CARD_DEF_FLAG_FRIEND)) {
            return TRUE;
        }
    }

    return FALSE;
}

s32 IsGenieTinkerBellSummon(CardDisplayWork** cards, u8 count) {
    u16 firstKind;
    u16 secondKind;
    u16 thirdFlags;

    if (count == 3) {
        firstKind = cards[0]->cardDef->kind;
        secondKind = cards[1]->cardDef->kind;
        thirdFlags = cards[2]->cardDef->flags;

        if (firstKind == CARD_KIND_GENIE && secondKind == CARD_KIND_TINKER_BELL && (thirdFlags & 4)) {
            return TRUE;
        }
    }

    return FALSE;
}

s32 IsMegaEtherMegalixirItem(CardDisplayWork** cards, u8 count) {
    const CardDef* thirdDef;
    u16 firstKind;
    u16 secondKind;
    u8 thirdCategory;

    if (count == 3) {
        firstKind = cards[0]->cardDef->kind;
        secondKind = cards[1]->cardDef->kind;
        thirdDef = cards[2]->cardDef;
        thirdCategory = thirdDef->category;

        if (firstKind == CARD_KIND_MEGA_ETHER && secondKind == CARD_KIND_MEGALIXIR && thirdCategory == CARD_CATEGORY_ITEM && !(thirdDef->flags & CARD_DEF_FLAG_FRIEND)) {
            return TRUE;
        }
    }

    return FALSE;
}

s32 IsFireDonaldMagic(CardDisplayWork** cards, u8 count) {
    const CardDef* thirdDef;
    u16 firstKind;
    u16 secondKind;
    u8 thirdCategory;

    if (count == 3) {
        firstKind = cards[0]->cardDef->kind;
        secondKind = cards[1]->cardDef->kind;
        thirdDef = cards[2]->cardDef;
        thirdCategory = thirdDef->category;

        if (firstKind == CARD_KIND_FIRE && secondKind == CARD_KIND_DONALD_DUCK && thirdCategory == CARD_CATEGORY_MAGIC && !(thirdDef->flags & CARD_DEF_FLAG_SUMMON)) {
            return TRUE;
        }
    }

    return FALSE;
}

s32 IsCloudStopAttack(CardDisplayWork** cards, u8 count) {
    u16 firstKind;
    u16 secondKind;
    u8 thirdCategory;

    if (count == 3) {
        firstKind = cards[0]->cardDef->kind;
        secondKind = cards[1]->cardDef->kind;
        thirdCategory = cards[2]->cardDef->category;

        if (firstKind == CARD_KIND_CLOUD && secondKind == CARD_KIND_STOP && thirdCategory == CARD_CATEGORY_ATTACK) {
            return TRUE;
        }
    }

    return FALSE;
}

s32 IsAeroFireMagic(CardDisplayWork** cards, u8 count) {
    const CardDef* thirdDef;
    u16 firstKind;
    u16 secondKind;
    u8 thirdCategory;

    if (count == 3) {
        firstKind = cards[0]->cardDef->kind;
        secondKind = cards[1]->cardDef->kind;
        thirdDef = cards[2]->cardDef;
        thirdCategory = thirdDef->category;

        if (firstKind == CARD_KIND_AERO && secondKind == CARD_KIND_FIRE && thirdCategory == CARD_CATEGORY_MAGIC && !(thirdDef->flags & CARD_DEF_FLAG_SUMMON)) {
            return TRUE;
        }
    }

    return FALSE;
}

s32 IsAeroBlizzardMagic(CardDisplayWork** cards, u8 count) {
    const CardDef* thirdDef;
    u16 firstKind;
    u16 secondKind;
    u8 thirdCategory;

    if (count == 3) {
        firstKind = cards[0]->cardDef->kind;
        secondKind = cards[1]->cardDef->kind;
        thirdDef = cards[2]->cardDef;
        thirdCategory = thirdDef->category;

        if (firstKind == CARD_KIND_AERO && secondKind == CARD_KIND_BLIZZARD && thirdCategory == CARD_CATEGORY_MAGIC && !(thirdDef->flags & CARD_DEF_FLAG_SUMMON)) {
            return TRUE;
        }
    }

    return FALSE;
}

s32 IsTwoMagicThenPeterPan(CardDisplayWork** cards, u8 count) {
    const CardDef* firstDef;
    const CardDef* secondDef;
    const CardDef* thirdDef;
    u16 firstKind;
    u16 secondKind;
    u16 thirdKind;
    u8 firstCategory;
    u8 secondCategory;
    u8 thirdCategory;

    if (count == 3) {
        firstDef = cards[0]->cardDef;
        firstKind = firstDef->kind;
        secondDef = cards[1]->cardDef;
        secondKind = secondDef->kind;
        thirdDef = cards[2]->cardDef;
        thirdKind = thirdDef->kind;
        firstCategory = firstDef->category;
        secondCategory = secondDef->category;
        thirdCategory = thirdDef->category;

        if (firstCategory == CARD_CATEGORY_MAGIC && !(firstDef->flags & CARD_DEF_FLAG_SUMMON) && secondCategory == CARD_CATEGORY_MAGIC && !(secondDef->flags & CARD_DEF_FLAG_SUMMON) &&
            thirdKind == CARD_KIND_PETER_PAN) {
            return TRUE;
        }

        if (firstKind == CARD_KIND_STOP && secondKind == CARD_KIND_AERO && thirdCategory == CARD_CATEGORY_ITEM && !(cards[2]->cardDef->flags & CARD_DEF_FLAG_FRIEND)) {
            return TRUE;
        }
    }

    return FALSE;
}

u8 IsLinkSideStockLearned(s32 stock, s32 side) {
    u8 learned;

    if (side != LINK_SIDE_SELF) {
        learned = IsLinkPartnerStockLearned(stock);
    } else {
        learned = IsLinkStockLearned(stock);
    }

    return learned;
}

s32 LookupStockPairName(StockKeys* cards, u8* output, u8 count) {
    u32 catalogNumbers[6];

    catalogNumbers[0] = cards->keys[0];
    catalogNumbers[1] = cards->keys[1];
    catalogNumbers[2] = cards->keys[2];

    switch ((catalogNumbers[1] << 10) | catalogNumbers[0] | 0x80000000) {
    case 0x8002ACAB:
        return STOCK_FIRA;
    case 0x8002D4B5:
        return STOCK_BLIZZARA;
    case 0x8002FCBF:
        return STOCK_THUNDARA;
    case 0x800324C9:
        return STOCK_CURA;
    case 0x80034CD3:
        return STOCK_GRAVIRA;
    case 0x800374DD:
        return STOCK_STOPRA;
    case 0x80039CE7:
        return STOCK_AERORA;
    case 0x8003ECFB:
        return STOCK_GOOFY_CHARGE;
    case 0x8003C4F1:
        return STOCK_MAGIC_PAIR;
    case 0x80041505:
        return STOCK_PROUD_ROAR_PAIR;
    case 0x80043D0F:
        return STOCK_SHOWTIME_PAIR;
    case 0x80046519:
        return STOCK_PARADISE_PAIR;
    case 0x80048D23:
        return STOCK_SPLASH_PAIR;
    case 0x8004B52D:
        return STOCK_TWINKLE_PAIR;
    case 0x8004DD37:
        return STOCK_FLARE_BREATH_PAIR;
    case 0x80050541:
        return STOCK_CROSS_SLASH;
    case 0x80052D4B:
        return STOCK_SANDSTORM_PAIR;
    case 0x80055555:
        return STOCK_SPIRAL_WAVE_PAIR;
    case 0x80057D5F:
        return STOCK_SURPRISE_PAIR;
    case 0x8005A569:
        return STOCK_HUMMINGBIRD_PAIR;
    case 0x8005CD73:
        return STOCK_FEROCIOUS_LUNGE_PAIR;
    case 0x800A7E9F:
        return STOCK_MM_MIRACLE_PAIR;
    }

    switch (catalogNumbers[1] | (catalogNumbers[2] << 10) | 0x80000000) {
    case 0x8002ACAB:
        return STOCK_FIRA;
    case 0x8002D4B5:
        return STOCK_BLIZZARA;
    case 0x8002FCBF:
        return STOCK_THUNDARA;
    case 0x800324C9:
        return STOCK_CURA;
    case 0x80034CD3:
        return STOCK_GRAVIRA;
    case 0x800374DD:
        return STOCK_STOPRA;
    case 0x80039CE7:
        return STOCK_AERORA;
    case 0x8003ECFB:
        return STOCK_GOOFY_CHARGE;
    case 0x8003C4F1:
        return STOCK_MAGIC_PAIR;
    case 0x80041505:
        return STOCK_PROUD_ROAR_PAIR;
    case 0x80043D0F:
        return STOCK_SHOWTIME_PAIR;
    case 0x80046519:
        return STOCK_PARADISE_PAIR;
    case 0x80048D23:
        return STOCK_SPLASH_PAIR;
    case 0x8004B52D:
        return STOCK_TWINKLE_PAIR;
    case 0x8004DD37:
        return STOCK_FLARE_BREATH_PAIR;
    case 0x80050541:
        return STOCK_CROSS_SLASH;
    case 0x80052D4B:
        return STOCK_SANDSTORM_PAIR;
    case 0x80055555:
        return STOCK_SPIRAL_WAVE_PAIR;
    case 0x80057D5F:
        return STOCK_SURPRISE_PAIR;
    case 0x8005A569:
        return STOCK_HUMMINGBIRD_PAIR;
    case 0x8005CD73:
        return STOCK_FEROCIOUS_LUNGE_PAIR;
    case 0x800A7E9F:
        return STOCK_MM_MIRACLE_PAIR;
    }

    return STOCK_NONE;
}

Mode gModeRikuBtlTutorial = {
    "Mode_riku_btlTutorial",
    RikuTutorialModeInit,
    Mode_riku_btlTutorial_1,
    RikuTutorialModeDestroy,
};

Mode gModeRikuDeckTutorial = {
    "Mode_riku_deckTutorial",
    RikuTutorialModeInit,
    Mode_riku_deckTutorial_1,
    RikuTutorialModeDestroy,
};

const StockKeys gTutorialEmptyKeys = {
    { -1, -1, -1, -1, -1, -1 },
};
