"""Verify GameState's original RAM address from each cartridge's Thumb code."""
from pathlib import Path
import hashlib
import json
import struct
ROOT = Path(__file__).resolve().parents[2]
expected = {
    'eu': [0x0203A1C0, 0x0203A3F0, 0x0203A690, 0x0203AE68, 0x0203AED0, 0x02034ACC, 0x03006C68, 0x0300749C, 0x02034000, 0x02034A28, 0x0203A3E4, 0x0203C9D4],
    'us': [0x02039BB0, 0x02039DE0, 0x0203A080, 0x0203A858, 0x0203A8C0, 0x02034AB0, 0x03006C68, 0x03007498, 0x02034000, 0x02034928, 0x02039DD4, 0x0203C3E4],
    'jp': [0x02039B20, 0x02039D50, 0x02039FF0, 0x0203A7C8, 0x0203A830, 0x02034A10, 0x03006C68, 0x03007498, 0x02034000, 0x02034928, 0x02039D44, 0x0203C354],
}
witnesses = [('InitGameState', 4), ('GetActiveDeck', 1), ('InitCardCollection', 4),
             ('InitCardCollection', 5), ('CountOwnedMapCardKinds', 7), ('GetActiveDeckIndex', 0),
             ('VBlankTimerStart', 0), ('ModeRequest', 0), ('GetKeysHeld', 0), ('BgFxIsBlocked', 0), ('GetSoraListIndex', 0), ('GetPooGaugeFrame', 0)]
# An LDR address alone does not identify the pointed-to object. Bind the
# witness to its C owner as well, so sBgFx cannot regress to gBtlWork again.
effect = (ROOT / 'src/btl/btl_effect.c').read_text(encoding='utf-8')
body = effect.split('u8 BgFxIsBlocked(u8 priority) {', 1)[1].split('\n}', 1)[0]
assert 'sBgFx->priority' in body and 'gBtlWork' not in body
projection = (ROOT / 'port/src/port_ra_state.c').read_text(encoding='utf-8')
assert 'sOriginalState[9], PortRaBgFxWork()' in projection
pooh = (ROOT / 'src/poo/poo.c').read_text(encoding='utf-8')
assert 'frame = 3 - gPoohGauge;' in pooh
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
