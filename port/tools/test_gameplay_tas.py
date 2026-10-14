#!/usr/bin/env python3
"""Replay a local USA power-on VBM and record checkpoints without player saves.

Surviving a replay is not an ending assertion: inspect progress checkpoints
and screenshots for desynchronization before claiming a completed story.
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

ROOT = Path(__file__).resolve().parents[2]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--build', default='build/codex-gameplay-us')
    ap.add_argument('--movie', type=Path, default=ROOT.parent / 'Kingdom Hearts - Chain of Memories (U).vbm')
    ap.add_argument('--frames', type=int, default=6000)
    ap.add_argument('--offset', type=int, default=0)
    args = ap.parse_args()
    movie = args.movie.resolve()
    data = movie.read_bytes()
    if data[:4] != b'VBM\x1a' or data[0x35:0x38] != b'8CE':
        ap.error('Expected local USA VBM')
    start = struct.unpack_from('<I', data, 0x3c)[0]
    available = (len(data) - start) // 2
    frames = min(args.frames, available)
    work = ROOT / 'build/codex-gameplay-tas' / datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')
    work.mkdir(parents=True)
    shutil.copy2(ROOT / args.build / 'khcom.exe', work / 'khcom.exe')
    shutil.copy2(ROOT.parent / 'toolchain/SDL3-3.4.18/i686-w64-mingw32/bin/SDL3.dll', work / 'SDL3.dll')
    (work / 'kh_com.ini').write_text('config_version=2\nview_mode=0\nwindow_scale=1\nhd_text=0\n')
    checks = list(range(1000, frames, 1000)) + [frames]
    env = {k: v for k, v in os.environ.items() if not k.startswith(('KHCOM_', 'SDL_'))}
    env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1', KHCOM_TRACE_MODES='1',
               KHCOM_VBM=str(movie), KHCOM_VBM_OFFSET=str(args.offset), KHCOM_QUIT=str(frames+1),
               KHCOM_MODE=','.join(f'{f}:progress' for f in checks),
               KHCOM_OUTPUT_SHOT=','.join(map(str, checks)))
    result = subprocess.run([str(work / 'khcom.exe'), '--rom', str(ROOT / 'roms/B8CE.gba')],
                            cwd=work, env=env, timeout=3600, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    log = (work / 'kh_com.log').read_text(errors='replace')
    report = dict(movie_sha256=hashlib.sha256(data).hexdigest(), frames=frames, available_frames=available,
                  offset=args.offset, exit=result.returncode, crash='CRASH' in log,
                  checkpoints=[line for line in log.splitlines() if 'harness: progress' in line],
                  full_story_verified=False)
    (work / 'report.json').write_text(json.dumps(report, indent=2))
    print(work)
    print(json.dumps(report, indent=2))
    return 0 if result.returncode == 0 and 'CRASH' not in log else 1


if __name__ == '__main__':
    raise SystemExit(main())
