"""Compare native title pixels and side-band continuity at multiple output sizes."""
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import os, subprocess, json
from PIL import Image, ImageChops
ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'build/title60-regression'
SIZES = [(1920,1080),(2560,1080),(3840,1080),(1024,768),(1280,720),(240,160)]
def run_case(case):
    region,(w,h) = case
    changes=[]
    pictures={}
    for variant in ['before','after']:
        d=OUT/f'{region}-{w}x{h}'/variant;d.mkdir(parents=True,exist_ok=True)
        (d/'kh_com.ini').write_text('config_version=11\nview_mode=4\nzoom=6\nscale_mode=0\nhd_text=0\nfullscreen=0\nvsync=0\n')
        e={k:v for k,v in os.environ.items() if not k.startswith(('KHCOM_','SDL_'))}
        e.update(SDL_VIDEO_DRIVER='dummy',SDL_AUDIO_DRIVER='dummy',KHCOM_FAST='1',KHCOM_MENU_FRAMECLOCK='1',KHCOM_WINDOW=f'{w}x{h}',KHCOM_MODE='60:mode_title',KHCOM_KEYS='700-702:START',KHCOM_SHOTS='600,900',KHCOM_OUTPUT_SHOT='600,900',KHCOM_QUIT='905')
        if variant=='before':e['KHCOM_TITLE_BANDS_OFF']='1'
        rom={'eu':'B8CP','us':'B8CE','jp':'B8CJ'}[region]
        subprocess.run([str(ROOT/f'build/ra-{region}/khcom.exe'),'--rom',str(ROOT/f'roms/{rom}.gba'),'--data-dir',str(d),'--language','es'],cwd=d,env=e,timeout=180,check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
        pictures[variant]={}
        for kind in ['shot','out']:
            for frame in [600,900]:
                name=f'{kind}_{frame:05d}'
                im=Image.open(d/f'{name}.bmp').convert('RGB');im.save(d/f'{name}.png');pictures[variant][name]=im
    for frame in [600,900]:
        assert not ImageChops.difference(pictures['before'][f'shot_{frame:05d}'],pictures['after'][f'shot_{frame:05d}']).getbbox(), 'Native title changed'
        a=pictures['before'][f'out_{frame:05d}'];b=pictures['after'][f'out_{frame:05d}']
        box=ImageChops.difference(a,b).getbbox();changes.append(box)
        scale=min(w/240,h/160);left=(w-240*scale)/2;right=left+240*scale
        # Every modified pixel belongs to the margins, never the original picture.
        middle=ImageChops.difference(a,b).crop((round(left+1),0,round(right-1),h))
        assert not middle.getbbox(), 'Band overwrote native image'
        if w/h>1.5:
            assert box, 'Fixture did not exercise bands'
            # Press-start right edge; brightness left edge. Inner samples avoid text.
            row=145 if frame==600 else 146
            yy=int((h-160*scale)/2+(row+0.5)*scale)
            if frame==600: assert b.getpixel((w-1,yy))==b.getpixel((int(right)-2,yy)), 'Right seam'
            else: assert b.getpixel((0,yy))==b.getpixel((int(left)+2,yy)), 'Left seam'
    result={'region':region,'size':[w,h],'native_pixels_identical':True,'changed_boxes':changes,'status':'PASS'}
    print(f'{region} {w}x{h}: PASS',flush=True)
    return result
if __name__=='__main__':
    with ThreadPoolExecutor(max_workers=3) as pool:results=list(pool.map(run_case,[(r,s) for r in ['eu','us','jp'] for s in SIZES]))
    (OUT/'report.json').write_text(json.dumps(results,indent=2)+'\n')
    print('PASS 18 comparisons / 36 native runs: title pixels identical, band margins continuous')
