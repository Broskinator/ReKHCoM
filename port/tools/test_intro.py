#!/usr/bin/env python3
"""Exercise the full opening and return to gameplay in an isolated SDL session."""
import argparse
import os
import shutil
import struct
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--language", choices=("en", "es"), default="en")
    parser.add_argument("--view", type=int, choices=range(5), default=0)
    parser.add_argument("--speed", type=int, choices=(0, 1), default=0)
    args = parser.parse_args()
    folder = ROOT / "build" / f"intro-regression-{args.language}-view{args.view}-speed{args.speed}"
    folder.mkdir(parents=True, exist_ok=True)
    for name in ("khcom.exe", "SDL3.dll"):
        shutil.copyfile(ROOT / "build/pc" / name, folder / name)
    # This test owns only its isolated fixture; never remove the player's save.
    for name in ("kh_com.sav", "kh_com.log"):
        (folder / name).unlink(missing_ok=True)
    (folder / "kh_com.ini").write_text(f"config_version=2\nview_mode={args.view}\ngame_speed={args.speed}\nwindow_scale=1\nvsync=0\n")
    keys = "1100-1110:A,1300-1310:A,1700-1710:START,2000-2010:A,2300-2310:A,2600-2610:A,3000-3010:A,4200-4210:A,4500-4510:A,4800-4810:A"
    if args.language == "es":
        keys = "100-110:DOWN,130-140:DOWN,160-170:DOWN,190-200:A," + keys
    env = dict(os.environ, SDL_VIDEO_DRIVER="dummy", SDL_AUDIO_DRIVER="dummy", KHCOM_FAST="1",
               KHCOM_KEYS=keys, KHCOM_SHOTS="5000,8500,10500,11500,12500", KHCOM_QUIT="13000")
    for name in ("KHCOM_LOADSTATE", "KHCOM_SAVESTATE", "KHCOM_WAVDUMP", "KHCOM_WATCHDOG"):
        env.pop(name, None)
    result = subprocess.run([str(folder / "khcom.exe")], cwd=folder, env=env, timeout=60)
    assert result.returncode == 0, f"Game exited with {result.returncode}"
    log = (folder / "kh_com.log").read_text()
    assert "Movie: stopped at frame 1617/1617" in log, "Opening did not complete"
    assert "CRASH" not in log and "FATAL" not in log, log
    for frame in (5000, 8500, 10500, 11500, 12500):
        shot = (folder / f"shot_{frame:05}.bmp").read_bytes()
        assert shot[:2] == b"BM"
        pixels = shot[struct.unpack_from("<I", shot, 10)[0]:]
        assert any(pixels), f"Frame {frame} is blank"
    print(f"PASS: full opening completed, game continued to frame 13000 ({args.language}, view {args.view}, speed {args.speed})")
    print(f"Screenshots: {folder}")


if __name__ == "__main__": main()
