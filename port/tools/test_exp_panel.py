"""ROM-free production-renderer check for transparent holes in HD EXP digits."""
from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[2]
out=ROOT/'build/exp-panel-regression';out.mkdir(exist_ok=True)
tc=sorted((ROOT.parent/'toolchain').glob('llvm-mingw-*'))[-1]
cmd=[str(tc/'bin/i686-w64-mingw32-clang.exe'),'-O2','-std=gnu11','-funsigned-char','-fno-strict-aliasing','-fwrapv','-mno-ms-bitfields','-DPLATFORM_PC','-DVERSION_EU','-I'+str(ROOT/'port/include')]
for p in [ROOT/'include']+[p for p in (ROOT/'include').iterdir() if p.is_dir() and p.name!='gba']:cmd+=['-iquote',str(p)]
exe=out/'exp_panel.exe';cmd += [str(ROOT/'port/tools/exp_panel_conformance.c'),'-o',str(exe)]
subprocess.run(cmd,check=True)
subprocess.run([str(exe)],cwd=out,check=True)
