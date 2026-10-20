#!/usr/bin/env python3
"""Write port/data/rom_pointers_<region>.inc: every word of the cartridge ROM that holds a pointer.

The relocatable GBA space (Switch, Vita; include/gba/gba_space.h) cannot load the ROM at 0x08000000, so the
pointers stored in the ROM's data are moved when it is loaded (port/src/port_gba_space.c). Which words are
pointers comes from the linker: build_gba_maps_vm.py relinks the decomp's ROM with --emit-relocs and dumps its
relocations (build/gba-maps/gba-maps/com_<region>.relocs.txt.gz). Only R_ARM_ABS32 entries inside the ROM hold a
full address. Each one is checked against the player's cartridge (roms/<code>.gba): the word must be the symbol's
address plus an addend, in the symbol's own memory region, as the linker wrote it; any other word stops the script, so a list that does not
describe that exact ROM is never written.

    python port/tools/gen_rom_pointers.py            # eu us jp
    python port/tools/gen_rom_pointers.py us
"""
import gzip
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MAPS = ROOT / 'build/gba-maps/gba-maps'
ROMS = {'eu': 'B8CP', 'us': 'B8CE', 'jp': 'B8CJ'}
ROM_BASE, ROM_END = 0x08000000, 0x0A000000
def region_of(address):
    return 0x08 if 0x08000000 <= address < 0x0E000000 else address >> 24


LINE = re.compile(r'^([0-9a-f]{8})\s+[0-9a-f]+\s+R_ARM_ABS32\s+([0-9a-f]{8})\b')


def generate(region):
    rom = (ROOT / 'roms' / f'{ROMS[region]}.gba').read_bytes()
    offsets, bad = set(), []
    with gzip.open(MAPS / f'com_{region}.relocs.txt.gz', 'rt', encoding='latin1') as dump:
        for line in dump:
            m = LINE.match(line.strip())
            if not m:
                continue
            at, symbol = int(m.group(1), 16), int(m.group(2), 16)
            if not ROM_BASE <= at < ROM_END or at & 3:
                continue
            if symbol < 0x02000000:
                continue  # absolute symbol (a constant such as gNumMusicPlayers), not an address
            off = at - ROM_BASE
            if off + 4 > len(rom):
                continue
            word = struct.unpack_from('<I', rom, off)[0]
            # the word is the symbol (bit 0 set for Thumb code, or a section start) plus the addend kept in
            # place: same memory region as the symbol and close to it (voicegroups point a little before a symbol)
            if not 0x02000000 <= word < 0x0F000000 or region_of(word) != region_of(symbol) or word < symbol - 0x10000:
                bad.append((off, word, symbol))
                continue
            offsets.add(off)
    if bad:
        for off, word, symbol in bad[:10]:
            print(f'  {region}: ROM+0x{off:07x} holds 0x{word:08x}, symbol 0x{symbol:08x}')
        sys.exit(f'{region}: {len(bad)} relocations do not match roms/{ROMS[region]}.gba; not written')
    out = ROOT / 'port/data' / f'rom_pointers_{region}.inc'
    lines = ['/* filled by port/tools/gen_rom_pointers.py: ROM offsets of the pointer words (%d) */' % len(offsets)]
    ordered = sorted(offsets)
    for i in range(0, len(ordered), 8):
        lines.append(', '.join(f'0x{o:07x}' for o in ordered[i:i + 8]) + ',')
    out.write_text('\n'.join(lines) + '\n', encoding='ascii', newline='\n')
    print(f'{out.relative_to(ROOT)}: {len(offsets)} pointer words')


if __name__ == '__main__':
    for region in sys.argv[1:] or ['eu', 'us', 'jp']:
        generate(region)
