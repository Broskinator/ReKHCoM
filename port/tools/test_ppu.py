#!/usr/bin/env python3
"""PPU regression: fixed scenes that cover the renderer's features, rendered
in the original 240x160 view and compared with stored references.

  lang      text BGs, the language select           (mode 0, 4bpp BGs)
  title     the title screen                         (BG blending, OBJ)
  debug     the debug menu                           (256-colour BG, OBJ)
  battle    a debug battle                           (affine BG 2, OBJ, windows)
  mosaic    the card-break mosaic in a battle        (BG and OBJ mosaic, affine mosaic)
  room      a field room                             (map-block BGs, OBJ priority)
  hall      a castle hall                            (fixed room)
  movie     the opening movie                        (bitmap mode)
  allmap    the world map menu                       (screen layer, OBJ)
  deck      the deck menu                            (menu BGs, OBJ)
  status    the status menu                          (menu BGs, OBJ)
  jiminy    Jiminy's journal                         (menu BGs, OBJ)

The game frame (KHCOM_SHOTS, before any window scaling) of each scene is
hashed; references live in port/progress/evidence/ppu_refs.json. A scene that
differs from its reference fails until the change is reviewed and the
references are written again with --update. A contact sheet goes to
build/pputest/scenes.png for the review.

    python port/tools/test_ppu.py [--build build/pc] [--update]
"""
import argparse
import hashlib
import json
import os
import shutil
import subprocess
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
REFS = ROOT / "port" / "progress" / "evidence" / "ppu_refs.json"

SCENES = {
    "lang": ({}, 120),
    "title": ({"KHCOM_KEYS": "200-201:A"}, 1000),
    "debug": ({"KHCOM_MODE": "300:mode_debug"}, 400),
    "battle": ({"KHCOM_BATTLE": "10:4:0", "KHCOM_KEYS": "60-63:A"}, 500),
    "mosaic": ({"KHCOM_BATTLE": "10:4:0", "KHCOM_KEYS": "60-63:A", "KHCOM_MODE": "500:mosaic"}, 505),
    "room": ({"KHCOM_MODE": "300:room:10000"}, 700),
    "hall": ({"KHCOM_MODE": "300:room:20000"}, 700),
    "movie": ({"KHCOM_MOVIE": "1"}, 400),
    "allmap": ({"KHCOM_MODE": "300:room:10000,600:mode_allmap"}, 800),
    "status": ({"KHCOM_MODE": "300:room:10000,600:mode_status"}, 800),
    "jiminy": ({"KHCOM_MODE": "300:room:10000,600:mode_jiminy"}, 800),
    "deck": ({"KHCOM_MODE": "300:room:10000,600:Mode_Deck"}, 800),
}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--build", default=str(ROOT / "build" / "pc"))
    ap.add_argument("--update", action="store_true", help="write the current images as the references")
    args = ap.parse_args()
    refs = json.loads(REFS.read_text()) if REFS.exists() else {}
    out = ROOT / "build" / "pputest"
    out.mkdir(parents=True, exist_ok=True)
    lines, images, failed = [], [], 0
    for name, (env_extra, frame) in SCENES.items():
        # two levels under the repository, where the exe finds ../../roms
        d = ROOT / "build" / f"ppu_{name}"
        shutil.rmtree(d, ignore_errors=True)
        d.mkdir(parents=True)
        for f in ("khcom.exe", "SDL3.dll"):
            shutil.copy2(Path(args.build) / f, d / f)
        (d / "kh_com.ini").write_text("config_version=2\nview_mode=0\n")
        env = dict(os.environ, SDL_VIDEO_DRIVER="dummy", SDL_AUDIO_DRIVER="dummy", KHCOM_FAST="1",
                   KHCOM_SHOTS=str(frame), KHCOM_QUIT=str(frame + 1))
        env.update(env_extra)
        code = subprocess.run([str(d / "khcom.exe")], cwd=str(d), env=env, timeout=300,
                              stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode
        shot = d / f"shot_{frame:05d}.bmp"
        if code != 0 or not shot.exists():
            lines.append(f"{name:8s} FAIL exit {code}, no picture")
            failed += 1
            continue
        img = Image.open(shot).convert("RGB")
        digest = hashlib.sha256(img.tobytes()).hexdigest()[:16]
        images.append((name, img))
        if args.update:
            refs[name] = digest
            lines.append(f"{name:8s} reference {digest}")
        elif refs.get(name) == digest:
            lines.append(f"{name:8s} ok   {digest}")
        else:
            lines.append(f"{name:8s} CHANGED {digest} (reference {refs.get(name, 'none')})")
            failed += 1
        img.save(out / f"{name}.png")
        shutil.rmtree(d, ignore_errors=True)
    if args.update:
        REFS.parent.mkdir(parents=True, exist_ok=True)
        REFS.write_text(json.dumps(refs, indent=2) + "\n")
    if images:
        w, h = images[0][1].size
        sheet = Image.new("RGB", (w * 5, h * ((len(images) + 4) // 5)), "black")
        for i, (_, im) in enumerate(images):
            sheet.paste(im.resize((w, h)), ((i % 5) * w, (i // 5) * h))
        sheet.save(out / "scenes.png")
    lines.append(f"{len(SCENES) - failed}/{len(SCENES)} scenes {'recorded' if args.update else 'match'}")
    (out / "report.txt").write_text("\n".join(lines) + "\n")
    print("\n".join(lines))
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
