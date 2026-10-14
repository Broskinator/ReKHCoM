#!/usr/bin/env python3
"""Hidden, isolated regression fixtures for fixed-room saves and spotlight.

The castle fixture opens the real save task directly, checks crystal task
counts and captures it before/after. It is not a recorded cartridge playthrough.
"""
import argparse
import datetime
import os
from pathlib import Path
import re
import shutil
import subprocess
from concurrent.futures import ThreadPoolExecutor

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", default="build/pcpriority")
    parser.add_argument("--save", default="build/pc/kh_com.sav")
    parser.add_argument("--cycles", type=int, default=12)
    parser.add_argument("--timeout", type=int, default=300)
    args = parser.parse_args()
    if args.cycles < 1:
        parser.error("--cycles must be positive")
    work = ROOT / "build/priority-regressions" / datetime.datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    work.mkdir(parents=True)

    def run(case):
        folder = work / case
        folder.mkdir()
        for name in ("khcom.exe", "SDL3.dll"):
            shutil.copy2(ROOT / args.build / name, folder / name)
        shutil.copy2(ROOT / args.save, folder / "kh_com.sav")
        shutil.copytree(ROOT / "port/assets/fonts", folder / "assets/fonts")
        (folder / "kh_com.ini").write_text("config_version=2\nview_mode=4\nwindow_scale=2\n")
        keys = [f"{frame}-{frame + 3}:A" for frame in range(200, 1400, 50)]
        if case == "castle":
            modes = ["1400:castle", "1510:crystalview", "1580:gmkcount"]
            shots = [1580]
            end = 1600 + args.cycles * 300 + 100
            for cycle in range(args.cycles):
                frame = 1600 + cycle * 300
                modes.extend((f"{frame}:mapsave", f"{frame + 240}:gmkcount"))
                keys.append(f"{frame + 140}-{frame + 143}:B")
                shots.append(frame + 260)
        elif case == "spotlight":
            modes, shots, end = ["1400:spotlight"], [1445, 1480, 1520], 1540
        elif case == "doorselect":
            modes, shots, end = ["1400:spotlight", "1480:doorselect"], [1560, 1640, 1760], 1800
        else:
            modes, shots, end = ["1400:mode_status"], [1700, 1800, 1900], 1920
            keys.extend(("1720-1723:UP", "1760-1763:UP", "1840-1843:DOWN"))
        env = dict(os.environ, SDL_VIDEO_DRIVER="dummy", SDL_AUDIO_DRIVER="dummy", KHCOM_FAST="1",
                   KHCOM_KEYS=",".join(keys), KHCOM_MODE=",".join(modes), KHCOM_TRACE_MODES="1",
                   KHCOM_OUTPUT_SHOT=",".join(map(str, shots)), KHCOM_QUIT=str(end))
        try:
            result = subprocess.run([str(folder / "khcom.exe"), "--rom", str(ROOT / "roms/B8CP.gba"),
                                     "--language", "es"], cwd=folder, env=env, timeout=args.timeout,
                                    stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            code = result.returncode
        except subprocess.TimeoutExpired:
            code = "timeout"
        log = (folder / "kh_com.log").read_text(errors="replace")
        passed = code == 0 and "CRASH" not in log and all((folder / f"out_{frame:05d}.bmp").exists() for frame in shots)
        counts = re.findall(r"harness: save crystals (\d+)", log)
        if case == "castle":
            passed = passed and len(counts) >= args.cycles + 1 and set(counts) == {"1"}
        if case in ("spotlight", "doorselect"):
            from PIL import Image
            for frame in shots:
                path = folder / f"out_{frame:05d}.bmp"
                if not path.exists():
                    continue
                pic = Image.open(path).convert("RGB")
                margin = int((pic.width - pic.height * 240 / 160) / 2)
                for box in ((0, 32, margin, pic.height), (pic.width - margin, 32, pic.width, pic.height)):
                    for _, (red, green, blue) in pic.crop(box).getcolors(pic.width * pic.height) or []:
                        if red < 80 and green > 180 and blue > 180:
                            passed = False
        report = f"{case}: exit={code}, crystals={counts}, {'PASS' if passed else 'FAIL'}\n"
        (folder / "report.txt").write_text(report)
        return report, passed

    with ThreadPoolExecutor(3) as pool:
        results = list(pool.map(run, ("castle", "spotlight", "doorselect", "status")))
    report = "".join(item[0] for item in results)
    (work / "report.txt").write_text(report)
    print(work)
    print(report)
    return 0 if all(item[1] for item in results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
