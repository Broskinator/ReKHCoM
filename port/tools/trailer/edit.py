import os, subprocess, sys
from paths import FF,T
CL=str(T/'clips'); SFX=str(T/'audio/sfx'); AU=str(T/'audio'); OUT=str(T/'edit'); CARDS=str(T/'tools/cards')
os.makedirs(OUT,exist_ok=True)
# sfx source for composites
SFXMAP={'06_comparacion_diario':'02a_diario_pc','06_comparacion_mapamundi':'06a_mapamundi_pc','06_comparacion_tienda':'06c_tienda_pc',
        '07_widescreen_2x32x9':'07a_triptico_combate_32x9','10_mosaico_idiomas':'10_idioma_es','09c_texto_barrido':'09a_tienda_texto_pixel','00e_intro_consola':'00d_castillo_pc_16x9'}
HEART=open(os.path.join(os.path.dirname(os.path.abspath(__file__)),'heart_expr.txt')).read().strip()
_b="(1-(0.5-0.5*cos(PI*(1-P))))"
SLIDER=f"if(lt(abs(X-W*{_b}),W*0.0018),if(eq(PLANE,0),250,128),if(gt(X,W*{_b}),B,A))"
def trans(kind):
    return {'flash':'fadewhite','fade':'fade','black':'fadeblack','card':'squeezeh','wipe':'wipeleft',
            'cut':'fade','heart':'custom','open':'circleopen','reveal':'horzopen','slider':'custom'}[kind]
