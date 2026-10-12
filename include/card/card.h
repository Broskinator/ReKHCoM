#ifndef GUARD_CARD_H
#define GUARD_CARD_H

#include "card_help_data.h"
#include "card_message_data.h"
#include "anim.h"
#include "card_description_data.h"
#include "msg_types.h"
#include "card_ui_types.h"
#include "types.h"
#include "text_types.h"
#include "obj.h"
#include "taskpool.h"
#include "listpool.h"
#include "battle_actor_types.h"
#include "card_types.h"
#include "fld_types.h"
#include "mode.h"
#include "event_background_types.h"
#include "map_animation_types.h"
#include "macros.h"

typedef struct PrizeMapCardBackAnimStep {
    u8 sprite;
    u8 duration;
    u8 unk_02;
    u8 unk_03;
} PrizeMapCardBackAnimStep;

void LevelUpSplitDigits2(u16 value, u16* digits);
void LevelUpSplitDigits3(u16 value, u16* digits);

typedef struct CardSlot {
    u32 cardId;
    u16 index;
    u8 unk_06;
    u8 stocked;
    u8 used;
    u8 restoreOnReload;
    u8 removed;
    u8 unk_0B;
} CardSlot;

STATIC_ASSERT(sizeof(CardSlot) == 0xC, CardSlotSize);

#define CARD_SLOT_NONE 0xFFFF

typedef struct CardDisplayArgs {
    ListPool* pool;
    CardSlot* slot;
    s32 variant;
    u16 index;
    u8 listIndex;
    u8 reloadCount;
} CardDisplayArgs;

STATIC_ASSERT(sizeof(CardDisplayArgs) == 0x10, CardDisplayArgsSize);

enum CardDisplayFlag {
    CARD_DISP_FLAG_FACE_DOWN = 0x1,
    CARD_DISP_FLAG_NO_CARD = 0x2,
    CARD_DISP_FLAG_SELECTED = 0x4,
    CARD_DISP_FLAG_DOUBLE_SIZE = 0x8,
    CARD_DISP_FLAG_DEALING = 0x10,
    CARD_DISP_FLAG_OPEN = 0x20,
    CARD_DISP_FLAG_SETTLED = 0x40,
    CARD_DISP_FLAG_GFX_LOADED = 0x80,
    CARD_DISP_FLAG_STOCKED = 0x200,
    CARD_DISP_FLAG_VISIBLE = 0x800,
    CARD_DISP_FLAG_FROZEN = 0x1000,
    CARD_DISP_FLAG_IN_PLAY = 0x2000,
    CARD_DISP_FLAG_REMOVE = 0x4000,
    CARD_DISP_FLAG_UNOPPOSED = 0x8000,
    CARD_DISP_FLAG_RELOAD_CARD = 0x100000,
    CARD_DISP_FLAG_BROKEN = 0x200000,
    CARD_DISP_FLAG_SPIN_MIRRORED = 0x400000,
    CARD_DISP_FLAG_RELOAD_GAUGE = 0x1000000,
    CARD_DISP_FLAG_RELOAD_DONE = 0x4000000,
    CARD_DISP_FLAG_STOCK_NAMED = 0x10000000
};

enum CardDisplayCommand {
    CARD_DISP_COMMAND_NONE,
    CARD_DISP_COMMAND_PLAY = 5,
    CARD_DISP_COMMAND_STOCK,
    CARD_DISP_COMMAND_REMOVE,
    CARD_DISP_COMMAND_FLY_OFF,
    CARD_DISP_COMMAND_SLIDE_OUT,
    CARD_DISP_COMMAND_HEARTLESS,
    CARD_DISP_COMMAND_GIMMICK
};

enum CardStockPhase {
    CARD_STOCK_PHASE_RISE,
    CARD_STOCK_PHASE_DROP
};

typedef struct CardDisplayWork {
    void* tiles;
    void* tiles2;
    void* tiles3;
    void* tiles4;
    void* tiles5;
    void* palette;
    void* palette2;
    void* children;
    struct ReloadGauge* reloadGauge;
    TaskPool tasks;
    CardDisplayArgs args;
    const CardDef* cardDef;
    s32 x;
    s32 y;
    s32 scaleX;
    s32 scaleY;
    u16 enemyKind;
    u8 angle;
    u8 bobAngle;
    u8 unk_60[0x04];
    ListNode node;
    u32 flags;
    s32 ringAngle;
    s32 ringAngleTarget;
    s32 ringRadius;
    s32 ringRadiusTarget;
    s32 ringCenterX;
    s32 ringCenterY;
    s32 swingAngle;
    s32 swingAngleTarget;
    u16 timer;
    u8 spinSpeed;
    u8 stockIndex;
    u8 priority;
    u8 command;
    u8 phase;
    u8 swingSteps;
    s8 ringIndex;
    u8 value;
    u8 premium;
    u8 valueModified;
} CardDisplayWork;

STATIC_ASSERT(sizeof(CardDisplayWork) == 0xA8, CardDisplayWorkSize);

enum DeckCardSet {
    DECK_CARD_SET_MAIN,
    DECK_CARD_SET_ENEMY
};

enum CardList {
    CARD_LIST_MAIN,
    CARD_LIST_MAGIC,
    CARD_LIST_ITEM,
    CARD_LIST_ENEMY,
    CARD_LIST_NONE = 0xFF
};

typedef struct CardBattleWork {
    TaskPool tasks;
    void* tiles;
    void* palette;
    CardDisplayWork* playedCards[3];
    CardDisplayWork* stock[3];
    CardDisplayWork* selectedCards[4];
    CardSlot* slots[4];
    ListPool cardDisplays[4];
    u16 cursors[4];
    s16 reloadCounts[4];
    s16 x;
    s16 timer;
    s16 slotCounts[4];
    s16 cardsLeft[4];
    s8 listIndex;
    u8 stockCount;
    u8 stockValue;
    u8 unk_BB;
    u8 revCountShown[4];
    u8 reloadPending[4];
    u8 reloadShown;
    u8 stockNameChecked;
    u8 dealtCount;
    u8 xSteps;
    u8 actionTaken;
    u8 cardsClosed;
} CardBattleWork;

STATIC_ASSERT(sizeof(CardBattleWork) == 0xCC, CardBattleWorkSize);

typedef struct CardListWork {
    ListPool cards;
    struct PremireChanceCardWork* selectedCard;
    TaskPool effectTasks;
    u8 effectCount;
    u8 unk_29;
} CardListWork;

STATIC_ASSERT(sizeof(CardListWork) == 0x2C, CardListWorkSize);

extern void* gLvupEffectSprites[];

typedef struct EventMapObjectWork {
    u8 background;
    void* tiles[0x0A];
    ObjPalette* palettes[0x0A];
    u8 unk_54[0x04];
    struct EventMapObjectDef* definition;
} EventMapObjectWork;

struct MapCardDef;
struct MapCardBackDef;

typedef struct PrizeMapCardWork {
    void* tiles;
    ObjPalette* palette;
    void* tiles2;
    void* tiles3;
    ObjPalette* palette2;
    void* tiles4;
    void* tiles5;
    ObjPalette* palette3;
    struct MapCardDef* cardDef;
    struct MapCardBackDef* cardBack;
    TaskPool tasks;
    u16 kind;
    u16 value;
    s32 unk_40;
    Collider collider;
    s32 posX;
    s32 posY;
    s32 posZ;
    s32 groundZ;
    s32 cardId;
    s32 vz;
    s32 speed;
    s32 dirX;
    s32 dirY;
    s32 distance;
    s16 scaleX;
    s16 scaleY;
    u16 priority;
    s16 x;
    s16 y;
    s16 targetX;
    s16 targetY;
    s16 shadowX;
    s16 shadowY;
    s16 scale;
    u16 moveAngle;
    u8 rotation;
    u8 flipAngleY;
    u8 flipAngleX;
    u8 timer;
    u8 steps;
    u8 holdTimer;
    u8 unk_E4;
    u8 collected;
    u8 backAnimTimer;
    u8 backAnimStep;
    u8 backFrame;
} PrizeMapCardWork;

STATIC_ASSERT(sizeof(struct PrizeMapCardWork) == 0xEC, PrizeMapCardWorkSize);

typedef struct PickupCardWork {
    void* tiles;
    void* palette;
    void* tiles2;
    void* palette2;
    void* tiles3;
    void* tiles4;
    void* palette3;
    const CardDef* cardDef;
    TaskPool tasks;
    u8 unk_34[0x04];
    s32 posX;
    s32 posY;
    s32 posZ;
    s32 floor;
    u8 unk_48[0xFC];
    Collider collider;
    s32 cardId;
    s32 vz;
    s32 speed;
    s32 distance;
    s32 dirX;
    s32 dirY;
    s16 scaleX;
    s16 scaleY;
    s16 scale;
    s16 x;
    s16 y;
    u16 priority;
    u16 timer;
    u8 moveAngle;
    u8 flipAngleX;
    u8 flipAngleY;
    u8 angle;
    u8 screenSpace;
    u8 unk_1CB;
    u8 unk_1CC;
    u8 visible;
    u8 backCategory;
    u16 spriteFlags;
} PickupCardWork;

STATIC_ASSERT(sizeof(struct PickupCardWork) == 0x1D4, PickupCardWorkSize);

STATIC_ASSERT(sizeof(struct CardStat) == 0x18, CardStatSize);

typedef struct UnkStruct_080993D4 {
    u8 unk_000[0xE4];
    s16 unk_0E4;
} UnkStruct_080993D4;

enum CardSide {
    CARD_SIDE_SORA = 1,
    CARD_SIDE_RIKU
};

typedef struct RevCountArgs {
    u8* shownList;
    s16* count;
    u8* visible;
    u8 list;
    u8 side;
} RevCountArgs;

typedef struct RevCountWork {
    void* tiles;
    void* palette;
    void* gfx;
    AnimState anim;
    u8 list;
    u16 shownCount;
    u8 steps;
    RevCountArgs args;
    s32 x;
    s32 y;
} RevCountWork;

STATIC_ASSERT(sizeof(struct RevCountWork) == 0x44, RevCountWorkSize);

extern const s32 gLvupEffectStartOffsetX[];
extern const s32 gLvupEffectStartOffsetY[];
extern const u16 gLvupEffectStartAngles[];
extern u16 gRandomHcEffects[47];

typedef struct WorldSelAnim {
    u8 palette;
    u8 duration;
    u8 unk_02[0x2];
} WorldSelAnim;

extern WorldSelAnim gWorldSelAnims[30];
extern const s32 gSysmsgwinChoiceCursorX[];
extern u8 gDeckClearConfirmText[];

enum CardMessageMode {
    CARD_MESSAGE_MODE_BG_WINDOW,
    CARD_MESSAGE_MODE_BG_WINDOW_PERSISTENT,
    CARD_MESSAGE_MODE_SPRITE_WINDOW,
    CARD_MESSAGE_MODE_SPRITE_WINDOW_PERSISTENT
};

typedef struct CardMessageArgs {
    u32 bg;
    u32 messageId : 16;
    u32 unk_06 : 8;
    u32 mode : 8;
} CardMessageArgs;

enum DeckCard2Panel {
    DECK_CARD2_PANEL_DECK,
    DECK_CARD2_PANEL_COLLECTION
};

typedef struct DeckCard2Args {
    void* pool;
    u16 cardId;
    s16 col;
    s16 row;
    u8 panel;
    u16* slot;
} DeckCard2Args;

