#!/usr/bin/env python3
"""Submenu stress: every entry of the pause menu, entered and left several
times through real START navigation, on a copy of a save.

For each entry k (k presses of DOWN after START), one run repeats: START,
k x DOWN, A, a while inside, then B until the field is back. A run passes when
the process exits normally, nothing crashed and the mode trace shows, for
each cycle, a mode other than the field followed by the field again. Each
entry runs in its own process; one at a time so it stays light.

    python port/tools/test_submenus.py [--build build/pc] [--save build/pc/kh_com.sav] [--entries 6] [--cycles 4]
"""
import argparse
import os
import re
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PERIOD = 700


def run(build, save, entry, cycles, timeout):
    # two levels under the repository, where the exe finds ../../roms
    work = ROOT / "build" / f"submenu_{entry}"
    shutil.rmtree(work, ignore_errors=True)
    work.mkdir(parents=True)
    for f in ("khcom.exe", "SDL3.dll"):
        shutil.copy2(build / f, work / f)
    shutil.copy2(save, work / "kh_com.sav")
    (work / "kh_com.ini").write_text("config_version=2\nview_mode=4\n")
    keys = [f"{f}-{f + 3}:A" for f in range(200, 1400, 50)]
    for c in range(cycles):
        f = 1600 + c * PERIOD
        keys.append(f"{f}-{f + 3}:START")
        for d in range(entry):
            keys.append(f"{f + 60 + d * 20}-{f + 63 + d * 20}:DOWN")
        a = f + 80 + entry * 20
        keys.append(f"{a}-{a + 3}:A")
        for b in (a + 250, a + 360, a + 470):
            keys.append(f"{b}-{b + 3}:B")
    end = 1600 + cycles * PERIOD
    env = dict(os.environ, SDL_VIDEO_DRIVER="dummy", SDL_AUDIO_DRIVER="dummy", KHCOM_FAST="1",
               KHCOM_TRACE_MODES="1", KHCOM_KEYS=",".join(keys), KHCOM_QUIT=str(end))
    try:
        code = subprocess.run([str(work / "khcom.exe"), "--language", "es"], cwd=str(work), env=env,
                              timeout=timeout, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode
    except subprocess.TimeoutExpired:
        code = "timeout"
    log = (work / "kh_com.log").read_text(errors="replace") if (work / "kh_com.log").exists() else ""
    trace = [(int(f), m) for f, m in re.findall(r"harness: frame (\d+) mode (\S+)", log) if int(f) >= 1600]
    # a visit: leaving the field for some mode and coming back
    visits, inside, names = 0, False, []
    for f, m in trace:
        if m != "Mode_MapFld" and not inside:
            inside = True
            if m not in names:
                names.append(m)
        elif m == "Mode_MapFld" and inside:
            inside = False
            visits += 1
    crashed = "CRASH" in log or "fatal" in log.lower()
    ok = code == 0 and not crashed and visits >= cycles
    shutil.rmtree(work, ignore_errors=True) if ok else None
    return entry, ok, code, visits, names


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--build", default=str(ROOT / "build" / "pc"))
    ap.add_argument("--save", default=str(ROOT / "build" / "pc" / "kh_com.sav"))
    ap.add_argument("--entries", type=int, default=6)
    ap.add_argument("--cycles", type=int, default=4)
    ap.add_argument("--timeout", type=int, default=600)
    args = ap.parse_args()
    lines = []
    results = [run(Path(args.build), Path(args.save), e, args.cycles, args.timeout) for e in range(args.entries)]
    for entry, ok, code, visits, names in results:
        lines.append(f"entry {entry}: {'ok ' if ok else 'FAIL'} exit {code}, {visits}/{args.cycles} visits, "
                     f"modes {', '.join(names) or '-'}")
    lines.append(f"{sum(r[1] for r in results)}/{len(results)} entries ok")
    out = ROOT / "build" / "submenutest"
    out.mkdir(parents=True, exist_ok=True)
    (out / "report.txt").write_text("\n".join(lines) + "\n")
    print("\n".join(lines))


if __name__ == "__main__":
    main()
