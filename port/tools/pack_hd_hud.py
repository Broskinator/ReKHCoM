#!/usr/bin/env python3
"""Validate transparent HD HUD pieces and pack a padded RGBA PNG atlas.

Input JSON: {"scale": 4, "pieces": [{"id": "hp_frame", "file": "hp.png",
"width": 64, "height": 16}]}. Dimensions are original logical pixels;
files are relative to the input manifest. Output is a NEW directory.
This validates alpha/geometry, not artistic quality: review on dark/light
backgrounds before enabling a pack in the renderer.
"""
import argparse
import json
from pathlib import Path

from PIL import Image

PAD = 4
MAX_SIZE = 2048


def pack(manifest, output):
    spec = json.loads(manifest.read_text(encoding="utf-8"))
    scale = spec["scale"]
    if type(scale) is not int or not 1 <= scale <= 8:
        raise ValueError("scale must be an integer from 1 to 8")
    pieces = []
    names = set()
    for item in spec["pieces"]:
        name = item["id"]
        if not isinstance(name, str) or not name or name in names:
            raise ValueError("piece IDs must be unique nonempty strings")
        names.add(name)
        width, height = item["width"], item["height"]
        if any(type(n) is not int or n <= 0 for n in (width, height)):
            raise ValueError(f"{name}: logical dimensions must be positive integers")
        with Image.open(manifest.parent / item["file"]) as src:
            if src.format != "PNG" or src.mode != "RGBA":
                raise ValueError(f"{name}: requires an RGBA PNG with explicit alpha")
            if src.size != (width * scale, height * scale):
                raise ValueError(f"{name}: dimensions do not match logical size times scale")
            img = src.copy()
        if img.width + PAD * 2 > MAX_SIZE or img.height + PAD * 2 > MAX_SIZE:
            raise ValueError(f"{name}: exceeds atlas budget")
        alpha = img.getchannel("A")
        if alpha.getextrema()[1] == 0:
            raise ValueError(f"{name}: empty image")
        edges = [(0, 0, img.width, 1), (0, img.height - 1, img.width, img.height),
                 (0, 0, 1, img.height), (img.width - 1, 0, img.width, img.height)]
        if any(alpha.crop(edge).getextrema()[1] for edge in edges):
            raise ValueError(f"{name}: exterior border must be fully transparent")
        # Clear hidden RGB to avoid matte colours in transparent pixels.
        img.putdata([(r, g, b, a) if a else (0, 0, 0, 0) for r, g, b, a in img.getdata()])
        pieces.append((item, img))
    if not pieces:
        raise ValueError("manifest contains no pieces")
    x = y = row_height = used_width = 0
    placements = []
    for item, img in pieces:
        w, h = img.width + PAD * 2, img.height + PAD * 2
        if x + w > MAX_SIZE:
            x, y, row_height = 0, y + row_height, 0
        if y + h > MAX_SIZE:
            raise ValueError("pack exceeds a single 2048x2048 atlas (16 MiB RGBA)")
        placements.append((item, img, x + PAD, y + PAD))
        x += w
        used_width = max(used_width, x)
        row_height = max(row_height, h)
    atlas = Image.new("RGBA", (used_width, y + row_height))
    entries = []
    for item, img, px, py in placements:
        atlas.paste(img, (px, py))
        entries.append(dict(id=item["id"], rect=[px, py, img.width, img.height],
                            logical_size=[item["width"], item["height"]]))
    output.mkdir(parents=True, exist_ok=False)
    atlas.save(output / "hud.png", optimize=True)
    (output / "hud.json").write_text(json.dumps(dict(version=1, scale=scale,
        texture="hud.png", alpha="straight", padding=PAD, pieces=entries), indent=2) + "\n")
    # Inspection backgrounds reveal opaque checkerboards and colour fringes.
    for name, color in (("dark", (24, 24, 32, 255)), ("light", (240, 240, 240, 255))):
        review = Image.new("RGBA", atlas.size, color)
        review.alpha_composite(atlas)
        review.convert("RGB").save(output / f"review-{name}.png")
    return atlas.size


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    try:
        size = pack(args.manifest, args.output)
    except (ValueError, KeyError, OSError) as exc:
        parser.exit(1, f"Invalid HUD pack: {exc}\n")
    print(f"Packed {size[0]}x{size[1]} RGBA atlas ({size[0] * size[1] * 4} bytes decoded); visual review required")
