import os, sys
exec(open('fx.py').read())
from paths import FF,T
OUT=str(T/'clips')

M=lambda m: ["300:room:10000",f"600:{m}"]
jobs=[
 ('01a_idioma_gba_pixel_exacto', dict(modes=[],a=60,b=420,lang=None,narrow=0)),
 ('01b_idioma_llenar_altura', dict(modes=[],a=60,b=420,lang=None)),
 ('02a_diario_pc', dict(modes=M('mode_jiminy'),a=700,b=1180)),
 ('02b_diario_gba_pixel_exacto', dict(modes=M('mode_jiminy'),a=700,b=1180,narrow=0)),
 ('03_entrada_combate_corazon', dict(modes=[],a=58,b=140,keys=['60-63:A'],extra={'KHCOM_BATTLE':'10:4:0'})),
 ('06a_mapamundi_pc', dict(modes=M('mode_allmap'),a=700,b=1000)),
 ('06b_mapamundi_gba_pixel_exacto', dict(modes=M('mode_allmap'),a=700,b=1000,narrow=0)),
 ('06c_tienda_pc', dict(modes=M('mode_ms_top'),a=700,b=1000)),
 ('06d_tienda_gba_pixel_exacto', dict(modes=M('mode_ms_top'),a=700,b=1000,narrow=0)),
 ('06e_naipes_mundo_pc', dict(modes=M('mode_worldinspect'),a=700,b=1000)),
 ('06f_vortice_pc', dict(modes=M('mode_worldwarp'),a=800,b=1100)),
 ('08a_pelicula_extendida', dict(modes=[],a=240,b=960,extra={'KHCOM_MOVIE':'1'})),
 ('08b_pelicula_subtitulos_hd', dict(modes=[],a=2640,b=3360,extra={'KHCOM_MOVIE':'1'},ini='hd_text=1\n')),
 ('09a_tienda_texto_pixel', dict(modes=M('mode_ms_top'),a=700,b=880)),
 ('09b_tienda_texto_hd', dict(modes=M('mode_ms_top'),a=700,b=880,ini='hd_text=1\n')),
 ('13_titulo', dict(modes=[],a=820,b=1180,keys=['200-201:A'])),
 ('14_opciones', dict(modes=[],a=500,b=680,extra={'KHCOM_OPTIONS_SHOT':'500'})),
]
for s_ in (0,1,2,3,4,5,6,7,8):
    jobs.append((f'04_combate_arena_{s_}', dict(modes=[],a=150,b=510,keys=['60-63:A'],extra={'KHCOM_BATTLE':f'10:{s_}:0','KHCOM_MONKEY':f'{s_+3}:120'})))
for w in (1,2,3,4,5,6,8,9):
    jobs.append((f"05_sala_mundo_{w}", dict(modes=[f"300:room:1{w:02d}01"],a=430,b=670,keys=["420-700:UP"])))
for l,b_,r,lg in (('fr','rel-eu',None,'fr'),('de','rel-eu',None,'de'),('es','rel-eu',None,'es'),('it','rel-eu',None,'it'),('us','rel-us','B8CE','en'),('jp','rel-jp','B8CJ',None)):
    jobs.append((f'10_idioma_{l}', dict(modes=M('mode_jiminy'),a=700,b=880,build=b_,rom=r,lang=lg)))
