"""Fresh beach proposal, using only original map data and the original capture.

No historical beach 'after' image is loaded. The seam of the orange building
rises to the right: source steps must follow that seam, not the repeated sand.
"""
import json

from PIL import Image, ImageDraw

import scene061_tile_preview as renderer

OUT = renderer.ROOT / "build/scene-review/review057"
CAMERA = (7, 43)
# Deduced from the original building's diagonal and structural repetition.
STEPS = {
    "bg3": {"left": (8, -4), "right": (-8, 4), "top": (8, 4), "bottom": (-8, -4)},
    "bg2": {"left": (8, -4), "right": (-8, 4), "top": (8, 4), "bottom": (-8, -4)},
}


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    _, address = renderer.sc.bg_symbol(57)
    bg = renderer.sc.load_bg(address)
    bg["colors"] = renderer.sc.active_bg_palette(renderer.ROOT / "build/scene-review/camera057/st.khst")
    original_entry = renderer.entry
    saved = (renderer.CAMERA, renderer.STEPS, renderer.entry)

    def source(grid, x, y, steps, fix_corner):
        # To the right of the original building the visible view is below its
        # foundation: sand continues; don't copy the original unique tree/vines.
        if grid is bg["bg2"] and x >= 32:
            return 0
        return original_entry(grid, x, y, steps, fix_corner)

    renderer.CAMERA, renderer.STEPS, renderer.entry = CAMERA, STEPS, source
    try:
        background = renderer.background(bg, True)
    finally:
        renderer.CAMERA, renderer.STEPS, renderer.entry = saved
    before = Image.open(renderer.ROOT / "build/scene-review/run1/0_057/before/shot_00100.bmp").convert("RGB")
    after = before.copy()
    origin = (CAMERA[0] - 40, CAMERA[1] - 10)
    for y in range(180):
        for x in range(320):
            wx, wy = origin[0] + x, origin[1] + y
            if wx < 0 or wx >= 256 or wy < 0 or wy >= 256:
                after.putpixel((x, y), background.getpixel((x, y)))
    for name, image in [("before", before), ("proposal", after)]:
        image.save(OUT / f"057-{name}-native.png")
        image.resize((960, 540), Image.Resampling.NEAREST).save(OUT / f"057-{name}.png")
    pair = Image.new("RGB", (1296, 392), (24, 27, 38))
    d = ImageDraw.Draw(pair)
    for x, title, image in [(0, "057 PLAYA - ORIGINAL", before), (656, "PROPUESTA NUEVA", after)]:
        d.text((x + 4, 5), title, fill="white")
        pair.paste(image.resize((640, 360), Image.Resampling.NEAREST), (x, 24))
    pair.save(OUT / "057-before-after.png")
    info = {"event": 57, "status": "accepted_improvable", "camera": CAMERA,
            "steps": STEPS, "references": "original tilemaps + original before capture only",
            "bg2_right": "transparent original entry; no cloned unique vines"}
    (OUT / "configuration.json").write_text(json.dumps(info, indent=2), encoding="utf-8")
    print(json.dumps(info, indent=2))


if __name__ == "__main__":
    main()
