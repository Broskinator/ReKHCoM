"""Audit the real catalog without evaluating/submitting official awards.

Requires an account already connected in the selected private launcher folder.
Exports public condition addresses only. No passwords, tokens, unlock states,
ROM bytes or memory values enter the report. Does not enable compatibility.
"""
import argparse
import collections
import csv
import json
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--data-dir', type=Path)
p.add_argument('--rom', type=Path)
p.add_argument('--out', type=Path, default=ROOT / 'build/ra-reference-audit')
p.add_argument('--offline', action='store_true', help='Summarize an existing references.csv without connecting')
args = p.parse_args()
out = args.out.resolve(); out.mkdir(parents=True, exist_ok=True)
if not args.offline:
    if not args.data_dir or not args.rom: p.error('--data-dir and --rom are required unless --offline')
    tc = next((ROOT.parent / 'toolchain').glob('llvm-mingw-*'))
    sdl = ROOT.parent / 'toolchain/SDL3-3.4.18/i686-w64-mingw32'
    objects = sorted((ROOT / 'build/ra-eu/ra').glob('*.o'))
    if not objects: p.error('Build the EU launcher/client first')
    subprocess.run([str(tc / 'bin/i686-w64-mingw32-clang.exe'), '-std=gnu11', '-O2',
        '-DRC_CLIENT_SUPPORTS_HASH', '-I' + str(sdl / 'include'),
        '-I' + str(ROOT / 'port/third_party/rcheevos/include'),
        str(ROOT / 'port/tools/audit_ra_references.c'), *map(str, objects),
        '-L' + str(sdl / 'lib'), '-lSDL3', '-lwinhttp', '-lcrypt32',
        '-Wl,-Bstatic', '-lwinpthread', '-Wl,-Bdynamic',
        '-Wl,--wrap=rc_api_process_fetch_game_sets_server_response', '-o', str(out / 'audit.exe')], check=True)
    shutil.copy2(sdl / 'bin/SDL3.dll', out / 'SDL3.dll')
    subprocess.run([str(out / 'audit.exe'), str(args.data_dir.resolve()), str(args.rom.resolve()),
                    str(out / 'references.csv')], cwd=out, check=True, timeout=45)
rows = list(csv.DictReader((out / 'references.csv').open()))
addresses = collections.Counter(int(row['address']) for row in rows if row['reference_type'] == '0')
# Conservative four-byte intersection test; indirect results are never certified
# by this root inventory. Keep these ranges aligned with port_ra_state.c (EU).
ranges = [(0x421C0, 0x42200), (0x422B8, 0x423D0), (0x423F0, 0x42690),
          (0x42690, 0x42E5E), (0x42E68, 0x42E6A), (0x42ED0, 0x42FDE)]
covered = lambda a: a in (0x3CACC, 0x3C000, 0x3C001, 0x6C68, 0x749C, 0x3D088, 0x449D4, 0x44B86) or any(start <= a and a + 4 <= end for start, end in ranges)
pointer_roots = (0x3CA28, 0x423E4)
scalar_covered = covered
covered = lambda a: a in pointer_roots or scalar_covered(a)
remaining = [hex(a) for a in sorted(addresses) if not covered(a)]
report = dict(achievement_graphs=len({r['achievement'] for r in rows}), references=len(rows),
    unique_direct_addresses=len(addresses), scalar_roots_projected=sum(map(scalar_covered, addresses)),
    heap_pointer_roots_projected=sum(a in pointer_roots for a in addresses),
    direct_roots_projected=sum(map(covered, addresses)),
    remaining_direct_addresses=remaining,
    modified_reference_operands=sum(r['reference_type'] == '1' for r in rows),
    coverage_scope='direct roots only; indirect target layouts and trigger semantics remain unvalidated',
    official_evaluation='NOT PERFORMED by this audit', awards_submitted=False)
(out / 'summary.json').write_text(json.dumps(report, indent=2))
print(json.dumps(report, indent=2))
