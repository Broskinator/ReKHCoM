"""ROM-free production PPU exterior-padding regression."""
from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[2]
out=ROOT/'build/world-padding-regression';out.mkdir(exist_ok=True)
tc=sorted((ROOT.parent/'toolchain').glob('llvm-mingw-*'))[-1]
cmd=[str(tc/'bin/i686-w64-mingw32-clang.exe'),'-O2','-std=gnu11','-funsigned-char','-fno-strict-aliasing','-fwrapv','-mno-ms-bitfields','-DPLATFORM_PC','-DVERSION_EU','-I'+str(ROOT/'port/include')]
for p in [ROOT/'include']+[p for p in (ROOT/'include').iterdir() if p.is_dir() and p.name!='gba']:cmd+=['-iquote',str(p)]
exe=out/'padding.exe';cmd += [str(ROOT/'port/tools/world_padding_conformance.c'),'-o',str(exe)]
subprocess.run(cmd,check=True)
subprocess.run([str(exe)],cwd=out,check=True)
