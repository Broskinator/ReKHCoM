"""Capture all 28 distinct battle backgrounds, without changing game data.

The 209 debug entries share backgrounds. Thirteen world/garden maps, six
additional humanoid maps and nine boss maps cover the distinct render paths.
Captures prove fixture execution, never visual acceptance or a full boss fight.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import hashlib
import html
import json
import os
from pathlib import Path
import shutil
import subprocess

from PIL import Image, ImageDraw
from test_battles import battle_entries, stage_table

ROOT = Path(__file__).resolve().parents[2]
SHOTS = (180, 380, 600)


def cases():
    entries, stages = battle_entries(), stage_table()
    result = [dict(arena=s.removeprefix('BATTLE_STAGE_'), battle=0, stage=i,
                   kind='world') for i, s in enumerate(stages)
              if s != 'BATTLE_STAGE_GARDEN']
    special = {
        'GARDEN': 'CARD_SOLDIERS', 'HADES': 'HADES', 'HOOK': 'HOOK',
        'MANSION': 'VEXEN_2', 'ANSEM_2': 'ANSEM_2', 'MARLUXIA': 'MARLUXIA',
        'HUM': 'AXEL_1', 'PARASITE_CAGE': 'PARASITE_CAGE', 'DARKSIDE': 'DARKSIDE',
        'GUARD_ARMOR': 'GUARD_ARMOR', 'TRICKMASTER': 'TRICKMASTER',
        'OOGIE_BOOGIE': 'OOGIE_BOOGIE', 'DRAGON_MALEFICENT': 'DRAGON_MALEFICENT',
        'URSULA': 'URSULA', 'JAFAR': 'JAFAR', 'MARLUXIA_2': 'MARLUXIA_2',
    }
    for name, battle in special.items():
        entry, world, _ = next(e for e in entries if e[2] == 'BATTLE_' + battle)
        stage = next((i for i, s in enumerate(stages)
                      if s.replace('BATTLE_STAGE_', 'WORLD_') == world), 0)
        result.append(dict(arena=name, battle=entry, stage=stage,
                           kind='boss' if entry >= 197 else 'affine'))
    assert len(result) == 28
    return result


def capture(case, aspect, args, root):
    directory = root / aspect / case['arena']
    directory.mkdir(parents=True, exist_ok=True)
    # Both retain a 270-line internal canvas. The first is equivalent to
    # 1920x1080 at x4; the second to 3840x1080 at x4.
    width, height, zoom = (960, 540, 4) if aspect == '16x9' else (960, 270, 2)
    for name in ('khcom.exe', 'SDL3.dll'):
        shutil.copy2(ROOT / args.build / name, directory / name)
    (directory / 'kh_com.ini').write_text(
        f'config_version=11\nview_mode=4\nzoom={zoom}\nhd_text=0\n'
        'fullscreen=0\nvsync=0\nexperimental_save_point_slot_select=0\n')
    env = dict(os.environ, SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy',
               KHCOM_FAST='1', KHCOM_WINDOW=f'{width}x{height}',
               KHCOM_BATTLE=f"{case['battle']}:{case['stage']}:{int(args.riku)}",
               KHCOM_KEYS='60-63:A,200-380:LEFT,400-600:RIGHT',
               KHCOM_OUTPUT_SHOT=','.join(map(str, args.shots)), KHCOM_QUIT=str(args.frames))
    rom = {'eu': 'B8CP', 'us': 'B8CE', 'jp': 'B8CJ'}[args.version]
    try:
        process = subprocess.run([str(directory / 'khcom.exe'), '--rom', str(ROOT / 'roms' / (rom + '.gba'))],
                                 cwd=directory, env=env, timeout=240,
                                 stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        code = process.returncode
    except subprocess.TimeoutExpired:
        code = 'timeout'
    logpath = directory / 'kh_com.log'
    log = logpath.read_text(errors='replace') if logpath.exists() else ''
    shots = []
    for frame in args.shots:
        bmp = directory / f'out_{frame:05d}.bmp'
        if bmp.exists():
            png = bmp.with_suffix('.png')
            with Image.open(bmp) as image:
                image.save(png)
            shots.append(str(png.relative_to(root)))
    return dict(**case, aspect=aspect, actor='riku' if args.riku else 'sora', exit=code,
                fixture_passed=code == 0 and 'CRASH' not in log and len(shots) == len(args.shots),
                visual_review='pending', shots=shots)


def gallery(root, rows):
    findings = json.loads((ROOT / 'port/progress/BATTLE_SCENERY_AUDIT.json').read_text(encoding='utf-8'))
    improvements = {item['arena']: item for item in findings['improvements']}
    body = ['<!doctype html><meta charset="utf-8"><title>Battle scenery audit</title>',
            '<style>body{background:#141720;color:#eee;font:16px system-ui;margin:24px}'
            'img{width:32%;image-rendering:pixelated}a{color:#9ce}section{margin-bottom:30px}</style>',
            '<h1>Battle scenery audit</h1><p>Capturas cronológicas; los fotogramas y '
            'el alcance están en report.json. Capturar no equivale a aprobar visualmente. '
            'Esta auditoría no modifica los escenarios.</p>']
    done = sum(item['done'] for item in improvements.values())
    body += [f"<h2>{findings['initially_reviewed']}/{findings['total_backgrounds']} revisión inicial · "
             f"{done}/{len(improvements)} mejoras terminadas</h2>",
             '<p>Hook y Monstro: conservar según indicación del usuario. '
             'Alicia: solo extender el ajedrez inferior.</p><ul>']
    for item in improvements.values():
        body.append('<li>' + html.escape(item['name'] + ': ' + item['plan']) + '</li>')
    body.append('</ul>')
    for aspect in dict.fromkeys(r['aspect'] for r in rows):
        group = [r for r in rows if r['aspect'] == aspect]
        for offset in range(0, len(group), 7):
            page = group[offset:offset + 7]
            thumb_height = 180 if aspect == '16x9' else 90
            row_height = thumb_height + 28
            sheet = Image.new('RGB', (960, row_height * len(page)), '#141720')
            draw = ImageDraw.Draw(sheet)
            for i, row in enumerate(page):
                y = i * row_height
                draw.text((8, y + 6), row['arena'] + '  ' + aspect, fill='white')
                for j, shot in enumerate(row['shots'][:3]):
                    with Image.open(root / shot) as image:
                        sheet.paste(image.convert('RGB').resize((320, thumb_height), Image.Resampling.NEAREST),
                                    (j * 320, y + 28))
            sheet.save(root / f'sheet-{aspect}-{offset // 7 + 1}.png')
    for row in rows:
        body.append('<section><h2>' + html.escape(row['arena'] + ' / ' + row['aspect']) + '</h2>')
        if row['arena'] in improvements:
            item = improvements[row['arena']]
            body.append('<p>' + ('Terminada: ' if item['done'] else 'Pendiente: ') + html.escape(item['plan']) + '</p>')
        elif row['arena'] in findings['preserve_user_accepted']:
            body.append('<p>Conservar: el usuario indicó que ya se ve bien.</p>')
        else:
            body.append('<p>Sin fallo bloqueante confirmado en estas muestras; no implica aprobación de todas sus fases.</p>')
        for shot in row['shots']:
            url = html.escape(shot.replace('\\', '/'), quote=True)
            body.append(f'<a href="{url}"><img src="{url}"></a>')
        body.append('</section>')
    (root / 'index.html').write_text('\n'.join(body), encoding='utf-8')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', required=True)
    parser.add_argument('--out', required=True, help='new isolated directory under build/')
    parser.add_argument('--version', choices=('eu', 'us', 'jp'), default='eu')
    parser.add_argument('--aspects', nargs='+', choices=('16x9', '32x9'), default=['16x9', '32x9'])
    parser.add_argument('--only', help='comma separated arena names')
    parser.add_argument('--riku', action='store_true')
    parser.add_argument('--jobs', type=int, choices=(1, 2), default=2)
    parser.add_argument('--frames', type=int, default=610)
    parser.add_argument('--shots', type=int, nargs='+', default=list(SHOTS))
    args = parser.parse_args()
    if args.frames <= 0 or any(f <= 0 or f >= args.frames for f in args.shots):
        parser.error('shot frames must be positive and before --frames')
    root = (ROOT / args.out).resolve()
    if not root.is_relative_to((ROOT / 'build').resolve()):
        parser.error('output must be inside build/')
    root.mkdir(parents=True, exist_ok=False)
    selected = cases()
    if args.only:
        selected = [c for c in selected if c['arena'] in args.only.split(',')]
    rows = []
    with ThreadPoolExecutor(args.jobs) as pool:
        futures = [pool.submit(capture, case, aspect, args, root)
                   for case in selected for aspect in args.aspects]
        for future in as_completed(futures):
            row = future.result()
            rows.append(row)
            (root / 'report.json').write_text(json.dumps(rows, indent=2) + '\n')
            print(f"{len(rows)}/{len(futures)} {row['arena']} {row['aspect']} "
                  f"{'captured' if row['fixture_passed'] else 'FAILED'}", flush=True)
    order = {case['arena']: i for i, case in enumerate(selected)}
    rows.sort(key=lambda row: (row['aspect'], order[row['arena']]))
    gallery(root, rows)
    metadata = dict(build=args.build, version=args.version, frames=args.frames, shot_frames=args.shots,
                    exe_sha256=hashlib.sha256((ROOT / args.build / 'khcom.exe').read_bytes()).hexdigest(),
                    fixtures=len(rows), passed=sum(r['fixture_passed'] for r in rows),
                    visually_approved=0, rows=rows)
    (root / 'report.json').write_text(json.dumps(metadata, indent=2) + '\n')
    print(root, flush=True)
    return 0 if rows and all(r['fixture_passed'] for r in rows) else 1


if __name__ == '__main__':
    raise SystemExit(main())
