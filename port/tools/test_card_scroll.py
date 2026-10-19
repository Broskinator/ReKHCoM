#!/usr/bin/env python3
"""Check battle-only card inversion, actual virtual-pad inputs and INI persistence.

No ROMs or player saves are needed. Pass --toolchain and --sdl when the
llvm-mingw / SDL3 development packages are not under ../toolchain.
"""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
HARNESS = r'''
#include <assert.h>
#include <stdio.h>
#include <SDL3/SDL.h>
#include "port/port.h"
#include "mode.h"
#include "text_types.h"
#include "msg.h"
#include "card_message_data.h"
void PortInputSuppressUntilRelease(void);

Mode gModeBattle, gModeVsbattle;
static Mode sMenu;
Mode* gCurrentMode;
static uint32_t sFrame;
uint32_t PortFrameCount(void) { return sFrame; }
void PortLog(const char* fmt, ...) { (void)fmt; }
int PortUiMenuCanvas(void) { return 0; }

static void config_test(void) {
    FILE* f;
    assert(gPortConfig.invertCardScroll == 0);
    PortConfigLoad(); /* no settings file */
    assert(gPortConfig.invertCardScroll == 0);
    for (int enabled = 0; enabled < 2; enabled++) {
        gPortConfig.invertCardScroll = enabled;
        PortConfigSave();
        gPortConfig.invertCardScroll = !enabled;
        PortConfigLoad();
        assert(gPortConfig.invertCardScroll == enabled);
    }
    f = fopen("kh_com.ini", "w");
    assert(f); fputs("invert_card_scroll=99\n", f); fclose(f);
    PortConfigLoad();
    assert(gPortConfig.invertCardScroll == 1);
    f = fopen("kh_com.ini", "w");
    assert(f); fputs("invert_card_scroll=-1\n", f); fclose(f);
    PortConfigLoad();
    assert(gPortConfig.invertCardScroll == 0);
    f = fopen("kh_com.ini", "w");
    assert(f); fputs("stock_button=1\n", f); fclose(f);
    PortConfigLoad(); /* old settings have no inversion key */
    assert(gPortConfig.invertCardScroll == 0);
}

static void key_test(void) {
    Mode* modes[] = { NULL, &sMenu, &gModeBattle, &gModeVsbattle };
    const uint16_t shoulders[] = { 0, 0x100, 0x200, 0x300 };
    for (int enabled = 0; enabled < 2; enabled++) {
        gPortConfig.invertCardScroll = enabled;
        for (unsigned m = 0; m < SDL_arraysize(modes); m++) {
            gCurrentMode = modes[m];
            int reverse = enabled && m >= 2;
            for (unsigned s = 0; s < SDL_arraysize(shoulders); s++) {
                unsigned direction = reverse && (s == 1 || s == 2) ? 3 - s : s;
                for (uint16_t other = 0; other < 0x100; other++) {
                    uint16_t input = other | shoulders[s];
                    assert(PortInputResolve(input) == (other | shoulders[direction]));
                }
            }
            assert(PortInputResolve(PORT_KEY_STOCK) == 0x300);
            assert(PortInputResolve(0x300 | PORT_KEY_STOCK) == 0x300);
        }
    }
}

static void tutorial_test(void) {
#ifdef VERSION_JP
    const TextChar original[] = {0x82, 0x6b, 0x83, 0x4c, 0x82, 0x71, 0};
    const TextChar expected[] = {0x82, 0x71, 0x83, 0x4c, 0x82, 0x6b, 0};
#else
    const TextChar original[] = {'L', 'R', MSG_CODE_HIGHLIGHT, 'L', MSG_CODE_PLAIN, ':', 'R', MSG_CODE_HIGHLIGHT, 'R', MSG_CODE_PLAIN, ':', 'L', 0};
    const TextChar expected[] = {'L', 'R', MSG_CODE_HIGHLIGHT, 'R', MSG_CODE_PLAIN, ':', 'R', MSG_CODE_HIGHLIGHT, 'L', MSG_CODE_PLAIN, ':', 'L', 0};
#endif
    TextChar unchanged[SDL_arraysize(original)], oversized[257];
    SDL_memcpy(unchanged, original, sizeof(original));
    gPortConfig.invertCardScroll = 0;
    assert(PortCardScrollTutorialText(CARD_MSG_ROBE_TUTORIAL_18, original) == original);
    gPortConfig.invertCardScroll = 1;
    assert(PortCardScrollTutorialText(CARD_MSG_ROBE_TUTORIAL_17, original) == original);
    assert(PortCardScrollTutorialText(CARD_MSG_ROBE_TUTORIAL_18, NULL) == NULL);
    const TextChar* result = PortCardScrollTutorialText(CARD_MSG_ROBE_TUTORIAL_18, original);
    assert(result != original && SDL_memcmp(result, expected, sizeof(expected)) == 0);
    assert(SDL_memcmp(original, unchanged, sizeof(original)) == 0);
    assert(SDL_memcmp(PortCardScrollTutorialText(CARD_MSG_ROBE_TUTORIAL_18, original), expected, sizeof(expected)) == 0);
    for (unsigned i = 0; i < SDL_arraysize(oversized) - 1; i++) oversized[i] = 'L';
    oversized[256] = 0;
    assert(PortCardScrollTutorialText(CARD_MSG_ROBE_TUTORIAL_18, oversized) == oversized);
}

static void device_test(void) {
    /* Inputs come through the production SDL virtual-pad and keyboard paths. */
    const uint16_t expected[] = { 0, 0x200, 0x100, 0x200, 0x100, 0x300, 0x300, PORT_KEY_STOCK };
    assert(SDL_Init(SDL_INIT_GAMEPAD));
    PortInputInit();
    gCurrentMode = &gModeBattle;
    for (sFrame = 1; sFrame < SDL_arraysize(expected); sFrame++) {
        uint16_t keys = PortInputKeys();
        assert(keys == expected[sFrame]);
        gPortConfig.invertCardScroll = 0;
        uint16_t normal = PortInputResolve(keys);
        gPortConfig.invertCardScroll = 1;
        uint16_t inverted = PortInputResolve(keys);
        assert(inverted == (normal == 0x100 ? 0x200 : normal == 0x200 ? 0x100 : normal));
        gCurrentMode = &sMenu;
        assert(PortInputResolve(keys) == normal);
        gCurrentMode = &gModeBattle;
    }
    sFrame = 1000;
    SDL_Event event = {0};
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = SDLK_A;
    PortInputHandleEvent(&event);
    assert(PortInputResolve(PortInputKeys()) == 0x100);
    event.type = SDL_EVENT_KEY_UP;
    PortInputHandleEvent(&event);
    assert(PortInputResolve(PortInputKeys()) == 0);
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = SDLK_S;
    PortInputHandleEvent(&event);
    assert(PortInputResolve(PortInputKeys()) == 0x200);
    PortInputSuppressUntilRelease();
    assert(PortInputKeys() == 0);
    event.type = SDL_EVENT_KEY_UP;
    PortInputHandleEvent(&event);
    assert(PortInputKeys() == 0);
    SDL_Quit();
}

int main(void) {
    config_test();
    key_test();
    tutorial_test();
    device_test();
    puts("PASS: defaults, INI round-trip/clamping, all GBA key combinations, battle/Versus/menu scope, keyboard, shoulders, right stick, stock, suppression and localized tutorial labels");
    return 0;
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--toolchain', type=Path)
    parser.add_argument('--sdl', type=Path)
    parser.add_argument('--version', choices=('eu', 'us', 'jp'), default='eu')
    args = parser.parse_args()
    tc = args.toolchain or next((ROOT.parent / 'toolchain').glob('llvm-mingw-*'))
    sdl = (args.sdl or next((ROOT.parent / 'toolchain').glob('SDL3-*'))) / 'i686-w64-mingw32'
    game_headers = [ROOT / 'include'] + sorted(p for p in (ROOT / 'include').iterdir() if p.is_dir() and p.name != 'gba')
    quoted = [arg for path in game_headers for arg in ('-iquote', str(path))]
    build = ROOT / 'build'
    build.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='card-scroll-', dir=build) as temp:
        work = Path(temp)
        source = work / 'test.c'
        source.write_text(HARNESS, encoding='utf-8')
        exe = work / 'test.exe'
        subprocess.run([str(tc / 'bin/i686-w64-mingw32-clang.exe'), '-std=gnu11',
                        '-DVERSION_' + args.version.upper(), '-DPLATFORM_PC', '-mno-ms-bitfields', '-ffunction-sections', '-fdata-sections', '-I' + str(ROOT / 'port/include'),
                        *quoted, '-I' + str(sdl / 'include'),
                        str(source), str(ROOT / 'port/src/port_input.c'), str(ROOT / 'port/src/port_config.c'),
                        '-Wl,--gc-sections', '-L' + str(sdl / 'lib'), '-lSDL3', '-o', str(exe)], check=True)
        shutil.copy2(sdl / 'bin/SDL3.dll', work / 'SDL3.dll')
        env = dict(os.environ, SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy',
                   KHCOM_VPAD='1-1:LB,2-2:RB,3-3:RS_LEFT,4-4:RS_RIGHT,5-5:RS_UP,6-6:LB,6-6:RB,7-7:NORTH')
        subprocess.run([str(exe)], cwd=work, env=env, check=True)


if __name__ == '__main__':
    main()
