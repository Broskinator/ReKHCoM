"""Render and navigate Options > Achievements using the production SDL handler.

Isolated synthetic local progress; never reads or writes player saves. Sequential
runs and dummy SDL backend keep this suitable while the user is playing.
"""
from pathlib import Path
import json
import os
import struct
import subprocess
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'build/achievement-menu'
OUT.mkdir(parents=True, exist_ok=True)
report = []
for region, width, height in [('eu', 240, 160), ('eu', 640, 360), ('eu', 1920, 1080),
                               ('us', 1920, 1080), ('jp', 1920, 1080)]:
    work = OUT / f'{region}-{width}x{height}'; work.mkdir(exist_ok=True)
    cache = work / 'cache'; cache.mkdir(exist_ok=True)
    catalog = b'KHRACAT1' + struct.pack('<I', 2)
    for id, title in [(101, 'Locked fixture'), (102, 'Earned fixture')]:
        catalog += struct.pack('<II', id, 5)
        for value in (title, 'Read only account snapshot fixture.', '12345'):
            b = value.encode(); catalog += struct.pack('<I', len(b)) + b
    (cache / 'achievements-768.dat').write_bytes(catalog)
    (cache / 'achievement-unlocks.dat').write_bytes(b'KHRAPRG1' + struct.pack('<III', 1, 1, 102))
    local = work / f'achievements_{region}.dat'
    local.write_text('KHLOCAL1\n1\n' + '0\n' * 16)
    original = local.read_bytes()
    (work / 'kh_com.ini').write_text('config_version=11\nview_mode=4\nzoom=6\n'
        'integer_zoom=1\nwindow_scale=1\nfullscreen=0\nvsync=0\n')
    env = {k: v for k, v in os.environ.items() if not k.startswith(('KHCOM_', 'SDL_'))}
    env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1',
        KHCOM_WINDOW=f'{width}x{height}', KHCOM_OPTIONS_TOGGLE='100', KHCOM_OPTIONS_SHOT='999999',
        KHCOM_OPTIONS_HOLD='15', KHCOM_ACHIEVEMENTS_SHOT='1', KHCOM_QUIT='101',
        KHCOM_ACHIEVEMENT_KEYS='1:1073741905,2:1073741903,3:1073741903,4:9,5:1073741905,6:8')
    rom = ROOT / ('roms/' + {'eu': 'B8CP', 'us': 'B8CE', 'jp': 'B8CJ'}[region] + '.gba')
    subprocess.run([str(ROOT / f'build/ra-{region}/khcom.exe'), '--rom', str(rom), '--data-dir', str(work),
                    '--language', 'es'], cwd=work, env=env, check=True, timeout=60)
    log = (work / 'kh_com.log').read_text(encoding='utf-8', errors='replace')
    for state in ['group=0 filter=0 choice=1', 'group=0 filter=1 choice=0',
                  'group=0 filter=2 choice=0', 'group=1 filter=2 choice=0']:
        assert state in log, (region, state)
    shots = sorted(work.glob('options_*.bmp')); assert len(shots) >= 6
    for shot in shots:
        with Image.open(shot) as im: im.save(shot.with_suffix('.png'))
    assert local.read_bytes() == original, 'Browsing must not change awards'
    assert not list(work.glob('*.sav')), 'Browsing must not create a game save'
    report.append(dict(region=region, output=[width, height], navigation='PASS', readonly='PASS'))
(OUT / 'report.json').write_text(json.dumps(report, indent=2))
print('PASS read-only achievement menu navigation/filters/groups/back at five region/size pairs')