enum DeckCard2Flag {
    DECK_CARD2_FLAG_GFX_LOADED = 0x1
};

typedef struct DeckCard2Work {
    u8 unk_00[0x04];
    ObjPalette* palette2;
    ObjTiles* tiles;
    ObjPalette* palette;
    ObjTiles* tiles2;
    const CardDef* cardDef;
    const CardBack* cardBack;
    DeckCard2Args args;
    ListNode node;
    s32 x;
    s32 y;
    u16 flags;
    u8 done;
    u8 unk_4B[0x02];
    u8 premium;
} DeckCard2Work;

enum PremireChanceCardState {
    PREMIRE_CHANCE_CARD_STATE_DEAL,
    PREMIRE_CHANCE_CARD_STATE_SPIN,
    PREMIRE_CHANCE_CARD_STATE_TO_CENTER,
    PREMIRE_CHANCE_CARD_STATE_MOVE_AWAY,
    PREMIRE_CHANCE_CARD_STATE_HIDDEN = 0xFF
};

typedef struct PremireChanceCardWork {
    const CardDef* cardDef;
    const CardBack* cardBack;
    void* tiles;
    void* tiles2;
    void* tiles3;
    void* unk_14;
    void* tiles4;
    void* tiles5;
    ObjPalette* palette;
    ObjPalette* palette2;
    ObjPalette* palette3;
    AnimState anim;
    void* gfx;
    u16 deckIndex;
    s16 x;
    s16 y;
    s16 angle;
    s16 radius;
    s8 position;
    u8 steps;
    u8 gfxLoaded;
    u8 state;
    ListNode node;
    s16 scaleX;
    s16 scaleY;
    u16 offsetX;
    u16 offsetY;
    u8 premium;
} PremireChanceCardWork;

enum DeckMenuView {
    DECK_MENU_VIEW_DECK_GRID,
    DECK_MENU_VIEW_DECK_SELECT,
    DECK_MENU_VIEW_DECK_FILTER,
    DECK_MENU_VIEW_COMMANDS,
    DECK_MENU_VIEW_ADD_GRID,
    DECK_MENU_VIEW_ADD_VALUE_SELECT,
    DECK_MENU_VIEW_ADD_FILTER,
    DECK_MENU_VIEW_REMOVE_GRID,
    DECK_MENU_VIEW_REMOVE_FILTER,
    DECK_MENU_VIEW_DELETE_GRID,
    DECK_MENU_VIEW_DELETE_FILTER,
    DECK_MENU_VIEW_DELETE_VALUE_SELECT,
    DECK_MENU_VIEW_DELETE_PROMPT,
    DECK_MENU_VIEW_KEYBOARD,
    DECK_MENU_VIEW_CLEAR_PROMPT = 15
};

enum DeckMenuMode {
    DECK_MENU_MODE_NONE,
    DECK_MENU_MODE_REMOVE,
    DECK_MENU_MODE_ADD,
    DECK_MENU_MODE_DELETE
};

enum CategoryFilter {
    CATEGORY_FILTER_DECK_ALL,
    CATEGORY_FILTER_ATTACK,
    CATEGORY_FILTER_MAGIC,
    CATEGORY_FILTER_ITEM,
    CATEGORY_FILTER_ENEMY,
    CATEGORY_FILTER_COLLECTION_ALL
};

enum DeckMenuResult {
    DECK_MENU_RESULT_NONE,
    DECK_MENU_RESULT_CLOSED = 6,
    DECK_MENU_RESULT_RETURN_TO_MAP,
    DECK_MENU_RESULT_RETURN_TO_MENU
};

enum DeckMenuSlideInStep {
    DECK_MENU_SLIDE_IN_STEP_VERTICAL,
    DECK_MENU_SLIDE_IN_STEP_HORIZONTAL
};

enum DeckFrameCursor {
    DECK_FRAME_CURSOR_CARD,
    DECK_FRAME_CURSOR_ROW
};

typedef struct DeckExchangeWork {
    ObjTiles* tiles;
    ObjTiles* tiles2;
    ObjTiles* tiles3;
    ObjPalette* palette;
    ObjTiles* tiles4;
    ObjTiles* tiles5;
    ObjTiles* tiles6;
    ObjTiles* tiles7;
    ObjPalette* palette2;
    ObjPalette* palette3;
    TextSlot textSlots[8];
    TextSlot textSlots2[8];
    TextSlot textSlots3[8];
    TextSlot textSlots4[30];
    TextSlot textSlots5[90];
    ObjTiles* tiles8;
    ObjPalette* palette5;
    ObjTiles* tiles9;
    ObjPalette* palette6;
    ObjPalette* palette7;
    ObjPalette* palette4;
    void* cursorCard;
    void* prevCursorCard;
    u8 unk_4C8[4];
    struct CardKindEntry* entries;
    struct CardKindEntry* kindEntries;
    void* gfx3;
    void* gfx4;
    void* gfx5;
    u8 unk_4E0[8];
    void* gfx;
    void* gfx2;
    u8 unk_4F0[4];
    u8 unk_4F4[0x120];
    TaskPool tasks;
    TaskPool tasks2;
    ListPool pool;
    AnimState anim;
    AnimState anim2;
    u8 unk_67C[0x18];
    s32 handX;
    s32 handY;
    s32 thumbX;
    s32 thumbY;
    s32 topBarX;
    s32 bottomBarX;
    s32 topBarY;
    s32 bottomBarY;
    s32 bannerX;
    s32 heldX;
    s32 heldY;
    u8 unk_6C0[2];
    u16 heldRow;
    u16 unk_6C4;
    u16 unk_6C6;
    u16 unk_6C8;
    u16 unk_6CA;
    u16 entryIndex;
    u16 handFlags;
    s16 cursorCol;
    s16 cursorRow;
    s16 deckNameX;
    s16 deckName2X;
    s16 deckName3X;
    s16 deckNameY;
    s16 deckName2Y;
    s16 deckName3Y;
    u16 entryCount;
    u16 collectionCategoryCounts[4];
    u8 unk_6EA[2];
    s16 scrollRowEnd;
    s16 rowCount;
    u8 view;
    u8 unk_6F1;
    u8 prevCursorCol;
    u8 prevCursorRow;
    u8 savedCol;
    u8 savedRow;
    u8 timer;
    u8 deckAttackCount;
    u8 deckMagicCount;
    u8 deckItemCount;
    u8 deckEnemyCount;
    u8* resultOut;
    u8 deckIndex;
    u8 categoryFilter;
    u8 textSlotCount;
    u8 textSlotCount2;
    u8 textSlotCount3;
    u8 textSlotCount4;
    u8 mode;
    u8 commandCursor;
    s16 descriptionX;
    s16 descriptionY;
    u8 textSlotCount5;
    u8 popupActive;
    u8 unk_70E;
    u8 unk_70F;
    u8 exitRequested;
    u8 barSlideTimer;
    u8 bannerSlideTimer;
    u8 inputDelay;
    u8 holding;
    u8 step;
    u16 gridEntryCount;
} DeckExchangeWork;

STATIC_ASSERT(sizeof(DeckExchangeWork) == 0x718, DeckExchangeWorkSize);

typedef struct CardKindEntry {
    u16 valueCounts[0x0A];
    u16 kind;
    u16 count;
    u16 indexCount;
    u16* indices;
} CardKindEntry;

typedef struct DeckMenuWork {
    ObjTiles* tiles;
    ObjTiles* tiles2;
    ObjTiles* tiles3;
    ObjPalette* palette2;
    ObjTiles* tiles4;
    ObjPalette* palette;
    ObjTiles* tiles12;
    ObjTiles* tiles7;
    ObjTiles* tiles8;
    ObjTiles* tiles9;
    ObjTiles* tiles10;
    void* unk_02C;
    ObjPalette* palette5;
    ObjPalette* palette6;
    TextSlot textSlots[8];
    TextSlot textSlots2[8];
    TextSlot textSlots3[8];
    TextSlot textSlots4[30];
    TextSlot textSlots5[90];
    ObjTiles* tiles5;
    ObjTiles* tiles6;
    ObjPalette* palette3;
    ObjPalette* palette4;
    DeckCard2Work* cursorCard;
    DeckCard2Work* prevCursorCard;
    void* unk_4D0;
    CardKindEntry* entries;
    CardKindEntry* kindEntries;
    void* gfx4;
    void* gfx5;
    void* gfx6;
    void* gfx7;
    void* gfx8;
    void* gfx;
    void* gfx2;
    void* gfx3;
    u8 unk_4FC[0x23C];
    ObjTiles* tiles11;
    ObjTiles* tiles13;
    ObjPalette* palette7;
    TextSlot textSlots6[8];
    u8 nameBuffer[20];
    AnimState anim4;
    void* gfx9;
    s32 keyCursorX;
    s32 keyCursorY;
    s32 caretX;
    union {
        struct {
            s16 x;
            s16 y;
        } parts;
        u32 packed;
    } cursor;
    u8 textSlotCount6;
    u8 unk_7C5;
    u8 keyCursorSteps;
    u8 keyboardPage;
#ifdef VERSION_EU
    u8 onEndKey;
#endif
    TaskPool taskpool;
    TaskPool cardpool;
    ListPool pool;
    AnimState anim2;
    AnimState anim3;
    AnimState anim;
    s32 handX;
    s32 handY;
    s32 thumbX;
    s32 thumbY;
    s32 topBarX;
    s32 bottomBarX;
    s32 topBarY;
    s32 bottomBarY;
    s32 bannerX;
    s32 heldX;
    s32 heldY;
    s16 heldCol;
    s16 heldRow;
    s16 removeLabelX;
    s16 removeLabelY;
    s16 addLabelX;
    s16 addLabelY;
    u16 entryIndex;
    u16 handFlags;
    s16 cursorCol;
    s16 cursorRow;
    s16 deckNameX;
    s16 deckName2X;
    s16 deckName3X;
    s16 deckNameY;
    s16 deckName2Y;
    s16 deckName3Y;
    s16 descriptionX;
    s16 descriptionY;
    u16 entryCount;
    u16 collectionCategoryCounts[4];
    u16 deckAttackCount;
    u16 deckMagicCount;
    u16 deckItemCount;
    u16 deckEnemyCount;
    u8 unk_8AA[2];
    s16 scrollRowEnd;
    s16 rowCount;
    u8 handVisible;
    u8 view;
    u8 prevView;
    u8 prevCursor[2];
    s8 savedCol;
    s8 savedRow;
    u8 timer;
    u8 unk_8B8[4];
    u8* resultOut;
    u8 deckIndex;
    u8 categoryFilter;
    u8 textSlotCount;
    u8 textSlotCount2;
    u8 textSlotCount3;
    u8 textSlotCount4;
    u8 textSlotCount5;
    u8 mode;
    u8 commandCursor;
    u8 popupActive;
    u8 unk_8CA;
    u8 exitRequested;
    u8 barSlideTimer;
    u8 bannerSlideTimer;
    s8 inputDelay;
    u8 holding;
    u8 step;
    u8 promptChoice;
    u8 result;
    u16 gridEntryCount;
} DeckMenuWork;

