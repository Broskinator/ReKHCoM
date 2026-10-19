"""Release checklist: run it before and after publishing; any FAIL blocks the release.

    python port/tools/release_check.py v1.9.9            # before: versions and packages
    python port/tools/release_check.py v1.9.9 --post     # after: GitHub release and Pages

Before: the version is the same in the launcher (REKHCOM_VERSION), the web page
and the top CHANGELOG section; every platform's file is in build/releases, newer
than the HEAD commit, and the Windows launcher and game inside the zip carry the
version. After: the release exists with every file, as stable or prerelease as
asked, and GitHub Pages serves the same version (the web is never left behind).
"""
import argparse
import json
import re
import subprocess
import sys
import time
import urllib.request
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
REPO = 'MatyMeatBoy/ReKHCoM'
PAGES = 'https://matymeatboy.github.io/ReKHCoM/'
ap = argparse.ArgumentParser()
ap.add_argument('version')
ap.add_argument('--post', action='store_true')
ap.add_argument('--prerelease', action='store_true', help='expect a prerelease instead of the stable Latest')
ap.add_argument('--skip', nargs='*', default=[], help='platform files allowed to be missing (e.g. ps5)')
args = ap.parse_args()
V = args.version
fails = []


def check(ok, text):
    print(('PASS ' if ok else 'FAIL ') + text)
    if not ok:
        fails.append(text)


FILES = {
    'windows': f'ReKHCoM-{V}-windows-x86.zip', 'linux': f'ReKHCoM-{V}-linux-x86.tar.gz',
    'steamdeck': f'ReKHCoM-{V}-steamdeck.tar.gz', 'anbernic': f'ReKHCoM-{V}-anbernic-alpha.zip',
    'android': f'ReKHCoM-{V}-android-alpha.apk', 'web': f'ReKHCoM-{V}-web-alpha.zip', 'ps5': 'PPSA98511.zip',
}
wanted = {k: v for k, v in FILES.items() if k not in args.skip}

if not args.post:
    launcher = re.search(r'REKHCOM_VERSION\s+"([^"]+)"', (ROOT / 'port/decompui/decompui.cpp').read_text()).group(1)
    check(launcher == V, f'launcher version {launcher}')
    web = (ROOT / 'port/web/index.html').read_text(encoding='utf-8')
    check(f'Web v{V[1:]}' in web or f'WEB {V}' in web, 'web page label')
    top = re.search(r'^## (v[\d.]+)', (ROOT / 'CHANGELOG.md').read_text(encoding='utf-8'), re.M).group(1)
    check(top == V, f'CHANGELOG top section {top}')
    head = int(subprocess.check_output(['git', 'log', '-1', '--format=%ct'], cwd=ROOT, text=True))
    dirty = subprocess.check_output(['git', 'status', '--porcelain', '--untracked-files=no'], cwd=ROOT, text=True)
    check(not dirty.strip(), 'working tree committed (packages come from HEAD)')
    rel = ROOT / 'build/releases'
    for platform, name in wanted.items():
        path = rel / name
        check(path.is_file() and path.stat().st_mtime >= head - 60, f'{platform}: {name} present and built after HEAD')
    win = rel / FILES['windows']
    if win.is_file():
        with zipfile.ZipFile(win) as z:
            names = z.namelist()
            launcher_exe = next(n for n in names if n.endswith('/ReKHCoM.exe'))
            game = next(n for n in names if n.endswith('/bin/khcom_eu.exe'))
            check(V.encode() in z.read(launcher_exe), 'Windows launcher inside the zip shows ' + V)
            check(len(z.read(game)) > 1_000_000, 'Windows game executables inside the zip')
else:
    info = json.loads(subprocess.check_output(['gh', 'release', 'view', V, '--repo', REPO, '--json',
                                               'isPrerelease,isDraft,assets'], text=True))
    check(not info['isDraft'], 'release is public (not a draft)')
    check(info['isPrerelease'] == args.prerelease, 'release marked ' + ('prerelease' if args.prerelease else 'stable'))
    assets = {a['name'] for a in info['assets']}
    for platform, name in wanted.items():
        check(name in assets, f'{platform}: {name} uploaded')
    page = ''
    for attempt in range(10):
        try:
            page = urllib.request.urlopen(PAGES + f'?check={int(time.time())}', timeout=30).read().decode('utf-8', 'replace')
        except Exception as error:  # noqa: BLE001
            page = str(error)
        if V[1:] in page:
            break
        time.sleep(30)
    check(V[1:] in page, f'GitHub Pages serves {V}')
print('\n%s: %d problem(s)' % ('BLOCKED' if fails else 'OK', len(fails)))
sys.exit(1 if fails else 0)
