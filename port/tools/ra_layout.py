"""Original GBA RAM addresses of the game's variables, from the decomp itself.

The cartridge links work RAM as configure.py's linker script says: IWRAM is
m4a2.o .bss, the IWRAM heap, then the .bss of main/mode/malloc/sprite, engine's
.iwram.*, pallet's .bss and engine's .iwram_common.*; EWRAM is the heap
(0x34000), every other object's .bss in link order (config/<v>/units.txt),
then every .ewram_common.* section in link order. This tool preprocesses each
unit as the GBA build does (no PLATFORM_PC), lists its RAM variables in
order, measures them with the port's own compiler (sizeof/alignof; the decomp
keeps GBA and port layouts equal) and lays them out with those rules.

Checked against addresses proven from the ROM's code (test_ra_state_addresses.py
witnesses). Output: build/ra-layout/<v>.json and, with --emit, the port table
port/src/port_ra_layout_<v>.inc used by port_ra_state.c.

    python port/tools/ra_layout.py --version eu --emit
"""
import argparse
import json
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ap = argparse.ArgumentParser()
ap.add_argument('--version', default='eu', choices=['eu', 'us', 'jp'])
ap.add_argument('--emit', action='store_true')
ap.add_argument('--locals', action='store_true', help='include block-scope statics in .bss')
ap.add_argument('--reverse-bss', action='store_true', help='static order in .bss reversed (gcc 2.95 check)')
args = ap.parse_args()
V = args.version
TC = next((ROOT.parent / 'toolchain').glob('llvm-mingw-*')) / 'bin'
CLANG = TC / 'i686-w64-mingw32-clang.exe'
OUT = ROOT / 'build/ra-layout'
OUT.mkdir(parents=True, exist_ok=True)
INCLUDES = ['-Iinclude'] + [f'-I{p.as_posix()}' for p in sorted((ROOT / 'include').iterdir())
                            if p.is_dir() and p.name != 'gba'] + [f'-Ibuild/{V}/gen']
INCLUDES = [i.replace(ROOT.as_posix() + '/', '') for i in INCLUDES]
IWRAM_BEFORE = ['m4a2']
IWRAM_AFTER = [('main', 'bss'), ('mode', 'bss'), ('malloc', 'bss'), ('sprite', 'bss'),
               ('engine', 'iwram'), ('pallet', 'bss'), ('engine', 'iwram_common')]
IWRAM_BSS_UNITS = {'m4a2', 'main', 'mode', 'malloc', 'sprite', 'pallet'}

sources = {p.name: p for p in (ROOT / 'src').rglob('*.c')}
units = []
for line in (ROOT / f'config/{V}/units.txt').read_text().splitlines():
    line = line.strip()
    if line and not line.startswith('#'):
        name = line.split()[0]
        if name in sources:
            units.append(sources[name])

MARK = re.compile(r'^# (\d+) "([^"]+)"')


def main_file_text(pre, src):
    keep, out = False, []
    want = src.as_posix().split('/src/', 1)[-1]
    for line in pre.splitlines():
        m = MARK.match(line)
        if m:
            keep = m.group(2).replace('\\\\', '/').replace('\\', '/').endswith(want)
            continue
        if keep:
            out.append(line)
    return '\n'.join(out)


def top_level_statements(text):
    depth, cur, out = 0, [], []
    for ch in text:
        if ch == '{':
            depth += 1
        elif ch == '}':
            depth -= 1
            if depth == 0:
                cur.append(ch)
                out.append(''.join(cur))
                cur = []
                continue
        if ch == ';' and depth == 0:
            out.append(''.join(cur))
            cur = []
            continue
        cur.append(ch)
    return [s.strip() for s in out if s.strip()]


SECTION = re.compile(r'section\s*\(\s*"\.(ewram_common|iwram|iwram_common)\.(?:\d+"|"\s*"\d+")\s*\)')
ALIGNED = re.compile(r'aligned\s*\(\s*(\d+)\s*\)')


def variables(stmt):
    """[(kind, name)] of a top-level statement that defines RAM variables."""
    s = ' '.join(stmt.split())
    if s.startswith(('typedef', 'extern', '__asm', 'asm')) or s.endswith('}') and '=' not in s:
        return []
    m = SECTION.search(s)
    if m:
        kind = m.group(1)
        head = s[:s.index('__attribute__')]
        name = re.findall(r'(\w+)\s*(?:\[[^\]]*\]\s*)*$', head.strip())
        return [(kind, name[0])] if name else []
    if '=' in s or '(' in s:
        return []          # initialised (.data) or a function / function pointer prototype
    if re.match(r'^(const\b)', s) or ' const ' in f' {s.split("[")[0]} ' and '*' not in s:
        return []
    names = []
    for part in s.split(','):
        n = re.findall(r'(\w+)\s*(?:\[[^\]]*\]\s*)*$', part.strip())
        if n and n[0] not in ('void', 'int', 'char', 'short', 'long', 'unsigned', 'signed', 'struct', 'union', 'enum'):
            names.append(('bss', n[0]))
    return names


