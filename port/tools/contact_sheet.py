#!/usr/bin/env python3
"""Tile build/pc/shot_*.bmp (from KHCOM_SHOTS) into one PNG for quick review."""
import sys
from pathlib import Path

from PIL import Image, ImageDraw

src = Path(sys.argv[1] if len(sys.argv) > 1 else "build/pc")
out = Path(sys.argv[2] if len(sys.argv) > 2 else src / "sheet.png")
cols = int(sys.argv[3]) if len(sys.argv) > 3 else 3
shots = sorted(src.glob("shot_*.bmp"))
if not shots:
    sys.exit("no shots")
ims = [Image.open(p).convert("RGB") for p in shots]
w, h = ims[0].size
scale = 2
rows = (len(ims) + cols - 1) // cols
sheet = Image.new("RGB", (cols * (w * scale + 6), rows * (h * scale + 20)), (30, 30, 30))
draw = ImageDraw.Draw(sheet)
for i, (p, im) in enumerate(zip(shots, ims)):
    x, y = (i % cols) * (w * scale + 6), (i // cols) * (h * scale + 20)
    sheet.paste(im.resize((im.width * scale, im.height * scale), Image.NEAREST), (x, y + 16))
    draw.text((x + 2, y + 2), p.stem, fill=(255, 255, 0))
sheet.save(out)
print(out)
