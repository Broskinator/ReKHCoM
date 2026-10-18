"""Compare parked/sliding dialogue portraits with the old wide-view behaviour.

Fresh isolated native runs of Riku's opening and Sora/Leon's scene, EU/US/JP.
The opt-out is a harness comparison only. Original 240x160 frames must remain
identical; wide comparisons record exact changed boxes, not a full-playthrough claim.
"""
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path
import json
import os
import subprocess
from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'build/portrait59-regression'
SHOTS = [300, 400, 600, 700, 900, 1000, 1100, 1200]


def run_case(region, actor, wide):
    case = OUT / f'{region}-{actor}-{"wide" if wide else "gba"}'
    rom = {'eu': 'B8CP', 'us': 'B8CE', 'jp': 'B8CJ'}[region]
    images = {}
    for variant in ['before', 'after']:
        work = case / variant
        work.mkdir(parents=True, exist_ok=True)
        (work / 'kh_com.ini').write_text(f'config_version=11\nview_mode={4 if wide else 0}\n'
            'zoom=6\nscale_mode=0\nhd_text=0\nfullscreen=0\nvsync=0\n')
        env = {k: v for k, v in os.environ.items() if not k.startswith(('KHCOM_', 'SDL_'))}
        env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1',
                   KHCOM_WINDOW='1920x1080' if wide else '240x160',
                   KHCOM_EVENT='1:0' if actor == 'riku' else '0:4',
                   KHCOM_SHOTS=','.join(map(str, SHOTS)), KHCOM_QUIT='1205',
                   KHCOM_KEYS=','.join(f'{f}-{f+2}:A' for f in range(100, 1200, 100)))
        if variant == 'before': env['KHCOM_PORTRAIT_UNCLIPPED'] = '1'
        proc = subprocess.run([str(ROOT / f'build/ra-{region}/khcom.exe'), '--rom',
                               str(ROOT / f'roms/{rom}.gba'), '--data-dir', str(work),
                               '--language', 'es'], cwd=work, env=env, timeout=240,
                              stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if proc.returncode: raise RuntimeError(f'{case.name}/{variant}: exit {proc.returncode}')
        if 'CRASH' in (work / 'kh_com.log').read_text(errors='replace'):
            raise RuntimeError(f'{case.name}/{variant}: crash')
        images[variant] = []
        for frame in SHOTS:
            image = Image.open(work / f'shot_{frame:05d}.bmp').convert('RGB')
            image.save(work / f'shot_{frame:05d}.png')
            images[variant].append(image)
    changes = [ImageChops.difference(a, b).getbbox()
               for a, b in zip(images['before'], images['after'])]
    if not wide and any(changes): raise AssertionError(f'Original GBA pixels changed: {case.name}: {changes}')
    return {'region': region, 'actor': actor, 'wide': wide, 'frames': SHOTS,
            'changed_boxes': changes, 'original_pixels_identical': not any(changes) if not wide else None,
            'directory': str(case), 'status': 'PASS'}


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    results = []
    with ThreadPoolExecutor(max_workers=3) as pool:
        jobs = [pool.submit(run_case, region, actor, wide)
                for region in ['eu', 'us', 'jp'] for actor in ['sora', 'riku'] for wide in [False, True]]
        for job in as_completed(jobs):
            result = job.result()
            results.append(result)
            print(f"{result['region']} {result['actor']} {'wide' if result['wide'] else 'GBA'}: PASS", flush=True)
    if not any(any(r['changed_boxes']) for r in results if r['wide']):
        raise AssertionError('The fixture did not exercise any visible portrait regression')
    (OUT / 'report.json').write_text(json.dumps(results, indent=2) + '\n')
    print('PASS 12 regional/actor/view comparisons; 24 native runs; original GBA frames identical')


if __name__ == '__main__': main()
