"""Approved improvement: complete beach grass across both original map boundaries.

Right patch: nine grass-bearing entries match a full original patch at offset
(+15,-4). Left patch: its generated half came from (+8,-4), so complete the
other half inside the old map with the same source offset. Keep wall overlays.
"""
import json

from PIL import Image, ImageDraw

import scene057_tile_preview as review

OUT = review.renderer.ROOT / "build/scene-review/polish057"


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    renderer = review.renderer
    _, address = renderer.sc.bg_symbol(57)
    bg = renderer.sc.load_bg(address)
    bg["colors"] = renderer.sc.active_bg_palette(renderer.ROOT / "build/scene-review/camera057/st.khst")
    original_floor = [row[:] for row in bg["bg3"]]
    # Only floor below the house; don't replace structural wood tiles in row17.
    for y in range(18, 23):
        for x in range(0, 5):
            bg["bg3"][y][x] = original_floor[y - 4][x + 8]
    saved = (renderer.CAMERA, renderer.STEPS, renderer.entry)
    original_entry = renderer.entry

    def source(grid, x, y, steps, fix_corner):
        if grid is bg["bg3"] and 32 <= x <= 35 and 5 <= y <= 11:
            return original_floor[y + 4][x - 15]
        if grid is bg["bg2"] and x >= 32:
            return 0
        # All extension sources refer to untouched originals, never patched maps.
        raw_grid = original_floor if grid is bg["bg3"] else grid
        return original_entry(raw_grid, x, y, steps, fix_corner)

    renderer.CAMERA, renderer.STEPS, renderer.entry = review.CAMERA, review.STEPS, source
    try:
        background = renderer.background(bg, True)
    finally:
        renderer.CAMERA, renderer.STEPS, renderer.entry = saved
    baseline = Image.open(renderer.ROOT / "build/scene-review/accepted057/057-proposal-native.png").convert("RGB")
    after = baseline.copy()
    for y in range(180):
        for x in range(320):
            tx, ty = (x - 33) // 8, (y + 33) // 8
            if (0 <= tx <= 4 and 18 <= ty <= 22) or (32 <= tx <= 35 and 5 <= ty <= 11):
                after.putpixel((x, y), background.getpixel((x, y)))
    for name, image in [("baseline", baseline), ("proposal", after)]:
        image.save(OUT / f"057-{name}-native.png")
        image.resize((960, 540), Image.Resampling.NEAREST).save(OUT / f"057-{name}.png")
    pair = Image.new("RGB", (1296, 392), (24, 27, 38))
    d = ImageDraw.Draw(pair)
    for x, title, image in [(0, "057 BASE ACEPTADA", baseline), (656, "PRUEBA - PASTO COMPLETO", after)]:
        d.text((x + 4, 5), title, fill="white")
        pair.paste(image.resize((640, 360), Image.Resampling.NEAREST), (x, 24))
    pair.save(OUT / "057-polish-before-after.png")
    details = Image.new("RGB", (1216, 360), (24, 27, 38))
    d = ImageDraw.Draw(details)
    for column, (box, label) in enumerate([((0, 95, 92, 151), "IZQUIERDA"), ((250, 0, 320, 80), "DERECHA")]):
        for row, (name, image) in enumerate([("ANTES", baseline), ("DESPUES", after)]):
            x, y = column * 608, row * 180
            d.text((x + 3, y + 3), label + " " + name, fill="white")
            crop = image.crop(box)
            factor = 2
            details.paste(crop.resize((crop.width * factor, crop.height * factor), Image.Resampling.NEAREST), (x + 3, y + 20))
    details.save(OUT / "057-grass-details.png")
    changes = [(x, y) for y in range(180) for x in range(320) if baseline.getpixel((x, y)) != after.getpixel((x, y))]
    info = {"status": "accepted_improved", "event": 57, "base_commit": "c924ff625",
            "overrides": [
                {"layer": "bg3", "target_tiles": [0, 18, 5, 23], "source_offset": [8, -4]},
                {"layer": "bg3", "target_tiles": [32, 5, 36, 12], "source_offset": [-15, 4], "matching_grass_entries": 9}],
            "changed_pixels": len(changes),
            "changed_inside_original_gba": sum(40 <= x < 280 and 10 <= y < 170 for x, y in changes)}
    (OUT / "configuration.json").write_text(json.dumps(info, indent=2), encoding="utf-8")
    print(json.dumps(info, indent=2))


if __name__ == "__main__":
    main()
