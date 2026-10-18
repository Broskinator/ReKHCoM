"""Recover the scene 061 tile study selected by the user, from original ROM tiles.

Private preview only. Requires the EU ROM, generated EU symbols and run1/0_061
captures. No screenshot supplied by the user is
used to construct the output. Run with: py -3 port/tools/scene061_tile_preview.py
"""
import json
import sys
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "build/scene-review/recovered061"
import scene_tile_data as sc

# These are the actual steps printed by the earlier, nearly complete preview.
# Units are 8x8 GBA tiles. The upper wall has its own period and direction.
STEPS = {
    "bg3": {"left": (4, 2), "right": (-4, 2), "top": (-4, 2), "bottom": (-4, -2)},
    "bg2": {"left": (12, 6), "right": (-12, -6), "top": (12, 6), "bottom": (-4, -2)},
}
CAMERA = (14, 55)
VIEW = (320, 180)


def entry(grid, x, y, steps, fix_corner):
    height, width = len(grid), len(grid[0])
    if not fix_corner:
        # Reproduce the original study exactly, including its corner cycle.
        for _ in range(512):
            if 0 <= x < width and 0 <= y < height:
                return grid[y][x]
            edge = ("left" if x < 0 else "right") if not 0 <= x < width else ("top" if y < 0 else "bottom")
            dx, dy = steps[edge]
            x, y = x + dx, y + dy
        return None

    # Preserve each edge's period. At the corner, mirror only the horizontal
    # component when the top/bottom step would leave through the crossed side.
    for _ in range(64):
        if 0 <= x < width:
            break
        dx, dy = steps["left" if x < 0 else "right"]
        x, y = x + dx, y + dy
    for _ in range(64):
        if 0 <= y < height:
            break
        dx, dy = steps["top" if y < 0 else "bottom"]
        if not 0 <= x + dx < width:
            dx = -dx
        x, y = x + dx, y + dy
    return grid[y][x] if 0 <= x < width and 0 <= y < height else None


def background(bg, fix_corner):
    image = Image.new("RGB", VIEW, (60, 60, 60))
    pixels = image.load()
    origin = (CAMERA[0] - 40, CAMERA[1] - 10)
    layers = [("bg3", bg["tiles"]), ("bg2", bg["tiles"]), ("bg1", bg["tiles2"])]
    for name, data in layers:
        grid = bg[name]
        if grid is None:
            continue
        for py in range(VIEW[1]):
            wy = origin[1] + py
            for px in range(VIEW[0]):
                wx = origin[0] + px
                tx, ty = wx // 8, wy // 8
                if 0 <= ty < len(grid) and 0 <= tx < len(grid[0]):
                    e = grid[ty][tx]
                elif name in STEPS:
                    e = entry(grid, tx, ty, STEPS[name], fix_corner)
                else:
                    continue
                if e is None:
                    continue
                sx, sy = wx % 8, wy % 8
                if e & 0x400:
                    sx = 7 - sx
                if e & 0x800:
                    sy = 7 - sy
                offset = (e & 0x3ff) * 32 + sy * 4 + sx // 2
                value = data[offset]
                color = value >> 4 if sx & 1 else value & 15
                if color or name == "bg3":
                    pixels[px, py] = bg["colors"][(e >> 12) * 16 + color]
    return image


def composite(before, after, bg):
    result = after.copy()
    original, output, rendered = before.load(), result.load(), bg.load()
    for y in range(VIEW[1]):
        for x in range(VIEW[0]):
            if original[x, y] == (0, 0, 0):
                output[x, y] = rendered[x, y]
    return result


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    captures = ROOT / "build/scene-review/run1/0_061"
    before = Image.open(captures / "before/shot_01000.bmp").convert("RGB")
    after = Image.open(captures / "after/shot_01000.bmp").convert("RGB")
    _, addr = sc.bg_symbol(61)
    bg = sc.load_bg(addr)
    recovered = composite(before, after, background(bg, False))
    fixed = composite(before, after, background(bg, True))
    for name, image in [("before", before), ("recovered", recovered), ("corner-fixed", fixed)]:
        image.save(OUT / f"061-{name}-native.png")
        image.resize((960, 540), Image.Resampling.NEAREST).save(OUT / f"061-{name}.png")
    changed = [(x, y) for y in range(180) for x in range(320)
               if recovered.getpixel((x, y)) != fixed.getpixel((x, y))]
    bounds = None if not changed else [min(x for x, y in changed), min(y for x, y in changed),
                                       max(x for x, y in changed) + 1, max(y for x, y in changed) + 1]
    metadata = {"event": 61, "capture": "shot_01000.bmp", "camera_gba_origin": CAMERA,
                "view": VIEW, "edge_steps_in_tiles": STEPS, "corner_changed_pixels": len(changed),
                "corner_changed_bounds": bounds, "source": "Original ROM tilemaps, tiles and palettes"}
    (OUT / "configuration.json").write_text(json.dumps(metadata, indent=2), encoding="utf-8")
    pair = Image.new("RGB", (1296, 392), (24, 27, 38))
    draw = ImageDraw.Draw(pair)
    for x, title, image in [(0, "ANTES", before), (656, "DESPUES - configuracion recuperada", fixed)]:
        draw.text((x + 4, 5), title, fill="white")
        pair.paste(image.resize((640, 360), Image.Resampling.NEAREST), (x, 24))
    pair.save(OUT / "061-before-after.png")
    print(json.dumps(metadata, indent=2))
    print(OUT)


if __name__ == "__main__":
    main()
