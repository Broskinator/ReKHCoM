# ReKHCoM

Native PC port of **Kingdom Hearts: Chain of Memories** (Game Boy Advance), built
on top of the matching decompilation by [Pheenoh/khcom](https://github.com/Pheenoh/khcom).

A native PC port of the Game Boy Advance game: the decompiled code runs as
a real Windows program, with its own renderer, audio and launcher, in 16:9
and at the cartridge's exact speed (59.7275 Hz).

**Current prerelease: [v0.8](https://github.com/MatyMeatBoy/ReKHCom/releases/tag/v0.8)**
(Windows x86). Works with the **European, US and Japanese** cartridges, in all
five European languages, English (US) and Japanese.

<p align="center"><img src="docs/screenshots/battle.png" width="49%"> <img src="docs/screenshots/field.png" width="49%"></p>

## Highlights

- **16:9 without stretching.** Fields and battles show more of the world;
  full-screen menus get margins drawn from the scene's own tiles, bars and
  panels, on the picture's exact pixel grid.
- **Original speed and sound.** One game update per VBlank, as on the GBA;
  m4a music, PSG and PCM reimplemented in C; native movie playback.
- **Launcher.** Installs and checks your dump (SHA-1), picks the version,
  language, graphics, sound, controls and saves; imports GBA saves.
- **Re:CoM mode (optional).** PS2-inspired options chosen at New Game:
  difficulty and quality-of-life buttons. Off = the cartridge, untouched.
- **HD text, HD-2D look, smooth audio** as optional enhancements.

<p align="center"><img src="docs/screenshots/deck.png" width="32%"> <img src="docs/screenshots/status.png" width="32%"> <img src="docs/screenshots/worldcards.png" width="32%"></p>
<p align="center"><img src="docs/screenshots/journal.png" width="32%"> <img src="docs/screenshots/options.png" width="32%"> <img src="docs/screenshots/moogle_hd.png" width="32%"></p>

### Pixel text or HD text

Optional HD text preserves the game's style in menus, dialogue and subtitles.

<p align="center"><img src="docs/screenshots/moogle_px.png" width="49%"> <img src="docs/screenshots/moogle_hd.png" width="49%"></p>

<p align="center"><img src="docs/screenshots/subtitles_hd.png" width="70%"></p>

### Every language of every cartridge

| English (US) | Français | Deutsch |
|---|---|---|
| ![](docs/screenshots/lang_us.png) | ![](docs/screenshots/lang_fr.png) | ![](docs/screenshots/lang_de.png) |
| **Español** | **Italiano** | **日本語** |
| ![](docs/screenshots/lang_es.png) | ![](docs/screenshots/lang_it.png) | ![](docs/screenshots/lang_jp.png) |

![progress](port/progress/progress.svg)

See [PROGRESS.md](PROGRESS.md) for the detailed breakdown.

> [!IMPORTANT]
> This repository contains **no game assets and no ROM**. You need your own dump of one
> of the cartridges:
>
> | Version | Code | SHA-1 |
> |---|---|---|
> | Europe (EN, FR, DE, ES, IT) | `B8CP` | `8db73586cdb11b3795907edebf43228dbcd3e6b2` |
> | USA | `B8CE` | `10729bd884f8fdca7a310b6d606c52e46657aa48` |
> | Japan | `B8CJ` | `59ec0a0a4ccd1e6acb3bbd7bfb21d63988958cfa` |
>
> Each version has its own executable and save file; the launcher picks the one
> that matches your dump.

## How it works

- The 269 decompiled C units are compiled for 32-bit x86, so pointers keep their GBA size.
- The GBA address space (work RAM, I/O, palette, VRAM, OAM, ROM, SRAM) lives at its original
  addresses inside the executable image; register writes and ROM pointers work unchanged.
- Asset data is read straight from your ROM: 61k data symbols are pinned to their ROM
  addresses, and the few ROM tables that point at code are patched at start-up.
- `port/` holds the platform layer: software PPU (all modes, affine, windows, blending,
  mosaic, HBlank effects), m4a sequencer/mixer in C, PSG emulation, BIOS, DMA, input,
  settings and the wide-view renderer.

## Running (Windows)

1. Extract `ReKHCoM-v0.8-windows-x86.zip` into a writable folder.
2. Run `ReKHCoM.exe` and install your cartridge dump with **Install ROM...** (or drag
   the file onto the window). The dump is checked by SHA-1 and copied to `roms/`.
3. Press **Play**. The launcher starts `khcom_eu.exe`, `khcom_us.exe` or `khcom_jp.exe`
   for the version you chose.

The launcher also sets the European game's language, shows each version's save
(open its folder, or back it up to `backups/` with a date stamp), and edits the
graphics, sound, enhancement and control settings shared with the game.

The ZIP includes SDL3; you do not need to install it separately. The executables
are 32-bit x86 and have been tested locally on Windows only.

Settings (`kh_com.ini`), your cartridge saves (`kh_com.sav`, `kh_com_us.sav`,
`kh_com_jp.sav`), and the diagnostic log (`kh_com.log`) are stored next to the
executables. Back up your saves before trying new builds. Save states are specific
to the build that created them.

### Known limitations

- Movie transitions during events and the ending still need verification.
- Neither Sora's story nor Reverse/Rebirth has a verified full playthrough.
- Some rendering, audio streaming, and widescreen scenes still need verification.
- Linux and macOS builds are not available.

## Building (Windows)

Requirements: Python 3 with PyYAML, [llvm-mingw](https://github.com/mstorsjo/llvm-mingw),
[SDL3 mingw devel](https://github.com/libsdl-org/SDL/releases) and [ninja](https://ninja-build.org).
By default the tools are looked up in `../toolchain/`.

```sh
git clone https://github.com/MatyMeatBoy/ReKHCom.git
cd ReKHCom
python -m pip install pyyaml
# Copy your dumps to roms/B8CP.gba, roms/B8CE.gba and/or roms/B8CJ.gba first.
python port/configure_pc.py              # European game + launcher, build/pc; --debug for a console build
python port/configure_pc.py --version us # USA game, build/pc-us
python port/configure_pc.py --version jp # Japanese game, build/pc-jp
ninja -f build/pc/build.ninja
build/pc/ReKHCoM.exe                     # launcher (decompui); or run a khcom.exe directly
```

The launcher, `ReKHCoM.exe` (source in `port/decompui`, Dear ImGui), installs your
dumps after checking their SHA-1, edits the game's settings and enhancements
and starts the executable built for the chosen version.

Use `--toolchain PATH --sdl PATH` when your dependencies are outside
`../toolchain/`. The SDL package must include `i686-w64-mingw32/`.
Build from the repository root. Asset glue and text are generated locally from
your dump during the first configure step.

To make a distributable ZIP without ROMs, saves or local settings:

```sh
python port/tools/package_release.py --version v0.8 --sdl ../toolchain/SDL3-3.4.18
```

## Controls

| GBA | Keyboard | Gamepad |
|---|---|---|
| A | X / K | A (south) |
| B | Z / J | B (east) |
| L / R | A / S | LB / RB, triggers |
| L + R (stock a card / sleight) | D | Y (north) |
| Dodge roll | C | X (west) |
| Start / Select | Enter / Backspace | Start / Back |
| D-pad | Arrows, W | D-pad, left stick |
| Change card / stock | | Right stick: left / right = L / R, up = L + R |

These are the defaults; every action takes two keys and a gamepad button, edited
on the launcher's Controls page (or `kb_*` / `pad_*` in `kh_com.ini`). The one-button
stock and dodge roll can be turned off in the options; their gamepad buttons then
act as A and B.

F11 or Alt+Enter toggles fullscreen.

F1, Esc, or Start + Select (keyboard or controller, as bound) opens the options
overlay anywhere; at the title screen Select alone opens it. F5 saves a state, F9
loads a state, and F6/F7 change the state slot. Save states do not include
cartridge SRAM.

During the opening movie, A shows a prompt; A again skips the movie.

## Development

See [CONTRIBUTING.md](CONTRIBUTING.md) for the project layout and verification
workflow. The progress percentages describe development estimates, not game
completion or a compatibility guarantee.

## Credits

- Decompilation: [Pheenoh/khcom](https://github.com/Pheenoh/khcom) and contributors (CC0).
- Third-party asset credits: [NOTICE.md](port/assets/fonts/NOTICE.md).
- PC port: ReKHCoM contributors.

The original decompilation README is kept in [README.decomp.md](README.decomp.md).

This is an independent fan project, unaffiliated with Square Enix, Disney or
Nintendo. Their game content and trademarks are not covered by the source
code's [CC0 license](LICENSE.md). SDL3 is distributed under its own license,
included in the Windows package.
