"""Regenerate scene 061 polish approved after commit a3ff671f6.

All replacements select original ROM tilemap entries; no screenshot painting.
The accepted baseline remains in build/scene-review/accepted061.
"""
import json
from pathlib import Path

from PIL import Image, ImageDraw

import scene061_tile_preview as base

OUT = base.ROOT / "build/scene-review/polish061"
WALL_SOURCES = [(16, 0), (17, 0), (2, 0), (3, 0), (16, 0)]


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    _, addr = base.sc.bg_symbol(61)
    bg = base.sc.load_bg(addr)
    originals = {name: [row[:] for row in bg[name]] for name in ("bg3", "bg2")}
    # The left extension cut a large floor ornament at the original map edge.
    # Complete it with a whole 9x5-tile piece of another original floor ornament.
    for y in range(3, 8):
        for x in range(0, 4):
            bg["bg3"][y][x] = originals["bg3"][y + 6][x + 12]
            bg["bg2"][y][x] = originals["bg2"][3][0]  # original transparent tile

    original_entry = base.entry

    def source(grid, x, y, steps, fix_corner):
        if grid is bg["bg3"] and -5 <= x < 0 and 3 <= y <= 7:
            return originals["bg3"][y + 6][x + 12]
        if grid is bg["bg2"] and -5 <= x < 0 and 3 <= y <= 7:
            return originals["bg2"][3][0]
        if grid is bg["bg2"] and 32 <= x <= 36 and y == 5:
            sx, sy = WALL_SOURCES[x - 32]
            return originals["bg2"][sy][sx]
        original_grid = originals["bg3"] if grid is bg["bg3"] else originals["bg2"] if grid is bg["bg2"] else grid
        return original_entry(original_grid, x, y, steps, fix_corner)

    base.entry = source
    try:
        rendered = base.background(bg, True)
    finally:
        base.entry = original_entry
    captures = base.ROOT / "build/scene-review/run1/0_061"
    before = Image.open(captures / "before/shot_01000.bmp").convert("RGB")
    after = Image.open(captures / "after/shot_01000.bmp").convert("RGB")
    result = base.composite(before, after, rendered)
    # The completed diamond spans the old map edge. These pixels come from the
    # updated BG3 tilemap, with BG2 rendered over it; this area contains no actors.
    for y in range(180):
        for x in range(320):
            tx, ty = (x - 26) // 8, (y + 45) // 8
            if -5 <= tx <= 3 and 3 <= ty <= 7:
                result.putpixel((x, y), rendered.getpixel((x, y)))
    result.save(OUT / "061-approved-polish-native.png")
    result.resize((960, 540), Image.Resampling.NEAREST).save(OUT / "061-approved-polish.png")
    baseline = Image.open(base.ROOT / "build/scene-review/accepted061/061-corner-fixed-native.png").convert("RGB")
    changes = [(x, y) for y in range(180) for x in range(320)
               if result.getpixel((x, y)) != baseline.getpixel((x, y))]
    report = {"event": 61, "status": "accepted_improved", "previous_accepted_commit": "a3ff671f6",
              "floor_target_tiles": [-5, 3, 3, 7], "floor_source_offset": [12, 6],
              "wall_target_tiles": [[x, 5] for x in range(32, 37)],
              "wall_source_tiles": WALL_SOURCES, "changed_pixels_from_base": len(changes),
              "changed_inside_original_gba_view": sum(40 <= x < 280 and 10 <= y < 170 for x, y in changes)}
    (OUT / "approved-configuration.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
    pair = Image.new("RGB", (1296, 392), (24, 27, 38))
    draw = ImageDraw.Draw(pair)
    for x, title, image in [(0, "ANTES - mapa original", before), (656, "DESPUES - 061 aceptada mejorada", result)]:
        draw.text((x + 4, 5), title, fill="white")
        pair.paste(image.resize((640, 360), Image.Resampling.NEAREST), (x, 24))
    pair.save(OUT / "061-approved-before-after.png")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
