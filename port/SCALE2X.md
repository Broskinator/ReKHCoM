# Experimental Scale2x graphics filter — second pass

Open ReKHCoM.exe and choose Graphics > Filter > Scale2x, or change Filter in the in-game Options menu. Choose Sharp for the original pixel presentation. The local experimental package defaults to Scale2x; upgrading does not require replacing saves or personal settings.

## Coverage

The optional filter reconstructs pixel layers at 2x before SDL scales them to the window. It now covers the main game picture (backgrounds, characters, enemies and effects), the card/HP HUD, menu overlays, foreground pixels drawn over HD replacements, generated menu side margins, the movie skip prompt and state notices. Native HD art and HD text keep their existing higher-resolution rendering. Native movie playback, decorative image borders, deliberately blurred backdrops and the experimental HD-2D world pass retain their own rendering.

This is edge-aware pixel reconstruction, not newly drawn HD art, DLSS or an AI remaster. Reconstruction runs on the CPU; graphics-backend sampling and presentation were verified on Direct3D 11. No claim is made that every scene or asset has been remastered.

## Transparent layers and fallback

Transparent layers retain alpha during reconstruction, then use premultiplied colours and premultiplied alpha blending during linear sampling. This prevents transparent padding from adding dark or coloured fringes. Each independently composited layer has its own texture cache, including separate left/right menu margins. Source crops are doubled while destination coordinates stay unchanged. Disabling the filter releases the caches; resizing reallocates them. If allocation, blend support or upload fails, drawing falls back to the original texture.

## Verification

- Windows EU game and launcher build.
- Algorithm assertions: single-pixel boundaries, uniform colour/alpha and corner reconstruction.
- SDL software-renderer assertions: translucent red blended over white, transparent coloured padding, crop coordinates, dimensions/format changes, disabling/re-enabling and fallback.
- Isolated original/filtered battle captures, 260 frames per run, Direct3D 11.
- Original GBA-view battle captures, 260 frames per run.
- Presented options-menu capture, inspected for layout.
- Live options changes from Scale2x through Sharp, Pixel art and Smooth back to Scale2x, then return to gameplay.
- Software-renderer battle capture, 260 frames.

The battle harness sets maximum HP to 560, so the original game's second (blue) HP bar is visible. Tests use isolated save/config folders. No complete story playthrough, native movie transition check or performance benchmark has been completed. Wider/zoomed-out scenes require more CPU work and memory.

## Building the checks

Build the game normally as documented in README.md. Compile port/tests/scale2x_test.c with -Iport/include. Compile port/tests/scale2x_sdl_test.c with the same include path and the SDL3 include/library paths, link SDL3, and run beside its matching SDL3.dll. Both executables must exit successfully; assertions must remain enabled.

Fork: https://github.com/Broskinator/ReKHCoM/tree/graphics/scale2x
