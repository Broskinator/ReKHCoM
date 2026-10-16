import subprocess, json, os
exec(open('edit.py').read().split('# shot:')[0])
def dur(p):
    r=subprocess.run([FF.replace('ffmpeg.exe','ffprobe.exe'),'-v','error','-show_entries','format=duration','-of','json',p],capture_output=True,text=True,creationflags=0x08000000)
    return float(json.loads(r.stdout)['format']['duration'])
acts=['A','B','D','E','F']
TR={'A':('heart',1.0),'B':('black',0.7),'D':('flash',0.5),'E':('flash',0.9)}
P=[os.path.join(OUT,f'act_{a}.mkv') for a in acts]; D=[dur(p) for p in P]
args=[FF,'-hide_banner','-loglevel','error','-y']
for p in P: args+=['-i',p]
args+=['-i',os.path.join(AU,'m_titulo.wav'),'-i',os.path.join(AU,'m_combate_agrabah.wav'),'-i',os.path.join(AU,'m_titulo.wav')]
NI=len(P)
fv=[]; fa=[]; v='0:v'; a='0:a'; t=D[0]; starts=[0]
for k in range(1,len(acts)):
    kind,td=TR[acts[k-1]]; off=t-td; starts.append(off)
    x=f'xfade=transition={trans(kind)}:duration={td}:offset={off:.4f}'
    if kind=='heart': x+=f":expr='{HEART}'"
    if kind=='slider': x+=f":expr='{SLIDER}'"
    fv.append(f'[{v}][{k}:v]{x}[x{k}]'); v=f'x{k}'
    fa.append(f'[{a}][{k}:a]acrossfade=d={td}[y{k}]'); a=f'y{k}'
    t=off+D[k]
total=t
tB=starts[1]; tD=starts[2]; tE=starts[3]; tF=starts[4]
freeze=tF+0.5
# music 1: title theme intro, until heart transition ends
T_REVEAL=3.8
L1=tB+1.0-T_REVEAL
fa.append(f'[{NI}:a]atrim=0:{L1},asetpts=PTS-STARTPTS,afade=t=in:d=0.25,afade=t=out:st={L1-1.2}:d=1.2,adelay={int(T_REVEAL*1000)}:all=1[m1]')
# music 2: battle theme from act B to the freeze; dip during act C
L2=freeze-tB
fa.append(f"[{NI+1}:a]atrim=0:{L2},asetpts=PTS-STARTPTS,afade=t=in:d=0.4,afade=t=out:st={L2-0.9}:d=0.9,adelay={int(tB*1000)}:all=1[m2]")
# music 3: title theme for the ending
L3=min(total-tF,8.6)  # the music ends with the disclaimer; the post-credits are silent
fa.append(f'[{NI+2}:a]atrim=10:{10+L3},asetpts=PTS-STARTPTS,afade=t=in:d=0.6,afade=t=out:st={L3-4.0}:d=4.0,adelay={int(tF*1000)}:all=1[m3]')
fa.append(f'[m1][m2][m3]amix=inputs=3:normalize=0:duration=longest,volume=0.85[mus]')
fa.append(f'[{a}]volume=0.9[sfx]')
fa.append(f'[mus][sfx]amix=inputs=2:normalize=0:duration=longest,atrim=0:{total},loudnorm=I=-14:TP=-1.5:LRA=11,aresample=48000[aout]')
out=os.path.join(T,'ReKHCoM_trailer_v4.mp4')
args+=['-filter_complex_threads','2','-filter_complex',';'.join(fv+fa),'-map',f'[{v}]','-map','[aout]','-c:v','libx264','-preset','slow','-crf','16','-pix_fmt','yuv420p','-profile:v','high','-threads','2','-movflags','+faststart','-c:a','aac','-b:a','320k',out]
r=subprocess.run(args,creationflags=0x08000000,capture_output=True,text=True)
print('ok' if r.returncode==0 else r.stderr[-3000:], 'total %.2f'%total, 'starts', [round(s,2) for s in starts])
