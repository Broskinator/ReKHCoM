import sys,os
from pathlib import Path
exec(open('fx.py').read())
source=open('trailer_rec.py').read();a=source.index('def blocks_to_wav');b=source.index("used={'03_",a)
import array,wave
exec(source[a:b])
from paths import T
for name,song,frames in [('m_titulo',6,1500),('m_combate_agrabah',23,3900)]:
 raw=T/'audio'/(name+'.raw');out=raw.with_suffix('.wav')
 if out.exists():continue
 fx('music_'+name,[],[frames],keys=[],lang=None,save=False,extra={'KHCOM_SONG':f'60:{song}','KHCOM_WAVDUMP':str(raw),'KHCOM_WINDOW':'640x360'},ini='music_volume=100\nsfx_volume=0\n')
 blocks_to_wav(raw,str(raw)+'.rates',60,frames,str(out));raw.unlink();Path(str(raw)+'.rates').unlink();print(name,'recorded',flush=True)
