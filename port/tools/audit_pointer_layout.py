"""Measure real game records at 32/64 bits before attempting a new backend.

Uses local LLVM MinGW; this is an ABI audit, not Linux execution proof.
No cartridge or player save is needed.
"""
from pathlib import Path
import json
import subprocess

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT/'build/pointer-layout-audit'
OUT.mkdir(parents=True, exist_ok=True)
source = OUT/'layout.c'
source.write_text(r'''
#include <stdio.h>
#include <stddef.h>
#include "card_types.h"
#include "save.h"
int main(void) {
    printf("%u %u %u %u %u %u\n", (unsigned)sizeof(void*),
        (unsigned)sizeof(CardDef), (unsigned)offsetof(CardDef,kind),
        (unsigned)sizeof(SaveBlockLarge), (unsigned)offsetof(SaveBlockLarge,data),
        (unsigned)sizeof(Deck));
    return 0;
}
''', encoding='utf-8')
rows = []
for arch in ['i686', 'x86_64']:
    compiler = next((ROOT.parent/'toolchain').glob('llvm-mingw-*/bin/'+arch+'-w64-mingw32-clang.exe'))
    executable = OUT/(arch+'.exe')
    subprocess.run([str(compiler), '-DPLATFORM_PC', '-DVERSION_EU',
                    '-iquote'+str(ROOT/'include'), '-iquote'+str(ROOT/'include/card'),
                    str(source), '-o', str(executable)], check=True)
    values = list(map(int, subprocess.check_output([str(executable)], text=True).split()))
    row = dict(zip(['pointer', 'CardDef_size', 'CardDef_kind_offset',
                    'SaveBlockLarge_size', 'SaveBlockLarge_data_offset', 'Deck_size'], values))
    row['arch'] = arch
    rows.append(row)
    print(row)
(OUT/'report.json').write_text(json.dumps(rows, indent=2), encoding='utf-8')
assert rows[0]['CardDef_size'] == 52
assert rows[1]['CardDef_size'] != rows[0]['CardDef_size'], 'Revisit the documented ABI constraint'
assert rows[0]['SaveBlockLarge_size'] == rows[1]['SaveBlockLarge_size'] == 0xf14
