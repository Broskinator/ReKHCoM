"""Validate the real downloaded Pooh condition against native RAM projection.
No downloaded condition or private RAM is versioned; the C runner never networks.
"""
from pathlib import Path
import argparse
import hashlib
import json
import subprocess
ROOT=Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--definitions',type=Path,default=ROOT/'build/ra-reference-audit/references.csv.definitions.json')
a=p.parse_args()
rows=json.loads(a.definitions.read_text(encoding='utf-8'))
definition=next(r['definition'] for r in rows if r['id']==160372)
out=ROOT/'build/ra71-pooh';out.mkdir(exist_ok=True)
fixture=out/'definition.txt';fixture.write_text(definition,encoding='utf-8')
compiler=next((ROOT.parent/'toolchain').glob('llvm-mingw-*/bin/i686-w64-mingw32-clang.exe'))
includes=[ROOT/'include',ROOT/'port/include']+[p for p in (ROOT/'include').iterdir() if p.is_dir() and p.name!='gba']
objects=sorted((ROOT/'build/ra-eu/ra').glob('port_third_party_rcheevos_src_rcheevos_*.o'))
objects += sorted((ROOT/'build/ra-eu/ra').glob('*rc_util.o'))+sorted((ROOT/'build/ra-eu/ra').glob('*md5.o'))
if not objects: p.error('Build EU rcheevos objects first')
exe=out/'check.exe'
subprocess.run([str(compiler),'-O1','-mno-ms-bitfields','-DPLATFORM_PC','-DVERSION_EU',
    *['-iquote'+str(p) for p in includes],str(ROOT/'port/tools/ra71_pooh_conformance.c'),
    str(ROOT/'port/src/port_ra_state.c'),str(ROOT/'port/src/port_ra_memory.c'),*map(str,objects),'-o',str(exe)],check=True)
subprocess.run([str(exe),str(fixture)],check=True)
report=dict(achievement=160372,definition_sha256=hashlib.sha256(definition.encode()).hexdigest(),
    frame_comparisons=300,positive_completion=1,empty_gauge_triggers=0,two_refills_triggers=0,
    scope='real condition graph and native memory mapping; controlled snapshots, not gameplay awards',
    awards_submitted=False)
(out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
