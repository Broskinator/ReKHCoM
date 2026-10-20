#!/usr/bin/env python3
"""Build the Switch alpha: the game image (AArch64 with 32-bit pointers) and the libnx host (.nro).

The game core needs 32-bit pointers, and the Switch runs only 64-bit programs that cannot map the GBA's low
addresses. So, as on the PS5 (port/ps5/README.md), the game and the port layer are one image the host loads and
calls through the bridge (port/ps5/shared/bridge_list.h), with two differences:

- the image is AArch64 code with 32-bit pointers: clang's arm64_32 target (Mach-O objects, linked by lld's Mach-O
  driver at 0x10000000, above the code region's start);
- the GBA space is relocatable (PORT_GBA_RELOC, include/gba/gba_space.h): every region is an array of the image.

    python port/switch/build_switch.py guest            # build/switch/guest-<region>.macho, on Windows or Linux
    python port/switch/build_switch.py host             # the .nro (devkitPro, in the Linux VM)

Needs the generated assets of each region (build/<region>) and the ROM pointer lists (port/data/rom_pointers_*.inc).
"""
import argparse
import concurrent.futures
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
PS5 = ROOT / 'port/ps5'
BUILD = ROOT / 'build/switch'
WINDOWS = os.name == 'nt'
LLVM = ROOT.parent / 'toolchain/llvm-mingw-20261006-ucrt-x86_64/bin'
CLANG = os.environ.get('CLANG', str(LLVM / 'clang.exe') if WINDOWS else 'clang-18')
LLD = os.environ.get('LLD', str(LLVM / 'ld.lld.exe') if WINDOWS else 'ld.lld-18')
TARGET = '--target=arm64_32-apple-watchos'
CPU = '-mcpu=cortex-a57'  # the Switch's CPU (ARMv8.0): without it clang targets Apple's and emits ARMv8.3 (xpaci)
BASE = 0x10000000
sys.path.insert(0, str(PS5))
import build_ps5  # noqa: E402  (its PORT_PATCHES and PORT_EXCLUDED are shared)

GUEST_SKIP = {'crt0', 'stubs'}  # x86-64 assembly: port/switch/guest has the AArch64 ones


def run(cmd, **kw):
    r = subprocess.run([str(c) for c in cmd], **kw)
    if r.returncode:
        sys.exit(f'{Path(str(cmd[0])).name} failed ({r.returncode})')


def common_flags():
    resource = subprocess.check_output([CLANG, '-print-resource-dir'], text=True).strip() + '/include'
    return [TARGET, CPU, '-ffreestanding', '-nostdinc', '-isystem', resource, f'-I{PS5 / "guest/include"}', f'-I{PS5 / "shared"}',
            '-O2', '-funsigned-char', '-fno-strict-aliasing', '-fwrapv', '-fno-stack-protector', '-fno-common',
            '-fno-omit-frame-pointer', '-fno-builtin-printf', '-fno-math-errno', '-DPORT_GBA_RELOC', '-DPORT_SWITCH']


def macho_symbols(source, out):
    """rom_symbols_reloc.s (COFF syntax for the Windows check build) as Mach-O: the regions as zero-filled space."""
    lines = []
    for line in source.read_text().splitlines():
        s = line.strip()
        if s.startswith('.section') or s.startswith('.balign'):
            continue
        m = re.match(r'_(\w+):$', s)
        if m:
            pending = m.group(1)
            continue
        m = re.match(r'\.space\s+(\S+)', s)
        if m:
            lines.append(f'\t.zerofill __GBA,__space,_{pending},{m.group(1)},12')
            continue
        lines.append(line)
    out.write_text('\n'.join(lines) + '\n')


