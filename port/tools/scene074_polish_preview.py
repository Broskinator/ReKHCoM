"""Approved polish: complete the right floor ornament with its original left counterpart.

Nineteen nontransparent tile entries already visible on the right match the
left ornament exactly at offset (+20,-1) tiles. Only continue that same piece;
do not let the generic wall/floor repeat create another partial ornament.
"""
import json

from PIL import Image, ImageDraw

import scene074_tile_preview as review

OUT = review.renderer.ROOT / "build/scene-review/polish074"


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    original_entry = review.renderer.entry
    original_out = review.OUT

    def ornament_source(grid, x, y, steps, fix_corner):
        if steps is review.APPROVED_LAYOUT_STEPS["bg2"] and 32 <= x <= 34 and 17 <= y <= 21:
            return grid[y + 1][x - 20]
        return original_entry(grid, x, y, steps, fix_corner)

    review.OUT = OUT
    review.renderer.entry = ornament_source
    try:
        review.main()
    finally:
        review.OUT = original_out
        review.renderer.entry = original_entry
    before = Image.open(review.renderer.ROOT / "build/scene-review/accepted074/074-proposal-native.png").convert("RGB")
    after = Image.open(OUT / "074-proposal-native.png").convert("RGB")
    pair = Image.new("RGB", (1296, 392), (24, 27, 38))
    draw = ImageDraw.Draw(pair)
    for x, title, image in [(0, "074 BASE PRELIMINAR ACEPTADA", before), (656, "PRUEBA - pieza inferior completada", after)]:
        draw.text((x + 4, 5), title, fill="white")
        pair.paste(image.resize((640, 360), Image.Resampling.NEAREST), (x, 24))
    pair.save(OUT / "074-polish-before-after.png")
    closeup = Image.new("RGB", (1024, 206), (24, 27, 38))
    d = ImageDraw.Draw(closeup)
    for x, title, image in [(0, "ANTES", before), (520, "DESPUES", after)]:
        d.text((x + 3, 3), title, fill="white")
        closeup.paste(image.crop((250, 136, 320, 180)).resize((490, 176), Image.Resampling.NEAREST), (x, 22))
    closeup.save(OUT / "074-lower-detail.png")
    metadata = json.loads((OUT / "configuration.json").read_text())
    metadata.update(status="accepted_improved", preliminary_commit="8cf93bc78",
                    override_layer="bg2", target_tile_bounds=[32, 17, 35, 22],
                    source_offset=[-20, 1], matching_original_tile_entries=19,
                    changed_from_preliminary=sum(before.getpixel((x, y)) != after.getpixel((x, y))
                                                 for y in range(180) for x in range(320)))
    (OUT / "configuration.json").write_text(json.dumps(metadata, indent=2), encoding="utf-8")
    print("Trial metadata:", json.dumps(metadata))


if __name__ == "__main__":
    main()
