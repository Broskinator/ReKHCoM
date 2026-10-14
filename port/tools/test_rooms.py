#!/usr/bin/env python3
"""Room battery: every world and every castle hall, entered as the debug
menu's MAP check does (KHCOM_MODE=<frame>:room:<mode*10000+world*100+floor>).

  mode 1  the floor's world (13 worlds, on an early and a late floor)
  mode 2  the castle's entrance hall of each floor
  mode 3  the castle's exit hall of each floor

In each run Sora walks in turn to each side and presses A now and then, so
rooms are crossed, door menus open and battles start. A case fails when the
process does not exit normally or the log reports a fault. Shots at the
middle and the end go to a contact sheet for the widescreen review.

    python port/tools/test_rooms.py [--version eu] [--build build/pc] [--jobs 4] [--frames 3000]
"""
import argparse
import os
import shutil
import subprocess
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
WORLDS = 13
FLOORS = 13
START = 300


def cases():
    out = []
    for world in range(WORLDS):
        for floor in (0, 9):
            out.append((1, world, floor))
    for floor in range(FLOORS):
        out.append((2, 0, floor))
        out.append((3, 0, floor))
    return out


def keys(frames):
    dirs = ("RIGHT", "DOWN", "LEFT", "UP", "RIGHT+DOWN", "LEFT+UP")
    out = []
    f, n = START + 120, 0
    while f < frames:
        out.append(f"{f}-{f + 90}:{dirs[n % len(dirs)]}")
        out.append(f"{f + 95}-{f + 98}:A")
        f += 130
        n += 1
    return ",".join(out)


def run(exe_dir, work, case, frames, timeout):
    mode, world, floor = case
    name = f"room_{mode}_{world:02d}_{floor:02d}"
    d = work / name
    shutil.rmtree(d, ignore_errors=True)
    d.mkdir(parents=True)
    for f in ("khcom.exe", "SDL3.dll"):
        shutil.copy2(exe_dir / f, d / f)
    (d / "kh_com.ini").write_text("config_version=2\nview_mode=4\n")
    mid = START + (frames - START) // 2
    env = dict(os.environ, SDL_VIDEO_DRIVER="dummy", SDL_AUDIO_DRIVER="dummy", KHCOM_FAST="1",
               KHCOM_MODE=f"{START}:room:{mode * 10000 + world * 100 + floor}", KHCOM_KEYS=keys(frames),
               KHCOM_SHOTS=f"{mid},{frames}", KHCOM_QUIT=str(frames + 1), KHCOM_TRACE_MODES="1")
    try:
        code = subprocess.run([str(d / "khcom.exe")], cwd=str(d), env=env, timeout=timeout,
                              stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode
    except subprocess.TimeoutExpired:
        code = "timeout"
    log = (d / "kh_com.log").read_text(errors="replace") if (d / "kh_com.log").exists() else ""
    faults = [l for l in log.splitlines() if "fatal" in l.lower() or "crash" in l.lower() or "exception" in l.lower()]
    modes = []
    for l in log.splitlines():
        if l.startswith("harness: frame") and " mode " in l:
            m = l.split(" mode ")[1]
            if m not in modes:
                modes.append(m)
    status = "ok" if code == 0 and not faults else (f"exit {code}" if code != 0 else "fault")
    shots = [d / f"shot_{mid:05d}.bmp", d / f"shot_{frames:05d}.bmp"]
    return name, status, modes, faults[:2], [s for s in shots if s.exists()]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--version", default="eu", choices=("eu", "us", "jp"))
    ap.add_argument("--build", help="folder with khcom.exe (default build/pc or build/pc-<version>)")
    ap.add_argument("--jobs", type=int, default=4)
    ap.add_argument("--frames", type=int, default=3000)
    ap.add_argument("--timeout", type=int, default=600)
    args = ap.parse_args()
    exe_dir = ROOT / (args.build or ("build/pc" if args.version == "eu" else f"build/pc-{args.version}"))
    # two levels under the repository, where the exe finds ../../roms
    work = ROOT / "build" / f"roomtest_{args.version}"
    with ThreadPoolExecutor(args.jobs) as pool:
        results = list(pool.map(lambda c: run(exe_dir, work.parent, c, args.frames, args.timeout), cases()))
    lines, thumbs = [], []
    for name, status, modes, faults, shots in results:
        lines.append(f"{name:18s} {status:10s} {' > '.join(modes)}{'  ' + ' | '.join(faults) if faults else ''}")
        thumbs.append((name, shots))
    ok = sum(1 for r in results if r[1] == "ok")
    lines.append(f"{ok}/{len(results)} rooms ok")
    work.mkdir(parents=True, exist_ok=True)
    (work / "report.txt").write_text("\n".join(lines) + "\n")
    # contact sheet: two shots per case
    tw, th, cols = 160, 90, 6
    sheet = Image.new("RGB", (tw * cols, th * ((len(thumbs) * 2 + cols - 1) // cols)), "black")
    k = 0
    for name, shots in thumbs:
        for s in shots:
            sheet.paste(Image.open(s).convert("RGB").resize((tw, th)), ((k % cols) * tw, (k // cols) * th))
            k += 1
    sheet.save(work / "rooms.png")
    for name, _ in thumbs:
        shutil.rmtree(work.parent / name, ignore_errors=True)
    print("\n".join(l for l in lines if " ok " not in l))
    print(f"report: {work / 'report.txt'}")


if __name__ == "__main__":
    main()
