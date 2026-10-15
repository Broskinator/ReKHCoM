"""Check production zoom defaults, bounds and distinct selectable scales."""
from pathlib import Path
import subprocess, shutil
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/zoom-regression'; OUT.mkdir(exist_ok=True)
code = r"""
#include <assert.h>
#include <stdio.h>
#include "port/port.h"
int main(void) {
 int sizes[][2]={{320,180},{960,540},{1280,720},{1920,1080},{3440,1440},{5120,1440}};
 int i,z,w,h,s,prev;
 for(i=0;i<6;i++) {
  gPortConfig.zoom=1; PortConfigAutoView(sizes[i][0],sizes[i][1],&w,&h,&s);
  if(i==3) assert(s==4);
  if(i==5) assert(s==5);
  prev=999;
  for(z=0;z<PORT_ZOOM_COUNT;z++) {
   gPortConfig.zoom=z; PortConfigAutoView(sizes[i][0],sizes[i][1],&w,&h,&s);
   assert(w<=PORT_MAX_W && h<=PORT_MAX_H && w>=240 && h>=160);
   if(PortConfigZoomValid(z)) { assert(s<prev); prev=s; }
   assert(PortConfigZoomValid(PortConfigZoomEffective(z)));
  }
  printf("%dx%d: distinct scales and bounds OK\n",sizes[i][0],sizes[i][1]);
 }
 return 0;
}
"""
(OUT/'zoom.c').write_text(code)
cc=next((ROOT.parent/'toolchain').glob('llvm-mingw-*/bin/i686-w64-mingw32-clang.exe'))
sdl=next((ROOT.parent/'toolchain').glob('SDL3-*/i686-w64-mingw32'))
shutil.copy2(sdl/'bin/SDL3.dll',OUT/'SDL3.dll')
subprocess.run([str(cc),'-std=gnu11','-I'+str(ROOT/'port/include'),'-I'+str(sdl/'include'),str(OUT/'zoom.c'),str(ROOT/'port/src/port_config.c'),'-L'+str(sdl/'lib'),'-lSDL3','-o',str(OUT/'zoom.exe')],check=True)
subprocess.run([str(OUT/'zoom.exe')],cwd=OUT,check=True)
