"""Study of a native-grid white fade on the ORIGINAL title artwork.

No external illustration is read, projected, resized or composited. The
published asset is only a grayscale strength mask, never extracted ROM art.
Private previews use the original palette and remain under ignored build/.
This tool does not integrate the effect into the game.
"""
import argparse
import json
from pathlib import Path

from PIL import Image


CORE = (240, 160)


def pixels(image):
    if hasattr(image, "get_flattened_data"):
        return image.get_flattened_data()
    return image.getdata()


def smoothstep(t):
    t = min(1.0, max(0.0, t))
    return t * t * (3 - 2 * t)


def fade(original, width=6):
    if not 1 <= width <= 12:
        raise ValueError("fade width must be 1..12 native pixels")
    original = original.convert("RGB").crop((0, 0, *CORE))
    palette = set(pixels(original))
    palette.discard((0, 255, 255))  # inspection transparency marker, not art
    palette = sorted(palette)
    if (255, 255, 255) not in palette:
        raise ValueError("expected the original title's white background")
    mask = Image.new("L", CORE)
    preview = original.copy()
    changed = 0
    for y in range(CORE[1]):
        for x in range(CORE[0]):
            right = smoothstep((x - (CORE[0] - 1 - width)) / width)
            bottom = smoothstep((y - (CORE[1] - 1 - width)) / width)
            strength = round(255 * (1 - (1 - right) * (1 - bottom)))
            mask.putpixel((x, y), strength)
            if not strength:
                continue
            c = original.getpixel((x, y))
            target = [v + (255 - v) * strength / 255 for v in c]
            q = min(palette, key=lambda v: sum((v[i] - target[i]) ** 2 for i in range(3)))
            preview.putpixel((x, y), q)
            changed += q != c
    protected = (0, 0, CORE[0] - width, CORE[1] - width)
    assert preview.crop(protected).tobytes() == original.crop(protected).tobytes()
    assert all(preview.getpixel((239, y)) == (255, 255, 255) for y in range(160))
    assert all(preview.getpixel((x, 159)) == (255, 255, 255) for x in range(240))
    assert set(pixels(preview)) <= set(palette)
    report = {"status": "native-fade-study-only", "width_native_pixels": width,
              "source": "original native title only", "external_art_sampled": False,
              "protected_rect": list(protected), "protected_pixels_identical": True,
              "geometry_changed": False, "palette_preserved": True,
              "right_and_bottom_end_in_white": True, "changed_pixels": changed}
    return mask, preview, report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original", type=Path, help="private decoded native title image")
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--width", type=int, default=6)
    args = parser.parse_args()
    original = Image.open(args.original)
    if original.width < 240 or original.height < 160:
        parser.error("expected at least the original 240x160 rectangle")
    mask, preview, report = fade(original, args.width)
    args.out.mkdir(parents=True, exist_ok=True)
    mask.save(args.out / "title-edge-fade-mask.png")
    preview.save(args.out / "title-original-faded.png")
    canvas = Image.new("RGB", (284, 160), "white")
    canvas.paste(preview, (22, 0))
    canvas.resize((1420, 800), Image.Resampling.NEAREST).save(args.out / "title-original-faded-preview.png")
    (args.out / "title-edge-fade-report.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report))


if __name__ == "__main__":
    main()