typedef struct RikuDeckMenuWork {
    ObjTiles* tiles;
    ObjTiles* tiles2;
    ObjTiles* tiles3;
    ObjPalette* palette2;
    ObjTiles* tiles4;
    ObjPalette* palette;
    ObjTiles* tiles7;
    ObjTiles* tiles8;
    ObjTiles* tiles9;
    ObjTiles* tiles10;
    ObjPalette* palette5;
    ObjPalette* palette6;
    TextSlot textSlots[8];
    TextSlot textSlots2[8];
    TextSlot textSlots3[8];
    TextSlot textSlots4[30];
    // @bug A card description can take more than 60 slots (overflow).
    TextSlot textSlots5[60];
    u8 unk_3C0[4];
    ObjTiles* tiles6;
    ObjPalette* palette3;
    ObjTiles* tiles12;
    u8 unk_3D0[8];
    ObjPalette* palette4;
    DeckCard2Work* cursorCard;
    DeckCard2Work* prevCursorCard;
    u8 unk_3E4[4];
    CardKindEntry* entries;
    void* gfx4;
    void* gfx5;
    void* gfx6;
    u8 unk_3F8[8];
    void* gfx;
    void* gfx2;
    void* gfx3;
    TaskPool taskpool;
    TaskPool cardpool;
    ListPool pool;
    AnimState anim2;
    AnimState anim3;
    AnimState anim;
    s32 handX;
    s32 handY;
    s32 thumbX;
    s32 thumbY;
    s32 topBarX;
    s32 bottomBarX;
    s32 topBarY;
    s32 bottomBarY;
    s32 bannerX;
    u8 unk_4B0[4];
    s32 heldY;
    u8 unk_4B8[2];
    s16 heldRow;
    u16 unk_4BC;
    u16 unk_4BE;
    u16 unk_4C0;
    u16 unk_4C2;
    u8 unk_4C4[2];
    u16 handFlags;
    s16 cursorCol;
    s16 cursorRow;
    u8 unk_4CC[0xC];
    s16 descriptionX;
    s16 descriptionY;
    u16 entryCount;
    u8 unk_4DE[8];
    u8 view;
    u8 unk_4E7;
    u8 prevCursorCol;
    u8 prevCursorRow;
    u8 unk_4EA[2];
    u8 timer;
    u8 unk_4ED;
    u8 scrollRowEnd;
    u8 deckAttackCount;
    u8 deckMagicCount;
    u8 deckItemCount;
    u8 deckEnemyCount;
    u8* resultOut;
    u8 deckIndex;
    u8 categoryFilter;
    u8 textSlotCount;
    u8 textSlotCount2;
    u8 textSlotCount3;
    u8 textSlotCount4;
    u8 textSlotCount5;
    u8 mode;
    u8 commandCursor;
    u8 popupActive;
    u8 unk_502;
    u8 unk_503;
    u8 exitRequested;
    u8 barSlideTimer;
    u8 bannerSlideTimer;
    s8 inputDelay;
    u8 holding;
    u8 step;
    u8 handVisible;
    u8 previewShown;
    u8 result;
} RikuDeckMenuWork;

typedef struct HcEffectNameWork {
    s16 x;
    u8 unk_02[0x06];
    void* tiles2;
    void* tiles3;
    void* palette;
    void* tiles;
    u8 unk_18;
    u8 side;
    u16 timer;
    u16 blinkInterval;
    u16 effect;
    u16 randomIndex;
    u8 countThousands;
    u8 countHundreds;
    u8 countTens;
    u8 countOnes;
    u8 countUnit;
    u8 visible;
} HcEffectNameWork;

struct PremireChanceWork;

typedef struct StockNameWork {
    u8 unk_00[4];
    u16 unk_04;
    ObjTiles* tiles;
    ObjPalette* palette;
    u8 unk_10;
    u8 stockNameIndex;
    u32 stockName;
    s32 stockNames[6];
    u8 cycling;
    u8 visible;
} StockNameWork;

enum ReloadChildFlag {
    RELOAD_CHILD_FLAG_SHIFTED = 0x1,
    RELOAD_CHILD_FLAG_IDLE = 0x2
};

typedef struct ReloadChildArgs {
    ListPool* pool;
    s32* parentX;
    s32* parentY;
    u8 index;
    u8 listIndex;
    u8 side;
    u16 flags;
} ReloadChildArgs;

typedef struct ReloadChildWork {
    void* tiles;
    void* palette;
    void* tiles2;
    ReloadChildArgs args;
    s32 offsetX;
    s32 offsetY;
    s32 scale;
    u8 unk_2C[0x04];
    ListNode node;
    u8 steps;
    u8 angle;
    u8 retractTimer;
} ReloadChildWork;

STATIC_ASSERT(sizeof(struct ReloadChildWork) == 0x48, ReloadChildWorkSize);

typedef struct PremiumCardEffectWork {
    void* tiles;
    void* palette;
    void* gfx;
    AnimState anim;
    s32 centerX;
    s32 centerY;
    s32 fallY;
    s32 x;
    s32 y;
    s32 unk_38;
    s32 angle;
    s32 radius;
    s32 vx;
    s32 vy;
    s32 speed;
    s32 fallSpeed;
} PremiumCardEffectWork;

STATIC_ASSERT(sizeof(struct PremiumCardEffectWork) == 0x54, PremiumCardEffectWorkSize);

typedef struct CardNameWork {
    void* tiles;
    ObjPalette* palette2;
    TextSlot textSlots[32];
    TextSlot textSlots2[32];
#ifdef VERSION_EU
    TextSlot textSlots3[32];
#else
    TextSlot textSlots3[2];
#endif
    ObjPalette* textPalette;
    ObjPalette* palette;
    s16 nameX;
    s16 messageX;
    s16 suffixX;
    u8 textSlotCount;
    u8 textSlotCount2;
    u8 textSlotCount3;
} CardNameWork;

typedef struct PrintWork {
    u32 unk_00;
} PrintWork;

#ifdef VERSION_EU
STATIC_ASSERT(sizeof(struct CardNameWork) == 0x31C, CardNameWorkSize);
#else
STATIC_ASSERT(sizeof(struct CardNameWork) == 0x22C, CardNameWorkSize);
#endif

typedef struct DarkPointWork {
    ObjTiles* tiles;
    s32 x;
    u8 unk_08[0x02];
    s8 slideTimer;
    u8 thousands;
    u8 hundreds;
    u8 tens;
    u8 ones;
} DarkPointWork;

STATIC_ASSERT(sizeof(struct DarkPointWork) == 0x10, DarkPointWorkSize);

typedef struct MapcardArgs {
    u8 baseCardId;
    u8 index;
    u8 kindCount;
    u8 count;
    struct MapSelectWork* parent;
    ListPool* pool;
    u8 unk_0C[0x0C];
} MapcardArgs;

STATIC_ASSERT(sizeof(MapcardArgs) == 0x18, MapcardArgsSize);

enum MapcardFlag {
    MAPCARD_FLAG_GFX_LOADED = 0x1,
    MAPCARD_FLAG_RAISED = 0x2,
    MAPCARD_FLAG_CHOSEN = 0x40,
    MAPCARD_FLAG_DELIVERED = 0x80,
    MAPCARD_FLAG_CURSOR = 0x100,
    MAPCARD_FLAG_OPENED = 0x200,
    MAPCARD_FLAG_REMOVED = 0x400
};

typedef struct MapcardWork {
    void* tiles;
    void* unk_04;
    void* tiles2;
    ObjPalette* palette;
    void* tiles3;
    ObjPalette* palette2;
    MapCardDef* cardDef;
    MapCardBackDef* cardBack;
    MapcardArgs args;
    ListNode node;
    s32 x;
    s32 y;
    s32 dirX;
    s32 dirY;
    s32 deceleration;
    s32 speed;
    s32 distance;
    u16 scale;
    u16 priority;
    u16 flags;
    u8 angle;
    u8 steps;
    u8 holdTimer;
    u8 unk_71;
    u8 unk_72;
    u8 unk_73;
    u8 value;
} MapcardWork;

STATIC_ASSERT(sizeof(MapcardWork) == 0x78, MapcardWorkSize);

typedef struct ReloadGauge {
    s16 sine;
    u16 angle;
    s32 offsetX;
    u8 unk_08[0x05];
    u8 gaugeAnim;
    AnimState anim;
    AnimState anim2;
    AnimState anim3;
    void* gfx;
    void* gfx2;
    void* gfx3;
    s8 reloadCounter;
    u8 chargeTick;
} ReloadGauge;

STATIC_ASSERT(sizeof(ReloadGauge) == 0x68, ReloadGaugeSize);

typedef struct PrintLine {
    u8 length;
    u8 x;
    u8 y;
    u8 palette;
    u16 tilemap[32];
} PrintLine;

typedef struct NumberPlusArgs {
    s32 cardDef;
    s32 x;
    s32 y;
    s32 scaleX;
    s32 scaleY;
    s32 unk_14;
    s32 unk_18;
} NumberPlusArgs;

typedef struct NumberPlusWork {
    void* tiles;
    void* palette;
    NumberPlusArgs args;
    s16 x;
    s16 y;
    u8 steps;
    u8 unk_29;
} NumberPlusWork;

typedef struct MapTileAnimationWork {
    u8 firstTrack;
    u8 frameTimers[8];
    u8 frameIndices[8];
    const MapTileAnimationDef* definition;
} MapTileAnimationWork;

typedef struct PrizeCardArgs {
    s32 unk_00[8];
} PrizeCardArgs;

typedef struct PrizeCardTaskArgs {
    s32 x;
    s32 y;
    s32 z;
    u8 unk_0C[0x14];
    s32 cardId;
} PrizeCardTaskArgs;

typedef struct PrizeCardInitWork {
    TaskPool tasks;
    u8 spawned;
    PrizeCardArgs args;
} PrizeCardInitWork;

typedef struct ScrollBarWork {
    u8 unk_00[0x08];
    u16 unk_08;
    u16 unk_0A;
    u16 unk_0C;
    u16 unk_0E;
    u16 position;
    u16 remaining;
    u16 count;
    u8 active;
    u8 unk_17;
} ScrollBarWork;

STATIC_ASSERT(sizeof(struct ScrollBarWork) == 0x18, ScrollBarWorkSize);

typedef struct PrizeCardWork {
    ObjTiles* tiles;
    ObjPalette* palette;
    ObjTiles* tiles2;
    ObjTiles* tiles3;
    ObjPalette* palette2;
    ObjTiles* tiles4;
    ObjTiles* tiles5;
    ObjPalette* palette3;
    TaskPool tasks;
    CardStat stat;
    Collider collider;
    FldPos pos;
    FldPos prevPos;
    u32 cardId;
    s32 vz;
    s32 speed;
    s32 dirX;
    s32 dirY;
    s32 distance;
    s16 scaleX;
    s16 scaleY;
    s16 priority;
    s16 x;
    s16 y;
    s16 targetX;
    s16 targetY;
    s16 shadowX;
    s16 shadowY;
    s16 scale;
    s16 moveAngle;
    u8 rotation;
    u8 flipAngleY;
    u8 flipAngleX;
    u8 timer;
    u8 steps;
    u8 holdTimer;
    u8 collected[0x04];
} PrizeCardWork;

typedef struct PrizeMapCardEntry {
    u16 cardId;
    u16 unk_02;
} PrizeMapCardEntry;

typedef struct PrizeMapCardGroup {
    const PrizeMapCardEntry* entries;
    u16 count;
    u16 chance;
} PrizeMapCardGroup;

typedef struct PrizeMapCardGroupList {
    const PrizeMapCardGroup* data;
    u16 size;
} PrizeMapCardGroupList;

