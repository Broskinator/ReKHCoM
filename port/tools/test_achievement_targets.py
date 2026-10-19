"""Check portable achievement code with the real target toolchains.

Wasm executes all sixteen conditions in Node. Linux/Android/PS5 compile the
actual game bridge and presenter for EU/US/JP. Compilation is not a device run.
No ROM, save, account, or release is modified.
"""
from pathlib import Path
import json
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'build/achievement-targets'
OUT.mkdir(parents=True, exist_ok=True)
TC = next((ROOT.parent / 'toolchain').glob('llvm-mingw-*'))
CC = TC / 'bin/clang.exe'
EMCC = Path('D:/ReKHCoM-build/toolchain/emsdk/upstream/emscripten/emcc.exe')
SDL = ROOT.parent / 'toolchain/android/SDL3-3.4.18/include'
NDK = next((ROOT.parent / 'toolchain/android/native').glob('android-ndk-*'))
NDKBIN = NDK / 'toolchains/llvm/prebuilt/windows-x86_64/bin'
includes = [ROOT / 'include', ROOT / 'port/include']
includes += [p for p in (ROOT / 'include').iterdir() if p.is_dir() and p.name != 'gba']
quotes = [f'-iquote{p}' for p in includes]
report = []

subprocess.run([str(EMCC), '-O2', 'port/achievements/local_achievements.c',
                'port/tools/local_achievements_conformance.c', '-sENVIRONMENT=node',
                '-sNODERAWFS=1', '-o', str(OUT / 'local.cjs')], cwd=ROOT, check=True)
subprocess.run([shutil.which('node'), str(OUT / 'local.cjs')], cwd=OUT, check=True)
report.append(dict(target='wasm32', execution='PASS: all 16 trigger boundaries, persistence, guards',
                   hardware='Node Wasm runtime; browser UI checked separately'))
for region in ('eu', 'us', 'jp'):
    script = OUT / ('state-' + region + '.cjs')
    subprocess.run([str(EMCC), '-O2', '-DPLATFORM_PC', '-DVERSION_' + region.upper(), *quotes,
                    'port/src/port_ra_state.c', 'port/tools/test_ra_state.c', '-sENVIRONMENT=node',
                    '-o', str(script)], cwd=ROOT, check=True)
    subprocess.run([shutil.which('node'), str(script)], cwd=OUT, check=True)

targets = [
    ('linux-i386', CC, ['--target=i686-linux-gnu', '--sysroot=D:/ReKHCoM-build/linux/i386/sysroot', '-DPORT_LINUX', f'-I{SDL}']),
    ('linux-armhf', CC, ['--target=arm-linux-gnueabihf', '--sysroot=D:/ReKHCoM-build/linux/armhf/sysroot', '-DPORT_LINUX', f'-I{SDL}']),
    ('android-armv7', NDKBIN / 'clang.exe', ['--target=armv7a-linux-androideabi21', f'-I{SDL}']),
    ('android-x86', NDKBIN / 'clang.exe', ['--target=i686-linux-android21', f'-I{SDL}']),
    ('ps5-guest-x32', CC, ['--target=x86_64-unknown-linux-gnux32', '-ffreestanding', '-nostdinc',
       '-isystem', subprocess.check_output([str(CC), '-print-resource-dir'], text=True).strip() + '/include',
       f'-I{ROOT / "port/ps5/guest/include"}', f'-I{ROOT / "port/ps5/shared"}']),
]
for target, compiler, flags in targets:
    for region in ('eu', 'us', 'jp'):
        folder = OUT / target / region
        folder.mkdir(parents=True, exist_ok=True)
        for source in ('port_achievements.c', 'port_achievement_toast.c', 'port_ui.c', 'port_ra_state.c'):
            subprocess.run([str(compiler), '-std=gnu11', '-O2', '-DPLATFORM_PC',
                '-DVERSION_' + region.upper(), *quotes, *flags, '-c',
                str(ROOT / 'port/src' / source), '-o', str(folder / (source + '.o'))], check=True)
        report.append(dict(target=target, region=region, compile='PASS: real bridge and presenter',
                           execution='NOT TESTED on device'))
(OUT / 'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
print('PASS Wasm execution and 15 target/region compilation pairs; physical devices remain untested')
