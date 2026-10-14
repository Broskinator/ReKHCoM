#!/usr/bin/env python3
"""Isolated enemy HUD pilot smoke: redraw, absent pack, Original fallback."""
import argparse
from datetime import datetime
import os
from pathlib import Path
import shutil
import subprocess

from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--version", choices=("eu", "us", "jp"), default="eu")
    parser.add_argument("--build", type=Path, required=True)
    parser.add_argument("--riku", action="store_true")
    parser.add_argument("--reuse-build", action="store_true", help="read the isolated build executable without duplicating it")
    parser.add_argument("--language", default="es", choices=("en", "fr", "de", "it", "es"))
    args = parser.parse_args()
    build = args.build.resolve()
    work = ROOT / "build/hd-hud-check" / (args.version + "-" + datetime.now().strftime("%Y%m%d-%H%M%S-%f"))
    rom = ROOT / "roms" / {"eu": "B8CP.gba", "us": "B8CE.gba", "jp": "B8CJ.gba"}[args.version]
    work.mkdir(parents=True)
    # One executable snapshot per run; each process has its own data directory.
    if not args.reuse_build:
        for f in ("khcom.exe", "SDL3.dll"):
            shutil.copy2(build / f, work / f)
    exe = build / "khcom.exe" if args.reuse_build else work / "khcom.exe"
    for case, enabled, original, pack in (("off", 0, 0, True), ("on", 1, 0, True),
                                        ("missing", 1, 0, False), ("original-on", 1, 1, True),
                                        ("original-off", 0, 1, True)):
        d = work / case
        d.mkdir()
        if pack:
            shutil.copytree(ROOT / "port/assets", d / "assets")
        (d / "kh_com.ini").write_text(f"config_version=2\nwindow_scale=4\nvsync=0\nview_mode={0 if original else 4}\nhd_hud={enabled}\n")
        env = dict(os.environ, SDL_VIDEO_DRIVER="dummy", SDL_AUDIO_DRIVER="dummy", KHCOM_FAST="1",
                   KHCOM_BATTLE=f"0:0:{int(args.riku)}", KHCOM_KEYS="60-63:A", KHCOM_OUTPUT_SHOT="180,380", KHCOM_QUIT="385")
        subprocess.run([str(exe), "--data-dir", str(d), "--rom", str(rom), "--language", args.language],
                       cwd=d, env=env, check=True, timeout=90,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        log = (d / "kh_com.log").read_text(errors="replace")
        assert "CRASH" not in log, case
        if case == "on":
            assert "cached transparent enemy HP pilot" in log
            assert "HD HUD: cached label" in log
        if case == "missing":
            assert "original HUD retained" in log
    for frame in (180, 380):
        name = f"out_{frame:05d}.bmp"
        def picture(case):
            with Image.open(work / case / name) as img:
                return img.convert("RGB")
        assert ImageChops.difference(picture("off"), picture("missing")).getbbox() is None
        assert ImageChops.difference(picture("original-on"), picture("original-off")).getbbox() is None
        on, off = picture("on"), picture("off")
        changed = ImageChops.difference(on, off).getbbox()
        assert changed is not None, "pilot not visible"
        diff = ImageChops.difference(on, off)
        # Only the enemy frame and numeric card ink may change in this stage.
        diff.paste((0, 0, 0), (on.width // 2, 0, on.width, on.height // 5))
        diff.paste((0, 0, 0), (0, 0, on.width // 2, on.height // 5))
        diff.paste((0, 0, 0), (0, on.height // 2, on.width // 4, on.height))
        assert diff.getbbox() is None, changed
        # Smooth gradient colours can change; the fill must retain geometry.
        def hp_ink(img):
            pixels = img.load()
            return {(x, y) for y in range(img.height // 10)
                    for x in range(img.width * 3 // 4, img.width - 120)
                    if pixels[x, y][1] - pixels[x, y][0] > 35
                    and pixels[x, y][1] - pixels[x, y][2] > 35}
        assert hp_ink(off), "fixture has no visible HP fill"
        assert hp_ink(on) == hp_ink(off), "native HP fill geometry changed"
        def player_hp_ink(img):
            pixels = img.load()
            return {(x, y) for y in range(img.height // 10)
                    for x in range(img.width // 8, img.width * 3 // 8)
                    if pixels[x, y][1] - pixels[x, y][0] > 35
                    and pixels[x, y][1] - pixels[x, y][2] > 35}
        assert player_hp_ink(off), "fixture has no visible player HP"
        assert player_hp_ink(on) == player_hp_ink(off), "player HP geometry changed"
    print(f"PASS {args.version}: HP pilot visible; missing pack and original view match baseline. Captures: {work}")


if __name__ == "__main__":
    main()