extern const s32 gSoraCardLayout[][2];
extern const s32 gSoraCardRingAngles[];
extern const s32 gSoraCardSwingAngles[];
extern const s32 gRikuCardLayout[][2];
extern const s32 gPlayedCardCenter[];
extern const s32 gPlayedCardAngles[];

extern const s16 gDeckGridColumnX[];
extern const s16 gDeckGridRowY[];

typedef struct PromptChoiceLayout {
    s32 x[2];
} PromptChoiceLayout;

typedef struct MapSelectKindEntry {
    u16 baseCardId;
    u16 count;
} MapSelectKindEntry;

typedef struct SpotlightWork {
    u8 steps;
    s32 blendB;
    s32 blendA;
    u16 bldAlpha;
    u8* endFlag;
    u8 ownEndFlag;
} SpotlightWork;

typedef struct DispCardnameWork {
    TextSlot textSlots[32];
    void* tiles;
    ObjPalette* textPalette;
    ObjPalette* palette;
    s16 x;
    u8 textSlotCount;
} DispCardnameWork;

STATIC_ASSERT(sizeof(DispCardnameWork) == 0x110, DispCardnameWorkSize);

typedef struct VersionWork {
    void* tiles;
    void* palette;
    u16 text[16];
    u8 textLength;
} VersionWork;

STATIC_ASSERT(sizeof(VersionWork) == 0x2C, VersionWorkSize);

typedef struct CardEffectArgs {
    s32 x;
    s32 y;
    s32 z;
    u8 screenSpace;
    u8* count;
} CardEffectArgs;

typedef struct CardEffectWork {
    void* tiles;
    void* palette;
    AnimState anim;
    void* gfx;
    s32 posX;
    s32 posY;
    s32 posZ;
    s16 x;
    s16 y;
    u16 priority;
    CardEffectArgs args;
} CardEffectWork;

typedef struct BossPrizeWork {
    ObjTiles* tiles;
    ObjPalette* palette;
    ObjTiles* tiles2;
    ObjTiles* tiles3;
    ObjPalette* palette2;
    ObjTiles* tiles4;
    ObjTiles* tiles5;
    ObjPalette* palette3;
    TaskPool tasks;
    CardStat stat;
    Collider collider;
    s32 posX;
    s32 posY;
    s32 posZ;
    s32 groundZ;
    s32 cardId;
    s32 vz;
    s32 speed;
    s32 dirX;
    s32 dirY;
    s32 distance;
    s16 scaleX;
    s16 scaleY;
    u16 priority;
    s16 x;
    s16 y;
    s16 targetX;
    s16 targetY;
    s16 shadowX;
    s16 shadowY;
    s16 scale;
    s16 moveAngle;
    u8 rotation;
    u8 unk_E7;
    u8 flipAngleY;
    u8 flipAngleX;
    u8 timer;
    u8 steps;
    u8 holdTimer;
    u8 collected;
    u8 effectCount;
    u8 effectTimer;
} BossPrizeWork;

STATIC_ASSERT(sizeof(struct BossPrizeWork) == 0xF0, BossPrizeWorkSize);

typedef struct EventMapObjectPlacement {
    s32 x : 24;
    s32 unk_03 : 8;
    s32 y : 24;
    s32 unk_07 : 8;
    u8 spriteIndex;
    u8 unk_09[0x03];
} EventMapObjectPlacement;

typedef struct EventMapObjectDef {
    PrizeMapCardGroupList* tileResources;
    PrizeMapCardGroupList* paletteResources;
    void** sprites;
    EventMapObjectPlacement* placements;
    u16 placementCount;
} EventMapObjectDef;

enum SelmapEventKeyCloseMode {
    SELMAP_EVENT_KEY_CLOSE_NONE,
    SELMAP_EVENT_KEY_CLOSE_ACCEPTED,
    SELMAP_EVENT_KEY_CLOSE_CANCELLED
};

typedef struct SelmapEventKeyArgs {
    ObjPalette* palette;
    void* unk_04;
    s32 closeMode;
} SelmapEventKeyArgs;

typedef struct MapSelectWork {
    TaskPool tasks;
    ListPool cards;
    u8 unk_024[8];
    ObjTiles* tiles4;
    ObjPalette* palette;
    void* tiles2;
    void* tiles3;
    void* tiles;
    ObjPalette* palette2;
    void* tiles5;
    ObjPalette* palette3;
    TextSlot textSlots[48];
    ObjPalette* palette4;
    u8 unk_1D0[0x10];
    void* tiles6;
    void* unk_1E4;
    void* tiles7;
    MapcardWork* card;
    MapcardWork* prevCard;
    MapcardWork* card2;
    AnimState anim;
    AnimState anim2;
    u8 unk_228[0x10];
    struct SelmapEventKeyWork* eventKey;
    s32 openedCardX;
    s32 bgScrollY;
    s32 nameY;
    s32 titleX;
    s32 titleY;
    s32 x;
    s32 y;
    s32 handX;
    s32 handY;
    s32 unk_260;
    s32 cursorTargetX;
    s32 cursorTargetY;
    s32 topBarY;
    s32 bottomBarY;
    void* gfx;
    void* gfx2;
    u16 kindCount;
    u16 requiredValue;
    s16 savedCursorX;
    s16 savedCursorY;
    u8 page;
    u8 lastPage;
    u8 messageTimer;
    u8 textSlotCounts[0x04];
    u8 steps;
    u8 steps2;
    u8 unk_28D[0x02];
    u8 slideSteps;
    u8 barSteps;
    u8* status;
    u8 pageScroll;
    u8 mosaicX;
    u8 mosaicY;
    u8 mosaicTimer;
    s8 valueColumn;
    s8 valueRow;
    u8 paletteBuffer[0x20];
    u8 isEventDoor;
    u8 cancelled;
    u8 scrollBarVisible;
    u8 inTutorial;
    u8 tutorialMessage;
    SelmapEventKeyArgs eventKeyArgs;
    u8 valueCounts[0x0A];
    u8 remainingKeys;
    void* nextEventKey;
    MapSelectKindEntry* kindEntries;
} MapSelectWork;

STATIC_ASSERT(sizeof(struct MapSelectWork) == 0x2E4, MapSelectWorkSize);

typedef struct LvupMsgWork {
    TextSlot textSlots[20];
    TextSlot textSlots2[20];
    TextSlot textSlots3[20];
#ifndef VERSION_JP
    TextSlot textSlots4[20];
#endif
    ObjPalette* textPalette;
    void* tiles;
    void* palette;
    u16 amount;
    s32 y;
    s32 x;
    s32 x2;
    s32 x3;
    s32 y2;
    s32 y3;
    s32 y4;
    s8 slideSteps;
    u8 textSlotCount;
    u8 textSlotCount2;
    u8 textSlotCount3;
#ifndef VERSION_JP
    u8 textSlotCount4;
#endif
    u8 unk_2B1;
    u8* active;
} LvupMsgWork;

typedef struct DeckConfirmWork {
    TextSlot textSlots[80];
    TextSlot textSlots2[80];
    TextSlot textSlots3[80];
    ObjTiles* tiles;
    ObjPalette* palette;
    ObjPalette* palette2;
    u8 textSlotCount;
    u8 textSlotCount2;
    u8 textSlotCount3;
    s16 unk_790;
    s16 x;
    s16 x2;
    s16 y;
    s16 y2;
    s16 x3;
    s16 y3;
    u8* active;
    u8 unk_7A4;
} DeckConfirmWork;

struct BtlObj;

typedef struct LevelUpEffectArgs {
    s32 x;
    s32 y;
    u8 unk_08;
    struct BtlObj* target;
    void* tiles;
    ObjPalette* palette;
} LevelUpEffectArgs;

typedef struct LevelUpEffectWork {
    void* tiles;
    void* palette;
    void* unk_08;
    struct BtlObj* target;
    s32 centerX[4];
    s32 centerY[4];
    s32 radius;
    s32 x[4];
    s32 y[4];
    s32 unk_54[4];
    s32 targetX;
    s32 targetY;
    s32 vy[4];
    s32 speed[4];
    u16 angle[4];
    s8 frame;
    u8 timer;
    u8 gatherSteps;
    u8 unk_97;
    TaskPool tasks;
} LevelUpEffectWork;

typedef struct StockInfoWork {
    void* tiles;
    void* palette;
    s32 x;
    s32 y;
    s8 timer;
    u8* active;
    TaskPool tasks;
} StockInfoWork;

typedef struct StockKeys {
    s32 keys[6];
} StockKeys;

extern const StockKeys gTutorialEmptyKeys;
extern const StockKeys gSoraEmptyKeys;

typedef struct GimmickCardArgs {
    s32 x;
    s32 y;
    s32 z;
    s32 cardId;
} GimmickCardArgs;

typedef struct WorldSelBeforeArgs {
    s32 x;
    s32 y;
    s32 z;
} WorldSelBeforeArgs;

typedef struct WorldSelBeforeWork {
    void* tiles;
    ObjPalette* palette;
    void* tiles2;
    ObjPalette* palette2;
    WorldSelBeforeArgs pos;
    u8 animStep;
    u8 animTimer;
    s32 cardX[10];
    s32 cardY[10];
    s32 cardZ[10];
    u8 angle[10];
    u8 spriteCount;
    u8 risenCount;
    u8 unk_A4[0x14];
} WorldSelBeforeWork;

typedef struct EventBgEffectFrame {
    u16 duration;
    u16 tilesOffset;
} EventBgEffectFrame;

#define EVENT_BG_EFFECT_LOOP_NONE (-1)

typedef struct EventBgEffectDef {
    void** maps;
    u8* tiles;
    void* palette;
    u16 tilesSize;
    u16 paletteSize;
    u8 unk_10[0x04];
    const EventBgEffectFrame* frames;
    u8 frameCount;
    s8 loopFrame;
} EventBgEffectDef;

typedef struct EventBgEffectWork {
    const EventBgEffectEntry* entries;
    u8 unk_04[0x08];
    u16 frameTimer;
    u16 frame;
    u8 unk_10[0x02];
    u8 effect;
    u8 eventId;
    u8 entry;
    u8 animating;
    u8 fadingIn;
} EventBgEffectWork;

extern const EventBgEffectDef* gEventBgEffectDefs[];

typedef struct ReloadArgs {
    u8 listIndex;
    u8 side;
    u8* state;
} ReloadArgs;

typedef struct ReloadWork {
    void* tiles;
    void* palette;
    void* gfx;
    AnimState anim;
    s32 x;
    s32 y;
    ReloadArgs args;
    u8 steps;
} ReloadWork;

typedef struct SysMsgWinWork {
    ObjTiles* tiles3;
    ObjPalette* palette;
    ObjTiles* tiles4;
    ObjPalette* palette2;
    ObjTiles* tiles;
    ObjPalette* palette3;
    ObjTiles* tiles2;
    ObjPalette* palette4;
    TextSlot textSlots[10];
    TextSlot textSlots2[10];
    ObjPalette* textPalette;
    AnimState anim;
    AnimState anim2;
    AnimState anim3;
    CardMessageArgs args;
    CardMessageDef* messageDef;
    s32 x;
    s32 cursorY;
    s32 frameX;
    s32 frameY;
    void* gfx4;
    void* gfx;
    TextChar* nextText;
    u16 glyphPaletteIndex;
    s16 closeTimer;
    u8 steps;
    u8 shownChars;
    u8 charTimer;
    u8 charCount;
    u8 unk_13C;
    u8 choice;
    u8 cursorSteps;
    u8 textSlotCount;
    u8 textSlotCount2;
    u8 waitIconVisible;
    u8 unk_142;
    u8 unk_143;
    u8 choiceVisible;
    u8 messagePending;
    u8 keepOpen;
    u8 fallbackFrame;
} SysMsgWinWork;

