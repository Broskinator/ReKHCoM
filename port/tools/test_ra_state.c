#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "game_state.h"
#include "map_types.h"
MapFloorState gMapFloorState;
GameState gGameState;
Deck gDecks[3];
uint16_t gCardCollection[999], gCardCount;
uint8_t gMapCardCounts[270];
uint16_t gPoohGauge;
uint8_t GetActiveDeckIndex(void) { return 2; }
uint16_t GetKeysHeld(void) { return 0x0201; }
volatile uint32_t gVBlankCounter;
int32_t gPendingModeArg;
static uint32_t sTitleStateTest;
uint32_t* PortTitleStatePtr(void) { return &sTitleStateTest; }
static const void* sBgFxForTest;
const void* PortRaBgFxWork(void) { return sBgFxForTest; }
struct CardBattleState* gCardBattleState;
void PortRaOverlayState(uint32_t address, uint8_t* buffer, uint32_t size);
int main(void) {
    uint8_t bytes[sizeof(GameState) + 8];
    const uint32_t base = 0x8000u +
#if defined(VERSION_US)
        0x39BB0u;
#elif defined(VERSION_JP)
        0x39B20u;
#else
        0x3A1C0u;
#endif
    const uint32_t battleRoot = 0x8000u +
#if defined(VERSION_EU)
        0x34A28u, cardRoot = 0x423E4u;
#elif defined(VERSION_US)
        0x34928u, cardRoot = 0x41DD4u;
#else
        0x34928u, cardRoot = 0x41D44u;
#endif
    const uint32_t poohGauge =
#if defined(VERSION_EU)
        0x449D4u;
#elif defined(VERSION_US)
        0x443E4u;
#else
        0x44354u;
#endif
    const uint32_t roomAddress =
#if defined(VERSION_EU)
        0x44B86u;
#elif defined(VERSION_US)
        0x44596u;
#else
        0x44506u;
#endif
    unsigned i;
    gMapFloorState.room = 7;
    memset(bytes, 0xA5, sizeof(bytes));
    PortRaOverlayState(roomAddress - 1, bytes, 3);
    assert(bytes[0] == 0xA5 && bytes[1] == 7 && bytes[2] == 0xA5);
    gPoohGauge = 0x0203;
    memset(bytes, 0xA5, sizeof(bytes));
    PortRaOverlayState(poohGauge - 1, bytes, 4);
    assert(bytes[0] == 0xA5 && bytes[1] == 3 && bytes[2] == 2 && bytes[3] == 0xA5);
    PortRaOverlayState(poohGauge + 1, bytes, 1);
    assert(bytes[0] == 2 && bytes[1] == 3);

    sBgFxForTest = (const void*)(uintptr_t)0x02033F00u;
    gCardBattleState = (struct CardBattleState*)(uintptr_t)0x03006700u;
    memset(bytes, 0xA5, sizeof(bytes));
    PortRaOverlayState(battleRoot, bytes, 4);
    assert(bytes[0] == 0 && bytes[1] == 0x3F && bytes[2] == 3 && bytes[3] == 2 && bytes[4] == 0xA5);
    PortRaOverlayState(cardRoot, bytes, 4);
    assert(bytes[0] == 0 && bytes[1] == 0x67 && bytes[2] == 0 && bytes[3] == 3);
    PortRaOverlayState(battleRoot + 1, bytes, 2);
    assert(bytes[0] == 0x3F && bytes[1] == 3 && bytes[2] == 0 && bytes[3] == 3);
    /* Host/native code addresses, ROM and past-the-heap pointers stay zero. */
    {
        const uintptr_t bad[] = {0, 0x00401234u, 0x02034000u, 0x03006800u, 0x08001234u};
        for (i = 0; i < sizeof(bad)/sizeof(*bad); ++i) {
            sBgFxForTest = (const void*)bad[i];
            gCardBattleState = (struct CardBattleState*)bad[i];
            memset(bytes, 0xA5, 8);
            PortRaOverlayState(battleRoot, bytes, 4);
            assert(!bytes[0] && !bytes[1] && !bytes[2] && !bytes[3] && bytes[4] == 0xA5);
            PortRaOverlayState(cardRoot, bytes, 4);
            assert(!bytes[0] && !bytes[1] && !bytes[2] && !bytes[3]);
        }
    }

    memset(&gGameState, 0x3C, sizeof(gGameState));
    memset(bytes, 0xA5, sizeof(bytes));
    gGameState.flags = 0x12345678;
    gGameState.progression.level = 50;
    gGameState.progression.maxHp = 400;
    gGameState.progression.learnedStocks2 = UINT64_C(0x1122334455667788);
    PortRaOverlayState(base - 4, bytes, sizeof(bytes));
    for (i = 0; i < 4; ++i) assert(bytes[i] == 0xA5 && bytes[sizeof(bytes) - i - 1] == 0xA5);
    assert(!memcmp(bytes + 4, &gGameState, offsetof(GameState, enemyCache)));
    for (i = offsetof(GameState, enemyCache); i < offsetof(GameState, progression); ++i)
        assert(bytes[4+i] == 0xA5); /* pointer-bearing section is never copied */
    assert(!memcmp(bytes + 4 + offsetof(GameState, progression), &gGameState.progression,
                   sizeof(GameState) - offsetof(GameState, progression)));
    memset(bytes, 0xA5, sizeof(bytes));
    PortRaOverlayState(base + offsetof(GameState, flags) + 1, bytes, 2);
    assert(bytes[0] == 0x56 && bytes[1] == 0x34 && bytes[2] == 0xA5);
    PortRaOverlayState(UINT32_MAX - 1, bytes, 4); assert(bytes[0] == 0x56);
    PortRaOverlayState(base, NULL, 4);
    {
        const uint32_t decks =
#if defined(VERSION_US)
            0x41DE0u, collection = 0x42080u, count = 0x42858u, maps = 0x428C0u, active = 0x3CAB0u;
#elif defined(VERSION_JP)
            0x41D50u, collection = 0x41FF0u, count = 0x427C8u, maps = 0x42830u, active = 0x3CA10u;
#else
            0x423F0u, collection = 0x42690u, count = 0x42E68u, maps = 0x42ED0u, active = 0x3CACCu;
#endif
        gDecks[1].cards[98] = 0x8123; gCardCollection[998] = 0x4567;
        gCardCount = 999; gMapCardCounts[269] = 9;
        PortRaOverlayState(decks + sizeof(Deck) + 196, bytes, 2); assert(bytes[0] == 0x23 && bytes[1] == 0x81);
        PortRaOverlayState(collection + 1996, bytes, 2); assert(bytes[0] == 0x67 && bytes[1] == 0x45);
        PortRaOverlayState(count, bytes, 2); assert(bytes[0] == 0xE7 && bytes[1] == 3);
        PortRaOverlayState(maps + 269, bytes, 1); assert(bytes[0] == 9);
        PortRaOverlayState(active, bytes, 1); assert(bytes[0] == 2);
    }
    gVBlankCounter = 0x12345678; gPendingModeArg = 162;
    PortRaOverlayState(0x6C68, bytes, 4); assert(bytes[0] == 0x78 && bytes[3] == 0x12);
    PortRaOverlayState(
#ifdef VERSION_EU
        0x749C,
#else
        0x7498,
#endif
        bytes, 2); assert(bytes[0] == 162 && bytes[1] == 0);
    PortRaOverlayState(0x3C000, bytes, 1); assert(bytes[0] == 1);
    PortRaOverlayState(0x3C001, bytes, 1); assert(bytes[0] == 2);
    puts("PASS scalar GameState/card projection: region, byte order, clipping, partial reads, native pointer exclusion");
    return 0;
}
