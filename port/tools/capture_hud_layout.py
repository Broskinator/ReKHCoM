"""Capture native HUD OBJ boxes for both campaigns without changing user data."""
import json
import os
from pathlib import Path
import re
import subprocess
from datetime import datetime

ROOT = Path(__file__).resolve().parents[2]
work = ROOT / 'build/hud-layout' / datetime.now().strftime('%Y%m%d-%H%M%S')
work.mkdir(parents=True)
for actor, riku in (('sora', 0), ('riku', 1)):
    folder = work / actor
    folder.mkdir()
    (folder / 'kh_com.ini').write_text('config_version=2\nview_mode=4\nwindow_scale=4\nhd_text=0\nhd_hud=0\n')
    env = {k: v for k, v in os.environ.items() if not k.startswith('KHCOM_')}
    env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1',
               KHCOM_BATTLE=f'0:0:{riku}', KHCOM_KEYS='60-63:A', KHCOM_HUD_LAYOUT='380',
               KHCOM_OUTPUT_SHOT='380', KHCOM_QUIT='385')
    subprocess.run([str(ROOT / 'build/cx-eu/khcom.exe'), '--data-dir', str(folder),
                    '--rom', str(ROOT / 'roms/B8CP.gba'), '--language', 'en'],
                   cwd=folder, env=env, check=True, timeout=90)
    log = (folder / 'kh_com.log').read_text(errors='replace')
    boxes = [dict((k, int(v)) for k, v in re.findall(r'(\w+)=(-?\d+)', line))
             for line in log.splitlines() if line.startswith('hud-layout:')]
    assert boxes, f'No actual HUD geometry captured for {actor}'
    (folder / 'layout.json').write_text(json.dumps({'actor': actor, 'frame': 380, 'objects': boxes}, indent=2))
    print(actor, len(boxes), 'native objects')
print(work)
