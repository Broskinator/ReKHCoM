"""Which cartridge functions load a RAM address (or a struct base just below it).

Scans the Thumb literal pools of every function in config/eu/funcmap.txt (EU ROM)
for 32-bit literals in [address - span, address]; prints function, literal and
the offset into it. Used to name the variables the official achievement set
reads, so the port can publish them at their original addresses.

    python port/tools/ra_witness.py 0x03007E8C 0x020000CC --span 0x200
"""
import argparse
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ap = argparse.ArgumentParser()
ap.add_argument('addresses', nargs='+')
ap.add_argument('--span', default='0x100')
args = ap.parse_args()
span = int(args.span, 0)
rom = (ROOT / 'roms/B8CP.gba').read_bytes()
funcs = []
for line in (ROOT / 'config/eu/funcmap.txt').read_text().splitlines():
    p = line.split()
    if len(p) >= 4:
        try:
            funcs.append((p[0], int(p[3], 16) - 0x08000000, int(p[2])))
        except ValueError:
            pass
for text in args.addresses:
    target = int(text, 0)
    hits = {}
    for name, start, size in funcs:
        if start < 0 or start + size > len(rom):
            continue
        for at in range(start, start + size, 2):
            op = struct.unpack_from('<H', rom, at)[0]
            if op & 0xF800 == 0x4800:
                lit = ((at + 4) & ~3) + (op & 255) * 4
                if lit + 4 <= len(rom):
                    v = struct.unpack_from('<I', rom, lit)[0]
                    if target - span <= v <= target:
                        hits.setdefault((v, target - v), []).append(name)
    print(hex(target))
    for (v, off), names in sorted(hits.items(), key=lambda x: x[0][1]):
        print('   base %s +0x%x: %s' % (hex(v), off, ', '.join(sorted(set(names))[:8])))
