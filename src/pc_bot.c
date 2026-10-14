/**
 * pc_bot.c
 * PC port: a playthrough bot for gameplay verification (KHCOM_BOT=1).
 *
 * It plays from power-on with ordinary pad input decided from the game's own
 * state each frame: no state is written, no fixture or warp is used. Events,
 * menus and messages are confirmed with A; on the field Sora walks to a door
 * that leads toward unexplored rooms (event doors first) and strikes it; in
 * battles the nearest enemy is approached and attacked, holding A on reload.
 */
#ifdef PLATFORM_PC
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "types.h"
#include "field_state.h"
#include "map.h"
#include "map_runtime.h"
#include "map_api.h"
#include "battle_work.h"
#include "battle_actor.h"
#include "listpool.h"
#include "game_state.h"
#include "card_msgwin.h"

extern void PortLog(const char* format, ...);
extern const char* PortHarnessModeName(void);
u8 IsMessageWindowOpen();

enum { K_A = 1, K_B = 2, K_SELECT = 4, K_START = 8, K_RIGHT = 16, K_LEFT = 32, K_UP = 64, K_DOWN = 128, K_R = 256, K_L = 512 };

static int sEnabled = -1;
static uint32_t sLastLog;
static int sStuck, sLastX, sLastY, sWiggle;
static int sTargetSide = -1;
static uint8_t sTriedSide[32][4];

static uint16_t Toward(int dx, int dy, int dead) {
    uint16_t k = 0;
    if (dx > dead) k |= K_RIGHT;
    else if (dx < -dead) k |= K_LEFT;
    if (dy > dead) k |= K_DOWN;
    else if (dy < -dead) k |= K_UP;
    return k;
}

static int RoomVisited(int room) {
    return room < 32 && (gMapFloorState.rooms[room].flags & FLOOR_ROOM_FLAG_VISITED);
}

/* Field: pick a door. Event doors first, then doors to rooms not yet visited,
   then any door, avoiding the ones already tried from this room. */
static int ChooseDoor(int room) {
    int side, best = -1, bestScore = -1000;

    for (side = 0; side < 4; side++) {
        MapDoor* door = GetMapDoor(side);
        int score;
        if (!(door->flags & DOOR_FLAG_PRESENT) || (door->flags & DOOR_FLAG_SEALED))
            continue;
        if ((u8)(door->room + 3) <= 1)
            continue;
        score = 0;
        if (door->flags & DOOR_FLAG_EVENT) score += 50;
        if (door->room < 32 && !RoomVisited(door->room)) score += 20;
        if (side == gMapFloorState.entrySide) score -= 5;
        score -= sTriedSide[room & 31][side] * 30;
        if (score > bestScore) {
            bestScore = score;
            best = side;
        }
    }
    return best;
}

static uint16_t FieldKeys(uint32_t frame) {
    FldPos* p = &gFieldState->actor.fieldPosition;
    int room = gMapFloorState.room;
    int px = p->x >> 8, py = (p->y - p->z) >> 8;
    MapDoor* door;
    int dx, dy, tx, ty;

    if (gFieldState->flags & (FIELD_FLAG_MENU_OPEN | FIELD_FLAG_ROOM_CREATE))
        return (frame % 16) < 2 ? K_A : 0; /* the door's card menu: take the card shown */
    if (IsMessageWindowOpen())
        return (frame % 16) < 2 ? K_A : 0;

    if (sTargetSide < 0 || !(GetMapDoor(sTargetSide)->flags & DOOR_FLAG_PRESENT))
        sTargetSide = ChooseDoor(room);
    if (sTargetSide < 0)
        return (frame % 30) < 2 ? K_A : 0;
    door = GetMapDoor(sTargetSide);
    tx = (door->cellX << 5) + 16;
    ty = (door->cellY << 4) + 8;
    dx = tx - px;
    dy = ty - py;

    /* stuck: wiggle sideways and jump for a while, then try another door */
    if (abs(px - sLastX) + abs(py - sLastY) < 1) {
        if (++sStuck > 90) {
            sWiggle = 40;
            sStuck = 0;
            if (++sTriedSide[room & 31][sTargetSide] > 3)
                sTargetSide = -1;
        }
    } else {
        sStuck = 0;
    }
    sLastX = px;
    sLastY = py;
    if (sWiggle > 0) {
        sWiggle--;
        return ((frame / 20) & 1 ? K_LEFT : K_RIGHT) | (sWiggle % 20 == 10 ? K_B : 0);
    }
    if (abs(dx) < 20 && abs(dy) < 14)
        return (frame % 12) < 2 ? K_A : Toward(dx, dy, 2); /* strike the door */
    return Toward(dx, dy, 4);
}

