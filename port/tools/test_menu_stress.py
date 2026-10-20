#!/usr/bin/env python3
"""Stress the game's field menu and the port's Options menu opened on top of it.

From a copied save in the field, each cycle opens the game's menu (START), opens
the Options menu inside it, closes both and goes on, many times, with seeded
random timing so openings also land in the middle of fades and transitions:

  pad     a virtual gamepad presses the real buttons: START for the game's menu,
          START+BACK held together for Options (the same route as a player),
          B / START to close, sometimes A to go one menu deeper (deck).
  toggle  the harness opens and closes Options at random frames (KHCOM_OPTIONS_TOGGLE)
          while the game's menu opens and closes.

Each run is a hidden process (no audio) in build/menustress/<variant>-<seed>. A run
fails on a crash, a timeout, a non-zero exit or a fault the desktop recovered
(those close the game on Android, the web and Linux ARM). Several kh_com.ini
setups are cycled (wide view, HD text, integer zoom) since the Options menu
resizes the picture.

    python port/tools/test_menu_stress.py [--build build/pc-eu] [--save file.sav] [--runs 8] [--cycles 40]
"""
import argparse
import os
import random
import re
import shutil
import subprocess
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
INIS = [
    'config_version=2\nview_mode=4\nhd_text=1\nwindow_scale=2\n',
    'config_version=2\nview_mode=0\nhd_text=0\nwindow_scale=1\n',
    'config_version=2\nview_mode=4\nhd_text=0\nzoom=3\ninteger_zoom=1\n',
]
FIELD = 1600  # the save's field after the boot route below


def boot_keys():
    """Title, file select and load: A every 50 frames, as test_menu_cycles.py."""
    return [f'{f}-{f + 3}:SOUTH' for f in range(200, 1400, 50)]


def pad_route(rng, cycles):
    keys, frame = boot_keys(), FIELD
    for _ in range(cycles):
        keys.append(f'{frame}-{frame + 3}:START')                     # the game's menu
        t = frame + rng.randint(5, 120)                                # sometimes mid-fade
        keys += [f'{t}-{t + 6}:START', f'{t + 2}-{t + 6}:BACK']       # Options (START+BACK)
        t += rng.randint(20, 90)
        if rng.random() < 0.5:
            keys.append(f'{t}-{t + 3}:DOWN')
            t += rng.randint(5, 20)
        keys.append(f'{t}-{t + 3}:START')                              # close Options
        t += rng.randint(10, 80)
        if rng.random() < 0.3:                                         # one menu deeper and back
            keys.append(f'{t}-{t + 3}:SOUTH')
            t += rng.randint(60, 200)
            keys.append(f'{t}-{t + 3}:EAST')
            t += rng.randint(60, 160)
        keys.append(f'{t}-{t + 3}:EAST')                              # close the game's menu
        frame = t + rng.randint(30, 140)
    return keys, frame


def toggle_route(rng, cycles):
    keys, toggles, frame = [k.replace('SOUTH', 'A') for k in boot_keys()], [], FIELD
    for _ in range(cycles):
        keys.append(f'{frame}-{frame + 3}:START')
        t = frame + rng.randint(1, 150)
        toggles.append(t)                                              # opens; KHCOM_OPTIONS_HOLD closes it
        t += rng.randint(1, 60)                                        # (game frames stop while it is open)
        keys.append(f'{t}-{t + 3}:B')
        frame = t + rng.randint(20, 120)
    return keys, toggles, frame


def run(args, variant, seed):
    rng = random.Random(seed)
    work = ROOT / 'build/menustress' / f'{variant}-{seed}'
    shutil.rmtree(work, ignore_errors=True)
    work.mkdir(parents=True)
    for f in ('khcom.exe', 'SDL3.dll'):
        shutil.copy2(ROOT / args.build / f, work / f)
    shutil.copy2(args.save, work / 'kh_com.sav')
    shutil.copytree(ROOT / 'port/assets/fonts', work / 'assets/fonts')
    (work / 'kh_com.ini').write_text(INIS[seed % len(INIS)])
    env = {k: v for k, v in os.environ.items() if not k.startswith(('KHCOM_', 'SDL_'))}
    env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1', KHCOM_TRACE_MODES='1',
               KHCOM_BIOS_STACK='1')
    if variant == 'pad':
        keys, end = pad_route(rng, args.cycles)
        env['KHCOM_VPAD'] = ','.join(keys)
    else:
        keys, toggles, end = toggle_route(rng, args.cycles)
        env['KHCOM_KEYS'] = ','.join(keys)
        env['KHCOM_OPTIONS_TOGGLE'] = ','.join(map(str, toggles))
        env['KHCOM_OPTIONS_HOLD'] = str(rng.randint(0, 40))
    env['KHCOM_QUIT'] = str(end + 120)
    try:
        code = subprocess.run([str(work / 'khcom.exe'), '--language', 'es', '--rom', str(ROOT / 'roms/B8CP.gba')],
                              cwd=work, env=env, stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL,
                              stderr=subprocess.DEVNULL, timeout=args.timeout).returncode
    except subprocess.TimeoutExpired:
        code = 'timeout'
    log = (work / 'kh_com.log').read_text(errors='replace') if (work / 'kh_com.log').exists() else ''
    modes = re.findall(r'harness: frame (\d+) mode (\S+)', log)
    menus = sum(int(f) >= FIELD and n not in ('Mode_MapFld', 'Mode_MapFix') for f, n in modes)
    faults = sorted(set(re.findall(r'bios: [^\n]+', log)))
    crash = [l for l in log.splitlines() if 'CRASH' in l or 'FATAL' in l]
    ok = code == 0 and not crash and not faults
    return f'{variant}-{seed}', ok, code, menus, faults, crash


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--build', default='build/pc-eu')
    ap.add_argument('--save', required=True, help='a field save (copied, never modified)')
    ap.add_argument('--runs', type=int, default=8, help='runs per variant')
    ap.add_argument('--cycles', type=int, default=40)
    ap.add_argument('--jobs', type=int, default=3)
    ap.add_argument('--timeout', type=int, default=900)
    ap.add_argument('--variants', default='pad,toggle')
    args = ap.parse_args()
    jobs = [(v, s) for v in args.variants.split(',') for s in range(args.runs)]
    with ThreadPoolExecutor(args.jobs) as pool:
        results = list(pool.map(lambda j: run(args, *j), jobs))
    lines = []
    for name, ok, code, menus, faults, crash in results:
        lines.append(f'{name:12s} {"ok" if ok else "FAIL"} exit={code} menu-mode-entries={menus}'
                     + (f' faults: {" | ".join(faults[:3])}' if faults else '')
                     + (f' crash: {crash[0]}' if crash else ''))
    bad = sum(not r[1] for r in results)
    lines.append(f'{len(results) - bad}/{len(results)} stress runs ok')
    out = ROOT / 'build/menustress/report.txt'
    out.write_text('\n'.join(lines) + '\n')
    print('\n'.join(lines))
    return 0 if bad == 0 else 1


if __name__ == '__main__':
    raise SystemExit(main())
