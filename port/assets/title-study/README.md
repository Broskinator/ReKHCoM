# Title extension — art study

Hand-drawn native-grid proposal for the missing Sora title-screen crystals.
Not loaded by the game and not included in a published release.

`title-extension-study.png` is a 320x240 RGBA **patch only**. The entire
original 240x160 rectangle is transparent. All 2,373 opaque pixels are new;
there are no extracted ROM pixels, partially transparent edges, upscaled
pixels or mirrored pieces. Colors match the private decoded reference.

The original artwork/characters belong to Disney/Square Enix. This study is
a fan-made imagined continuation, not recovered official art. Its recipe is
`port/tools/draw_title_extension.py`. No image generator produced this patch.

To reproduce privately (requires Pillow and a local decoded title reference):

```powershell
python port/tools/draw_title_extension.py build/title-widescreen-review/native-full.png --out build/title-widescreen-review/study
```

Optional `--mockup <local title screenshot>` produces a **static composition**
using the current full-height menu scale. It is not an in-engine screenshot
of the extension. The original screenshot's center remains unchanged.
Full previews contain original art and must remain under ignored `build/`.

Before integration: review the imagined glove/facets and seams; map colors to
the live ROM palette; honor title intro/fades/brightness; preserve every core
pixel and menu position; test different ratios and leaving/reopening title.
Do not stretch, mirror or rescale the core to make the patch fit.
