# ReKHCoM for PS5

Alpha build of the PS5 port: the three regions (EU, US, JP) in one app, `PPSA98511`.
Rendering is Vulkan, with audio and controller input through the PS5 platform layer.

## How it works

The game needs 32-bit pointers and the PS5 is 64-bit only, so:

- `guest/` compiles the game and the port layer as x32 (`--target=x86_64-unknown-linux-gnux32`) and links one
  flat image per region at `0x10000000` (`guest/guest.ld`). It carries a small C library and an SDL3 subset
  (`guest/src/sdl_shim.c`) that draws through the bridge.
- `app/ps5_host.c` is the PS5 program: it maps the GBA regions at their hardware addresses and the image
  below 4 GiB, fills the bridge table (`shared/bridge_list.h`) and starts the game. The cartridge version found
  in `/data/ReKHCom` picks the image. Integer divisions by zero are recovered like the cartridge does.
- `test/linux_host.c` runs the same image headless on Linux with a software rasterizer, to test without a console.
- The core (`src/`, `port/src/`) is compiled unchanged; `guest/src/` replaces what is platform-specific.

## Build

The folder is self-contained: it carries its own toolchain (`tooling/`, `tools/`, `runtime/`), the platform
layer (`app/`) and its resources (`sce_sys/`, `licenses/`). The downloaded dependencies (PS5 SDK, SDL2, the
Vulkan driver) go to `.deps/`, ignored by git; the Vulkan driver build comes from a GitHub release, so the first
build needs `RADV_RELEASE_REPO=<owner/repo>` (and `gh` logged in) or a prepared `.deps/`.

```
python3 port/ps5/build_ps5.py all                      # guest images (eu us jp) + app; needs roms/<code>.gba per region
python3 port/ps5/build_ps5.py deploy --ip <console>    # FTP to /data/homebrew/PPSA98511
```

`guest`, `host` and `deploy` can be run separately. Output: `port/ps5/dist/PPSA98511`.
Copy your cartridge dump (any name, `.gba`) to `/data/ReKHCom` on the console.

Test without a console:
`clang -O2 -Iport/ps5/shared port/ps5/test/linux_host.c -o port/ps5/build/linux_host -lm` then
`KHCOM_DATA=<dir with the .gba> port/ps5/build/linux_host port/ps5/build/guest-eu/guest.bin --shot 100 shot.ppm`
(`KHCOM_PADS="60-63:A"` scripts buttons; the game's own `KHCOM_*` variables work).

## Not available yet

Quick states, HD text and Shift-JIS text input (Japanese names) are off, as on Android.
Not tested on the console yet.
