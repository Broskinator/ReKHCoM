/* Scalar GameState and card data projection for RA's flattened GBA address space.
 * Addresses are the r4 literal used by each original InitGameState, verified
 * by test_ra_state_addresses.py against the user's private cartridges.
 * Never expose native pointer bytes from enemyCache or its intrusive pool.
 */
#include <stdint.h>
#include <string.h>
#include "game_state.h"
extern Deck gDecks[3];
extern uint16_t gCardCollection[999], gCardCount;
extern uint8_t gMapCardCounts[270];
extern uint8_t GetActiveDeckIndex(void);
STATIC_ASSERT(sizeof(Deck) == 0xE0, RaDeckLayout);

/* GameState, decks, collection, count, map cards, active deck. */
static const uint32_t sOriginalState[6] = {
#if defined(VERSION_US)
    0x02039BB0u, 0x02039DE0u, 0x0203A080u, 0x0203A858u, 0x0203A8C0u, 0x02034AB0u
#elif defined(VERSION_JP)
    0x02039B20u, 0x02039D50u, 0x02039FF0u, 0x0203A7C8u, 0x0203A830u, 0x02034A10u
#else
    0x0203A1C0u, 0x0203A3F0u, 0x0203A690u, 0x0203AE68u, 0x0203AED0u, 0x02034ACCu
#endif
};
static void Overlay(uint32_t address, uint8_t* buffer, uint32_t size,
                    uint32_t original, const void* source, uint32_t length) {
    uint32_t begin = 0x8000u + original - 0x02000000u, end = begin + length;
    uint32_t from = address > begin ? address : begin;
    uint32_t to = address + size < end ? address + size : end;
    if (from < to) memcpy(buffer + from - address, (const uint8_t*)source + from - begin, to - from);
}

void PortRaOverlayState(uint32_t address, uint8_t* buffer, uint32_t size) {
    const uint32_t spans[2][2] = {
        {0, offsetof(GameState, enemyCache)},
        {offsetof(GameState, progression), sizeof(GameState)}
    };
    unsigned i;
    if (!buffer || !size || address > 0x58000u || size > 0x58000u - address) return;
    for (i = 0; i < 2; ++i) {
        Overlay(address, buffer, size, sOriginalState[0] + spans[i][0],
                (const uint8_t*)&gGameState + spans[i][0], spans[i][1] - spans[i][0]);
    }
    Overlay(address, buffer, size, sOriginalState[1], gDecks, sizeof(gDecks));
    Overlay(address, buffer, size, sOriginalState[2], gCardCollection, sizeof(gCardCollection));
    Overlay(address, buffer, size, sOriginalState[3], &gCardCount, sizeof(gCardCount));
    Overlay(address, buffer, size, sOriginalState[4], gMapCardCounts, sizeof(gMapCardCounts));
    { uint8_t active = GetActiveDeckIndex();
      Overlay(address, buffer, size, sOriginalState[5], &active, sizeof(active)); }
}
