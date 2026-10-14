"""Compare HD/native player HP through original damage and healing interpolation."""
import os
from pathlib import Path
import shutil
import subprocess
from datetime import datetime
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
work = ROOT / 'build/hud-hp-motion' / datetime.now().strftime('%Y%m%d-%H%M%S')
work.mkdir(parents=True)
frames = (300, 362, 430, 562, 590, 650)
for actor in (0, 1):
    captures = {}
    for hd in (0, 1):
        folder = work / f'actor{actor}-hd{hd}'
        folder.mkdir()
        shutil.copytree(ROOT / 'port/assets/hd-hud', folder / 'assets/hd-hud', ignore=shutil.ignore_patterns('drafts'))
        (folder / 'kh_com.ini').write_text(f'config_version=2\nview_mode=4\nwindow_scale=4\nhd_text=0\nhd_hud={hd}\n')
        env = {k: v for k, v in os.environ.items() if not k.startswith('KHCOM_')}
        env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1',
                   KHCOM_BATTLE=f'0:0:{actor}', KHCOM_KEYS='60-63:A',
                   KHCOM_HUD_FULLHP_TRACE='300,430',
                   KHCOM_MODE='250:hudhp:100,350:hudhp:50,400:hudhp:25,550:hudhp:100',
                   KHCOM_OUTPUT_SHOT=','.join(map(str, frames)), KHCOM_QUIT='655')
        subprocess.run([str(ROOT / 'build/cx-eu/khcom.exe'), '--data-dir', str(folder),
                        '--rom', str(ROOT / 'roms/B8CP.gba'), '--language', 'en'],
                       cwd=folder, env=env, check=True, timeout=90)
        log = (folder / 'kh_com.log').read_text(errors='replace')
        assert log.count('harness: hudhp') == 4 and 'CRASH' not in log
        if hd:
            assert 'fullhp frame=300 value=1' in log
            assert 'fullhp frame=430 value=0' in log
        masks = []
        for frame in frames:
            with Image.open(folder / f'out_{frame:05d}.bmp') as im:
                rgb = im.convert('RGB')
                pix = rgb.load()
                mask = {(x, y) for y in range(rgb.height // 10)
                        for x in range(rgb.width // 8, rgb.width * 3 // 8)
                        if (pix[x, y][1] - pix[x, y][0] > 35 and pix[x, y][1] - pix[x, y][2] > 35)
                        or (pix[x, y][2] > 120 and pix[x, y][2] - pix[x, y][0] > 35
                            and pix[x, y][2] - pix[x, y][1] > 35)}
            assert mask, (actor, hd, frame)
            masks.append(mask)
        captures[hd] = masks
    assert captures[0] == captures[1], f'HP geometry differs for actor {actor}'
    counts = [len(mask) for mask in captures[0]]
    assert len(set(counts)) >= 3 and counts[2] < counts[0] and counts[-1] > counts[2], counts
    print('PASS', 'Riku' if actor else 'Sora', 'native/HD HP motion', counts)
print(work)
