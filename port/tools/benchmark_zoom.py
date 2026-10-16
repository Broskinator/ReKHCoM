"""Repeatable, isolated PC zoom benchmark. Requires psutil, Pillow and private EU ROM.

Dummy SDL measures software rendering, not a real GPU/monitor. CPU/profile means
exclude pacing; comparison images come from the actual presented SDL output.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import time
import psutil
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--build', type=Path, default=ROOT/'build/rel-eu')
p.add_argument('--label', required=True)
p.add_argument('--zooms', nargs='+', type=int, default=[1,2,4])
p.add_argument('--scenes', nargs='+', choices=['field','battle'], default=['field','battle'])
p.add_argument('--frames', type=int, default=720)
p.add_argument('--window', default='1920x1080')
args = p.parse_args()
if not re.fullmatch(r'[A-Za-z0-9_-]+', args.label):p.error('Use a simple label')
if args.frames < 480:p.error('At least 480 frames are required for warmup and a profile interval')
out=ROOT/'build/zoom-benchmark'/args.label;out.mkdir(parents=True,exist_ok=True)
rows=[]
for scene in args.scenes:
 for zoom in args.zooms:
  d=out/f'{scene}-x{zoom}';d.mkdir(exist_ok=True)
  for n in ['khcom.exe','SDL3.dll']:shutil.copy2(args.build/n,d/n)
  shutil.copytree(ROOT/'port/assets/fonts',d/'assets/fonts',dirs_exist_ok=True)
  (d/'kh_com.ini').write_text(f'config_version=11\nview_mode=4\nzoom={zoom*2}\nnarrow_fit=1\nhd_text=0\nvsync=0\n')
  e={k:v for k,v in os.environ.items() if not k.startswith(('KHCOM_','SDL_'))}
  for hint in ['SDL_XINPUT_ENABLED','SDL_JOYSTICK_HIDAPI','SDL_JOYSTICK_RAWINPUT','SDL_JOYSTICK_DIRECTINPUT','SDL_JOYSTICK_WGI','SDL_JOYSTICK_GAMEINPUT','SDL_JOYSTICK_GAMEINPUT_RAW']:e[hint]='0'
  shot=args.frames-1
  e.update(SDL_VIDEO_DRIVER='dummy',SDL_AUDIO_DRIVER='dummy',KHCOM_FAST='1',KHCOM_PROFILE='1',KHCOM_WINDOW=args.window,KHCOM_QUIT=str(args.frames),KHCOM_OUTPUT_SHOT=str(shot))
  if scene=='field':e.update(KHCOM_MODE='60:room:10901,120:fieldfreeze',KHCOM_KEYS='')
  else:e.update(KHCOM_BATTLE='10:4:0',KHCOM_KEYS='60-63:A')
  start=time.perf_counter();peak_rss=peak_private=0
  with subprocess.Popen([str(d/'khcom.exe'),'--rom',str(ROOT/'roms/B8CP.gba'),'--language','en'],cwd=d,env=e,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,creationflags=0x08000000) as child:
   proc=psutil.Process(child.pid)
   while child.poll() is None:
    try:
     mem=proc.memory_info();peak_rss=max(peak_rss,mem.rss);peak_private=max(peak_private,getattr(mem,'private',mem.vms))
    except psutil.NoSuchProcess:break
    if time.perf_counter()-start>300:child.kill();raise TimeoutError(d)
    time.sleep(.05)
   assert child.wait()==0,d
  elapsed=time.perf_counter()-start
  profile=re.findall(r'profile: canvas (\d+)x(\d+) ppu ([\d.]+) ms present ([\d.]+) ms',(d/'kh_com.log').read_text(errors='replace'))
  assert len(profile)>=2,profile
  bmp=d/f'out_{shot:05d}.bmp';im=Image.open(bmp).convert('RGB');im.save(d/'presented.png');bmp.unlink()
  steady=profile[1:]
  row=dict(scene=scene,zoom=zoom,window=args.window,canvas=[int(profile[-1][0]),int(profile[-1][1])],
           ppu_ms=sum(float(v[2]) for v in steady)/len(steady),present_ms=sum(float(v[3]) for v in steady)/len(steady),
           wall_seconds=elapsed,peak_working_set_mib=peak_rss/1024**2,peak_private_commit_mib=peak_private/1024**2,
           presented_sha256=hashlib.sha256(im.tobytes()).hexdigest(),exe_sha256=hashlib.sha256((d/'khcom.exe').read_bytes()).hexdigest())
  rows.append(row);print(json.dumps(row),flush=True)
(out/'report.json').write_text(json.dumps(rows,indent=2))
