"""Capture README screenshots from release builds, using isolated private ROM runs.

Requires EU/US/JP builds in build/rel-* and private dumps in roms/.
Reuses the trailer's isolated capture helper; never changes player saves.
"""
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'port/tools/trailer'))
from fx import fx

OUT = ROOT / 'docs/screenshots'
WORK = ROOT / 'build/readme-v183'
WORK.mkdir(parents=True, exist_ok=True)
RUN = str(os.getpid())
M = lambda mode: ['60:room:10901', '120:fieldfreeze', '180:' + mode]
jobs = [
    ('battle', [], 300, dict(keys=['60-63:A'], extra={'KHCOM_BATTLE': '10:4:0'})),
    ('field', ['60:room:10901', '120:fieldfreeze'], 260, dict(keys=['130-170:UP', '171-210:RIGHT'])),
    ('deck', M('Mode_Deck'), 760, {}),
    ('status', M('mode_status'), 760, {}),
    ('mapcards', M('mode_mapinspect'), 760, {}),
    ('worldcards', M('mode_worldinspect'), 760, {}),
    ('journal', M('mode_jiminy'), 760, {}),
    ('worldmap', M('mode_allmap'), 760, {}),
    ('vortex', M('mode_worldwarp'), 850, {}),
    ('ultrawide', [], 600, dict(keys=['60-63:A'], extra={'KHCOM_BATTLE': '10:4:0', 'KHCOM_WINDOW': '2560x720'})),
    ('ultrawide_menu', M('mode_worldinspect'), 760, dict(extra={'KHCOM_WINDOW': '2560x720'})),
    ('moogle_px', M('mode_ms_top'), 760, {}),
    ('moogle_hd', M('mode_ms_top'), 760, dict(hd=1)),
    ('movie_extended', [], 480, dict(extra={'KHCOM_MOVIE': '1'})),
    ('subtitles_hd', [], 2850, dict(hd=1, extra={'KHCOM_MOVIE': '1'})),
    ('options', [], 500, dict(extra={'KHCOM_OPTIONS_SHOT': '500'})),
]
for language in ['fr', 'de', 'es', 'it']:
    index = {'fr': 2, 'de': 4, 'it': 1, 'es': 3}[language]
    keys = [f'{150 + 15 * k}-{151 + 15 * k}:DOWN' for k in range(index)] + ['200-201:A']
    jobs.append(('lang_' + language, M('mode_jiminy'), 760, dict(lang=language, keys=keys)))
jobs += [('lang_us', M('mode_jiminy'), 760, dict(build='rel-us', rom='B8CE', lang='en')),
         ('lang_jp', M('mode_jiminy'), 760, dict(build='rel-jp', rom='B8CJ', lang=None))]

def capture(job):
    name, modes, frame, options = job
    if any(mode.startswith('180:') for mode in modes):
        frame = 260
    options = dict(options)
    options.setdefault('keys', [])
    extra = {'KHCOM_WINDOW': '1536x864', **options.pop('extra', {})}
    if name.startswith('lang_'):
        modes = ['300:room:10901', '400:fieldfreeze', '600:mode_jiminy']
        frame = 760
    extra['KHCOM_TRACE_MODES'] = '1'
    files = fx('readme183_' + RUN + '_' + name, modes, [frame], save=False, lang=options.pop('lang', 'en'), extra=extra, **options)
    assert files, name
    log = (files[-1].parent / 'kh_com.log').read_text(errors='replace')
    assert 'harness: no mode' not in log, log
    source = files[-1]
    if name == 'options':
        source = source.parent / 'shot_options.bmp'
    im = Image.open(source).convert('RGB')
    im.save(OUT / (name + '.png'), optimize=True)
    print(name, im.size, flush=True)
    return name

with ThreadPoolExecutor(max_workers=2) as pool:
    names = list(pool.map(capture, jobs))

stage = WORK / 'launcher'
stage.mkdir(exist_ok=True)
for filename in ['ReKHCoM.exe', 'SDL3.dll']:
    shutil.copy2(ROOT / 'build/rel-eu' / filename, stage / filename)
for region in ['eu', 'us', 'jp']:
    target = stage / 'bin' / ('khcom_' + region + '.exe')
    target.parent.mkdir(exist_ok=True)
    shutil.copy2(ROOT / f'build/rel-{region}/khcom.exe', target)
shutil.copytree(ROOT / 'port/decompui/assets', stage / 'assets', dirs_exist_ok=True)
shutil.copytree(ROOT / 'port/assets/fonts', stage / 'assets/fonts', dirs_exist_ok=True)
(stage / 'decompui.ini').write_text('ui_language=2\ngame_language=en\n')
shots = stage / 'shots'
shots.mkdir(exist_ok=True)
env = {k: v for k, v in os.environ.items() if not k.startswith(('KHCOM_', 'SDL_', 'DECOMPUI_'))}
env.update(DECOMPUI_SHOT=str(shots), DECOMPUI_INSTALL=str(ROOT / 'roms/B8CP.gba'))
for hint in ['SDL_XINPUT_ENABLED', 'SDL_JOYSTICK_HIDAPI', 'SDL_JOYSTICK_RAWINPUT',
             'SDL_JOYSTICK_DIRECTINPUT', 'SDL_JOYSTICK_WGI', 'SDL_JOYSTICK_GAMEINPUT',
             'SDL_JOYSTICK_GAMEINPUT_RAW']:
    env[hint] = '0'
subprocess.run([str(stage / 'ReKHCoM.exe')], cwd=stage, env=env, check=True, timeout=90, creationflags=0x08000000)
Image.open(shots / 'decompui_0.bmp').convert('RGB').save(OUT / 'launcher.png', optimize=True)
names.append('launcher')

sheet = Image.new('RGB', (960, 196 * ((len(names) + 2) // 3)), '#222222')
draw = ImageDraw.Draw(sheet)
for i, name in enumerate(names):
    im = Image.open(OUT / (name + '.png')).convert('RGB')
    im.thumbnail((320, 170))
    x, y = (i % 3) * 320, (i // 3) * 196
    sheet.paste(im, (x, y))
    draw.text((x + 5, y + 173), name, fill='white')
sheet.save(WORK / 'contact.png')
manifest = {'version': 'v1.8.3', 'zoom': 'x4', 'captures': names,
            'build_sha256': {r: hashlib.sha256((ROOT / f'build/rel-{r}/khcom.exe').read_bytes()).hexdigest() for r in ['eu', 'us', 'jp']},
            'retained_unchanged_reference_images': ['newgame_version.png', 'newgame_difficulty.png']}
(WORK / 'manifest.json').write_text(json.dumps(manifest, indent=2))
