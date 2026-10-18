# Original title edge fade — visual study

The user rejected the imagined long crystal/glove. That proposal has been
removed. Do not integrate or restore it as a finished asset.

The current study fades only the ORIGINAL native title toward white at its
right and bottom cutoffs, inspired by the existing left edge. It changes
neither positions nor proportions, and reads no external illustration.
The user's new illustration is a visual guide only.

- `title-edge-fade-mask.png`: grayscale strength, 240x160. Contains no ROM art.
- `title-edge-fade-report.json`: checks from a private local preview.
- Recipe: `port/tools/fade_title_edges.py` (Pillow only).

```powershell
python port/tools/fade_title_edges.py build/title-widescreen-review/native-full.png --out build/title-widescreen-review/fade-study
```

The six-pixel transition quantizes to colors already in the original title.
Pixels outside the right/bottom strips remain byte-for-byte identical.
No external image sampling, mirrored fragments, extrapolated glove, image
replacement, Gaussian blur, geometry changes or fractional-grid drawing.
Private previews include original artwork and must remain in ignored build/.
Characters/original art belong to Disney/Square Enix.

This is a STATIC STUDY, not loaded by the game. Before integration, review
the transition width, then implement it on the title background only using
the live palette and fade state. Do not fade menu text, sprites or the UI.
Validate the intro, brightness, mode exit and each supported aspect ratio.
