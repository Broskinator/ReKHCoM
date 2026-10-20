# ReKHCoM on Linux

The game core needs 32-bit pointers, so the Linux builds are 32-bit programs
(x86 for PCs and the Steam Deck, armhf for ARM handhelds). Each package brings
its own loader and libraries in `lib/`: no 32-bit system packages are needed.

You need your own cartridge dump (EU, US or JP, 32 MiB, unpatched). No ROM is
included. Copy it into `roms/` with any file name: the version is read from
the ROM header. Saves (`kh_com.sav`), settings (`kh_com.ini`) and quick states
(`states/`) are kept in the game folder. The archives have no top folder: to
update, extract the new one over the old install; saves, settings and the ROM
stay.

## Linux PC (`linux-x86`)

    mkdir ReKHCoM && cd ReKHCoM
    tar xf /path/to/ReKHCoM-*-linux-x86.tar.gz
    cp /path/to/your/rom.gba roms/
    ./ReKHCoM.sh

With several ROMs in `roms/`, pick one with `./ReKHCoM.sh --region us` (`eu`,
`us` or `jp`). Settings are in the in-game options menu (there is no separate
launcher on Linux). F11 or Alt+Enter toggles fullscreen.

It runs under X11 or XWayland and plays sound through PulseAudio/PipeWire or
ALSA. A 64-bit system's GPU driver cannot be loaded into a 32-bit program, so
the picture is drawn with SDL's software renderer; that is light at the
game's own resolution.

## Steam Deck (`steamdeck`)

Same program, set to fullscreen from the first launch.

1. In Desktop Mode, extract the archive into a folder of its own (for example
   `~/Games/ReKHCoM`) and copy your ROM into its `roms/` folder.
2. Steam > Games > *Add a Non-Steam Game to My Library* > Browse, show all
   file types, pick `ReKHCoM.sh`.
3. Back in Game Mode, start it from the library. The default *Gamepad* Steam
   Input layout works; the touchscreen is not used.

Not reviewed by Valve: there is no Deck Verified status.

## ARM handhelds (`anbernic-alpha`)

**Alpha, not yet tested on a real handheld.** Aimed at the Anbernic RG34XX
(H700) and similar devices with Knulli, muOS or ROCKNIX, in the PortMaster
layout.

1. Extract the zip into the ports folder (for example `roms/ports/`): it adds
   `ReKHCoM.sh` and the `rekhcom/` folder.
2. Copy your ROM into `rekhcom/roms/`.
3. Start *ReKHCoM* from the Ports menu. SELECT+START closes it when the
   firmware provides PortMaster's gptokeyb.

Requirements: the firmware's 64-bit kernel must run 32-bit ARM programs, and
the screen must be reachable as `/dev/fb0` (the game draws there directly;
it does not use the GPU). The log is written to `rekhcom/log.txt`.

## Building

See `port/progress/LINUX.md` in the source repository.
