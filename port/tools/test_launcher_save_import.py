"""Import into a copied bank through the real launcher, never a player folder."""
import ctypes
from ctypes import wintypes
import datetime
import json
import os
from pathlib import Path
import shutil
import subprocess

ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/launcher-save-import'/datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')
OUT.mkdir(parents=True)
source=(ROOT/'build/banktest1/kh_com.sav').read_bytes()
assert len(source)==65536 and source[16:40]==b'KHCOM_BACKUP_VER00000013'
report=[]
kernel=ctypes.WinDLL('kernel32',use_last_error=True)
kernel.CreateFileW.argtypes=[wintypes.LPCWSTR,wintypes.DWORD,wintypes.DWORD,ctypes.c_void_p,wintypes.DWORD,wintypes.DWORD,wintypes.HANDLE]
kernel.CreateFileW.restype=wintypes.HANDLE
kernel.CloseHandle.argtypes=[wintypes.HANDLE]
for case in ['valid','invalid','backup_failure','locked_destination','damaged_primary_header','wrapped_srm']:
    work=OUT/case;work.mkdir()
    for name in ['ReKHCoM.exe','SDL3.dll']:
        shutil.copy2(ROOT/'build/rel-eu'/name,work/name)
    shutil.copytree(ROOT/'port/decompui/assets',work/'assets')
    (work/'kh_com.ini').write_text('config_version=11\nsave_bank=3\naspect=6\n')
    sentinel=b'original bank three'+b'\xff'*(65536-19)
    assert len(sentinel)==65536
    target=work/'kh_com_bank3.sav';target.write_bytes(sentinel)
    other=work/'kh_com.sav';other.write_bytes(b'bank one must stay intact')
    data=bytearray(source)
    if case=='invalid':data=bytearray(65536)
    if case=='damaged_primary_header':data[16+28]^=1
    if case=='wrapped_srm':data=bytearray(512)+data+bytearray(128)
    incoming=work/'emulator.srm';incoming.write_bytes(data)
    if case=='backup_failure':(work/'backups').write_bytes(b'not a directory')
    locked=None
    if case=='locked_destination':
        locked=kernel.CreateFileW(str(target),0x80000000,1,None,3,0,None)
        assert locked not in (None,ctypes.c_void_p(-1).value)
    shots=work/'shots';shots.mkdir()
    env={k:v for k,v in os.environ.items() if not k.startswith(('DECOMPUI_','KHCOM_','SDL_'))}
    env.update(DECOMPUI_SHOT=str(shots),DECOMPUI_IMPORT_SAVE=str(incoming),DECOMPUI_SAVE_REGION='0')
    try:
        subprocess.run([str(work/'ReKHCoM.exe')],cwd=work,env=env,check=True,timeout=90,creationflags=0x08000000)
    finally:
        if locked is not None:kernel.CloseHandle(locked)
    expect=bytes(data) if case=='damaged_primary_header' else source
    success=case in ['valid','damaged_primary_header','wrapped_srm']
    assert target.read_bytes()==(expect if success else sentinel),case
    assert other.read_bytes()==b'bank one must stay intact',case
    assert not list(work.glob('*.import-*')),case
    if case!='backup_failure':
        backups=list((work/'backups').glob('*.sav')) if (work/'backups').is_dir() else []
        assert len(backups)==(0 if case=='invalid' else 1),case
        for backup in backups:assert backup.read_bytes()==sentinel,case
    report.append({'case':case,'passed':True,'imported':success,'bank_one_preserved':True})
    print(case,'PASS',flush=True)
(OUT/'report.json').write_text(json.dumps(report,indent=2))
print(OUT)
