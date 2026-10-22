#!/usr/bin/env python3
"""Rows of port/data/hd_sprites.tsv for the menus' title banners (group 5): the cartridge's banner (a bar with an
arrow end) drawn as polygons at 8x in its own palette's colours, and the title in Re:CoM's lettering, which is the
GBA's typeface: whole words where Re:CoM has them, letter by letter (sblit, src= for another texture) where it does
not. Everything about the banner is read from the ROM: its frame's OAM parts (the box), the top line's length (where
the arrow starts), the body and outline colours, where the text starts. The text's capitals fill the GBA's rows 4..10.

    python port/tools/hd_menu_titles.py [--write]   (without --write: print the rows)
"""
import re
import struct
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
TSV = ROOT / 'port/data/hd_sprites.tsv'
K = 8
SHAPES = {(0, 0): (8, 8), (0, 1): (16, 16), (0, 2): (32, 32), (0, 3): (64, 64), (1, 0): (16, 8), (1, 1): (32, 8),
          (1, 2): (32, 16), (1, 3): (64, 32), (2, 0): (8, 16), (2, 1): (8, 32), (2, 2): (16, 32), (2, 3): (32, 64)}
CAPS = 4, 11  # the GBA rows (frame y) the capitals fill

# Re:CoM's Spanish lettering: texture, then glyphs as (x0, y0, x1, y1, cap top, cap bottom[, op[, advance]]): the
# advance (texture pixels to the next letter) where the rectangle's width minus the italic overlap is not it
CAMP = 'FORM/0100/FO0100.RTM:SP_camp.imz#0'
WWARP = 'FORM/0108/FO0108.RTM:SP_wwarp.imz#0'
SAVE = 'FORM/0106/FO0106.RTM:SP_save.imz#0'
JIMINY = 'FORM/0110/FO0110.RTM:SP_jiminy_top.imz#0'
ROOM = 'FORM/0102/FO0102.RTM:SP_room01.imz#0'
WSEL = 'FORM/0006/FO0006.RTM:SP_wselect.imz#0'
GETC = 'FORM/0005/FO0005.RTM:SP_get_card.imz#0'
LVUP = 'FORM/0103/FO0103.RTM:SP_levelup.imz#0'
UCAMP = 'FORM/0100/FO0100.RTM:UK_camp.imz#0'
UWWARP = 'FORM/0108/FO0108.RTM:UK_wwarp.imz#0'  # WORLD WARP / WORLD CARDS
USAVE = 'FORM/0106/FO0106.RTM:UK_save.imz#0'
UJIMINY = 'FORM/0110/FO0110.RTM:UK_jiminy_top.imz#0'
UROOM = 'FORM/0102/FO0102.RTM:UK_room01.imz#0'
UWSEL = 'FORM/0006/FO0006.RTM:UK_wselect.imz#0'
UGETC = 'FORM/0005/FO0005.RTM:UK_get_card.imz#0'  # CARD OBTAINED! / PREMIUM CARD! / CLEAR BONUS / KINGDOM HEARTS
ULVUP = 'FORM/0103/FO0103.RTM:UK_levelup.imz#0'
U1, U2, U3, U4 = (282, 300), (316, 334), (352, 370), (388, 406)
UG = {'C': (62, 84, U1), 'A': (84, 105, U1), 'R': (105, 126, U1), 'D': (126, 147, U1), 'O': (157, 179, U1),
      'B': (179, 200, U1), 'T': (202, 220, U1), 'I': (240, 253, U1), 'N': (252, 274, U1), 'E': (273, 295, U1),
      '!': (316, 328, U1), 'P': (71, 92, U2), 'M': (134, 156, U2), 'U': (167, 190, U2), 'L': (104, 122, U3),
      'S': (284, 305, U3), 'K': (12, 34, U4), 'G': (67, 89, U4), 'H': (163, 185, U4)}
W2 = dict(N=(522, 555), A=(552, 585), I=(582, 602), P=(598, 632), E=(629, 663), S=(660, 694), M=(704, 737),
          U=(734, 769), D=(796, 830), O=(826, 860))
