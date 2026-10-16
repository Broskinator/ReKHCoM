"""240x160 GBA image grows, then a slider reveals the native wide view."""
import subprocess
from pathlib import Path
from PIL import Image
exec(open('edit.py').read().split('# shot:')[0])
SC=Path(T)/'tools'
# Crop exactly 240x160 native pixels from the x4 recording. Using one
# camera for both sides keeps Sora and every moving tile aligned in the wipe.
x0,y0,x1,y1=480,220,1440,860
PUSH,FULL,SLIDE,DURATION,TOTAL=2.6,3.4,3.8,.8,9.2
u=f'min(1,max(0,(t-{PUSH})/{FULL-PUSH}))';ease=f'(3*pow({u},2)-2*pow({u},3))'
k=f'(1+5.75*{ease})'
# Match the two cameras during the wipe, then ease from full-height GBA
# magnification to the release's x4 wide view. No discontinuous zoom step.
u2='min(1,max(0,(t+3.8-4.6)/1.0))';ease2=f'(3*pow({u2},2)-2*pow({u2},3))';z=f'(1+0.6875*(1-{ease2}))'
g=(f'[0:v]crop={x1-x0}:{y1-y0}:{x0}:{y0},setpts=PTS-STARTPTS,fps=60,tpad=stop_mode=clone:stop_duration=4,scale=w=\'2*trunc(120*{k})\':h=\'2*trunc(80*{k})\':eval=frame:flags=neighbor[gba];'
   f'color=c=black:s=1920x1080:r=60:d={TOTAL}[bg];[bg][gba]overlay=x=(W-w)/2:y=(H-h)/2:eval=frame:shortest=1,format=yuv420p,trim=duration={TOTAL}[old];'
   f'[1:v]trim=start=3.8,setpts=PTS-STARTPTS,fps=60,tpad=stop_mode=clone:stop_duration=4,scale=w=\'2*trunc(960*{z})\':h=\'2*trunc(540*{z})\':eval=frame:flags=neighbor,crop=1920:1080,format=yuv420p,trim=duration={TOTAL}[wide];'
   f'[old][wide]xfade=transition=custom:duration={DURATION}:offset={SLIDE}:expr=\'{SLIDER}\',format=yuv422p10le[v]')
r=subprocess.run([FF,'-loglevel','error','-y','-filter_complex_threads','2','-i',str(Path(CL)/'00f_castillo_paseo.mov'),'-i',str(Path(CL)/'00f_castillo_paseo.mov'),'-filter_complex',g,'-map','[v]','-t',str(TOTAL),'-c:v','prores_ks','-profile:v','3','-threads','2',str(Path(CL)/'00e_intro_castillo.mov')],capture_output=True,text=True,creationflags=0x08000000)
print(r.returncode,r.stderr[-2000:]);assert r.returncode==0
