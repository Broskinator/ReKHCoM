"""Validate HUD pieces and require explicit visual review before packaging.

PNG checks cannot prove perspective or fidelity; the review fields record
manual comparison against the original. Pending art is never packaged.
"""
import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CHECKS = ('silhouette', 'orientation', 'perspective', 'gradient', 'shadows',
          'internal_layout', 'runtime_bounds', 'gba_palette', 'toon_style')


def approved_pieces(manifest):
    spec = json.loads(manifest.read_text(encoding='utf-8'))
    names = set()
    approved = []
    for piece in spec['pieces']:
        key = piece['source_hash']
        if len(key) != 16 or any(c not in '0123456789abcdef' for c in key) or key in names:
            raise ValueError('Content keys must be unique lowercase FNV64 hashes')
        names.add(key)
        if piece.get('status') != 'approved':
            continue
        from PIL import Image
        if piece['file'] != key + '.png':
            raise ValueError(f'{key}: approved file must match its content key')
        if not all(piece.get('review', {}).get(field) is True for field in CHECKS):
            raise ValueError(f'{key}: incomplete original/style/runtime review')
        if len(piece['native_size']) != 2 or any(type(n) is not int or n < 1 or n > 64 for n in piece['native_size']):
            raise ValueError(f'{key}: invalid native OBJ size')
        with Image.open(manifest.parent / piece['file']) as im:
            if im.format != 'PNG' or im.mode != 'RGBA' or max(im.size) > 4096 or im.width*im.height > 4194304:
                raise ValueError(f'{key}: invalid/budget-exceeding RGBA PNG')
            a = im.getchannel('A')
            if not a.getbbox():
                raise ValueError(f'{key}: empty art')
            edges = ((0, 0, im.width, 1), (0, im.height-1, im.width, im.height),
                     (0, 0, 1, im.height), (im.width-1, 0, im.width, im.height))
            # The renderer clears only 1/255 quantization noise on import.
            if any(a.crop(edge).getextrema()[1] > 1 for edge in edges):
                raise ValueError(f'{key}: residual alpha on exterior edge')
        approved.append(piece)
    return approved


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest', type=Path, default=ROOT / 'port/assets/hd-hud/sprites/manifest.json')
    args = parser.parse_args()
    try:
        pieces = approved_pieces(args.manifest)
    except (ValueError, KeyError, OSError) as exc:
        parser.exit(1, f'HUD review failed: {exc}\n')
    total = len(json.loads(args.manifest.read_text())['pieces'])
    print(f'{len(pieces)}/{total} approved for packaging; remaining pieces are drafts, not completed coverage')