GLYPHS = {
    **{(WWARP, k): (x0, 60, x1, 86, 60, 86) for k, (x0, x1) in W2.items()},
    (WWARP, 'V'): (522, 15, 565, 42, 15, 42, 'sblit', 38), (WWARP, 'T'): (645, 15, 680, 42, 15, 42),
    (GETC, '¡'): (84, 316, 98, 336, 316, 334), (GETC, '!'): (290, 316, 304, 334, 316, 334),
    (GETC, 'N'): (95, 316, 117, 334, 316, 334), (GETC, 'A'): (116, 316, 137, 334, 316, 334),
    (GETC, 'I'): (136, 316, 149, 334, 316, 334), (GETC, 'E'): (168, 316, 190, 334, 316, 334),
    (GETC, 'T'): (253, 316, 271, 334, 316, 334), (GETC, 'B'): (12, 352, 35, 370, 352, 370, 'sblit', 19),
    (GETC, 'J'): (116, 352, 138, 370, 352, 370), (GETC, 'U'): (135, 352, 158, 370, 352, 370),
    (GETC, 'G'): (67, 388, 89, 406, 388, 406), (GETC, 'D'): (88, 388, 110, 406, 388, 406),
    (GETC, 'O'): (110, 388, 131, 406, 388, 406), (GETC, 'M'): (131, 388, 152, 406, 388, 406),
    (GETC, 'R'): (228, 388, 249, 406, 388, 406), (GETC, 'S'): (269, 388, 290, 406, 388, 406),
    **{(UGETC, k): (x0, y[0], x1, y[1], y[0], y[1]) for k, (x0, x1, y) in UG.items()},
    (UWWARP, 'W'): (522, 16, 561, 42, 16, 42), (ULVUP, 'V'): (590, 154, 632, 180, 154, 180, 'blit', 36),
}
WORDS = {  # whole words: texture, rectangle, cap top, cap bottom
    'MENÚ': (CAMP, (14, 856, 157, 889), 863, 889), 'NAIPES MUNDO': (WWARP, (522, 60, 860, 86), 60, 86),
    'CARGAR': (SAVE, (522, 18, 717, 44), 18, 44), 'GUARDAR': (SAVE, (522, 62, 749, 88), 62, 88),
    'DIARIO': (JIMINY, (800, 11, 1013, 41), 11, 41), 'VÓRTICE': (WWARP, (522, 7, 781, 42), 15, 42),
    'SÍNTESIS DE ESTANCIAS': (ROOM, (521, 7, 944, 42), 15, 42), 'ELIGE UN MUNDO': (WSEL, (24, 168, 502, 194), 168, 194),
    'NIVEL': (LVUP, (524, 154, 712, 180), 154, 180), '↑': (LVUP, (746, 148, 770, 180), 154, 180),
    'BONUS': (GETC, (12, 352, 110, 370), 352, 370), 'ÉLITE': (GETC, (198, 310, 290, 334), 316, 334),
    'MENU': (UCAMP, (14, 858, 156, 889), 863, 889), 'WORLD CARDS': (UWWARP, (522, 60, 860, 86), 60, 86),
    'WORLD WARP': (UWWARP, (522, 16, 861, 42), 16, 42), 'LOAD': (USAVE, (522, 18, 663, 44), 18, 44),
    'SAVE': (USAVE, (522, 62, 665, 88), 62, 88), 'JOURNAL': (UJIMINY, (713, 11, 1006, 41), 11, 41),
    'ROOM SYNTHESIS': (UROOM, (523, 15, 940, 41), 15, 41), 'SELECT WORLD': (UWSEL, (23, 168, 442, 194), 168, 194),
    'LEVEL UP': (ULVUP, (524, 154, 772, 180), 154, 180), 'PREMIUM': (UGETC, (71, 316, 211, 334), 316, 334),
    'CLEAR BONUS:BONUS': (UGETC, (199, 352, 305, 370), 352, 370),
}

