# Contributing

OpenKHCoM is an experimental Windows native port of the GBA game, based on
[Pheenoh/khcom](https://github.com/Pheenoh/khcom).

## Project layout

- `src/`, `include/`, `asm/`: the original decompilation and ARM routines.
- `port/src/`, `port/include/`: native platform, rendering, audio and input.
- `port/configure_pc.py`: Windows x86 build configuration.
- `port/tools/gen_pc_assets.py`: local ROM symbol and asset glue generation.
- `port/progress/status.json`: manually maintained subsystem estimates.
- `port/tools/progress.py`: generated progress report and graphic.
- `roms/`, `assets/`, `build/`: local inputs and generated output, ignored by Git.

## Working on the port

Follow the build instructions in the README. Keep native changes behind
`PLATFORM_PC` where they touch original game code. Preserve the 32-bit pointer
layout and original-address memory map. Do not commit ROMs, extracted assets,
saves, save states, logs or personal configuration.

Verify changes using an isolated folder under `build/`, with a copy of the
executable and SDL3. The ROM loader can find `../../roms/B8CP.gba` from there.
Do not run automated checks against a player's existing save.

The executable exposes a local regression harness:

- `KHCOM_FAST=1`: disable pacing.
- `KHCOM_KEYS=100-110:A,300-310:START`: simulate GBA buttons by display frame.
- `KHCOM_SHOTS=500,1000`: write framebuffer BMPs.
- `KHCOM_QUIT=1200`: exit after a fixed number of display frames.
- `KHCOM_SAVESTATE=1000:test.state`: request a state snapshot.
- `KHCOM_LOADSTATE=test.state`: load a state from the same build.

Use a clean build for releases, inspect the package's explicit file list, and
retain SHA-256 checksums. Record what was actually verified and keep known
limitations visible in the release notes.

## Next milestones

1. Replace the self-modifying ARM movie codecs with native C routines.
2. Integrate movie timing, video presentation and audio streaming.
3. Verify the opening movie and the return to gameplay.
4. Expand scene regression coverage and complete both story playthroughs.
