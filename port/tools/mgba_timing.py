#!/usr/bin/env python3
"""Measure the cartridge's own pacing on real-hardware timing (mGBA).

Runs the European ROM in mGBA with its GDB stub and, without touching the
game, records per 300 VBlanks how many main-loop updates the game made
(gFrameCounter) and whether a movie was playing. Input is injected right
after UpdateKeyState reads REG_KEYINPUT, following a KHCOM_KEYS-style
schedule keyed on VBlank count. mGBA's timing is cycle-based, so the
debugger's pauses do not change what is measured.

    python port/tools/mgba_timing.py --mgba ../toolchain/mgba/mGBA-0.10.5-win64/mgba-sdl.exe
"""
import argparse
import os
import socket
import struct
import subprocess
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

VBLANK_INTR = 0x08000520        # VBlankIntr (Thumb), EU
KEY_READ_DONE = 0x080019BE      # UpdateKeyState, after ldrh r0, [REG_KEYINPUT]
FRAME_COUNTER = 0x03007480      # gFrameCounter: main-loop updates
VBLANK_COUNTER = 0x03006C68     # gVBlankCounter
VBLANK_OVERRIDE = 0x03007488    # gVBlankHandlerOverride (movies install one)

KEYS = {"A": 1, "B": 2, "SELECT": 4, "START": 8, "RIGHT": 16, "LEFT": 32, "UP": 64, "DOWN": 128,
        "R": 256, "L": 512}


def parse_schedule(text):
    out = []
    for item in filter(None, text.split(",")):
        frames, name = item.split(":")
        a, _, b = frames.partition("-")
        out.append((int(a), int(b or a), KEYS[name]))
    return out


class Gdb:
    def __init__(self, port):
        for _ in range(100):
            try:
                self.s = socket.create_connection(("127.0.0.1", port), timeout=30)
                break
            except OSError:
                time.sleep(0.2)
        else:
            raise SystemExit("could not connect to mGBA's GDB stub")
        self.buf = b""

    def _recv_packet(self):
        while True:
            start = self.buf.find(b"$")
            end = self.buf.find(b"#", start + 1) if start >= 0 else -1
            if start >= 0 and end >= 0 and len(self.buf) >= end + 3:
                data = self.buf[start + 1:end]
                self.buf = self.buf[end + 3:]
                self.s.sendall(b"+")
                return data.decode()
            chunk = self.s.recv(65536)
            if not chunk:
                raise SystemExit("mGBA closed the connection")
            self.buf += chunk

    def cmd(self, payload, wait=True):
        data = payload.encode()
        self.s.sendall(b"$" + data + b"#" + b"%02x" % (sum(data) & 0xFF))
        return self._recv_packet() if wait else None

    def read32(self, addr):
        return struct.unpack("<I", bytes.fromhex(self.cmd("m%x,4" % addr)))[0]

    def pc(self):
        regs = self.cmd("g")
        return struct.unpack("<I", bytes.fromhex(regs[15 * 8:16 * 8]))[0]

    def set_reg(self, n, value):
        return self.cmd("P%x=%s" % (n, struct.pack("<I", value).hex()))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--mgba", required=True)
    ap.add_argument("--rom", default=str(ROOT / "roms" / "B8CP.gba"))
    ap.add_argument("--vblanks", type=int, default=26000)
    ap.add_argument("--out", default=str(ROOT / "build" / "mgba_timing.log"))
    ap.add_argument("--keys", default=None, help="frame-range:KEY list (default: Spanish new-game route)")
    args = ap.parse_args()

    if args.keys is None:
        keys = "150-152:DOWN,165-167:DOWN,180-182:DOWN,200-204:A,1260-1264:START,1400-1404:A,1550-1554:A"
        keys += "".join(",%d-%d:A" % (f, f + 3) for f in range(1700, args.vblanks, 50))
    else:
        keys = args.keys
    schedule = parse_schedule(keys)

    env = dict(os.environ, SDL_VIDEO_DRIVER="dummy", SDL_AUDIO_DRIVER="dummy")
    proc = subprocess.Popen([args.mgba, "-g", "-C", "audioSync=0", "-C", "videoSync=0", args.rom],
                            env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    gdb = Gdb(2345)
    gdb.cmd("?")
    gdb.cmd("Z0,%x,2" % VBLANK_INTR)
    gdb.cmd("Z0,%x,2" % KEY_READ_DONE)

    vbl = 0
    last = (0, 0)
    log = open(args.out, "w")
    log.write("# window_end_vblank game_vblank_counter updates_in_window movie\n")
    try:
        while vbl < args.vblanks:
            gdb.cmd("c")
            pc = gdb.pc()
            if pc == VBLANK_INTR:
                vbl += 1
                if vbl % 300 == 0:
                    updates = gdb.read32(FRAME_COUNTER)
                    game_vbl = gdb.read32(VBLANK_COUNTER)
                    movie = gdb.read32(VBLANK_OVERRIDE) != 0
                    line = "%6d %8d %4d %s" % (vbl, game_vbl, (updates - last[0]) & 0xFFFFFFFF,
                                               "movie" if movie else "-")
                    log.write(line + "\n")
                    log.flush()
                    print(line, flush=True)
                    last = (updates, game_vbl)
            elif pc == KEY_READ_DONE:
                held = 0
                for a, b, bit in schedule:
                    if a <= vbl <= b:
                        held |= bit
                gdb.set_reg(0, (~held) & 0x3FF)
    finally:
        try:
            gdb.cmd("k", wait=False)
        except OSError:
            pass
        proc.kill()


if __name__ == "__main__":
    main()