# tiles, frame, palette, text as pieces: a whole word, (texture, letters), or ' ' for a word space; then the tiles
# the game loads when they are not <tiles>Tiles, and those the banner's shape is read from when they differ
BANNERS = [
    ('gMapMenuTitleSpanish', 0, 'gMapMenuBarPalette', ['MENÚ']),
    ('gWorldInspectBarSpanish', 0, 'gWorldInspectBarPalette', ['NAIPES MUNDO']),
    ('gMapInspectBarEs', 5, 'gMapInspectBarPalette', [(WWARP, 'NAIPES'), ' ', (WWARP, 'MAPA')]),
    ('gStatusBarSpanish', 2, 'gStatusBarPalette', [(WWARP, 'ES'), (WWARP, 'T'), (WWARP, 'ADO')]),
    ('gMenuLoadTitleSpanish', 0, 'gLoadMenuTitlePalette', ['CARGAR']),
    ('gJiminyTitleLocalized', 4, 'gJiminyTitlePalette', ['DIARIO']),
    ('gMapSaveTitleSpanish', 0, 'gSaveMenuTitlePalette', ['GUARDAR']),
    ('gMapSelectTitleSpanish', 0, 'gSmallHandCursorPalette', ['SÍNTESIS DE ESTANCIAS']),
    ('gWorldWarpBarSpanish', 0, 'gWorldWarpBarPalette', ['VÓRTICE']),
    ('gWorldselectTitleSpanish', 0, 'gWorldselectOverlayPalette', ['ELIGE UN MUNDO']),
    ('gLevelUpHeaderSpanish', 10, 'gLevelUpHeaderPalette', [(GETC, '¡'), 'NIVEL', '↑', (GETC, '!')]),
    ('gPremireChanceTitleSpanish', 0, 'gPremireChancePalette', ['BONUS', ' ', 'ÉLITE']),
    ('gDeckTitleBannerSpanish', 0, 'gDeckTitleBannerPalette',
     [(WWARP, 'V'), (GETC, 'ER'), ' ', (GETC, 'BARAJAS')]),
    ('gMsTopBarSpanish', 0, 'gMsTopBarPalette', [(GETC, 'TIENDA'), ' ', (GETC, 'MOGURI')], 'gMsTopBarTiles',
     'gMsTopBarSpanishTiles'),
    ('gRikuDeckTitleBannerSpanish', 0, 'gDeckTitleBannerPalette',
     [(WWARP, 'V'), (GETC, 'ER'), ' ', (GETC, 'BARAJA')]),
    # English (the European version's and the American one)
    ('gMapMenuTitle', 0, 'gMapMenuBarPalette', ['MENU'], None, None, 'eu,us'),
    ('gWorldInspectBar', 0, 'gWorldInspectBarPalette', ['WORLD CARDS'], None, None, 'eu,us'),
    ('gMapInspectBar', 5, 'gMapInspectBarPalette', [(UGETC, 'MAP'), ' ', (UGETC, 'CARDS')], None, None, 'eu,us'),
    ('gStatusBar', 2, 'gStatusBarPalette', [(UGETC, 'STATUS')], None, None, 'eu,us'),
    ('gMenuLoadTitle', 0, 'gLoadMenuTitlePalette', ['LOAD'], None, None, 'eu,us'),
    ('gJiminyTitle', 0, 'gJiminyTitlePalette', ['JOURNAL'], None, None, 'eu,us'),
    ('gMapSaveTitle', 0, 'gSaveMenuTitlePalette', ['SAVE'], None, None, 'eu,us'),
    ('gMapSelectTitle', 0, 'gSmallHandCursorPalette', ['ROOM SYNTHESIS'], None, None, 'eu,us'),
    ('gWorldWarpBar', 0, 'gWorldWarpBarPalette', ['WORLD WARP'], None, None, 'eu,us'),
    ('gWorldselectTitle', 0, 'gWorldselectOverlayPalette', ['SELECT WORLD'], None, None, 'eu,us'),
    ('gLevelUpHeader', 10, 'gLevelUpHeaderPalette', ['LEVEL UP', (UGETC, '!')], None, None, 'eu,us'),
    ('gPremireChanceTitle', 0, 'gPremireChancePalette', ['PREMIUM', ' ', 'CLEAR BONUS:BONUS'], None, None, 'eu,us'),
    ('gMsTopBar', 0, 'gMsTopBarPalette', [(UGETC, 'MOOGLE'), ' ', (UGETC, 'SHOP')], None, None, 'eu,us'),
    ('gDeckTitleBanner', 0, 'gDeckTitleBannerPalette',
     [(UGETC, 'RE'), (ULVUP, 'V'), (UGETC, 'IE'), (UWWARP, 'W'), ' ', (UGETC, 'DECKS')], None, None, 'eu,us'),
    ('gRikuDeckTitleBanner', 0, 'gDeckTitleBannerPalette',
     [(UGETC, 'RE'), (ULVUP, 'V'), (UGETC, 'IE'), (UWWARP, 'W'), ' ', (UGETC, 'DECK')], None, None, 'eu,us'),
]


def rom():
    data = (ROOT / 'roms' / 'B8CP.gba').read_bytes()
    syms = {k: int(v, 16) for k, v in re.findall(r'\.set _(\w+), (0x[0-9a-f]+)',
                                                 (ROOT / 'build/eu/pc/rom_symbols.s').read_text())}
    return data, syms


def frame_pixels(data, syms, frame, tiles):
    """The frame's pixels as {(x, y): palette index}, and the union of its OAM parts (x0, y0, w, h)."""
    fo, to = syms[frame] - 0x08000000, syms[tiles] - 0x08000000
    n = struct.unpack_from('<H', data, fo)[0]
    px, box = {}, [9999, 9999, -9999, -9999]
    for k in range(min(n, 64)):
        a0, a1, a2 = struct.unpack_from('<HHH', data, fo + 2 + k * 6)
        if (a0 >> 14, a1 >> 14) not in SHAPES:
            continue
        pw, ph = SHAPES[(a0 >> 14, a1 >> 14)]
        x = a1 & 0x1FF
        x -= 512 if x & 0x100 else 0
        y = a0 & 0xFF
        y -= 256 if y & 0x80 else 0
        box = [min(box[0], x), min(box[1], y), max(box[2], x + pw), max(box[3], y + ph)]
        for ty in range(ph):
            for tx in range(pw):
                sx = pw - 1 - tx if (a1 >> 12) & 1 else tx
                sy = ph - 1 - ty if (a1 >> 13) & 1 else ty
                t = (a2 & 0x3FF) + (sy >> 3) * (pw >> 3) + (sx >> 3)
                b = data[to + t * 32 + (sy & 7) * 4 + ((sx & 7) >> 1)]
                v = b >> 4 if sx & 1 else b & 15
                if v:
                    px[(x + tx, y + ty)] = v
    return px, (box[0], box[1], box[2] - box[0], box[3] - box[1])