STATIC_ASSERT(sizeof(SysMsgWinWork) == 0x148, SysMsgWinWorkSize);

typedef struct CardMsgWinWork {
    ObjTiles* tiles3;
    ObjPalette* palette;
    ObjTiles* tiles4;
    ObjPalette* palette2;
    ObjTiles* tiles;
    ObjPalette* palette3;
    ObjTiles* tiles2;
    ObjPalette* palette4;
    TextSlot textSlots[10];
    TextSlot textSlots2[10];
    ObjPalette* textPalette;
    AnimState anim;
    AnimState anim2;
    AnimState anim3;
    CardMessageArgs args;
    CardMessageDef* messageDef;
    s32 x;
    s32 faceX;
    s32 faceY;
    s32 cursorX;
    s32 cursorY;
    void* gfx;
    void* gfx2;
    void* gfx3;
    TextChar* nextText;
    u16 glyphPaletteIndex;
    s16 closeTimer;
    u8 steps;
    u8 shownChars;
    u8 charTimer;
    u8 charCount;
    u8 choice;
    u8 cursorSteps;
    u8 textSlotCounts[0x02];
    u8 waitIconVisible;
    u8 textVisible;
    u8 unk_14A[0x02];
    u8 unk_14C;
    u8 faceFlip;
    u8 messagePending;
    u8 keepOpen;
} CardMsgWinWork;

STATIC_ASSERT(sizeof(CardMsgWinWork) == 0x150, CardMsgWinWorkSize);

typedef struct BossCardWork {
    const CardDef* cardDef;
    const CardBack* cardBack;
    const s32* cardIds;
    u8 unk_0C[0x18];
    s32 enemyKind;
    s16 x;
    u16 y;
    s16 flipScale;
    u8 bobAngle;
    u8 unk_2F;
    u8 unk_30;
    u8 slideSteps;
    u8 flipShrinking;
    u8 flipTimer;
    u8 flipDelay;
} BossCardWork;

extern const s16 gCollectionGridColumnX[];
extern const s16 gCollectionGridRowY[];

s32 AppendKeyboardChar(DeckMenuWork* work);

void func_0807E230();
void RequestCycleRikuCardList();

Deck* GetActiveDeck();
Deck* GetDeck(u8 index);
u8 GetCollectionCardCategory(u16 index);
u16 GetCollectionCardKind(u16 index);
void SetActiveDeckIndex(u8 index);
u16 GetDeckCpCost(u8 index);

void CreateBosscardTask(TaskPool* pool);
void ListOwnedMapCardKinds(MapSelectKindEntry* out);
u8 CanUseSoraSelectedCard();
u8 CanUseRikuSelectedCard();
u16 CountAvailableCardSlots(CardBattleWork* work, u8 listIndex);
u8 UpdatePremireChanceResult(struct PremireChanceWork* work, void* task);
u8 SoraCardUpdateLoaded(CardDisplayWork* work, void* task);
u8 UpdateDeckMenuOpenCommands(DeckMenuWork* work, void* task);
void LoadActiveDeckCardSlots(CardSlot* slots, s32 cardSet);
u8 SoraCardClosed(CardDisplayWork* work, void* task);
u8 SoraReloadCardClosed(CardDisplayWork* work, void* task);
u8 UpdateMapcardToCenter(MapcardWork* work, void* task);
u8 SpotLight_1(SpotlightWork* work, void* task);
u8 UpdateRevCountListChanged(RevCountWork* work);
void REV_COUNT_0(RevCountWork* work, RevCountArgs* args);
u8 REV_COUNT_1(RevCountWork* work, void* task);
void REV_COUNT_2(RevCountWork* work);
void REV_COUNT_3(RevCountWork* work);
u8 UpdateRevCountEmpty(RevCountWork* work, void* task);
void StartPickupCardFlight(PickupCardWork* work, u8 target);
void Friend_card_0(PickupCardWork* work, s32* args);
void Heartless_card_0(PickupCardWork* work, s32* args);
s32 Friend_card_1(PickupCardWork* work, void* task);
s32 Gimmick_card_1(PickupCardWork* work, void* task);
u8 FlyPickupCardToDeck(PickupCardWork* work);
s32 WaitHeartlessCardName(PickupCardWork* work, void* task);
s32 FlyHeartlessCardToCenter(PickupCardWork* work, void* task);
s32 Heartless_card_1(PickupCardWork* work, void* task);
void PickupCardDraw(PickupCardWork* work);
void Heartless_card_2(PickupCardWork* work);
void PickupCardDestroy(PickupCardWork* work);
void Heartless_card_3(PickupCardWork* work);
void DeckCard2LoadGfx(DeckCard2Work* work);
u8 Card_EFFECT_1(CardEffectWork* work);
void InitPrintLayer(u8 bg);
u8 FlipBossCard(BossCardWork* work, u8 requested);
void AimPrizeMapCardAtCenter(PrizeMapCardWork* work);
void AimBossPrizeAtCenter(BossPrizeWork* work);
u8 StockInfo_1(StockInfoWork* work, void* task);
void EnemyCardSlideBack(CardDisplayWork* work, void* task);
u8 UpdateMapSelectValueTutorial(MapSelectWork* work, void* task);
void LoadRikuDeckNameTexts(RikuDeckMenuWork* work);
void LoadDeckExchangeDeckNameTexts(DeckExchangeWork* work);
void CopyLinkPartnerDeckCards(u8 listIndex, u16* out);
void SetDeckMenuFrameCursor(DeckMenuWork* work, u8 frameCursor);
void UpdateFieldPrizeCardScale(PrizeCardWork* work);
void UpdatePrizeMapCardScale(PrizeMapCardWork* work);
void UpdateBossPrizeScale(BossPrizeWork* work);
void PrizeBoss_0(BossPrizeWork* work, PrizeCardTaskArgs* args);
u8 PrizeBoss_1(BossPrizeWork* work, void* task);
void PrizeBoss_2(BossPrizeWork* work);
void PrizeBoss_3(BossPrizeWork* work);
u8 PremireChanceCard_1(PremireChanceCardWork* work, void* task);
void SetDeckExchangeFrameCursor(DeckExchangeWork* work, u8 frameCursor);
u8 UpdateDeckMenuCloseKeyboard(DeckMenuWork* work, void* task);
void OpenRikuCards(CardBattleWork* work);
void InitDecks();
void EnemyCardSlideOut(CardDisplayWork* work, void* task);
u8 UpdateMapcardMoveBack(MapcardWork* work, void* task);
void SpotLight_0(SpotlightWork* work, u8* src);
u8 UpdateRevCountHidden(RevCountWork* work, void* task);
u8 FlyHeartlessCardToPlayer(PickupCardWork* work);
u8 UpdatePremireChanceClose(struct PremireChanceWork* work, void* task);
void PrintBinary16(u16 x, u16 y, u16 color, u16 bits);
void LevelUpSplitDigits4(u16 value, u16* out);
u8 UpdateRikuDeckMenuEnterDeckGrid(RikuDeckMenuWork* work, void* task);
u8 UpdatePrizeMapCardShrink(PrizeMapCardWork* work);
u8 UpdatePremireChanceCardMoveAway(PremireChanceCardWork* work, void* task);
u8 UpdateRikuDeckMenuSlideOut(RikuDeckMenuWork* work, void* task);
void RecalculateInactiveDeckCpCosts();
u8 RELOAD_1(ReloadWork* work, void* task);
u8 UpdateBossPrizeShrink(BossPrizeWork* work);
void LoadEventMapObjectGfx(EventMapObjectWork* work, EventBackgroundDef* background);
void Mode_riku_deckTutorial_1();
void OpenSoraCards(CardBattleWork* work);
u8 UpdateSoraAutoCycle(CardBattleWork* work, void* task);
void CloseRikuCards(CardBattleWork* work);
u8 RikuCardDeal(CardDisplayWork* work, void* task);
void card_enemy_0(CardDisplayWork* work, CardDisplayArgs* args);
void card_not_have_2(CardDisplayWork* work);
u8 Premire_Chance_1(struct PremireChanceWork* work, void* task);
void PrintHex32(u16 x, u16 y, u16 color, u32 value);
void ScrollRikuGridDown(RikuDeckMenuWork* work);
void ScrollDeckExchangeGridDown(DeckExchangeWork* work);
void SelectOtherSoraCard(CardBattleWork* work, u8 listIndex, u8 timer);
u8 SoraCardDeal(CardDisplayWork* work, void* task);
u8 card_enemy_1(CardDisplayWork* work, void* task);
void SetRikuDeckMenuHandAnim(RikuDeckMenuWork* work);
void DrawRikuDeckCategoryCount(u8 count, u8 category);
void SetDeckExchangeHandAnim(DeckExchangeWork* work);
u8 SoraHeartlessCardShow(CardDisplayWork* work);
u8 RikuHeartlessCardShow(CardDisplayWork* work);
void SetDeckMenuHandAnim(DeckMenuWork* work);
void Mode_Premire_0();
void LoadEventBgEffect(EventBgEffectWork* work);
u8 StepEventBgEffectAnim(EventBgEffectWork* work);
u8 UpdateRikuDeckMenuLoadBgs(RikuDeckMenuWork* work, void* task);
void UpdateRikuCardValue(CardDisplayWork* work);
u8 map_anim_1(MapTileAnimationWork* work);
u8 Mapcard_1(MapcardWork* work, void* task);
void RELOAD_0(ReloadWork* work, ReloadArgs* args);
void Premire_Chance_3(struct PremireChanceWork* work);
void NO_Card_2(CardDisplayWork* work);
void ClearDeck(u8 deck);
void CloseSoraCards(CardBattleWork* work);
u8 AddBreakDarkPoints();
u8 SoraCardFlyOff(CardDisplayWork* work);
u8 RikuCardFlyOff(CardDisplayWork* work);
void RemoveCardFromDeck(u16* slot, u8 deck);
void DrawCollectionFilterTab(u8 categoryFilter, u8 mode);
u8 EnemyCardFlyOff(CardDisplayWork* work);
u8 ScrollRikuGridUp(RikuDeckMenuWork* work);
u8 ScrollDeckExchangeGridUp(DeckExchangeWork* work);
void DrawDeckExchangeCollectionFilterTab(u8 categoryFilter, u8 mode);
void Ev_mapObj_2(EventMapObjectWork* work);
void UpdateEventKeyTotal(EventKeyCard* card);
u8 card_reload_1(CardDisplayWork* work, void* task);
void ResetGridScroll(DeckMenuWork* work);
u8 UpdateMapcardMoveToFront(MapcardWork* work, void* task);
u8 UpdateRikuDeckMenuStartSlideOut(RikuDeckMenuWork* work, void* task);
s16 CountDeckCards(s32 cardSet, const Deck* deck);
u8 UpdateCardMsgwinClose(CardMsgWinWork* work);
u8 UpdateReloadChildRetracted(ReloadChildWork* work, void* task);
void DeckCard2_2(DeckCard2Work* work);
void ApplyRikuHcEffect(CardBattleWork* work);
void UpdateMapcardGfx(MapcardWork* work);
void DrawDeckCardCount(u8 deck);
void DrawDeckExchangeDeckCardCount(u8 deck);
void DrawRikuDeckCardCount(u8 deck);
u8 StockNameSora_1(StockNameWork* work);
u8 StockNameRiku_1(StockNameWork* work);
u8 DeckCard2_1(DeckCard2Work* work);
void ResetCardSlotsForReload(CardBattleWork* work, u8 listIndex);
u8 RikuCardWaitPlayEnd(CardDisplayWork* work, void* task);
void Bosscard_0(BossCardWork* work, u32* arg);
void DrawLayeredCardSpriteScaled(LayeredCardSprite* sprite, u16 flags, s16 dy, s16 scale);
void RemoveCardFromActiveDeck(u16 slot);
u8 SoraCardWaitPlayEnd(CardDisplayWork* work, void* task);
u8 FindCardInDirection(DeckMenuWork* work, s16 x, s16 y, u16 dir);
void Mapcard_0(MapcardWork* work, MapcardArgs* args);
u8 AddCardToDeck(u16 card, u8 deck);
void CountCardsNotInDeckByKind(CardKindEntry* out, u8 deck, u8 thisDeckOnly, u16 entryCount, void* p);
void FillDebugCardCollection();
u16 CountOwnedMapCardKinds();
void LVUP_EFFECT_0(LevelUpEffectWork* work, LevelUpEffectArgs* arg);
void Deck_Clear_0(DeckConfirmWork* work, u8* active);
void Deck_Yes_No_0(DeckConfirmWork* work, u8* active);
void LVUP_EFFECT_2(LevelUpEffectWork* work);
u8 UpdatePremireChanceCardToCenter(PremireChanceCardWork* work, void* task);
void Lvup_Logo_0(LevelUpEffectWork* work, LevelUpEffectArgs* args);
u8 UpdateCardMsgwinChoice(CardMsgWinWork* work, void* task);
u8 UpdateRikuDeckMenuSlideIn(RikuDeckMenuWork* work, void* task);
void sysmsgwin_3(SysMsgWinWork* work);
void sysmsgwinChoice_3(SysMsgWinWork* work);
s32 ReplaceSysmsgwinMessage(CardMessageArgs* src);
s32 CloseSysmsgwin();
void UpdateEnemyCardRingPosition(CardDisplayWork* work);
void SpawnBossPrizeCardEffects(BossPrizeWork* work);
void Premire_Chance_2(struct PremireChanceWork* work);
void PremireEffectInit(PremiumCardEffectWork* work, s16* arg);
void PremireEffectConvergeInit(PremiumCardEffectWork* work, s16* arg);
void RELOAD_CHILDREN_2(ReloadChildWork* work);
u8 FindDeckExchangeCardInDirection(DeckExchangeWork* work, s16 x, s16 y, u16 dir);
u8 EnemyCardWaitPlayEnd(CardDisplayWork* work, void* task);
u8 AddCardToActiveDeck(u16 card);
void SetRikuCardKindObtained(u16 kind);
void CopyActiveDeckCards(s32 cardSet, u16* out);
void DeckConfirmDraw(DeckConfirmWork* work);
void DeckConfirmDestroy(DeckConfirmWork* work);
u8 ScrollGridUp(DeckMenuWork* work, u8 playSound);
void DeckMenuDestroy(DeckMenuWork* work);
void HCEffectName_2(HcEffectNameWork* work);
u8 UpdateCardMsgwinOpen(CardMsgWinWork* work, void* task);
void DispatchEnemyCardCommand(CardDisplayWork* work, void* task);
void RELOAD_CHILDREN_0(ReloadChildWork* work, ReloadChildArgs* args);
u8 Bosscard_1(BossCardWork* work, void* task);
void RemoveMapSelectCard(MapSelectWork* work);
u8 UpdateBossPrizeShow(BossPrizeWork* work, void* task);
void LoadRikuReloadCardGfx(CardDisplayWork* work);
void InitRikuReloadCounterAnim(ReloadGauge* gauge, void* tiles, u8 listIndex, s8 count);
u8 RikuCardClosed(CardDisplayWork* work, void* task);
void RemoveCursorCardFromDeck(DeckMenuWork* work);
u8 UpdateLevelUpEffectScatter(LevelUpEffectWork* work);
u8 UpdatePrizeMapCardShow(PrizeMapCardWork* work, void* task);
void StockNameSora_0(StockNameWork* work, const s32* src);
void StockNameRiku_0(StockNameWork* work, const s32* src);
void DrawDeckFilterTab(u8 categoryFilter, u8 mode);
u8 SoraCardUpdate(CardDisplayWork* work, void* task);
u8 SoraCardBreakFall(CardDisplayWork* work, void* task);
u8 RikuCardBreakFall(CardDisplayWork* work, void* task);
u8 EnemyCardBreakFall(CardDisplayWork* work, void* task);
void card_reload_0(CardDisplayWork* work, CardDisplayArgs* args);
void EnemyUsecardByIndexInit(CardDisplayWork* work, CardDisplayArgs* args);
void LoadSoraReloadCardGfx(CardDisplayWork* work);
void Reload_Card_0(CardDisplayWork* work, CardDisplayArgs* args);
void EnemyUsecardRandomInit(CardDisplayWork* work, CardDisplayArgs* args);
u8 UpdatePremireChanceCardSpin(PremireChanceCardWork* work, void* task);
u8 EV_BG_EFFECT_1(EventBgEffectWork* work, void* task);
void PremireChanceCard_0(PremireChanceCardWork* work, CardSlot* slot);
void Card_EFFECT_0(CardEffectWork* work, CardEffectArgs* args);
void deckexchange_3(DeckExchangeWork* work);
void TickSoraHcEffectOnReload();
void TickRikuHcEffectOnReload();
u8 GetHcEffectCountUnit(HcEffectNameWork* work, u16 effect);
void StockInfo_0(StockInfoWork* work, u8* active);
u8 UpdateCardMsgwinLoadFace(CardMsgWinWork* work, void* task);
u8 DispatchRikuCardCommand(CardDisplayWork* work, void* task);
u8 Reload_Card_1(CardDisplayWork* work, void* task);
u8 UpdateDeckMenuSlideIn(DeckMenuWork* work, void* task);
u8 SoraCardMoveToPlay(CardDisplayWork* work, void* task);
u8 UpdateSysmsgwinChoiceSetup(SysMsgWinWork* work, void* task);
u8 CheckCardDeletable(DeckMenuWork* work);
u8 UpdateCardMsgwinTyping(CardMsgWinWork* work, void* task);
u8 RikuCardMoveToPlay(CardDisplayWork* work, void* task);
u8 RikuCardUpdate(CardDisplayWork* work, void* task);
u8 SoraGimmickCardFly(CardDisplayWork* work, void* task);
u8 UpdateDeckMenuOpenDeleteMode(DeckMenuWork* work, void* task);
u8 UpdateSysmsgwinChoiceInput(SysMsgWinWork* work, void* task);
void sysmsgwinChoice_2(SysMsgWinWork* work);
void DrawRikuCardTotals();
void WorldSel_Before_0(WorldSelBeforeWork* work, WorldSelBeforeArgs* args);
void SoraCardInit(CardDisplayWork* work, CardDisplayArgs* args);
void RikuCardInit(CardDisplayWork* work, CardDisplayArgs* args);
u8 RELOAD_CHILDREN_1(ReloadChildWork* work, void* task);
u8 SoraStockMoveToPlay(CardDisplayWork* work, void* task);
u8 UpdatePrizeMapCardFlight(PrizeMapCardWork* work, void* task);
u8 RikuStockMoveToPlay(CardDisplayWork* work, void* task);
u8 UpdateMapSelectTutorial(MapSelectWork* work, void* task);
u8 UpdateBossPrizeFlight(BossPrizeWork* work, void* task);
void HCEffectName_0(HcEffectNameWork* work, u8* arg);
u8 RikuStockWaitPlayEnd(CardDisplayWork* work, void* task);
u8 UpdatePremireChanceStop(struct PremireChanceWork* work, void* task);
u8 EnemyStockMoveToSlot(CardDisplayWork* work, void* task);