def compile_all(jobs, units):
    def one(unit):
        src, obj, extra = unit
        obj.parent.mkdir(parents=True, exist_ok=True)
        if obj.exists() and obj.stat().st_mtime >= src.stat().st_mtime and not os.environ.get('REBUILD'):
            dep = obj.with_suffix('.d')
            if dep.exists():
                text = dep.read_text().replace('\\\n', ' ')
                deps = [Path(p) for p in re.findall(r'(?:[A-Za-z]:)?[/\\][^\s:]+(?:\s|$)', text.split(':', 1)[-1])]
                newest = max((p.stat().st_mtime for p in (Path(str(d).strip()) for d in deps) if p.exists()), default=0)
                if newest <= obj.stat().st_mtime:
                    return None
        cmd = [CLANG, *extra, '-MD', '-MF', obj.with_suffix('.d'), '-c', src, '-o', obj]
        r = subprocess.run([str(c) for c in cmd], capture_output=True, text=True)
        return f'{src}\n{r.stderr}' if r.returncode else None
    with concurrent.futures.ThreadPoolExecutor(jobs) as pool:
        errors = [e for e in pool.map(one, units) if e]
    if errors:
        print('\n'.join(errors[:5]), file=sys.stderr)
        sys.exit(f'{len(errors)} file(s) failed to compile')


def build_guest(region, jobs):
    upper = region.upper()
    pc = ROOT / 'build' / region / 'pc'
    if not (pc / 'rom_symbols_reloc.s').is_file():
        run([sys.executable, ROOT / 'port/tools/gen_pc_assets.py', '--version', region], cwd=ROOT,
            env=dict(os.environ, PYTHONUTF8='1'))
    out = BUILD / f'guest-{region}'
    out.mkdir(parents=True, exist_ok=True)
    includes = build_ps5.game_includes(region)
    game_flags = [*common_flags(), '-w', '-std=gnu89', '-include', ROOT / 'port/include/port/game_prefix.h', '-DPLATFORM_PC',
                  f'-DVERSION_{upper}', *includes]
    port_flags = [*common_flags(), '-w', '-std=gnu11', '-DSTBI_NO_THREAD_LOCALS', f'-I{PS5 / "third_party/stb"}',
                  f'-I{ROOT / "port/src"}', '-DPLATFORM_PC', f'-DVERSION_{upper}', *includes]
    units = []
    for src in sorted((ROOT / 'src').rglob('*.c')):
        units.append((src, out / 'game' / src.relative_to(ROOT / 'src').with_suffix('.o'), game_flags))
    for src in sorted((ROOT / 'port/src').glob('*.c')):
        if src.stem in build_ps5.PORT_EXCLUDED:
            continue
        if src.name in build_ps5.PORT_PATCHES:
            patched = PS5 / 'build' / f'guest-{region}' / 'patched' / src.name
            if not patched.exists():
                sys.exit(f'{patched} missing: run port/ps5/build_ps5.py guest --region {region} once (it writes the patched copies)')
            src = patched
        units.append((src, out / 'port' / (src.stem + '.o'), port_flags))
    units.append((pc / 'rom_relocs.c', out / 'port/rom_relocs.o', port_flags))
    for src in sorted((PS5 / 'guest/src').glob('*.c')):
        if src.stem not in GUEST_SKIP:
            units.append((src, out / 'ps5' / (src.stem + '.o'), port_flags))
    units.append((HERE / 'guest/stubs_a64.c', out / 'switch/stubs_a64.o', port_flags))
    macho_symbols(pc / 'rom_symbols_reloc.s', out / 'rom_symbols.s')
    for src in (out / 'rom_symbols.s', HERE / 'guest/crt0_a64.S'):
        units.append((src, out / 'asm' / (src.stem + '.o'), [TARGET, CPU, '-c']))
    compile_all(jobs, units)
    image = BUILD / f'guest-{region}.macho'
    run([LLD, '-flavor', 'darwin', '-arch', 'arm64_32', '-platform_version', 'watchos', '9.0', '9.0', '-e', '_switch_boot',
         '-pagezero_size', hex(BASE), '-map', out / 'guest.map', '-o', image, *[u[1] for u in units]])
    print(f'guest {region}: {image} ({image.stat().st_size} bytes)')
    return image


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('step', nargs='?', default='guest', choices=['guest', 'host', 'all'])
    p.add_argument('--regions', nargs='+', choices=['eu', 'us', 'jp'], default=['eu', 'us', 'jp'])
    p.add_argument('--jobs', type=int, default=max(1, (os.cpu_count() or 2) // 2))
    args = p.parse_args()
    if args.step in ('guest', 'all'):
        for region in args.regions:
            build_guest(region, args.jobs)
    if args.step in ('host', 'all'):
        sys.exit('host: see port/switch/host (devkitPro)')


if __name__ == '__main__':
    main()
