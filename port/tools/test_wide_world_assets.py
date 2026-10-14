"""Decode arena source assets for reviewing horizontal repetition (no game/ROM edits).

Generated images are local extracted assets: keep them under ignored build/.
This is an inventory, not a visual PASS or gameplay verification.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

import yaml
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[2]
PROFILES = {
    'TraverseTown': (96, 96), 'OlympusColiseum': (72, 72), 'Garden': (64, 72),
    'Wonderland': (96, 64), 'Atlantica': (64, 64), 'Monstro': (96, 96),
    'Agrabah': (72, 64), 'HalloweenTown': (64, 64), 'NeverLand': (96, 64),
    'DestinyIslands': (64, 80), 'HollowBastion': (96, 96), 'TwilightTown': (64, 64),
    'CastleOblivion': (64, 64), 'Hook': (64, 64), 'Hum': (96, 64),
    'Mansion': (64, 88), 'Ansem2': (64, 64), 'Marluxia': (72, 80), 'Hades': (64, 64),
}


def decompress(data):
    assert data[0] == 0x10
    size = int.from_bytes(data[1:4], 'little')
    out = bytearray()
    pos = 4
    while len(out) < size:
        flags = data[pos]
        pos += 1
        for bit in range(7, -1, -1):
            if len(out) >= size:
                break
            if flags & (1 << bit):
                first, second = data[pos:pos + 2]
                pos += 2
                count = (first >> 4) + 3
                distance = ((first & 15) << 8 | second) + 1
                for _ in range(count):
                    out.append(out[-distance])
            else:
                out.append(data[pos])
                pos += 1
    return bytes(out[:size])


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--version', choices=('eu', 'us', 'jp'), default='eu')
    ap.add_argument('--out', default='build/codex-wide-assets')
    args = ap.parse_args()
    target = ROOT / args.out / args.version
    target.mkdir(parents=True, exist_ok=True)
    rom = (ROOT / 'roms' / ({'eu': 'B8CP', 'us': 'B8CE', 'jp': 'B8CJ'}[args.version] + '.gba')).read_bytes()
    entries = yaml.safe_load((ROOT / 'config/assets/battle_backgrounds.yaml').read_text())['entries']
    entries = {entry['name']: entry for entry in entries}
    events = yaml.safe_load((ROOT / 'config/assets/event_backgrounds.yaml').read_text())['entries']
    entries.update({entry['name']: entry for entry in events if entry['name'].startswith('gBtlBgHades')})

    def read(name):
        entry = entries[name][args.version]
        start = entry['address'] - 0x08000000
        data = rom[start:start + entry['size']]
        return decompress(data) if entry.get('compress') == 'lz77' else data

    records = []
    pictures = []
    band_pictures = []
    for name in entries:
        if not name.endswith('Map'):
            continue
        base = name[:-3]
        tiles, tilemap, raw_palette = read(base + 'Tiles'), read(name), read(base + 'Palette')
        # BtlMap uses affine 512x512: 64x64 byte entries, 8bpp characters.
        assert len(tilemap) == 4096, (name, len(tilemap))
        palette = [( (c & 31) * 255 // 31, ((c >> 5) & 31) * 255 // 31,
                     ((c >> 10) & 31) * 255 // 31) for (c,) in struct.iter_unpack('<H', raw_palette)]
        palette += [(0, 0, 0)] * (256 - len(palette))
        indices = bytearray(512 * 512)
        for ty in range(64):
            for tx in range(64):
                tile = tilemap[ty * 64 + tx]
                assert (tile + 1) * 64 <= len(tiles), (name, tile, len(tiles))
                for row in range(8):
                    offset = (ty * 8 + row) * 512 + tx * 8
                    indices[offset:offset + 8] = tiles[tile * 64 + row * 8:tile * 64 + row * 8 + 8]
        im = Image.frombytes('P', (512, 512), bytes(indices))
        im.putpalette([component for color in palette for component in color])
        im = im.convert('RGB')
        im.save(target / (base + '.png'))
        left, right = (24, 488) if base == 'gBtlBgHook' else (48, 464)
        pixels = im.load()
        # Compare authored edge joins, weighted over visible battle rows. This
        # ranks candidates; an artist/reviewer must still inspect the result.
        def seam(a, b):
            return sum(sum(abs(x - y) for x, y in zip(pixels[a, row], pixels[b, row]))
                       for row in range(144, 496)) / (352 * 3)
        left_periods = sorted((seam(left, left + period - 1), period) for period in range(8, 97, 8))
        right_periods = sorted((seam(right - 1, right - period), period) for period in range(8, 97, 8))
        lp, rp = PROFILES[base.removeprefix('gBtlBg')]
        right_start = left if base == 'gBtlBgHook' else right - rp
        def reflect(distance, width):
            phase = distance % (width * 2)
            return phase if phase < width else width * 2 - 1 - phase
        extended = Image.new('RGB', (672, 512))
        extended.paste(im.crop((left, 0, right, 512)), (128, 0))
        for dx in range(128):
            sx = left + reflect(127 - dx, lp)
            extended.paste(im.crop((sx, 0, sx + 1, 512)), (dx, 0))
            sx = right_start + rp - 1 - reflect(dx, rp)
            extended.paste(im.crop((sx, 0, sx + 1, 512)), (128 + right - left + dx, 0))
        extended.save(target / (base + '_bands.png'))
        band_panel = Image.new('RGB', (672, 540), '#202020')
        band_panel.paste(extended, (0, 24))
        ImageDraw.Draw(band_panel).text((4, 4), base + f' bands {lp}/{rp}', fill='white')
        band_pictures.append(band_panel)
        # Full authored width and seam: last 64 pixels followed by first 64.
        panel = Image.new('RGB', (644, 540), '#202020')
        panel.paste(im, (0, 25))
        panel.paste(im.crop((448, 0, 512, 512)), (516, 25))
        panel.paste(im.crop((0, 0, 64, 512)), (580, 25))
        ImageDraw.Draw(panel).text((4, 5), base + ' | wrap seam at x=580', fill='white')
        pictures.append(panel)
        records.append({'asset': base, 'map_bytes': len(tilemap), 'tiles_bytes': len(tiles),
                        'pixel_hash': hashlib.sha256(indices).hexdigest(),
                        'rgb_hash': hashlib.sha256(im.tobytes()).hexdigest(),
                        'edge_tile_matches': sum(tilemap[y * 64] == tilemap[y * 64 + 63] for y in range(64)),
                        'bounds': [left, right], 'left_candidates': left_periods[:4],
                        'right_candidates': right_periods[:4],
                        'preview_periods': [lp, rp],
                        'right_start': right_start, 'extension': 'reflected authored bands',
                        'visual_review': 'required'})
    for start in range(0, len(pictures), 6):
        sheet = Image.new('RGB', (1288, 1620), '#202020')
        for i, panel in enumerate(pictures[start:start + 6]):
            sheet.paste(panel, ((i % 2) * 644, (i // 2) * 540))
        sheet.save(target / f'arenas_{start // 6}.png')
        band_sheet = Image.new('RGB', (1344, 1620), '#202020')
        for i, panel in enumerate(band_pictures[start:start + 6]):
            band_sheet.paste(panel, ((i % 2) * 672, (i // 2) * 540))
        band_sheet.save(target / f'bands_review{start // 6}.png')
    (target / 'inventory.json').write_text(json.dumps(records, indent=2) + '\n')
    print(f'{len(records)} decoded arenas: {target}; visual review required')


if __name__ == '__main__':
    main()
