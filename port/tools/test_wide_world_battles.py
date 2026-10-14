"""Capture each authored arena at rest and opposite camera edges, serially.

Exit zero proves fixture execution only. Review every captured arena before
claiming completion. Output and configuration are isolated beneath build/.
"""
import argparse
import datetime
import json
import os
from pathlib import Path
import shutil
import subprocess

from test_battles import battle_entries, stage_table

ROOT = Path(__file__).resolve().parents[2]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--build', required=True)
    ap.add_argument('--version', choices=('eu', 'us', 'jp'), default='eu')
    ap.add_argument('--only', help='comma-separated arena labels')
    ap.add_argument('--riku', action='store_true')
    args = ap.parse_args()
    target = ROOT / 'build/codex-wide-battles' / datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')
    target.mkdir(parents=True)
    entries = battle_entries()
    stages = stage_table()
    cases = [(s.removeprefix('BATTLE_STAGE_'), 0, i) for i, s in enumerate(stages)
             if s != 'BATTLE_STAGE_GARDEN']
    special = {'Garden': 'BATTLE_CARD_SOLDIERS', 'Hades': 'BATTLE_HADES', 'Hook': 'BATTLE_HOOK',
               'Mansion': 'BATTLE_VEXEN_2', 'Ansem2': 'BATTLE_ANSEM_2',
               'Marluxia': 'BATTLE_MARLUXIA', 'Hum': 'BATTLE_AXEL_1'}
    for label, battle in special.items():
        hits = [i for i, _, b in entries if b == battle]
        if not hits:
            raise ValueError('debug menu has no ' + battle)
        cases.append((label, hits[0], 0))
    if args.only:
        cases = [case for case in cases if case[0] in args.only.split(',')]
    results = []
    for label, battle, stage in cases:
        directory = target / label
        directory.mkdir()
        for name in ('khcom.exe', 'SDL3.dll'):
            shutil.copy2(ROOT / args.build / name, directory / name)
        (directory / 'kh_com.ini').write_text('config_version=2\nview_mode=4\nwindow_scale=4\nhd_text=0\n')
        env = dict(os.environ, SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1',
                   KHCOM_BATTLE=f'{battle}:{stage}:{int(args.riku)}',
                   KHCOM_KEYS='60-63:A,200-380:LEFT,400-600:RIGHT',
                   KHCOM_OUTPUT_SHOT='180,380,600', KHCOM_QUIT='610')
        rom = {'eu': 'B8CP', 'us': 'B8CE', 'jp': 'B8CJ'}[args.version]
        result = subprocess.run([str(directory / 'khcom.exe'), '--rom', str(ROOT / 'roms' / (rom + '.gba'))],
                                cwd=directory, env=env, timeout=180, stdout=subprocess.DEVNULL,
                                stderr=subprocess.DEVNULL)
        log = (directory / 'kh_com.log').read_text(errors='replace')
        passed = result.returncode == 0 and 'CRASH' not in log and all(
            (directory / f'out_{frame:05d}.bmp').exists() for frame in (180, 380, 600))
        results.append({'arena': label, 'battle': battle, 'stage': stage, 'exit': result.returncode,
                        'fixture_passed': passed, 'visual_review': 'required'})
        (target / 'report.json').write_text(json.dumps(results, indent=2) + '\n')
        print(label, 'captured' if passed else 'FAIL', flush=True)
    print(target)
    return 0 if results and all(case['fixture_passed'] for case in results) else 1


if __name__ == '__main__':
    raise SystemExit(main())
