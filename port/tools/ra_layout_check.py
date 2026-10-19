"""Check build/ra-layout/<v>.json against the cartridge's code.

For every function of config/<v>/funcmap.txt, the RAM literals of its Thumb
literal pools are compared with the variables its C body (src/) names: a
variable is confirmed when some literal of a function using it falls inside
[address, address + size). Prints, per unit, confirmed / contradicted counts
and the most common shift for contradicted ones, to calibrate ra_layout.py.
"""
import collections
import json
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
V = sys.argv[1] if len(sys.argv) > 1 else 'eu'
code = {'eu': 'B8CP', 'us': 'B8CE', 'jp': 'B8CJ'}[V]
rom = (ROOT / f'roms/{code}.gba').read_bytes()
table = json.loads((ROOT / f'build/ra-layout/{V}.json').read_text())
by_unit = collections.defaultdict(dict)
for v in table:
    by_unit[v['unit']][v['name']] = v
globals_ = {v['name']: v for v in table if not v['name'].startswith('s')}

funcmap = ROOT / ('config/jp/funcmap.txt' if V == 'jp' else 'config/eu/funcmap.txt')
col = 1 if V == 'us' else 3
funcs = {}
for line in funcmap.read_text().splitlines():
    p = line.split()
    if len(p) >= 4:
        try:
            funcs[p[0]] = (int(p[col], 16) - 0x08000000, int(p[2]))
        except ValueError:
            pass

bodies = {}
FUNC = re.compile(r'^[A-Za-z_][\w \*]*?\b(\w+)\s*\([^;{]*\)\s*\{', re.M)
for src in (ROOT / 'src').rglob('*.c'):
    text = src.read_text(encoding='utf-8', errors='replace')
    for m in FUNC.finditer(text):
        depth, i = 0, m.end() - 1
        while i < len(text):
            if text[i] == '{':
                depth += 1
            elif text[i] == '}':
                depth -= 1
                if depth == 0:
                    break
            i += 1
        bodies[m.group(1)] = (src.stem, text[m.end():i])


def literals(name):
    start, size = funcs[name]
    out = set()
    for at in range(start, start + size, 2):
        op = struct.unpack_from('<H', rom, at)[0]
        if op & 0xF800 == 0x4800:
            lit = ((at + 4) & ~3) + (op & 255) * 4
            if lit + 4 <= len(rom):
                val = struct.unpack_from('<I', rom, lit)[0]
                if 0x02034000 <= val < 0x02040000 or 0x03000000 <= val < 0x03008000:
                    out.add(val)
    return out


ok = collections.Counter()
bad = collections.Counter()
shifts = collections.defaultdict(collections.Counter)
evidence = collections.defaultdict(list)
for name, (unit, body) in bodies.items():
    if name not in funcs:
        continue
    lits = literals(name)
    if not lits:
        continue
    words = set(re.findall(r'\b[gs][A-Z]\w*', body))
    for w in words:
        v = by_unit.get(unit, {}).get(w) or globals_.get(w)
        if not v:
            continue
        a, s = int(v['address'], 16), max(v['size'], 1)
        if any(a <= l < a + s for l in lits):
            ok[(v['unit'], w)] += 1
        else:
            near = [l - a for l in lits if -0x2000 < l - a < 0x2000]
            if near:
                bad[(v['unit'], w)] += 1
                for d in near:
                    shifts[(v['unit'], w)][d] += 1
confirmed = {k for k in ok}
contra = {k for k in bad if k not in ok}
print(f'{V}: confirmed {len(confirmed)} variables, contradicted {len(contra)}')
units = collections.defaultdict(lambda: [0, 0])
for k in confirmed:
    units[k[0]][0] += 1
for k in contra:
    units[k[0]][1] += 1
order = [v['unit'] for v in table]
seen = []
for u in order:
    if u not in seen:
        seen.append(u)
for u in seen:
    if u in units:
        c, x = units[u]
        tops = [f'{k[1]}:{shifts[k].most_common(1)[0][0]:+#x}' for k in contra if k[0] == u][:4]
        print(f'  {u:28s} ok {c:3d}  bad {x:3d}  {" ".join(tops)}')
