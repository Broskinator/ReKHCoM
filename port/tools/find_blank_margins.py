"""Find event scenes whose wide margins still show a flat white or black strip.

Runs every story event (Sora and Riku menus) hidden at 1920x1080, zoom x3, with
A pressed regularly, takes window shots and flags a shot when a band at the
left or right edge is one flat white/black colour over most of its height while
the margin next to it shows something else (the room's void or scenery).
Output: build/blank-margins/report.txt and a contact sheet of flagged shots.

    python port/tools/find_blank_margins.py [--build build/pc-eu] [--jobs 4]
"""
import argparse
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'port/tools'))
from test_events import event_lists  # noqa: E402

SHOTS = (700, 1400, 2100, 2800)


def strip_flag(im):
    """(side, colour) of a flat edge band beside a non-flat margin, or None."""
    w, h = im.size
    px = im.load()
    for side, xs in (('left', range(0, 40)), ('right', range(w - 40, w))):
        cols = list(xs)
        colour = px[cols[0], h // 2]
        if colour not in ((0, 0, 0), (255, 255, 255)):
            continue
        same = sum(1 for y in range(0, h, 8) for x in cols[::8] if px[x, y] == colour)
        total = len(range(0, h, 8)) * len(cols[::8])
        if same < total * 0.8:
            continue
        # the band must stop somewhere before the GBA picture: margin beside it differs
        inner = range(200, 560) if side == 'left' else range(w - 560, w - 200)
        other = sum(1 for y in range(0, h, 16) for x in inner[::16] if px[x, y] != colour)
        if other > len(range(0, h, 16)) * len(inner[::16]) * 0.3:
            return side, colour
    return None


def run(build, work, case):
    list_id, index, name = case
    d = work / f'{list_id}_{index:03d}'
    shutil.rmtree(d, ignore_errors=True)
    d.mkdir(parents=True)
    for f in ('khcom.exe', 'SDL3.dll'):
        shutil.copy2(build / f, d / f)
    (d / 'kh_com.ini').write_text('config_version=11\nzoom=6\n')
    keys = ','.join(f'{f}-{f + 2}:A' for f in range(240, SHOTS[-1], 40))
    env = {k: v for k, v in __import__('os').environ.items() if not k.startswith(('KHCOM_', 'SDL_'))}
    env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1', KHCOM_WINDOW='1920x1080',
               KHCOM_EVENT=f'{list_id}:{index}', KHCOM_KEYS=keys,
               KHCOM_OUTPUT_SHOT=','.join(map(str, SHOTS)), KHCOM_QUIT=str(SHOTS[-1] + 5))
    try:
        subprocess.run([str(d / 'khcom.exe'), '--language', 'es', '--rom', str(ROOT / 'roms/B8CP.gba')],
                       cwd=d, env=env, timeout=240, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except subprocess.TimeoutExpired:
        return case, 'timeout', []
    flagged = []
    for f in SHOTS:
        shot = d / f'out_{f:05d}.bmp'
        if shot.exists():
            im = Image.open(shot).convert('RGB')
            flag = strip_flag(im)
            if flag:
                im.resize((480, 270)).save(d / f'flag_{f}.png')
                flagged.append((f, flag))
        shot.unlink(missing_ok=True)
    return case, 'ok', flagged


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--build', type=Path, default=ROOT / 'build/pc-eu')
    p.add_argument('--jobs', type=int, default=4)
    p.add_argument('--only', help='list:index,... to limit the run')
    args = p.parse_args()
    work = ROOT / 'build/blank-margins'
    work.mkdir(parents=True, exist_ok=True)
    cases = event_lists()
    if args.only:
        wanted = {tuple(map(int, c.split(':'))) for c in args.only.split(',')}
        cases = [c for c in cases if (c[0], c[1]) in wanted]
    lines, sheet = [], []
    with ThreadPoolExecutor(args.jobs) as pool:
        for case, status, flagged in pool.map(lambda c: run(args.build, work, c), cases):
            line = f'{case[0]}:{case[1]:03d} {case[2]} {status} ' + ' '.join(f'{f}:{s}:{c}' for f, (s, c) in flagged)
            lines.append(line)
            print(line, flush=True)
            for f, _ in flagged:
                sheet.append(work / f'{case[0]}_{case[1]:03d}' / f'flag_{f}.png')
    (work / 'report.txt').write_text('\n'.join(lines) + '\n', encoding='utf-8')
    if sheet:
        cols = 4
        img = Image.new('RGB', (480 * cols, 270 * ((len(sheet) + cols - 1) // cols)), 'gray')
        for i, f in enumerate(sheet):
            img.paste(Image.open(f), ((i % cols) * 480, (i // cols) * 270))
        img.save(work / 'flagged.png')
    print(f'{sum(1 for l in lines if l.count(":") > 2)} events flagged of {len(lines)}')


if __name__ == '__main__':
    main()
