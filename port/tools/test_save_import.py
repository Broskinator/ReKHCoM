"""Exercise production PC save parsing, progress preservation and atomic writes."""
from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[2]
compiler=next((ROOT.parent/'toolchain').glob('llvm-mingw-*/bin/i686-w64-mingw32-clang++.exe'))
output=ROOT/'build/save-import-conformance'
output.mkdir(parents=True,exist_ok=True)
executable=output/'test.exe'
subprocess.run([str(compiler),'-std=c++17','-O2','-static','-DNOMINMAX',
                '-I'+str(ROOT/'port/decompui'),str(ROOT/'port/tools/save_import_conformance.cpp'),
                '-o',str(executable)],check=True)
subprocess.run([str(executable)],cwd=output,check=True)
