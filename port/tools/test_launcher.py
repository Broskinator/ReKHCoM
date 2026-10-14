#!/usr/bin/env python3
"""Launcher (ReKHCoM.exe) install tests, run headless in fresh folders.

Each case copies the launcher into build/launchertest/<case>, prepares the
folder, starts it with DECOMPUI_INSTALL=<file> and DECOMPUI_SHOT=<dir> (hidden
window: renders every page to PNG and exits) and checks what was installed.

    python port/tools/test_launcher.py [--build build/pc] [--roms roms]
"""
import argparse
import hashlib
import os
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CODES = {"eu": "B8CP", "us": "B8CE", "jp": "B8CJ"}


def run(build, work, name, prepare, install, expect):
    d = work / name
    shutil.rmtree(d, ignore_errors=True)
    d.mkdir(parents=True)
    for f in ("ReKHCoM.exe", "SDL3.dll"):
        shutil.copy2(build / f, d / f)
    shutil.copytree(build / "assets", d / "assets")
    src = prepare(d)
    env = dict(os.environ, DECOMPUI_SHOT=str(d / "shots"))
    if install:
        env["DECOMPUI_INSTALL"] = str(src)
    (d / "shots").mkdir()
    r = subprocess.run([str(d / "ReKHCoM.exe")], cwd=str(d), env=env, timeout=120)
    roms = d / "roms"
    got = sorted(p.name for p in roms.iterdir()) if roms.is_dir() else []
    ok = r.returncode == 0 and got == expect(d)
    for p in got:
        if p.endswith(".gba"):
            code = p[:4]
            want = [v for v, c in CODES.items() if c == code]
            ok &= bool(want) and (roms / p).read_bytes() == (ROMS / f"{code}.gba").read_bytes()
    print(f"{'PASS' if ok else 'FAIL'}  {name:22s} exit={r.returncode} roms={got}")
    return ok


def main():
    global ROMS
    ap = argparse.ArgumentParser()
    ap.add_argument("--build", default=str(ROOT / "build" / "pc"))
    ap.add_argument("--roms", default=str(ROOT / "roms"))
    ap.add_argument("--out", default=str(ROOT / "build" / "launchertest"))
    args = ap.parse_args()
    build, ROMS, work = Path(args.build), Path(args.roms), Path(args.out)

    def copy_rom(code, mutate=None):
        def prep(d):
            data = bytearray((ROMS / f"{code}.gba").read_bytes())
            if mutate:
                mutate(data)
            p = d / "dump.gba"
            p.write_bytes(data)
            return p
        return prep

    def flip(data):
        data[-1] ^= 0xFF

    def blob(size):
        def prep(d):
            p = d / "dump.gba"
            p.write_bytes(bytes(size))
            return p
        return prep

    def roms_is_file(d):
        (d / "roms").write_text("not a folder")
        return copy_rom("B8CP")(d)

    def english(d):
        (d / "decompui.ini").write_text("ui_language=2\n")

    def preinstalled(d):
        (d / "roms").mkdir()
        shutil.copy2(ROMS / "B8CP.gba", d / "roms" / "B8CP.gba")
        return copy_rom("B8CP")(d)

    results = []
    for v, code in CODES.items():
        results.append(run(build, work, f"install_{v}", copy_rom(code), True, lambda d, c=code: [f"{c}.gba"]))
    results += [
        run(build, work, "reinstall_eu", preinstalled, True, lambda d: ["B8CP.gba"]),
        run(build, work, "too_small", blob(1024), True, lambda d: []),
        run(build, work, "bad_sha1", copy_rom("B8CP", flip), True, lambda d: []),
        run(build, work, "not_khcom_32mib", blob(32 << 20), True, lambda d: []),
        run(build, work, "missing_file", lambda d: d / "nothing.gba", True, lambda d: []),
        run(build, work, "clean_start", lambda d: None, False, lambda d: []),
        run(build, work, "ui_english", english, False, lambda d: []),
    ]
    # roms is a file: nothing can be installed and the launcher must not crash
    d = work / "roms_unwritable"
    ok = run(build, work, "roms_unwritable", roms_is_file, True, lambda d: [])
    ok &= (d / "roms").is_file() and not list(d.glob("roms*.tmp"))
    results.append(ok)
    print(f"{sum(results)}/{len(results)} passed")
    return 0 if all(results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