def colour(data, syms, palette, index):
    c = struct.unpack_from('<H', data, syms[palette] - 0x08000000 + 2 * index)[0]
    return '%02X%02X%02X' % ((c & 31) * 255 // 31, ((c >> 5) & 31) * 255 // 31, ((c >> 10) & 31) * 255 // 31)


def pieces(text):
    """(texture, rect, cap top, cap bottom, op) of each glyph or word, None for a word space."""
    out = []
    for p in text:
        if p == ' ':
            out.append(None)
        elif isinstance(p, tuple):
            for ch in p[1]:
                g = GLYPHS[(p[0], ch)]
                out.append((p[0], g[:4], g[4], g[5], g[6] if len(g) > 6 else 'sblit', g[7] if len(g) > 7 else None))
        else:
            tex, rect, ct, cb = WORDS[p]
            out.append((tex, rect, ct, cb, 'blit', None))
    return out


def row(spec, data, syms):
    name, fn, palette, text = spec[:4]
    tiles, frame = (spec[4] if len(spec) > 4 and spec[4] else name + 'Tiles'), f'{name}Frame{fn}'
    px, (bx, by, bw, bh) = frame_pixels(data, syms, frame, tiles)
    if len(spec) > 5 and spec[5]:  # the picture the screen shows once still (its BG copy) is drawn from these tiles
        px = frame_pixels(data, syms, frame, spec[5])[0]
    top = min(y for _, y in px)
    top_index = px[(0, top)]
    length = 0
    while px.get((length, top)) == top_index:
        length += 1
    body_index = Counter(v for (x, y), v in px.items() if y == top + 1 and x < length - 2).most_common(1)[0][0]
    tx = min(x for (x, y), v in px.items() if CAPS[0] <= y - top + 1 < CAPS[1] and x < length - 1 and
             v not in (body_index, top_index))
    line, body = colour(data, syms, palette, top_index), colour(data, syms, palette, body_index)

    def X(x):
        return round((x - bx) * K)

    def Y(y):
        return round((y - by) * K)
    t = top - 1  # rows are counted from the banner's top line at frame row 1
    tip = length + 6.5
    ops = [f'canvas={bw * K},{bh * K}',
           f'poly={line}FF,' + ','.join(map(str, (X(bx), Y(t + 1), X(length), Y(t + 1), X(tip), Y(t + 7.5),
                                                   X(length), Y(t + 14), X(bx), Y(t + 14)))),
           f'poly={body}FF,' + ','.join(map(str, (X(bx), Y(t + 2), X(length - 0.5), Y(t + 2), X(tip - 1.3),
                                                   Y(t + 7.5), X(length - 0.5), Y(t + 13), X(bx), Y(t + 13)))),
           'ink=FFFFFF,' + ''.join('%02X' % (255 - (255 - int(body[i:i + 2], 16)) * 12 // 100) for i in (0, 2, 4))]
    h = CAPS[1] - CAPS[0]
    # lay the pieces out at the capitals' height, letters overlapping as italics do, then squeeze if needed
    laid, x = [], 0.0
    for p in pieces(text):
        if p is None:
            x += 13 * h / 26
            continue
        tex, r, ct, cb, op, advance = p
        s = h / (cb - ct)
        w = (r[2] - r[0]) * s
        laid.append((tex, r, op, x, w, CAPS[0] + t - (ct - r[1]) * s, CAPS[0] + t + (r[3] - ct) * s))
        x += advance * s if advance else w - (3 * s if op == 'sblit' else 0)
    width = max(l[3] + l[4] for l in laid)
    f = min(1.0, (length + 1 - tx) / width)
    current = None
    own = laid[0][0]
    for tex, r, op, x0, w, y0, y1 in laid:
        want = '' if tex == own else tex
        if want != current:
            if current is not None or want:
                ops.append(f'src={want}')
            current = want
        ops.append(f'{op}=' + ','.join(map(str, (*r, X(tx + x0 * f), Y(y0), X(tx + (x0 + w) * f), Y(y1)))))
    ops.append(f'scale={K}')
    return '\t'.join([name + 'Banner', '5', frame, tiles, str(bx), str(by), str(bw), str(bh), own, ';'.join(ops),
                      spec[6] if len(spec) > 6 else 'eu'])


def main():
    data, syms = rom()
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
