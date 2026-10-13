#!/usr/bin/env python3
"""Play a .vbm input movie in BizHawk (minimised) and save screenshots.

    python port/tools/bizhawk_vbm.py --bizhawk ../toolchain/bizhawk/EmuHawk.exe --vbm movie.vbm --out build/tas-bh

Compare with the PC port playing the same movie:
    KHCOM_VBM=movie.vbm KHCOM_OUTPUT_SHOT=600,1200,... khcom.exe
"""
import argparse
import os
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bizhawk", required=True)
    ap.add_argument("--vbm", required=True)
    ap.add_argument("--rom", default=str(ROOT / "roms" / "B8CE.gba"))
    ap.add_argument("--out", default=str(ROOT / "build" / "tas-bh"))
    ap.add_argument("--every", type=int, default=600)
    ap.add_argument("--frames", type=int, default=0)
    ap.add_argument("--timeout", type=int, default=14400)
    args = ap.parse_args()

    out = Path(args.out).resolve()
    out.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, KHCOM_BH_VBM=str(Path(args.vbm).resolve()), KHCOM_BH_OUT=str(out),
               KHCOM_BH_EVERY=str(args.every), KHCOM_BH_FRAMES=str(args.frames))
    startup = subprocess.STARTUPINFO()
    startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    startup.wShowWindow = 6  # SW_MINIMIZE: keep test runs off the screen
    lua = ROOT / "port" / "tools" / "bizhawk_vbm.lua"
    proc = subprocess.Popen([args.bizhawk, "--lua=" + str(lua), str(Path(args.rom).resolve())],
                            cwd=str(Path(args.bizhawk).parent), env=env, startupinfo=startup)
    try:
        proc.wait(timeout=args.timeout)
    except subprocess.TimeoutExpired:
        proc.kill()
        print("timeout")
    print(f"{len(list(out.glob('bh_*.png')))} screenshots in {out}")


if __name__ == "__main__":
    main()
