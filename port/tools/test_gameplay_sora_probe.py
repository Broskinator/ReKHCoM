"""Continue natural Sora from an unchanged native checkpoint with buttons only.

The exact executable is preserved. State is read only; no debug mode request,
memory patch, inventory assignment, enemy kill or progress injection is used.
"""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess

from test_gameplay_observe_state import State

ROOT = Path(__file__).resolve().parents[2]


def observe(exe, checkpoint):
    state = State(exe, checkpoint)
    report = state.snapshot()
    if report['mode'] == 'mode_battle' and report['battle'].get('tutorial'):
        descriptor = state.symbol('gTaskDescTutorial')
        needle = struct.pack('<I', descriptor)
        tasks = []
        for base, size, offset in state.regions:
            if base != 0x02000000:
                continue
            memory = state.raw[offset:offset + size]
            start = 0
            while (found := memory.find(needle, start)) >= 0:
                start = found + 4
                work = struct.unpack_from('<I', memory, found + 4)[0]
                if 0x02000000 <= work <= 0x0203ffe8:
                    flags, message, phase, following, timer, count, cooldown = struct.unpack(
                        '<HHIIhhh', state.read(work, 18))
                    if phase <= 70 and following <= 70:
                        tasks.append(dict(state=phase, next_state=following, timer=timer,
                                          count=count, cooldown=cooldown, message=message, flags=flags))
        report['tutorial_tasks'] = tasks
    return report


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--build', type=Path, default=ROOT/'build/codex-gameplay-tas/20261014_091214_546195')
    ap.add_argument('--resume', type=Path, default=ROOT/'build/codex-gameplay-tas/20261014_091214_546195/tas_009000.khst')
    ap.add_argument('--keys', default='', help='ordinary local-frame KHCOM_KEYS schedule')
    ap.add_argument('--frames', type=int, default=500)
    ap.add_argument('--checks', default='', help='comma-separated local-frame snapshots/screenshots')
    args = ap.parse_args()
    original = args.resume.read_bytes()
    initial = observe(args.build/'khcom.exe', args.resume)
    if initial['actor'] != 'sora':
        ap.error('Expected natural Sora checkpoint')
    work = ROOT/'build/codex-sora-probe'/datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')
    work.mkdir(parents=True)
    shutil.copy2(args.build/'khcom.exe', work/'khcom.exe')
    dll = args.build/'SDL3.dll'
    if not dll.exists():
        dll = ROOT.parent/'toolchain/SDL3-3.4.18/i686-w64-mingw32/bin/SDL3.dll'
    shutil.copy2(dll, work/'SDL3.dll')
    shutil.copy2(args.resume, work/'resume.khst')
    (work/'kh_com.ini').write_text('config_version=2\nview_mode=0\nhd_text=0\nwindow_scale=2\n')
    checks = sorted(set([int(x) for x in args.checks.split(',') if x] + [args.frames]))
    if min(checks) < 1 or max(checks) > args.frames:
        ap.error('checks must be within 1..frames')
    env = {k:v for k,v in os.environ.items() if not k.startswith(('KHCOM_', 'SDL_'))}
    env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1',
               KHCOM_TRACE_MODES='1', KHCOM_LOADSTATE='resume.khst', KHCOM_KEYS=args.keys,
               KHCOM_OUTPUT_SHOT=','.join(map(str, checks)), KHCOM_QUIT=str(args.frames+2),
               KHCOM_SAVESTATE=','.join(f'{frame}:checkpoint_{frame}.khst' for frame in checks))
    result = subprocess.run([str(work/'khcom.exe'), '--rom', str(ROOT/'roms/B8CE.gba')],
                            cwd=work, env=env, timeout=300, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    log = (work/'kh_com.log').read_text(errors='replace')
    loaded = any('state: load ' in line and line.endswith(' -> 1') for line in log.splitlines())
    snapshots = [dict(observe(work/'khcom.exe', work/f'checkpoint_{frame}.khst'), requested_frame=frame)
                 for frame in checks if (work/f'checkpoint_{frame}.khst').exists()]
    report = dict(exit=result.returncode, crash='CRASH' in log, resume_loaded=loaded, initial=initial,
                  resumed_from=str(args.resume.resolve()), source_state_sha256=hashlib.sha256(original).hexdigest(),
                  executable_sha256=hashlib.sha256((work/'khcom.exe').read_bytes()).hexdigest(),
                  source_state_unchanged=args.resume.read_bytes()==original, keys=args.keys,
                  snapshots=snapshots, full_campaign_verified=False,
                  scope='Unmodified checkpoint plus ordinary buttons; read-only observation')
    (work/'report.json').write_text(json.dumps(report, indent=2))
    print(work)
    print(json.dumps(report, indent=2))
    return 0 if result.returncode == 0 and loaded and not report['crash'] and len(snapshots)==len(checks) else 1


if __name__ == '__main__':
    raise SystemExit(main())
