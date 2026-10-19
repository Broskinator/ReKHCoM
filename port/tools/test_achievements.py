"""Production local/RA regressions; account/server fixtures never contact RA.

If the user's EU ROM is present, also exercise its real rhash identification
and catalog/evaluation through a synthetic server. No real unlock is submitted.
"""
from pathlib import Path
import json
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
TC = next((ROOT.parent / 'toolchain').glob('llvm-mingw-*'))
CC = TC / 'bin/i686-w64-mingw32-clang.exe'
SDL = ROOT.parent / 'toolchain/SDL3-3.4.18/i686-w64-mingw32'
OUT = ROOT / 'build/achievements-test'
OUT.mkdir(parents=True, exist_ok=True)

subprocess.run([sys.executable, 'port/configure_pc.py', '--version', 'eu', '--out', 'build/ra-eu'], cwd=ROOT, check=True)
subprocess.run([str(ROOT.parent / 'toolchain/bin/ninja.exe'), '-f', 'build/ra-eu/build.ninja',
                '-j', '6', 'build/ra-eu/ReKHCoM.exe'], cwd=ROOT, check=True)

def run(name, sources, flags=(), args=()):
    folder = OUT / name
    folder.mkdir(exist_ok=True)
    exe = folder / (name + '.exe')
    shutil.copy2(SDL / 'bin/SDL3.dll', folder / 'SDL3.dll')
    command = [str(CC), '-O2', '-std=gnu11', '-Wall', '-mno-ms-bitfields',
               '-I' + str(ROOT / 'port/include'), '-I' + str(SDL / 'include'),
               '-I' + str(ROOT / 'port/third_party/rcheevos/include')]
    command += [str(ROOT / source) for source in sources] + list(flags)
    command += ['-L' + str(SDL / 'lib'), '-lSDL3', '-lwinhttp', '-lcrypt32',
                '-Wl,-Bstatic', '-lwinpthread', '-Wl,-Bdynamic', '-o', str(exe)]
    subprocess.run(command, cwd=ROOT, check=True)
    subprocess.run([str(exe), *map(str, args)], cwd=folder, check=True)
    if name.startswith('native-'):
        subprocess.run([str(exe), 'invalidate'], cwd=folder, check=True)

run('memory', ['port/src/port_ra_memory.c', 'port/tools/test_ra_memory.c'])
run('local', ['port/achievements/local_achievements.c', 'port/tools/local_achievements_conformance.c'])
includes = [ROOT / 'include'] + [p for p in (ROOT / 'include').iterdir() if p.is_dir() and p.name != 'gba']
for version in ['eu', 'us', 'jp']:
    run('native-' + version, ['port/src/port_achievements.c', 'port/tools/native_achievements_conformance.c'],
        ['-DPLATFORM_PC', '-DVERSION_' + version.upper(), *['-iquote' + str(p) for p in includes]])

rom = ROOT / 'roms/B8CP.gba'
objects = sorted((ROOT / 'build/ra-eu/ra').glob('*.o'))
run('ra-client', ['port/tools/ra_client_conformance.c'],
    ['-DRC_CLIENT_SUPPORTS_HASH', *map(str, objects)], [rom] if rom.exists() else [])
(OUT / 'report.json').write_text(json.dumps({
    'memory': 'PASS', 'local': 'PASS', 'native_regions': ['eu', 'us', 'jp'],
    'local_triggers_per_region': 16,
    'physical_console_execution': 'NOT TESTED: requires each device',
    'ra_contract': 'PASS', 'eu_rom_hash_and_synthetic_catalog': 'PASS' if rom.exists() else 'SKIPPED: ROM absent',
    'real_account_login': 'NOT TESTED', 'official_set_native_mapping': 'PENDING',
    'physical_controller': 'NOT TESTED',
}, indent=2), encoding='utf-8')
