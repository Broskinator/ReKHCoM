#!/usr/bin/env python3
"""Play every song and sound effect of the ROM and check the output.

Each named SongId (include/songs.h) is started at frame 30 of a fresh run
(KHCOM_SONG) and recorded for 9 seconds (KHCOM_WAVDUMP). A song fails when
the sequencer logs an error (a track leaving the ROM, ...), the process does
not exit normally, or the output is silent; clipped samples and PSG use are
reported. Run against the US build: it boots without the language select,
so nothing else plays in those frames.

    python port/tools/test_songs.py [--build build/pc-us] [--jobs 8] [--only 1,6,101]
"""
import argparse
import array
import math
import os
import re
import shutil
import subprocess
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
START, FRAMES = 30, 540


def song_ids():
    """(id, name) for every entry of enum SongId, following explicit values."""
    text = (ROOT / "include" / "songs.h").read_text()
    body = text[text.index("enum SongId {"):text.index("};")]
    out, value = [], 0
    for name, explicit in re.findall(r"(SONG_\w+)(?:\s*=\s*(\w+))?\s*,", body):
        if explicit:
            value = int(explicit, 0)
        if name != "SONG_NONE":
            out.append((value, name))
        value += 1
    return out


def run(exe_dir, work, song_id, name, timeout):
    # two levels under the repository, where the exe finds ../../roms
    d = work / f"songtest_{song_id:03d}"
    shutil.rmtree(d, ignore_errors=True)
    d.mkdir(parents=True)
    for f in ("khcom.exe", "SDL3.dll"):
        shutil.copy2(exe_dir / f, d / f)
    (d / "kh_com.ini").write_text("config_version=2\n")
    env = dict(os.environ, SDL_VIDEO_DRIVER="dummy", SDL_AUDIO_DRIVER="dummy", KHCOM_FAST="1",
               KHCOM_SONG=f"{START}:{song_id}", KHCOM_WAVDUMP="song.f32", KHCOM_PSGLOG="1",
               KHCOM_QUIT=str(START + FRAMES))
    try:
        code = subprocess.run([str(d / "khcom.exe")], cwd=str(d), env=env, timeout=timeout,
                              stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode
    except subprocess.TimeoutExpired:
        code = "timeout"
    log = (d / "kh_com.log").read_text(errors="replace") if (d / "kh_com.log").exists() else ""
    errors = [l for l in log.splitlines() if l.startswith("m4a:") or "fault" in l.lower()]
    psg = "psg:" in log
    samples = array.array("f")
    if (d / "song.f32").exists():
        samples.frombytes((d / "song.f32").read_bytes())
    # skip the frames before the song starts (about START frames of output)
    rates = [int(l.split()[0]) for l in (d / "song.f32.rates").read_text().split("\n") if l] \
        if (d / "song.f32.rates").exists() else []
    skip = sum(rates[:START]) * 2
    body = samples[skip:]
    n = len(body)
    rms = math.sqrt(sum(v * v for v in body) / n) if n else 0.0
    peak = max((abs(v) for v in body), default=0.0)
    clipped = sum(1 for v in body if abs(v) >= 0.999)
    status = "ok"
    if code != 0:
        status = f"exit {code}"
    elif errors:
        status = "seq error"
    elif peak < 1e-4:
        status = "silent"
    shutil.rmtree(d, ignore_errors=True)
    return song_id, name, status, rms, peak, clipped, psg, errors[:2]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--build", default=str(ROOT / "build" / "pc-us"))
    ap.add_argument("--jobs", type=int, default=8)
    ap.add_argument("--timeout", type=int, default=120)
    ap.add_argument("--only", help="comma list of song ids")
    args = ap.parse_args()
    exe_dir, work = Path(args.build), ROOT / "build"
    songs = song_ids()
    if args.only:
        wanted = {int(x) for x in args.only.split(",")}
        songs = [s for s in songs if s[0] in wanted]
    with ThreadPoolExecutor(args.jobs) as pool:
        results = sorted(pool.map(lambda s: run(exe_dir, work, s[0], s[1], args.timeout), songs))
    lines = []
    for sid, name, status, rms, peak, clipped, psg, errors in results:
        lines.append(f"{sid:4d} {name:32s} {status:10s} rms {rms:.4f} peak {peak:.3f} clipped {clipped:6d}"
                     f"{'  psg' if psg else ''}{'  ' + ' | '.join(errors) if errors else ''}")
    report = work / "songtest_report.txt"
    ok = sum(1 for r in results if r[2] == "ok")
    silent = [r for r in results if r[2] == "silent"]
    summary = (f"{ok}/{len(results)} ok, {len(silent)} silent, "
               f"{sum(1 for r in results if r[2] not in ('ok', 'silent'))} failed, "
               f"{sum(1 for r in results if r[6])} use PSG, {sum(1 for r in results if r[5])} with clipped samples")
    report.write_text("\n".join(lines + [summary]) + "\n")
    print("\n".join(l for l in lines if " ok " not in l))
    print(summary)
    print(f"report: {report}")


if __name__ == "__main__":
    main()
