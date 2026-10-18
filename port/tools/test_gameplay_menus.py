#!/usr/bin/env python3
"""Exercise one gameplay submenu per fresh process with START/DOWN/A input.

Each process gets only the initial MAP fixture. Isolating menus avoids carrying
random encounters, stale game state, or forced room changes between cases.
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
    root_work = ROOT / 'build/codex-gameplay-menus' / datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')
    root_work.mkdir(parents=True)
    cases = []
    for cycle in range(args.cycles):
        for index, name in enumerate(MENUS):
            start = 60
            sequence = [(180, 'START')] + [(280 + d*20, 'DOWN') for d in range(index)]
            sequence += [(420, 'A'), (560, 'RIGHT'), (590, 'LEFT'), (620, 'R'), (650, 'L'),
                         (730, 'START')]
            end = 850
            # Riku's first deck visit is a tutorial with multiple pages. Advance
            # each message, inspect the fixed-deck notice, then back out.
            if args.actor == 'riku' and index == 0:
                sequence = [(180, 'START'), (420, 'A'), (600, 'A'),
                            (720, 'A'), (840, 'A'), (960, 'A'), (1010, 'B')]
                end = 1200
            cases.append(dict(start=start, end=end, name=name, cycle=cycle,
                              keys=','.join(f'{start+f}-{start+f+2}:{button}' for f, button in sequence)))
    rom = {'eu': 'B8CP', 'us': 'B8CE', 'jp': 'B8CJ'}[args.version]
    dll = ROOT.parent / 'toolchain/SDL3-3.4.18/i686-w64-mingw32/bin/SDL3.dll'
    for case in cases:
        work = root_work / f"cycle-{case['cycle']}-{case['name']}"
        work.mkdir()
        shutil.copy2(ROOT / args.build / 'khcom.exe', work / 'khcom.exe')
        shutil.copy2(dll, work / 'SDL3.dll')
        shutil.copytree(ROOT / 'port/assets/fonts', work / 'assets/fonts')
        (work / 'kh_com.ini').write_text('config_version=2\nview_mode=4\nwindow_scale=2\nhd_text=1\n')
        env = {k: v for k, v in os.environ.items() if not k.startswith(('KHCOM_', 'SDL_'))}
        env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1',
                   KHCOM_TRACE_MODES='1', KHCOM_KEYS=case['keys'],
                   KHCOM_MODE=f"60:{'roomriku' if args.actor == 'riku' else 'room'}:10901,120:fieldfreeze",
                   KHCOM_OUTPUT_SHOT='690,830', KHCOM_QUIT=str(case['end']))
        result = subprocess.run([str(work / 'khcom.exe'), '--rom', str(ROOT / 'roms' / (rom+'.gba')),
                                 '--language', 'es'], cwd=work, env=env, timeout=1800,
                                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        log = (work / 'kh_com.log').read_text(errors='replace')
        trace = [(int(f), name) for f, name in re.findall(r'harness: frame (\d+) mode (\S+)', log)]
        seen = [(f, name) for f, name in trace if case['start'] <= f <= case['end']]
        entry = [f for f, name in seen if name == case['name']]
        returned = any(name == 'Mode_MapFld' and f > entry[-1] for f, name in seen) if entry else False
        case.update(passed=result.returncode == 0 and 'CRASH' not in log and bool(entry) and returned,
                    exit=result.returncode, trace=seen, directory=str(work))
        print(f"{case['name']} cycle {case['cycle']}: {'PASS' if case['passed'] else 'FAIL'}", flush=True)
    report = dict(version=args.version, actor=args.actor, cases=cases,
                  passed=sum(c['passed'] for c in cases), total=len(cases),
                  scope='one natural submenu path per fresh process; initial room and enemy-freeze fixtures only')
    (root_work / 'report.json').write_text(json.dumps(report, indent=2))
    print(root_work)
    print(f"{report['passed']}/{report['total']} submenu cycles passed")
    return 0 if all(c['passed'] for c in cases) else 1


if __name__ == '__main__':
    raise SystemExit(main())
