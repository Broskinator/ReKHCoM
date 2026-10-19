#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <SDL3/SDL.h>
#include "port/port.h"
#include "game_state.h"
#include "mode.h"
#include "../achievements/local_achievements.h"
#include "port/port_achievement_list.h"

GameState gGameState;
PortConfig gPortConfig;
Mode gModeMapFld, gModeBattle, gModeTitle;
Mode* gCurrentMode;
uint32_t gLanguage;
static int notices;
const char* PortConfigPath(const char* path) { return path; }
void PortLog(const char* format, ...) { (void)format; }
int PortUiActive(void) { return 0; }
void PortAchievementToast(const char* title, const char* detail, const char* label) {
    assert(title[0] && detail[0] && label[0]); ++notices;
}
void PortAchievementToastShutdown(void) {}

int main(int argc, char** argv) {
    KhLocalState state;
    const char* file = "achievements_"
#if defined(VERSION_US)
        "us"
#elif defined(VERSION_JP)
        "jp"
#else
        "eu"
#endif
        ".dat";
    assert(SDL_Init(0));
    remove(file);
    PortAchievementsInit();
    {
        PortAchievementInfo info;
        PortAchievementsListRefresh();
        assert(PortAchievementsListCount(0) == 16 && PortAchievementsListCount(1) == 2);
        assert(PortAchievementsListGet(1, 0, 0, &info) && info.state == 0 && !strcmp(info.title, "Locked test"));
        assert(PortAchievementsListGet(1, 1, 0, &info) && info.state == 1 && !strcmp(info.title, "Earned test"));
        assert(!PortAchievementsListGet(1, 2, 0, &info));
        assert(PortAchievementsListGet(0, 0, 1, &info) && info.state == 0 && info.title[0]);
    }
    gCurrentMode = &gModeMapFld;
    gGameState.progression.level = 1; gGameState.progression.maxHp = 80;
    PortAchievementsFrame();
    gGameState.progression.level = 10;
    PortAchievementsFrame();
    assert(notices == 1 && KhLocalLoad(&state, file) && state.unlocked == 1);
    PortAchievementsFrame(); assert(notices == 1);
    if (argc == 1) {
        unsigned id;
        for (id = 1; id < KH_LOCAL_COUNT; ++id) {
            switch (id) {
            case 1: gGameState.progression.level = 25; break;
            case 2: gGameState.progression.level = 50; break;
            case 3: gGameState.progression.level = 99; break;
            case 4: gGameState.progression.maxHp = 200; break;
            case 5: gGameState.progression.cp = 500; break;
            case 6: gGameState.progression.mooglePoints = 1000; break;
            case 7: gGameState.progression.mooglePoints = 10000; break;
            case 8: gGameState.progression.friendFlags = 4; break;
            case 9: gGameState.progression.learnedStocks = 0x1ff;
                    gGameState.progression.learnedStocks2 = 1; break;
            case 10: gGameState.progression.obtainedCardKinds = (1ull << 20) - 1; break;
            case 11: gGameState.flags |= 0x20; break;
            case 12: gGameState.flags |= 0x800; break;
            case 13: gGameState.floor = 4; break;
            case 14: gGameState.floor = 8; break;
            case 15: gGameState.floor = 12; break;
            }
            gCurrentMode = id & 1 ? &gModeBattle : &gModeMapFld;
            PortAchievementsFrame();
            assert(notices == (int)id + 1);
            assert(KhLocalLoad(&state, file) && state.unlocked == (1u << (id + 1)) - 1);
            PortAchievementsFrame(); assert(notices == (int)id + 1);
            { PortAchievementInfo info; assert(PortAchievementsListGet(0, id, 0, &info) && info.state == 1); }
        }
        PortAchievementsShutdown(); SDL_Quit();
        puts("PASS native bridge: all 16 triggers, battle/map modes, region persistence, no duplicates");
        return 0;
    }
    (void)argv;
    /* Real bridge observes modifiers before it reads the changed progression. */
    gPortConfig.cheatOneHit = 1; gGameState.progression.level = 25;
    PortAchievementsFrame();
    gPortConfig.cheatOneHit = 0; gGameState.progression.level = 99;
    PortAchievementsFrame();
    assert(notices == 1 && KhLocalLoad(&state, file) && state.unlocked == 1);
    PortAchievementsShutdown();
    SDL_Quit();
    puts("PASS native bridge: local award, durable region file, no duplicate, modifier invalidation survives toggle off");
    return 0;
}
