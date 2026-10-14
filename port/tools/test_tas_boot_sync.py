#!/usr/bin/env python3
"""Locate the first boot/input divergence against an isolated original ROM.

Reads original mode descriptors from the local ROM's own names, observes
IWRAM pointers in BizHawk, and replays the unchanged VBM. No progress writes,
SRAM import, or patching of the cartridge. All output remains in build/.
"""
import argparse
import datetime
import json
import os
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[2]
NAMES = ('mode_copyright1', 'mode_copyright2', 'mode_title', 'Mode_MenuNew',
         'Mode_MenuLoad', 'mode_movie', 'Event', 'mode_worldselect',
         'mode_battle', 'Mode_MapFld', 'Mode_MapFix')

def descriptors(rom):
    modes = {}
    for name in NAMES:
        needle = name.encode('ascii') + b'\0'
        start = rom.find(needle)
        if start < 0:
            continue
        pointer = struct.pack('<I', 0x08000000 + start)
        pos = 0
        while True:
            pos = rom.find(pointer, pos)
            if pos < 0:
                break
            # Mode is name + three Thumb callback pointers in ROM.
            if pos % 4 == 0 and pos + 16 <= len(rom):
                callbacks = struct.unpack_from('<3I', rom, pos + 4)
                if all(p == 0 or 0x08000000 <= p < 0x0a000000 for p in callbacks):
                    modes[0x08000000 + pos] = name
            pos += 1
    return modes

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--frames', type=int, default=1600)
    ap.add_argument('--offset', type=int, default=0)
    ap.add_argument('--every', type=int, default=50)
    args = ap.parse_args()
    work = ROOT / 'build/codex-tas-boot-sync' / datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')
    work.mkdir(parents=True)
    bizhawk = ROOT.parent / 'toolchain/bizhawk'
    movie = ROOT.parent / 'Kingdom Hearts - Chain of Memories (U).vbm'
    rom = ROOT / 'roms/B8CE.gba'
    modes = descriptors(rom.read_bytes())
    (work / 'mode_descriptors.json').write_text(json.dumps({hex(k): v for k, v in modes.items()}, indent=2))
    # Keep emulator paths and configuration private, including any SRAM writes.
    config = json.loads((bizhawk / 'config.ini').read_text(encoding='utf-8-sig'))
    paths = config.get('PathEntries', {}).get('Paths', [])
    for item in paths:
        if item.get('System') == 'GBA':
            item['Path'] = str(work / ('gba' if item['Type'] == 'Base' else item['Type'].replace(' ', '_')))
    cfg = work / 'bizhawk.config.json'
    cfg.write_text(json.dumps(config, indent=2), encoding='utf-8')
    table = ',\n'.join(f'[{k}]= {json.dumps(v)}' for k, v in modes.items())
    lua = work / 'boot_sync.lua'
    lua.write_text('''local modes = { TABLE }
local file = assert(io.open(os.getenv("KHCOM_SYNC_MOVIE"), "rb"))
local data = file:read("*a"); file:close()
local start = string.unpack("<I4", data, 0x3c+1)
local outdir = os.getenv("KHCOM_SYNC_OUT")
local log = assert(io.open(outdir .. "/modes.csv", "w"))
log:write("frame,address,mode,keys\\n"); log:flush()
local candidates, previous = {}, {}
local total = tonumber(os.getenv("KHCOM_SYNC_FRAMES"))
local offset = tonumber(os.getenv("KHCOM_SYNC_OFFSET"))
local every = tonumber(os.getenv("KHCOM_SYNC_EVERY"))
local names = {"A","B","Select","Start","Right","Left","Up","Down","R","L"}
pcall(client.speedmode, 6400)
while emu.framecount() < total do
  local frame = emu.framecount()
  local index = frame + offset
  local keys = 0
  if index >= 0 and start+index*2+2 <= #data then
    keys = string.unpack("<I2", data, start+index*2+1)
  end
  local pad = {}
  for bit=0,9 do if keys & (1 << bit) ~= 0 then pad[names[bit+1]]=true end end
  joypad.set(pad)
  if frame % 64 == 0 then
    for address=0x03000000,0x03007ffc,4 do
      local value=memory.read_u32_le(address,"System Bus")
      if modes[value] then candidates[address]=true end
    end
  end
  for address,_ in pairs(candidates) do
    local value=memory.read_u32_le(address,"System Bus")
    if value ~= previous[address] then
      log:write(string.format("%d,0x%08x,%s,0x%04x\\n",frame,address,modes[value] or string.format("0x%08x",value),keys))
      previous[address]=value; log:flush()
    end
  end
  emu.frameadvance()
  local frame=emu.framecount()
  if frame % every == 0 then client.screenshot(string.format("%s/bh_%06d.png",outdir,frame)) end
end
log:close(); client.exit()
'''.replace('TABLE', table))
    env = dict(os.environ, KHCOM_SYNC_MOVIE=str(movie), KHCOM_SYNC_OUT=str(work),
               KHCOM_SYNC_FRAMES=str(args.frames), KHCOM_SYNC_OFFSET=str(args.offset),
               KHCOM_SYNC_EVERY=str(args.every))
    startup = subprocess.STARTUPINFO()
    startup.dwFlags = subprocess.STARTF_USESHOWWINDOW
    startup.wShowWindow = 0
    result = subprocess.run([str(bizhawk/'EmuHawk.exe'), '--config='+str(cfg),
                             '--lua='+str(lua), str(rom)], cwd=bizhawk, env=env,
                            startupinfo=startup, timeout=180)
    report = dict(exit=result.returncode, frames=args.frames, offset=args.offset,
                  screenshots=len(list(work.glob('bh_*.png'))),
                  original_mode_descriptors={hex(k): v for k,v in modes.items()},
                  scope='original cartridge input/boot timing observation, not a campaign completion')
    (work/'report.json').write_text(json.dumps(report, indent=2))
    print(work, flush=True)
    print(json.dumps(report, indent=2))
    return int(result.returncode != 0 or not (work/'modes.csv').exists())

if __name__ == '__main__':
    raise SystemExit(main())
