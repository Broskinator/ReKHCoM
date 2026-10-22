#!/usr/bin/env python3
"""Make the HD sprites (port_hdsprites.c) from the player's Re:CoM: every row of port/data/hd_sprites.tsv names a
cartridge sprite (its ROM tiles, frame and box) and the recipe that turns one of Re:CoM's HD textures into its
picture, written to <data folder>/hd/sprites/<name>.png. Reference implementation of the launcher's importer
(port/decompui/hd_import.inc, HdSprites): the same operations, the same resampling.

Recipe operations, applied in order to the texture (RGBA):
  crop=x0,y0,x1,y1   keep this rectangle
  rot=90|180|270     turn counter-clockwise
  erase=x0,y0,x1,y1  make this rectangle transparent
  shift=x0,y0,x1,y1,dx  move this rectangle dx pixels sideways (what it leaves is transparent)
  tint=RRGGBB        multiply the colours
  canvas=w,h         start over from a transparent w x h picture (for blit)
  blit=sx0,sy0,sx1,sy1,dx0,dy0,dx1,dy1  paste that rectangle of the texture, resized, over the picture's rectangle
  place=x0,y0,x1,y1  the picture fills only this part of the sprite's box (GBA pixels); the rest is transparent
  scale=k            the output is the sprite's box at k times the GBA's pixels (last; required)

    python port/tools/khhd_sprites.py "<install>/Image/dt/Recom.hed" <data folder> [--only name,...]
"""
import argparse
import hashlib
import struct
import sys
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
import khhd_pkg  # noqa: E402

RECIPE = Path(__file__).resolve().parents[1] / 'data' / 'hd_sprites.tsv'


def recipe(path=RECIPE):
    rows = []
    for line in path.read_text(encoding='utf-8').splitlines():
        if line.strip() and not line.startswith('#'):
            f = line.split('\t')
            rows.append({'name': f[0], 'group': int(f[1]), 'frame': f[2], 'tiles': f[3],
                         'box': tuple(int(v) for v in f[4:8]), 'source': f[8], 'ops': f[9]})
    return rows


def decode(blob, index=0):
    """RGBA array of a DDS (uncompressed 32-bit), PNG or IMGZ/IMGD (index-th image) texture."""
    if blob[:4] == b'DDS ':
        h, w = struct.unpack_from('<II', blob, 12)
        flags, fourcc, bits, rm, gm, bm, am = struct.unpack_from('<I4sIIIII', blob, 80)
        if bits != 32 or fourcc != b'\0\0\0\0':
            raise ValueError(f'DDS format {fourcc} {bits} bits not handled')
        px = np.frombuffer(blob, dtype='<u4', count=w * h, offset=128).reshape(h, w)
        out = np.zeros((h, w, 4), dtype=np.uint8)
        for c, m in enumerate((rm, gm, bm, am)):
            if m:
                shift = (m & -m).bit_length() - 1
                out[:, :, c] = (px & m) >> shift
            else:
                out[:, :, c] = 255
        return out
    if blob[:4] == b'\x89PNG':
        import io
        return np.asarray(Image.open(io.BytesIO(blob)).convert('RGBA')).copy()
    if blob[:4] in (b'IMGZ', b'IMGD'):
        sys.path.insert(0, str(Path(__file__).resolve().parent))
        import khhd_imgz
        for i, img in khhd_imgz.imgd_images(blob):
            if i == index:
                return np.asarray(img).copy()
    raise ValueError('unknown texture')


def resample(img, w, h):
    """Separable resize with a triangle filter widened when shrinking (the launcher's ResizeRgba)."""
    def axis(a, n, ax):
        m = a.shape[ax]
        scale = m / n
        support = max(1.0, scale)
        res = np.zeros(a.shape[:ax] + (n,) + a.shape[ax + 1:], dtype=np.float64)
        for i in range(n):
            centre = (i + 0.5) * scale
            lo, hi = int(np.floor(centre - support)), int(np.ceil(centre + support))
            idx = np.arange(max(lo, 0), min(hi, m))
            wts = np.maximum(0.0, 1.0 - np.abs((idx + 0.5 - centre) / support))
            if wts.sum() == 0:
                idx, wts = np.array([min(max(int(centre), 0), m - 1)]), np.array([1.0])
            wts /= wts.sum()
            sl = [slice(None)] * a.ndim
            sl[ax] = i
            res[tuple(sl)] = np.tensordot(wts, np.take(a, idx, axis=ax), axes=([0], [ax]))
        return res
    f = img.astype(np.float64)
    f[:, :, :3] *= f[:, :, 3:4] / 255.0  # premultiplied: no dark fringes at transparent edges
    f = axis(axis(f, h, 0), w, 1)
    a = f[:, :, 3:4]
    rgb = np.where(a > 0, f[:, :, :3] * 255.0 / np.maximum(a, 1e-9), 0)
    return np.clip(np.concatenate([rgb, a], axis=2) + 0.5, 0, 255).astype(np.uint8)


