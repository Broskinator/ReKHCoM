#!/usr/bin/env python3
"""Open submenus with actual START/DOWN/A input, assert entry and field return.

Only initial floor/world setup uses a MAP fixture. Menu transitions use keys.
"""
import argparse
import datetime
import json
import os
from pathlib import Path
import re
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
MENUS = ('Mode_Deck', 'mode_allmap', 'mode_mapinspect', 'mode_worldinspect', 'mode_status', 'mode_jiminy')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--build', default='build/codex-gameplay-us')
    ap.add_argument('--version', choices=('eu', 'us', 'jp'), default='us')
    ap.add_argument('--cycles', type=int, default=3)
    ap.add_argument('--actor', choices=('sora', 'riku'), default='sora')
    args = ap.parse_args()
    if args.cycles < 1:
        ap.error('cycles must be positive')
    work = ROOT / 'build/codex-gameplay-menus' / datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')
    work.mkdir(parents=True)
    shutil.copy2(ROOT / args.build / 'khcom.exe', work / 'khcom.exe')
    shutil.copy2(ROOT.parent / 'toolchain/SDL3-3.4.18/i686-w64-mingw32/bin/SDL3.dll', work / 'SDL3.dll')
    shutil.copytree(ROOT / 'port/assets/fonts', work / 'assets/fonts')
    (work / 'kh_com.ini').write_text('config_version=2\nview_mode=4\nwindow_scale=2\nhd_text=1\n')
    cases, keys, fixtures, shots = [], [], [], []
    for cycle in range(args.cycles):
        for index, name in enumerate(MENUS):
            if args.actor == 'riku' and index in (3, 5):
                continue  # Disabled options in Reverse/Rebirth, tested separately as disabled.
            start = 60 + len(cases) * 1100
            fixtures.append(f"{start}:{'roomriku' if args.actor == 'riku' else 'room'}:10901")
            sequence = [(180, 'START')] + [(280 + d*20, 'DOWN') for d in range(index)]
            sequence += [(420, 'A'), (560, 'RIGHT'), (590, 'LEFT'), (620, 'R'), (650, 'L'),
                         (730, 'START'), (910, 'B')]
            keys.extend(f'{start+f}-{start+f+2}:{button}' for f, button in sequence)
            shots.extend((start+690, start+1060))
            cases.append(dict(start=start, end=start+1099, name=name, cycle=cycle))
    env = {k: v for k, v in os.environ.items() if not k.startswith(('KHCOM_', 'SDL_'))}
    env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1', KHCOM_TRACE_MODES='1',
               KHCOM_KEYS=','.join(keys), KHCOM_MODE=','.join(fixtures),
               KHCOM_OUTPUT_SHOT=','.join(map(str, shots)), KHCOM_QUIT=str(cases[-1]['end']))
    rom = {'eu': 'B8CP', 'us': 'B8CE', 'jp': 'B8CJ'}[args.version]
    result = subprocess.run([str(work / 'khcom.exe'), '--rom', str(ROOT / 'roms' / (rom+'.gba')),
                             '--language', 'es'], cwd=work, env=env, timeout=1800,
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    log = (work / 'kh_com.log').read_text(errors='replace')
    trace = [(int(f), name) for f, name in re.findall(r'harness: frame (\d+) mode (\S+)', log)]
    for case in cases:
        seen = [(f, name) for f, name in trace if case['start'] <= f <= case['end']]
        entry = [f for f, name in seen if name == case['name']]
        returned = any(name == 'Mode_MapFld' and f > entry[-1] for f, name in seen) if entry else False
        case.update(passed=result.returncode == 0 and 'CRASH' not in log and bool(entry) and returned,
                    trace=seen)
    report = dict(version=args.version, actor=args.actor, exit=result.returncode, cases=cases,
                  passed=sum(c['passed'] for c in cases), total=len(cases), scope='natural submenu keys from MAP fixture')
    (work / 'report.json').write_text(json.dumps(report, indent=2))
    print(work)
    print(f"{report['passed']}/{report['total']} submenu cycles passed")
    return 0 if all(c['passed'] for c in cases) else 1


if __name__ == '__main__':
    raise SystemExit(main())
