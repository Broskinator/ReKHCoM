#!/usr/bin/env python3
"""Rows of port/data/hd_sprites.tsv for the menus' title banners (group 5): the cartridge's banner (a bar with an
arrow end, its outline and its colour) drawn as polygons at 8x, and the title in Re:CoM's own lettering, which is
the GBA's typeface: whole words where Re:CoM has them, letter by letter (sblit) from one of its textures where it
does not. Each banner's box is the union of its frame's OAM parts (read from the ROM), so every part finds its piece.

    python port/tools/hd_menu_titles.py [--write]   (without --write: print the rows)
"""
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
TSV = ROOT / 'port/data/hd_sprites.tsv'
K = 8
SHAPES = {(0, 0): (8, 8), (0, 1): (16, 16), (0, 2): (32, 32), (0, 3): (64, 64), (1, 0): (16, 8), (1, 1): (32, 8),
          (1, 2): (32, 16), (1, 3): (64, 32), (2, 0): (8, 16), (2, 1): (8, 32), (2, 2): (16, 32), (2, 3): (32, 64)}

# Re:CoM's Spanish lettering (HD texture of each menu, first image): whole words and letters, x0, y0, x1, y1
WWARP = 'FORM/0108/FO0108.RTM:SP_wwarp.imz#0'  # VÓRTICE / NAIPES MUNDO
L2 = 60, 86  # NAIPES MUNDO's line
G = {'N': (522, 555), 'A': (552, 585), 'I': (582, 602), 'P': (598, 632), 'E': (629, 663), 'S': (660, 694),
     'M': (704, 737), 'U': (734, 769), 'D': (796, 830), 'O': (826, 860)}
GT = (645, 15, 680, 42)  # VÓRTICE's T (a slightly taller line)
SPACE = 13  # a word space beyond a letter's advance, in the texture's pixels


def letters(text):
    """(rect, x) of each letter of text in the texture's pixels, x from the word's start; and the word's width."""
    out, x = [], 0
    for ch in text:
        if ch == ' ':
            x += SPACE
            continue
        if ch == 'T':
            r = GT
            w = round((GT[2] - GT[0]) * (L2[1] - L2[0]) / (GT[3] - GT[1]))
        else:
            r = (G[ch][0], L2[0], G[ch][1], L2[1])
            w = r[2] - r[0]
        out.append((r, x, w))
        x += w - 3  # italic letters overlap by three pixels
    return out, x + 3


# name, tiles, frame, top line length, text x, text rows (frame y0, y1), body colour, text source, regions
BANNERS = [
    ('gMapMenuTitleSpanishTiles', 'gMapMenuTitleSpanishFrame0', 45, 5.5, (3, 11), '002A8C',
     ('FORM/0100/FO0100.RTM:SP_camp.imz#0', (14, 856, 157, 889)), 'eu'),  # MENÚ
    ('gWorldInspectBarSpanishTiles', 'gWorldInspectBarSpanishFrame0', 97, 5, (4, 11), '002A8C',
     (WWARP, (522, 60, 860, 86)), 'eu'),  # NAIPES MUNDO
    ('gMapInspectBarEsTiles', 'gMapInspectBarEsFrame5', 89, 5, (4, 11), '002A8C', (WWARP, 'NAIPES MAPA'), 'eu'),
    ('gStatusBarSpanishTiles', 'gStatusBarSpanishFrame2', 56, 5, (4, 11), '002A8C', (WWARP, 'ESTADO'), 'eu'),
    ('gMenuLoadTitleSpanishTiles', 'gMenuLoadTitleSpanishFrame0', 56, 5, (4, 11), '002A8C',
     ('FORM/0106/FO0106.RTM:SP_save.imz#0', (522, 18, 717, 44)), 'eu'),  # CARGAR
    ('gJiminyTitleLocalizedTiles', 'gJiminyTitleLocalizedFrame4', 51, 4.5, (4, 11), '31AD00',
     ('FORM/0110/FO0110.RTM:SP_jiminy_top.imz#0', (800, 11, 1013, 41)), 'eu'),  # DIARIO
]


