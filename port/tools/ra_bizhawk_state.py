"""Read a private BizHawk/mGBA snapshot without starting an emulator.

Exports private RAM fixtures under build/, never modifies the input snapshot.
Formats: BizHawk MGBAHawk.IStatable.cs and mGBA internal/gba/serialize.h.
No account, credentials, networking, or achievement evaluation is involved.
"""
import argparse
import ctypes
import hashlib
import json
from pathlib import Path
import struct
import zipfile
import zlib

LIMIT = 4 * 1024 * 1024

def read_state(path, zstd):
    with zipfile.ZipFile(path) as archive:
        entry = archive.getinfo('Core.bin.zst')
        if entry.file_size > LIMIT:
            raise ValueError('Compressed core exceeds the diagnostic limit')
        encoded = archive.read(entry)
    lib = ctypes.CDLL(str(Path(zstd).resolve()))
    lib.ZSTD_decompress.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t]
    lib.ZSTD_decompress.restype = ctypes.c_size_t
    lib.ZSTD_isError.argtypes = [ctypes.c_size_t]
    lib.ZSTD_isError.restype = ctypes.c_uint
    source = ctypes.create_string_buffer(encoded)
    target = ctypes.create_string_buffer(LIMIT)
    length = lib.ZSTD_decompress(target, LIMIT, source, len(encoded))
    if lib.ZSTD_isError(length):
        raise ValueError('Invalid or oversized Zstandard core')
    decoded = target.raw[:length]
    if len(decoded) < 4:
        raise ValueError('Missing BizHawk binary length')
    core_length = struct.unpack_from('<I', decoded)[0]
    if core_length < 0x61000 or core_length > len(decoded) - 4:
        raise ValueError('Truncated BizHawk core')
    core = decoded[4:4 + core_length]
    magic = struct.unpack_from('<I', core)[0]
    if magic not in (0x01000009, 0x0100000A, 0x0100000B):
        raise ValueError('Unsupported mGBA state revision')
    code = core[0x1C:0x20].decode('ascii')
    if code not in ('B8CP', 'B8CE', 'B8CJ'):
        raise ValueError('Snapshot is not a supported KHCoM cartridge')
    return core, {'cartridge': code, 'state_revision': magic & 255,
                  'rom_crc32': struct.unpack_from('<I', core, 8)[0]}

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('state', type=Path)
    parser.add_argument('--rom', required=True, type=Path)
    parser.add_argument('--zstd', required=True, type=Path)
    parser.add_argument('--out', default='build/ra71-original-state', type=Path)
    args = parser.parse_args()
    core, report = read_state(args.state, args.zstd)
    rom = args.rom.read_bytes()
    if rom[0xAC:0xB0].decode('ascii') != report['cartridge'] or zlib.crc32(rom) != report['rom_crc32']:
        raise ValueError('Snapshot ROM identity/checksum does not match --rom')
    root = Path(__file__).resolve().parents[2]
    out = args.out.resolve()
    if not out.is_relative_to(root / 'build'):
        raise ValueError('Private RAM fixtures must stay inside repository/build')
    out.mkdir(parents=True, exist_ok=True)
    ram = {'iwram.bin': core[0x19000:0x21000], 'ewram.bin': core[0x21000:0x61000]}
    for name, data in ram.items():
        (out / name).write_bytes(data)
    report.update(source_sha256=hashlib.sha256(args.state.read_bytes()).hexdigest(),
                  memory_bytes={name:len(data) for name,data in ram.items()},
                  scope='original cartridge snapshot only; no port parity or trigger validation',
                  awards_submitted=False)
    (out / 'snapshot.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    print('Extracted original '+report['cartridge']+' RAM without running an emulator; no parity claim')

if __name__ == '__main__':
    main()
