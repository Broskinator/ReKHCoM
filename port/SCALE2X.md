# Experimental Scale2x graphics filter

Open ReKHCoM.exe and choose Scale2x under Graphics > Filter. This local package defaults to Scale2x. Choose Sharp to restore the original presentation.

The optional filter reconstructs the main game picture at 2x before SDL scales it to the window. It applies to backgrounds, world sprites, enemies and effects in that picture. Original palettes are preserved in the reconstruction; final linear scaling blends pixels. CPU reconstruction is followed by graphics-backend presentation, verified with Direct3D 11. This is not DLSS or AI-generated HD artwork.

Separate HUD/menu/HD replacement layers and native movie playback keep their existing paths. HD-2D scenes use their own existing presentation pass. Therefore this is a first implementation, not a complete remaster of every graphic. Disable HD-2D to evaluate this filter on world scenes. A software renderer remains supported. Wider/zoomed-out scenes require more processing and memory; no frame-time benchmark has been completed.

Verification: Windows EU build including launcher; algorithm assertions for single-pixel input, uniform alpha/colour and corner reconstruction; isolated 180-frame battle capture with Direct3D 11; isolated 180-frame title run with software renderer. No full-story playthrough verified.

Build as documented in README.md. The algorithm test is port/tests/scale2x_test.c; compile it with -Iport/include and run the resulting executable.

Fork: https://github.com/Broskinator/ReKHCoM/tree/graphics/scale2x
