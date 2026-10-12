# Native movie playback

The development branch plays the original European ROM's movies using native C
code. The v0.1 release is the earlier baseline and still skips movies.

## Decoder

`port/src/port_movie.c` replaces the copied, self-modifying ARM routines in
`asm/movie_codec.s`. The game keeps its existing container parsing, sound
streaming, subtitle callbacks and transitions.

- Key frames store two RGB555 endpoints and sixteen two-bit selectors for each
  16x16 block. Native code reconstructs a sparse grid and performs the original
  horizontal and vertical interpolation.
- Delta frames use separate control-bit, color and motion-vector streams. A
  recursive rectangular block tree implements copies, motion compensation,
  color offsets, fills and raw two-pixel leaves.
- Postprocessing smooths the 8-pixel block boundaries.
- Audio supports PCM16 to signed PCM8, silence, and the cartridge's two-bit
  adaptive differential decoder, preserving its predictor wrap and step table.

Packed arithmetic matters: the ARM code shifts and adds pairs of pixels as
32-bit values. The native code preserves the resulting bit-15 transfers and
carry between pixels, even though bit 15 does not affect RGB555 display.

Native callbacks replace generated code pointers. Codec buffers and placeholder
ARM symbols are no longer needed. The PC player initializes optional audio
fields, checks supported dimensions and services a dedicated display cycle
while waiting for the next movie frame. Movie timing is independent of the
gameplay speed option. Save-state operations wait until the main gameplay loop.

## Verification

Build the PC executable normally, then run:

```sh
python port/tools/test_intro.py
python port/tools/test_intro.py --language es --view 1
python port/tools/test_intro.py --view 4 --speed 1
```

These tests use isolated folders under `build/`, dummy SDL video/audio devices,
scripted input and screenshots. They require the local European ROM. Inspect
the resulting screenshots to verify subtitles and the transition to gameplay.

The independent codec oracle requires 64-bit Python and Unicorn:

```sh
python -m pip install unicorn
python port/tools/test_movie_codec.py --toolchain ../toolchain/llvm-mingw-20261006-ucrt-x86_64 --oracle-every 1
```

The test assembles and links the original ARM codec, applies its dimension
patches and tables, and runs it in Unicorn. It compares native decoded buffers
byte for byte. All 3,605 video frames and 3,604 audio blocks across opening,
two event movies and both endings have passed. No ROM or extracted movie data
is committed. The emulator is a development test dependency only.

The English opening completed all 1,617 frames at 15 fps and returned to the
first Castle Oblivion scene. Spanish subtitles and widescreen playback also
passed, as did auto view with the alternate gameplay speed setting.
An isolated audio dump contained 107.807 seconds
of non-silent movie streaming, consistent with the approximately 108-second
opening. These checks do not replace full gameplay or listening tests on real
audio hardware. Event and ending transitions still need gameplay verification.
