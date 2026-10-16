"""Compare native EU/US/JP SDL output against the published v1.8.3 package.
Only pixel text is used here; HD occlusion has its independent oracle.
"""
import argparse,hashlib,json,sys,zipfile,shutil,time
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
from PIL import Image
R=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(R/'port/tools/trailer'))
from fx import fx
p=argparse.ArgumentParser();p.add_argument('--reference',type=Path,default=R/'build/releases/ReKHCoM-v1.8.3-windows-x86.zip');args=p.parse_args()
with zipfile.ZipFile(args.reference) as z:
 for region in ['eu','us','jp']:
  d=R/'build/v19-reference'/region;d.mkdir(parents=True,exist_ok=True)
  name=next(n for n in z.namelist() if n.endswith('/khcom_'+region+'.exe'))
  (d/'khcom.exe').write_bytes(z.read(name))
  shutil.copy2(R/'build/rel-eu/SDL3.dll',d/'SDL3.dll')
cases=[(r,s,w) for r in ['eu','us','jp'] for s in ['field','battle','deck'] for w in ['1920x1080','2560x720']]
def check(c):
 region,scene,window=c;hashes=[]
 for tag,build in [('before','v19-reference/'+region),('after','rel-'+region)]:
  name='v19-reg-'+region+'-'+scene+'-'+window+'-'+tag
  work=R/'build'/('trailer182_'+name)
  modes=[] if scene=='battle' else ['60:room:10901','120:fieldfreeze']+(['180:Mode_Deck'] if scene=='deck' else [])
  frame=600 if scene=='battle' else 260
  extra={'KHCOM_WINDOW':window,'KHCOM_TRACE_MODES':'1'}
  if scene=='battle':extra['KHCOM_BATTLE']='10:4:0'
  shots=fx(name,modes,[frame],keys=['60-63:A'] if scene=='battle' else [],save=False,build=build,
           rom={'eu':'B8CP','us':'B8CE','jp':'B8CJ'}[region],lang=None,extra=extra)
  assert len(shots)==1,(c,tag,shots)
  log=(work/'kh_com.log').read_text(errors='replace');assert 'harness: no mode' not in log
  assert {'eu':'B8CP','us':'B8CE','jp':'B8CJ'}[region]+'.gba' in log
  with Image.open(shots[0]) as im:
   hashes.append(hashlib.sha256(im.convert('RGB').tobytes()).hexdigest());im.save(work/'presented.png')
  shots[0].unlink()
 assert hashes[0]==hashes[1],('region output changed',c,hashes)
 print(c,'exact SDL image',flush=True)
 return {'region':region,'scene':scene,'window':window,'identical':True,'sha256':hashes[0]}
with ThreadPoolExecutor(max_workers=2) as pool:report=list(pool.map(check,cases))
(R/'build/v19-regions-report.json').write_text(json.dumps(report,indent=2))
