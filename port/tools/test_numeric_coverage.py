#!/usr/bin/env python3
"""Capture BG statistical digits through real menu paths, off/on in isolated runs.

Trace verifies character/source offsets and VRAM alignment; images verify the
presented replacement. Every process runs serially with its own files.
"""
import argparse
import datetime
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[2]
NUMERIC_SOURCES = ('src/card/card_deckmenu2.c', 'src/card/card_deckmenu2_riku.c',
                   'src/card/card_deckexchange.c', 'src/menu/mode_status.c')


def keys_for(campaign, empty_values=False):
    if campaign == 'riku':
        return '480-483:A,520-523:DOWN,850-853:B,1220-1223:B'
    if empty_values:
        # Initial collection has no spare cards: Add displays the grey zero
        # variants in all value cells and in selected CP.
        return '480-483:A,520-523:DOWN,550-553:DOWN,580-583:DOWN,640-643:A,1220-1223:B'
    # Remove's first card has CP16. Two rows down and one column right has
    # CP12, testing both problematic tile patterns (2 and 6); return to CP16.
    return ('480-483:A,520-523:DOWN,550-553:DOWN,580-583:DOWN,610-613:DOWN,640-643:A,'
            '780-783:DOWN,810-813:DOWN,840-843:RIGHT,920-923:UP,950-953:UP,980-983:LEFT,1220-1223:B')


def build_baseline(version, ref):
    """Replace only four game objects; reuse frozen PPU/assets/other objects.

    No checkout or user's build is edited. Omitting unrelated build edges is
    intentional: another worker may edit source while these A/B runs execute.
    """
    target = ROOT / f'build/codex-numeric-baseline-{version}'
    target.mkdir(exist_ok=True)
    current = ROOT / f'build/codex-numeric-coverage-{version}'
    ninja = (current / 'build.ninja').read_text()
    header, body = ninja.split('build ', 1)
    blocks = re.split(r'(?m)(?=^build |^default )', 'build ' + body)
    selected = []
    objects = {}
    for source in NUMERIC_SOURCES:
        copy = target / source
        copy.parent.mkdir(parents=True, exist_ok=True)
        copy.write_bytes(subprocess.check_output(['git', 'show', f'{ref}:{source}'], cwd=ROOT))
        old_object = '$builddir/game/' + source.removeprefix('src/').removesuffix('.c') + '.o'
        new_object = f'{target.relative_to(ROOT).as_posix()}/objects/' + source.removeprefix('src/').removesuffix('.c') + '.o'
        (ROOT / new_object).parent.mkdir(parents=True, exist_ok=True)
        objects[old_object] = new_object
        block = next(b for b in blocks if b.startswith(f'build {old_object}:'))
        selected.append(block.replace(old_object, new_object).replace(source, copy.relative_to(ROOT).as_posix()))
    link = next(b for b in blocks if b.startswith('build $builddir/khcom.exe:'))
    for old, new in objects.items():
        link = link.replace(old, new)
    executable = f'{target.relative_to(ROOT).as_posix()}/khcom.exe'
    link = link.replace('build $builddir/khcom.exe:', f'build {executable}:')
    selected.append(link)
    (target / 'baseline.ninja').write_text(header + ''.join(selected) + f'\ndefault {executable}\n')
    subprocess.run([str(ROOT.parent / 'toolchain/bin/ninja.exe'), '-f',
                    str(target.relative_to(ROOT) / 'baseline.ninja'), '-j2', executable], cwd=ROOT, check=True)
    shutil.copy2(current / 'SDL3.dll', target / 'SDL3.dll')
    return target


