#!/usr/bin/env python3
"""Compare regional and Japanese voice dumps for the optional Undub mode.

Uses only locally installed original ROMs. All generated INIs, logs and audio
dumps stay under build/codex-evidence/undub/.

    python port/tools/test_undub.py
"""
import argparse
import hashlib
import os
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EVIDENCE = ROOT / "build" / "codex-evidence" / "undub"
VOICE_IDS = (150, 152)  # SONG_VO_SR_ATTACK00 / SONG_VO_SR_ATTACK02
BUILD_DIRS = {"eu": EVIDENCE.parent / "final-eu", "us": EVIDENCE.parent / "final-us"}


def run(region: str, song_id: int, enabled: bool) -> tuple[bytes, str]:
    exe_dir = BUILD_DIRS[region]
    # The repository's EU build maps B8CP and US maps B8CE (see port.h).
    rom_name = {"eu": "B8CP.gba", "us": "B8CE.gba"}[region]
    run_dir = EVIDENCE / "runs" / f"{region}-{song_id}-{int(enabled)}"
    if run_dir.exists():
        shutil.rmtree(run_dir)
    (run_dir / "roms").mkdir(parents=True)
    for name in ("khcom.exe", "SDL3.dll"):
        shutil.copy2(exe_dir / name, run_dir / name)
    shutil.copy2(ROOT / "roms" / "B8CJ.gba", run_dir / "roms" / "B8CJ.gba")
    shutil.copy2(ROOT / "roms" / rom_name, run_dir / "regional.gba")
    (run_dir / "kh_com.ini").write_text(
        f"config_version=2\nundub={int(enabled)}\n",
        encoding="utf-8",
    )
    env = dict(os.environ, SDL_VIDEO_DRIVER="dummy", SDL_AUDIO_DRIVER="dummy",
               KHCOM_FAST="1", KHCOM_SONG=f"30:{song_id}",
               KHCOM_WAVDUMP="voice.f32", KHCOM_QUIT="180")
    result = subprocess.run(
        [str(run_dir / "khcom.exe"), "--data-dir", str(run_dir),
         "--rom", str(run_dir / "regional.gba")],
        cwd=run_dir, env=env, timeout=120, capture_output=True,
    )
    log_path = run_dir / "kh_com.log"
    log = log_path.read_text(errors="replace") if log_path.exists() else ""
    dump_path = run_dir / "voice.f32"
    if result.returncode != 0:
        raise RuntimeError(f"{region} song {song_id} undub={enabled}: exit {result.returncode}\n{log}")
    if not dump_path.exists() or dump_path.stat().st_size < 4096:
        raise RuntimeError(f"{region} song {song_id} undub={enabled}: missing/short audio dump\n{log}")
    if "m4a:" in log or "fault" in log.lower():
        raise RuntimeError(f"{region} song {song_id} undub={enabled}: sequencer fault\n{log}")
    should_apply = enabled
    if should_apply and ("undub: " not in log or "voice tracks loaded" not in log):
        raise RuntimeError(f"{region} song {song_id}: Undub did not report applying its voice map\n{log}")
    if not should_apply and "undub: " in log:
        raise RuntimeError(f"{region} song {song_id}: Undub applied while disabled\n{log}")
    return dump_path.read_bytes(), log


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-eu", default=str(BUILD_DIRS["eu"]))
    parser.add_argument("--build-us", default=str(BUILD_DIRS["us"]))
    args = parser.parse_args()
    BUILD_DIRS.update(eu=Path(args.build_eu), us=Path(args.build_us))
    for song_id in VOICE_IDS:
        regional_reference = None
        undub_reference = None
        for region in ("eu", "us"):
            regional, _ = run(region, song_id, False)
            if regional_reference is not None and regional != regional_reference:
                raise RuntimeError(f"{region} voice {song_id}: disabled mode changed regional audio")
            regional_reference = regional
            japanese, _ = run(region, song_id, True)
            if undub_reference is not None and japanese != undub_reference:
                raise RuntimeError(f"{region} voice {song_id}: Undub differs between EU and US")
            undub_reference = japanese
            if regional == japanese:
                raise RuntimeError(f"{region} voice {song_id}: regional and Undub dumps are identical")
            print(f"PASS {region.upper()} voice {song_id}: regional {len(regional)} bytes "
                  f"({hashlib.sha256(regional).hexdigest()[:12]}), Japanese {len(japanese)} bytes "
                  f"({hashlib.sha256(japanese).hexdigest()[:12]})")
    print("PASS: both tested Sora voice dumps change with Undub, match across EU/US, and remain regional when disabled.")


if __name__ == "__main__":
    main()