jobs += [
 ('07a_triptico_combate_32x9', dict(modes=[],a=150,b=750,keys=['60-63:A'],extra={'KHCOM_BATTLE':'10:4:0','KHCOM_MONKEY':'5:120'},win='2560x720')),
 ('07d_widescreen_halloween_32x9', dict(modes=[],a=150,b=750,keys=['60-63:A'],extra={'KHCOM_BATTLE':'10:7:0','KHCOM_MONKEY':'11:120'},win='2560x720')),

 ('07c_triptico_menu_32x9', dict(modes=M('mode_worldinspect'),a=700,b=1300,win='2560x720')),
]
jobs += [
 ('05_sala_mundo_3_salto', dict(modes=["300:room:10301"],a=430,b=670,keys=['420-700:UP','480-486:B','500-506:B','540-546:B'])),
 ('00a_ciudad_gba_4x3', dict(modes=["300:room:10901"],a=430,b=830,keys=['420-640:UP','640-830:RIGHT'],view=0,narrow=0)),
 ('00b_ciudad_pc_16x9', dict(modes=["300:room:10901"],a=430,b=830,keys=['420-640:UP','640-830:RIGHT'],view=4,narrow=1)),
 ('15_menu_opciones', dict(modes=["300:room:10901"],a=500,b=975,keys=['420-470:UP'],extra={'KHCOM_VPAD':','.join(['480-520:BACK','490-494:START']+[f'{f}-{f+3}:DOWN' for f in range(560,960,26)]+['985-988:EAST'])})),
]
jobs += [
 ('00c_castillo_gba_4x3', dict(modes=["1400:castle"],a=1450,b=1870,keys=['1450-1900:DOWN'],view=0,narrow=0)),
 ('00d_castillo_pc_16x9', dict(modes=["1400:castle"],a=1450,b=1870,keys=['1450-1900:DOWN'])),
]

import wave,array,json,hashlib
from concurrent.futures import ThreadPoolExecutor

def blocks_to_wav(raw, rates, a, b, out):
    data=array.array('f'); data.frombytes(open(raw,'rb').read())
    lines=[tuple(map(int,l.split())) for l in open(rates)]
    pos=0; pcm=array.array('h')
    for i,(n,r) in enumerate(lines):
        if a<=i<=b:
            for k in range(800):
                if n==0: pcm.extend((0,0)); continue
                t=k*(n-1)/799; j=int(t); f=t-j; j2=min(j+1,n-1)
                for c in (0,1):
                    v=data[(pos+j)*2+c]*(1-f)+data[(pos+j2)*2+c]*f
                    pcm.append(int(max(-1,min(1,v))*32767))
        pos+=n
    with wave.open(str(out),'wb') as w:
        w.setnchannels(2); w.setsampwidth(2); w.setframerate(48000); w.writeframes(pcm.tobytes())
    return len(lines)
used={'03_entrada_combate_corazon','08a_pelicula_extendida','09a_tienda_texto_pixel','09b_tienda_texto_hd','13_titulo','15_menu_opciones','00c_castillo_gba_4x3','00d_castillo_pc_16x9','05_sala_mundo_3_salto','07a_triptico_combate_32x9','07d_widescreen_halloween_32x9'}
used.update(f'04_combate_arena_{n}' for n in [0,1,2,4,5,6,8])
used.update(f'05_sala_mundo_{n}' for n in [2,4,5,6,8])
used.update(f'10_idioma_{n}' for n in ['fr','de','es','it','us','jp'])
def capture(job):
 name,j=job;j=dict(j);out=Path(OUT)/(name+'.mov');wav=Path(OUT).parent/'audio/sfx'/(name+'.wav')
 if out.exists() and wav.exists():return name
 a=j.pop('a');b=j.pop('b');win=j.pop('win','1920x1080');modes=j.pop('modes');keys=j.pop('keys',())
 e=dict(j.pop('extra',None) or {});raw=wav.with_suffix('.raw')
 e.update(KHCOM_WINDOW=win,KHCOM_RECORD=f'{a}-{b}:{out}',KHCOM_FFMPEG=FF,KHCOM_WAVDUMP=str(raw))
 j['ini']=j.get('ini','')+'music_volume=0\n';j.setdefault('lang','en');j.setdefault('build','rel-eu')
 if not (out.exists() and raw.exists() and Path(str(raw)+'.rates').exists()):
  fx('capture_'+name,modes,[b+2],keys=list(keys),save=any('castle' in m for m in modes),extra=e,**j)
 assert out.exists() and out.stat().st_size>10000,name
 blocks_to_wav(raw,str(raw)+'.rates',a,b,wav)
 raw.unlink();Path(str(raw)+'.rates').unlink()
 print(name,'recorded v1.8.2',flush=True);return name
with ThreadPoolExecutor(max_workers=2) as pool:list(pool.map(capture,[(n,j) for n,j in jobs if n in used]))
