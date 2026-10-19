"""Official battle achievements, offline (71): every boss battle the official set
reads (pending mode argument 0x0300749C = battle id 148-177) is started on its
stage, as Sora or Riku (the graphs test floors[0].world at 0x0203A346: 10 for
Sora's game, 8 for Riku's, whose first floor is Hollow Bastion), through the game's own battle mode (harness battleid)
and won through the game's own damage route (bosskill), while KHCOM_RA_DRYRUN
evaluates the 90 official definitions. The control run starts the same battle
and loses it the same way (playerkill): nothing may trigger there. Needs build/ra-reference-audit's
catalog and build/pc-eu.

    python port/tools/ra_battle_dryrun.py                 # every pending battle achievement
    python port/tools/ra_battle_dryrun.py 15651 160387    # only these
"""
import json
import os
import re
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DEFS = ROOT / 'build/ra-reference-audit/references.csv.definitions.json'
OUT = ROOT / 'build/ra-battle-dryrun'
# the stage each boss is fought on (gWorldBattleStages[world])
STAGE = {148: 10, 149: 1, 150: 4, 151: 2, 152: 5, 153: 8, 154: 9, 155: 6, 156: 12, 157: 3, 158: 7, 159: 3, 160: 3}
OBTAIN = ','.join(f'1100:obtain:{k}' for k in range(30, 60))


def verified():
    src = (ROOT / 'port/src/port_achievements.c').read_text()
    block = src[src.index('sOfficialVerified'):]
    return {int(x) for x in re.findall(r'(\d+)u', block[:block.index('};')])}


def cases(only):
    defs = {a['id']: a['definition'] for a in json.loads(DEFS.read_text())}
    done = verified()
    out = {}
    for ident, text in defs.items():
        if ident > 1000000 or (not only and ident in done) or (only and ident not in only):
            continue
        battle = re.findall(r'0x 00749c!=(\d+)', text)
        core = text.split('S')[0]
        world = re.findall(r'0xH042346=(\d+)', core)
        if battle and world:
            out[ident] = (int(battle[0]), int(world[-1]))
    return out


def run(battle, world, win):
    d = OUT / f'{battle}-{world}{"-win" if win else "-control"}'
    d.mkdir(parents=True, exist_ok=True)
    for f in ('khcom.exe', 'SDL3.dll'):
        shutil.copy2(ROOT / 'build/pc-eu' / f, d / f)
    (d / 'kh_com.log').unlink(missing_ok=True)
    hit = 'bosskill' if win else 'playerkill'
    wins = ',' + ','.join(f'{f}:{hit}' for f in range(2200, 6000, 150))
    env = dict(os.environ, SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1',
               KHCOM_RA_DRYRUN=str(DEFS), KHCOM_TRACE_MODES='1', KHCOM_RA_WATCH='1',
               KHCOM_MODE=f'300:{"roomriku:10700" if world == 8 else "room:10900"},302:roomenemies:0,{OBTAIN},'
                          f'1300:battleid:{battle | STAGE.get(battle, 12) << 8}{wins}',
               KHCOM_QUIT='6600')
    subprocess.run([str(d / 'khcom.exe'), '--language', 'es', '--rom', str(ROOT / 'roms/B8CP.gba'),
                    '--data-dir', str(d)], cwd=d, env=env, timeout=1200,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    log = (d / 'kh_com.log').read_text(errors='replace')
    trig = sorted({int(a) for a, _ in re.findall(r'ra-dryrun: trigger (\d+) frame (\d+)', log)})
    modes = [m for _, m in re.findall(r'harness: frame (\d+) mode (\S+)', log)]
    return trig, 'mode_battle' in modes, 'CRASH' in log


def main():
    only = {int(a) for a in sys.argv[1:]}
    todo = cases(only)
    battles = sorted(set(todo.values()))
    OUT.mkdir(parents=True, exist_ok=True)
    with ThreadPoolExecutor(max_workers=6) as pool:
        wins = dict(zip(battles, pool.map(lambda bw: run(*bw, True), battles)))
        ctls = dict(zip(battles, pool.map(lambda bw: run(*bw, False), battles)))
    report, passed = {}, []
    for ident, bw in sorted(todo.items()):
        (tw, bat, crash), (tc, _, crash2) = wins[bw], ctls[bw]
        ok = ident in tw and ident not in tc and not crash and not crash2
        report[ident] = dict(battle=bw[0], world=bw[1], battle_mode=bat, win=ident in tw, control=ident in tc,
                             crash=crash or crash2, ok=ok)
        passed += [ident] if ok else []
        print(f'{ident:7d} battle {bw[0]} world {bw[1]:2d}: win={"Y" if ident in tw else "-"} '
              f'control={"Y" if ident in tc else "-"} battle={bat} {"PASS" if ok else ""}', flush=True)
    (OUT / 'report.json').write_text(json.dumps(report, indent=1))
    print(f'{len(passed)}/{len(todo)} pass: {passed}')


if __name__ == '__main__':
    main()
