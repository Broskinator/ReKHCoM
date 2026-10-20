"""Many harness scenarios on one Android emulator/device, using the separate --smoke app.

The smoke APK (build_android.py --abi x86 --smoke --smoke-env KHCOM_FAST=1) reads
files/smoke_env.txt (KEY=VALUE lines) before the game starts, so a single install runs
menus, fixtures, bosses and the playthrough bot. A scenario passes when the game process
exits by itself (KHCOM_QUIT) with no native crash and no CRASH/FATAL line in kh_com.log.
Platforms without fault recovery (Android, web, Linux ARM) die on a stray NULL access that
the desktop ports silently recover, so this is the check for those.

    python port/tools/test_android_scenarios.py --serial emulator-5554 --regions us \
        --groups menus fixtures bosses bot [--only name,name]
"""
import argparse, json, re, subprocess, sys, time
from pathlib import Path

R = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(R / 'port/tools'))
import test_battles

ADB = str(R.parent / 'toolchain/android/sdk/platform-tools/adb.exe')
PKG = 'com.rekhcom.android.smoke'
CODES = {'eu': 'B8CP', 'us': 'B8CE', 'jp': 'B8CJ'}
OUT = R / 'build/android-scenarios'

MENU_MODES = ['mode_status', 'mode_worldselect', 'Mode_Deck', 'mode_deckexchange', 'mode_jiminy', 'mode_ms_top',
              'mode_allmap', 'mode_worldinspect', 'mode_worldwarp', 'mode_pooh', 'mode_staffroll', 'mode_continue',
              'mode_premire', 'mode_wlogo', 'mode_title']

FIXTURES = {
    'room-castle': 'KHCOM_MODE=60:room:10901,120:fieldfreeze',
    'doorselect': 'KHCOM_MODE=60:room:10901,200:doorselect',
    'doortutorial': 'KHCOM_MODE=60:room:10901,200:doortutorial',
    'castle': 'KHCOM_MODE=60:castle',
    'obtain': 'KHCOM_MODE=60:room:10901,200:obtain',
    'deckuse': 'KHCOM_MODE=60:deckuse',
    'jiminy-fixture': 'KHCOM_MODE=60:jiminy',
    'mapsave': 'KHCOM_MODE=60:room:10901,200:mapsave',
    'moogle': 'KHCOM_MODE=60:room:10901,200:moogle',
    'rikuopening': 'KHCOM_MODE=60:rikuopening',
    'roomriku': 'KHCOM_MODE=60:roomriku:10901',
    'levelup': 'KHCOM_BATTLE=0:1:0;KHCOM_MODE=400:levelup',
    'battlewin': 'KHCOM_BATTLE=0:1:0;KHCOM_MODE=400:battlewin',
    'bosskill': 'KHCOM_BATTLE=10:1:0;KHCOM_MODE=400:bosskill',
    'playerkill': 'KHCOM_BATTLE=0:1:0;KHCOM_MODE=400:playerkill',
}


def adb(*args, check=True):
    return subprocess.run([ADB, '-s', SERIAL, *map(str, args)], capture_output=True, check=check)


def private(*args):
    return adb('shell', 'run-as', PKG, *args, check=False)


def battle_keys(frames):
    keys = ['60-63:A']
    dirs = ('RIGHT', 'LEFT', 'UP', 'DOWN')
    for n, f in enumerate(range(300, frames, 24)):
        keys.append(f'{f}-{f + 3}:A')
        if n % 4 == 0:
            keys.append(f'{f + 6}-{f + 20}:{dirs[(n // 4) % 4]}')
    return ','.join(keys)


def scenarios(groups, only, riku):
    out = []
    if 'menus' in groups:
        for m in MENU_MODES:
            out.append((f'menu-{m}', 400, 90, f'KHCOM_MODE=60:{m}\nKHCOM_KEYS=200-203:A,260-263:DOWN,300-303:A,340-343:B'))
    if 'fixtures' in groups:
        for name, env in FIXTURES.items():
            out.append((f'fx-{name}', 900, 120, env.replace(';', '\n') + '\nKHCOM_KEYS=300-303:A,360-363:A,420-423:A'))
    if 'bosses' in groups or 'battles' in groups:
        stages = test_battles.stage_table()
        for i, world, battle in test_battles.battle_entries():
            if 'bosses' in groups and 'battles' not in groups and not re.search(r'BOSS|BOS_|_BTL_[A-Z_]*(LEXAEUS|VEXEN|LARXENE|MARLUXIA|AXEL|RIKU|ANSEM)', battle):
                continue
            stage = next((k for k, s in enumerate(stages) if s.replace('BATTLE_STAGE_', 'WORLD_') == world), 0)
            out.append((f'battle-{i:03d}-{battle}', 1800, 150,
                        f'KHCOM_BATTLE={i}:{stage}:{int(riku)}\nKHCOM_KEYS={battle_keys(1800)}'))
    if 'bot' in groups:
        out.append(('bot-playthrough', 60000, 1500, 'KHCOM_BOT=1\nKHCOM_SHOTS=5000,15000,30000,45000,60000'))
    if only:
        out = [s for s in out if any(o in s[0] for o in only)]
    return out


