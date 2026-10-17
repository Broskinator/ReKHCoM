"""Actual launcher settings round trips and optional copied-save Riku loading.
Never edits the supplied save. Requires private EU ROM for --save cases.
"""
import argparse,datetime,hashlib,json,os,shutil,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'port/tools/trailer'))
from fx import fx
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--save',type=Path)
p.add_argument('--write-roundtrip',action='store_true',help='Write Riku1 through the save-point menu on the COPY and verify isolation')
p.add_argument('--build',type=Path,default=ROOT/'build/rel-eu')
a=p.parse_args();build=a.build.resolve()
out=ROOT/'build/riku-persistence'/datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f');out.mkdir(parents=True)
report=[]
launcher=out/'launcher';launcher.mkdir()
for n in ['ReKHCoM.exe','SDL3.dll']:shutil.copy2(build/n,launcher/n)
shutil.copytree(ROOT/'port/assets/fonts',launcher/'assets/fonts')
for value in [0,1]:
 (launcher/'kh_com.ini').write_text(f'config_version=11\nzoom=8\ncheat_unlock_riku={value}\n')
 for boot in range(2):
  shots=launcher/f'shots-{value}-{boot}';shots.mkdir()
  env={k:v for k,v in os.environ.items() if not k.startswith(('DECOMPUI_','KHCOM_','SDL_'))}
  env.update(DECOMPUI_SHOT=str(shots),DECOMPUI_REWRITE_SETTINGS='1')
  subprocess.run([str(launcher/'ReKHCoM.exe')],cwd=launcher,env=env,check=True,timeout=90,creationflags=0x08000000)
  assert f'cheat_unlock_riku={value}\n' in (launcher/'kh_com.ini').read_text()
  report.append({'case':'launcher-restart','value':value,'boot':boot,'passed':True})
if a.save:
 data=a.save.read_bytes();assert len(data)==65536,'Expected raw SRAM'
 for cheat,cleared in [(0,0),(1,0),(0,1)]:
  name=f'riku-load-{out.name}-{cheat}-{cleared}';work=ROOT/'build'/('trailer182_'+name);work.mkdir()
  fixture=bytearray(data)
  if cleared:
   # Synthetic completion on the isolated COPY only, with native checksums.
   for at in [0x10,0x50]:
    fixture[at+28]|=1
    fixture[at+24:at+26]=b'\0\0'
    total=sum(int.from_bytes(fixture[i:i+2],'little') for i in range(at,at+0x40,2))
    total=(total&65535)+(total>>16)
    fixture[at+24:at+26]=((~(total+(total>>16)))&65535).to_bytes(2,'little')
  (work/'kh_com.sav').write_bytes(fixture)
  write=a.write_roundtrip and cheat==1 and not cleared
  modes=['10:mode_title','1800:progress']+(['1900:mapsave','2300:progress'] if write else [])
  keys=['900-903:A','1100-1103:A','1400-1403:DOWN','1450-1453:DOWN','1550-1553:A']+(['2000-2003:LEFT','2050-2053:A'] if write else [])
  fx(name,modes,[1400,2300 if write else 1800],save=False,build=str(build.relative_to(ROOT/'build')),
     keys=keys,
     ini=f'cheat_unlock_riku={cheat}\n',extra={'KHCOM_WINDOW':'1280x720'})
  log=(work/'kh_com.log').read_text(errors='replace')
  assert f'load-menu: riku={int(bool(cheat or cleared))} sio=0 flags={32 if cleared else 0}' in log,log
  if cheat or cleared: assert f'load-menu: loaded slot=2 flags={264 | (32 if cleared else 0)}' in log,log
  else: assert 'load-menu: loaded slot=2' not in log,'Riku exposed without access'
  after=(work/'kh_com.sav').read_bytes()
  if write:
   assert after[0x2f20:0x6b70]==data[0x2f20:0x6b70],'Sora files modified'
   assert after[0x26f0:0x2f20]==data[0x26f0:0x2f20],'Other Riku file modified'
   assert after[0x1ec0:0x26f0]!=data[0x1ec0:0x26f0],'Native save did not write'
   assert not int.from_bytes(after[44:46],'little')&1,'Fake Sora completion'
   assert len(list(work.glob('*.sav')))==1
  else: assert after==fixture,'SRAM copy modified'
  report.append({'case':'load-and-save-riku' if write else 'riku-slot-access','cheat':cheat,'sora_cleared':cleared,'other_slots_preserved':True,'passed':True})
 assert a.save.read_bytes()==data,'Source changed during test; check for concurrent player writes'
(out/'report.json').write_text(json.dumps(report,indent=2))
print('PASS',len(report),'checks',out)
