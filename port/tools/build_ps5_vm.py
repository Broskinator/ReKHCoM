#!/usr/bin/env python3
"""Build the PS5 package (PPSA98511.zip) from Windows, inside the prepared Debian VM.

The PS5 app needs a native Linux host. On this machine the VM image is
D:/ReKHCoM-build/ps5/ps5env.qcow2 (dependencies already installed); this script
packs the committed source (git archive HEAD) and the ROMs, boots it through
port/tools/linux_vm_test.py, runs build_all.py --platform ps5 --package inside
and fetches the zip.

    python port/tools/build_ps5_vm.py                 # version from the launcher
    python port/tools/build_ps5_vm.py --out build/releases

Needs: a clean tree (the source is HEAD), roms/B8CP.gba, B8CE.gba and B8CJ.gba.
Output: <out>/PPSA98511.zip and <out>/PPSA98511.zip.sha256. build_all.py --platform
all calls this automatically when the image exists.
"""
import argparse
import hashlib
import os
import re
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
IMAGE = Path('D:/ReKHCoM-build/ps5/ps5env.qcow2')
SHARE = Path('D:/ReKHCoM-build/ps5/share-release')


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--version', help='release version (default: REKHCOM_VERSION of the launcher)')
    ap.add_argument('--out', type=Path, default=ROOT / 'build/releases')
    ap.add_argument('--mem', default='4096')
    ap.add_argument('--cpus', default='4')
    args = ap.parse_args()
    version = args.version or re.search(r'#define REKHCOM_VERSION\s+"([^"]+)"',
                                        (ROOT / 'port/decompui/decompui.cpp').read_text(encoding='utf-8'))[1]
    if not IMAGE.is_file():
        sys.exit(f'PS5 VM image not found: {IMAGE}')
    for code in ('B8CP', 'B8CE', 'B8CJ'):
        if not (ROOT / 'roms' / f'{code}.gba').is_file():
            sys.exit(f'Missing roms/{code}.gba (the PS5 build needs the three cartridges)')
    if subprocess.check_output(['git', 'status', '--porcelain', '--untracked-files=no'], cwd=ROOT, text=True).strip():
        sys.exit('Commit your changes first: the PS5 package is built from HEAD')
    if SHARE.exists():
        shutil.rmtree(SHARE)
    (SHARE / 'roms').mkdir(parents=True)
    subprocess.check_call(['git', 'archive', 'HEAD', '-o', str(SHARE / 'source.tar')], cwd=ROOT)
    for code in ('B8CP', 'B8CE', 'B8CJ'):
        shutil.copyfile(ROOT / 'roms' / f'{code}.gba', SHARE / 'roms' / f'{code}.gba')
    script = '\n'.join([
        '#!/bin/bash',
        'exec > /root/ps5-release.log 2>&1',
        'set -eo pipefail',
        'rm -rf /root/rk-release && mkdir -p /root/rk-release',
        'tar -xf /root/share/source.tar -C /root/rk-release',
        'cd /root/rk-release && mkdir -p roms && cp /root/share/roms/B8C?.gba roms/',
        'ln -sfn /root/ps5-deps port/ps5/.deps',
        'export RADV_RELEASE_REPO=darkxex/SwanStationPS5',
        f'python3 port/tools/build_all.py --platform ps5 --package --release-version {version} --jobs 3',
        'unzip -t port/ps5/dist/PPSA98511.zip | tail -1',
    ]) + '\n'
    (SHARE / 'build-release.sh').write_bytes(script.encode())
    args.out.mkdir(parents=True, exist_ok=True)
    fetched = args.out / 'ps5-fetch'
    command = [sys.executable, str(ROOT / 'port/tools/linux_vm_test.py'), '--share', str(SHARE),
               '--disk', str(IMAGE), '--net', '--mem', args.mem, '--cpus', args.cpus,
               '--run', 'bash /root/share/build-release.sh; code=$?; tail -40 /root/ps5-release.log; exit $code',
               '--fetch', '/root/rk-release/port/ps5/dist/PPSA98511.zip', '--out', str(fetched)]
    # Git Bash rewrites /root/... arguments into Windows paths unless told not to.
    subprocess.run(command, cwd=ROOT, check=True, env=dict(os.environ, MSYS_NO_PATHCONV='1', PYTHONUTF8='1'))
    package = fetched / 'PPSA98511.zip'
    if not package.is_file():
        sys.exit('The VM did not return PPSA98511.zip: see the log tail above')
    with zipfile.ZipFile(package) as z:
        bad = z.testzip()
        if bad:
            sys.exit(f'Corrupt PS5 package: {bad}')
    final = args.out / 'PPSA98511.zip'
    shutil.copyfile(package, final)
    digest = hashlib.sha256(final.read_bytes()).hexdigest()
    (args.out / 'PPSA98511.zip.sha256').write_text(f'{digest}  PPSA98511.zip\n')
    shutil.rmtree(fetched)
    print(f'PS5 package: {final} sha256 {digest}')


if __name__ == '__main__':
    main()
