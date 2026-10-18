"""Scene 074 review: continue only the real occupied block to right/bottom.

The 512x512 declaration includes three default empty map blocks. This scene's
content occupies the first 256x256 block; its internal authored void is retained.
Uses the shared ROM decoder and scene 061 pixel renderer, with a separate profile.
"""
import json
import struct
from collections import Counter

from PIL import Image, ImageDraw

import scene061_tile_preview as renderer

OUT = renderer.ROOT / "build/scene-review/review074"

# Layout praised by the user. Preserve the source coordinates while fixing
# presentation; palette effects must never trigger a new layout search.
APPROVED_LAYOUT_STEPS = {
    "bg3": {"right": [-4, -2], "bottom": [4, -2], "left": [2, 1], "top": [2, 1]},
    "bg2": {"right": [-4, -2], "bottom": [-2, -1], "left": [2, 1], "top": [2, 1]},
    "bg1": {"right": [-4, -2], "bottom": [-4, -2], "left": [2, 1], "top": [2, 1]},
}


def frame_palette():
    # The frame100 screenshot contains the rendered frame99 palette. A state
    # requested at 100 already contains the next palette update (one step later).
    # This is the hardware palette, not BLDCNT brightness: all blend registers
    # are zero in this frame. Do not bake a constant RGB floor into game assets.
    path = renderer.ROOT / "build/scene-review/palette074/frame99.khst"
    return renderer.sc.active_bg_palette(path)


def select_steps(grid):
    height, width = len(grid), len(grid[0])
    freq = Counter(e for row in grid for e in row)
    result = {}
    ranges = {"right": (range(width - 12, width), range(height), -1, 0),
              "bottom": (range(width), range(height - 6, height), 0, -1)}
    for edge, (xs, ys, ix, iy) in ranges.items():
        choices = []
        for k in range(1, 9):
            for sign in (-1, 1):
                dx, dy = (ix * 2 * k, sign * k) if ix else (sign * 2 * k, iy * k)
                hit, total = 0.0, 0.0
                for y in ys:
                    for x in xs:
                        nx, ny = x + dx, y + dy
                        if 0 <= nx < width and 0 <= ny < height:
                            weight = 1.0 / freq[grid[y][x]]
                            total += weight
                            if grid[y][x] == grid[ny][nx]:
                                hit += weight
                if total:
                    choices.append((hit / total, dx, dy))
        best = max(c[0] for c in choices)
        choice = min((c for c in choices if c[0] >= best - 0.03), key=lambda c: abs(c[2]))
        result[edge] = [choice[1], choice[2]]
    # Never used: these sides are deliberately closed.
    result.update(left=[2, 1], top=[2, 1])
    return result


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    _, address = renderer.sc.bg_symbol(74)
    bg = renderer.sc.load_bg(address)
    loaded_palette_entries = struct.unpack("<H", renderer.sc.at(address + 28, 2))[0] // 2
    rom_colors = bg["colors"][:]
    bg["colors"] = frame_palette()
    original_dimensions = [bg["w"] * 256, bg["h"] * 256]
    for name in ("bg3", "bg2", "bg1"):
        if bg[name] is not None:
            bg[name] = [row[:32] for row in bg[name][:32]]
    steps = APPROVED_LAYOUT_STEPS
    saved = (renderer.CAMERA, renderer.STEPS, renderer.entry)
    original_entry = renderer.entry

    def route(grid, x, y, layer_steps, fix_corner):
        if x < 0 or y < 0:
            return None
        return original_entry(grid, x, y, layer_steps, fix_corner)

    renderer.CAMERA, renderer.STEPS, renderer.entry = (0, 0), steps, route
    try:
        background = renderer.background(bg, True)
    finally:
        renderer.CAMERA, renderer.STEPS, renderer.entry = saved
    path = renderer.ROOT / "build/scene-review/run1/0_074/before/shot_00100.bmp"
    before = Image.open(path).convert("RGB")
    after = before.copy()
    for y in range(180):
        for x in range(320):
            wx, wy = x - 40, y - 10
            if wx >= 0 and wy >= 0 and (wx >= 256 or wy >= 256):
                after.putpixel((x, y), background.getpixel((x, y)))
    for label, image in [("before", before), ("proposal", after)]:
        image.save(OUT / f"074-{label}-native.png")
        image.resize((960, 540), Image.Resampling.NEAREST).save(OUT / f"074-{label}.png")
    pair = Image.new("RGB", (1296, 392), (24, 27, 38))
    draw = ImageDraw.Draw(pair)
    for x, title, image in [(0, "074 MONSTRUO - ANTES", before), (656, "PROPUESTA - solo derecha/abajo", after)]:
        draw.text((x + 4, 5), title, fill="white")
        pair.paste(image.resize((640, 360), Image.Resampling.NEAREST), (x, 24))
    pair.save(OUT / "074-before-after.png")
    info = {"event": 74, "status": "accepted_preliminary", "camera": [0, 0],
            "declared_dimensions": original_dimensions, "content_dimensions": [256, 256],
            "edges": ["right", "bottom"], "layer_steps": steps,
            "palette_source": "palette074/frame99.khst (active BG palette for shot_00100)",
            "palette_entries_different_from_rom": sum(rom_colors[i] != bg["colors"][i] for i in range(loaded_palette_entries)),
            "changed_pixels": sum(a != b for a, b in zip(before.getdata(), after.getdata()))}
    (OUT / "configuration.json").write_text(json.dumps(info, indent=2), encoding="utf-8")
    print(json.dumps(info, indent=2))


if __name__ == "__main__":
    main()
