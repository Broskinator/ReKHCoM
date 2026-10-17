from paths import T
CARDS=T/'tools/cards'
CARDS.mkdir(parents=True,exist_ok=True)
from PIL import Image, ImageDraw, ImageFont, ImageFilter
F=r"C:/Windows/Fonts/pala.ttf"
W,H=1920,1080
def card(lines, out, glow=(150,190,255), orn=True):
    # lines: list of (text, size, color, track)
    img=Image.new('RGBA',(W,H),(0,0,0,0)); txt=Image.new('L',(W,H),0)
    d=ImageDraw.Draw(img); dm=ImageDraw.Draw(txt)
    blocks=[]
    for t,s,c,tr in lines:
        f=ImageFont.truetype(F,s); ws=[f.getbbox(ch)[2] for ch in t]
        w=sum(f.getlength(ch) for ch in t)+tr*(len(t)-1); blocks.append((t,f,c,tr,w,s))
    tot=sum(b[5]*1.25 for b in blocks)+(30 if orn else 0); y=(H-tot)/2
    for t,f,c,tr,w,s in blocks:
        x=(W-w)/2
        for ch in t:
            d.text((x,y),ch,font=f,fill=c); dm.text((x,y),ch,font=f,fill=255); x+=f.getlength(ch)+tr
        y+=s*1.25
    if orn:
        cy=int(y+10); cx=W//2
        for dx in range(-300,301):
            a=int(255*max(0,1-abs(dx)/300)**1.5)
            if abs(dx)>14: d.point((cx+dx,cy),fill=GD+(a,)); dm.point((cx+dx,cy),fill=a//3)
        d.polygon([(cx-9,cy),(cx,cy-9),(cx+9,cy),(cx,cy+9)],fill=GD); dm.polygon([(cx-9,cy),(cx,cy-9),(cx+9,cy),(cx,cy+9)],fill=200)
    g=txt.filter(ImageFilter.GaussianBlur(18)); g2=txt.filter(ImageFilter.GaussianBlur(5))
    base=Image.new('RGBA',(W,H),glow+(0,)); base.putalpha(g.point(lambda v:min(255,v*2)))
    b2=Image.new('RGBA',(W,H),(255,255,255,0)); b2.putalpha(g2.point(lambda v:min(255,int(v*1.2))))
    o=Image.alpha_composite(base,b2); o=Image.alpha_composite(o,img); o.save(out)
WH=(255,255,255); GD=(235,215,150)
C={
 'c01':[("2004",64,GD,24),("GAME BOY ADVANCE",88,WH,14)],
 'c02':[("2026",64,GD,24),("NATIVE ON PC",110,WH,14)],
 'c03':[("ReKHCoM",200,WH,10),("KINGDOM HEARTS  CHAIN OF MEMORIES",44,GD,10)],
 'c04':[("THE WORLD, WIDER",110,WH,12)],
 'c05':[("EVERY ARENA IN 16:9",110,WH,12)],
 'c06':[("FAITHFUL TO THE PIXEL",110,WH,12)],
 'c07':[("NO STRETCHED PIXELS",110,WH,12)],
 'c08':[("UP TO",64,GD,24),("32:9",200,WH,20)],
 'c09':[("HD TEXT",96,WH,12),("6 LANGUAGES",96,WH,12),("RE:COM MODE",96,WH,12)],
 'c12':[("EVERY SETTING, IN GAME",100,WH,12)],
 'c10':[("ReKHCoM",180,WH,10),("AVAILABLE NOW  -  FREE",60,GD,14),("github.com/MatyMeatBoy/ReKHCoM",48,WH,4)],
 'c11':[("Fan project, not affiliated with Square Enix, Disney or Nintendo.",42,(205,205,205),2),("Your own cartridge is required; no ROM is included.",42,(205,205,205),2)],
}
import sys
for k,v in C.items(): card(v,str(CARDS/(k+'.png')),orn=(k!='c11'))
prev=Image.new('RGB',(W,H*2//4*1),(0,0,0))

shade=Image.new('RGBA',(W,H),(0,0,0,0));d=ImageDraw.Draw(shade)
for y in range(H):
 a=int(235*min(1,max(0,(y-560)/360))**1.3)
 d.line([(0,y),(W,y)],fill=(0,0,0,a))
shade.save(CARDS/'shade.png')
