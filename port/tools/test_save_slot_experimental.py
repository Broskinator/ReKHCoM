#!/usr/bin/env python3
"""Verify save-point slot selection is opt-in through Options > Experimental.

Runs the menu harness in an isolated folder and confirms the setting starts
Off, can be enabled, and is persisted. It does not write game saves.

    python port/tools/test_save_slot_experimental.py [--build build/codex-evidence/gated-eu]
"""
import argparse
import os
import re
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def item_index() -> int:
    source = (ROOT / "port/src/port_ui.c").read_text(encoding="utf-8")
    start = source.index("enum {\n    ITEM_VIEW")
    enum = source[start:source.index("};", start)]
    names = re.findall(r"\b(ITEM_[A-Z0-9_]+)\b", enum)
    return names.index("ITEM_SAVE_SLOT_EXPERIMENTAL")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build", default=str(ROOT / "build/codex-evidence/gated-eu"))
    args = parser.parse_args()
    run = ROOT / "build/codex-evidence/save-slot-experimental/run"
    if run.exists():
        shutil.rmtree(run)
    run.mkdir(parents=True)
    for name in ("khcom.exe", "SDL3.dll"):
        shutil.copy2(Path(args.build) / name, run / name)
    rom = ROOT / "roms/B8CP.gba"
    (run / "kh_com.ini").write_text("config_version=2\n", encoding="utf-8")
    slot_item = item_index()
    env = dict(os.environ, SDL_VIDEO_DRIVER="dummy", SDL_AUDIO_DRIVER="dummy", KHCOM_FAST="1",
               KHCOM_OPTIONS_TOGGLE="1", KHCOM_OPTIONS_ACTIONS=f"1:{slot_item}:1",
               KHCOM_OPTIONS_HOLD="3", KHCOM_QUIT="200")
    result = subprocess.run([str(run / "khcom.exe"), "--data-dir", str(run), "--rom", str(rom)],
                            cwd=run, env=env, timeout=120, capture_output=True)
    log_path = run / "kh_com.log"
    log = log_path.read_text(errors="replace") if log_path.exists() else ""
    expected = f"enabled=1 before=[Off] after=[On]"
    ini = (run / "kh_com.ini").read_text(errors="replace")
    if result.returncode != 0 or expected not in log or "experimental_save_point_slot_select=1" not in ini:
        raise SystemExit(f"FAIL save-slot option (exit {result.returncode})\n{log}\n{ini}")
    print(f"PASS: item {slot_item} starts Off, toggles On under Experimental, and persists in kh_com.ini")
    print(f"Evidence: {run}")


if __name__ == "__main__":
    main()
