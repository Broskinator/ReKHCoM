"""Compare accepted event profiles with the same executable and native pixels.

Hidden, isolated runs; captures and saves are private under build/. A passing
fixture proves pixel isolation and execution, not user acceptance of new art.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import json
import os
from pathlib import Path
import shutil
import subprocess
from PIL import Image, ImageChops
from test_events import event_lists

ROOT = Path(__file__).resolve().parents[2]
FRAMES = (100, 180, 300)


def run(case, args, root):
    region, event, aspect, original = case
    label = f'{region}-{event:03d}-{aspect}-{int(original)}'
    entry = next((l, i) for l, i, name in event_lists() if name.startswith(f'EVENT_{event:03d}_'))
    images = []
    for enabled in (False, True):
        d = root / label / ('after' if enabled else 'before')
        d.mkdir(parents=True)
        for name in ('khcom.exe', 'SDL3.dll'):
            shutil.copy2(ROOT / (args.build_prefix + region) / name, d / name)
        width, height, zoom = (960, 540, 4) if aspect == '16x9' else (960, 270, 2)
        (d / 'kh_com.ini').write_text(f'config_version=11\nview_mode={0 if original else 4}\nzoom={zoom}\nhd_text=0\nvsync=0\n')
        env = dict(os.environ, SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1',
                   KHCOM_WINDOW=f'{width}x{height}', KHCOM_EVENT=f'{entry[0]}:{entry[1]}',
                   KHCOM_SHOTS=','.join(map(str, FRAMES)), KHCOM_OUTPUT_SHOT=','.join(map(str, FRAMES)),
                   KHCOM_QUIT='310', KHCOM_KEYS='60-63:A')
        env.pop('KHCOM_NO_SCENE_PROFILES', None)
        if not enabled:
            env['KHCOM_NO_SCENE_PROFILES'] = '1'
        code = {'eu': 'B8CP', 'us': 'B8CE', 'jp': 'B8CJ'}[region]
        process = subprocess.run([str(d / 'khcom.exe'), '--rom', str(ROOT / 'roms' / (code + '.gba'))],
                                 cwd=d, env=env, timeout=180, capture_output=True)
        if process.returncode or 'CRASH' in (d / 'kh_com.log').read_text(errors='replace'):
            raise RuntimeError(f'{label}: process failed {process.returncode}')
        frames = []
        for frame in FRAMES:
            with Image.open(d / f'shot_{frame:05d}.bmp') as image:
                frames.append(image.convert('RGB'))
            with Image.open(d / f'out_{frame:05d}.bmp') as image:
                image.save(d / f'out_{frame:05d}.png')
        images.append(frames)
    native, changed = 0, 0
    for before, after in zip(*images):
        if before.size != after.size:
            raise RuntimeError(label + ': viewport changed')
        w, h = before.size
        box = ((w - 240) // 2, (h - 160) // 2, (w + 240) // 2, (h + 160) // 2)
        native += ImageChops.difference(before.crop(box), after.crop(box)).getbbox() is not None
        changed += ImageChops.difference(before, after).getbbox() is not None
    want = not original and event in (57, 61, 74)
    passed = native == 0 and (changed > 0 if want else changed == 0)
    result = dict(case=label, passed=passed, native_changed_frames=native, changed_frames=changed)
    print(('PASS ' if passed else 'FAIL ') + json.dumps(result), flush=True)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-prefix', default='build/codex-evidence/guard-patio-')
    parser.add_argument('--out', default='build/codex-evidence/accepted-scene-tests')
    parser.add_argument('--regions', nargs='+', choices=('eu', 'us', 'jp'), default=['eu', 'us', 'jp'])
    args = parser.parse_args()
    root = (ROOT / args.out).resolve()
    if not root.is_relative_to(ROOT / 'build'):
        parser.error('output must be under build/')
    root.mkdir(parents=True, exist_ok=False)
    cases = [(region, event, aspect, False) for region in args.regions for event in (57, 61, 74)
             for aspect in ('16x9', '32x9')]
    cases += [('eu', event, '16x9', True) for event in (57, 61, 74)]
    cases += [('eu', 4, aspect, False) for aspect in ('16x9', '32x9')]
    with ThreadPoolExecutor(max_workers=2) as pool:
        results = list(pool.map(lambda case: run(case, args, root), cases))
    (root / 'report.json').write_text(json.dumps(results, indent=2) + '\n')
    print(f'{sum(r["passed"] for r in results)}/{len(results)} passed', flush=True)
    return 0 if all(r['passed'] for r in results) else 1


if __name__ == '__main__':
    raise SystemExit(main())
