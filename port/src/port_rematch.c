/*
 * Boss rematches and battle tutorials (Options -> Rematches): the field is
 * kept in a state (states/rematch.khst), the battle runs as usual, and when
 * it ends -won or lost- the state brings the player back where they were,
 * with nothing from that battle kept (no experience, cards or story).
 */
#include <SDL3/SDL.h>
#include <string.h>
#include "port/port.h"

extern void PortStateRequestQuiet(int load, const char* path);
extern int PortStateBusy(void);
extern int PortStateLastResult(void);
extern const char* PortHarnessModeName(void);
extern int PortRematchAvailable(int i);
extern void PortRematchBegin(int i);
extern void PortUiNotice(const char* text);

enum { REMATCH_IDLE, REMATCH_SAVING, REMATCH_STARTING, REMATCH_FIGHTING, REMATCH_RETURNING };
static int sState = REMATCH_IDLE, sWhich, sWait;

static const char* StatePath(void) {
    return PortConfigPath("states/rematch.khst");
}

int PortRematchActive(void) {
    return sState != REMATCH_IDLE;
}

/* from the options menu: keep the field, then start the battle */
int PortRematchStart(int i) {
    if (sState != REMATCH_IDLE || !PortRematchAvailable(i))
        return 0;
    PortAchievementsInvalidate("Rematch started");
    sWhich = i;
    sState = REMATCH_SAVING;
    PortStateRequestQuiet(0, StatePath());
    PortLog("rematch: %d, keeping the field", i);
    return 1;
}

/* every frame, after the state service */
void PortRematchFrame(void) {
    const char* mode = PortHarnessModeName();

    switch (sState) {
        case REMATCH_SAVING:
            if (PortStateBusy())
                break;
            if (PortStateLastResult() <= 0) {
                PortLog("rematch: the field could not be kept; cancelled");
                sState = REMATCH_IDLE;
                break;
            }
            PortRematchBegin(sWhich);
            sState = REMATCH_STARTING;
            sWait = 0;
            break;
        case REMATCH_STARTING:
            if (strcmp(mode, "mode_battle") == 0) {
                sState = REMATCH_FIGHTING;
                PortLog("rematch: battle started");
            } else if (++sWait > 600) { /* never started: back to the field */
                sState = REMATCH_RETURNING;
                PortStateRequestQuiet(1, StatePath());
            }
            break;
        case REMATCH_FIGHTING:
            /* the battle is over when its mode ends (results, game over or
               the story's next scene): the field comes back instead */
            if (strcmp(mode, "mode_battle") != 0) {
                PortLog("rematch: battle over (%s), back to the field", mode);
                sState = REMATCH_RETURNING;
                PortStateRequestQuiet(1, StatePath());
            }
            break;
        case REMATCH_RETURNING:
            if (!PortStateBusy())
                sState = REMATCH_IDLE;
            break;
    }
}
