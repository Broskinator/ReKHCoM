"""Hand-authored title crystal study, drawn on the native one-pixel grid.

The input is a PRIVATE local decoded reference, never a bundled game image.
Only the newly drawn pixels are exported as an RGBA patch; the original
240x160 rectangle stays entirely transparent. This is an art proposal, not
a renderer hook or a replacement for the original ROM artwork.
"""
import argparse
import json
from pathlib import Path

from PIL import Image, ImageDraw


SIZE = (320, 240)
CORE = (240, 160)
CYAN = (0, 255, 255)


def pixels(image):
    # Pillow 14 removes getdata(); retain compatibility with older toolchains.
    if hasattr(image, "get_flattened_data"):
        return image.get_flattened_data()
    return image.getdata()


def draw(reference):
    palette = sorted(set(pixels(reference.crop((0, 0, *CORE)))) - {CYAN})

    def color(rgb):
        return min(palette, key=lambda p: sum((a-b)**2 for a, b in zip(p, rgb)))

    # Warm, slightly desaturated crystal ramp sampled from the existing art.
    colors = {
        "paper": (255, 255, 255), "chalk": (239, 232, 215),
        "ivory": (222, 210, 180), "sand": (194, 178, 140),
        "stone": (155, 143, 115), "taupe": (125, 113, 91),
        "slate": (85, 84, 70), "smoke": (64, 62, 52),
        "shadow": (39, 34, 25), "ink": (15, 13, 11),
        "violet": (115, 99, 117), "violet_dark": (87, 78, 91),
        "ochre": (164, 114, 56), "gold": (201, 148, 69),
        "cream": (245, 229, 193),
    }
    colors = {k: color(v) + (255,) for k, v in colors.items()}
    art = Image.new("RGBA", SIZE)
    pencil = ImageDraw.Draw(art)

    def poly(tone, points):
        pencil.polygon(points, fill=colors[tone])

    def line(tone, points, width=1):
        pencil.line(points, fill=colors[tone], width=width)

    # Uppermost cropped splinter. Its point remains off the top of the canvas;
    # the lower edge closes naturally rather than copying an edge column.
    poly("chalk", [(229, -7), (242, -2), (249, 13), (243, 19), (235, 5)])
    poly("sand", [(236, -3), (241, 0), (246, 12), (242, 16), (239, 7)])
    poly("slate", [(238, -3), (241, 0), (244, 8), (242, 11)])
    line("ivory", [(242, 1), (245, 9), (247, 13), (243, 17)])

    # Donald's tall crystal: a single enclosing silhouette with different
    # lit bevels and dark faces, not a reflected duplicate of the portrait.
    poly("chalk", [(219, 79), (225, 85), (247, 119), (258, 141),
                   (270, 176), (266, 184), (249, 198), (239, 190),
                   (215, 163), (207, 145)])
    poly("ivory", [(220, 82), (243, 117), (256, 142), (266, 177),
                   (263, 183), (249, 194), (240, 186), (215, 158)])
    poly("stone", [(222, 87), (241, 116), (253, 141), (263, 177),
                   (250, 190), (240, 181), (216, 154)])
    poly("slate", [(221, 89), (239, 113), (250, 137), (259, 171),
                   (247, 184), (231, 170), (215, 149)])
    # Narrow right side of the glass, with an angular violet reflection.
    poly("smoke", [(239, 113), (248, 132), (259, 168), (254, 174),
                   (247, 151), (242, 135)])
    poly("violet_dark", [(240, 119), (247, 131), (253, 153),
                         (248, 156), (241, 139)])
    poly("violet", [(240, 124), (245, 133), (248, 143), (244, 141)])
    poly("taupe", [(248, 132), (252, 142), (258, 162), (256, 166),
                   (250, 146)])
    # Two separate rim highlights, with no continuous pillow-shaded band.
    line("chalk", [(248, 126), (254, 140), (258, 152)])
    line("sand", [(259, 159), (264, 175), (261, 182), (250, 191)])
    line("chalk", [(266, 177), (263, 183), (250, 194)])
    line("paper", [(268, 177), (265, 184)])

    # The glove crosses the old right boundary. The fingers form three
    # small steps and fold into the cuff; no additional face is invented.
    poly("shadow", [(235, 134), (241, 134), (245, 137), (247, 141),
                    (246, 146), (250, 150), (250, 155), (246, 159),
                    (241, 157), (237, 151), (234, 143)])
    poly("stone", [(237, 135), (241, 135), (244, 138), (245, 143),
                   (244, 146), (248, 151), (247, 155), (243, 154),
                   (239, 149), (237, 143)])
    poly("chalk", [(236, 136), (240, 136), (242, 138), (242, 142),
                   (240, 143), (242, 147), (246, 151), (246, 153),
                   (242, 152), (238, 147), (236, 142)])
    poly("cream", [(239, 137), (241, 138), (241, 141), (239, 141)])
    line("taupe", [(241, 143), (243, 145), (243, 148)])
    line("ivory", [(244, 148), (247, 151), (247, 153)])
    poly("ochre", [(242, 155), (247, 156), (249, 161), (245, 163), (241, 158)])
    line("gold", [(243, 156), (246, 157), (247, 160)])
    poly("shadow", [(242, 162), (248, 161), (252, 170), (249, 175), (244, 171)])
    poly("smoke", [(245, 164), (248, 164), (250, 170), (248, 172)])

    # Finish the lower cut-off black silhouette and crystal faces. These
    # converge to points; the future taller view has a deliberate endpoint.
    poly("ink", [(207, 150), (222, 153), (230, 153), (237, 161),
                 (241, 171), (248, 182), (243, 181), (232, 174),
                 (222, 168), (215, 161)])
    poly("shadow", [(221, 155), (229, 156), (235, 162), (237, 170),
                    (231, 168), (225, 164)])
    poly("taupe", [(216, 158), (223, 164), (230, 173), (240, 185),
                   (249, 194), (243, 191), (230, 180), (219, 168)])
    poly("sand", [(218, 162), (224, 168), (230, 178), (242, 190),
                  (238, 188), (225, 176)])
    line("chalk", [(218, 165), (227, 177), (240, 190), (248, 196)])
    poly("smoke", [(236, 177), (246, 183), (253, 181), (251, 189), (248, 191)])
    line("ivory", [(251, 191), (257, 186), (262, 180)])

    # A small broken facet beneath the collage, with generous white space.
    poly("chalk", [(231, 191), (239, 199), (235, 212), (230, 203)])
    poly("ivory", [(232, 194), (236, 200), (234, 207), (232, 202)])
    poly("stone", [(233, 197), (235, 200), (234, 204)])

    # Absolute protection: drawn shapes may cross the seam to help the
    # artist construct forms, but not one original pixel ships in the patch.
    art.paste((0, 0, 0, 0), (0, 0, *CORE))
    preview = Image.new("RGB", SIZE, (255, 255, 255))
    preview.paste(art, mask=art.getchannel("A"))
    preview.paste(reference.crop((0, 0, *CORE)), (0, 0))
    assert preview.crop((0, 0, *CORE)).tobytes() == reference.crop((0, 0, *CORE)).tobytes()
    assert art.crop((0, 0, *CORE)).getchannel("A").getextrema() == (0, 0)
    assert set(pixels(art.getchannel("A"))) <= {0, 255}
    assert all(rgb[:3] in palette for rgb in pixels(art) if rgb[3])
    return art, preview


