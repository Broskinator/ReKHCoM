"""Exercise the real official frame path without credentials or networking."""
from pathlib import Path
import os
import shutil
import subprocess
ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'build/official-frame-guard'; OUT.mkdir(parents=True,exist_ok=True)
CC = next((ROOT.parent/'toolchain').glob('llvm-mingw-*/bin/i686-w64-mingw32-clang.exe'))
SDL = ROOT.parent/'toolchain/SDL3-3.4.18/i686-w64-mingw32'
includes = [ROOT/'include', ROOT/'port/include'] + [p for p in (ROOT/'include').iterdir() if p.is_dir() and p.name!='gba']
for region in ('eu','us','jp'):
    exe = OUT / (region+'.exe')
    subprocess.run([str(CC),'-O1','-mno-ms-bitfields','-DPLATFORM_PC','-DPORT_RETROACHIEVEMENTS','-DVERSION_'+region.upper(),
        *['-iquote'+str(p) for p in includes],'-I'+str(SDL/'include'),'-I'+str(ROOT/'port/third_party/rcheevos/include'),
        str(ROOT/'port/src/port_achievements.c'),str(ROOT/'port/tools/official_frame_guard.c'),'-L'+str(SDL/'lib'),'-lSDL3',
        '-Wl,-Bstatic','-lwinpthread','-Wl,-Bdynamic','-o',str(exe)],check=True)
    shutil.copy2(SDL/'bin/SDL3.dll',OUT/'SDL3.dll')
    for which in range(4):
        work = OUT / (region+'-'+str(which)); work.mkdir(exist_ok=True)
        env = {k:v for k,v in os.environ.items() if not k.startswith(('KHCOM_','SDL_'))}
        subprocess.run([str(exe),str(which)],cwd=work,env=env,check=True)
print('PASS 12 region/modifier cases; synthetic RA spy, no network or real award')
