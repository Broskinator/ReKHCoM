"""Compare final SDL frames across menu transitions, ultrawide and resizing.
Reference must be the pre-opaque-compositor build. Private EU ROM required.
"""
import argparse,hashlib,json
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
from PIL import Image
from test_scene_transitions import run,R
p=argparse.ArgumentParser();p.add_argument('--reference',required=True);args=p.parse_args()
frames=[170,185,220,260,479]
cases=[(mode,win,resize) for mode in ['Mode_Deck','mode_mapinspect','mode_status','door']
       for win,resize in [('1536x864',None),('2560x720',None),('1536x864','200:2560x720')]]
def check(c):
 mode,win,resize=c;rows=[]
 for tag,build in [('before',args.reference),('after','rel-eu')]:
  modes='60:room:20001,180:roomtransition' if mode=='door' else '60:room:10901,120:fieldfreeze,180:'+mode
  d=run('v19-'+mode+'-'+win+('-resize' if resize else '')+'-'+tag,frames,keys='470-472:B',modes=modes,
        build=build,win=win,extra={'KHCOM_RESIZE':resize} if resize else {})
  log=(d/'kh_com.log').read_text(errors='replace');assert 'harness: no mode' not in log
  expected='Mode_MapFld' if mode=='door' else mode
  assert 'mode '+expected in log,(mode,log[-1000:])
  hashes=[]
  for f in frames:
   file=d/f'out_{f:05d}.bmp'
   with Image.open(file) as im:
    hashes.append(hashlib.sha256(im.convert('RGB').tobytes()).hexdigest())
    if f in [220,260]:im.save(file.with_suffix('.png'))
   file.unlink()
  rows.append(hashes)
 assert rows[0]==rows[1],('SDL output changed',c)
 print(c,'5 exact frames',flush=True)
 return {'mode':mode,'window':win,'resize':resize,'identical_frames':len(frames)}
with ThreadPoolExecutor(max_workers=2) as pool:report=list(pool.map(check,cases))
out=R/'build/v19-composition-regression.json';out.write_text(json.dumps(report,indent=2));print(out)
