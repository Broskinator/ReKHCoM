"""Capture the release launcher's hidden pages; never alter an installed copy."""
import os,shutil,subprocess
from paths import ROOT,T
stage=ROOT/'build/trailer182_launcher';stage.mkdir(exist_ok=True)
for name in ['ReKHCoM.exe','SDL3.dll']:shutil.copy2(ROOT/'build/rel-eu'/name,stage/name)
shutil.copytree(ROOT/'port/decompui/assets',stage/'assets',dirs_exist_ok=True)
shutil.copytree(ROOT/'port/assets/fonts',stage/'assets/fonts',dirs_exist_ok=True)
(stage/'decompui.ini').write_text('ui_language=2\ngame_language=en\n')
out=T/'launcher';out.mkdir(exist_ok=True)
e={k:v for k,v in os.environ.items() if not k.startswith(('KHCOM_','SDL_','DECOMPUI_'))}
e.update(DECOMPUI_SHOT=str(out),DECOMPUI_INSTALL=str(ROOT/'roms/B8CP.gba'))
for name in ['SDL_XINPUT_ENABLED','SDL_JOYSTICK_HIDAPI','SDL_JOYSTICK_RAWINPUT','SDL_JOYSTICK_DIRECTINPUT','SDL_JOYSTICK_WGI','SDL_JOYSTICK_GAMEINPUT','SDL_JOYSTICK_GAMEINPUT_RAW']:e[name]='0'
subprocess.run([str(stage/'ReKHCoM.exe')],cwd=stage,env=e,timeout=60,check=True,creationflags=0x08000000)