# shot: dict(src=clip|'black'|'bg:<clip>', ss, d, card=(name, t_in, t_out), zoom=bool, sfxvol, tr=(kind,dur) into next)
def build(name, shots):
    args=[FF,'-hide_banner','-loglevel','error','-y']; fv=[]; fa=[]; ni=0; vlabels=[]; alabels=[]
    def inp(a):
        nonlocal ni; args.extend(a); ni+=1; return ni-1
    for k,s in enumerate(shots):
        d=s['d']; src=s.get('src','black'); ss=s.get('ss',0)
        if src=='black':
            i=inp(['-f','lavfi','-t',str(d),'-i','color=c=black:s=1920x1080:r=60'])
            fv.append(f'[{i}:v]format=yuv420p,setsar=1[v{k}b]')
        else:
            clip=src.split(':',1)[-1]
            i=inp(['-threads','1']+(['-stream_loop','-1'] if src.startswith('bg:') else [])+['-ss',str(ss),'-t',str(d),'-i',os.path.join(CL,clip+'.mov')])
            chain=f'[{i}:v]setpts=PTS-STARTPTS,fps=60'
            if src.startswith('bg:'): chain+=',scale=480:270,boxblur=6:2,scale=1920:1080,eq=saturation=0.7,colorchannelmixer=rr=0.28:gg=0.3:bb=0.36,vignette=PI/3.5'
            if s.get('zoom'):
                z0=s['zoom']; z1=s.get('zoom1',1); zd=s.get('zoomd',d); zt=s.get('zoomt0',0)
                u=f"min(1,max(0,(t-{zt})/{zd}))"; zz=f"({z0}+({z1}-{z0})*(3*pow({u},2)-2*pow({u},3)))"
                zc=inp(['-f','lavfi','-t',str(d),'-i','color=c=black:s=1920x1080:r=60'])
                fv.append(chain+f",scale=w='2*trunc(960*{zz})':h='2*trunc(540*{zz})':eval=frame:flags=bicubic[z{k}]")
                chain=f"[{zc}:v][z{k}]overlay=x='(W-w)/2':y='(H-h)/2':eval=frame:shortest=1"
            if s.get('push'):
                p=s['push']
                chain+=f",scale=w='2*trunc(960*(1+{p}*t/{d}))':h='2*trunc(540*(1+{p}*t/{d}))':eval=frame:flags=bicubic,crop=1920:1080"
            if s.get('freeze'): chain+=f",trim=end_frame=1,tpad=stop_mode=clone:stop_duration={d}"
            fv.append(chain+(f',tpad=stop_mode=clone:stop_duration=3,trim=duration={d},setpts=PTS-STARTPTS' if not s.get('freeze') else '')+f',format=yuv420p,setsar=1[v{k}b]')
        cur=f'v{k}b'
        if s.get('card'):
            cn,ti,to=s['card'][:3]; cy=s['card'][3] if len(s['card'])>3 else 0
            if cy:
                g=inp(['-loop','1','-t',str(d),'-framerate','60','-i',os.path.join(CARDS,'shade.png')])
                fv.append(f'[{g}:v]format=rgba,fade=in:st={max(0,ti-0.3)}:d=0.6:alpha=1,fade=out:st={to-0.4}:d=0.6:alpha=1[g{k}]')
                fv.append(f'[{cur}][g{k}]overlay=shortest=1:format=auto,format=yuv420p[v{k}g]'); cur=f'v{k}g'
            j=inp(['-loop','1','-t',str(d),'-framerate','60','-i',os.path.join(CARDS,cn+'.png')])
            cz=f"(1+0.045*t/{d})"
            fv.append(f"[{j}:v]format=rgba,scale=w='2*trunc(960*{cz})':h='2*trunc(540*{cz})':eval=frame:flags=bicubic,fade=in:st={ti}:d=0.8:alpha=1,fade=out:st={to-0.7}:d=0.7:alpha=1[c{k}]")
            fv.append(f"[{cur}][c{k}]overlay=x='(W-w)/2':y='(H-h)/2+{cy}':eval=frame:shortest=1:format=auto,format=yuv420p[v{k}c]"); cur=f'v{k}c'
        if s.get('card2'):
            cn,ti,to,cy=s['card2']
            j=inp(['-loop','1','-t',str(d),'-framerate','60','-i',os.path.join(CARDS,cn+'.png')])
            g=inp(['-loop','1','-t',str(d),'-framerate','60','-i',os.path.join(CARDS,'shade.png')])
            fv.append(f'[{g}:v]format=rgba,fade=in:st={max(0,ti-0.3)}:d=0.6:alpha=1,fade=out:st={to-0.4}:d=0.6:alpha=1[g2{k}]')
            fv.append(f'[{cur}][g2{k}]overlay=shortest=1:format=auto,format=yuv420p[v{k}h]'); cur=f'v{k}h'
            cz=f"(1+0.045*t/{d})"
            fv.append(f"[{j}:v]format=rgba,scale=w='2*trunc(960*{cz})':h='2*trunc(540*{cz})':eval=frame:flags=bicubic,fade=in:st={ti}:d=0.8:alpha=1,fade=out:st={to-0.7}:d=0.7:alpha=1[c2{k}]")
            fv.append(f"[{cur}][c2{k}]overlay=x='(W-w)/2':y='(H-h)/2+{cy}':eval=frame:shortest=1:format=auto,format=yuv420p[v{k}i]"); cur=f'v{k}i'
        if s.get('line'):
            fv.append(f'[{cur}]drawbox=x=0:y=538:w=1920:h=4:color=black:t=fill[v{k}l]'); cur=f'v{k}l'
        vlabels.append(cur)
        # audio
        clip=src.split(':',1)[-1] if src!='black' else None
        sf=os.path.join(SFX,SFXMAP.get(clip,clip)+'.wav') if clip else None
        if sf and os.path.exists(sf) and not src.startswith('bg:') and not s.get('freeze'):
            a=inp(['-ss',str(ss),'-t',str(d),'-i',sf])
            fa.append(f'[{a}:a]asetpts=PTS-STARTPTS,aresample=48000,apad,atrim=0:{d},volume={s.get("sfxvol",1.0)}[a{k}]')
        else:
            a=inp(['-f','lavfi','-t',str(d),'-i','anullsrc=r=48000:cl=stereo'])
            fa.append(f'[{a}:a]atrim=0:{d}[a{k}]')
        alabels.append(f'a{k}')
    # chain xfades
    v=vlabels[0]; a=alabels[0]; t=shots[0]['d']
    for k in range(1,len(shots)):
        kind,td=shots[k-1].get('tr',('cut',1/60))
        off=t-td
        x=f'xfade=transition={trans(kind)}:duration={td}:offset={off:.4f}'
        if kind=='heart': x+=f":expr='{HEART}'"
        if kind=='slider': x+=f":expr='{SLIDER}'"
        fv.append(f'[{v}][{vlabels[k]}]{x}[x{k}]'); v=f'x{k}'
        fa.append(f'[{a}][{alabels[k]}]acrossfade=d={td}[y{k}]'); a=f'y{k}'
        t=t+shots[k]['d']-td
    graph=';'.join(fv+fa)
    out=os.path.join(OUT,name+'.mkv')
    args+=['-filter_complex_threads','2','-filter_complex',graph,'-map',f'[{v}]','-map',f'[{a}]','-c:v','libx264','-preset','medium','-crf','12','-pix_fmt','yuv420p','-threads','2','-c:a','pcm_s16le',out]
    r=subprocess.run(args,creationflags=0x08000000,capture_output=True,text=True)
    print(name, 'ok %.2fs'%t if r.returncode==0 else 'FAILED\n'+r.stderr[-3000:], flush=True)
    return t
