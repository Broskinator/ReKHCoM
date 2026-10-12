# OpenKHCoM

Native PC port of **Kingdom Hearts: Chain of Memories** (Game Boy Advance), built
on top of the matching decompilation by [Pheenoh/khcom](https://github.com/Pheenoh/khcom).

**v0.1 is an early Windows prerelease.** It boots into gameplay, supports wider
views and graphics options, and saves to disk. Full playthroughs have not been
verified. Opening and other movies are skipped in this release.

Download the Windows ZIP from [Releases](https://github.com/MatyMeatBoy/OpenKHCoM/releases).
See [CHANGELOG.md](CHANGELOG.md) for release notes.

Development now includes native movie playback. See [the movie implementation
and verification notes](port/MOVIES.md); v0.1 remains the earlier baseline.

![progress](port/progress/progress.svg)

See [PROGRESS.md](PROGRESS.md) for the detailed breakdown.

> [!IMPORTANT]
> This repository contains **no game assets and no ROM**. You need your own dump of the
> European cartridge (`B8CP`, SHA-1 `8db73586cdb11b3795907edebf43228dbcd3e6b2`).

## How it works

- The 269 decompiled C units are compiled for 32-bit x86, so pointers keep their GBA size.
- The GBA address space (work RAM, I/O, palette, VRAM, OAM, ROM, SRAM) lives at its original
  addresses inside the executable image; register writes and ROM pointers work unchanged.
- Asset data is read straight from your ROM: 61k data symbols are pinned to their ROM
  addresses, and the few ROM tables that point at code are patched at start-up.
- `port/` holds the platform layer: software PPU (all modes, affine, windows, blending,
  mosaic, HBlank effects), m4a sequencer/mixer in C, PSG emulation, BIOS, DMA, input,
  settings and the wide-view renderer.

## Running v0.1 (Windows)

1. Extract `OpenKHCoM-v0.1-windows-x86.zip` into a writable folder.
2. Place your European cartridge dump next to `khcom.exe`, named `baserom.gba`.
3. Run `khcom.exe` and select your language.

The supported ROM is 32 MiB, cartridge code `B8CP`, SHA-1
`8db73586cdb11b3795907edebf43228dbcd3e6b2`. US and Japanese ROMs are not supported
by the PC build. The ZIP includes SDL3; you do not need to install it separately.
The executable is 32-bit x86 and has been tested locally on Windows only.

Settings (`kh_com.ini`), your cartridge save (`kh_com.sav`), and the diagnostic
log (`kh_com.log`) are stored next to the executable. Back up your save before
trying new builds. Save states are specific to the build that created them.

### Known limitations

- Movies are skipped in v0.1; native decoding is the next development priority.
- Neither Sora's story nor Reverse/Rebirth has a verified full playthrough.
- Some rendering, audio streaming, and widescreen scenes still need verification.
- Controls use fixed default bindings; remapping is not implemented.
- Linux and macOS builds are not available.

## Building (Windows)

Requirements: Python 3 with PyYAML, [llvm-mingw](https://github.com/mstorsjo/llvm-mingw),
[SDL3 mingw devel](https://github.com/libsdl-org/SDL/releases) and [ninja](https://ninja-build.org).
By default the tools are looked up in `../toolchain/`.

```sh
git clone https://github.com/MatyMeatBoy/OpenKHCoM.git
cd OpenKHCoM
python -m pip install pyyaml
# Copy your European dump to roms/B8CP.gba before configuring.
python port/configure_pc.py              # --debug for a console build
ninja -f build/pc/build.ninja
build/pc/khcom.exe
```

Use `--toolchain PATH --sdl PATH` when your dependencies are outside
`../toolchain/`. The SDL package must include `i686-w64-mingw32/`.
Build from the repository root. Asset glue and text are generated locally from
your dump during the first configure step.

To make a distributable ZIP without ROMs, saves or local settings:

```sh
python port/tools/package_release.py --version v0.1 --sdl ../toolchain/SDL3-3.4.18
```

## Controls

| GBA | Keyboard | Gamepad |
|---|---|---|
| A | X / K | A (south), Y |
| B | Z / J | B (east), X |
| L / R | A / S | LB / RB, triggers |
| Start / Select | Enter / Backspace | Start / Back |
| D-pad | Arrows, W | D-pad, left stick |

F11 or Alt+Enter toggles fullscreen.

F1 or Esc opens the options overlay. At the title screen, Select also opens it;
on a controller, use Back + Start. F5 saves a state, F9 loads a state, and F6/F7
change the state slot. Save states do not include cartridge SRAM.

## Development

See [CONTRIBUTING.md](CONTRIBUTING.md) for the project layout and verification
workflow. The progress percentages describe development estimates, not game
completion or a compatibility guarantee.

## Credits

- Decompilation: [Pheenoh/khcom](https://github.com/Pheenoh/khcom) and contributors (CC0).
- PC port: OpenKHCoM contributors.

The original decompilation README is kept in [README.decomp.md](README.decomp.md).

This is an independent fan project, unaffiliated with Square Enix, Disney or
Nintendo. Their game content and trademarks are not covered by the source
code's [CC0 license](LICENSE.md). SDL3 is distributed under its own license,
included in the Windows package.