static uint16_t BattleKeys(uint32_t frame) {
    BtlObj* me = gBtlWork->actor;
    BtlObj* obj;
    BtlObj* best = NULL;
    int bestD = 1 << 30;

    if (me == NULL)
        return (frame % 10) < 2 ? K_A : 0;
    {
        /* every 8 seconds, the moves the battle tutorials ask for: reload
           (hold A), jump, dodge roll (double tap), cycle cards, switch list */
        uint32_t t = frame % 480;
        if (t >= 200 && t < 260) return K_A;
        if (t >= 262 && t < 266) return K_B;
        if ((t >= 300 && t < 303) || (t >= 307 && t < 310)) return K_RIGHT;
        if (t >= 340 && t < 343) return K_R;
        if (t >= 360 && t < 363) return K_L;
        if (t >= 400 && t < 403 && (frame / 480) % 3 == 0) return K_SELECT;
    }
    for (obj = ListPoolFirst(&gBtlWork->pool); obj != NULL; obj = ListPoolNext(&obj->node)) {
        int d = abs((obj->x - me->x) >> 8) + abs((obj->y - me->y) >> 8);
        if (obj->hp > 0 && d < bestD) {
            bestD = d;
            best = obj;
        }
    }
    if (best == NULL)
        return (frame % 10) < 2 ? K_A : 0;
    {
        int dx = (best->x - me->x) >> 8, dy = (best->y - me->y) >> 8;
        uint16_t k = Toward(dx, dy, 18);
        if (abs(dx) < 48 && abs(dy) < 24)
            k = (uint16_t)((k & (K_UP | K_DOWN)) | ((frame % 8) < 3 ? K_A : 0)
                           | (dx > 0 ? K_RIGHT : K_LEFT) * ((frame % 8) == 7));
        else if ((frame / 120) % 4 == 3)
            k |= K_A; /* hold A: charges a reload when the hand is empty */
        return k;
    }
}

uint16_t PortBotKeys(uint32_t frame) {
    const char* mode;

    if (sEnabled < 0) {
        const char* e = getenv("KHCOM_BOT");
        sEnabled = e != NULL && atoi(e) != 0;
    }
    if (!sEnabled)
        return 0;
    mode = PortHarnessModeName();
    if (frame - sLastLog >= 300) {
        sLastLog = frame;
        PortLog("bot: frame %u mode %s floor %u room %u story %u level %u hp %d", (unsigned)frame, mode,
                (unsigned)gGameState.floor, (unsigned)gMapFloorState.room, (unsigned)gMapFloorState.progress,
                (unsigned)gGameState.progression.level,
                gFieldState ? (int)(gFieldState->actor.fieldPosition.x >> 8) : -1);
    }
    if (strcmp(mode, "mode_battle") == 0 && gBtlWork != NULL)
        return BattleKeys(frame);
    if (strcmp(mode, "Mode_MapFld") == 0 && gFieldState != NULL) {
        static int lastRoom = -1;
        if (gMapFloorState.room != lastRoom) {
            lastRoom = gMapFloorState.room;
            sTargetSide = -1;
            sStuck = 0;
        }
        return FieldKeys(frame);
    }
    /* title, events, movies, menus, level up, game over: confirm */
    return (frame % 20) < 2 ? K_A : 0;
}
#endif
