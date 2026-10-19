#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#if __has_include(<time.h>)
#include <time.h>
#define KH_LOCAL_WALL_CLOCK 1
#endif
#include "port/port.h"
#include "port/port_ui.h"
#include "game_state.h"
#include "mode.h"
#include "../achievements/local_achievements.c"
#ifdef PORT_RETROACHIEVEMENTS
int PortRaReadMemory(uint32_t address, uint8_t* buffer, uint32_t size);
static uint32_t PortRaRead(uint32_t address, uint8_t* out, uint32_t size, void* userdata) {
    (void)userdata;
    return PortRaReadMemory(address, out, size) ? size : 0;
}
/* Official achievements whose reads are all verified (build/ra-reference-audit/enable.json). */
static const uint32_t sOfficialVerified[] = { 15580u, 15584u, 15585u, 15587u, 15588u, 15590u, 15592u, 15595u, 15597u, 15598u, 15599u, 15600u, 15612u, 156793u, 160372u, 160403u, 160404u, 160405u, 160406u, 160407u, 160408u, 160409u };
#endif

#ifdef PORT_RETROACHIEVEMENTS
#include "../achievements/ra_client.h"
static KhRa* sRa;
#endif

extern Mode* gCurrentMode;
extern Mode gModeMapFld, gModeBattle, gModeTitle;
static KhLocalState sLocal;
static int sLocalEnabled = 1, sInvalidated, sStarted;
static uint32_t sPendingNotices;
static unsigned sPreviewFrame;
static char sLocalPath[1200];
#include "port_achievement_list.inc"

void PortAchievementsInit(void) {
    FILE* config;
    snprintf(sLocalPath, sizeof(sLocalPath), "%s", PortConfigPath("achievements_"
#ifdef VERSION_US
        "us"
#elif defined(VERSION_JP)
        "jp"
#else
        "eu"
#endif
        ".dat"));
    KhLocalLoad(&sLocal, sLocalPath);
    config = fopen(PortConfigPath("achievements_local.ini"), "r");
    if (config) {
        char line[64]; int value = 1;
        if (fgets(line, sizeof(line), config) && sscanf(line, "enabled=%d", &value) == 1) sLocalEnabled = value == 1;
        fclose(config);
    }
#ifdef PORT_RETROACHIEVEMENTS
    /* Official set (EU cartridge): evaluated from the port's memory as the
     * cartridge lays it out (port_ra_memory.c / port_ra_state.c). Only the
     * achievements whose every read is verified against the ROM are active;
     * the rest stay listed but disabled until their memory is verified
     * (queue item 71). */
    sRa = KhRaCreate(PortConfigPath("."), PortRaRead, NULL, NULL);
#ifdef VERSION_EU
    KhRaStartOfficial(sRa, sOfficialVerified, sizeof(sOfficialVerified) / sizeof(sOfficialVerified[0]));
#endif
#endif
    if (SDL_getenv("KHCOM_MODE") || SDL_getenv("KHCOM_BOT") || SDL_getenv("KHCOM_BATTLE") ||
        SDL_getenv("KHCOM_EVENT") || SDL_getenv("KHCOM_KEYS")) sInvalidated = 1;
}

void PortAchievementsRomLoaded(const uint8_t* rom, size_t size) {
#ifdef PORT_RETROACHIEVEMENTS
    KhRaSetRom(sRa, rom, size);
#else
    (void)rom; (void)size;
#endif
}

void PortAchievementsInvalidate(const char* reason) {
    if (sInvalidated) return;
    sInvalidated = 1; sPendingNotices = 0;
    PortLog("achievements: session paused (%s)", reason);
#ifdef PORT_RETROACHIEVEMENTS
    KhRaInvalidate(sRa, reason);
#endif
}

