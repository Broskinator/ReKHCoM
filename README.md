# OpenKHCoM

Native PC port of **Kingdom Hearts: Chain of Memories** (Game Boy Advance), built
on top of the matching decompilation by [Pheenoh/khcom](https://github.com/Pheenoh/khcom).

> **Resumen (ES):** port nativo para PC basado en la decompilación. El código del juego
> corre tal cual en x86; una capa de plataforma (SDL3) sustituye al hardware de la GBA:
> vídeo, sonido, BIOS, DMA, interrupciones y guardado. Incluye vista panorámica 16:9 que
> conserva el pixel art a escala 1:1 y opciones gráficas.

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

## Building (Windows)

Requirements: Python 3 with PyYAML, [llvm-mingw](https://github.com/mstorsjo/llvm-mingw),
[SDL3 mingw devel](https://github.com/libsdl-org/SDL/releases) and [ninja](https://ninja-build.org).
By default the tools are looked up in `../toolchain/`.

```sh
cp /path/to/your/dump.gba roms/B8CP.gba
python tools/extract_assets.py eu        # optional; text is extracted on configure
python port/configure_pc.py              # --debug for a console build
ninja -f build/pc/build.ninja
build/pc/khcom.exe
```

Put `baserom.gba` next to `khcom.exe` when distributing the build.

## Controls

| GBA | Keyboard | Gamepad |
|---|---|---|
| A | X / K | A (south), Y |
| B | Z / J | B (east), X |
| L / R | A / S | LB / RB, triggers |
| Start / Select | Enter / Backspace | Start / Back |
| D-pad | Arrows, W | D-pad, left stick |

F11 or Alt+Enter toggles fullscreen.

## Credits

- Decompilation: [Pheenoh/khcom](https://github.com/Pheenoh/khcom) and contributors (CC0).
- PC port: OpenKHCoM contributors.

The original decompilation README is kept in [README.decomp.md](README.decomp.md).
