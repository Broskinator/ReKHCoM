#!/usr/bin/env python3
"""Contrast doubtful VBM synchronization with original USA in isolated BizHawk.

Copies emulator runtime/config; original emulator settings and saves remain
untouched. This is a behavior reference, not a port rendering approval test.
"""
import argparse
import datetime
import json
import os
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--frames', type=int, default=20000)
    ap.add_argument('--every', type=int, default=1000)
    ap.add_argument('--timeout', type=int, default=600)
    args = ap.parse_args()
    work = ROOT / 'build/codex-gameplay-reference' / datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')
    work.mkdir(parents=True)
    source = ROOT.parent / 'toolchain/bizhawk'
    emulator = work / 'emulator'
    emulator.mkdir()
    for name in ('EmuHawk.exe', 'EmuHawk.exe.config', 'defctrl.json'):
        shutil.copy2(source / name, emulator / name)
    for name in ('dll', 'gamedb', 'Firmware', 'Shaders', 'overlay'):
        if (source / name).exists():
            shutil.copytree(source / name, emulator / name)
    config = json.loads((source / 'config.ini').read_text(encoding='utf-8-sig'))
    for entry in config['PathEntries']['Paths']:
        kind, system = entry['Type'], entry['System']
        if kind == 'Firmware':
            path = emulator / 'Firmware'
        else:
            path = work / 'data' / system / kind
        path.mkdir(parents=True, exist_ok=True)
        entry['Path'] = str(path)
    config['AutosaveSaveRAM'] = False
    (emulator / 'config.ini').write_text(json.dumps(config, indent=2))
    lua = ROOT / 'port/tools/bizhawk_vbm.lua'
    env = dict(os.environ, KHCOM_BH_VBM=str(ROOT.parent / 'Kingdom Hearts - Chain of Memories (U).vbm'),
               KHCOM_BH_OUT=str(work), KHCOM_BH_EVERY=str(args.every), KHCOM_BH_FRAMES=str(args.frames))
    startup = subprocess.STARTUPINFO()
    startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    startup.wShowWindow = 0
    proc = subprocess.Popen([str(emulator / 'EmuHawk.exe'), '--lua='+str(lua), str(ROOT / 'roms/B8CE.gba')],
                            cwd=emulator, env=env, startupinfo=startup,
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        code = proc.wait(timeout=args.timeout)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.wait()
        code = 'timeout'
    report = dict(exit=code, frames=args.frames, captured=len(list(work.glob('bh_*.png'))),
                  expected=args.frames//args.every, scope='original behavior comparison; requires visual inspection')
    (work / 'report.json').write_text(json.dumps(report, indent=2))
    print(work)
    print(json.dumps(report, indent=2))
    return 0 if code == 0 and report['captured'] == report['expected'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
