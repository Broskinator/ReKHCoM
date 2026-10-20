"""Official ending achievements, offline (71): the ending movie plays through the
game's own movie mode (it hands over to the staff roll) on the floor the story
ends on, with the official definitions evaluated (KHCOM_RA_DRYRUN). Each case
says what must and must not trigger:

  15605  Sora's ending (13F, Marluxia's card recorded)
  41892  the same with max HP still 80
  15666  Riku's ending (B1F)

    python port/tools/ra_ending_dryrun.py
"""
import os
import re
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DEFS = ROOT / 'build/ra-reference-audit/references.csv.definitions.json'
OUT = ROOT / 'build/ra-ending-dryrun'
IDS = {15605, 41892, 15666}


def place(floor, stage, world, first):
    return floor | stage << 8 | world << 16 | first << 24


SORA = f'300:room:10900,350:gsplace:{place(12, 12, 0, 10)}'
# Riku's ending is played as the game does: his last event (EVENT_195, debug
# event list 1:46) hands over to the movie with the heap reset, so the movie
# starts over the event's leftovers (EVENT_EXIT_MOVIE_RIKU_ENDING)
RIKU_KEYS = ','.join(f'{f}-{f + 2}:A' for f in range(120, 2600, 40))


def riku(floor):
    g = place(floor, 12, 12, 8)
    return dict(event='1:46', keys=RIKU_KEYS, modes=f'60:gsplace:{g},100:gsplace:{g},140:gsplace:{g},2600:movieend:5')
CASES = [
    ('sora-ending', f'{SORA},360:jiminy:208,400:movieend:4', {15605, 41892}),
    ('sora-no-marluxia', f'{SORA},400:movieend:4', set()),
    ('sora-hp-raised', f'{SORA},360:jiminy:208,370:cheat:1,400:movieend:4', {15605}),
    ('sora-other-movie', f'{SORA},360:jiminy:208,400:movieend:3', set()),
    ('riku-ending', riku(11), {15666}),
    ('riku-other-floor', riku(10), set()),
]


def run(case):
    name, modes, _ = case
    extra = {}
    if isinstance(modes, dict):
        extra = dict(KHCOM_EVENT=modes['event'], KHCOM_KEYS=modes['keys'])
        modes = modes['modes']
    d = OUT / name
    d.mkdir(parents=True, exist_ok=True)
    for f in ('khcom.exe', 'SDL3.dll'):
        shutil.copy2(ROOT / 'build/pc-eu' / f, d / f)
    (d / 'kh_com.log').unlink(missing_ok=True)
    env = dict(os.environ, SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1',
               KHCOM_RA_DRYRUN=str(DEFS), KHCOM_TRACE_MODES='1', KHCOM_MODE=modes, KHCOM_QUIT='6500', **extra)
    subprocess.run([str(d / 'khcom.exe'), '--language', 'es', '--rom', str(ROOT / 'roms/B8CP.gba'),
                    '--data-dir', str(d)], cwd=d, env=env, timeout=900,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    log = (d / 'kh_com.log').read_text(errors='replace')
    return {int(a) for a, _ in re.findall(r'ra-dryrun: trigger (\d+) frame (\d+)', log)} & IDS


def main():
    # one at a time: the movie runs on real-time ticks (the GBA timer), and a
    # loaded machine delays its frames past what the graphs watch
    results = [run(case) for case in CASES]
    ok = True
    for (name, _, want), got in zip(CASES, results):
        good = got == want
        ok &= good
        print(f'{name:20s} want {sorted(want)} got {sorted(got)} {"OK" if good else "FAIL"}')
    print('PASS' if ok else 'FAIL')


if __name__ == '__main__':
    main()
