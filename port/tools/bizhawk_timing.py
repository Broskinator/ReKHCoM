#!/usr/bin/env python3
"""Run port/tools/bizhawk_timing.lua in BizHawk (minimised) and print the log.

    python port/tools/bizhawk_timing.py --bizhawk ../toolchain/bizhawk/EmuHawk.exe [--frames 26000]
"""
import argparse
import os
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bizhawk", required=True)
    ap.add_argument("--rom", default=str(ROOT / "roms" / "B8CP.gba"))
    ap.add_argument("--frames", type=int, default=26000)
    ap.add_argument("--out", default=str(ROOT / "build" / "bizhawk_timing.log"))
    ap.add_argument("--timeout", type=int, default=1800)
    ap.add_argument("--show", action="store_true", help="visible window at real-time speed")
    args = ap.parse_args()

    out = Path(args.out)
    out.unlink(missing_ok=True)
    env = dict(os.environ, KHCOM_BH_OUT=str(out), KHCOM_BH_FRAMES=str(args.frames),
               KHCOM_BH_SPEED="100" if args.show else "6400")
    lua = ROOT / "port" / "tools" / "bizhawk_timing.lua"
    startup = subprocess.STARTUPINFO()
    startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    startup.wShowWindow = 1 if args.show else 6  # SW_MINIMIZE: keep test runs off the screen
    proc = subprocess.Popen([args.bizhawk, "--lua=" + str(lua), args.rom],
                            cwd=str(Path(args.bizhawk).parent), env=env, startupinfo=startup)
    try:
        proc.wait(timeout=args.timeout)
    except subprocess.TimeoutExpired:
        proc.kill()
        print("timeout")
    if out.exists():
        print(out.read_text())


if __name__ == "__main__":
    main()
