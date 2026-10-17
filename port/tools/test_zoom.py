"""Check production zoom defaults, bounds and distinct selectable scales."""
from pathlib import Path
import subprocess, shutil
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/zoom-regression'; OUT.mkdir(exist_ok=True)
code = r"""
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "port/port.h"
extern float gPortAutoScaleF;
int PortUiActive(void) { return 0; }
int main(void) {
 int sizes[][2]={{320,180},{960,540},{1280,720},{1920,1080},{3440,1440},{5120,1440},{7680,2160}};
 int i,z,w,h,s,k,only,prev,cases=0;
 assert(gPortConfig.zoom==PORT_ZOOM_DEFAULT && gPortConfig.integerZoom==0);
 assert(!gPortConfig.cheatUnlockRiku && !PortRikuAvailable(0));
 assert(PortRikuAvailable(32));
 gPortConfig.cheatUnlockRiku=1;assert(PortRikuAvailable(0));
 PortConfigSetDataPath(".");PortConfigSave();
 gPortConfig.cheatUnlockRiku=0;PortConfigLoad();assert(gPortConfig.cheatUnlockRiku==1);
 gPortConfig.cheatUnlockRiku=0;PortConfigSave();PortConfigLoad();assert(!PortRikuAvailable(0));
 for(only=1;only<2;only++) {
  gPortConfig.integerZoom=only;
  for(i=0;i<7;i++) {
   int W=sizes[i][0],H=sizes[i][1],count=PortZoomCount(only,W,H);
   prev=999;
   for(k=0;k<count;k++) {
    z=PortZoomAt(k,only,W,H);
    assert(z<prev);prev=z;
    assert(PortZoomIndex(z,only,W,H)==k);
    assert(PortZoomStep(z,1,only,W,H)==PortZoomAt(k+1,only,W,H));
    assert(PortZoomStep(z,-1,only,W,H)==PortZoomAt(k-1,only,W,H));
    if(only) assert(!(z&1));
    gPortConfig.zoom=z;PortConfigAutoView(W,H,&w,&h,&s);
    assert(w<=PORT_MAX_W && h<=PORT_MAX_H && w>=240 && h>=160);
    assert(fabsf(gPortAutoScaleF-z*0.5f)<0.001f);
    assert(abs(h-(int)ceilf(H/(z*0.5f)))<=1);
    cases++;
   }
  }
 }
 gPortConfig.integerZoom=1;gPortConfig.zoom=PORT_ZOOM_CLASSIC;
 PortConfigAutoView(1920,1080,&w,&h,&s);
 assert(w==480 && h==270 && s==4);
 gPortConfig.zoom=PORT_ZOOM_DEFAULT;PortConfigAutoView(5120,1440,&w,&h,&s);
 assert(w==1280 && h==360 && s==4);
 assert(PortZoomClassicScale(5120,1440)==5);
 assert(PortZoomNormalize(9,0,1920,1080)==9);
 assert(PortZoomNormalize(9,1,1920,1080)==10);
 PortConfigSetDataPath(".");
 FILE* f=fopen("kh_com.ini","w"); assert(f);
 fprintf(f,"config_version=8\nzoom=6\ninteger_zoom=0\n"); fclose(f);
 PortConfigLoad(); assert(gPortConfig.zoom==PORT_ZOOM_DEFAULT && !gPortConfig.integerZoom);
 gPortConfig.zoom=10;gPortConfig.integerZoom=0;PortConfigSave();
 gPortConfig.zoom=1;PortConfigLoad();assert(gPortConfig.zoom==10 && !gPortConfig.integerZoom);
 assert(PortZoomReference(12)==1 && PortZoomReference(6)==2 && PortZoomReference(8)==0);
 f=fopen("kh_com.ini","w");assert(f);
 fprintf(f,"config_version=9\nzoom=9\ninteger_zoom=1\n");fclose(f);
 PortConfigLoad();assert(gPortConfig.zoom==10 && !gPortConfig.integerZoom);
 PortConfigAutoView(1920,1080,&w,&h,&s);
 gPortConfig.integerZoom=1;assert(PortConfigZoomNext(12,1)==10); /* no intermediate step */
 gPortConfig.integerZoom=0;assert(PortConfigZoomNext(12,1)==10);
 for(z=1;z<=PORT_ZOOM_LEVELS;z++)assert(!(PortConfigZoomId(z)&1));
 printf("PASS: %d resolution/zoom cases, exact v1.4 x4, numbered selector without intermediate steps, INI migration/persistence\n",cases);
 return 0;
}

"""
(OUT/'zoom.c').write_text(code)
cc=next((ROOT.parent/'toolchain').glob('llvm-mingw-*/bin/i686-w64-mingw32-clang.exe'))
sdl=next((ROOT.parent/'toolchain').glob('SDL3-*/i686-w64-mingw32'))
shutil.copy2(sdl/'bin/SDL3.dll',OUT/'SDL3.dll')
subprocess.run([str(cc),'-std=gnu11','-I'+str(ROOT/'port/include'),'-I'+str(sdl/'include'),str(OUT/'zoom.c'),str(ROOT/'port/src/port_config.c'),'-L'+str(sdl/'lib'),'-lSDL3','-o',str(OUT/'zoom.exe')],check=True)
subprocess.run([str(OUT/'zoom.exe')],cwd=OUT,check=True)
