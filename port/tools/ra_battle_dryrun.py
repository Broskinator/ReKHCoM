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
ALL = os.environ.get('RA_ALL') == '1'  # also re-check the already verified ones
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
        if ident > 1000000 or (not only and ident in done and not ALL) or (only and ident not in only):
            continue
        battle = re.findall(r'0x 00749c!=(\d+)', text)
        core = text.split('S')[0]
        world = re.findall(r'0xH042346=(\d+)', core)
        if battle and world:
            out[ident] = (int(battle[0]), int(world[-1]), challenge(text))
    return out


COMMON = re.compile(r'^R:0x 00749c!=\d+_N:d0xW0423e4=0_0xW0423e4!=0\.1\._.*?R:0x 0080cc!=27_N:0xH042346=8_.*?R:0x 0080cc!=27')


def challenge(text):
    """The setup a challenge graph asks for, solved from the graph itself: the
    active deck (0x02034ACC), its card count (0x0203A4CC + 0xE0 * deck) and a
    card outside every forbidden id range (collection entry: L nibble and H
    byte); the player is kept unhurt for the no-damage ones."""
    rest = COMMON.sub('', text)
    if re.fullmatch(r'_d0xW0423e4!=0_0xW0423e4=0_0xH042346=\d+', rest):
        return None
    deck = None
    m = re.search(r'0xH03cacc=(\d)', rest)
    if m:
        deck = int(m.group(1))
    count = 20
    for addr, op, n in re.findall(r'0xH0(424cc|425ac|4268c)(<=|>=)(\d+)', rest):
        d = {'424cc': 0, '425ac': 1, '4268c': 2}[addr]
        if deck is None or d == deck:
            deck, count = d, int(n)
            break
    slots = re.findall(r'I:0x 0(42[3-6][0-9a-f]{2})\*2', rest)
    if deck is None:
        deck = 1 if slots and int(slots[0], 16) >= 0x424d0 else 0
    banned = [(int(a) * 256 + int(lo), int(a) * 256 + int(hi)) for a, lo, hi in re.findall(
        r'N:0xL042691=(\d+)_I:0x 0[0-9a-f]+\*2_N:0xH042690>=(\d+)_I:0x 0[0-9a-f]+\*2_R:0xH042690<=(\d+)', rest)]
    card = next(c for c in list(range(10)) + list(range(10, 450)) if not any(lo <= c <= hi for lo, hi in banned))
    return dict(deck=deck, count=count, card=card, noharm=True)


def run(battle, world, win, setup=None, tag=''):
    d = OUT / f'{battle}-{world}{tag}{"-win" if win else "-control"}'
    d.mkdir(parents=True, exist_ok=True)
    for f in ('khcom.exe', 'SDL3.dll'):
        shutil.copy2(ROOT / 'build/pc-eu' / f, d / f)
    (d / 'kh_com.log').unlink(missing_ok=True)
    extra = ''
    if setup:
        extra = f',1200:deckuse:{setup["deck"] | setup["count"] << 2 | setup["card"] << 9}'
        if setup['noharm']:
            extra += ',' + ','.join(f'{f}:noharm' for f in range(1320, 1450, 10))
    hit = 'bosskill' if win else 'playerkill'
    wins = ',' + ','.join(f'{f}:{hit}' for f in range(1450, 6000, 100))
    env = dict(os.environ, SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1',
               KHCOM_RA_DRYRUN=str(DEFS), KHCOM_TRACE_MODES='1', KHCOM_RA_WATCH='1',
               KHCOM_MODE=f'300:{"roomriku:10700" if world == 8 else "room:10900"},302:roomenemies:0,{OBTAIN}{extra},'
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
    keys = sorted({(b, w) if c is None else (b, w, i) for i, (b, w, c) in todo.items()}, key=str)

    def go(key, win):
        if len(key) == 2:
            return run(key[0], key[1], win)
        return run(key[0], key[1], win, todo[key[2]][2], f'-c{key[2]}')
    OUT.mkdir(parents=True, exist_ok=True)
    with ThreadPoolExecutor(max_workers=6) as pool:
        wins = dict(zip(keys, pool.map(lambda k: go(k, True), keys)))
        ctls = dict(zip(keys, pool.map(lambda k: go(k, False), keys)))
    report, passed = {}, []
    for ident, (b, w, c) in sorted(todo.items()):
        bw = (b, w) if c is None else (b, w, ident)
        (tw, bat, crash), (tc, _, crash2) = wins[bw], ctls[bw]
        ok = ident in tw and ident not in tc and not crash and not crash2
        report[ident] = dict(battle=bw[0], world=bw[1], setup=c, battle_mode=bat, win=ident in tw, control=ident in tc,
                             crash=crash or crash2, ok=ok)
        passed += [ident] if ok else []
        print(f'{ident:7d} battle {bw[0]} world {bw[1]:2d}: win={"Y" if ident in tw else "-"} '
              f'control={"Y" if ident in tc else "-"} battle={bat} {"PASS" if ok else ""}', flush=True)
    (OUT / 'report.json').write_text(json.dumps(report, indent=1))
    print(f'{len(passed)}/{len(todo)} pass: {passed}')


if __name__ == '__main__':
    main()
