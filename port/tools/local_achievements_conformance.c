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
    puts("PASS local achievements: transitions, no retroactive grants, persistence, corrupt data, six toast resolutions");
    return 0;
}
