import sys
exec(open('edit.py').read())
R=lambda w,d,ss=0.5,**k: dict(src=f'05_sala_mundo_{w}',ss=ss,d=d,**k)
C=lambda n,ss,d,**k: dict(src=f'04_combate_arena_{n}',ss=ss,d=d,**k)
FL=('flash',0.22)
ACTS={
'A':[dict(src='00e_intro_castillo',d=9.2,card=('c01',0.4,2.5,390),card2=('c02',5.0,9.2,390),tr=('flash',0.6)),
     dict(src='03_entrada_combate_corazon',d=1.38,sfxvol=1.3,tr=('flash',0.6)),
     dict(src='bg:13_titulo',ss=1,d=5.0,card=('c03',0.5,5.0))],
'B':[R(2,1.4,ss=0.2,card=('c04',0.3,3.4,390),tr=('cut',1/60)),R(4,0.55,ss=1.6,tr=('cut',1/60)),R(5,0.55,ss=2.15,tr=('cut',1/60)),
     R(6,0.55,ss=2.7,tr=('cut',1/60)),R(8,0.8,ss=3.15,tr=('fade',0.3)),R('3_salto',2.4,ss=0.6,tr=('flash',0.4)),
     C(1,0.4,3.0,card=('c05',0.3,2.9,390),tr=FL),C(5,0.8,1.6,tr=FL),C(0,3.0,1.6,tr=FL),C(2,1.0,1.5,tr=FL),C(6,4.4,1.6,tr=FL),C(4,4.4,1.6,tr=FL),C(8,2.4,1.9)],
'D':[dict(src='black',d=2.6,card=('c08',0.2,2.6),tr=('open',0.8)),
     dict(src='07_widescreen_2x32x9',d=9.0,line=True)],
'E':[dict(src='08a_pelicula_extendida',ss=7.0,d=3.5,tr=('flash',0.4)),
     dict(src='09c_texto_barrido',d=5.4,tr=('card',0.5)),
     dict(src='10_mosaico_idiomas',ss=0.1,d=2.8,push=0.04,tr=('card',0.5)),
     dict(src='15_menu_opciones',ss=0.6,d=4.2,push=0.04,sfxvol=0,card=('c12',0.3,4.2,390),tr=('fade',0.4)),
     dict(src='14b_launcher',ss=0,d=3.2,card=('c09',0.2,3.2,390))],
'F':[dict(src='bg:13_titulo',ss=0.5,d=6.5,card=('c10',0.7,6.5),tr=('fade',0.9)),
     dict(src='black',d=4.0,card=('c11',0.4,3.7),tr=('fade',1.0)),
     dict(src='black',d=1.6,tr=('fade',0.6)),
     dict(src='16_postcreditos_android',d=6.0,tr=('fade',1.0)),
     dict(src='black',d=1.0)],
}
for a in (sys.argv[1:] or ACTS): build('act_'+a, ACTS[a])
