import subprocess,json,array,math
from pathlib import Path
from PIL import Image,ImageDraw,ImageChops,ImageStat
exec(open('edit.py').read().split('# shot:')[0])
T=Path(T);f=T/'ReKHCoM_trailer_v4.mp4';probe=FF.replace('ffmpeg.exe','ffprobe.exe')
meta=json.loads(subprocess.check_output([probe,'-v','error','-show_streams','-show_format','-of','json',str(f)],creationflags=0x08000000))
v=next(s for s in meta['streams'] if s['codec_type']=='video');a=next(s for s in meta['streams'] if s['codec_type']=='audio')
assert (v['width'],v['height'],v['r_frame_rate'])==(1920,1080,'60/1')
assert abs(float(v['duration'])-float(a['duration']))<.1
subprocess.run([FF,'-v','error','-threads','2','-i',str(f),'-f','null','-'],check=True,creationflags=0x08000000)
# Compare the actual export to the same source moment. An xfade offset
# shifts the second stream's clock: forgetting trim=start=3.8 looks plausible
# on backgrounds, but splits Sora and delays the return to x4.
camera={}
for time,box,scale in [(4.2,(1100,0,1920,1080),'scale=3240:1822:flags=neighbor,crop=1920:1080'),(7.0,(0,0,1920,500),'null')]:
 actual=T/'previews'/f'camera_actual_{time}.png';expected=T/'previews'/f'camera_expected_{time}.png'
 for src,png,filters in [(f,actual,'null'),(T/'clips/00f_castillo_paseo.mov',expected,scale)]:
  subprocess.run([FF,'-v','error','-y','-ss',str(time),'-i',str(src),'-vf',filters,'-frames:v','1',str(png)],check=True,creationflags=0x08000000)
 diff=ImageChops.difference(Image.open(actual).convert('RGB').crop(box),Image.open(expected).convert('RGB').crop(box))
 error=sum(ImageStat.Stat(diff).mean)/3
 assert error<8,('intro source time/zoom mismatch',time,error)
 camera[str(time)]={'mean_rgb_error':error,'max_allowed':8}
checks={}
for name,start,duration in [('silent_intro',0,3.65),('silent_postcredits',66.5,4.8),('music_after_reveal',4.1,1.0)]:
 r=subprocess.run([FF,'-v','error','-ss',str(start),'-i',str(f),'-t',str(duration),'-vn','-f','f32le','-acodec','pcm_f32le','-'],capture_output=True,check=True,creationflags=0x08000000)
 x=array.array('f');x.frombytes(r.stdout);assert len(x)>0
 peak=max(map(abs,x));rms=math.sqrt(sum(z*z for z in x)/len(x));checks[name]={'peak':peak,'rms':rms}
 if name.startswith('silent'):assert peak<1e-6,(name,peak)
 else:assert rms>.001,(name,rms)
for act,times in [('A',[.8,3.4,4.2,7]),('B',[14,16,19.5,26]),('D',[30.5,33,36,39]),('E',[40.5,44.7,48.7,54.5]),('F',[57.7,62.5,68.8,70.8])]:
 sheet=Image.new('RGB',(1280,768))
 for i,t in enumerate(times):
  png=T/'previews'/f'final_{act}_{i}.png'
  subprocess.run([FF,'-v','error','-y','-ss',str(t),'-i',str(f),'-vf','scale=640:360','-frames:v','1',str(png)],check=True,creationflags=0x08000000)
  sheet.paste(Image.open(png),(i%2*640,i//2*384));ImageDraw.Draw(sheet).text((i%2*640,i//2*384+360),f'{t:.1f}s',fill='white')
 sheet.save(T/'previews'/f'final_{act}.png')
report={'video':{k:v[k] for k in ['codec_name','width','height','r_frame_rate','duration']},'audio':{k:a[k] for k in ['codec_name','sample_rate','channels','duration']},'audio_checks':checks,'camera_checks':camera,'decoded_without_errors':True}
(T/'verification.json').write_text(json.dumps(report,indent=2));print(json.dumps(report),flush=True)
