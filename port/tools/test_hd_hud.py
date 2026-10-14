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
    args = parser.parse_args()
    build = args.build.resolve()
    work = ROOT / "build/hd-hud-check" / (args.version + "-" + datetime.now().strftime("%Y%m%d-%H%M%S-%f"))
    rom = ROOT / "roms" / {"eu": "B8CP.gba", "us": "B8CE.gba", "jp": "B8CJ.gba"}[args.version]
    for case, enabled, original, pack in (("off", 0, 0, True), ("on", 1, 0, True),
                                        ("missing", 1, 0, False), ("original-on", 1, 1, True),
                                        ("original-off", 0, 1, True)):
        d = work / case
        d.mkdir(parents=True)
        for f in ("khcom.exe", "SDL3.dll"):
            shutil.copy2(build / f, d / f)
        if pack:
            shutil.copytree(ROOT / "port/assets", d / "assets")
        (d / "kh_com.ini").write_text(f"config_version=2\nwindow_scale=4\nvsync=0\nview_mode={0 if original else 4}\nhd_hud={enabled}\n")
        env = dict(os.environ, SDL_VIDEO_DRIVER="dummy", SDL_AUDIO_DRIVER="dummy", KHCOM_FAST="1",
                   KHCOM_BATTLE="0:0:0", KHCOM_KEYS="60-63:A", KHCOM_OUTPUT_SHOT="180,380", KHCOM_QUIT="385")
        subprocess.run([str(d / "khcom.exe"), "--rom", str(rom), "--language", "es"],
                       cwd=d, env=env, check=True, timeout=90,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        log = (d / "kh_com.log").read_text(errors="replace")
        assert "CRASH" not in log, case
        if case == "on":
            assert "cached transparent enemy HP pilot" in log
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
        diff.paste((0, 0, 0), (0, on.height // 2, on.width // 4, on.height))
        assert diff.getbbox() is None, changed
        # The green HP fill/lettering must retain positions, size and colours,
        # not merely stay somewhere in the same corner after an HD redraw.
        def hp_ink(img):
            pixels = img.load()
            return {(x, y, pixels[x, y]) for y in range(img.height // 10)
                    for x in range(img.width * 3 // 4, img.width)
                    if pixels[x, y][1] - pixels[x, y][0] > 35
                    and pixels[x, y][1] - pixels[x, y][2] > 35}
        assert hp_ink(off), "fixture has no visible HP fill/lettering"
        assert hp_ink(on) == hp_ink(off), "native HP geometry or colours changed"
    print(f"PASS {args.version}: HP pilot visible; missing pack and original view match baseline. Captures: {work}")


if __name__ == "__main__":
    main()