void CardName_0(CardNameWork* work);
void DarkPoint_0(DarkPointWork* work);
s32 DarkPoint_1(DarkPointWork* work);
void DarkPoint_2(DarkPointWork* work);
void DarkPoint_3(DarkPointWork* work);

#ifndef VERSION_EU
void DrawDeckExchangeDeckNames(DeckExchangeWork* work, u8 selectedOnly);
#ifndef VERSION_EU
u8 UpdateDeckExchangeLoadDeckInfo(DeckExchangeWork* work, void* task);
void HighlightDeckExchangeDeckTab(DeckExchangeWork* work, u8 deckIndex);
#endif
#endif

typedef struct SelmapEventKeyWork {
    void* tiles;
    ObjPalette* palette;
    EventKeyCard cards[4];
    SelmapEventKeyArgs* args;
    AnimState anim;
    void* gfx;
    void* unk_F8;
    s32 rowX;
    s32 rowY;
    s32 rowTargetX;
    s32 unk_108;
    s32 unk_10C;
    s32 unk_110;
    u16 unk_114;
    u16 unk_116;
    u16 pulseAngle;
    u8 slideSteps;
    u8 unk_11B;
    u8 unk_11C;
    u8 frame;
    u8 mosaicX;
    u8 mosaicY;
    u8 mosaicTimer;
    u8 keyCount;
    u8 paidCount;
    u8 unk_123;
} SelmapEventKeyWork;

STATIC_ASSERT(sizeof(SelmapEventKeyWork) == 0x124, SelmapEventKeyWorkSize);

typedef struct KeyboardLineLayout {
    const s16* positions;
    s16 count;
} KeyboardLineLayout;

extern const s16 gKeyboardKeyX[];
extern const s16 gKeyboardKeyY[];
extern const KeyboardLineLayout gKeyboardRowLayouts[];
extern const KeyboardLineLayout gKeyboardColumnLayouts[];

#ifdef VERSION_EU
extern const KeyboardLineLayout gKeyboardSymbolRowLayouts[];
extern const KeyboardLineLayout gKeyboardSymbolColumnLayouts[];
extern const s16 gKeyboardPageTabXEu[];
#endif

#ifdef VERSION_JP
extern const s16 gKeyboardPageTabXJp[];
#endif

typedef struct PremireChanceWork {
    void* tiles;
    ObjPalette* palette;
    void* tiles2;
    ObjPalette* palette2;
    void* tiles3;
    ObjPalette* palette3;
    void* tiles4;
    ObjPalette* palette4;
    void* tiles5;
    CardSlot* slots;
    void* gfx;
    void* gfx2;
    s16 titleX;
    s32 topY;
    s32 bottomY;
    TaskPool tasks;
    u8 cardCount;
    u8 spinDelay;
    u8 advanced;
    AnimState anim;
    AnimState anim2;
    u8 stopped;
    u8 inputEnabled;
    u8 resultPending;
    u8 cursorHidden;
    u8 bgAnimDuration;
    u8 resultTimer;
    u8 stopTimer;
    u8 titleSteps;
    u8 slideSteps;
} PremireChanceWork;

