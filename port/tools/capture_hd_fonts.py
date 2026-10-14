"""Capture the actual menu/dialogue/subtitle faces in an isolated data folder."""
import os
from pathlib import Path
import subprocess
from datetime import datetime

ROOT = Path(__file__).resolve().parents[2]
import argparse
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--scenes', nargs='+', default=['deck', 'dialogue', 'subtitles'])
args = parser.parse_args()
work = ROOT / 'build/font-review' / datetime.now().strftime('%Y%m%d-%H%M%S')
work.mkdir(parents=True)
exe = ROOT / 'build/cx-eu/khcom.exe'
for name, fixture, shots, end in (
    ('deck', {'KHCOM_MODE': '300:room:10000,600:Mode_Deck'}, '800', 810),
    ('status', {'KHCOM_MODE': '300:room:10000,600:mode_status'}, '800', 810),
    ('journal', {'KHCOM_MODE': '300:room:10000,600:mode_jiminy'}, '800', 810),
    ('worldcards', {'KHCOM_MODE': '300:room:10000,600:mode_worldinspect'}, '800', 810),
    ('dialogue', {'KHCOM_MODE': '300:room:10000,600:mode_ms_top'}, '800,1000,1500', 1510),
    ('subtitles', {'KHCOM_MOVIE': '1'}, '2700,2770,3330', 3340),
):
    if name not in args.scenes:
        continue
    data = work / name
    data.mkdir()
    # Use the package's bin/ layout with root assets via --data-dir.
    import shutil
    shutil.copytree(ROOT / 'port/assets/fonts', data / 'assets/fonts')
    (data / 'kh_com.ini').write_text('config_version=2\nview_mode=4\nwindow_scale=4\nhd_text=1\nhd_hud=0\n')
    env = {k: v for k, v in os.environ.items() if not k.startswith('KHCOM_')}
    env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1',
               KHCOM_OUTPUT_SHOT=shots, KHCOM_QUIT=str(end), **fixture)
    subprocess.run([str(exe), '--data-dir', str(data), '--rom', str(ROOT / 'roms/B8CP.gba'),
                    '--language', 'en'], cwd=data, env=env, check=True, timeout=120)
    log = (data / 'kh_com.log').read_text(errors='replace')
    selected = [line for line in log.splitlines() if 'HD text:' in line]
    print(name, '\n'.join(selected))
print(work)
