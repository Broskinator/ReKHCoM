"""Compare actual card-menu transition windows with the v1.8.2 reference.
Private EU ROM required; physical controller input is isolated per subprocess.
"""
import argparse,hashlib,json,os,shutil,subprocess
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from PIL import Image,ImageChops
ROOT=Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser();p.add_argument('--reference',type=Path,required=True);args=p.parse_args()
OUT=ROOT/'build/mapcard-transition/review';OUT.mkdir(parents=True,exist_ok=True)
frames=list(range(213,226))+[185,200,210,260]+list(range(470,506,3))
cases=[(W,H,z,mode,tag,build) for W,H,z in [(1536,864,8),(1920,1080,6),(5120,1440,12)] for mode in ['mode_mapinspect','mode_deck'] for tag,build in [('before',args.reference),('after',ROOT/'build/rel-eu')]]
def run(c):
 W,H,z,mode,tag,build=c;d=OUT/f'{mode}-{W}x{H}-z{z}-{tag}';d.mkdir(exist_ok=True)
 for name in ['khcom.exe','SDL3.dll']:shutil.copy2(build/name,d/name)
 shutil.copytree(ROOT/'port/assets/fonts',d/'assets/fonts',dirs_exist_ok=True)
 (d/'kh_com.ini').write_text(f'config_version=11\nview_mode=4\nzoom={z}\nhd_text=1\nnarrow_fit=1\n')
 e={k:v for k,v in os.environ.items() if not k.startswith(('KHCOM_','SDL_'))}
 for hint in ['SDL_XINPUT_ENABLED','SDL_JOYSTICK_HIDAPI','SDL_JOYSTICK_RAWINPUT','SDL_JOYSTICK_DIRECTINPUT','SDL_JOYSTICK_WGI','SDL_JOYSTICK_GAMEINPUT','SDL_JOYSTICK_GAMEINPUT_RAW']:e[hint]='0'
 actual_mode='Mode_Deck' if mode=='mode_deck' else mode
 e.update(SDL_VIDEO_DRIVER='dummy',SDL_AUDIO_DRIVER='dummy',KHCOM_FAST='1',KHCOM_WINDOW=f'{W}x{H}',KHCOM_MODE=f'60:room:10901,120:fieldfreeze,180:{actual_mode}',KHCOM_KEYS='470-472:B',KHCOM_QUIT='510',KHCOM_OUTPUT_SHOT=','.join(map(str,frames)),KHCOM_TRACE_MODES='1')
 subprocess.run([str(d/'khcom.exe'),'--rom',str(ROOT/'roms/B8CP.gba'),'--language','es'],cwd=d,env=e,check=True,timeout=180,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
 log=(d/'kh_com.log').read_text(errors='replace')
 assert 'harness: no mode' not in log,log
 assert f'mode {actual_mode}' in log
 rows={}
 for f in frames:
  file=d/f'out_{f:05d}.bmp';im=Image.open(file).convert('RGB');file.unlink()
  # At 16:9/32:9 MenuScale fits the original picture to full height.
  edge=round((W-240*H/160)/2)
  core=im.crop((edge,0,W-edge,H));margin=im.crop((0,round(H*.1),edge,round(H*.3)))
  pixels=margin.get_flattened_data() if hasattr(margin,'get_flattened_data') else margin.getdata()
  rows[f]={'core':hashlib.sha256(core.tobytes()).hexdigest(),'margin':hashlib.sha256(margin.tobytes()).hexdigest(),'white':sum(min(px)>=220 and max(px)-min(px)<=10 for px in pixels)}
  if f in [219,220,260,479]:im.resize((1280,round(H*1280/W))).save(d/f'frame_{f}.png')
 print(d.name,'captured',flush=True);return (W,H,z,mode,tag),rows
with ThreadPoolExecutor(max_workers=2) as pool:data=dict(pool.map(run,cases))
report=[]
for W,H,z in [(1536,864,8),(1920,1080,6),(5120,1440,12)]:
 for mode in ['mode_mapinspect','mode_deck']:
  a=data[W,H,z,mode,'before'];b=data[W,H,z,mode,'after']
  assert all(a[f]['core']==b[f]['core'] for f in frames),('authored menu changed',W,mode)
  if mode=='mode_mapinspect':
   fixed=[f for f in frames if a[f]['white']>b[f]['white']]
   assert fixed,('glove fixture did not reproduce',W,mode)
   assert a[260]==b[260],('settled picture changed',W,mode)
  else:fixed=[f for f in frames if a[f]['margin']!=b[f]['margin']]
  report.append({'window':[W,H],'zoom':z/2,'mode':mode,'original_picture_identical':True,'corrected_margin_frames':fixed})
(OUT/'report.json').write_text(json.dumps(report,indent=2));print(json.dumps(report),flush=True)
