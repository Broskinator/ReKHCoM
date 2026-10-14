#ifndef GUARD_CARD_BATTLE_H
#define GUARD_CARD_BATTLE_H

#include "types.h"
#include "anim.h"
#include "taskpool.h"
#include "card.h"
#include "card_types.h"
#include "macros.h"

struct CardDisplayWork;
struct CardSlot;
struct CardBattleWork;

enum CardRequest {
    CARD_REQUEST_NONE,
    CARD_REQUEST_NEXT_CARD,
    CARD_REQUEST_PREV_CARD,
    CARD_REQUEST_USE_CARD,
    CARD_REQUEST_STOCK_CARD,
    CARD_REQUEST_USE_STOCK,
    CARD_REQUEST_OPEN_CARDS,
    CARD_REQUEST_CLOSE_CARDS,
    CARD_REQUEST_NOP,
    CARD_REQUEST_CYCLE_LIST,
    CARD_REQUEST_SWITCH_LIST,
    CARD_REQUEST_AUTO_CYCLE_60,
    CARD_REQUEST_AUTO_CYCLE_180,
    CARD_REQUEST_AUTO_CYCLE_300,
    CARD_REQUEST_KING_RELOAD_0,
    CARD_REQUEST_KING_RELOAD_1,
    CARD_REQUEST_KING_RELOAD_2,
    CARD_REQUEST_POTION,
    CARD_REQUEST_HI_POTION,
    CARD_REQUEST_MEGA_POTION,
    CARD_REQUEST_REMOVE_ITEM_CARDS,
    CARD_REQUEST_ETHER,
    CARD_REQUEST_MEGA_ETHER,
    CARD_REQUEST_ELIXIR,
    CARD_REQUEST_MEGALIXIR
};

typedef struct CardBattleState {
    struct CardDisplayWork* activeCards[6];
    struct CardDisplayWork* soraStockedCards[3];
    struct CardDisplayWork* rikuStockedCards[3];
    void* tiles[4];
    void* premiumTiles;
    void* tiles5;
    void* tiles6;
    void* tiles7;
    void* premiumTiles2;
    void* palette;
    void* palette2;
    struct CardBattleWork* soraWork;
    struct CardBattleWork* rikuWork;
    AnimState anim;
    AnimState anim2;
    void* gfx;
    void* gfx2;
    TaskPool tasks;
    u32 unk_0B0;
    u32 unk_0B4;
    u32 pickedFriendCardId;
    u32 pickedGimmickCardId;
    u16 unk_0C0;
    s16 activeValue;
    u16 soraStockName;
    u16 rikuStockName;
    u16 unk_0C8;
    u16 unk_0CA;
    u16 soraHcEffect;
    u16 rikuHcEffect;
    u8 activeCardCount;
    u8 unk_0D1;
    u8 soraListIndex;
    u8 soraStockCount;
    u8 rikuListIndex;
    u8 rikuStockCount;
    u8 friendCardCount;
    u8 nextEnemyCardIndex;
    u8 unk_0D8;
    u8 unk_0D9;
    u8 addedFriendCards[0x02];
    u8 gimmickCardCount;
    u8 stockMoveCount;
    u8 soraStockedCount;
    u8 rikuStockedCount;
    u8 enemyCardUsed;
    u8 soraStockActive;
    u8 rikuStockActive;
    u8 soraStockNameShown;
    u8 rikuStockNameShown;
    u8 unk_0E5;
    u8 unk_0E6;
    u8 soraReloadCharging;
    u8 rikuReloadCharging;
    u8 levelUpShown;
    u8 cardsOpen;
    u8 soraHcEffectReplaced;
    u8 rikuHcEffectReplaced;
    u8 unk_0ED;
    u8 darkModeReady;
    u16 rikuCardsLeft;
    u32 soraReloadGauge;
    u32 rikuReloadGauge;
    u16 soraReloadCounter;
    u16 rikuReloadCounter;
    s16 soraGaugeFullFrame;
    s16 rikuGaugeFullFrame;
    s16 soraGaugeAnim;
    s16 rikuGaugeAnim;
    u8 reloadGaugeFull[0x04];
} CardBattleState;

STATIC_ASSERT(sizeof(CardBattleState) == 0x10C, CardBattleStateSize);

extern CardBattleState* gCardBattleState;

void SetSoraReloadCharging();
void RequestSoraNextCard();
void RequestSoraPrevCard();
void RequestSoraCardUse();
void RequestSoraCardStock();
void RequestSoraStockUse();
void RequestSwitchSoraCardList();
u8 IsSoraReloadCardSelected();
u8 IsSoraSelectionEmpty();
void RequestOpenCards();
void RequestCloseCards();
void RequestSoraMegalixir();
void RequestSoraPotion();
void RequestSoraHiPotion();
void RequestSoraMegaPotion();
void RequestSoraEther();
void RequestSoraMegaEther();
void RequestSoraElixir();
void RequestSoraKingReload0();
void RequestSoraKingReload1();
void RequestSoraKingReload2();
void RequestSoraRemoveItemCards();
s32 cardbattleSora_1(struct CardBattleWork* work, Task* task);
struct CardSlot* FindNextAvailableSlot(struct CardBattleWork* work, u8 listIndex, u16* index);
struct CardSlot* FindPrevAvailableSlot(struct CardBattleWork* work, u8 listIndex, u16* index);
void CreateSoraCardRing(struct CardBattleWork* work, u8 listIndex);
s32 UpdateSoraReloadDeal(CardBattleWork* work, Task* task);
void func_0807B458(CardBattleWork* work, u16 hcEffect);
void SyncSoraHcEffect(CardBattleWork* work);
void ApplySoraHcEffect(CardBattleWork* work);
void TickSoraHcEffectOnCardUse();
u8 SoraStockMoveToSlot(CardDisplayWork* work, void* task);
void UpdateSoraCardRingPosition(CardDisplayWork* work);
u8 SoraCardShrinkAway(CardDisplayWork* work);
void UpdateSoraPlayedCardPosition(CardDisplayWork* work);
u8 DispatchSoraCardCommand(CardDisplayWork* work, void* task);
void LookupSoraCardDef(CardDisplayArgs* args, const CardDef** out, u8 index);
void LinkSoraCardDisplay(CardDisplayWork* work);
void LoadSoraCardDisplayGfx2(CardDisplayWork* work);
void RefreshSoraCardDisplayGfx(CardDisplayWork* work);
u8 SoraGimmickCardLaunch(CardDisplayWork* work, void* task);
u8 SoraGimmickCardHit(CardDisplayWork* work);
u8 SoraStockVanish(CardDisplayWork* work);
void UpdateSoraReloadGauge(CardDisplayWork* work);
void UpdateSoraCardValue(CardDisplayWork* work);
void TickSoraHcEffectOnPlayEnd();
void TickSoraHcEffectOnAttackEnd();

#ifdef PLATFORM_PC
/* Port option: return Sora's stocked cards to the hand. */
void RequestSoraUnstock(void);
#endif

#endif