typedef struct LevelUpWork {
    void* tilesPalettes[8];
#ifdef VERSION_EU
    void* tiles5[3];
    u8 unk_02C[0xC];
#else
    TextSlot textSlots[6][36];
#endif
    ObjPalette* palette;
    ObjPalette* palette2;
    void* tiles;
    ObjPalette* palette3;
    void* tiles2;
    void* tiles3;
    ObjPalette* palette4;
    TaskPool pool;
    AnimState anim;
    void* tiles4;
    ObjPalette* palette5;
    void* gfx;
    AnimState anim2;
    void* gfx2;
    s16 bonusX[3];
    s16 bonusY[3];
    s16 textX[3];
    s16 textY[3];
    s16 topBarX;
    s16 bottomBarX;
    s32 topBarY;
    s32 bottomBarY;
    s16 cursorX;
    s16 cursorY;
    s16 headerX;
    s16 bgScrollX;
    s16 statsOffsetX;
    u16 levelDigits[3];
    u16 maxHpDigits[4];
    u16 cpDigits[4];
    u16 dpDigits[4];
    u16 apDigits[4];
    s16 timer;
    s32 playerX;
    s32 playerY;
    s8 cursor;
    s8 headerSteps;
    s8 optionSteps[3];
    s8 slideSteps;
    s8 cursorSteps;
    u8 textSlotCounts[6];
    u8 state;
    u8 blinkTimer;
    u8 barSteps;
    u8 playerSteps;
    u8 blinkOn;
    u8 loaded[2];
    u8 messageActive;
    u8 effectShown;
    u8 bossBattle;
    u8 applied;
    u8 optionEnabled[3];
} LevelUpWork;

enum StatIncreaseFlag {
    STAT_INCREASE_FLAG_DP = 0x4000,
    STAT_INCREASE_FLAG_MAX_HP = 0x8000
};

typedef struct StatIncreaseDisplayArgs {
    u8* done;
    u32 flags : 16;
    u32 amount : 16;
} StatIncreaseDisplayArgs;

extern s8 gLinkDecksAllocated;

extern const MapTileAnimationDef* gMapTileAnimationDefs[6];
extern const MapTileAnimationDef* gUnk_09EE4A44;
extern const MapTileAnimationDef* gUnk_09EE4A48;
extern const MapTileAnimationDef* gUnk_09EE4A4C;
extern const u16* gRikuDeckCards[12];
extern const u16* gRikuDeckEnemyCards[12];
#ifdef VERSION_EU
extern void* gDeckButtonLabelTilesByLanguage[5];
extern void** gDeckButtonLabelSpritesByLanguage[5];
extern void* gDeckCommandMenuTilesByLanguage[5];
extern void* gDeckTitleBannerTilesByLanguage[5];
extern u8* gDeckEquipMarkerTilesByLanguage[5];
extern void* gDeckKeyboardCursorTilesByLanguage[5];
extern void** gDeckKeyboardCursorSpritesByLanguage[5];
extern void* gDeckKeyboardCursorAnimsByLanguage[5];
extern const u8* gDeckKeyboardLetterRows[8];
extern const u8* gDeckKeyboardSymbolRows[7];
extern void* gMapCardUiExtraTilesByLanguage[5];
extern void** gMapCardUiSpritesByLanguage[5];
extern void** gMapSelectTitleSpritesByLanguage[5];
#else
extern const u8* gDeckKeyboardRows[7];
#endif
#ifdef VERSION_JP
extern const u8* gDeckKeyboardKatakanaRows[7];
extern const u8* gDeckKeyboardAlphanumericRows[7];
#endif
extern const void* gMapSelectBgMapBlocks[2];
extern s16 gMapSelectValueColumnX[5];
extern s16 gMapSelectValueRowY[2];
extern u16 gMapSelectCountTileIndices[10];

extern Mode gModePremire;
#ifdef VERSION_EU
extern void* gHcEffectCountUnitTilesByLanguage[5];
extern void** gHcEffectCountUnitSpritesByLanguage[5];
extern void* gLevelUpBgTilesByLanguage[5];
extern void* gLevelUpHeaderTilesByLanguage[5];
extern void** gLvupEffectSpritesByLanguage[5];
#else
#ifdef VERSION_JP
extern const u8 gLevelUpDisabledText[];
#else
extern const u16 gLevelUpDisabledText[];
#endif
extern u16 gLevelUpHpBoostText[];
extern u16 gLevelUpCpBoostText[];
extern u16 gLevelUpSleightsText[];
extern u16 gLevelUpRaiseSoraHpText[];
extern u16 gLevelUpRaiseSoraCpText[];
extern u16 gLevelUpLearnSleightText[];
extern u16 gLevelUpAttackBoostText[];
extern u16 gLevelUpDarknessBoostText[];
extern u16 gLevelUpRaiseRikuHpText[];
extern u16 gLevelUpRaiseRikuApText[];
extern u16 gLevelUpRaiseRikuDpText[];
extern u16* gLevelUpSoraTexts[7];
extern u16* gLevelUpRikuTexts[7];
#endif
extern const void* gLevelUpBgMapBlocks[2];
extern void* gLevelUpOptionBgMaps[3];
extern void* gEventBgEffectMaps[7];

extern const CardHelpText* gBlitzHelpTexts[];
extern const CardHelpText* gSonicBladeHelpTexts[];
extern const CardHelpText* gArsArcanumHelpTexts[];
extern const CardHelpText* gStrikeRaidHelpTexts[];
extern const CardHelpText* gRagnarokHelpTexts[];
extern const CardHelpText* gTrinityLimitHelpTexts[];
extern const CardHelpText* gSlidingDashHelpTexts[];
extern const CardHelpText* gStunImpactHelpTexts[];
extern const CardHelpText* gZantetsukenHelpTexts[];
extern const CardHelpText* gFiraHelpTexts[];
extern const CardHelpText* gBlizzaraHelpTexts[];
extern const CardHelpText* gThundaraHelpTexts[];
extern const CardHelpText* gCuraHelpTexts[];
extern const CardHelpText* gGraviraHelpTexts[];
extern const CardHelpText* gStopraHelpTexts[];
extern const CardHelpText* gAeroraHelpTexts[];
extern const CardHelpText* gFiragaHelpTexts[];
extern const CardHelpText* gBlizzagaHelpTexts[];
extern const CardHelpText* gThundagaHelpTexts[];
extern const CardHelpText* gCuragaHelpTexts[];
extern const CardHelpText* gGravigaHelpTexts[];
extern const CardHelpText* gStopgaHelpTexts[];
extern const CardHelpText* gAerogaHelpTexts[];
extern const CardHelpText* gProudRoarHelpTexts[];
extern const CardHelpText* gProudRoarPairHelpTexts[];
extern const CardHelpText* gShowtimeHelpTexts[];
extern const CardHelpText* gShowtimePairHelpTexts[];
extern const CardHelpText* gTwinkleHelpTexts[];
extern const CardHelpText* gTwinklePairHelpTexts[];
extern const CardHelpText* gFlareBreathHelpTexts[];
extern const CardHelpText* gFlareBreathPairHelpTexts[];
extern const CardHelpText* gCrossSlashHelpTexts[];
extern const CardHelpText* gOmnislashHelpTexts[];
extern const CardHelpText* gParadiseHelpTexts[];
extern const CardHelpText* gParadisePairHelpTexts[];
extern const CardHelpText* gSplashHelpTexts[];
extern const CardHelpText* gSplashPairHelpTexts[];
extern const CardHelpText* gMagicHelpTexts[];
extern const CardHelpText* gMagicPairHelpTexts[];
extern const CardHelpText* gGoofyChargeHelpTexts[];
extern const CardHelpText* gGoofyTornadoHelpTexts[];
extern const CardHelpText* gSandstormHelpTexts[];
extern const CardHelpText* gSandstormPairHelpTexts[];
extern const CardHelpText* gSurpriseHelpTexts[];
extern const CardHelpText* gSurprisePairHelpTexts[];
extern const CardHelpText* gSpiralWaveHelpTexts[];
extern const CardHelpText* gSpiralWavePairHelpTexts[];
extern const CardHelpText* gHummingbirdHelpTexts[];
extern const CardHelpText* gHummingbirdPairHelpTexts[];
extern const CardHelpText* gFerociousLungeHelpTexts[];
extern const CardHelpText* gFerociousLungePairHelpTexts[];
extern const CardHelpText* gMmMiracleHelpTexts[];
extern const CardHelpText* gSecretHelpTexts[];
extern const CardHelpText* gMmMiraclePairHelpTexts[];
extern const CardHelpText* gWarpHelpTexts[];
extern const CardHelpText* gWarpinatorHelpTexts[];
extern const CardHelpText* gTerrorHelpTexts[];
extern const CardHelpText* gConfuseHelpTexts[];
extern const CardHelpText* gSleight57HelpTexts[];
extern const CardHelpText* gStopRaidHelpTexts[];
extern const CardHelpText* gJudgmentHelpTexts[];
extern const CardHelpText* gReflectRaidHelpTexts[];
extern const CardHelpText* gFireRaidHelpTexts[];
extern const CardHelpText* gBlizzardRaidHelpTexts[];
extern const CardHelpText* gThunderRaidHelpTexts[];
extern const CardHelpText* gGravityRaidHelpTexts[];
extern const CardHelpText* gAquaSplashHelpTexts[];
extern const CardHelpText* gHolyHelpTexts[];
extern const CardHelpText* gBlazingDonaldHelpTexts[];
extern const CardHelpText* gSleight68HelpTexts[];
extern const CardHelpText* gGiftedMiracleHelpTexts[];
extern const CardHelpText* gMegaFlareHelpTexts[];
extern const CardHelpText* gFiragaBreakHelpTexts[];
extern const CardHelpText* gShockImpactHelpTexts[];
extern const CardHelpText* gIdyllRompHelpTexts[];
extern const CardHelpText* gCrossSlashPlusHelpTexts[];
extern const CardHelpText* gHomingFiraHelpTexts[];
extern const CardHelpText* gHomingBlizzaraHelpTexts[];
extern const CardHelpText* gSynchroHelpTexts[];
extern const CardHelpText* gBindHelpTexts[];
extern const CardHelpText* gTornadoHelpTexts[];
extern const CardHelpText* gQuakeHelpTexts[];
extern const CardHelpText* gTeleportHelpTexts[];
extern const CardHelpText* gDarkBreakHelpTexts[];
extern const CardHelpText* gDarkFiragaHelpTexts[];
extern const CardHelpText* gDarkAuraHelpTexts[];
extern const CardHelpDef* gCardHelpDefs[];
#ifdef VERSION_EU
extern u8 gLeaveWorldText[];
extern u8* gLeaveWorldTextByLanguage[5];
extern void* gRikuDeckTitleBannerTilesByLanguage[5];
extern u8* gRikuDeckEquipMarkerTilesByLanguage[5];
extern Mode gModeTextCheck;
#else
extern Mode gModeDeckExchange;
#endif

