#!/usr/bin/env python3
"""Capture the real level-up task after closing an isolated debug battle.

Debug stats are normalized by KHCOM_MODE=levelup to leave selectable options.
This fixture supplements, rather than replaces, natural enemy/EXP navigation.
Screenshots require visual review; exit zero alone does not verify rendering.
"""
import argparse
import datetime
import os
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--version', choices=('eu', 'us', 'jp'), default='eu')
    ap.add_argument('--build', default='build/pcpriority')
    ap.add_argument('--view', type=int, default=4)
    ap.add_argument('--entry', action='store_true', help='capture battle entry instead of the reward screen')
    args = ap.parse_args()
    target = ROOT / 'build/levelup-regressions' / datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')
    target.mkdir(parents=True)
    for name in ('khcom.exe', 'SDL3.dll'):
        shutil.copy2(ROOT / args.build / name, target / name)
    shutil.copytree(ROOT / 'port/assets/fonts', target / 'assets/fonts')
    (target / 'kh_com.ini').write_text(f'config_version=2\nview_mode={args.view}\nwindow_scale=2\n')
    frames = tuple(range(80, 161, 10)) if args.entry else (1410, 1440, 1500, 1600, 1800, 1820, 1850, 1900, 2000)
    env = dict(os.environ, SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1',
               KHCOM_BATTLE='0:0:0', KHCOM_KEYS='60-63:A,1800-1803:A', KHCOM_MODE='1400:levelup',
               KHCOM_OUTPUT_SHOT=','.join(map(str, frames)), KHCOM_QUIT='2050')
    if args.entry:
        env.update(KHCOM_KEYS='60-63:A', KHCOM_MODE='', KHCOM_QUIT='165')
    rom = {'eu': 'B8CP', 'us': 'B8CE', 'jp': 'B8CJ'}[args.version]
    result = subprocess.run([str(target / 'khcom.exe'), '--rom', str(ROOT / 'roms' / (rom + '.gba')),
                             '--language', 'es'], cwd=target, env=env, timeout=300,
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    log = (target / 'kh_com.log').read_text(errors='replace')
    passed = result.returncode == 0 and 'CRASH' not in log and all(
        (target / f'out_{f:05d}.bmp').exists() for f in frames)
    (target / 'report.txt').write_text(f'exit={result.returncode}; captures={passed}; visual review required\n')
    print(target, 'capture PASS' if passed else 'FAIL')
    return 0 if passed else 1

if __name__ == '__main__':
    raise SystemExit(main())
