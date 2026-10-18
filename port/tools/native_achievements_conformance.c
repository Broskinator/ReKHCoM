#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <SDL3/SDL.h>
#include "port/port.h"
#include "game_state.h"
#include "mode.h"
#include "../achievements/local_achievements.h"

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

int main(void) {
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
    gCurrentMode = &gModeMapFld;
    gGameState.progression.level = 1; gGameState.progression.maxHp = 80;
    PortAchievementsFrame();
    gGameState.progression.level = 10;
    PortAchievementsFrame();
    assert(notices == 1 && KhLocalLoad(&state, file) && state.unlocked == 1);
    PortAchievementsFrame(); assert(notices == 1);
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
