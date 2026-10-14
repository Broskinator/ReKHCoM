# Preliminary HUD pieces

Manifest entries contain generated redraws only. Original ROM-local captures
stay under build/. A filename is the FNV64 content key emitted by the local
HUD exporter: width, height and original ARGB pixels. The loader applies
pieces inside original native ink bounds and clips by actual OAM ownership.
Missing/invalid pieces stay native; Original view and disabled HD HUD stay native.

First piece: Kingdom Key card face. Preserve the original royal-blue gradient,
silver diagonal blade, yellow guard, outlines and shadows. The first redraw
was corrected following user feedback because its background looked too flat.
Card borders and dynamic digits are separate sprites and remain separate.

Current loader covers non-affine HUD sprites, mirrored states and black/white
fades. Affine poses retain native art until transformed geometry is supported.
Decoded CPU data is freed after upload. Longest texture edge: 256 pixels;
8 MiB total cache budget including white fade masks. Missing lookups are cached.

Editing prompt: preserve approved keyblade illustration exactly; restore the
original royal-blue background's dark-to-light gradient and edge shading,
using the original sprite as authority. No added texture, glow or new objects.
True transparent exterior padding; retain the intentional blue inner panel.