extern u8 gBossCardRequestValue;
extern u8 gBossCardRequest;
extern Deck gDecks[3];
extern u16 gCardCollection[999];
extern Deck* gLinkPartnerDeck;
extern u16 gCardCount;
extern CardUiSpriteState gCardUiSpriteState;
extern MapCardUiResources gMapCardUiResources;
extern u8 gMapCardCounts[270];
extern struct CardListWork* gCardListWork;
extern u8 gMessageWindowOpen;
extern u8 gMessageWindowAnswerYes;
#ifndef VERSION_EU
extern u16 gSioTradeCardId;
#endif
extern u8 gRikuDeckTutorialState;
extern TaskDesc gTaskDescCardSora;
extern TaskDesc gTaskDescCardNotHave;
extern TaskDesc gTaskDescCardReload;
extern TaskDesc gTaskDescCardRiku;
extern TaskDesc gTaskDescNOCard;
extern TaskDesc gTaskDescReloadCard;
extern TaskDesc gTaskDescBosscard;
extern const MapTileAnimationDef* gMapTileAnimationDefs[6];
extern const u16* gRikuDeckCards[12];
extern const u16* gRikuDeckEnemyCards[12];
#ifdef VERSION_EU
extern void* gDeckButtonLabelTilesByLanguage[5];
extern void** gDeckButtonLabelSpritesByLanguage[5];
extern void* gDeckCommandMenuTilesByLanguage[5];
extern void** gDeckCommandMenuSpritesByLanguage[5];
extern void* gDeckTitleBannerTilesByLanguage[5];
extern void** gDeckTitleBannerSpritesByLanguage[5];
extern u8* gDeckEquipMarkerTilesByLanguage[5];
extern void* gDeckKeyboardCursorTilesByLanguage[5];
extern void** gDeckKeyboardCursorSpritesByLanguage[5];
extern void* gDeckKeyboardCursorAnimsByLanguage[5];
#endif
#ifdef VERSION_US
extern const u8* gDeckKeyboardRows[7];
#endif
#ifdef VERSION_JP
extern const u8* gDeckKeyboardKatakanaRows[7];
extern const u8* gDeckKeyboardAlphanumericRows[7];
#endif
#ifdef VERSION_EU
extern const u8* gDeckKeyboardLetterRows[8];
extern const u8* gDeckKeyboardSymbolRows[7];
#endif
extern TaskDesc gTaskDescDeckCard2;
extern TaskDesc gTaskDescEnemyUsecard;
extern TaskDesc gTaskDescEnemyUsecardByIndex;
extern TaskDesc gTaskDescEnemyUsecardRandom;
#ifdef VERSION_EU
extern void* gMapCardUiExtraTilesByLanguage[5];
extern void** gMapCardUiSpritesByLanguage[5];
#endif
extern const void* gMapSelectBgMapBlocks[2];
extern s16 gMapSelectValueColumnX[5];
extern s16 gMapSelectValueRowY[2];
#ifdef VERSION_EU
extern void** gMapSelectTitleSpritesByLanguage[5];
#endif
extern TaskDesc gTaskDescMapSelect;
extern u16 gMapSelectCountTileIndices[10];
extern MapCardBackDef gMapCardBackDefs[5];
extern MapCardDef gMapCardDefs[260];
extern s16 gMapcardSlotX[6];
extern PrizeMapCardBackAnimStep gPrizeMapCardBackAnim[7];
extern TaskDesc gTaskDescMapcard;
extern TaskDesc gTaskDescReloadGage;
extern void* gReloadCounterTiles[4];
extern AnimHeader** gReloadCounterAnims[4];
extern void** gReloadCounterFrames[4];
extern void* gReloadCardTiles[4];
extern void** gReloadGaugeFrames[4];
extern AnimHeader** gReloadGaugeAnims[4];
extern TaskDesc gTaskDescFieldPrizeCard;
extern TaskDesc gTaskDescPrizeCardInit;
extern TaskDesc gTaskDescPrizeCardInitBoss;
extern TaskDesc gTaskDescDispCardname;
extern TaskDesc gTaskDescVersion;
extern TaskDesc gTaskDescPrizeMapCard;
#ifdef VERSION_EU
extern void* gSelmapEventKeyTitleAnimsByLanguage[5];
extern void* gSelmapEventKeyTitleFramesByLanguage[5];
extern void* gSelmapEventKeyTitleTilesByLanguage[5];
#endif
extern TaskDesc gTaskDescSELMAPEVKEY;
extern void* gReloadChildTiles[4];
extern TaskDesc gTaskDescReloadChildren;
extern void* gRevCountTileSources[4];
extern void** gRevCountSprites[4];
extern TaskDesc gTaskDescREVCOUNT;
extern void* gReloadTiles[3];
extern AnimHeader** gReloadAnims[3];
extern void** gReloadFrames[3];
extern TaskDesc gTaskDescRELOAD;
extern TaskDesc gTaskDescPrizeBoss;
extern TaskDesc gTaskDescCardEFFECT;
extern TaskDesc gTaskDescScrollbar;
extern TaskDesc gTaskDescFriendCard;
extern TaskDesc gTaskDescHeartlessCard;
extern TaskDesc gTaskDescGimmickCard;
extern TaskDesc gTaskDescStockNameRiku;
#ifdef VERSION_EU
extern void** gPremireChanceTitles[5];
#endif
extern TaskDesc gTaskDescPremireChance;
extern TaskDesc gTaskDescPremireChanceCard;
extern TaskDesc gTaskDescCardName;
#ifdef VERSION_EU
extern void* gHcEffectCountUnitTilesByLanguage[5];
extern void** gHcEffectCountUnitSpritesByLanguage[5];
#endif
extern TaskDesc gTaskDescHCEffectName;
extern TaskDesc gTaskDescNumberPlus;
#ifdef VERSION_EU
extern void* gLevelUpBgTilesByLanguage[5];
extern void* gLevelUpHeaderTilesByLanguage[5];
extern void** gLevelUpHeaderSpritesByLanguage[5];
extern void* gLevelUpOptionTilesByLanguage[5];
extern void** gLevelUpOptionSpritesByLanguage[5];
#endif
#ifndef VERSION_EU
extern u16* gLevelUpSoraTexts[7];
extern u16* gLevelUpRikuTexts[7];
#endif
extern const void* gLevelUpBgMapBlocks[2];
extern void* gLevelUpOptionBgMaps[3];
extern TaskDesc gTaskDescLevelUp;
#ifdef VERSION_EU
extern void* gLvupEffectSprites[6];
extern void** gLvupEffectSpritesByLanguage[5];
#endif
extern TaskDesc gTaskDescLVUPEFFECT;
extern TaskDesc gTaskDescLvupLogo;
extern const EventBgEffectDef* gEventBgEffectDefs[8];
extern TaskDesc gTaskDescEVBGEFFECT;
extern const CardHelpDef* gCardHelpDefs[];
extern TaskDesc gTaskDescStockInfo;
extern TaskDesc gTaskDescLvupMsg;
extern TaskDesc gTaskDescDeckEquip;
extern TaskDesc gTaskDescDeckYesNo;
extern TaskDesc gTaskDescDeckClear;
extern TaskDesc gTaskDescDeckErrorCp;
extern TaskDesc gTaskDescDeckErrorNoAttackCard;
extern TaskDesc gTaskDescDeckErrorLastAttackCard;
extern TaskDesc gTaskDescDeckErrorDeckFull;
extern CardMessageDef gCardMessageDefs[];
extern TaskDesc gTaskDescCardMsgwin;
extern TaskDesc gTaskDescSysmsgwin;
extern TaskDesc gTaskDescSysmsgwinChoice;
extern WorldSelAnim gWorldSelAnims[30];
extern TaskDesc gTaskDescWorldSelBefore;
#ifdef VERSION_EU
extern void* gRikuDeckTitleBannerTilesByLanguage[5];
extern void** gRikuDeckTitleBannerSpritesByLanguage[5];
extern u8* gRikuDeckEquipMarkerTilesByLanguage[5];
#endif
#ifndef VERSION_EU
extern TaskDesc gTaskDescDeckexchange;
#endif
extern CardDescriptionText* gCardKindDescriptions[98];

void AddPickedCardToSoraDeck(CardBattleWork* work);
u8 AreCardsSettled(CardDisplayWork** stock, u8 stockCount);
void BeginSoraReloadDeal(CardBattleWork* work);
void BuildDebugKingdomKeyDeck(u8 deck);
void ClearStockedCardSlots(CardBattleWork* work);
void ClearUsedCardSlots(CardBattleWork* work, u8 listIndex);
u8 CollectionHasCard(u16 id);
void ConvertActiveDeckCardToPremium(u16 index);
u16 CountActiveDeckCardsOfCategory(u8 category);
u16 CountAvailableCards(CardBattleWork* work, u8 listIndex);
void CountCardsNotInDeckByCategory(u8 deck, u16* out);
u16 CountMapCardsOfKind(u16 baseCardId);
u16 CountRemainingAttackCards(CardBattleWork* work, u8 listIndex);
u16 CountZeroValueMapCards();
void CreateCardNameDisplay(void* pool, const void* name);
void CycleSoraCardList(CardBattleWork* work);
void DeckCard2ReleaseGfx(DeckCard2Work* work);
void DrawCollectionCategoryCount(u16 count, u8 category);
void DrawDeckCategoryCount(u8 count, u8 category);
void DrawValueCount(u8 count, u16 value);
void FillStarterDeck();
void FreePrintLayer();
u16 GetNextRandomHcEffect(u16* index);
u16 GetRandomHcEffect();
u8 HasMapCard(u16 cardId);
void IncrementReloadCount(CardBattleWork* work);
void InitSoraCardList(CardBattleWork* work, s32 cardSet);
void InitSoraTutorialCardList(CardBattleWork* work, s32 cardSet);
u8 IsCardDisplayOffScreen(CardDisplayWork* work);
u8 IsLevelUpStockUnlocked();
u16 ListCardsNotInDeckByKind(CardKindEntry* out, u8 deck, u8 thisDeckOnly, u16 entryCount, void* p);
void LoadCardDisplayGfx(CardDisplayWork* work);
void ObtainStarterCards();
u16 PickPrizeMapCardForWorld(u16 world, s32 b);
void ReleaseCardDisplayGfx(CardDisplayWork* work);
void RemoveItemCards(CardBattleWork* work);
void RemoveSoraCardDisplays(CardBattleWork* work);
void ResetBossCardValue();
void ResetPrintLines();
void ResetSoraReloadGauge(CardBattleWork* work);
void RestoreCardsForElixir(CardBattleWork* work);
void RestoreCardsForEther(CardBattleWork* work);
void RestoreCardsForHiPotion(CardBattleWork* work);
void RestoreCardsForMegaEther(CardBattleWork* work);
void RestoreCardsForMegaPotion(CardBattleWork* work);
void RestoreCardsForPotion(CardBattleWork* work);
void SelectNextSoraCard(CardBattleWork* work, u8 listIndex);
void ShuffleCardSlots(CardSlot* slots, u8 count);
s32 StockSoraCard(CardBattleWork* work);
void SwitchSoraCardList(CardBattleWork* work);
void SyncCardDisplayGfx(CardDisplayWork* work);
void TrackLevelUpEffectTarget(LevelUpEffectWork* work);
void UpdateCardDisplayFlip(CardDisplayWork* work);
s32 UseSoraCard(CardBattleWork* work);
s32 UseSoraGimmickCard(CardBattleWork* work);
s32 UseSoraHeartlessCard(CardBattleWork* work);
void UseSoraStock(CardBattleWork* work);
void ClearSoraCardPlayFlags();
u8 SoraStockStartUnopposedPlay(CardDisplayWork* work, void* task);
void BuildDebugKeybladeDeck(u8 deck);
void BuildDebugMixedDeck(u8 deck);
void func_080AB964();
void func_080AB968();
#ifdef VERSION_JP
extern u8 gDeckErrorDeckFullText[];
#endif

#endif /* GUARD_CARD_H */
