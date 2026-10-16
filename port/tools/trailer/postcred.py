# Heartless post-credits: a dark field, a crowd of Shadow eyes opening in the
# dark (drawn: two yellow ellipses each), the Heartless emblem silhouette,
# and the text. Exported as frames composed later by ffmpeg.
from PIL import Image, ImageDraw, ImageFont, ImageFilter
import random, math
W,H=1920,1080
PAL="C:/Windows/Fonts/palab.ttf"
random.seed(7)
eyes=[]
while len(eyes)<40:
    x,y=random.randint(80,W-80),random.randint(80,H-80)
    if 520<x<1400 and 100<y<880: continue
    if any(abs(x-a)<90 and abs(y-b)<70 for a,b,_,_ in eyes): continue
    eyes.append((x,y,random.uniform(0.7,1.5),random.uniform(0,1.8)))
def eye_layer(t):
    im=Image.new('RGBA',(W,H),(0,0,0,0)); d=ImageDraw.Draw(im)
    for x,y,s,delay in eyes:
        o=min(1,max(0,(t-delay)/0.35))
        if o<=0: continue
        blink=1.0
        ph=(t*0.9+delay*3)%4.0
        if ph<0.12: blink=max(0.1,abs(ph-0.06)/0.06)
        w,h=int(11*s),int(15*s*o*blink)+1
        for dx in (-int(16*s),int(16*s)):
            d.ellipse([x+dx-w,y-h,x+dx+w,y+h],fill=(255,226,60,int(255*o)))
    glow=im.filter(ImageFilter.GaussianBlur(10))
    return Image.alpha_composite(glow,im)
def emblem(size):
    # heart split by an X, as a silhouette (not the official mark)
    im=Image.new('L',(size,size),0); d=ImageDraw.Draw(im)
    pts=[]
    for k in range(200):
        a=k/200*2*math.pi
        x=16*math.sin(a)**3; y=13*math.cos(a)-5*math.cos(2*a)-2*math.cos(3*a)-math.cos(4*a)
        pts.append((size/2+x*size/38,size/2.2-y*size/38))
    d.polygon(pts,fill=255)
    d.line([(size*0.28,size*0.32),(size*0.72,size*0.78)],fill=0,width=size//18)
    d.line([(size*0.72,size*0.32),(size*0.28,size*0.78)],fill=0,width=size//18)
    return im
EM=emblem(420)
def frame(t):
    bg=Image.new('RGBA',(W,H),(4,2,10,255))
    # purple darkness swirl
    fog=Image.new('L',(W,H),0); fd=ImageDraw.Draw(fog)
    for i in range(9):
        r=300+i*90+40*math.sin(t*0.8+i)
        fd.ellipse([W/2-r,H/2-r*0.6,W/2+r,H/2+r*0.6],outline=40-i*4,width=60)
    fog=fog.filter(ImageFilter.GaussianBlur(40))
    bg.alpha_composite(Image.merge('RGBA',(*Image.new('RGB',(W,H),(90,30,140)).split(),fog)))
    bg.alpha_composite(eye_layer(t))
    # emblem fades in at 2.2s, text at 3.0s
    a=min(1,max(0,(t-2.2)/0.8))
    if a>0:
        col=Image.new('RGBA',EM.size,(150,20,40,255)); m=EM.point(lambda v:int(v*a*0.92))
        col.putalpha(m); glow=col.filter(ImageFilter.GaussianBlur(18))
        x0,y0=(W-EM.size[0])//2,140
        bg.alpha_composite(glow,(x0,y0)); bg.alpha_composite(col,(x0,y0))
    b=min(1,max(0,(t-3.0)/0.7))
    if b>0:
        d=ImageDraw.Draw(bg)
        for txt,size,y,c in (("ANDROID RELEASE",104,620,(255,255,255)),("COMING SOON",58,760,(255,226,60))):
            f=ImageFont.truetype(PAL,size)
            sh=Image.new('RGBA',(W,H),(0,0,0,0)); ImageDraw.Draw(sh).text((W/2,y),txt,font=f,fill=(120,20,200,int(255*b)),anchor='ma')
            bg.alpha_composite(sh.filter(ImageFilter.GaussianBlur(12)))
            ImageDraw.Draw(bg).text((W/2,y),txt,font=f,fill=c+(int(255*b),),anchor='ma')
    return bg.convert('RGB')
from paths import T,FF
import subprocess
frames=T/'tools/pc_frames';frames.mkdir(parents=True,exist_ok=True)
N=int(6.0*30)
for i in range(N):frame(i/30).save(frames/f'f{i:04d}.png')
subprocess.run([FF,'-v','error','-y','-framerate','30','-i',str(frames/'f%04d.png'),'-vf','fps=60','-c:v','prores_ks','-profile:v','3','-threads','2',str(T/'clips/16_postcreditos_android.mov')],check=True,creationflags=0x08000000)
