"""Inspect local BPS patches; writes metadata only, never a patched ROM."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import zlib


def decode(source, patch):
    assert patch[:4] == b'BPS1'
    pos = 4

    def number():
        nonlocal pos
        value, shift = 0, 1
        while True:
            x = patch[pos]
            pos += 1
            value += (x & 127) * shift
            if x & 128:
                return value
            shift <<= 7
            value += shift

    assert number() == len(source)
    size, meta = number(), number()
    pos += meta
    out = bytearray()
    srcpos = dstpos = 0
    while len(out) < size:
        action = number()
        mode, length = action & 3, (action >> 2) + 1
        if mode == 0:
            out.extend(source[len(out):len(out) + length])
        elif mode == 1:
            out.extend(patch[pos:pos + length])
            pos += length
        else:
            offset = number()
            offset = -(offset >> 1) if offset & 1 else offset >> 1
            if mode == 2:
                srcpos += offset
                out.extend(source[srcpos:srcpos + length])
                srcpos += length
            else:
                dstpos += offset
                for _ in range(length):
                    out.append(out[dstpos])
                    dstpos += 1
    assert len(out) == size and pos == len(patch) - 12
    assert struct.unpack_from('<III', patch, pos) == (
        zlib.crc32(source), zlib.crc32(out), zlib.crc32(patch[:-4]))
    return out


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--roms', type=Path, required=True)
    p.add_argument('--patches', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    args = p.parse_args()
    jp = (args.roms / 'B8CJ.gba').read_bytes()
    report = {}
    for code, letter in [('B8CP', 'E'), ('B8CE', 'U')]:
        source = (args.roms / (code + '.gba')).read_bytes()
        patch = next(args.patches.glob(f'*({letter})*.bps')).read_bytes()
        target = decode(source, patch)
        ranges = []
        start = None
        for i, (a, b) in enumerate(zip(source, target)):
            if a != b and start is None:
                start = i
            if a == b and start is not None and i - start > 0:
                ranges.append([start, i])
                start = None
        if start is not None:
            ranges.append([start, len(target)])
        groups = []
        for a, b in ranges:
            if groups and a - groups[-1][1] < 256:
                groups[-1][1] = b
            else:
                groups.append([a, b])
        report[code] = {'source_sha1': hashlib.sha1(source).hexdigest(),
                        'changed_bytes': sum(b-a for a,b in ranges), 'groups': []}
        for a, b in groups:
            chunk = bytes(target[a:b])
            match = jp.find(chunk) if len(chunk) >= 8 else -1
            report[code]['groups'].append({'start': hex(a), 'end': hex(b), 'size': b-a,
                                           'jp_match': hex(match) if match >= 0 else None})
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2))
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
