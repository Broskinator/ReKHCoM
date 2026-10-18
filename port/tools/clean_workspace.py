#!/usr/bin/env python3
"""Keep the project folder small: delete test evidence, keep what compiling needs.

Dry run by default; --apply deletes. Looks at <worktree>/build of the given
worktrees (default: this one) and at old ReKHCoM-vX test installs in the
project root. Never touches: sources, roms/, backups/, the build caches listed
in KEEP, the newest --keep-installs ReKHCoM-vX folders and their saves.

    python port/tools/clean_workspace.py                    # report for this worktree
    python port/tools/clean_workspace.py --apply
    python port/tools/clean_workspace.py --worktrees ../khcom ../khcom-claude --apply
    python port/tools/clean_workspace.py --installs --keep-installs 3 --apply
"""
import argparse
import fnmatch
import shutil
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
# Build caches and outputs the toolchain reuses (rebuilding them costs minutes).
KEEP = ['pc', 'pc-*', 'rel-*', 'linux-*', 'android-native*', 'android-app', 'android-build', 'web', 'web-*',
        'releases', 'ps5*', 'vita*', 'switch*', 'ps4*', '*.ninja', 'build-all-*.json', 'gen', 'eu', 'us', 'jp',
        'userstates', 'evidence']  # the user's own savestates and the kept evidence


def size(path):
    if path.is_file():
        return path.stat().st_size
    return sum(f.stat().st_size for f in path.rglob('*') if f.is_file())


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--worktrees', nargs='*', type=Path, default=[ROOT])
    ap.add_argument('--apply', action='store_true')
    ap.add_argument('--installs', action='store_true', help='also prune old ReKHCoM-vX folders in the project root')
    ap.add_argument('--keep-installs', type=int, default=3)
    args = ap.parse_args()
    total = 0
    for wt in args.worktrees:
        build = (wt / 'build').resolve()
        if not build.is_dir():
            continue
        for item in sorted(build.iterdir()):
            if any(fnmatch.fnmatch(item.name, pattern) for pattern in KEEP):
                continue
            n = size(item)
            total += n
            print(f'{n / 2**20:9.0f} MB  {item}')
            if args.apply:
                shutil.rmtree(item) if item.is_dir() else item.unlink()
    if args.installs:
        installs = sorted((p for p in ROOT.parent.glob('ReKHCoM-v*') if p.is_dir()), key=lambda p: p.stat().st_mtime)
        for old in installs[:max(0, len(installs) - args.keep_installs)]:
            n = size(old)
            total += n
            print(f'{n / 2**20:9.0f} MB  {old}  (old install; back up its saves first)')
            if args.apply:
                shutil.rmtree(old)
    print(f'{"deleted" if args.apply else "would delete"}: {total / 2**30:.1f} GB')


if __name__ == '__main__':
    main()
