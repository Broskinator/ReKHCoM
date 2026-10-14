#!/usr/bin/env python3
"""Options test: every value of the picture and sound options, and the
options screen in each language.

Part 1 starts a debug battle with each option value set in kh_com.ini and
checks that the run exits normally and the window image (KHCOM_OUTPUT_SHOT)
changes against the defaults when the option is visible in a battle.
Part 2 captures the options screen (KHCOM_OPTIONS_SHOT) in the five
languages of the European version, and in the US and Japanese versions.

    python port/tools/test_options.py [--build build/pc] [--build-us build/pc-us] [--build-jp build/pc-jp]
"""
import argparse
import os
import shutil
import subprocess
from pathlib import Path

from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[2]

# (key, values, expected to change the battle picture against the default)
OPTIONS = [
    ("view_mode", [0, 1, 2, 3], True),  # 2 (320x180) can equal the default Auto view
    ("zoom", [0, 2, 3], True),
    ("hud_size", [0, 2], True),
    ("filter", [2], True),  # 1 (pixel art) equals the default at integer scale
    ("scale_mode", [1, 2], False),
    ("scanlines", [50, 100], True),
    ("color_correction", [1], True),
    ("hd_text", [1], False),
    ("card_hud_raise", [0, 16], True),
    ("show_fps", [1], True),
    ("hd2d", [1], True),
    ("smooth_audio", [1], False),
    ("master_volume", [0], False),
    ("music_volume", [0], False),
    ("sfx_volume", [0], False),
    ("dodge_button", [0], False),
    ("stock_button", [0], False),
    ("unstock_button", [0], False),
]
LANGUAGES = ["English", "Italiano", "Francais", "Espanol", "Deutsch"]


def fresh(work, name, build):
    d = work / name
    shutil.rmtree(d, ignore_errors=True)
    d.mkdir(parents=True)
    for f in ("khcom.exe", "SDL3.dll"):
        shutil.copy2(build / f, d / f)
    return d


def run(d, ini, env, timeout=300):
    (d / "kh_com.ini").write_text("config_version=2\nwindow_scale=6\n" + ini)
    base = dict(os.environ, SDL_VIDEO_DRIVER="dummy", SDL_AUDIO_DRIVER="dummy", KHCOM_FAST="1")
    base.update(env)
    try:
        return subprocess.run([str(d / "khcom.exe")], cwd=str(d), env=base, timeout=timeout,
                              stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode
    except subprocess.TimeoutExpired:
        return "timeout"


def battle(work, build, name, ini):
    d = fresh(work, name, build)
    code = run(d, ini, {"KHCOM_BATTLE": "10:4:0", "KHCOM_KEYS": "60-63:A", "KHCOM_OUTPUT_SHOT": "600",
                        "KHCOM_QUIT": "601"})
    shot = d / "out_00600.bmp"
    return code, (Image.open(shot).convert("RGB") if shot.exists() else None)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--build", default=str(ROOT / "build" / "pc"))
    ap.add_argument("--build-us", default=str(ROOT / "build" / "pc-us"))
    ap.add_argument("--build-jp", default=str(ROOT / "build" / "pc-jp"))
    ap.add_argument("--only", help="comma list of option keys (skips the menus)")
    args = ap.parse_args()
    # two levels under the repository, where the exe finds ../../roms
    work = ROOT / "build"
    out = ROOT / "build" / "optiontest"
    out.mkdir(parents=True, exist_ok=True)
    lines, ok, total = [], 0, 0

    code, ref = battle(work, Path(args.build), "opt_default", "")
    lines.append(f"default            exit {code}")
    for key, values, visible in [o for o in OPTIONS if not args.only or o[0] in args.only.split(",")]:
        for v in values:
            total += 1
            code, img = battle(work, Path(args.build), f"opt_{key}_{v}", f"{key}={v}\n")
            changed = img is not None and ref is not None and (img.size != ref.size or
                                                              ImageChops.difference(img, ref).getbbox() is not None)
            good = code == 0 and img is not None and (changed or not visible)
            ok += good
            lines.append(f"{key}={v:<4} {'ok ' if good else 'FAIL'} exit {code}"
                         f"{'  picture changed' if changed else ''}")
            if img is not None:
                img.resize((img.width // 2, img.height // 2)).save(out / f"{key}_{v}.png")
            shutil.rmtree(work / f"opt_{key}_{v}", ignore_errors=True)

    # the options screen in each language and version
    shots = []
    if args.only:
        print("\n".join(lines))
        return
    for n, lang in enumerate(LANGUAGES):
        total += 1
        d = fresh(work, f"opt_lang_{n}", Path(args.build))
        keys = ",".join(f"{150 + 15 * k}-{151 + 15 * k}:DOWN" for k in range(n)) + ("," if n else "") + "200-201:A"
        code = run(d, "", {"KHCOM_KEYS": keys, "KHCOM_OPTIONS_SHOT": "500", "KHCOM_QUIT": "501"})
        good = code == 0 and (d / "shot_options.bmp").exists()
        ok += good
        lines.append(f"menu EU {lang:9s} {'ok ' if good else 'FAIL'} exit {code}")
        if good:
            shots.append(Image.open(d / "shot_options.bmp").convert("RGB"))
        shutil.rmtree(d, ignore_errors=True) if not good else None
    for region, build in (("US", args.build_us), ("JP", args.build_jp)):
        total += 1
        d = fresh(work, f"opt_menu_{region}", Path(build))
        code = run(d, "", {"KHCOM_OPTIONS_SHOT": "500", "KHCOM_QUIT": "501"})
        good = code == 0 and (d / "shot_options.bmp").exists()
        ok += good
        lines.append(f"menu {region}              {'ok ' if good else 'FAIL'} exit {code}")
        if good:
            shots.append(Image.open(d / "shot_options.bmp").convert("RGB"))
    if shots:
        w, h = shots[0].size
        sheet = Image.new("RGB", (w * 4, h * ((len(shots) + 3) // 4)), "black")
        for i, s in enumerate(shots):
            sheet.paste(s.resize((w, h)), ((i % 4) * w, (i // 4) * h))
        sheet.save(out / "menus.png")
    for d in work.glob("opt_*"):
        shutil.rmtree(d, ignore_errors=True)
    lines.append(f"{ok}/{total} options ok")
    (out / "report.txt").write_text("\n".join(lines) + "\n")
    print("\n".join(lines))


if __name__ == "__main__":
    main()
