"""Render real preview toasts at output resolutions, without granting awards.

Uses the original ROM privately, fresh data directories, SDL's software backend,
and a room fixture. This validates presentation, not approval of map artwork.
"""
from pathlib import Path
import json
import os
import shutil
import subprocess
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'build/achievement-presentation'
OUT.mkdir(parents=True, exist_ok=True)
exe = ROOT / 'build/ra-eu/khcom.exe'
rom = ROOT / 'roms/B8CP.gba'
ffmpeg = shutil.which('ffmpeg')
report = []
for width, height in [(240, 160), (640, 360), (1920, 1080), (2560, 1080)]:
    work = OUT / f'{width}x{height}'
    work.mkdir(exist_ok=True)
    (work / 'kh_com.ini').write_text('config_version=11\nview_mode=4\nzoom=6\n'
        'scale_mode=0\ninteger_zoom=1\nwindow_scale=1\nfullscreen=0\nvsync=0\n', encoding='ascii')
    env = {k: v for k, v in os.environ.items() if not k.startswith(('KHCOM_', 'SDL_'))}
    env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_WINDOW=f'{width}x{height}',
               KHCOM_MODE='60:room:10901,120:fieldfreeze', KHCOM_ACHIEVEMENT_PREVIEW='180',
               KHCOM_OUTPUT_SHOT='175,186,200,260,470', KHCOM_QUIT='475',
               KHCOM_FAST='1', KHCOM_TOAST_FRAMECLOCK='1')
    if width == 1920 and ffmpeg:
        env.update(KHCOM_RECORD='170-470:toast.mov', KHCOM_FFMPEG=ffmpeg)
    subprocess.run([str(exe), '--rom', str(rom), '--data-dir', str(work), '--language', 'es'],
                   cwd=work, env=env, check=True, timeout=120)
    assert not list(work.glob('achievements_*.dat')), 'A preview must never grant an achievement'
    for frame in [175, 186, 200, 260, 470]:
        image = Image.open(work / f'out_{frame:05d}.bmp').convert('RGB')
        assert image.size == (width, height)
        image.save(work / f'out_{frame:05d}.png')
    image = Image.open(work / 'out_00260.png').convert('RGB')
    mask = Image.new('L', image.size)
    mask.putdata([255 if p == (252, 251, 251) else 0 for p in image.get_flattened_data()])
    box = mask.getbbox()
    assert box, 'No toast panel found at the holding phase'
    left, top, right, bottom = box
    assert abs((left + right) / 2 - width / 2) <= 3, (width, box)
    assert top >= 0 and bottom <= height and left > 0 and right < width, (width, box)
    report.append(dict(resolution=[width, height], holding_panel_bounds=box, status='PASS'))
    if width == 1920 and ffmpeg:
        subprocess.run([ffmpeg, '-y', '-loglevel', 'error', '-i', str(work / 'toast.mov'),
                        '-an', '-c:v', 'libx264', '-crf', '19', '-pix_fmt', 'yuv420p',
                        '-movflags', '+faststart', str(work / 'toast.mp4')], check=True)
(OUT / 'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
print('PASS real toast presentation at four output sizes; preview did not grant achievements')
