"""Write a room seed into a cartridge save (both copies), fixing the checksums.

For the crystal test (72): the same save, with only the current room's seed
changed, is loaded on the original (BizHawk) and on the port; the game builds
the room again from that seed, so both can be compared seed by seed.

    python port/tools/save_reseed.py in.sav out.sav 0x2242C2F1 [--old 0x90FAECF1]
"""
import argparse
import struct

ap = argparse.ArgumentParser()
ap.add_argument('src')
ap.add_argument('dst')
ap.add_argument('seed')
ap.add_argument('--old', default='0x90FAECF1', help='the seed to replace (finds the room record)')
args = ap.parse_args()
data = bytearray(open(args.src, 'rb').read())
SIG = b'KHCOM_BACKUP'
BLOCK = 0xF14


def checksum(buf):
    s = 0
    for i in range(0, len(buf) - 1, 2):
        s += buf[i] | buf[i + 1] << 8
    if len(buf) & 1:
        s += buf[-1]
    s = (s & 0xFFFF) + (s >> 16)
    return ~(s + (s >> 16)) & 0xFFFF


old, new = struct.pack('<I', int(args.old, 0)), struct.pack('<I', int(args.seed, 0))
starts = [i for i in range(len(data) - len(SIG)) if data[i:i + len(SIG)] == SIG]
patched = 0
for start in starts:
    block = data[start:start + BLOCK]
    at = block.find(old)
    if at < 0 or len(block) < BLOCK:
        continue
    assert checksum(block) == 0, f'block {start:#x} does not verify before the change'
    block[at:at + 4] = new
    block[0x18:0x1A] = b'\0\0'
    block[0x18:0x1A] = struct.pack('<H', checksum(block))
    assert checksum(block) == 0
    data[start:start + BLOCK] = block
    patched += 1
    print(f'block {start:#x}: seed at +{at:#x} -> {args.seed}')
assert patched, 'no save block holds that seed'
open(args.dst, 'wb').write(data)