units_vars = {}
for src in units:
    rel = src.relative_to(ROOT).as_posix()
    pre = subprocess.run([str(CLANG), '-E', '-std=gnu89', f'-DVERSION_{V.upper()}', '-w', *INCLUDES, rel],
                         cwd=ROOT, capture_output=True, text=True)
    if pre.returncode:
        raise SystemExit(f'{rel}: preprocess failed\n{pre.stderr[:2000]}')
    text = main_file_text(pre.stdout, src)
    found = []
    for stmt in top_level_statements(text):
        head = stmt.split('{', 1)[0]
        if stmt.rstrip().endswith('}') and '=' not in head and re.search(r'\)\s*$', head):
            # a function: its uninitialised block-scope statics live in .bss too
            if args.locals:
                for m in re.finditer(r'\bstatic\s+(?!const\b)([^;{}()=]*?)\b(\w+)(\s*(?:\[[^\]]*\]\s*)*);', stmt):
                    found.append(dict(kind='bss', name=m.group(2), align=None,
                                      decl=m.group(1) + '@@' + m.group(3), local=head.split('(')[0].split()[-1]))
            continue
        for kind, name in variables(stmt):
            al = ALIGNED.search(stmt)
            found.append(dict(kind=kind, name=name, align=int(al.group(1)) if al and kind != 'bss' else None))
    if not found:
        continue
    # sizes and natural alignments with the port's compiler, on the GBA view of the unit
    # measured with the port's own flags (PLATFORM_PC keeps the GBA struct layouts)
    probe = OUT / f'{src.stem}.probe.c'
    locals_ = ''.join(f'static {v["decl"].replace("@@", f" __rl{i}")};\n'
                      for i, v in enumerate(found) if v.get('decl'))
    ref = lambda i, v: f'__rl{i}' if v.get('decl') else v['name']
    body = f'#include "{src.as_posix()}"\n{locals_}unsigned int __rasz[] = {{' + ','.join(
        f'sizeof({ref(i, v)}),__alignof__({ref(i, v)})' for i, v in enumerate(found)) + '};\n'
    probe.write_text(body, encoding='utf-8')
    asm = subprocess.run([str(CLANG), '-S', '-O0', '-std=gnu89', '-w', '-funsigned-char', '-mno-ms-bitfields',
                          '-fno-common', f'-DVERSION_{V.upper()}', '-DPLATFORM_PC',
                          '-include', 'port/include/port/game_prefix.h', *INCLUDES, '-Iport/include',
                          '-o', '-', str(probe)], cwd=ROOT, capture_output=True, text=True)
    if asm.returncode:
        raise SystemExit(f'{rel}: size probe failed\n{asm.stderr[:2000]}')
    tail = asm.stdout[asm.stdout.index('__rasz:'):]
    vals = [int(x, 0) for x in re.findall(r'\.long\s+(\S+)', tail)[:2 * len(found)]]
    for i, v in enumerate(found):
        v['size'], v['natural'] = vals[2 * i], vals[2 * i + 1]
    units_vars[src.stem] = found


def place(addr, var, align):
    a = max(align, 1)
    addr = (addr + a - 1) & ~(a - 1)
    var['address'] = addr
    return addr + var['size']


def bss_of(stem):
    vs = [v for v in units_vars.get(stem, []) if v['kind'] == 'bss']
    return list(reversed(vs)) if args.reverse_bss else vs


layout = []
addr = 0x03000000
for stem in IWRAM_BEFORE:
    for v in bss_of(stem):
        addr = place(addr, v, v['natural']); layout.append((stem, v))
addr = max(addr, 0x03000000) + 0  # heap follows the m4a block
heap_start = (addr + 3) & ~3
addr = heap_start + 0x6800
for stem, kind in IWRAM_AFTER:
    vs = bss_of(stem) if kind == 'bss' else [v for v in units_vars.get(stem, []) if v['kind'] == kind]
    for v in vs:
        addr = place(addr, v, v['align'] or v['natural']); layout.append((stem, v))
addr = 0x02000000 + 0x34000
for src in units:
    if src.stem in IWRAM_BSS_UNITS:
        continue
    for v in bss_of(src.stem):
        addr = place(addr, v, v['natural']); layout.append((src.stem, v))
for src in units:
    for v in [v for v in units_vars.get(src.stem, []) if v['kind'] == 'ewram_common']:
        addr = place(addr, v, v['align']); layout.append((src.stem, v))

table = [dict(unit=u, name=v['name'], kind=v['kind'], address=hex(v['address']), size=v['size']) for u, v in layout]
(OUT / f'{V}.json').write_text(json.dumps(table, indent=1))
by_name = {}
for u, v in layout:
    by_name.setdefault(v['name'], []).append(v['address'])
print(f'{V}: {len(table)} variables, EWRAM ends at {hex(addr)}, IWRAM heap at {hex(heap_start)}')

if args.emit:
    lines = [f'/* Generated by port/tools/ra_layout.py --version {V}: original GBA address, port symbol, size. */']
    for u, v in layout:
        lines.append(f'RA_VAR({v["address"]:#010x}, {v["name"]}, {v["size"]}) /* {u} */')
    (ROOT / f'port/src/port_ra_layout_{V}.inc').write_text('\n'.join(lines) + '\n')
