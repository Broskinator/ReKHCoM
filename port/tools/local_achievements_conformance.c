#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../achievements/local_achievements.h"
#include "../achievements/toast_layout.h"

int main(void) {
    KhLocalState state = {0}, loaded;
    KhLocalSnapshot s = {0};
    KhToastLayout box, entering, leaving;
    const int sizes[][2] = {{240,160}, {640,360}, {1280,720}, {1920,1080}, {2560,1080}, {3840,2160}};
    unsigned i;
    /* Exercise every condition independently, including its exact boundary.
     * A high save loaded first must never produce a retroactive award. */
    for (i = 0; i < KH_LOCAL_COUNT; ++i) {
        KhLocalState fresh = {0}, baseline = {0};
        KhLocalSnapshot below = {0}, above;
        below.playing = 1; below.level = 1; below.max_hp = 80;
        above = below;
        switch (i) {
        case 0: below.level = 9; above.level = 10; break;
        case 1: below.level = 24; above.level = 25; break;
        case 2: below.level = 49; above.level = 50; break;
        case 3: below.level = 98; above.level = 99; break;
        case 4: below.max_hp = 199; above.max_hp = 200; break;
        case 5: below.cp = 499; above.cp = 500; break;
        case 6: below.moogle_points = 999; above.moogle_points = 1000; break;
        case 7: below.moogle_points = 9999; above.moogle_points = 10000; break;
        case 8: below.friends = 3; above.friends = 4; break;
        case 9: below.learned = 0x1ff; above.learned = 0x1ff; above.learned2 = 1; break;
        case 10: below.card_kinds = (1ull << 19) - 1; above.card_kinds = (1ull << 20) - 1; break;
        case 11: above.flags = 0x20; break;
        case 12: above.flags = 0x808; below.flags = 8; break;
        case 13: below.floor = 3; above.floor = 4; break;
        case 14: below.floor = 7; above.floor = 8; break;
        case 15: below.floor = 11; above.floor = 12; break;
        }
        assert(!KhLocalObserve(&fresh, &below, 1));
        assert(KhLocalObserve(&fresh, &above, 2) == (1u << i));
        assert(fresh.when[i] == 2 && !KhLocalObserve(&fresh, &above, 3));
        assert(!KhLocalObserve(&baseline, &above, 4) && !baseline.unlocked);
        assert(!KhLocalObserve(&baseline, &below, 5));
        assert(!KhLocalObserve(&baseline, &above, 6));
        if (i == 5 || i == 8 || i == 9 || i == 10 || i >= 13) {
            memset(&fresh, 0, sizeof(fresh));
            below.flags |= 8; above.flags |= 8;
            assert(!KhLocalObserve(&fresh, &below, 1));
            assert(!KhLocalObserve(&fresh, &above, 2));
        }
    }
    s.level = 99;
    assert(KhLocalObserve(&state, &s, 1) == 0 && !state.initialized);
    s.playing = 1;
    assert(KhLocalObserve(&state, &s, 2) == 0 && state.initialized);
    /* Existing level-99 saves establish a baseline, without retroactive awards. */
    assert(state.unlocked == 0);
    s.flags = 0x20;
    assert(KhLocalObserve(&state, &s, 3) == (1u << 11));
    assert(KhLocalObserve(&state, &s, 4) == 0);
    assert(state.when[11] == 3);
    memset(&state, 0, sizeof(state)); memset(&s, 0, sizeof(s));
    s.playing = 1; s.level = 1; s.max_hp = 80; s.cp = 275;
    assert(!KhLocalObserve(&state, &s, 1));
    s.level = 10;
    assert(KhLocalObserve(&state, &s, 10) == 1);
    s.level = 25; s.max_hp = 200; s.cp = 500; s.moogle_points = 1000;
    s.friends = 4; s.learned = 1023; s.card_kinds = (1u << 20) - 1; s.floor = 4;
    assert(KhLocalObserve(&state, &s, 20) == ((1u<<1)|(1u<<4)|(1u<<5)|(1u<<6)|(1u<<8)|(1u<<9)|(1u<<10)|(1u<<13)));
    assert(KhLocalSave(&state, "local-test.dat"));
    assert(KhLocalLoad(&loaded, "local-test.dat"));
    assert(loaded.unlocked == state.unlocked && loaded.when[0] == 10 && loaded.when[13] == 20);
    assert(!loaded.initialized && !KhLocalObserve(&loaded, &s, 30));
    /* Windows text-mode saves contain CRLF; consoles and Wasm must read them. */
    { FILE* f = fopen("local-crlf.dat", "wb"); assert(f);
      fputs("KHLOCAL1\r\n1\r\n", f);
      for (i = 0; i < KH_LOCAL_COUNT; ++i) fputs("0\r\n", f);
      fclose(f);
      assert(KhLocalLoad(&loaded, "local-crlf.dat") && loaded.unlocked == 1);
    }
    { FILE* f = fopen("local-test.dat", "w"); assert(f); fputs("KHLOCAL1\nffffffff\n", f); fclose(f); }
    assert(!KhLocalLoad(&loaded, "local-test.dat") && !loaded.unlocked);
    assert(!KhLocalSave(&state, "missing-parent/local-test.dat"));
    for (i = 0; i < sizeof(sizes)/sizeof(sizes[0]); ++i) {
        box = KhAchievementToastLayout(1000, sizes[i][0], sizes[i][1], 220);
        assert(box.x >= 0 && box.x + box.width <= sizes[i][0]);
        assert(box.y >= 0 && box.y + box.height <= sizes[i][1]);
        assert(box.alpha == 1 && box.x * 2 + box.width == sizes[i][0]);
        entering = KhAchievementToastLayout(150, sizes[i][0], sizes[i][1], 220);
        leaving = KhAchievementToastLayout(4450, sizes[i][0], sizes[i][1], 220);
        assert(entering.y < box.y && entering.alpha > 0 && entering.alpha < 1);
        assert(leaving.y < box.y && leaving.alpha > 0 && leaving.alpha < 1);
    }
    puts("PASS all 16 local triggers: exact boundaries, no duplicates/retroactive grants, Sora-only guards, persistence, corrupt data, six toast resolutions");
    return 0;
}
