#!/usr/bin/env python3
"""Rows of port/data/hd_sprites.tsv for the worlds' icons (group 5): each cartridge icon (world inspect, world warp)
is Re:CoM's own world_icon picture (FORM/0150..0162), the same drawing. The picture's opaque box is fitted into the
cartridge icon's ink box (from the ROM) keeping its proportions, centred; the sprite's box is its frame's OAM parts.
The picture's box is measured here from the player's install, so the rows only name rectangles.

    python port/tools/hd_world_icons.py <Recom.hed> [--write]   (without --write: print the rows)
"""
import re
import struct
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
import khhd_pkg  # noqa: E402
from hd_menu_titles import frame_pixels  # noqa: E402
from khhd_sprites import decode  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
TSV = ROOT / 'port/data/hd_sprites.tsv'
K = 8
NAMES = 'D:/01Claude/ReKHCoM-build/refs/OpenKh/OpenKh.Egs/resources/recom.txt'
# cartridge world, Re:CoM's form; 100 Acre Wood's picture also holds a page beside the book: only the book
WORLDS = [('TraverseTown', 150), ('Agrabah', 151), ('OlympusColiseum', 152), ('Wonderland', 153), ('Monstro', 154),
          ('HalloweenTown', 155), ('Atlantica', 156), ('NeverLand', 157), ('HollowBastion', 158),
          ('100AcreWood', 159, (0, 0, 140, 256)), ('TwilightTown', 160), ('DestinyIslands', 161),
          ('CastleOblivion', 162)]


def main():
    hed = Path(sys.argv[1])
    data = (ROOT / 'roms' / 'B8CP.gba').read_bytes()
    syms = {k: int(v, 16) for k, v in re.findall(r'\.set _(\w+), (0x[0-9a-f]+)',
                                                 (ROOT / 'build/eu/pc/rom_symbols.s').read_text())}
    names = khhd_pkg.names_by_md5(NAMES)
    where = {names.get(m, m): o for m, o, _, _ in khhd_pkg.entries(hed)}
    rows = []
    with open(hed.with_suffix('.pkg'), 'rb') as pkg:
        for world, form, *limit in WORLDS:
            asset = f'FORM/0{form}/FO0{form}.RTM'
            img = decode(khhd_pkg.read_asset(pkg, where[asset], with_remastered=True)[2]['world_icon.imz'], 0)
            a = img[:, :, 3] > 16
            if limit:
                x0, y0, x1, y1 = limit[0]
                a[:, :x0] = a[:, x1:] = False
                a[:y0], a[y1:] = False, False
            ys, xs = np.nonzero(a)
            crop = xs.min(), ys.min(), xs.max() + 1, ys.max() + 1
            tiles, frame = f'gWorldIcon{world}Tiles', f'gWorldIcon{world}Frame0'
            px, (bx, by, bw, bh) = frame_pixels(data, syms, frame, tiles)
            ix0, iy0 = min(x for x, _ in px), min(y for _, y in px)
            ix1, iy1 = max(x for x, _ in px) + 1, max(y for _, y in px) + 1
            # fit (the cartridge's ink includes a one-pixel outline the picture does not have)
            cw, ch = crop[2] - crop[0], crop[3] - crop[1]
            iw, ih = ix1 - ix0 - 1, iy1 - iy0 - 1
            s = min(iw / cw, ih / ch)
            w, h = cw * s, ch * s
            cx, cy = (ix0 + ix1) / 2, (iy0 + iy1) / 2
            place = [round(cx - w / 2 - bx), round(cy - h / 2 - by), round(cx + w / 2 - bx), round(cy + h / 2 - by)]
            ops = f'crop={",".join(map(str, crop))};place={",".join(map(str, place))};scale={K}'
            rows.append('\t'.join([f'gWorldIcon{world}HD', '5', frame, tiles, str(bx), str(by), str(bw), str(bh),
                                   f'{asset}:world_icon.imz#0', ops, 'eu,us,jp']))
    if '--write' not in sys.argv:
        print('\n'.join(rows))
        return
    keep = [l for l in TSV.read_text(encoding='utf-8').splitlines() if not re.match(r'gWorldIcon\w+HD\t', l)]
    TSV.write_text('\n'.join(keep + rows) + '\n', encoding='utf-8')
    print(f'{len(rows)} world icon rows in {TSV.relative_to(ROOT)}')


if __name__ == '__main__':
    main()
