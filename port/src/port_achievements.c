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
#ifdef VERSION_EU
/* Official achievements whose reads are all verified (build/ra-reference-audit/enable.json;
   the battle ones by ra_battle_dryrun.py: won triggers, lost never). */
static const uint32_t sOfficialVerified[] = { 15580u, 15582u, 15583u, 15584u, 15585u, 15587u, 15588u, 15590u, 15591u, 15592u, 15594u, 15595u, 15596u, 15597u, 15598u, 15599u, 15600u, 15601u, 15602u, 15603u, 15604u, 15611u, 15612u, 15619u, 15651u, 15652u, 15653u, 15654u, 15655u, 15656u, 15657u, 15658u, 15659u, 15660u, 15661u, 15662u, 15663u, 15664u, 15665u, 156793u, 160363u, 160364u, 160365u, 160367u, 160369u, 160371u, 160372u, 160373u, 160376u, 160377u, 160383u, 160385u, 160387u, 160390u, 160391u, 160392u, 160393u, 160394u, 160395u, 160396u, 160397u, 160398u, 160400u, 160403u, 160404u, 160405u, 160406u, 160407u, 160408u, 160409u };
#endif
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

#ifdef PORT_RETROACHIEVEMENTS
/* Test harness (KHCOM_RA_DRYRUN=definitions.json, the catalog audit_ra_references.py
   saved): every official achievement is evaluated each frame against the port's
   memory with rcheevos' own runtime, offline, and each trigger is logged
   ("ra-dryrun: trigger <id> frame <n>"). Nothing is submitted; no account. */
#include "rc_runtime.h"
static rc_runtime_t sDryRuntime;
static int sDryReady = -1;
static unsigned sDryFrame;

static uint32_t DryPeek(uint32_t address, uint32_t bytes, void* ud) {
    uint8_t b[4] = { 0, 0, 0, 0 };
    (void)ud;
    if (bytes > 4 || !PortRaReadMemory(address, b, bytes))
        return 0;
    return b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24;
}

static void DryEvent(const rc_runtime_event_t* event) {
    if (event->type == RC_RUNTIME_EVENT_ACHIEVEMENT_TRIGGERED)
        PortLog("ra-dryrun: trigger %u frame %u", (unsigned)event->id, sDryFrame);
}

static void DryRunFrame(void) {
    const char* path = SDL_getenv("KHCOM_RA_DRYRUN");
    if (!path)
        return;
    if (sDryReady < 0) {
        size_t size = 0;
        char* json = (char*)SDL_LoadFile(path, &size);
        char* p = json;
        int count = 0;
        sDryReady = 0;
        if (!json)
            return;
        rc_runtime_init(&sDryRuntime);
        /* [{"id": N, "definition": "..."}, ...] */
        while (p && (p = SDL_strstr(p, "\"id\":")) != NULL) {
            unsigned id;
            char* d;
            char* e;
            p += 5;
            while (*p == ' ') p++;
            id = (unsigned)SDL_strtoul(p, NULL, 10);
            d = SDL_strstr(p, "\"definition\":");
            if (!d)
                break;
            d += 13;
            while (*d == ' ') d++;
            if (*d++ != '"')
                break;
            e = SDL_strchr(d, '"');
            if (!e)
                break;
            *e = 0;
            if (id != 101000001u && rc_runtime_activate_achievement(&sDryRuntime, id, d, NULL, 0) == RC_OK)
                count++;
            p = e + 1;
        }
        PortLog("ra-dryrun: %d official achievements active", count);
        SDL_free(json);
        sDryReady = 1;
    }
    if (sDryReady == 1) {
        ++sDryFrame;
        rc_runtime_do_frame(&sDryRuntime, DryEvent, DryPeek, NULL, NULL);
        /* KHCOM_RA_WATCH=1: the battle graphs' operands, logged when they change
           (mode argument, world, card battle root and its +0xCC, the effect
           root's -0x330 and -0x174 neighbours as the official AddAddress reads) */
        if (SDL_getenv("KHCOM_RA_WATCH")) {
            static char last[160];
            char line[160];
            uint32_t fx = DryPeek(0x3CA28, 3, NULL), cbs = DryPeek(0x423E4, 3, NULL);
            SDL_snprintf(line, sizeof(line), "arg=%u world=%u cbs=%06x cbs+cc=%u fx=%06x fx-330=%u fx-174=%u",
                         (unsigned)DryPeek(0x749C, 2, NULL), (unsigned)DryPeek(0x42346, 1, NULL), (unsigned)cbs,
                         (unsigned)(cbs ? DryPeek(cbs + 0x80CC, 2, NULL) : 0), (unsigned)fx,
                         (unsigned)(fx ? DryPeek(fx + 0x7CD0, 2, NULL) : 0), (unsigned)(fx ? DryPeek(fx + 0x7E8C, 2, NULL) : 0));
            if (SDL_strcmp(line, last) != 0) {
                SDL_strlcpy(last, line, sizeof(last));
                PortLog("ra-watch: frame %u %s", sDryFrame, line);
            }
        }
    }
}
#endif

void PortAchievementsFrame(void) {
    KhLocalSnapshot snapshot;
    int i;
#ifdef PORT_RETROACHIEVEMENTS
    DryRunFrame();
#endif
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
