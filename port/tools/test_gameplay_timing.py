#!/usr/bin/env python3
"""Measure paced gameplay after the opening, with a real SDL renderer."""
import os
import re
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
folder = ROOT / 'build' / 'gameplay-timing'
folder.mkdir(parents=True, exist_ok=True)
for name in ('khcom.exe', 'SDL3.dll'):
    shutil.copyfile(ROOT / 'build/pc' / name, folder / name)
(folder / 'kh_com.ini').write_text('config_version=2\nview_mode=4\ngame_speed=1\nwindow_scale=4\nvsync=1\n')
env = {k: v for k, v in os.environ.items() if not k.startswith(('KHCOM_', 'SDL_'))}
# Save after the movie returns to the main loop; never touch player saves.
fixture = dict(env, SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy',
               KHCOM_FAST='1', KHCOM_QUIT='7200', KHCOM_SAVESTATE='7000:gameplay.khst')
subprocess.run([str(folder / 'khcom.exe'), '--intro', '--language', 'es'],
               cwd=folder, env=fixture, check=True, timeout=60)
assert (folder / 'gameplay.khst').exists(), 'Post-movie state missing'
runenv = dict(env, KHCOM_LOADSTATE='gameplay.khst', KHCOM_QUIT='900', KHCOM_TIMING='1',
              KHCOM_SHOTS='600')
subprocess.run([str(folder / 'khcom.exe')], cwd=folder, env=runenv, check=True, timeout=40)
log = (folder / 'kh_com.log').read_text()
rates = [float(x) for x in re.findall(r'\(([\d.]+) Hz\)', log)]
assert rates and min(rates) > 55, log
print(log)
print('PASS: paced smooth gameplay stays above 55 Hz after the opening.')
