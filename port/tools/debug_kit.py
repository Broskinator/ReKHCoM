#!/usr/bin/env python3
"""Debug kit: run the automatic test batteries and report every fault, by source line.

The desktop builds recover the original's stray accesses (NULL pointers, reads of the
GBA BIOS area, divisions by zero) and log each one as "bios: ... at eip X" in
kh_com.log. Android, the web and Linux ARM cannot recover them: there the same access
closes the game (Cloud's division by zero, #30's NULL write). So every logged site is a
bug to fix in the game code (BIOS_PTR, DIV0/MOD0 or a guard, following the cartridge's
result), even when the desktop game kept running.

    python port/tools/debug_kit.py run [--versions us,eu,jp] [--suites battles,riku,events,rooms,monkey]
                                       [--jobs N] [--quick] [--build]
    python port/tools/debug_kit.py report        # only scan the results already on disk

run    builds nothing unless --build (ninja for build/pc-<version>), then runs each suite
       with the desktop test builds (hidden windows, no audio, isolated folders):
         battles  every entry of the debug battle menu (test_battles.py)
         riku     the same battles as Riku (Reverse/Rebirth)
         events   every story event of the debug event menu (test_events.py), GBA view
         wide     the same events in the 16:9 view (port-only drawing paths)
         rooms    every world and castle hall, walked (test_rooms.py)
         monkey   random reproducible play from each floor entrance (test_monkey.py)
         scenes   the Android runner's menus and fixtures (every menu mode, doors, saving,
                  Moogle, level up, victory, death...) on the desktop build
       --quick shortens frames/minutes for a fast sweep. --jobs defaults to what free RAM
       allows (about 300 MB per game process), so the PC stays usable.
report scans build/{battletest,eventtest,roomtest_*,monkeytest} and writes
       build/debugkit/report.md and report.json: one row per source line with the kind
       of access, how many cases hit it and which ones; crashes and timeouts apart.
       Each address is symbolized with the exe copy that ran that case. Cases whose exe
       is older than the current build are marked "stale" (the bug may be fixed already).

Android x86 (UBSan, no recovery) is the next check after a clean report:
    python port/tools/build_android.py --abi x86 --smoke --ubsan --smoke-env KHCOM_FAST=1
    python port/tools/test_android_scenarios.py --serial emulator-5554 --regions us --groups menus fixtures bosses
"""
import argparse
import ctypes
import json
import os
import re
import shutil
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
TOOLS = ROOT / 'port/tools'
OUT = ROOT / 'build/debugkit'
SYMBOLIZER = ROOT.parent / 'toolchain/llvm-mingw-20261006-ucrt-x86_64/bin/llvm-symbolizer.exe'
FAULT = re.compile(r'bios: (division by zero|read|write)(?: ([0-9a-f]{8}))? at eip ([0-9a-f]{8})')
CRASH_EIP = re.compile(r'(?:eip=|at )([0-9a-f]{8})')


def free_ram_mb():
    if os.name != 'nt':
        return 4096
    class Status(ctypes.Structure):
        _fields_ = [('dwLength', ctypes.c_ulong), ('dwMemoryLoad', ctypes.c_ulong),
                    ('ullTotalPhys', ctypes.c_ulonglong), ('ullAvailPhys', ctypes.c_ulonglong),
                    ('ullTotalPageFile', ctypes.c_ulonglong), ('ullAvailPageFile', ctypes.c_ulonglong),
                    ('ullTotalVirtual', ctypes.c_ulonglong), ('ullAvailVirtual', ctypes.c_ulonglong),
                    ('ullAvailExtendedVirtual', ctypes.c_ulonglong)]
    s = Status()
    s.dwLength = ctypes.sizeof(Status)
    ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(s))
    return s.ullAvailPhys // (1024 * 1024)


