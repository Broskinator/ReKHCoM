"""The launcher's save editor end to end, on copies only:
1. the game writes Sora's file 1 at a save point (harness mapsave);
2. save_editor.h changes level, HP, CP, munny, a map card and the cards;
3. the other blocks stay byte for byte, both copies and the header verify;
4. the game loads the edited file from the title screen and reports it.

usage: python port/tools/test_save_editor.py   (from the repo root, after
building build/pc-eu; needs roms/B8CP.gba and the llvm-mingw toolchain)
"""
import os
import shutil
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BUILD = ROOT / 'build/pc-eu'
WORK = ROOT / 'build/save-editor-test'
TOOLCHAIN = next((ROOT.parent / 'toolchain').glob('llvm-mingw-*/bin'))


def checksum_ok(block):
    s = sum(struct.unpack('<%dH' % (len(block) // 2), block))
    s = (s & 0xFFFF) + (s >> 16)
    return ((~(s + (s >> 16))) & 0xFFFF) == 0


def game(env_extra, quit_at):
    env = {k: v for k, v in os.environ.items() if not k.startswith('KHCOM_')}
    env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_QUIT=str(quit_at), **env_extra)
    log = WORK / 'kh_com.log'
    if log.exists():
        log.unlink()
    subprocess.run([str(WORK / 'khcom.exe')], cwd=WORK, env=env, timeout=300,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return log.read_text(errors='replace')


def main():
    shutil.rmtree(WORK, ignore_errors=True)
    WORK.mkdir(parents=True)
    for name in ('khcom.exe', 'SDL3.dll'):
        shutil.copy2(BUILD / name, WORK / name)
    # 1. a real save: Sora's file 1 written at the save point
    game({'KHCOM_MODE': '300:room:20000,900:mapsave', 'KHCOM_KEYS': '1000-1003:LEFT,1010-1013:A'}, 1100)
    sav = WORK / 'kh_com.sav'
    before = sav.read_bytes()
    assert before[0x2F20:0x2F38] == b'KHCOM_BACKUP_VER00000013', 'the game did not save file 1'

    # 2. the editor
    probe = WORK / 'save_editor_probe.exe'
    env = dict(os.environ, PATH=str(TOOLCHAIN) + os.pathsep + os.environ.get('PATH', ''))
    subprocess.run([str(TOOLCHAIN / 'i686-w64-mingw32-clang++.exe'), '-O1', '-std=c++17', '-static', '-o', str(probe), str(ROOT / 'port/tools/save_editor_probe.cpp')],
                   check=True, env=env)
    edited = WORK / 'edited.sav'
    out = subprocess.run([str(probe), str(sav), str(edited), '0', 'level', '50', 'hp', '400', 'cp', '1500',
                          'moogle', '12345', 'map', '0', '5', 'add', '9', '3', 'add', '459', '1'],
                         check=True, capture_output=True, text=True, env=env).stdout
    print(out.strip())
    after = edited.read_bytes()

    # 3. only Sora's file 1 (both copies) and the header changed, all valid
    for at, size in [(0x10, 0x40), (0x50, 0x40), (0x2F20, 0xF14), (0x2F20 + 0xF14, 0xF14)]:
        assert checksum_ok(after[at:at + size]), f'bad checksum at {at:#x}'
    assert after[0x2F20:0x2F20 + 0xF14] == after[0x2F20 + 0xF14:0x2F20 + 0x1E28], 'copies differ'
    changed = [i for i in range(0x10000) if before[i] != after[i]]
    outside = [i for i in changed if not (0x10 <= i < 0x90 or 0x2F20 <= i < 0x2F20 + 0x1E28)]
    assert not outside, f'bytes outside file 1 changed: {outside[:8]}'
    summary = after[0x10 + 0x1C + 4:0x10 + 0x1C + 12]
    assert summary[2] == 50, f'header level {summary[2]}'

    # 4. the game loads it (title -> continue -> file 1) and reports it
    shutil.copy2(edited, sav)
    log = game({'KHCOM_MODE': '10:mode_title,1800:cheat:0',
                'KHCOM_KEYS': '900-903:A,1100-1103:A,1550-1553:A'}, 1810)
    line = next((l for l in log.splitlines() if 'harness: cheat 0' in l), '')
    print(line)
    assert 'level=50 hp=400 cp=1500' in line, 'the game did not load the edited values'
    assert 'maps=6' in line, 'map cards'
    assert 'cards=19/999' in line, 'battle cards'
    print('PASS: edited file 1 loads in the game with its new values; nothing else changed')


if __name__ == '__main__':
    sys.exit(main())
