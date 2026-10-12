#!/usr/bin/env python3
"""Compare native movie codecs with the original ARM routines using a local ROM.

Requires 64-bit Python with unicorn; all generated files stay under build/.
python port/tools/test_movie_codec.py --toolchain ../toolchain/llvm-mingw-...
"""
import argparse
import ctypes as ct
import hashlib
import re
import struct
import subprocess
from pathlib import Path
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM
from unicorn.arm_const import UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3, UC_ARM_REG_SP, UC_ARM_REG_LR

ROOT = Path(__file__).resolve().parents[2]
MOVIES = {"opening": 0x12FB24, "event026": 0x4B466C, "event057": 0x4C7D98,
          "ending": 0x5303EC, "riku-ending": 0x6CF14C}


def chunks(rom, pos, count):
    types_len = struct.unpack_from("<I", rom, pos)[0]
    types = rom[pos + 4:pos + 4 + count]
    pos += 4 + types_len
    sizes_len, data_len, maximum = struct.unpack_from("<IIH", rom, pos)
    sizes = struct.unpack_from(f"<{count}H", rom, pos + 10)
    pos += 8 + sizes_len
    payload = []
    cursor = pos
    for size in sizes:
        payload.append(rom[cursor:cursor + size])
        cursor += size
    assert sum(sizes) <= data_len
    return types, payload, pos + data_len


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--toolchain", type=Path, required=True)
    parser.add_argument("--rom", type=Path, default=ROOT / "roms/B8CP.gba")
    parser.add_argument("--oracle-every", type=int, default=37, help="compare every Nth video frame, plus first 40 and all key frames")
    args = parser.parse_args()
    if args.oracle_every < 1: parser.error("oracle-every must be positive")
    build = ROOT / "build/movie-tests"
    build.mkdir(parents=True, exist_ok=True)
    bins = args.toolchain.resolve() / "bin"
    def run(name, *params):
        return subprocess.check_output([str(bins / (name + ".exe")), *map(str, params)], cwd=ROOT, text=True)
    run("clang", "-shared", "-O2", "-DPORT_MOVIE_TEST", "-Iport/include", "port/src/port_movie.c", "-o", build / "native.dll")
    run("clang", "--target=arm-none-eabi", "-c", "asm/movie_codec.s", "-o", build / "arm.o")
    run("ld.lld", "-Ttext=0x100000", "--entry=MovieVideoCodecKeyFrame", build / "arm.o", "-o", build / "arm.elf")
    run("llvm-objcopy", "-O", "binary", "--only-section=.text", build / "arm.elf", build / "arm.bin")
    symbols = {name: int(addr, 16) for addr, name in re.findall(r"^([0-9a-f]+) \w (\w+)$", run("llvm-nm", "-n", build / "arm.o"), re.M)}
    original = (build / "arm.bin").read_bytes()
    dll = ct.CDLL(str(build / "native.dll"))
    key = dll.PortMovieDecodeKey
    key.argtypes = [ct.c_void_p, ct.c_uint32, ct.c_uint32, ct.c_void_p, ct.c_size_t]
    delta = dll.PortMovieDecodeDelta
    delta.argtypes = [ct.c_void_p, ct.c_void_p, ct.c_uint32, ct.c_uint32, ct.c_void_p, ct.c_size_t]
    post = dll.PortMoviePostProcess
    post.argtypes = [ct.c_void_p, ct.c_uint32, ct.c_uint32]
    audio = dll.PortMovieDecodeAudio
    audio.argtypes = [ct.c_void_p, ct.c_size_t, ct.c_void_p, ct.c_uint32, ct.c_uint]
    rom = args.rom.read_bytes()
    assert hashlib.sha1(rom).hexdigest() == "8db73586cdb11b3795907edebf43228dbcd3e6b2", "Expected European B8CP ROM"
    code, output, source, previous, stack = 0x100000, 0x200000, 0x300000, 0x400000, 0x500000
    uc = Uc(UC_ARCH_ARM, UC_MODE_ARM)
    for addr in (code, output, source, previous, stack): uc.mem_map(addr, 0x20000)
    def invoke(name, *regs):
        for reg, value in zip((UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3), regs): uc.reg_write(reg, value)
        uc.reg_write(UC_ARM_REG_SP, stack + 0x1F000)
        uc.reg_write(UC_ARM_REG_LR, code + 0x1FF00)
        uc.emu_start(code + symbols[name], code + 0x1FF00, count=4000000)
        assert uc.reg_read(UC_ARM_REG_SP) == stack + 0x1F000, "ARM routine did not return"
    def immediate(value):
        for rotation in range(16):
            shift = 2 * rotation
            rotated = ((value << shift) | (value >> ((32 - shift) & 31))) & 0xFFFFFFFF
            if rotated <= 255: return rotation << 8 | rotated
        raise ValueError(value)
    steps = [7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,
             73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,408,
             449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,
             2066,2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,
             7845,8630,9493,10442,11487,12635,13899,15289,16818,18500,20350,22358,
             24633,27086,29794,32767]
    totals = [0, 0, 0]
    for name, offset in MOVIES.items():
        w, h, fps, frames, blocks, channels, rate, codec = struct.unpack_from("<IIfIIIII", rom, offset)
        types, video, pos = chunks(rom, offset + 32, frames)
        _, sound, _ = chunks(rom, pos, blocks) if channels else (None, [], None)
        arm = bytearray(original)
        arm[:96] = bytes((i + 1) // 3 if i < 94 else 0 for i in range(96))
        for cursor in range(symbols["MovieDeltaCodecStart"], symbols["MovieAudioCodecStart"], 4):
            ins = struct.unpack_from("<I", arm, cursor)[0]
            if ins & 0xFF000000 != 0xEF000000: continue
            kind, rd, rn, row, col = (ins >> 20) & 15, (ins >> 16) & 15, (ins >> 12) & 15, (ins >> 4) & 15, ins & 15
            value = w * 2 * row + 4 * col
            if kind in (0, 1): patched = (0xE5800000 if kind == 0 else 0xE5900000) | rn << 16 | rd << 12 | value
            elif kind in (3, 4): patched = (0xE2800000 if kind == 3 else 0xE2400000) | rn << 16 | rd << 12 | immediate(value)
            elif kind == 2: patched = 0xE3A00000 | rd << 12 | immediate((w if col == 0 else h) // 8)
            else: raise ValueError(kind)
            struct.pack_into("<I", arm, cursor, patched)
        for i in range(256): struct.pack_into("<i", arm, symbols["MovieDeltaCodecOffsets"] + i * 4, ((i // 16 - 8) * w + i % 16 - 8) * 2)
        for i, step in enumerate(steps):
            for j in range(4): struct.pack_into("<H", arm, symbols["MovieAudioCodecAdpcmSteps"] + (i * 4 + j) * 2, (((j & 1) * step + step // 2) * (-1 if j & 2 else 1)) & 65535)
        uc.mem_write(code, bytes(arm))
        prev = bytes(w * h * 2)
        for index, (kind, payload) in enumerate(zip(types, video)):
            dst = ct.create_string_buffer(w * h * 2)
            src = ct.create_string_buffer(payload)
            before = ct.create_string_buffer(prev)
            ok = key(dst, w, h, src, len(payload)) if kind == 0 else delta(before, dst, w, h, src, len(payload))
            assert ok, f"{name} frame {index}: native decoder rejected input"
            if kind == 2: post(dst, w, h)
            if kind == 0 or index < 40 or index % args.oracle_every == 0 or index == frames - 1:
                uc.mem_write(source, payload + bytes(64))
                uc.mem_write(previous, prev)
                uc.mem_write(output, bytes(w * h * 2))
                if kind == 0: invoke("MovieVideoCodecKeyFrame", output, w, h, source)
                else:
                    invoke("MovieDeltaCodecDecode", previous, output, source)
                    if kind == 2: invoke("MovieVideoCodecPostProcess", output, w, h)
                expected = bytes(uc.mem_read(output, w * h * 2))
                if dst.raw != expected:
                    (build / "actual.raw").write_bytes(dst.raw)
                    (build / "expected.raw").write_bytes(expected)
                    print(f"Mismatch kind={kind}, delta header={payload[:4].hex()}", flush=True)
                assert dst.raw == expected, f"{name} frame {index}: ARM/native mismatch at byte {next(i for i,(a,b) in enumerate(zip(dst.raw,expected)) if a != b)}"
                totals[1] += 1
            prev = dst.raw
            totals[0] += 1
        for index, payload in enumerate(sound):
            decoded = struct.unpack_from("<I", payload)[0] // channels
            size = (len(payload) - 4) // channels
            for channel in range(channels):
                data = payload[4 + size * channel:4 + size * (channel + 1)]
                dst = ct.create_string_buffer(decoded // 2)
                assert audio(ct.create_string_buffer(data), len(data), dst, decoded, codec)
                uc.mem_write(source, data + bytes(64)); uc.mem_write(output, bytes(decoded // 2))
                invoke(("MovieAudioCodecPcm16", "MovieAudioCodecNull", "MovieAudioCodecAdpcm")[codec], source, output, decoded)
                assert dst.raw == bytes(uc.mem_read(output, decoded // 2)), f"{name} audio block {index}: ARM/native mismatch"
                totals[2] += 1
        print(f"{name}: {frames} video frames decoded; {blocks} audio blocks matched ARM", flush=True)
    # Reject truncated delta/color and audio input without reading past buffers.
    out = ct.create_string_buffer(128)
    assert not delta(out, out, 8, 8, ct.create_string_buffer(b'\x04\0\0\0'), 4)
    assert not audio(ct.create_string_buffer(b'\0'), 1, out, 32, 2)
    print(f"PASS: {totals[0]} video frames, {totals[1]} ARM video comparisons, {totals[2]} ARM audio comparisons", flush=True)


if __name__ == "__main__": main()
