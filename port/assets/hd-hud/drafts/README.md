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

`sora-player-assembly-v2.png` joins the two white/silver bar contours using a
continuous shared separator, following user feedback. Other shapes remain
the approved composition reference. `riku-player-assembly-v1.png` supplies
the matching preliminary Riku composition. These full-bar images are still
references, not runtime textures. Split the frame/portrait from sample fills
before use; native HP is authoritative for every fill width and palette.

Runtime preparation now smooths the player's live HP fill directly from its
native affine sprite, including its visible mask, palette rows and OAM order.
That does not complete the player frame/portrait family or extra-bar coverage.
The generated attack-card candidate was rejected for pixelated contours and
background glow; it is not part of the pack.

`player-double-frame-v1.png` separates the connected metallic frame from
portrait and sample fills. Both channels have transparent centres (verified
alpha zero). It remains a fitting candidate, not a loaded runtime replacement.
