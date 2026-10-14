/**
 * map_stair.c
 * Hall Stairs Trigger
 */

#include "map_tasks.h"
#include "sprites_btl.h"
#include "sprite_palettes.h"
#include "engine_math.h"
#include "card_api.h"
#include "field_state.h"
#include "fld_types.h"
#include "game_state.h"
#include "map.h"
#include "map_api.h"
#include "map_types.h"
#include "obj.h"
#include "obj_api.h"
#include "player_progression_types.h"
#include "registration_data.h"
#include "taskpool.h"
#include "types.h"
#include <stddef.h>
#include "card_message_data.h"

s32 IsPlayerWithin(FldPos* pos, s32 lim) {
    s32 dx;
    s32 dy;

    dx = pos->x - gFieldState->actor.fieldPosition.x;

    if (dx < 0) {
        dx = gFieldState->actor.fieldPosition.x - pos->x;
    }

    dy = pos->y - gFieldState->actor.fieldPosition.y;

    if (dy < 0) {
        dy = gFieldState->actor.fieldPosition.y - pos->y;
    }

    if (dx > 0x8000 || dy > 0x8000) {
        return FALSE;
    }

    return Sqrt8((dx * dx >> 8) + (dy * dy >> 8)) < lim ? TRUE : FALSE;
}

void MapStairWaitStepOn(MapStairWork* work) {
    if ((u8)IsPlayerWithin(&work->obj.fieldPosition, 0x800)) {
        if (gFieldState->actor.fieldPosition.z == gFieldState->actor.fieldPosition.ground) {
            if (gMapFloorState.room == MAP_ROOM_ENTRANCE_HALL) {
                gMapRoomState->flags |= ROOM_FLAG_WALK_OUT;
            } else {
                gMapRoomState->flags |= ROOM_FLAG_EXIT_NEXT_FLOOR;
            }
        }
    }
}

void MapStairWaitStepOn2(MapStairWork* work) {
    s32 k = 0x800;

    if ((u8)IsPlayerWithin(&work->obj.fieldPosition, k)) {
        if (gFieldState->actor.fieldPosition.z == gFieldState->actor.fieldPosition.ground) {
            if (gMapFloorState.room == MAP_ROOM_ENTRANCE_HALL) {
                gMapRoomState->flags |= k;
            } else {
                gMapRoomState->flags |= ROOM_FLAG_ENTER_WORLD;
            }
        }
    }
}

void MapStairWaitApproach(MapStairWork* work) {
    if ((u8)IsPlayerWithin(&work->obj.fieldPosition, 0x3000)) {
        gFieldState->flags |= FIELD_FLAG_FREEZE_PLAYER;
        gMapRoomState->flags |= ROOM_FLAG_TUTORIAL_ACTIVE;
        CreateCardMessageTask(&work->tasks, 0, CARD_MSG_MAP_STAIR_TUTORIAL);
        work->update = MapStairWaitMessage;
    }
}

void MapStairWaitMessage(MapStairWork* work) {
    if (!IsMessageWindowOpen()) {
        gFieldState->flags &= ~FIELD_FLAG_FREEZE_PLAYER;
        gMapRoomState->flags &= ~ROOM_FLAG_TUTORIAL_ACTIVE;
        gGameState.progression.tutorialFlags |= TUTORIAL_FLAG_MAP_STAIR;
        work->update = MapStairWaitStepOn;
    }
}

#ifdef PLATFORM_PC
void PortBotStair(int angle, FldPos* pos);
#endif
void Task_MapStair_0(MapStairWork* work, FldObj* arg) {
    s32 y;

    work->obj.angle = arg->angle;
    work->obj.fieldPosition.x = arg->fieldPosition.x;
    y = arg->fieldPosition.y;
    work->obj.fieldPosition.ground = 0;
    work->obj.fieldPosition.z = 0;
    work->obj.fieldPosition.y = y;
    work->palette = LoadObjPalette(gCommonObjPalette, sizeof(gCommonObjPalette));
    work->tiles = LoadObjTiles(gBtlAreaTiles, sizeof(gBtlAreaTiles));
    work->visible = FALSE;

    switch (work->obj.angle) {
    case FLD_ANGLE_UP_RIGHT:
        if ((gGameState.progression.tutorialFlags & TUTORIAL_FLAG_MAP_STAIR) == 0 && gMapFloorState.room == MAP_ROOM_EXIT_HALL) {
            work->update = MapStairWaitApproach;
        } else {
            work->update = MapStairWaitStepOn;
        }

        break;
    case FLD_ANGLE_DOWN_LEFT:
        work->update = MapStairWaitStepOn2;
        break;
    }

    TaskPoolInit(&work->tasks, 1);
#ifdef PLATFORM_PC
    PortBotStair(work->obj.angle, &work->obj.fieldPosition); /* port_testbot.c */
#endif
}

s32 Task_MapStair_1(MapStairWork* work) {
    TaskPoolUpdate(&work->tasks);

    if (work->update != NULL) {
        work->update(work);
    }

    return 1;
}

void Task_MapStair_2(MapStairWork* work) {
    s32 x;
    s32 y;

    TaskPoolDraw(&work->tasks);

    if (work->visible == TRUE) {
        x = (work->obj.fieldPosition.x >> 8) - (gFieldState->x >> 8);
        y = (work->obj.fieldPosition.y >> 8) + (work->obj.fieldPosition.z >> 8) - (gFieldState->y >> 8);
        DrawSprite(x, y, gBtlAreaFrame5, work->tiles, work->palette, NULL, SPRITE_PRIORITY(2), 0x101);
    }
}

void Task_MapStair_3(MapStairWork* work) {
#ifdef PLATFORM_PC
    PortBotStair(work->obj.angle, NULL);
#endif
    ReleaseObjTiles(work->tiles);
    ReleaseObjPalette(work->palette);
    TaskPoolDestroy(&work->tasks);
}

TaskDesc gTaskDescMapStair = {
    "Task_MapStair",
    (TaskInitFunc)Task_MapStair_0,
    (TaskUpdateFunc)Task_MapStair_1,
    (TaskDrawFunc)Task_MapStair_2,
    (TaskDestroyFunc)Task_MapStair_3,
    sizeof(MapStairWork),
};
