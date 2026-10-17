"""Test production aspect geometry on desktop and phone-shaped outputs."""
from pathlib import Path
import subprocess
ROOT = Path(__file__).resolve().parents[2]
cc = next((ROOT.parent/'toolchain').glob('llvm-mingw-*/bin/i686-w64-mingw32-clang.exe'))
out = ROOT/'build/aspect-test'
out.mkdir(exist_ok=True)
exe = out/'aspect.exe'
subprocess.run([str(cc), '-O2', '-I'+str(ROOT/'port/include'),
                str(ROOT/'port/tools/aspect_conformance.c'), '-o', str(exe)], check=True)
subprocess.run([str(exe)], check=True)