void PortAchievementsFrame(void) {
    KhLocalSnapshot snapshot;
    int i;
    ++sPreviewFrame;
    if (SDL_getenv("KHCOM_ACHIEVEMENT_PREVIEW") &&
        sPreviewFrame == (unsigned)SDL_atoi(SDL_getenv("KHCOM_ACHIEVEMENT_PREVIEW")))
        PortAchievementToast("Primeros recuerdos", "Alcanza el nivel 10.", "VISTA PREVIA - LOGRO LOCAL");
    /* Invalidate before this frame can evaluate or submit an official award. */
    if (gPortConfig.cheatOneHit || gPortConfig.cheatInfiniteHp || gPortConfig.cheatUnlockRiku ||
        gPortConfig.ps2Mode)
        PortAchievementsInvalidate("Gameplay modifiers enabled");
#ifdef PORT_RETROACHIEVEMENTS
    KhRaPump(sRa, !sInvalidated && (gCurrentMode == &gModeMapFld || gCurrentMode == &gModeBattle ||
                                     gCurrentMode == &gModeTitle || sStarted));
    {
        const char* notice = KhRaTakeNotice(sRa);
        if (notice) {
            char title[256];
            const char* nl = SDL_strchr(notice, '\n');
            SDL_strlcpy(title, notice, nl ? (size_t)(nl - notice + 1) : sizeof(title));
            PortAchievementToast(title, nl ? nl + 1 : "", "RETROACHIEVEMENTS");
        }
    }
#endif
    if (!sLocalEnabled || sInvalidated) return;
    if (gCurrentMode == &gModeTitle) {
        sStarted = 0; sLocal.initialized = 0; sLocal.previous = 0;
    }
    if (gCurrentMode == &gModeMapFld || gCurrentMode == &gModeBattle) sStarted = 1;
    if (!sStarted) return;
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.playing = gGameState.progression.level >= 1 && gGameState.progression.level <= 99 &&
                       gGameState.progression.maxHp > 0;
    snapshot.flags = gGameState.flags;
    snapshot.level = gGameState.progression.level;
    snapshot.max_hp = gGameState.progression.maxHp;
    snapshot.cp = gGameState.progression.cp;
    snapshot.floor = gGameState.floor;
    snapshot.moogle_points = gGameState.progression.mooglePoints;
    snapshot.learned = gGameState.progression.learnedStocks;
    snapshot.learned2 = gGameState.progression.learnedStocks2;
    snapshot.card_kinds = gGameState.progression.obtainedCardKinds;
    snapshot.friends = gGameState.progression.friendFlags;
    {
        uint64_t now = 0; /* A guest without a wall clock stores an unknown time. */
#ifdef KH_LOCAL_WALL_CLOCK
        now = (uint64_t)time(NULL);
#endif
        uint32_t added = KhLocalObserve(&sLocal, &snapshot, now);
        if (added) {
            if (!KhLocalSave(&sLocal, sLocalPath)) PortLog("achievements: could not save local progress");
#ifdef __EMSCRIPTEN__
            else EM_ASM({ if (Module.persistSoon) Module.persistSoon();
                          if (Module.onAchievementsChanged) Module.onAchievementsChanged(); });
#endif
            sPendingNotices |= added;
        }
    }
    if (sPendingNotices && !PortUiActive()) {
        int spanish = 0;
#ifdef VERSION_EU
        extern uint32_t gLanguage;
        spanish = gLanguage == 4;
#endif
        for (i = 0; i < KH_LOCAL_COUNT; ++i) if (sPendingNotices & (1u << i)) {
            const KhLocalDefinition* def = &gKhLocalDefinitions[i];
            PortAchievementToast(spanish ? def->title_es : def->title_en,
                spanish ? def->description_es : def->description_en, spanish ? "LOGRO LOCAL" : "LOCAL ACHIEVEMENT");
            PortLog("achievement local: %d", i);
            sPendingNotices &= ~(1u << i);
        }
    }
}

void PortAchievementsShutdown(void) {
    PortAchievementToastShutdown();
#ifdef PORT_RETROACHIEVEMENTS
    KhRaDestroy(sRa); sRa = NULL;
#endif
}
