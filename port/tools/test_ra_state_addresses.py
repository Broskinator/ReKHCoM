"""Verify GameState's original RAM address from each cartridge's Thumb code."""
from pathlib import Path
import hashlib
import json
import struct
ROOT = Path(__file__).resolve().parents[2]
expected = {
    'eu': [0x0203A1C0, 0x0203A3F0, 0x0203A690, 0x0203AE68, 0x0203AED0, 0x02034ACC, 0x03006C68, 0x0300749C, 0x02034000],
    'us': [0x02039BB0, 0x02039DE0, 0x0203A080, 0x0203A858, 0x0203A8C0, 0x02034AB0, 0x03006C68, 0x03007498, 0x02034000],
    'jp': [0x02039B20, 0x02039D50, 0x02039FF0, 0x0203A7C8, 0x0203A830, 0x02034A10, 0x03006C68, 0x03007498, 0x02034000],
}
witnesses = [('InitGameState', 4), ('GetActiveDeck', 1), ('InitCardCollection', 4),
             ('InitCardCollection', 5), ('CountOwnedMapCardKinds', 7), ('GetActiveDeckIndex', 0),
             ('VBlankTimerStart', 0), ('ModeRequest', 0), ('GetKeysHeld', 0)]
report = []
for region, code in [('eu', 'B8CP'), ('us', 'B8CE'), ('jp', 'B8CJ')]:
    manifest = ROOT / ('config/jp/funcmap.txt' if region == 'jp' else 'config/eu/funcmap.txt')
    lines = manifest.read_text().splitlines()
    rom = (ROOT / f'roms/{code}.gba').read_bytes()
    for n, (function, register) in enumerate(witnesses):
        row = next(line.split() for line in lines if line.startswith(function + '\t'))
        start = int(row[1 if region == 'us' else 3], 16) - 0x08000000
        refs = []
        for at in range(start, start + int(row[2]), 2):
            op = struct.unpack_from('<H', rom, at)[0]
            if op & 0xF800 == 0x4800 and (op >> 8) & 7 == register:
                literal = ((at + 4) & ~3) + (op & 255) * 4
                refs.append(struct.unpack_from('<I', rom, literal)[0])
        assert expected[region][n] in refs, (region, function, refs)
    report.append(dict(region=region, rom_sha1=hashlib.sha1(rom).hexdigest(),
                       original_addresses=[hex(a) for a in expected[region]],
                       evidence=[f'{name} LDR r{reg}' for name, reg in witnesses]))
out = ROOT / 'build/achievements-test'; out.mkdir(parents=True, exist_ok=True)
(out / 'state-addresses.json').write_text(json.dumps(report, indent=2))
print('PASS original state/card addresses proven in EU/US/JP cartridges; no patch or award submitted')
