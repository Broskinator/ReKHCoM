"""Official boss achievements, offline: each boss rematch is fought and won in the
port (EU) while KHCOM_RA_DRYRUN evaluates the 90 official definitions; the
triggers per boss are reported, with a control run that loses no time and never
wins (nothing may trigger there). Needs build/ra-reference-audit's catalog.

    python port/tools/ra_boss_dryrun.py [rows...]
"""
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DEFS = ROOT / 'build/ra-reference-audit/references.csv.definitions.json'
OUT = ROOT / 'build/ra-boss-dryrun'
OUT.mkdir(parents=True, exist_ok=True)
NAMES = ['Guard Armor', 'Trickmaster', 'Jafar', 'Hades', 'Ursula', 'Parasite Cage', 'Oogie Boogie', 'Hook',
         'Darkside', 'Dragon Maleficent', 'Riku', 'Vexen', 'Tutorial 1', 'Tutorial 2']
rows = [int(r) for r in sys.argv[1:]] or list(range(14))
obtain = ','.join(f'1100:obtain:{k}' for k in range(30, 60))


def run(row, win):
    d = OUT / f'{row:02d}{"-win" if win else "-control"}'
    d.mkdir(exist_ok=True)
    for f in ('khcom.exe', 'SDL3.dll'):
        shutil.copy2(ROOT / 'build/pc-eu' / f, d / f)
    (d / 'kh_com.log').unlink(missing_ok=True)
    wins = ',' + ','.join(f'{f}:bosskill' for f in range(2200, 4400, 200)) if win else ''
    env = dict(os.environ, SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1',
               KHCOM_RA_DRYRUN=str(DEFS), KHCOM_TRACE_MODES='1',
               KHCOM_MODE=f'300:room:10900,302:roomenemies:0,{obtain},1300:rematch:{row}{wins}', KHCOM_QUIT='5200')
    subprocess.run([str(d / 'khcom.exe'), '--language', 'es', '--rom', str(ROOT / 'roms/B8CP.gba'),
                    '--data-dir', str(d)], cwd=d, env=env, timeout=900,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    log = (d / 'kh_com.log').read_text(errors='replace')
    trig = re.findall(r'ra-dryrun: trigger (\d+) frame (\d+)', log)
    modes = re.findall(r'harness: frame (\d+) mode (\S+)', log)
    return trig, modes, 'ra-dryrun:' in log


report = {}
for row in rows:
    t_win, modes, active = run(row, True)
    t_ctl, _, _ = run(row, False)
    report[NAMES[row]] = dict(win=t_win, control=t_ctl, battle='mode_battle' in ' '.join(m for _, m in modes), dryrun=active)
    print(f'{NAMES[row]:18s} win -> {[a for a, _ in t_win]}   control -> {[a for a, _ in t_ctl]}   '
          f'battle={"mode_battle" in " ".join(m for _, m in modes)}')
(OUT / 'report.json').write_text(json.dumps(report, indent=1))
