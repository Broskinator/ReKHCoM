# Browser port

The original C game and SDL3 renderer compile to wasm32. The browser reads a
player-selected cartridge locally, checks SHA-1 and copies it directly into
linear memory. No ROM is included in the deployment or uploaded by the launcher.
SRAM and settings use IndexedDB; normal `.sav` import/export provides backups.

## Current verification

- EU/US/JP WebAssembly build with Emscripten 6.0.12 and SDL 3.4.18.
- User tested the actual game in Chrome through the opening dialogue. Original
  text works. Selecting HD at New Game exposed an empty-dialogue bug: the PPU
  now forces original text every frame on Web, as on Android.
- US/JP modules require regional browser play tests.
- This alpha uses original pixel text. Windows GDI font rasterization and
  pointer-based quick states are unavailable on Web. Normal SRAM is supported.
- Physical mobile browser performance and long play sessions remain unmeasured.

## Resource use

- Only the selected region's module is downloaded and instantiated.
- The ROM is written directly at `0x08000000`; no second ROM copy in MEMFS.
- Existing demand-sized PPU buffers and opaque-row fast paths are reused.
- Canvas uses CSS pixels, without multiplying work by devicePixelRatio.
- Pace yields to the browser instead of spinning. Hidden tabs pause logic,
  drawing and audio; SDL RenderPresent does not add a second Asyncify timer.
- Touch drawing is enabled only when selected (default for a coarse pointer).
- GBA pointers retain their addresses in sparse linear memory. The initial
  256 MiB linear address range is **not a measurement of resident RAM**.
  Static native data starts after the GBA range. A future memory-layout port
  could lower this address-space reservation; do not claim that it is done.

## Local build

Generate the usual PC assets locally for each region first. Do not distribute
the cartridge used during generation. Install the official Emsdk 6.0.12 and
build SDL 3.4.18 with its Emscripten CMake toolchain, SDL_STATIC=ON,
SDL_SHARED=OFF. Then, from the repository root:

```powershell
python port/tools/build_web.py --emsdk D:/ReKHCoM-build/toolchain/emsdk --sdl-build D:/ReKHCoM-build/web-sdl --sdl-source ../toolchain/android/SDL3-3.4.18 --work D:/ReKHCoM-build/web --region eu
python -m http.server 8765 --bind 127.0.0.1 --directory build/web
```

Repeat for `us` and `jp`. `build/web` contains the deployable site. Public
deployment must contain only the launcher, regional JS/Wasm modules, licenses
and the integrity manifest, never `.gba`, `.sav`, `.khst` or user configuration.
Serve `.wasm` as `application/wasm`. This single-thread build needs HTTPS in
production for Web Crypto, but does not require COOP/COEP headers.

## Compiler details

Wasm assemblers reject the absolute symbols used by the desktop linker.
`build_web.py` compiles C to LLVM IR and replaces **imported** cartridge globals
with `inttoptr` constants. Native definitions with the same name are rejected.
ROM relocations that reference SrollBlit functions use actual function-pointer
declarations, preserving Wasm table-index relocations. Pointer casts in existing
callbacks use Emscripten's compatibility pass. Asyncify supports AgbMain and the
movie player's nested VBlank waits. Browser reset reloads the page.

Important: missing C prototypes can become trapping signature-mismatch thunks
under Wasm even when x86 tolerates them. Fix declarations, and treat any linker
signature warning as a release blocker. The first Binaryen fpcast/Asyncify crash
was resolved by removing the desktop-only setjmp entry from the Web main loop.
