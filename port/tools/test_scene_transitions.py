"""Pixel checks of the presented SDL window, not the PPU canvas.
Requires a private EU ROM and copies the supplied normal save into isolated tests.
Physical controller backends are disabled only in each test process.
"""
import os,shutil,subprocess
from pathlib import Path
from PIL import Image,ImageDraw
R=Path(__file__).resolve().parents[2]
SAVE=None
def run(name,frames,keys='200-203:A',modes='',build='rel-eu',lang=None,save=False,extra=None,win='1536x864',narrow=1):
 d=R/'build/transition-review'/name;d.mkdir(parents=True,exist_ok=True)
 for f in ['khcom.exe','SDL3.dll']:shutil.copy2((R/'build'/build/f) if (R/'build'/build/f).exists() else R/'build/rel-eu'/f,d/f)
 shutil.copytree(R/'port/assets/fonts',d/'assets/fonts',dirs_exist_ok=True)
 (d/'kh_com.ini').write_text(f'config_version=11\nview_mode=4\nzoom=8\nnarrow_fit={narrow}\nhd_text=1\n')
 if save:shutil.copy2(SAVE,d/'kh_com.sav')
 e={k:v for k,v in os.environ.items() if not k.startswith(('KHCOM_','SDL_'))}
 for k in ['SDL_XINPUT_ENABLED','SDL_JOYSTICK_HIDAPI','SDL_JOYSTICK_RAWINPUT','SDL_JOYSTICK_DIRECTINPUT','SDL_JOYSTICK_WGI','SDL_JOYSTICK_GAMEINPUT','SDL_JOYSTICK_GAMEINPUT_RAW']:e[k]='0'
 e.update(SDL_VIDEO_DRIVER='dummy',SDL_AUDIO_DRIVER='dummy',KHCOM_FAST='1',KHCOM_KEYS=keys,KHCOM_MODE=modes,KHCOM_OUTPUT_SHOT=','.join(map(str,frames)),KHCOM_QUIT=str(max(frames)+1),KHCOM_WINDOW=win,KHCOM_DBGSIDE='1',KHCOM_TRACE_MODES='1',KHCOM_PPULOG='1')
 if extra:e.update(extra)
 subprocess.run([str(d/'khcom.exe'),'--rom',str(R/'roms/B8CP.gba')]+(['--language',lang] if lang else []),cwd=d,env=e,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=180,check=True)
 sheet=Image.new('RGB',(768,240*((len(frames)+1)//2)))
 for i,f in enumerate(frames):
  im=Image.open(d/f'out_{f:05d}.bmp').convert('RGB');im.thumbnail((384,216));sheet.paste(im,(i%2*384,i//2*240));ImageDraw.Draw(sheet).text((i%2*384,i//2*240+216),str(f),fill='white')
 sheet.save(d/'sheet.png');return d

if __name__=='__main__':
 import argparse,json
 from concurrent.futures import ThreadPoolExecutor
 from PIL import ImageChops
 p=argparse.ArgumentParser(description='Verify actual presented transition frames; save is copied into isolated tests.')
 p.add_argument('--save',type=Path,required=True,help='Existing normal .sav with a loadable slot (never modified)')
 a=p.parse_args();SAVE=a.save.resolve();assert SAVE.is_file()
 def check(win):
  W,H=map(int,win.split('x'));checks={}
  d=run('lang-'+win,list(range(232,256)),win=win)
  # No logo or emblem may enter the margin as the paper darkens. Its
  # bottom band must have the same luminance range as the clean top band.
  for f in range(240,250):
   im=Image.open(d/f'out_{f:05d}.bmp').convert('RGB')
   for x in [0,W-16]:
    top=im.crop((x,int(H*.1),x+16,int(H*.15)))
    bottom=im.crop((x,int(H*.85),x+16,int(H*.9)))
    assert top.getextrema()==bottom.getextrema(),('repeated emblem',win,f,x)
  for f in [250,251,252,253]:
   im=Image.open(d/f'out_{f:05d}.bmp').convert('RGB')
   assert len(im.getcolors(W*H))==1,('flat frame has mismatched margins',win,f)
  checks['language_frames']=24
  keys=','.join([f'{f}-{f+3}:A' for f in range(200,801,60)])+',1120-1123:START,1400-1403:DOWN,1420-1423:A,1560-1563:A'
  d=run('load-'+win,list(range(1560,1655)),keys=keys,save=True,win=win)
  assert 'mode Mode_MenuLoad' in (d/'kh_com.log').read_text(errors='replace')
  for f in range(1560,1655):
   im=Image.open(d/f'out_{f:05d}.bmp').convert('RGB');edge=int(W*.05)
   assert ImageChops.difference(im.crop((0,0,edge,H)),im.crop((W-edge,0,W,H))).getbbox() is None,('unequal title fade',win,f)
  checks['title_fade_frames']=95
  d=run('door-'+win,list(range(194,210)),keys='',modes='60:room:20001,180:roomtransition',win=win)
  log=(d/'kh_com.log').read_text(errors='replace');assert 'mode Mode_MapFld' in log
  for f in [198,199]:
   im=Image.open(d/f'out_{f:05d}.bmp').convert('RGB')
   assert im.getextrema()==((255,255),(255,255),(255,255)),('room silhouette',win,f)
  checks['room_transition_frames']=16
  # Contact sheets retain exact ordering; retain selected full-size PNGs too.
  for name,keep in [('lang',[246,247,251]),('load',[1605,1630]),('door',[198,199,200])]:
   folder=R/'build/transition-review'/(name+'-'+win)
   for file in folder.glob('out_*.bmp'):
    f=int(file.stem[4:])
    if f in keep:Image.open(file).save(file.with_suffix('.png'))
    file.unlink()
  return win,checks
 with ThreadPoolExecutor(max_workers=2) as pool:report=dict(pool.map(check,['1536x864','1920x1080']))
 (R/'build/transition-review/report.json').write_text(json.dumps(report,indent=2));print(json.dumps(report))
