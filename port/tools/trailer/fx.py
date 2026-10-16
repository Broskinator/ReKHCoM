import os, shutil, subprocess
from pathlib import Path
from PIL import Image
from paths import ROOT, SAVE_SOURCE
def fx(name, modes, shots, keys=None, save=True, scale=4, lang="es", extra=None, build="rel-eu", narrow=1, view=4, hd=0, rom=None, ini=""):
    d=ROOT/'build'/('trailer182_'+name); d.mkdir(parents=True,exist_ok=True)
    shutil.copy2(ROOT/'build'/build/'khcom.exe', d); shutil.copy2(ROOT/'build'/'rel-eu'/'SDL3.dll', d)
    if os.environ.get('COPYSTATE'): shutil.copy2(os.environ['COPYSTATE'], d/'slot1.khst')
    if save and SAVE_SOURCE and SAVE_SOURCE.is_file(): shutil.copy2(SAVE_SOURCE,d/'kh_com.sav')
    shutil.copytree(ROOT/'port/assets/fonts', d/'assets/fonts',dirs_exist_ok=True)
    (d/'kh_com.ini').write_text('config_version=11\nzoom=8\nview_mode=%d\nnarrow_fit=%d\nwindow_scale=%d\nhd_text=%d\n' % (view, narrow, scale, hd) + ini)
    if keys is None: keys=[f"{f}-{f+3}:A" for f in range(200,1400,50)]
    e={k:v for k,v in os.environ.items() if not k.startswith(('KHCOM_','SDL_'))}
    e.update(SDL_VIDEO_DRIVER='dummy',SDL_AUDIO_DRIVER='dummy',SDL_JOYSTICK_HIDAPI='0',SDL_JOYSTICK_RAWINPUT='0',SDL_XINPUT_ENABLED='0',SDL_JOYSTICK_DIRECTINPUT='0',SDL_JOYSTICK_WGI='0',SDL_JOYSTICK_GAMEINPUT='0',SDL_JOYSTICK_GAMEINPUT_RAW='0',KHCOM_FAST='1',KHCOM_MODE=','.join(modes),KHCOM_KEYS=','.join(keys),KHCOM_OUTPUT_SHOT=','.join(map(str,shots)),KHCOM_QUIT=str(max(shots)+1))
    if extra: e.update(extra)
    args=[str(d/'khcom.exe'),'--rom',str(ROOT/'roms'/((rom or 'B8CP')+'.gba'))]+(['--language',lang] if lang else [])
    subprocess.run(args,cwd=d,env=e,timeout=180,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,creationflags=0x08000000)
    return sorted(d.glob('out_*.bmp'))
def sheet(bs, out, w=640, h=360):
    o=Image.new('RGB',(w*2,h*((len(bs)+1)//2)))
    for i,b in enumerate(bs): o.paste(Image.open(b).resize((w,h)),((i%2)*w,(i//2)*h))
    o.save(out)
