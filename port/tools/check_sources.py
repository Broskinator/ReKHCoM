#!/usr/bin/env python3
"""Source checks that take seconds and catch what otherwise only fails in a console or Android build.

- The console guests (PS5, PS4, Switch, Vita, Android 64) build port/src with a small SDL shim
  (port/ps5/guest/include/SDL3/SDL.h): every SDLK_/SDL_EVENT_/SDL_GAMEPAD_/SDL_SCANCODE_ constant the port uses must be
  defined there, with SDL's own value (21/10: SDLK_AC_BACK and SDL_EVENT_MOUSE_BUTTON_DOWN broke the Android 64 build).
- Java string and char literals must close on their line (21/10: a script wrote real newlines into GameActivity's
  share-log strings and the APK did not build).
- The generated HD sprite tables (gen_hd_sprites_inc.py) match port/data/hd_sprites.tsv.

Exit code 1 and one line per problem. release_check.py and build_android.py run it first.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SHIM = ROOT / 'port/ps5/guest/include/SDL3/SDL.h'
SDL = ROOT.parent / 'toolchain/android/SDL3-3.4.18/include/SDL3'
CONST = re.compile(r'\b(SDLK_\w+|SDL_EVENT_\w+|SDL_GAMEPAD_\w+|SDL_SCANCODE_\w+)\b')


def guest_sources():
    return sorted((ROOT / 'port/src').glob('*.c')) + [ROOT / 'port/android/native/touch_android.c']


def shim_problems():
    shim = SHIM.read_text(encoding='utf-8')
    defined = set(CONST.findall(shim))
    out = []
    for f in guest_sources():
        for name in sorted(set(CONST.findall(f.read_text(encoding='utf-8', errors='replace'))) - defined):
            out.append(f'{f.relative_to(ROOT)}: {name} is not in the console SDL shim ({SHIM.relative_to(ROOT)})')
    # the shim's keycodes must be SDL's own: the hosts pass SDL's values through
    real = {}
    if SDL.is_dir():
        for h in SDL.glob('*.h'):
            for name, value in re.findall(r'#define\s+(SDLK_\w+)\s+(0x[0-9A-Fa-f]+)u?', h.read_text(encoding='utf-8')):
                real[name] = int(value, 16)
    for name, value in re.findall(r'#define\s+(SDLK_\w+)\s+(0x[0-9A-Fa-f]+)u?', shim):
        if name in real and real[name] != int(value, 16):
            out.append(f'{SHIM.relative_to(ROOT)}: {name} is {value}, SDL has {real[name]:#x}')
    return out


def generated_problems():
    import subprocess
    r = subprocess.run([sys.executable, str(ROOT / 'port/tools/gen_hd_sprites_inc.py'), '--check'],
                       capture_output=True, text=True)
    return [l for l in r.stdout.splitlines() if l.strip()] if r.returncode else []


def java_problems():
    out = []
    for f in sorted((ROOT / 'port/android/app/src').rglob('*.java')):
        block = False
        for n, line in enumerate(f.read_text(encoding='utf-8').splitlines(), 1):
            i, quote = 0, None
            while i < len(line):
                c = line[i]
                if block:
                    if line.startswith('*/', i):
                        block = False
                        i += 1
                elif quote:
                    if c == '\\':
                        i += 1
                    elif c == quote:
                        quote = None
                elif line.startswith('//', i):
                    break
                elif line.startswith('/*', i):
                    block = True
                    i += 1
                elif c in '"\'':
                    quote = c
                i += 1
            if quote:
                out.append(f'{f.relative_to(ROOT)}:{n}: string or char literal not closed on its line')
    return out


def main():
    problems = shim_problems() + java_problems() + generated_problems()
    for p in problems:
        print(p)
    if problems:
        print(f'check_sources: {len(problems)} problem(s)')
        sys.exit(1)
    print('check_sources: ok')


if __name__ == '__main__':
    main()
