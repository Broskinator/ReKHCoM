#!/usr/bin/env python3
"""Begin natural Riku USA from exact original SRAM and ordinary BK2 buttons.

Does not import emulator CPU/RAM state or assign game flags, room, stats or
inventory. A successful startup is not a completed campaign. The supplied
published movie begins from VBA-Next state, so RNG/synchronization may differ.
"""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import zipfile

from test_gameplay_riku_plan import inspect_save
from test_gameplay_observe_state import State

ROOT = Path(__file__).resolve().parents[2]
BUTTON_BITS = (64,128,32,16,8,4,2,1,512,256)


def bk2_buttons(movie):
    with zipfile.ZipFile(movie) as archive:
        header = archive.read('Header.txt').decode().replace('\r','')
        if 'Platform GBA' not in header or '10729BD884F8FDCA7A310B6D606C52E46657AA48' not in header:
            raise ValueError('Expected original USA Riku BK2')
        text = archive.read('Input Log.txt').decode().replace('\r','')
    expected = 'LogKey:#Tilt X|Tilt Y|Tilt Z|Light Sensor|Up|Down|Left|Right|Start|Select|B|A|L|R|Power|'
    if not text.startswith(expected):
        raise ValueError('Unexpected input ordering')
    masks = []
    for row in text.splitlines():
        if not row.startswith('|'):
            continue
        buttons = row.split(',')[-1].strip('| ')
        if len(buttons) != 11 or buttons[10] != '.':
            raise ValueError('Unsupported controller row or power reset')
        masks.append(sum(bit for bit, character in zip(BUTTON_BITS, buttons) if character != '.'))
    return masks, header


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--build', type=Path, default=ROOT/'build/codex-riku-us')
    ap.add_argument('--sram', type=Path, default=ROOT/'build/codex-riku-plan/starting-sram-us.sav')
    ap.add_argument('--movie', type=Path, default=ROOT/'build/codex-riku-plan/riku-movie.bk2')
    ap.add_argument('--frames', type=int, default=6000)
    ap.add_argument('--offset', type=int, default=0)
    ap.add_argument('--resume', type=Path, help='Continue an unmodified checkpoint from this exact native build')
    ap.add_argument('--keys', default='', help='Ordinary local-frame KHCOM_KEYS schedule')
    ap.add_argument('--no-movie', action='store_true', help='Use only the supplied normal buttons')
    ap.add_argument('--link-runtime', action='store_true', help='Hardlink immutable exe/DLL to save disk space; never rebuild the supplied binary')
    args = ap.parse_args()
    original_sram = args.sram.read_bytes()
    inventory = inspect_save(args.sram)
    if not inventory['can_start_new_riku'] or 'us' not in inventory['compatible_signatures'] or len(original_sram)!=65536:
        ap.error('Need a valid unchanged USA SRAM with natural Sora clear unlock')
    masks, header = bk2_buttons(args.movie)
    frames = min(args.frames, len(masks))
    work = ROOT/'build/codex-riku-probe'/datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')
    work.mkdir(parents=True)
    def runtime_file(source,destination):
        if args.link_runtime:
            os.link(source,destination)
        else:
            shutil.copy2(source,destination)
    runtime_file(args.build/'khcom.exe',work/'khcom.exe')
    runtime_file(ROOT.parent/'toolchain/SDL3-3.4.18/i686-w64-mingw32/bin/SDL3.dll',work/'SDL3.dll')
    (work/'kh_com_us.sav').write_bytes(original_sram)
    vbm = bytearray(256)
    vbm[:4]=b'VBM\x1a'
    struct.pack_into('<I',vbm,0x3c,256)
    if not args.no_movie:
        (work/'buttons.vbm').write_bytes(vbm+struct.pack('<'+'H'*len(masks),*masks))
    (work/'kh_com.ini').write_text('config_version=2\nview_mode=0\nhd_text=0\nwindow_scale=1\n')
    checks = list(range(1000,frames,1000))+[frames]
    env = {k:v for k,v in os.environ.items() if not k.startswith(('KHCOM_','SDL_'))}
    env.update(SDL_VIDEO_DRIVER='dummy',SDL_AUDIO_DRIVER='dummy',KHCOM_FAST='1',KHCOM_TRACE_MODES='1',
               KHCOM_VBM=str(work/'buttons.vbm'),KHCOM_VBM_OFFSET=str(args.offset),
               KHCOM_MODE=','.join(f'{f}:progress'for f in checks),
               KHCOM_OUTPUT_SHOT=','.join(map(str,checks)),KHCOM_QUIT=str(frames+2),
               KHCOM_SAVESTATE=','.join(f'{f}:checkpoint_{f}.khst'for f in checks))
    if args.resume:
        State(work/'khcom.exe',args.resume)  # Verify code/layout before launching.
        # PortConfigPath prefixes even an absolute LOADSTATE path; use a local
        # copied filename so the engine actually resumes the validated state.
        shutil.copy2(args.resume,work/'resume.khst')
        env['KHCOM_LOADSTATE']='resume.khst'
    if args.no_movie:
        env.pop('KHCOM_VBM')
        env.pop('KHCOM_VBM_OFFSET')
    if args.keys:
        env['KHCOM_KEYS']=args.keys
    result = subprocess.run([str(work/'khcom.exe'),'--rom',str(ROOT/'roms/B8CE.gba')],cwd=work,env=env,
                            timeout=300,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    log = (work/'kh_com.log').read_text(errors='replace')
    resume_loaded=not args.resume or any('state: load 'in line and line.endswith(' -> 1')for line in log.splitlines())
    snapshots = []
    for frame in checks:
        path = work/f'checkpoint_{frame}.khst'
        if path.exists():
            snapshot = State(work/'khcom.exe',path).snapshot()
            snapshot['requested_frame']=frame
            snapshots.append(snapshot)
    report = dict(exit=result.returncode,crash='CRASH'in log,frames=frames,offset=args.offset,
                  source_movie_sha256=hashlib.sha256(args.movie.read_bytes()).hexdigest(),
                  original_sram_sha256=hashlib.sha256(original_sram).hexdigest(),
                  original_sram_unchanged=args.sram.read_bytes()==original_sram,
                  resumed_from=str(args.resume.resolve()) if args.resume else None,
                  resume_loaded=resume_loaded,
                  buttons_only=args.no_movie,keys=args.keys,
                  snapshots=snapshots,natural_riku_entry=any(s['actor']=='riku'for s in snapshots),
                  full_campaign_verified=False,scope='SRAM copy + buttons only; read-only checkpoint observation')
    (work/'report.json').write_text(json.dumps(report,indent=2))
    (work/'source-header.txt').write_text(header)
    print(work)
    print(json.dumps(report,indent=2))
    return 0 if result.returncode==0 and resume_loaded and not report['crash'] and report['natural_riku_entry'] else 1


if __name__=='__main__':
    raise SystemExit(main())
