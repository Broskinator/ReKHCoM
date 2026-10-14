#!/usr/bin/env python3
"""Save state determinism: a state saved mid-battle and loaded in a fresh
process must play on exactly like the original run.

Run A starts a debug battle, presses A every 24 frames, saves a state at
frame SAVE and takes shots. Run B loads that state with the same presses
(shifted: the first frame after a load is the original's SAVE + 1) and must
produce pixel-identical shots.

    python port/tools/test_savestate.py [--build build/pc]
"""
import argparse
import os
import shutil
import subprocess
from pathlib import Path

from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[2]
SAVE = 700
SHIFT = SAVE + 1
LATER = (1000, 1300, 1800, 2400)


def keys(shift):
    """The presses of run A, as frames of a run that starts at frame shift."""
    out = ["60-63:A"] if shift == 0 else []
    for f in range(300, LATER[-1], 24):
        if f >= shift + 3:
            out.append(f"{f - shift}-{f - shift + 3}:A")
    return ",".join(out)


def run(build, d, env):
    shutil.rmtree(d, ignore_errors=True)
    d.mkdir(parents=True)
    for f in ("khcom.exe", "SDL3.dll"):
        shutil.copy2(build / f, d / f)
    (d / "kh_com.ini").write_text("view_mode=0\n")
    return d, env


def case(build, work, name, battle):
    base = dict(os.environ, SDL_VIDEO_DRIVER="dummy", SDL_AUDIO_DRIVER="dummy", KHCOM_FAST="1")
    # two levels under the repository, where the exe finds ../../roms
    a = work / f"statetest_{name}_a"
    b = work / f"statetest_{name}_b"
    run(build, a, None)
    subprocess.run([str(a / "khcom.exe")], cwd=str(a), timeout=600, stdout=subprocess.DEVNULL,
                   env=dict(base, KHCOM_BATTLE=battle, KHCOM_KEYS=keys(0), KHCOM_SAVESTATE=f"{SAVE}:s.khst",
                            KHCOM_SHOTS=",".join(map(str, LATER)), KHCOM_QUIT=str(LATER[-1] + 1)))
    run(build, b, None)
    shutil.copy2(a / "s.khst", b / "s.khst")
    subprocess.run([str(b / "khcom.exe")], cwd=str(b), timeout=600, stdout=subprocess.DEVNULL,
                   env=dict(base, KHCOM_LOADSTATE="s.khst", KHCOM_KEYS=keys(SHIFT),
                            KHCOM_SHOTS=",".join(str(f - SHIFT) for f in LATER), KHCOM_QUIT=str(LATER[-1] - SHIFT + 1)))
    ok = True
    for f in LATER:
        pa, pb = a / f"shot_{f:05d}.bmp", b / f"shot_{f - SHIFT:05d}.bmp"
        same = pa.exists() and pb.exists() and \
            ImageChops.difference(Image.open(pa).convert("RGB"), Image.open(pb).convert("RGB")).getbbox() is None
        ok &= same
    print(f"{'PASS' if ok else 'FAIL'}  {name}")
    return ok


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--build", default=str(ROOT / "build" / "pc"))
    ap.add_argument("--out", default=str(ROOT / "build"))
    args = ap.parse_args()
    build, work = Path(args.build), Path(args.out)
    results = [case(build, work, "sora_battle", "10:4:0"), case(build, work, "riku_battle", "10:4:1"),
               case(build, work, "boss_battle", "160:9:0")]
    print(f"{sum(results)}/{len(results)} passed")
    return 0 if all(results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
