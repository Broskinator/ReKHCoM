"""Runs the long test batteries one after another, resumable, outside the session that starts them.

    python port/tools/run_batteries.py --detach     # starts itself through WMI and returns at once

A battery started as a child of an agent session dies when that session ends (Windows ends the whole process tree:
adb fails with 0xC0000142, sh cannot fork, ninja gets 0x40010004). With --detach the process is created by WMI,
outside that tree, so it keeps going; each step runs with --resume, so starting it again carries on where it stopped.
Progress: build/batteries.log; a step's own output is in its log (build/debugkit/*.txt, build/android-scenarios).
"""
import argparse
import re
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LOG = ROOT / 'build/batteries.log'
SDK = ROOT.parent / 'toolchain/android/sdk'
ADB = SDK / 'platform-tools/adb.exe'
PY = sys.executable
SCN = 'port/tools/test_android_scenarios.py'

# desktop first (what the user asked for), then the Android emulator: both together do not fit in the RAM
STEPS = [
    ('desktop events wide rooms', [PY, 'port/tools/debug_kit.py', 'run', '--build', '--resume', '--jobs', '2',
                                     '--versions', 'eu,us,jp', '--suites', 'events,wide,rooms']),
    ('android', None),
    ('android eu bosses', [PY, SCN, '--serial', 'emulator-5554', '--regions', 'eu', '--groups', 'bosses',
                           '--from-battle', '30', '--resume', '--slow', '6']),
    ('android eu bosses riku', [PY, SCN, '--serial', 'emulator-5554', '--regions', 'eu', '--groups', 'bosses',
                                '--riku', '--resume', '--slow', '6']),
    ('android us/jp menus bosses', [PY, SCN, '--serial', 'emulator-5554', '--regions', 'us', 'jp', '--groups',
                                    'menus', 'bosses', '--resume', '--slow', '6']),
]


def say(text):
    with open(LOG, 'a', encoding='utf-8') as f:
        f.write(time.strftime('%d/%m %H:%M:%S ') + text + '\n')


def emulator():
    """The Android 14 emulator (Pixel6_API34), started if it is not running, ready when it says boot completed."""
    def booted():
        r = subprocess.run([str(ADB), '-s', 'emulator-5554', 'shell', 'getprop', 'sys.boot_completed'],
                           capture_output=True, text=True)
        return r.stdout.strip() == '1'
    if booted():
        return True
    subprocess.Popen([str(SDK / 'emulator/emulator.exe'), '-avd', 'Pixel6_API34', '-no-window', '-no-audio',
                      '-no-snapshot', '-gpu', 'swiftshader_indirect', '-memory', '3072'],
                     stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, creationflags=0x08000000)
    for _ in range(120):
        time.sleep(5)
        if booted():
            time.sleep(20)
            return True
    return False


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--detach', action='store_true')
    args = ap.parse_args()
    if args.detach:
        cmd = f'"{PY}" "{Path(__file__).resolve()}"'
        ps = (f"$s=New-CimInstance -ClassName Win32_ProcessStartup -ClientOnly -Property @{{ShowWindow=[uint16]0}};"
              f"$r=Invoke-CimMethod -ClassName Win32_Process -MethodName Create -Arguments "
              f"@{{CommandLine='{cmd}';CurrentDirectory='{ROOT}';ProcessStartupInformation=$s}};"
              f"'pid ' + $r.ProcessId + ' result ' + $r.ReturnValue")
        print(subprocess.run(['powershell', '-NoProfile', '-Command', ps], capture_output=True, text=True).stdout.strip())
        return
    say('start')
    for name, cmd in STEPS:
        if cmd is None:
            ok = emulator()
            say(f'emulator {"ready" if ok else "did not boot"}')
            if not ok:
                break
            continue
        say(f'{name}: running')
        with open(ROOT / 'build' / ('battery-' + re.sub(r'[^a-z0-9]+', '_', name) + '.log'),
                  'a', encoding='utf-8') as out:
            code = subprocess.run(cmd, cwd=ROOT, stdout=out, stderr=subprocess.STDOUT,
                                  creationflags=0x08000000).returncode
        say(f'{name}: exit {code}')
    say('BATTERIES-DONE')


if __name__ == '__main__':
    main()
