#!/usr/bin/env python3
"""Exercise real START -> deck -> START -> field transitions on a copied save.

Runs hidden, in a fresh isolated directory. A zero exit alone is not a pass:
the mode trace must show a deck entry and return to the field for each cycle.
--edit additionally navigates the popup into Add cards. Other submenus and
different saves require separate routes.
"""
import argparse
import datetime
import os
from pathlib import Path
import re
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", default="build/pcwork")
    parser.add_argument("--save", default="build/pc/kh_com.sav")
    parser.add_argument("--cycles", type=int, default=12)
    parser.add_argument("--timeout", type=int, default=600)
    parser.add_argument("--edit", action="store_true", help="Open deck editing and switch between add/remove")
    args = parser.parse_args()
    if args.cycles < 1:
        parser.error("--cycles must be positive")
    work = ROOT / "build/menu_cycles" / datetime.datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    work.mkdir(parents=True)
    for filename in ("khcom.exe", "SDL3.dll"):
        shutil.copy2(ROOT / args.build / filename, work / filename)
    shutil.copy2(ROOT / args.save, work / "kh_com.sav")
    shutil.copytree(ROOT / "port/assets/fonts", work / "assets/fonts")
    (work / "kh_com.ini").write_text("config_version=2\nview_mode=4\nhd_text=1\nwindow_scale=2\n")
    keys = [f"{frame}-{frame + 3}:A" for frame in range(200, 1400, 50)]
    shots = []
    period = 1300 if args.edit else 650
    for cycle in range(args.cycles):
        frame = 1600 + cycle * period
        if args.edit:
            keys.extend(f"{frame + offset}-{frame + offset + 3}:{button}" for offset, button in
                        ((0, "START"), (90, "A"), (240, "A"), (320, "DOWN"),
                         (350, "DOWN"), (380, "DOWN"), (440, "A"), (620, "L"),
                         (780, "B"), (950, "B"), (1120, "B")))
            shots.extend((frame + 560, frame + 730, frame + 900, frame + 1270))
            continue
        keys.extend((f"{frame}-{frame + 3}:START", f"{frame + 90}-{frame + 93}:A",
                     f"{frame + 280}-{frame + 283}:B", f"{frame + 460}-{frame + 463}:B"))
        shots.extend((frame + 240, frame + 610))
    end = 1600 + args.cycles * period
    env = dict(os.environ, SDL_VIDEO_DRIVER="dummy", SDL_AUDIO_DRIVER="dummy", KHCOM_FAST="1",
               KHCOM_TRACE_MODES="1", KHCOM_KEYS=",".join(keys), KHCOM_QUIT=str(end),
               KHCOM_OUTPUT_SHOT=",".join(map(str, shots)))
    try:
        result = subprocess.run([str(work / "khcom.exe"), "--language", "es", "--rom",
                                 str(ROOT / "roms/B8CP.gba")], cwd=work, env=env,
                                stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL,
                                stderr=subprocess.DEVNULL, timeout=args.timeout)
        code = result.returncode
    except subprocess.TimeoutExpired:
        code = "timeout"
    log = (work / "kh_com.log").read_text(errors="replace")
    trace = re.findall(r"harness: frame (\d+) mode (\S+)", log)
    deck = sum(name == "Mode_Deck" for _, name in trace)
    returns = sum(int(frame) >= 1600 and name == "Mode_MapFld" for frame, name in trace)
    passed = code == 0 and "CRASH" not in log and deck >= args.cycles and returns >= args.cycles
    report = (f"exit={code}; deck entries={deck}; field returns={returns}; requested={args.cycles}\n"
              + "\n".join(f"{frame} {name}" for frame, name in trace)
              + f"\n{'PASS' if passed else 'INCOMPLETE / FAIL'}\n")
    (work / "report.txt").write_text(report)
    print(work)
    print(report)
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
