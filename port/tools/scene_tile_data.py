"""Decode original event background tiles for local scene studies. No ROM assets are stored here."""
import re
import struct
from pathlib import Path

K = Path(__file__).resolve().parents[2]
ROM = (K / "roms/B8CP.gba").read_bytes()
def at(a, n):
    return ROM[a - 0x08000000:a - 0x08000000 + n]


def u32(a):
    return struct.unpack("<I", at(a, 4))[0]


def lz77(a):
    hdr = u32(a); size = hdr >> 8
    src = ROM[a - 0x08000000 + 4:]; dst = bytearray(); i = 0
    while len(dst) < size:
        f = src[i]; i += 1
        for b in range(8):
            if len(dst) >= size:
                break
            if f & (0x80 >> b):
                v = src[i] << 8 | src[i + 1]; i += 2
                for _ in range((v >> 12) + 3):
                    dst.append(dst[-((v & 0xFFF) + 1)])
            else:
                dst.append(src[i]); i += 1
    return bytes(dst[:size])


def bg_symbol(event_id):
    src = (K / "src/evt/event_index_data.c").read_text(encoding="utf-8")
    body = src[src.index("gEventBackgroundDefs[] = {"):]
    body = body[:body.index("};")]
    names = re.findall(r"(NULL|&gEventBg\w+)", body)
    name = names[event_id]
    if name == "NULL":
        return None, None
    name = name[1:]
    syms = (K / "build/eu/pc/rom_symbols.s").read_text(encoding="utf-8")
    m = re.search(r"\.set _" + name + r", (0x[0-9a-fA-F]+)", syms)
    return name, int(m.group(1), 16)


def load_bg(addr):
    d = at(addr, 48)
    tiles, tiles2, pal, maps, maps2, maps3 = struct.unpack_from("<6I", d, 0)
    tsize, tsize2, psize, mw, mh = struct.unpack_from("<HHHBB", d, 24)
    comp = d[46]
    tdata = lz77(tiles) if comp in (1, 3) else at(tiles, tsize)
    tdata2 = (lz77(tiles2) if comp in (1, 3) else at(tiles2, tsize2)) if tiles2 else b""
    pdata = at(pal, psize)
    colors = [((c & 31) * 255 // 31, (c >> 5 & 31) * 255 // 31, (c >> 10 & 31) * 255 // 31)
              for c in struct.unpack_from("<%dH" % (len(pdata) // 2), pdata)]
    colors += [(255, 0, 255)] * (256 - len(colors))

    def grid(ptrs):
        if not ptrs:
            return None
        g = [[0] * (mw * 32) for _ in range(mh * 32)]
        for by in range(mh):
            for bx in range(mw):
                p = u32(ptrs + 4 * (by * mw + bx))
                blk = lz77(p) if comp in (2, 3) else at(p, 0x800)
                for ty in range(32):
                    for tx in range(32):
                        g[by * 32 + ty][bx * 32 + tx] = struct.unpack_from("<H", blk, (ty * 32 + tx) * 2)[0]
        return g
    return dict(tiles=tdata, tiles2=tdata2, colors=colors, w=mw, h=mh,
                bg3=grid(maps), bg2=grid(maps2), bg1=grid(maps3))


