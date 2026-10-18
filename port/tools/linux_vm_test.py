"""Smoke-test a Linux bundle inside a QEMU VM (Debian nocloud image).

The shared folder is exposed read-only as a FAT disk; the VM copies it to
its own disk, runs the commands given with --run, and prints files listed
with --fetch back over the serial console (base64), saved to --out.

    python port/tools/linux_vm_test.py --share D:/ReKHCoM-build/linux/vmshare \
        --run "cd /root/share/ReKHCoM && KHCOM_FAST=1 ./ReKHCoM.sh" --fetch /root/share/ReKHCoM/kh_com.log
"""
import argparse
import base64
import queue
import re
import subprocess
import sys
import threading
import time
from pathlib import Path

QEMU = Path('D:/ReKHCoM-build/toolchain/qemu/runtime')
IMAGES = {
    'x86_64': Path('D:/ReKHCoM-build/linux/vm/debian-12-nocloud-amd64.qcow2'),
    'aarch64': Path('D:/ReKHCoM-build/linux/vm/debian-12-nocloud-arm64.qcow2'),
}


class Console:
    def __init__(self, proc):
        self.proc = proc
        self.q = queue.Queue()
        self.buf = ''
        threading.Thread(target=self._pump, daemon=True).start()

    def _pump(self):
        while True:
            b = self.proc.stdout.read(1)
            if not b:
                self.q.put(None)
                return
            self.q.put(b.decode('latin-1'))

    def expect(self, pattern, timeout):
        end = time.time() + timeout
        while time.time() < end:
            m = re.search(pattern, self.buf)
            if m:
                out, self.buf = self.buf[:m.end()], self.buf[m.end():]
                return out
            try:
                c = self.q.get(timeout=0.5)
            except queue.Empty:
                continue
            if c is None:
                raise SystemExit('QEMU exited:\n' + self.buf[-2000:])
            self.buf += c
        raise SystemExit(f'timeout waiting for {pattern!r}:\n' + self.buf[-3000:])

    def send(self, text):
        # The emulated UART drops input that arrives faster than it drains.
        data = text.encode()
        for i in range(0, len(data), 8):
            self.proc.stdin.write(data[i:i + 8])
            self.proc.stdin.flush()
            time.sleep(0.03)

    def run(self, cmd, timeout=600):
        marker = f'__DONE_{int(time.time() * 1000)}__'
        self.send(f'{cmd}; echo {marker}$?\n')
        out = self.expect(marker + r'(\d+)', timeout)
        code = int(re.search(marker + r'(\d+)', out).group(1))
        return out, code


def main():
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--machine', choices=IMAGES, default='x86_64')
    ap.add_argument('--share', type=Path, required=True)
    ap.add_argument('--run', action='append', default=[])
    ap.add_argument('--fetch', action='append', default=[])
    ap.add_argument('--out', type=Path, default=Path('.'))
    ap.add_argument('--accel', default='whpx,kernel-irqchip=off')
    args = ap.parse_args()
    # The shared folder travels as a tar archive used as a raw disk (tar
    # blocks are 512 bytes): fast, and keeps the executable bits.
    import tarfile
    image = args.share.with_suffix('.tar')
    with tarfile.open(image, 'w', format=tarfile.GNU_FORMAT) as t:
        for f in sorted(args.share.rglob('*')):
            info = t.gettarinfo(f, f.relative_to(args.share).as_posix())
            info.mode = 0o755 if f.is_dir() or f.suffix in ('', '.sh') or '.so' in f.name else 0o644
            if f.is_file():
                with open(f, 'rb') as fh:
                    t.addfile(info, fh)
            else:
                t.addfile(info)
    with open(image, 'ab') as fh:
        fh.write(b'\0' * (-fh.tell() % 1048576))
    share = str(image).replace('\\', '/')
    if args.machine == 'x86_64':
        cmd = [str(QEMU / 'qemu-system-x86_64.exe'), '-m', '2048', '-smp', '2', '-accel', args.accel, '-accel', 'tcg',
               '-drive', f'file={IMAGES["x86_64"]},if=virtio,snapshot=on',
               '-drive', f'file={share},format=raw,if=virtio,readonly=on', '-nographic', '-nic', 'none']
    else:
        cmd = [str(QEMU / 'qemu-system-aarch64.exe'), '-M', 'virt', '-cpu', 'cortex-a53', '-m', '1024', '-smp', '2',
               '-bios', str(QEMU / 'share/edk2-aarch64-code.fd'),
               '-drive', f'file={IMAGES["aarch64"]},if=virtio,snapshot=on',
               '-drive', f'file={share},format=raw,if=virtio,readonly=on', '-nographic', '-nic', 'none']
    proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    con = Console(proc)
    try:
        con.expect(r'login: ', 900)
        con.send('root\n')
        con.expect(r'# ', 120)
        con.send('stty -echo cols 250; export PS1="# "\n')
        con.expect(r'# ', 30)
        for step in ('mkdir -p /root/share && tar -xf /dev/vdb -C /root/share', 'ls /root/share', 'uname -m'):
            out, code = con.run(step, 300)
            print(f'# {step} [exit {code}]\n{out.strip()[-600:]}', flush=True)
        for c in args.run:
            out, code = con.run(c, 1800)
            print(f'$ {c}\n{out.strip()}\n[exit {code}]', flush=True)
        args.out.mkdir(parents=True, exist_ok=True)
        for f in args.fetch:
            out, code = con.run(f'echo BEGIN64; base64 -w0 {f}; echo; echo END64')
            m = re.search(r'BEGIN64\s*([A-Za-z0-9+/=\s]*?)\s*END64', out)
            if code == 0 and m:
                data = base64.b64decode(re.sub(r'\s', '', m.group(1)))
                (args.out / Path(f).name).write_bytes(data)
                print('fetched', f, len(data), 'bytes')
            else:
                print('could not fetch', f)
        con.send('poweroff\n')
        proc.wait(120)
    finally:
        if proc.poll() is None:
            proc.kill()


if __name__ == '__main__':
    main()
