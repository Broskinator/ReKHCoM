"""ROM-free production renderer checks for the Riku opening extension."""
from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[2]
out=ROOT/'build/riku-backdrop-conformance';out.mkdir(exist_ok=True)
tc=sorted((ROOT.parent/'toolchain').glob('llvm-mingw-*'))[-1]
cmd=[str(tc/'bin/i686-w64-mingw32-clang.exe'),'-O2','-std=gnu11','-funsigned-char','-fno-strict-aliasing','-fwrapv','-mno-ms-bitfields','-DPLATFORM_PC','-DVERSION_EU','-I'+str(ROOT/'port/include')]
for p in [ROOT/'include']+[p for p in (ROOT/'include').iterdir() if p.is_dir() and p.name!='gba']:cmd+=['-iquote',str(p)]
for name in ['riku_backdrop_conformance','riku_tiled_backdrop_conformance']:
    exe=out/(name+'.exe')
    subprocess.run(cmd+[str(ROOT/'port/tools'/(name+'.c')),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],cwd=out,check=True)
