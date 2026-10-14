#!/usr/bin/env python3
"""Serial MAP fixtures. Covers entries, not natural playthrough completion.

Use --matrix for every world/floor combination, --halls for all castle halls.
Every captured entry must actually reach its expected field mode.
"""
import argparse
import datetime
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import yaml

ROOT = Path(__file__).resolve().parents[2]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--build', default='build/codex-gameplay-eu')
    ap.add_argument('--version', choices=('eu', 'us', 'jp'), default='eu')
    ap.add_argument('--actor', choices=('sora', 'riku'), default='sora')
    ap.add_argument('--matrix', action='store_true')
    ap.add_argument('--halls', action='store_true')
    ap.add_argument('--all-rooms', action='store_true', help='every reachable room in selected floor graphs')
    ap.add_argument('--world', type=int, choices=range(13))
    ap.add_argument('--floor', type=int, choices=range(13))
    ap.add_argument('--scale', type=int, choices=(1, 2, 4), default=4)
    args = ap.parse_args()
    work = ROOT / 'build/codex-gameplay-rooms' / datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')
    work.mkdir(parents=True)
    shutil.copy2(ROOT / args.build / 'khcom.exe', work / 'khcom.exe')
    dll = ROOT / args.build / 'SDL3.dll'
    if not dll.exists():
        dll = ROOT.parent / 'toolchain/SDL3-3.4.18/i686-w64-mingw32/bin/SDL3.dll'
    shutil.copy2(dll, work / 'SDL3.dll')
    shutil.copytree(ROOT / 'port/assets/fonts', work / 'assets/fonts')
    (work / 'kh_com.ini').write_text(f'config_version=2\nview_mode=4\nwindow_scale={args.scale}\nhd_text=1\n')
    floors = range(12 if args.actor == 'riku' else 13)
    cases = ([(mode, 0, floor) for floor in floors for mode in (2, 3)] if args.halls else
             [(1, world, floor) for world in ([args.world] if args.world is not None else range(12))
              for floor in ([args.floor] if args.floor is not None else
                            floors if args.matrix else [10 if world == 10 else 11 if world == 8 else 12 if world == 11 and args.actor == 'sora' else 1])])
    if args.actor == 'riku' and any(f >= 12 for _, _, f in cases):
        ap.error('Riku floors are 0..11')
    entries = {e['name']: e for e in yaml.safe_load((ROOT / 'config/assets/map_room_tables.yaml').read_text())['entries']}
    def reachable(floor):
        prefix = 'gRikuMap' if args.actor == 'riku' else 'gMap'
        definition = entries[prefix + 'FloorDefs']['elements'][floor]
        links = entries[definition[4]]['values']
        queue = list(dict.fromkeys((definition[0], definition[1])))
        seen = set(queue)
        for room in queue:
            for neighbor in links[room]:
                if neighbor < len(links) and neighbor not in seen:
                    seen.add(neighbor)
                    queue.append(neighbor)
        return queue
    expanded = [(mode, world, floor, room) for mode, world, floor in cases
                for room in (reachable(floor) if args.all_rooms and mode == 1 else [None])]
    fixture = 'roomriku' if args.actor == 'riku' else 'room'
    steps = [dict(start=60 + i * 300, shot=260 + i * 300, mode=mode, world=world, floor=floor, room=room)
             for i, (mode, world, floor, room) in enumerate(expanded)]
    actions = []
    for s in steps:
        actions.append(f"{s['start']}:{fixture}:{s['mode']*10000+s['world']*100+s['floor']}")
        if s['room'] is not None:
            actions.append(f"{s['start']+80}:roomgoto:{s['room']}")
        actions.append(f"{s['start']+150}:progress")
    env = {k: v for k, v in os.environ.items() if not k.startswith(('KHCOM_', 'SDL_'))}
    env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1', KHCOM_TRACE_MODES='1',
               KHCOM_MODE=','.join(actions),
               KHCOM_OUTPUT_SHOT=','.join(str(s['shot']) for s in steps), KHCOM_QUIT=str(steps[-1]['shot']+20))
    rom = {'eu': 'B8CP', 'us': 'B8CE', 'jp': 'B8CJ'}[args.version]
    result = subprocess.run([str(work / 'khcom.exe'), '--rom', str(ROOT / 'roms' / (rom + '.gba')),
                             '--language', 'es'], cwd=work, env=env, timeout=1200,
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    log = (work / 'kh_com.log').read_text(errors='replace')
    trace = [(int(f), n) for f, n in re.findall(r'harness: frame (\d+) mode (\S+)', log)]
    progress = [line for line in log.splitlines() if 'harness: progress' in line]
    for step in steps:
        expected = 'Mode_MapFld' if step['mode'] == 1 else 'Mode_MapFix'
        observed = [name for f, name in trace if f <= step['shot']]
        marker = f"actor={args.actor} mode={step['mode']} world={step['world']} floor={step['floor']} room="
        step['observed_mode'] = observed[-1] if observed else None
        room_ok = step['room'] is None or any(
            f"floor={step['floor']} " in line and f"room={step['room']} " in line for line in progress)
        step['passed'] = result.returncode == 0 and 'CRASH' not in log and 'harness: no mode' not in log and marker in log and room_ok and (
            work / f"out_{step['shot']:05d}.bmp").exists() and step['observed_mode'] == expected
    report = dict(actor=args.actor, version=args.version, exit=result.returncode, cases=steps,
                  passed=sum(s['passed'] for s in steps), total=len(steps), scope='forced MAP entry fixtures')
    (work / 'report.json').write_text(json.dumps(report, indent=2))
    print(work)
    print(f"{report['passed']}/{report['total']} entry fixtures passed, exit={result.returncode}")
    return 0 if all(s['passed'] for s in steps) else 1


if __name__ == '__main__':
    raise SystemExit(main())
