#!/usr/bin/env python3
"""Measure paced idle level-up intervals and logic advances from a local fixture.

KHCOM_FAST must be absent, not '0': Pace checks environment presence.
Dummy SDL isolates CPU timing; it does not validate the user's GPU/vsync.
"""
import argparse
import csv
import datetime
import json
import os
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--build', default='build/codex-gameplay-us')
    ap.add_argument('--version', choices=('eu', 'us', 'jp'), default='us')
    ap.add_argument('--view', type=int, choices=(0, 4), default=4)
    ap.add_argument('--scale', type=int, choices=(1, 2, 4), default=2)
    args = ap.parse_args()
    work = ROOT / 'build/codex-gameplay-level-timing' / datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')
    work.mkdir(parents=True)
    shutil.copy2(ROOT / args.build / 'khcom.exe', work / 'khcom.exe')
    shutil.copy2(ROOT.parent / 'toolchain/SDL3-3.4.18/i686-w64-mingw32/bin/SDL3.dll', work / 'SDL3.dll')
    shutil.copytree(ROOT / 'port/assets/fonts', work / 'assets/fonts')
    (work / 'kh_com.ini').write_text(f'config_version=2\nview_mode={args.view}\nwindow_scale={args.scale}\nhd_text=1\ngame_speed=1\n')
    env = {k: v for k, v in os.environ.items() if not k.startswith(('KHCOM_', 'SDL_'))}
    env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy')
    rom = {'eu': 'B8CP', 'us': 'B8CE', 'jp': 'B8CJ'}[args.version]
    command = [str(work / 'khcom.exe'), '--rom', str(ROOT / 'roms' / (rom+'.gba')), '--language', 'es']
    fixture = dict(env, KHCOM_FAST='1', KHCOM_BATTLE='0:0:0', KHCOM_KEYS='60-63:A',
                   KHCOM_MODE='1400:levelup', KHCOM_SAVESTATE='1600:reward.khst', KHCOM_QUIT='1605')
    subprocess.run(command, cwd=work, env=fixture, check=True, timeout=180,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    log = (work / 'kh_com.log').read_text(errors='replace')
    if 'CRASH' in log or not (work / 'reward.khst').exists():
        raise RuntimeError('reward fixture failed')
    shutil.copy2(work / 'kh_com.log', work / 'fixture.log')
    paced = dict(env, KHCOM_LOADSTATE='reward.khst', KHCOM_QUIT='700', KHCOM_TIMING='1',
                 KHCOM_FRAMELOG=str(work / 'frames.csv'))
    result = subprocess.run(command, cwd=work, env=paced, timeout=60,
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    log = (work / 'kh_com.log').read_text(errors='replace')
    with (work / 'frames.csv').open() as f:
        rows = [r for r in csv.DictReader(f) if 100 <= int(r['frame']) <= 699]
    intervals = sorted(float(r['interval_ms']) for r in rows)
    logic = [int(r['logic']) for r in rows]
    steps = [b-a for a,b in zip(logic,logic[1:])]
    report = dict(version=args.version, view=args.view, scale=args.scale, exit=result.returncode,
                  paced_fast_environment_absent='KHCOM_FAST' not in paced, frames=len(rows),
                  logic_steps=sorted(set(steps)), expected_period_ms=1000/59.7275,
                  intervals_ms={label: intervals[int((len(intervals)-1)*q)] for label,q in
                                (('p50',.5),('p95',.95),('p99',.99),('max',1))},
                  over_two_periods=sum(t>2000/59.7275 for t in intervals),
                  renderer='SDL dummy software, CPU timing; excludes GPU/vsync')
    report['passed'] = result.returncode == 0 and 'CRASH' not in log and len(rows)==600 and set(steps)=={1} and report['intervals_ms']['p99']<2000/59.7275
    (work / 'report.json').write_text(json.dumps(report,indent=2))
    print(work)
    print(json.dumps(report,indent=2))
    return 0 if report['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
