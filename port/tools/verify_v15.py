"""Isolated v1.5 presentation fixtures; screenshots require visual review."""
import datetime, json, os, shutil, subprocess, sys
from pathlib import Path
from PIL import Image, ImageDraw
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/v15-review'/datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')
OUT.mkdir(parents=True,exist_ok=True)

def run(label,size,zoom=1,modes='',battle='',keys='',shots='480',end=500,record=None,version='eu',extra=None):
    d=OUT/label; d.mkdir(exist_ok=True)
    for name in ('khcom.exe','SDL3.dll'): shutil.copy2(ROOT/f'build/rel-{version}'/name,d/name)
    shutil.copytree(ROOT/'port/assets/fonts',d/'assets/fonts',dirs_exist_ok=True)
    (d/'kh_com.ini').write_text(f'config_version=4\nview_mode=4\nzoom={zoom}\nwindow_scale=4\nhd_text=1\n')
    env={k:v for k,v in os.environ.items() if not k.startswith(('KHCOM_','SDL_'))}
    env.update(SDL_VIDEO_DRIVER='dummy',SDL_AUDIO_DRIVER='dummy',KHCOM_FAST='1',KHCOM_WINDOW=size,
        KHCOM_MODE=modes,KHCOM_KEYS=keys,KHCOM_OUTPUT_SHOT=shots,KHCOM_QUIT=str(end),KHCOM_TRACE_MODES='1')
    if battle: env['KHCOM_BATTLE']=battle
    if record: env['KHCOM_RECORD']=record
    if extra: env.update(extra)
    rom={'eu':'B8CP','us':'B8CE','jp':'B8CJ'}[version]
    r=subprocess.run([str(d/'khcom.exe'),'--rom',str(ROOT/f'roms/{rom}.gba'),'--language','es'],cwd=d,env=env,timeout=240,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    log=(d/'kh_com.log').read_text(errors='replace')
    ok=r.returncode==0 and 'CRASH' not in log and all((d/f'out_{int(f):05d}.bmp').exists() for f in shots.split(','))
    suspicious=[line for line in log.splitlines() if any(word in line.lower() for word in ('division by zero','null read','unavailable','crash','failed'))]
    ok=ok and not suspicious
    print(d, 'CAPTURED' if ok else 'FAIL', flush=True)
    if suspicious: print('\n'.join(suspicious),flush=True)
    return dict(label=label,passed=ok,size=size,zoom=zoom,exit=r.returncode,path=str(d),suspicious=suspicious)

if __name__=='__main__':
    results=[]
    for size in ('1920x1080','5120x1440'):
        for z in (1,2):
            tag=f'{size}-z{z}'
            results.append(run(tag+'-battle',size,z,battle='0:0:0',keys='60-63:A',shots='180,380',end=390))
            results.append(run(tag+'-field',size,z,modes='60:room:10901,120:fieldfreeze',shots='400'))
            results.append(run(tag+'-door',size,z,modes='60:room:10901,120:fieldfreeze,200:doorpose:0',keys='240-242:A,420-422:A,500-502:B,560-562:B',shots='280,380,470,580,650',end=660))
            results.append(run(tag+'-status',size,z,modes='60:room:10901,120:fieldfreeze,320:mode_status',shots='500,620',end=630))
    (OUT/'report.json').write_text(json.dumps(results,indent=2))
    thumbs=[]
    for r in results:
        for f in sorted((OUT/r['label']).glob('out_*.bmp')):
            im=Image.open(f).convert('RGB'); im.thumbnail((640,360))
            tile=Image.new('RGB',(640,386),'#333'); tile.paste(im,((640-im.width)//2,26)); ImageDraw.Draw(tile).text((4,6),r['label']+' '+f.stem,fill='white'); thumbs.append(tile)
    for i in range(0,len(thumbs),8):
        sheet=Image.new('RGB',(1280,386*4),'#222')
        for j,t in enumerate(thumbs[i:i+8]):sheet.paste(t,((j%2)*640,(j//2)*386))
        sheet.save(OUT/f'sheet-{i//8}.png')
    sys.exit(0 if all(r['passed'] for r in results) else 1)
