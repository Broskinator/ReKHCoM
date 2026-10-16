"""Compare real status entry/exit with NEW badges on/off, including 32:9.
An optional --reference build must contain the same statusnew harness action.
"""
import argparse, hashlib, json, os, shutil, subprocess
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser();p.add_argument('--build',type=Path,default=ROOT/'build/rel-eu');p.add_argument('--reference',type=Path);p.add_argument('--zooms',type=int,nargs='+',default=[8]);args=p.parse_args()
out=ROOT/'build/status-transition/review-isolated';out.mkdir(parents=True,exist_ok=True)
frames=[200,210,220,260,472,478,490,498]
versions=[('after',args.build)]+([('before',args.reference)] if args.reference else [])
cases=[(tag,source,W,H,z,new) for tag,source in versions for W,H in [(1920,1080),(5120,1440)] for z in args.zooms for new in [0,1]]
def run(c):
 tag,source,W,H,z,new=c;label=f'{tag}-{W}x{H}-z{z}-new{new}';d=out/label;d.mkdir(exist_ok=True)
 for f in ['khcom.exe','SDL3.dll']:shutil.copy2(source/f,d/f)
 shutil.copytree(ROOT/'port/assets/fonts',d/'assets/fonts',dirs_exist_ok=True)
 (d/'kh_com.ini').write_text(f'config_version=11\nview_mode=4\nzoom={z}\nhd_text=1\n')
 env={k:v for k,v in os.environ.items() if not k.startswith(('KHCOM_','SDL_'))}
 # Disable physical controller backends only in this subprocess. The user
 # may keep playing in another window while the screenshot oracle runs.
 for hint in ['SDL_XINPUT_ENABLED','SDL_JOYSTICK_HIDAPI','SDL_JOYSTICK_RAWINPUT','SDL_JOYSTICK_DIRECTINPUT','SDL_JOYSTICK_WGI','SDL_JOYSTICK_GAMEINPUT','SDL_JOYSTICK_GAMEINPUT_RAW']:
  env[hint]='0'
 env.update(SDL_VIDEO_DRIVER='dummy' ,SDL_AUDIO_DRIVER='dummy',KHCOM_FAST='1',KHCOM_WINDOW=f'{W}x{H}',
  KHCOM_MODE=f'60:room:10901,120:fieldfreeze,140:statusnew:{new},180:mode_status',KHCOM_KEYS='470-472:B',
  KHCOM_OUTPUT_SHOT=','.join(map(str,frames)),KHCOM_QUIT='520',KHCOM_TRACE_MODES='1')
 proc=subprocess.run([str((d/'khcom.exe').resolve()),'--rom',str(ROOT/'roms/B8CP.gba'),'--language','es'],cwd=d,env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=180)
 log=(d/'kh_com.log').read_text(errors='replace');assert proc.returncode==0 and 'mode mode_status' in log and 'no mode' not in log and 'CRASH' not in log,label
 rows=[];edge=int((W-240*(H/160))/2)
 for f in frames:
  file=d/f'out_{f:05d}.bmp';im=Image.open(file).convert('RGB');file.unlink()
  # Exclude the title/tab bands: NEW glyphs live in the panel body.
  panel=(0,round(32*H/160),edge,round(132*H/160))
  body=im.crop(panel);body.save(d/f'margin_{f}.png')
  core=im.crop((edge,0,W-edge,H));center=hashlib.sha256(core.tobytes()).hexdigest()
  if f in [220,478]:im.resize((1280,round(H*1280/W))).save(d/f'frame_{f}.png')
  rows.append(dict(frame=f,margin=hashlib.sha256(body.tobytes()).hexdigest(),center=center))
 print(label,'captured',flush=True);return label,rows
with ThreadPoolExecutor(max_workers=2) as pool: captures=dict(pool.map(run,cases))
checks=[]
for W,H in [(1920,1080),(5120,1440)]:
 for z in args.zooms:
  a=captures[f'after-{W}x{H}-z{z}-new0'];b=captures[f'after-{W}x{H}-z{z}-new1']
  assert all(x['margin']==y['margin'] for x,y in zip(a,b)),('NEW leaked into margin',W,z)
  if args.reference:
   ba=captures[f'before-{W}x{H}-z{z}-new0'];bb=captures[f'before-{W}x{H}-z{z}-new1']
   changed=[x['frame'] for x,y in zip(ba,bb) if x['margin']!=y['margin']]
   assert changed,('fixture did not reproduce bug',W,z)
   for new,current,old in [(0,a,ba),(1,b,bb)]:
    assert all(x['center']==y['center'] for x,y in zip(current,old)),('authored picture changed',W,z,new)
   checks.append(dict(resolution=[W,H],zoom=z/2,before_bad_frames=changed,after_new_badge_leak=False,original_picture_identical=True))
  else:checks.append(dict(resolution=[W,H],zoom=z/2,after_new_badge_leak=False))
(out/'report.json').write_text(json.dumps(checks,indent=2));print(json.dumps(checks),flush=True)