def parts_box(data, syms, frame):
    """The union of the frame's OAM parts: x0, y0, w, h in frame coordinates."""
    fo = syms[frame] - 0x08000000
    n = struct.unpack_from('<H', data, fo)[0]
    xs, ys, xe, ye = [], [], [], []
    for k in range(min(n, 64)):
        a0, a1, _ = struct.unpack_from('<HHH', data, fo + 2 + k * 6)
        if (a0 >> 14, a1 >> 14) not in SHAPES:
            continue
        pw, ph = SHAPES[(a0 >> 14, a1 >> 14)]
        x = a1 & 0x1FF
        x -= 512 if x & 0x100 else 0
        y = a0 & 0xFF
        y -= 256 if y & 0x80 else 0
        xs.append(x), ys.append(y), xe.append(x + pw), ye.append(y + ph)
    return min(xs), min(ys), max(xe) - min(xs), max(ye) - min(ys)


def row(spec, data, syms):
    tiles, frame, top, tx, (ty0, ty1), body, (tex, text), regions = spec
    bx, by, bw, bh = parts_box(data, syms, frame)

    def px(x):
        return round((x - bx) * K)

    def py(y):
        return round((y - by) * K)
    tip = top + 6.5
    ops = [f'canvas={bw * K},{bh * K}',
           # the outline (white), then the body inside it: a bar from the left edge with an arrow end
           'poly=FFFFFFFF,' + ','.join(map(str, (px(bx), py(1), px(top), py(1), px(tip), py(7.5), px(top), py(14),
                                                  px(bx), py(14)))),
           f'poly={body}FF,' + ','.join(map(str, (px(bx), py(2), px(top - 0.5), py(2), px(tip - 1.3), py(7.5),
                                                   px(top - 0.5), py(13), px(bx), py(13)))),
           'ink=FFFFFF,E4EAF8']
    h = ty1 - ty0
    room = top + 1 - tx  # the text ends before the arrow's slope
    if isinstance(text, tuple):
        sw, sh = text[2] - text[0], text[3] - text[1]
        w = min(sw * h / sh, room)
        ops.append('blit=' + ','.join(map(str, (*text, px(tx), py(ty0), px(tx + w), py(ty1)))))
    else:
        glyphs, width = letters(text)
        sh = L2[1] - L2[0]
        f = min(h / sh, room / width)  # GBA pixels a texture pixel; squeezed only when it would not fit
        for r, x, w in glyphs:
            x0 = tx + x * f
            ops.append('sblit=' + ','.join(map(str, (*r, px(x0), py(ty0), px(x0 + w * f), py(ty1)))))
    ops.append(f'scale={K}')
    return '\t'.join([tiles.replace('Tiles', 'Banner'), '5', frame, tiles, str(bx), str(by), str(bw), str(bh),
                      tex, ';'.join(ops), regions])


def main():
    data = (ROOT / 'roms' / 'B8CP.gba').read_bytes()
    syms = {k: int(v, 16) for k, v in re.findall(r'\.set _(\w+), (0x[0-9a-f]+)',
                                                 (ROOT / 'build/eu/pc/rom_symbols.s').read_text())}
    rows = [row(b, data, syms) for b in BANNERS]
    if '--write' not in sys.argv:
        print('\n'.join(rows))
        return
    names = {r.split('\t')[0] for r in rows}
    keep = [l for l in TSV.read_text(encoding='utf-8').splitlines() if l.split('\t')[0] not in names]
    TSV.write_text('\n'.join(keep + rows) + '\n', encoding='utf-8')
    print(f'{len(rows)} banner rows in {TSV.relative_to(ROOT)}')


if __name__ == '__main__':
    main()
