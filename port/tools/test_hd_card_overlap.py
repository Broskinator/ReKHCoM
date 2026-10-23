"""Isolated capture regression for the stale native-frame/HD-card overlap.
Requires Pillow and a local ROM + Kingdom Key replacement; no game assets are bundled.
Usage: python port/tools/test_hd_card_overlap.py --exe build/pc/khcom.exe --rom roms/B8CP.gba --art ../hd-assets/pack --out build/card-overlap-regression
"""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
from PIL import Image

ap = argparse.ArgumentParser(description=__doc__)
for key in ("exe", "rom", "art", "out"):
    ap.add_argument("--"+key, type=Path, required=True)
a = ap.parse_args()
exe, rom, art, out = (p.resolve() for p in (a.exe, a.rom, a.art, a.out))
for name, driver, enabled, keys in (
    ("native", "direct3d11", 0, "60-70:A,100-110:A"),
    ("hd", "direct3d11", 1, "60-70:A,100-110:A"),
    ("software", "software", 1, "60-70:A,100-110:A"),
    ("cycle", "direct3d11", 1, "60-70:A,100-110:A,180-185:L,210-215:R"),
):
    folder = out / name
    folder.mkdir(parents=True, exist_ok=True)
    # Refuse an existing player's save/config directory; regression folders are reusable.
    marker = folder / ".card-overlap-test"
    if not marker.exists() and any(folder.iterdir()):
        raise RuntimeError("Test output must be empty or marked as a regression folder")
    marker.touch()
    shutil.copy2(exe, folder / "khcom.exe")
    shutil.copy2(exe.parent / "SDL3.dll", folder / "SDL3.dll")
    shutil.copytree(art, folder / "hd/sprites", dirs_exist_ok=True)
    (folder / "kh_com.ini").write_text(
        "config_version=11\nfilter=0\nfullscreen=0\ndiscord_presence=0\nhd_text=0\n"
        "experimental_frame_skip=0\nhd_portraits=0\n" + f"hd_cards={enabled}\n")
    env = dict(os.environ, SDL_RENDER_DRIVER=driver, KHCOM_FAST="1", KHCOM_BATTLE="10:4:0",
               KHCOM_WINDOW="1280x720", KHCOM_QUIT="260", KHCOM_KEYS=keys,
               KHCOM_PRESENTSHOTS="160-160", KHCOM_HDDRAWLOG="160,240")
    subprocess.run([str(folder / "khcom.exe"), "--rom", str(rom)], cwd=folder, env=env,
                   timeout=60, check=True)
    im = Image.open(folder / "present_00160.bmp").convert("RGB")
    assert im.size == (1280,720), im.size
    # Native reference is blue here. The bug put a red rear-card frame across
    # this foreground illustration: (255,123,115) rather than blue.
    for xy in ((90,542),(100,542)):
        r,g,b = im.getpixel(xy)
        assert b > r + 50, (name,xy,(r,g,b),"stray frame across HD card")
    log = (folder / "kh_com.log").read_text()
    assert ("gCardWep01Tiles layer 1" in log) == bool(enabled), name
    im.save(folder / "preview.png")
    print(name, "passed", flush=True)