def run(region, name, frames, timeout, env):
    private('rm', '-f', 'files/kh_com.log', 'files/smoke_env.txt')
    text = f'KHCOM_FAST=1\nKHCOM_QUIT={frames}\n{env}\n'
    tmp = OUT / 'env.txt'
    tmp.write_text(text)
    adb('push', tmp, '/data/local/tmp/smoke_env.txt')
    private('cp', '/data/local/tmp/smoke_env.txt', 'files/smoke_env.txt')
    adb('shell', 'am', 'force-stop', PKG)
    adb('logcat', '-c')
    code = CODES[region]
    adb('shell', 'run-as', PKG, 'am', 'start', '--user', '0', '-n', PKG + '/com.rekhcom.android.GameActivity',
        '--es', 'region', region, '--es', 'rom', f'/data/user/0/{PKG}/files/roms/{code}.gba')
    start = time.monotonic()
    time.sleep(6)
    status = 'timeout'
    while time.monotonic() - start < timeout:
        crashes = adb('logcat', '-d', '-b', 'crash').stdout.decode(errors='replace')
        if PKG in crashes and ('Fatal signal' in crashes or 'FATAL EXCEPTION' in crashes):
            status = 'CRASH'
            break
        if adb('shell', 'pidof', PKG + ':game', check=False).returncode:
            status = 'ok'
            break
        time.sleep(3)
    log = private('cat', 'files/kh_com.log').stdout.decode(errors='replace')
    detail = ''
    if status == 'CRASH':
        crashes = adb('logcat', '-d', '-b', 'crash').stdout.decode(errors='replace')
        detail = '\n'.join(l.split(': ', 1)[-1] for l in crashes.splitlines() if re.search(r'Fatal signal|#0[0-6] pc', l))
    elif status == 'ok' and re.search(r'CRASH|FATAL', log):
        status = 'LOGGED-CRASH'
    elif status == 'ok' and 'harness: no mode' in private('cat', 'files/kh_com.log').stdout.decode(errors='replace'):
        status = 'BAD-SCENARIO'
    elif status == 'timeout':
        adb('shell', 'am', 'force-stop', PKG)
    return status, detail, log[-300:]


def main():
    global SERIAL
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('--serial', required=True)
    p.add_argument('--regions', nargs='+', choices=CODES, default=['us'])
    p.add_argument('--groups', nargs='+', default=['menus', 'fixtures', 'bosses'],
                   choices=['menus', 'fixtures', 'bosses', 'battles', 'bot'])
    p.add_argument('--only', default='')
    p.add_argument('--riku', action='store_true')
    args = p.parse_args()
    SERIAL = args.serial
    OUT.mkdir(parents=True, exist_ok=True)
    report = []
    for region in args.regions:
        code = CODES[region]
        private('mkdir', '-p', 'files/roms')
        adb('push', R / 'roms' / f'{code}.gba', f'/data/local/tmp/{code}.gba')
        private('cp', f'/data/local/tmp/{code}.gba', f'files/roms/{code}.gba')
        for name, frames, timeout, env in scenarios(args.groups, [o for o in args.only.split(',') if o], args.riku):
            status, detail, tail = run(region, name, frames, timeout, env)
            print(f'{region} {name}: {status}', flush=True)
            if status != 'ok':
                print(detail or tail, flush=True)
            report.append(dict(region=region, name=name, status=status, detail=detail))
            (OUT / f'report-{"-".join(args.regions)}.json').write_text(json.dumps(report, indent=1))
    bad = [r for r in report if r['status'] != 'ok']
    print(f'{len(report) - len(bad)}/{len(report)} scenarios ok')
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
