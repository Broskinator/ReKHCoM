# Enemy HP frame: AI redraw pilot

Optional experimental `hd_hud=1`, default off. This is a placeholder,
not final remaster artwork. It replaces the enemy HP panel in views using
the separate HUD layer. Original-size views retain the original panel.
Other HUD elements remain unchanged. The frame bounds come from the original
sprites every frame, including language and gauge-size variants. The native
coloured HP lettering, layered bars and animated ratios retain their exact
positions and pixels. Only the neutral frame is redrawn at HD resolution.

Generated with the built-in imagegen tool from an original battle screenshot.
`enemy-hp-frame-v2.png` has real RGBA transparency outside the frame and a
black opaque interior. Decoded once, CPU pixels freed after texture upload.
Missing, oversized, empty or opaque-border images fall back to the game HUD.
The uploaded crop is limited to 512 pixels wide; two cached textures (colour
and white-fade mask) use about 320 KiB for this pilot. Software SDL uses nearest
sampling of the antialiased asset to avoid its linear-minification crash;
accelerated renderers use linear sampling. No per-frame decoding.
Do not claim perfect fidelity: final polishing and play feedback remain.

Final edit prompt (v1 isolated the enemy HP frame from the screenshot):

> Edit this single HUD frame into a production-quality HD 2D game UI asset.
> Preserve exact wide shallow proportions, clipped corner shape, black opaque
> interior, white ivory highlight and gray bevel colors. Critical change:
> REDRAW EVERY EDGE as clean continuous straight lines with smooth antialiased
> diagonal chamfers. Eliminate ALL staircase pixel blocks, jagged edges, stray
> pixels, broken white lines, grain and noisy borders. Do not simply blur or
> upscale the input. Reconstruct the contours as precise crisp vector-like 2D
> graphic art rasterized at high resolution. Thin restrained beveled frame,
> flat dark interior, no reflections or texture. Same one frame, no text,
> no green bar, no ornaments. Centered with fully transparent exterior and
> padding, actual alpha. Pixel staircase corners are explicitly unacceptable.
