#!/usr/bin/env python3
"""Configure the native Windows build of Kingdom Hearts: Chain of Memories.

    python port/configure_pc.py [--toolchain DIR] [--sdl DIR] [--debug]
    ninja -f build/pc/build.ninja

The game is built for 32-bit x86 so pointers keep their GBA size; see
port/include/port/port.h for the overall design.
"""
import argparse
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import ninja_syntax  # noqa: E402

VERSION = "eu"
BUILD = ROOT / "build" / "pc"
GEN = ROOT / "build" / VERSION / "gen"
PC_GEN = ROOT / "build" / VERSION / "pc"


def rel(path):
    return os.path.relpath(path, ROOT).replace("\\", "/")


def find_default(name, pattern):
    base = ROOT.parent / "toolchain"
    if base.is_dir():
        hits = sorted(base.glob(pattern))
        if hits:
            return hits[-1]
    sys.exit(f"error: {name} not found; pass --{name.lower()}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--toolchain", type=Path, help="llvm-mingw directory")
    parser.add_argument("--sdl", type=Path, help="SDL3 mingw development package")
    parser.add_argument("--debug", action="store_true", help="no optimisation, console window")
    args = parser.parse_args()

    toolchain = args.toolchain or find_default("toolchain", "llvm-mingw-*")
    sdl = args.sdl or find_default("sdl", "SDL3-*")
    sdl_arch = sdl / "i686-w64-mingw32"
    cc = toolchain / "bin" / "i686-w64-mingw32-clang.exe"
    if not cc.exists():
        sys.exit(f"error: {cc} does not exist")

    # Asset glue (headers, text fragments, ROM symbol table).
    if not (PC_GEN / "rom_symbols.s").exists():
        env = dict(os.environ, PYTHONUTF8="1")
        subprocess.check_call([sys.executable, str(ROOT / "port" / "tools" / "gen_pc_assets.py")], env=env)

    include_dirs = ["include"] + sorted(rel(p) for p in (ROOT / "include").iterdir()
                                        if p.is_dir() and p.name != "gba")
    includes = " ".join(f"-I{d}" for d in include_dirs + [rel(GEN), "port/include"])
    opt = "-O0 -g" if args.debug else "-O2 -g"
    common = (f"{opt} -std=gnu89 -funsigned-char -fno-strict-aliasing -fwrapv -fno-common "
              f"-mno-ms-bitfields -DVERSION_EU -DPLATFORM_PC")
    game_cflags = f"{common} -w -include port/game_prefix.h {includes}"
    # Port sources see the game headers only through #include "...", so names
    # such as malloc.h cannot shadow the system headers.
    quoted = " ".join(f"-iquote {d}" for d in include_dirs + [rel(GEN)])
    port_cflags = (f"{opt} -std=gnu11 -Wall -Wno-unused-function -Wno-missing-braces -mno-ms-bitfields "
                   f"-DVERSION_EU -DPLATFORM_PC {quoted} -Iport/include -I{rel(sdl_arch / 'include')}")
    ldflags = (f"-L{rel(sdl_arch / 'lib')} -lSDL3 -Wl,--disable-dynamicbase -Wl,--large-address-aware "
               + ("" if args.debug else "-Wl,--subsystem,windows"))

    BUILD.mkdir(parents=True, exist_ok=True)
    with open(BUILD / "build.ninja", "w") as f:
        n = ninja_syntax.Writer(f)
        n.variable("ninja_required_version", "1.10")
        n.variable("builddir", rel(BUILD))
        n.variable("cc", "\"" + str(cc).replace("\\", "/") + "\"")
        n.variable("game_cflags", game_cflags)
        n.variable("port_cflags", port_cflags)
        n.variable("ldflags", ldflags)
        n.newline()
        n.rule("cc_game", "$cc $game_cflags -MD -MF $out.d -c $in -o $out", description="CC $in",
               depfile="$out.d", deps="gcc")
        n.rule("cc_port", "$cc $port_cflags -MD -MF $out.d -c $in -o $out", description="CC $in",
               depfile="$out.d", deps="gcc")
        n.rule("as", "$cc -c $in -o $out", description="AS $in")
        n.rule("link", "$cc @$out.rsp -o $out $ldflags", rspfile="$out.rsp", rspfile_content="$in",
               description="LINK $out")
        n.rule("copy", 'cmd /c copy /y "$in_win" "$out_win" >nul', description="COPY $out")
        n.newline()

        objs = []
        for src in sorted((ROOT / "src").rglob("*.c")):
            obj = f"$builddir/game/{rel(src)[4:-2]}.o"
            n.build(obj, "cc_game", rel(src))
            objs.append(obj)
        for src in sorted((ROOT / "port" / "src").glob("*.c")):
            obj = f"$builddir/port/{src.stem}.o"
            n.build(obj, "cc_port", rel(src))
            objs.append(obj)
        n.build("$builddir/port/rom_relocs.o", "cc_port", rel(PC_GEN / "rom_relocs.c"))
        objs.append("$builddir/port/rom_relocs.o")
        for src in [ROOT / "port" / "src" / "port_symbols.s", PC_GEN / "rom_symbols.s"]:
            obj = f"$builddir/port/{src.stem}_s.o"
            n.build(obj, "as", rel(src))
            objs.append(obj)
        n.newline()
        n.build("$builddir/khcom.exe", "link", objs)
        dll = sdl_arch / "bin" / "SDL3.dll"
        n.build("$builddir/SDL3.dll", "copy", rel(dll),
                variables={"in_win": str(dll), "out_win": str(BUILD / "SDL3.dll")})
        n.default(["$builddir/khcom.exe", "$builddir/SDL3.dll"])
    print(f"configured {rel(BUILD / 'build.ninja')} ({'debug' if args.debug else 'release'})")


if __name__ == "__main__":
    main()
