"""Regional title-access regression, using private ROMs in isolated directories."""
import json,re,sys
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'port/tools/trailer'))
from fx import fx

def check(case):
 region,enabled=case
 name=f'riku-regression-{region}-{enabled}'
 work=ROOT/'build'/('trailer182_'+name)
 # Never reuse player saves; this fixture must enter with no completion flags.
 work.mkdir(parents=True,exist_ok=True)
 for file in work.glob('*.sav'): file.unlink()
 keys=['900-903:A']+(['1000-1003:DOWN','1100-1103:A'] if enabled else [])
 shots=fx(name,['10:mode_title','1300:progress'],[960,1300],keys=keys,save=False,
          build='rel-'+region,rom={'eu':'B8CP','us':'B8CE','jp':'B8CJ'}[region],
          lang='es' if region=='eu' else None,ini=f'cheat_unlock_riku={enabled}\n',
          extra={'KHCOM_WINDOW':'1280x720'})
 log=(work/'kh_com.log').read_text(errors='replace')
 assert 'harness: no mode' not in log,log
 assert f'layout={4 if enabled else 1} choice={4 if enabled else 0} flags=0 riku={enabled}' in log,log
 flags=int(re.search(r'harness: progress flags=(\d+)',log)[1])
 assert not flags & 32,('Sora clear was fabricated',region,flags)
 if enabled: assert flags & 8,('Riku selection did not start Riku',region,flags)
 assert len(shots)==2
 print(region,'cheat',enabled,'PASS flags',flags,flush=True)
 return {'region':region,'enabled':bool(enabled),'flags_after_selection':flags,'passed':True}

if __name__=='__main__':
 with ThreadPoolExecutor(max_workers=2) as pool:
  result=list(pool.map(check,[(r,e) for r in ['eu','us','jp'] for e in [0,1]]))
 out=ROOT/'build/riku-access-report.json';out.write_text(json.dumps(result,indent=2))
 print(out)