def default_jobs():
    return max(1, min(6, (free_ram_mb() - 1024) // 300))


def build_dir(version):
    return ROOT / f'build/pc-{version}'


def run_scenes(version, jobs):
    """test_android_scenarios.py's menus and fixtures, each in its own hidden desktop process."""
    from concurrent.futures import ThreadPoolExecutor
    sys.path.insert(0, str(TOOLS))
    import test_android_scenarios as scn
    rom = ROOT / 'roms' / (scn.CODES[version] + '.gba')
    work = ROOT / 'build/scenetest' / version
    work.mkdir(parents=True, exist_ok=True)

    def one(item):
        name, frames, timeout, text = item
        d = work / name
        shutil.rmtree(d, ignore_errors=True)
        d.mkdir(parents=True)
        for f in ('khcom.exe', 'SDL3.dll'):
            shutil.copy2(build_dir(version) / f, d / f)
        env = {k: v for k, v in os.environ.items() if not k.startswith(('KHCOM_', 'SDL_'))}
        env.update(SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy', KHCOM_FAST='1', KHCOM_QUIT=str(frames))
        for line in text.splitlines():
            if '=' in line:
                k, v = line.split('=', 1)
                env[k] = v
        try:
            code = subprocess.run([str(d / 'khcom.exe'), '--rom', str(rom), '--language', 'es'], cwd=d, env=env,
                                  stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                                  timeout=timeout * 4).returncode
        except subprocess.TimeoutExpired:
            code = 'timeout'
        return f'{name:40s} {"ok" if code == 0 else code}'

    items = scn.scenarios(['menus', 'fixtures'], [], False)
    with ThreadPoolExecutor(jobs) as pool:
        lines = list(pool.map(one, items))
    bad = sum(not l.endswith(' ok') for l in lines)
    lines.append(f'{len(lines) - bad}/{len(lines)} scenes exited normally')
    (work / 'report.txt').write_text(chr(10).join(lines) + chr(10), encoding='utf-8')
    return lines[-1]


def suite_commands(suite, version, jobs, quick):
    b = ['--build', str(build_dir(version).relative_to(ROOT)), '--version', version, '--jobs', str(jobs)]
    py = [sys.executable, '-u']
    if suite == 'battles':
        return py + [str(TOOLS / 'test_battles.py'), *b] + (['--frames', '1500'] if quick else [])
    if suite == 'riku':
        return py + [str(TOOLS / 'test_battles.py'), *b, '--riku'] + (['--frames', '1500'] if quick else [])
    if suite == 'events':
        return py + [str(TOOLS / 'test_events.py'), *b] + (['--frames', '2000'] if quick else [])
    if suite == 'wide':
        return py + [str(TOOLS / 'test_events.py'), *b, '--wide'] + (['--frames', '2000'] if quick else [])
    if suite == 'rooms':
        return py + [str(TOOLS / 'test_rooms.py'), *b] + (['--frames', '1500'] if quick else [])
    if suite == 'monkey':
        return py + [str(TOOLS / 'test_monkey.py'), *b, '--minutes', '2' if quick else '10']
    raise SystemExit(f'unknown suite {suite}')


def case_dirs():
    """(suite, version, case, folder) for every case folder left by the batteries."""
    out = []
    build = ROOT / 'build'
    for base, suite in ((build / 'battletest', 'battles'), (build / 'eventtest', 'events'),
                        (build / 'monkeytest', 'monkey'), (build / 'scenetest', 'scenes')):
        if not base.is_dir():
            continue
        for vdir in sorted(p for p in base.iterdir() if p.is_dir()):
            name = suite
            version = vdir.name
            if version.endswith('_riku'):
                name, version = 'riku', version[:-5]
            elif version.endswith('_wide'):
                name, version = 'wide', version[:-5]
            for case in sorted(p for p in vdir.iterdir() if p.is_dir()):
                out.append((name, version, case.name, case))
    for vdir in sorted(build.glob('roomtest_*')):
        for case in sorted(p for p in vdir.iterdir() if p.is_dir()):
            out.append(('rooms', vdir.name[len('roomtest_'):], case.name, case))
    return out


def symbolize(exe, addresses):
    """{address: (function, file:line)} using the exe that produced them."""
    if not addresses:
        return {}
    tool = str(SYMBOLIZER) if SYMBOLIZER.exists() else (shutil.which('llvm-symbolizer') or '')
    if not tool:
        return {a: ('?', '?') for a in addresses}
    ordered = sorted(addresses)
    res = subprocess.run([tool, f'--obj={exe}', *('0x' + a for a in ordered)],
                         capture_output=True, text=True)
    blocks = [b.strip().splitlines() for b in res.stdout.strip().split('\n\n')]
    result = {}
    for a, lines in zip(ordered, blocks):
        func = lines[0] if lines else '?'
        where = lines[1] if len(lines) > 1 else '?'
        where = where.replace('\\', '/')
        m = re.search(r'/(src|port|include)/', where)
        if m:
            where = where[m.start() + 1:]
        where = re.sub(r':(\d+):\d+$', r':\1', where)
        result[a] = (func, where)
    return result


def source_hint(where):
    """'@bug' notes or guards near the line, so known original bugs stand out."""
    m = re.match(r'(.+):(\d+)$', where)
    if not m or not (ROOT / m.group(1)).exists():
        return ''
    lines = (ROOT / m.group(1)).read_text(encoding='utf-8', errors='replace').splitlines()
    line = int(m.group(2))
    near = lines[max(0, line - 4):line]
    for text in reversed(near):
        if '@bug' in text:
            return text.strip()
    return ''


def report():
    sites = defaultdict(lambda: {'kinds': set(), 'targets': set(), 'cases': [], 'stale': 0})
    crashes, scanned = [], 0
    current = {v: (build_dir(v) / 'khcom.exe') for v in ('eu', 'us', 'jp')}
    for suite, version, case, folder in case_dirs():
        log = folder / 'kh_com.log'
        exe = folder / 'khcom.exe'
        if not log.exists() or not exe.exists():
            continue
        scanned += 1
        text = log.read_text(encoding='utf-8', errors='replace')
        cur = current.get(version)
        stale = bool(cur and cur.exists() and exe.stat().st_mtime < cur.stat().st_mtime - 5)
        found = {}
        for kind, target, eip in FAULT.findall(text):
            found.setdefault(eip, set()).add((kind, target))
        crash_lines = [l for l in text.splitlines() if 'CRASH' in l or 'FATAL' in l]
        crash_eips = set()
        for l in crash_lines:
            crash_eips.update(CRASH_EIP.findall(l))
        names = symbolize(exe, set(found) | crash_eips)
        for eip, kinds in found.items():
            func, where = names.get(eip, ('?', '?'))
            s = sites[(func, where)]
            for kind, target in kinds:
                s['kinds'].add(kind)
                if target:
                    s['targets'].add(target)
            if f'{suite}/{version}/{case}' not in s['cases']:
                s['cases'].append(f'{suite}/{version}/{case}')
                s['stale'] += stale
        if crash_lines:
            crashes.append({'case': f'{suite}/{version}/{case}', 'stale': stale, 'lines': crash_lines[:3],
                            'at': sorted({' '.join(names.get(e, ('?', '?'))) for e in crash_eips})})
    rows = []
    for (func, where), s in sorted(sites.items(), key=lambda kv: -len(kv[1]['cases'])):
        rows.append({'function': func, 'source': where, 'kinds': sorted(s['kinds']),
                     'addresses': sorted(s['targets']), 'hits': len(s['cases']),
                     'all_stale': s['stale'] == len(s['cases']), 'cases': s['cases'][:12],
                     'note': source_hint(where)})
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'report.json').write_text(json.dumps({'scanned': scanned, 'sites': rows, 'crashes': crashes}, indent=2),
                                     encoding='utf-8')
    md = ['# Debug kit report', '', f'{scanned} case logs scanned. {len(rows)} fault sites, {len(crashes)} crashes.', '',
          'Every site closes the game on Android, the web and Linux ARM. "stale" = only seen with an exe older',
          'than the current build (rerun before fixing).', '']
    if rows:
        md += ['| Source | Function | Access | Hits | Cases |', '|---|---|---|---|---|']
        for r in rows:
            access = ', '.join(r['kinds']) + (f" ({', '.join(r['addresses'][:3])})" if r['addresses'] else '')
            md.append(f"| `{r['source']}` | {r['function']} | {access} | {r['hits']}{' stale' if r['all_stale'] else ''} | "
                      f"{', '.join(r['cases'][:4])}{' ...' if r['hits'] > 4 else ''} |")
    if crashes:
        md += ['', '## Crashes and fatal errors', '']
        for c in crashes:
            md.append(f"- {c['case']}{' (stale)' if c['stale'] else ''}: {'; '.join(c['at']) or c['lines'][0]}")
    (OUT / 'report.md').write_text('\n'.join(md) + '\n', encoding='utf-8')
    print('\n'.join(md))
    return rows, crashes


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('command', choices=('run', 'report'))
    ap.add_argument('--versions', default='us,eu,jp')
    ap.add_argument('--suites', default='battles,riku,events,wide,rooms,monkey,scenes')
    ap.add_argument('--jobs', type=int, default=0)
    ap.add_argument('--quick', action='store_true')
    ap.add_argument('--build', action='store_true', help='ninja the desktop test builds first')
    args = ap.parse_args()
    if args.command == 'run':
        versions = [v for v in args.versions.split(',') if v]
        env = dict(os.environ, PATH=str(ROOT.parent / 'toolchain/bin') + os.pathsep + os.environ.get('PATH', ''))
        if args.build:
            for v in versions:
                if not (build_dir(v) / 'build.ninja').exists():
                    subprocess.run([sys.executable, 'port/configure_pc.py', '--version', v, '--out', f'build/pc-{v}'],
                                   cwd=ROOT, check=True, env=env)
                subprocess.run([shutil.which('ninja', path=env['PATH']) or 'ninja', '-f', str(build_dir(v) / 'build.ninja')], cwd=ROOT, check=True, env=env)
        OUT.mkdir(parents=True, exist_ok=True)
        for v in versions:
            if not (build_dir(v) / 'khcom.exe').exists():
                raise SystemExit(f'{build_dir(v)}/khcom.exe missing: run with --build')
            for suite in [s for s in args.suites.split(',') if s]:
                jobs = args.jobs or default_jobs()
                if suite == 'scenes':
                    print(f'== scenes {v} (jobs {jobs})', flush=True)
                    print(run_scenes(v, jobs), flush=True)
                    continue
                cmd = suite_commands(suite, v, jobs, args.quick)
                print(f'== {suite} {v} (jobs {jobs})', flush=True)
                with open(OUT / f'{suite}-{v}.txt', 'w', encoding='utf-8') as log:
                    subprocess.run(cmd, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, env=env)
                print((OUT / f'{suite}-{v}.txt').read_text(encoding='utf-8', errors='replace').strip().splitlines()[-1:],
                      flush=True)
    report()


if __name__ == '__main__':
    main()
