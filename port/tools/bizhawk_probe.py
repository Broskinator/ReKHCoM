#!/usr/bin/env python3
"""Compare field movement with the original cartridge in BizHawk.

Plays the speedrun movie (.vbm, USA) to --stop, logging the player's position
at --addr (EWRAM; the PC port prints it with KHCOM_POSLOG) with screenshots.
With --set x,y,z it then places the player there and holds --hold, logging the
position, like the port does from a savestate (issue 63: stuck behind the
crystal). Output: <out>/probe.log and bh_*.png.

    python port/tools/bizhawk_probe.py --stop 9000 --every 60
    python port/tools/bizhawk_probe.py --stop 7200 --set 69271,59228,0 --hold Left
"""
import argparse
import os
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--bizhawk', default=str(ROOT.parent / 'toolchain/bizhawk/EmuHawk.exe'))
    ap.add_argument('--vbm', default=str(ROOT.parent / 'Kingdom Hearts - Chain of Memories (U).vbm'))
    ap.add_argument('--rom', default=str(ROOT / 'roms/B8CE.gba'))
    ap.add_argument('--out', default=str(ROOT / 'build/bh-probe'))
    ap.add_argument('--addr', default='0x02033EF8')
    ap.add_argument('--stop', type=int, default=6000)
    ap.add_argument('--every', type=int, default=30)
    ap.add_argument('--set')
    ap.add_argument('--hold', default='Left')
    ap.add_argument('--hold-frames', type=int, default=60)
    ap.add_argument('--timeout', type=int, default=1800)
    args = ap.parse_args()
    out = Path(args.out).resolve()
    out.mkdir(parents=True, exist_ok=True)
    for old in out.glob('bh_*.png'):
        old.unlink()
    env = dict(os.environ, KHCOM_BH_VBM=str(Path(args.vbm).resolve()), KHCOM_BH_OUT=str(out),
               KHCOM_BH_EVERY=str(args.every), KHCOM_BH_STOP=str(args.stop), KHCOM_BH_ADDR=args.addr,
               KHCOM_BH_HOLD=args.hold, KHCOM_BH_HOLDFRAMES=str(args.hold_frames))
    if args.set:
        env['KHCOM_BH_SET'] = args.set
    startup = subprocess.STARTUPINFO()
    startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    startup.wShowWindow = 6  # minimised
    proc = subprocess.Popen([args.bizhawk, '--lua=' + str(ROOT / 'port/tools/bizhawk_probe.lua'),
                             str(Path(args.rom).resolve())],
                            cwd=str(Path(args.bizhawk).parent), env=env, startupinfo=startup)
    try:
        proc.wait(timeout=args.timeout)
    except subprocess.TimeoutExpired:
        proc.kill()
        print('timeout')
    print((out / 'probe.log').read_text() if (out / 'probe.log').exists() else 'no log')


if __name__ == '__main__':
    main()