def mockup(screenshot, patch):
    """Static composition only, NOT evidence of in-engine integration."""
    result = screenshot.copy().convert("RGB")
    scale = min(result.width / CORE[0], result.height / CORE[1])
    ox = int((result.width - CORE[0] * scale) / 2)
    oy = int((result.height - CORE[1] * scale) / 2)
    for y in range(result.height):
        for x in range(max(0, int(ox + CORE[0] * scale)), result.width):
            px, py = int((x - ox) / scale), int((y - oy) / scale)
            if not (CORE[0] <= px < patch.width and 0 <= py < patch.height):
                continue
            c = patch.getpixel((px, py))
            if c[3]:
                result.putpixel((x, y), c[:3])
    core = (ox, oy, int(ox + CORE[0] * scale), int(oy + CORE[1] * scale))
    assert result.crop(core).tobytes() == screenshot.convert("RGB").crop(core).tobytes()
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reference", type=Path)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--mockup", type=Path, help="optional private title screenshot; outputs a static mockup")
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    reference = Image.open(args.reference).convert("RGB")
    if reference.width < CORE[0] or reference.height < CORE[1]:
        parser.error("reference must include the original 240x160 core")
    patch, preview = draw(reference)
    patch.save(args.out / "title-extension-study.png")
    preview.save(args.out / "title-extension-preview.png")
    preview.resize((1280, 960), Image.Resampling.NEAREST).save(args.out / "title-extension-preview-4x.png")
    if args.mockup:
        mockup(Image.open(args.mockup), patch).save(args.out / "title-extension-16x9-mockup.png")
    report = {"status": "art-study-only", "canvas": list(SIZE),
              "original_pixels_preserved": 240 * 160, "core_patch_transparent": True,
              "alpha": "binary", "colors": "original reference palette only",
              "new_pixels": sum(a == 255 for a in pixels(patch.getchannel("A")))}
    (args.out / "title-extension-report.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report))


if __name__ == "__main__":
    main()
