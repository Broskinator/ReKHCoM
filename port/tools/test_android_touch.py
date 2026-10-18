"""Run Android touch logic against SDL on Windows; not a device/GPU test."""
from pathlib import Path
import subprocess,shutil
R=Path(__file__).resolve().parents[2]
cc=next((R.parent/'toolchain').glob('llvm-mingw-*/bin/i686-w64-mingw32-clang.exe'))
sdl=R.parent/'toolchain/SDL3-3.4.18/i686-w64-mingw32'
out=R/'build/android-touch-test';out.mkdir(exist_ok=True)
exe=out/'touch.exe'
subprocess.run([str(cc),'-O2','-std=gnu11','-I'+str(R/'port/include'),'-I'+str(sdl/'include'),
               str(R/'port/tools/android_touch_conformance.c'),'-L'+str(sdl/'lib'),'-lSDL3','-o',str(exe)],check=True)
shutil.copy2(sdl/'bin/SDL3.dll',out/'SDL3.dll')
subprocess.run([str(exe)],cwd=out,check=True)
