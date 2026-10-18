/* Guard Armor patio: continue authored architecture and perspective rows.
 * Source tiles stay in the user's ROM; no copied/derived image is shipped.
 * See progress/BATTLE_SCENERY_TECHNIQUE.md for the reusable workflow. */
#include <string.h>
#include "port/port.h"
#include "port/port_battle_backdrop.h"

extern const uint16_t gBosGaBgTopLeftMap[];
extern const uint16_t gBosGaBgTopRightMap[];
extern const uint16_t gBosGaBgBottomLeftMap[];
extern const uint16_t gBosGaBgBottomRightMap[];
extern const uint8_t gBosGaBgTiles[];

typedef struct FloorRow {
    int left, leftPeriod;
    int right, rightPeriod;
} FloorRow;
static FloorRow sRows[120];
static int sPrepared;

/* Positive remainder is essential on the left of the authored map. */
static int Repeat(int position, int start, int width) {
    int offset = (position - start) % width;
    return start + (offset < 0 ? offset + width : offset);
}

static int SourceIndex(int x, int y) {
    const uint16_t* maps[4] = { gBosGaBgTopLeftMap, gBosGaBgTopRightMap,
                              gBosGaBgBottomLeftMap, gBosGaBgBottomRightMap };
    const uint16_t* block = maps[(y >> 8) * 2 + (x >> 8)];
    unsigned entry = block[((y & 255) >> 3) * 32 + ((x & 255) >> 3)];
    int px = x & 7, py = y & 7, index;
    unsigned offset;
    if (entry & 0x400) px = 7 - px;
    if (entry & 0x800) py = 7 - py;
    offset = (entry & 1023) * 32 + py * 4 + px / 2;
    if (offset >= 16352) return 0;
    index = (gBosGaBgTiles[offset] >> ((px & 1) * 4)) & 15;
    return index ? (int)(entry >> 12) * 16 + index : 0;
}

static void PrepareFloor(void) {
    int y;
    if (sPrepared) return;
    for (y = 384; y < 504; y++) {
        unsigned counts[256] = {0};
        int order[256], count = 0, x, a = 0, b = 0, changes[320], n = 0, last = -1;
        FloorRow* row = &sRows[y - 384];
        /* Two dominant colours identify a pair of paving cells. Keep ties
         * in first-seen order, independent of palette animation or fading. */
        for (x = 224; x < 384; x++) {
            int c = SourceIndex(x, y);
            if (!counts[c]) order[count++] = c;
            counts[c]++;
        }
        for (x = 0; x < count; x++) {
            int c = order[x];
            if (counts[c] > counts[a]) a = c;
        }
        for (x = 0; x < count; x++) {
            int c = order[x];
            if (c != a && (b == a || counts[c] > counts[b])) b = c;
        }
        for (x = 96; x < 416; x++) {
            int c = SourceIndex(x, y);
            if (c != a && c != b) continue;
            if (last >= 0 && c != last) changes[n++] = x;
            last = c;
        }
        if (n >= 3) {
            row->left = changes[0]; row->leftPeriod = changes[2] - changes[0];
            row->right = changes[n - 3]; row->rightPeriod = changes[n - 1] - changes[n - 3];
        }
    }
    sPrepared = 1;
}

int PortGuardArmorBackdropMatches(const void* const* maps) {
    /* Identify the exact background, not a guessed battle ID or mode.
     * No state/lifecycle flag is needed when leaving/reloading a fight. */
    return maps && maps[0] == gBosGaBgTopLeftMap && maps[1] == gBosGaBgTopRightMap &&
           maps[2] == gBosGaBgBottomLeftMap && maps[3] == gBosGaBgBottomRightMap;
}

int PortGuardArmorBackdropPixel(int x, int y, uint16_t* color) {
    int sx, sy = y, index;
    if (x >= 96 && x < 416 && y >= 232 && y < 512)
        return 0; /* exact authored pixels, including its front curb */
    if (y < 232) {
        sx = Repeat(x, 352, 64); sy = 232; /* quiet night sky */
    } else if (y < (x < 96 ? 400 : 384)) {
        /* A facade bay on the left; roof/arches on the right. The tower,
         * stairs and the whole patio are never wrapped to another screen. */
        sx = x < 96 ? Repeat(x, 96, 56) : Repeat(x, 368, 48);
    } else if (y >= 512) {
        int width = 32 + (y - 504) / 6;
        int column = x - 256 + width / 2;
        int row = (y - 480) / 32;
        /* Floor division, rather than truncation toward zero. */
        column = column >= 0 ? column / width : -((-column + width - 1) / width);
        sx = ((column + row) & 1) ? 288 : 256;
        sy = 500;
    } else if (y >= 504) {
        sx = Repeat(x, 96, 32); /* extend the authored foreground curb once */
    } else {
        const FloorRow* row;
        PrepareFloor();
        row = &sRows[y - 384];
        if (x < 96 && row->leftPeriod > 0)
            sx = Repeat(x, row->left, row->leftPeriod);
        else if (x >= 416 && row->rightPeriod > 0)
            sx = Repeat(x, row->right, row->rightPeriod);
        else
            sx = 256;
    }
    index = SourceIndex(sx, sy);
    *color = index ? ((const uint16_t*)(uintptr_t)GBA_PLTT)[index] & 0x7FFF : 0x8000;
    return 1;
}
