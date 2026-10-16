import wave,array
from pathlib import Path
exec(open('fx.py').read())
source=open('trailer_rec.py').read();a=source.index('def blocks_to_wav');b=source.index("used={'03_",a);exec(source[a:b])
exec(open('edit.py').read().split('# shot:')[0])
name='00f_castillo_paseo';out=Path(CL)/(name+'.mov');raw=Path(AU)/'sfx'/(name+'.raw')
keys=[]
for i in range(14):
 d=['DOWN','LEFT','UP','RIGHT'][i%4]
 f=1450+i*45;keys.append(f'{f}-{f+44}:{d}')
fx('intro_castle', ['1400:castle'], [2072],keys=keys,save=True,lang='en',extra={'KHCOM_TRACE_MODES':'1','KHCOM_WINDOW':'1920x1080','KHCOM_RECORD':f'1450-2070:{out}','KHCOM_FFMPEG':FF,'KHCOM_WAVDUMP':str(raw)},ini='music_volume=0\n')
blocks_to_wav(raw,str(raw)+'.rates',1450,2070,str(raw.with_suffix('.wav')));raw.unlink();Path(str(raw)+'.rates').unlink();print(name,'recorded',flush=True)

log=(ROOT/'build/trailer182_intro_castle/kh_com.log').read_text(errors='replace')
assert 'mode Mode_MapFld' not in log and 'mode mode_worldselect' not in log, 'walk left the castle'
