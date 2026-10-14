#!/usr/bin/env python3
"""Capture original/HD small EXP numbers through real battle sprite draws.

Two fixture values cover all ten digits. This is a rendering fixture, not
natural experience gain or a tutorial. BMP pairs require visual review.
"""
import argparse
import datetime
import json
import os
from pathlib import Path
import shutil
import subprocess
from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--versions', nargs='+', default=['eu', 'us', 'jp'])
    parser.add_argument('--deck', action='store_true', help='also capture Sora deck counts/CP after a room transition')
    args = parser.parse_args()
    base = ROOT / 'build/codex-text-options-hd' / datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')
    base.mkdir(parents=True)
    results = []
    for version in args.versions:
        for hd in (0, 1):
            work = base / f'{version}-hd{hd}'
            work.mkdir()
            for name in ('khcom.exe', 'SDL3.dll'):
                shutil.copy2(ROOT / f'build/codex-text-options-{version}' / name, work / name)
            shutil.copytree(ROOT / 'port/assets/fonts', work / 'assets/fonts')
            (work / 'kh_com.ini').write_text(f'config_version=2\nview_mode=0\nwindow_scale=4\nhd_text={hd}\nfilter=0\n')
            env = {key: value for key, value in os.environ.items() if not key.startswith('KHCOM_')}
            env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1',
                       KHCOM_BATTLE='0:0:0', KHCOM_KEYS='60-63:A',
                       KHCOM_MODE='320:textdigits:12345,400:textdigits:67890', KHCOM_TRACE_MODES='1',
                       KHCOM_OUTPUT_SHOT='324,404', KHCOM_QUIT='410')
            env['KHCOM_HD_TEXT_TRACE'] = '324,404,944'
            if args.deck:
                env.update(KHCOM_MODE=env['KHCOM_MODE'] + ',500:room:10000,740:Mode_Deck',
                           KHCOM_OUTPUT_SHOT='324,404,944', KHCOM_QUIT='950')
            rom = {'eu': 'B8CP', 'us': 'B8CE', 'jp': 'B8CJ'}[version]
            proc = subprocess.run([str(work / 'khcom.exe'), '--language', 'es', '--rom',
                                   str(ROOT / 'roms' / f'{rom}.gba')], cwd=work, env=env,
                                  stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=120)
            log = (work / 'kh_com.log').read_text(errors='replace')
            passed = proc.returncode == 0 and 'CRASH' not in log and 'harness: textdigits 12345' in log and 'harness: textdigits 67890' in log
            passed &= all((work / f'out_{frame:05d}.bmp').exists() for frame in (324, 404))
            if args.deck:
                passed &= (work / 'out_00944.bmp').exists() and 'Mode_Deck' in log
                if hd:
                    import re
                    counts = re.findall(r'hd-text: frame=944 layer=\d+ flowing=\d+ subtitles=\d+ small=(\d+)', log)
                    passed &= bool(counts) and sum(map(int, counts)) >= 10
            results.append(dict(version=version, hd=hd, exit=proc.returncode, captures=passed, directory=str(work)))
        for frame in (324, 404):
            off = Image.open(base / f'{version}-hd0/out_{frame:05d}.bmp').convert('RGB')
            on = Image.open(base / f'{version}-hd1/out_{frame:05d}.bmp').convert('RGB')
            # Original view, scale4 and 960x640: EXP at y40, digits follow the
            # 32px label. Verify HD changed actual counter pixels.
            box = (32 * 4, 40 * 4, 80 * 4, 50 * 4)
            diff = ImageChops.difference(off.crop(box), on.crop(box))
            changed = diff.getbbox() is not None
            results.append(dict(version=version, frame=frame, digits_changed=changed))
            contact = Image.new('RGB', (off.width, 80), 'black')
            contact.paste(off.crop((0, 152, off.width, 192)), (0, 0))
            contact.paste(on.crop((0, 152, on.width, 192)), (0, 40))
            contact.save(base / f'{version}_{frame}_digits.png')
    (base / 'results.json').write_text(json.dumps(results, indent=2))
    print(json.dumps(results, indent=2))
    print(base)
    return 0 if all(result.get('captures', result.get('digits_changed', False)) for result in results) else 1


if __name__ == '__main__':
    raise SystemExit(main())