def solo(img):
    """Keep only the largest 8-connected region of alpha > 32 (a glyph without its italic neighbours' corners)."""
    h, w = img.shape[:2]
    on = img[:, :, 3] > 32
    label = np.zeros((h, w), dtype=np.int32)
    best, best_size, n = 0, 0, 0
    for y0 in range(h):
        for x0 in range(w):
            if not on[y0, x0] or label[y0, x0]:
                continue
            n += 1
            label[y0, x0] = n
            stack, size = [(y0, x0)], 0
            while stack:
                y, x = stack.pop()
                size += 1
                for dy in (-1, 0, 1):
                    for dx in (-1, 0, 1):
                        yy, xx = y + dy, x + dx
                        if 0 <= yy < h and 0 <= xx < w and on[yy, xx] and not label[yy, xx]:
                            label[yy, xx] = n
                            stack.append((yy, xx))
            if size > best_size:
                best, best_size = n, size
    out = img.copy()
    # faint pixels (alpha <= 32) next to the kept glyph stay; others go
    keep = label == best
    near = np.zeros_like(keep)
    near[1:-1, 1:-1] = keep[:-2, :-2] | keep[:-2, 1:-1] | keep[:-2, 2:] | keep[1:-1, :-2] | keep[1:-1, 1:-1] | \
        keep[1:-1, 2:] | keep[2:, :-2] | keep[2:, 1:-1] | keep[2:, 2:]
    out[~(keep | (near & ~on))] = 0
    return out


def over(dst, src, x0, y0):
    """src pasted over dst at (x0, y0), alpha-blended (the launcher's BlitOver)."""
    h, w = src.shape[:2]
    out = dst.astype(np.float64)
    ys, xs = max(y0, 0), max(x0, 0)
    ye, xe = min(y0 + h, dst.shape[0]), min(x0 + w, dst.shape[1])
    if ye <= ys or xe <= xs:
        return dst
    s = src[ys - y0:ye - y0, xs - x0:xe - x0].astype(np.float64)
    d = out[ys:ye, xs:xe]
    sa, da = s[:, :, 3:4] / 255.0, d[:, :, 3:4] / 255.0
    oa = sa + da * (1 - sa)
    rgb = np.where(oa > 0, (s[:, :, :3] * sa + d[:, :, :3] * da * (1 - sa)) / np.maximum(oa, 1e-9), 0)
    out[ys:ye, xs:xe] = np.concatenate([rgb, oa * 255.0], axis=2)
    return np.clip(out + 0.5, 0, 255).astype(np.uint8)


def apply(img, ops, box):
    place = None
    source = img
    for op in ops.split(';'):
        key, _, val = op.partition('=')
        v = [int(x) if key != 'tint' else x for x in val.split(',')] if val else []
        if key == 'crop':
            img = img[v[1]:v[3], v[0]:v[2]]
        elif key == 'rot':
            img = np.rot90(img, v[0] // 90)
        elif key == 'erase':
            img = img.copy()
            img[v[1]:v[3], v[0]:v[2]] = 0
        elif key == 'shift':
            img = img.copy()
            part = img[v[1]:v[3], v[0]:v[2]].copy()
            img[v[1]:v[3], v[0]:v[2]] = 0
            x0 = v[0] + v[4]
            img[v[1]:v[3], max(x0, 0):min(x0 + part.shape[1], img.shape[1])] = \
                part[:, max(-x0, 0):part.shape[1] - max(x0 + part.shape[1] - img.shape[1], 0)]
        elif key == 'solo':
            img = solo(img)
        elif key == 'canvas':
            img = np.zeros((v[1], v[0], 4), dtype=np.uint8)
        elif key == 'blit':
            part = resample(source[v[1]:v[3], v[0]:v[2]], v[6] - v[4], v[7] - v[5])
            img = over(img, part, v[4], v[5])
        elif key == 'tint':
            c = int(v[0], 16)
            img = img.copy()
            for k, s in enumerate((16, 8, 0)):
                img[:, :, k] = (img[:, :, k].astype(np.uint32) * ((c >> s) & 255) // 255).astype(np.uint8)
        elif key == 'place':
            place = v
        elif key == 'scale':
            if op != ops.split(';')[-1]:
                raise ValueError('scale must be the last operation')
            k = v[0]
            out = np.zeros((box[3] * k, box[2] * k, 4), dtype=np.uint8)
            x0, y0, x1, y1 = place if place else (0, 0, box[2], box[3])
            out[y0 * k:y1 * k, x0 * k:x1 * k] = resample(img, (x1 - x0) * k, (y1 - y0) * k)
            return out
        else:
            raise ValueError(f'unknown operation {op}')
    raise ValueError('recipe without scale')


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('hed')
    ap.add_argument('data')
    ap.add_argument('--only')
    args = ap.parse_args()
    out = Path(args.data) / 'hd' / 'sprites'
    out.mkdir(parents=True, exist_ok=True)
    only = set(args.only.split(',')) if args.only else None
    idx = {m: o for m, o, s, r in khhd_pkg.entries(args.hed)}
    cache = {}
    done = 0
    with open(Path(args.hed).with_suffix('.pkg'), 'rb') as pkg:
        for row in recipe():
            if only and row['name'] not in only:
                continue
            asset, _, tex = row['source'].partition(':')
            tex, _, num = tex.partition('#')
            if asset not in cache:
                m = hashlib.md5(asset.encode()).hexdigest()
                cache[asset] = khhd_pkg.read_asset(pkg, idx[m], with_remastered=True)[2] if m in idx else {}
            blob = cache[asset].get(tex)
            if blob is None:
                print(row['name'], 'MISSING', row['source'])
                continue
            img = apply(decode(blob, int(num or 0)), row['ops'], row['box'])
            Image.fromarray(img, 'RGBA').save(out / f"{row['name']}.png", optimize=True)
            done += 1
    print(done, 'HD sprites ->', out)


if __name__ == '__main__':
    main()