def compare_original(args, base):
    reference = (ROOT / args.compare_original).resolve()
    results = []
    frames = (440, 750, 900, 990, 1160, 1390)
    for version in args.versions:
        build = build_baseline(version, args.baseline_ref)
        for campaign in args.campaigns:
            work = base / f'{version}-{campaign}-baseline'
            work.mkdir()
            for name in ('khcom.exe', 'SDL3.dll'):
                shutil.copy2(build / name, work / name)
            shutil.copytree(ROOT / 'port/assets/fonts', work / 'assets/fonts')
            (work / 'kh_com.ini').write_text('config_version=2\nview_mode=0\nwindow_scale=4\nhd_text=0\nfilter=0\n')
            env = {k: v for k, v in os.environ.items() if not k.startswith('KHCOM_')}
            room = 'roomriku' if campaign == 'riku' else 'room'
            keys = keys_for(campaign, args.empty_values)
            env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1',
                       KHCOM_MODE=f'30:{room}:10000,240:Mode_Deck,1000:mode_status', KHCOM_KEYS=keys,
                       KHCOM_OUTPUT_SHOT=','.join(map(str, frames)), KHCOM_QUIT='1400')
            rom = {'eu': 'B8CP', 'us': 'B8CE', 'jp': 'B8CJ'}[version]
            proc = subprocess.run([str(work / 'khcom.exe'), '--language', 'es', '--rom',
                                   str(ROOT / 'roms' / f'{rom}.gba')], cwd=work, env=env,
                                  stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=args.timeout)
            matches = {}
            for frame in frames:
                name = f'out_{frame:05d}.bmp'
                original = Image.open(work / name).convert('RGB')
                changed = Image.open(reference / f'{version}-{campaign}-hd0' / name).convert('RGB')
                matches[str(frame)] = ImageChops.difference(original, changed).getbbox() is None
            result = dict(version=version, campaign=campaign, exit=proc.returncode,
                          baseline_source_ref=args.baseline_ref, pixel_exact=matches,
                          passed=proc.returncode == 0 and all(matches.values()), directory=str(work))
            results.append(result)
            print(json.dumps(result), flush=True)
    (base / 'original-results.json').write_text(json.dumps(results, indent=2))
    return 0 if all(r['passed'] for r in results) else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--versions', nargs='+', choices=('eu', 'us', 'jp'), default=['eu', 'us', 'jp'])
    parser.add_argument('--campaigns', nargs='+', choices=('sora', 'riku'), default=['sora', 'riku'])
    parser.add_argument('--timeout', type=int, default=180)
    parser.add_argument('--compare-original', help='compare HD-off frames from a previous results directory with original game objects')
    parser.add_argument('--baseline-ref', default='53d654365', help='Git revision of the four numeric source files before this change')
    parser.add_argument('--empty-values', action='store_true', help='exercise grey zero variants in the empty Add collection')
    args = parser.parse_args()
    base = ROOT / 'build/codex-numeric-coverage-results' / datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')
    base.mkdir(parents=True)
    if args.compare_original:
        result = compare_original(args, base)
        print(base)
        return result
    results = []
    frames = (440, 750, 900, 990, 1160, 1390)
    for version in args.versions:
        for campaign in args.campaigns:
            for hd in (0, 1):
                work = base / f'{version}-{campaign}-hd{hd}'
                work.mkdir()
                for name in ('khcom.exe', 'SDL3.dll'):
                    shutil.copy2(ROOT / f'build/codex-numeric-coverage-{version}' / name, work / name)
                shutil.copytree(ROOT / 'port/assets/fonts', work / 'assets/fonts')
                (work / 'kh_com.ini').write_text(f'config_version=2\nview_mode=0\nwindow_scale=4\nhd_text={hd}\nfilter=0\n')
                env = {k: v for k, v in os.environ.items() if not k.startswith('KHCOM_')}
                room = 'roomriku' if campaign == 'riku' else 'room'
                # Command popup -> Remove (the initial deck has cards while
                # the spare collection is empty), then a card's value grid;
                # Status via
                # normal mode init, followed by actual B exit to the field.
                keys = keys_for(campaign, args.empty_values)
                env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1',
                           KHCOM_MODE=f'30:{room}:10000,240:Mode_Deck,1000:mode_status',
                           KHCOM_KEYS=keys, KHCOM_NUMERIC_TRACE='1', KHCOM_TRACE_MODES='1',
                           KHCOM_HD_TEXT_TRACE=','.join(map(str, frames)),
                           KHCOM_OUTPUT_SHOT=','.join(map(str, frames)), KHCOM_QUIT='1400')
                rom = {'eu': 'B8CP', 'us': 'B8CE', 'jp': 'B8CJ'}[version]
                proc = subprocess.run([str(work / 'khcom.exe'), '--language', 'es', '--rom',
                                       str(ROOT / 'roms' / f'{rom}.gba')], cwd=work, env=env,
                                      stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=args.timeout)
                log = (work / 'kh_com.log').read_text(errors='replace')
                records = re.findall(r'numeric-text: family=(\S+) byte=(\d+) code=(\d+) destination=(?:0x)?([0-9A-Fa-f]+) size=(\d+) style=([0-9a-f]+)', log)
                families = sorted({r[0] for r in records})
                expected_offset = {'deck-category': 1, 'deck-category-zero': None,
                                   'deck-value': 3, 'deck-value-zero': None, 'status': 0}
                expected_style = {'deck-category': 0x1DC02, 'deck-category-zero': 0x13F02,
                                  'deck-value': 0x13202, 'deck-value-zero': 0x13402, 'status': 0x87102}
                signature_ok = bool(records)
                for family, offset, code, dest, size, style in records:
                    digit, offset = int(code) - 48, int(offset)
                    signature_ok &= 0 <= digit <= 9 and int(size) == 32 and 0x6000000 <= int(dest, 16) < 0x6010000 and int(dest, 16) % 32 == 0
                    signature_ok &= offset == 0 if expected_offset[family] is None else offset == (digit + expected_offset[family]) * 32
                    signature_ok &= int(style, 16) == expected_style[family]
                passed = proc.returncode == 0 and 'CRASH' not in log and signature_ok
                passed &= 'status' in families and 'deck-category' in families
                if campaign == 'sora':
                    passed &= ('deck-value-zero' if args.empty_values else 'deck-value') in families
                passed &= all((work / f'out_{f:05d}.bmp').exists() for f in frames)
                passed &= 'mode_status' in log and 'Mode_Deck' in log
                trace = [(int(frame), name) for frame, name in re.findall(r'harness: frame (\d+) mode (\S+)', log)]
                passed &= any(frame > 1220 and name == 'Mode_MapFld' for frame, name in trace)
                result = dict(version=version, campaign=campaign, hd=hd, exit=proc.returncode,
                              empty_values=args.empty_values,
                              passed=bool(passed), families=families, registrations=len(records),
                              signature_ok=bool(signature_ok), directory=str(work))
                results.append(result)
                print(json.dumps(result), flush=True)
            for frame in (440, 1160):
                off = Image.open(base / f'{version}-{campaign}-hd0/out_{frame:05d}.bmp').convert('RGB')
                on = Image.open(base / f'{version}-{campaign}-hd1/out_{frame:05d}.bmp').convert('RGB')
                # Bottom category counters are independent of the names and
                # main deck counters which were already HD before this change.
                box = (0, 560, 320, 640) if frame == 440 else (480, 40, 928, 604)
                changed = ImageChops.difference(off.crop(box), on.crop(box)).getbbox() is not None
                result = dict(version=version, campaign=campaign, frame=frame, numeric_changed=changed)
                results.append(result)
                contact = Image.new('RGB', (off.width * 2, off.height))
                contact.paste(off, (0, 0))
                contact.paste(on, (off.width, 0))
                contact.save(base / f'{version}-{campaign}-{frame}-comparison.png')
            off = Image.open(base / f'{version}-{campaign}-hd0/out_01390.bmp').convert('RGB')
            on = Image.open(base / f'{version}-{campaign}-hd1/out_01390.bmp').convert('RGB')
            results.append(dict(version=version, campaign=campaign,
                                field_unchanged=ImageChops.difference(off, on).getbbox() is None))
    (base / 'results.json').write_text(json.dumps(results, indent=2))
    print(base)
    return 0 if all(r.get('passed', r.get('numeric_changed', r.get('field_unchanged', False))) for r in results) else 1


if __name__ == '__main__':
    raise SystemExit(main())
