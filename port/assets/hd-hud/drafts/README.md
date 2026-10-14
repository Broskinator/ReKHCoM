# Drafts — not loaded or packaged

The Sora normal portrait is a generated reference only. Its isolated placement
was rejected by the user: head and surround must be composed with the complete
player HUD before integration. Do not count it as integrated portrait coverage.
Player HP frames, all bars, rim, Sora/Riku expressions and native anchors remain
pending in HUD_HD_INVENTARIO.md.

Generated from the local original portrait and battle reference; second pass
preserved the face/hair/rim and cleaned the transparent exterior with padding.
No runtime path references this draft.

`sora-player-assembly-v1.png` is the first complete composition reference
(portrait, rim and both bars). It is not a runtime replacement: the generated
bar offsets must be fitted against the native object geometry first. Fills
must remain dynamic; never bake these full sample bars into gameplay.

Native Sora capture at frame380: portrait box (1,-7,32,32), frame pieces
(4,2,32,8), (36,2,32,8), (60,2,32,8), (4,10,32,16); live fill object
(30,1,64,16), affine. Riku portrait box is (0,-7,32,32).
Coordinates are HUD-native units, including transparent margins; they are
not equivalent to visible ink bounds. Reproduce with capture_hud_layout.py.
