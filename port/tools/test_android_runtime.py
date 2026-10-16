"""ROM-private battle smoke on an explicitly selected Android emulator/device.
Uses the separate --smoke APK/application ID; never touches the player's app.
This validates the selected ABI only, not physical ARM performance.
"""
import argparse,hashlib,io,json,subprocess,time
from pathlib import Path
from PIL import Image
R=Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser();p.add_argument('--serial',required=True);p.add_argument('--regions',nargs='+',choices=['eu','us','jp'],default=['eu','us','jp']);args=p.parse_args()
ADB=R.parent/'toolchain/android/sdk/platform-tools/adb.exe';PKG='com.rekhcom.android.smoke'
OUT=R/'build/android-runtime-battles';OUT.mkdir(exist_ok=True)
def adb(*commands,check=True):
 return subprocess.run([str(ADB),'-s',args.serial,*map(str,commands)],check=check,capture_output=True)
def private(*commands,check=True):return adb('shell','run-as',PKG,*commands,check=check)
report=[]
for region in args.regions:
 code={'eu':'B8CP','us':'B8CE','jp':'B8CJ'}[region]
 adb('shell','am','force-stop',PKG);adb('logcat','-c')
 private('mkdir','-p','files/roms')
 source=R/'roms'/f'{code}.gba';temp=f'/data/local/tmp/rekhcom-test-{code}.gba'
 adb('push',source,temp);private('cp',temp,f'files/roms/{code}.gba')
 digest=hashlib.sha1(source.read_bytes()).hexdigest()
 assert private('sha1sum',f'files/roms/{code}.gba').stdout.decode().split()[0]==digest
 private('rm','-f','files/kh_com.log','files/out_00600.bmp')
 adb('shell','run-as',PKG,'am','start','--user','0','-n',PKG+'/com.rekhcom.android.GameActivity',
     '--es','region',region,'--es','rom',f'/data/user/0/{PKG}/files/roms/{code}.gba','--es','language','es')
 start=time.monotonic()
 while private('ls','files/out_00600.bmp',check=False).returncode:
  time.sleep(2)
  log=private('cat','files/kh_com.log',check=False).stdout.decode(errors='replace')
  assert 'FATAL:' not in log,log
  crashes=adb('logcat','-d','-b','crash').stdout.decode(errors='replace')
  if PKG in crashes and ('Fatal signal' in crashes or 'FATAL EXCEPTION' in crashes):raise RuntimeError(crashes)
  if time.monotonic()-start>180:raise TimeoutError(region+': frame600 not reached')
 bmp=adb('exec-out','run-as',PKG,'cat','files/out_00600.bmp').stdout
 with Image.open(io.BytesIO(bmp)) as im:
  rgb=im.convert('RGB');assert len(rgb.getcolors(rgb.width*rgb.height))>100
  rgb.save(OUT/(region+'.png'));image_hash=hashlib.sha256(rgb.tobytes()).hexdigest();size=list(rgb.size)
 while adb('shell','pidof',PKG+':game',check=False).returncode==0:
  if time.monotonic()-start>240:raise TimeoutError(region+': test did not exit at frame900')
  time.sleep(2)
 log=private('cat','files/kh_com.log').stdout.decode(errors='replace');(OUT/(region+'.log')).write_text(log,encoding='utf-8')
 assert 'mode mode_battle' in log and 'FATAL:' not in log,log
 crashes=adb('logcat','-d','-b','crash').stdout.decode(errors='replace');assert 'Fatal signal' not in crashes and 'FATAL EXCEPTION' not in crashes,crashes
 row={'region':region,'serial':args.serial,'battle_frame600':True,'finished_frame900':True,'pixels':size,'image_sha256':image_hash,'seconds':round(time.monotonic()-start,2)}
 report.append(row);print(json.dumps(row),flush=True)
(OUT/'report.json').write_text(json.dumps(report,indent=2))
