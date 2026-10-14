#!/usr/bin/env python3
"""Read-only SRAM inventory and original Riku floor routes; never edit a save.

Checks both redundant blocks using the game's checksum and signature rules.
An unlocked header is an entry candidate, never proof of campaign completion.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

import yaml

ROOT = Path(__file__).resolve().parents[2]
SIGNATURES = {b'KHCOM_BACKUP_VER00000013': ['eu'], b'KHCOM_BACKUP_VER00000012': ['us', 'jp']}


def checksum(data):
    words = struct.unpack('<' + 'H' * (len(data)//2), data[:len(data)//2*2])
    total = sum(words) + (data[-1] if len(data)%2 else 0)
    total = (total & 0xffff) + (total >> 16)
    return ~(total + (total >> 16)) & 0xffff


def block(data, offset, size):
    raw = data[offset:offset+size]
    if len(raw) != size:
        return None
    signature = raw[:24]
    if signature not in SIGNATURES or checksum(raw) != 0:
        return None
    return raw


def inspect_save(path):
    data = path.read_bytes()
    headers = [(i, block(data, 0x10+i*0x40, 0x40)) for i in range(2)]
    good = [(i, raw) for i, raw in headers if raw]
    result = dict(path=str(path.resolve()), bytes=len(data), sha256=hashlib.sha256(data).hexdigest(),
                  valid_header_copies=[i for i, _ in good], files=[])
    if not good:
        result.update(can_start_new_riku=False, reason='No valid original header')
        return result
    raw = good[0][1]
    flags, language = struct.unpack_from('<HH', raw, 0x1c)
    result.update(header_flags=flags, compatible_signatures=SIGNATURES[raw[:24]], language=language,
                  sora_clear=bool(flags & 1), riku_title=bool(flags & 2), riku_clear=bool(flags & 4),
                  can_start_new_riku=bool(flags & 1), provenance='Existing save; origin not established by bytes')
    for file_index in range(4):
        large = file_index < 2
        base, size, number = (0x2f20, 0xf14, file_index) if large else (0x1ec0, 0x418, file_index-2)
        copies = [(i, block(data, base+number*size*2+i*size, size)) for i in range(2)]
        valid = [(i, payload) for i, payload in copies if payload]
        if not valid:
            continue
        payload = valid[0][1]
        common = 0x1c
        game_flags = struct.unpack_from('<I', payload, common)[0]
        floor, world = struct.unpack_from('<BB', payload, common+0x90)
        level = payload[common+4+0x10]
        hp = struct.unpack_from('<H', payload, common+0x8e)[0]
        result['files'].append(dict(file=file_index, actor='sora' if large else 'riku',
                                   valid_copies=[i for i, _ in valid], flags=game_flags,
                                   floor=floor, world=world, level=level, hp=hp,
                                   can_continue=level > 0))
    return result


def floor_routes():
    entries = {e['name']: e for e in yaml.safe_load((ROOT/'config/assets/map_room_tables.yaml').read_text())['entries']}
    routes = []
    for floor, definition in enumerate(entries['gRikuMapFloorDefs']['elements'][:12]):
        entry, exit_room = definition[:2]
        links = entries[definition[4]]['values']
        queue = [(entry, [])]
        seen = {entry}
        path = None
        for room, steps in queue:
            if room == exit_room:
                path = steps
                break
            for side, neighbor in enumerate(links[room]):
                if neighbor < len(links) and neighbor not in seen:
                    seen.add(neighbor)
                    queue.append((neighbor, steps+[dict(source=room, side=side, destination=neighbor)]))
        doors = entries[definition[5]].get('elements', [])
        routes.append(dict(floor=floor, basement=12-floor, entry=entry, exit=exit_room,
                           rooms=len(links), ordinary_shortest_path=path,
                           event_doors=doors, note='Topology only; story locks and card costs still apply'))
    return routes


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--root', type=Path, action='append', help='Directories to inspect (default build)')
    ap.add_argument('--save', type=Path, action='append', help='Specific original SRAM file')
    ap.add_argument('--output', type=Path, default=ROOT/'build/codex-riku-plan/report.json')
    args = ap.parse_args()
    paths = set(args.save or [])
    for directory in args.root or ([] if args.save else [ROOT/'build']):
        for extension in ('*.sav', '*.srm'):
            paths.update(directory.rglob(extension))
    grouped = {}
    for path in sorted(paths):
        record = inspect_save(path)
        key = record['sha256']
        if key in grouped:
            grouped[key]['equivalent_paths'].append(record['path'])
        else:
            record['equivalent_paths'] = [record['path']]
            grouped[key] = record
    saves = list(grouped.values())
    report = dict(scope='Read-only entry candidates and topology; no runtime or completion assertion',
                  inspected_files=len(paths), distinct_saves=len(saves), saves=saves, riku_floors=floor_routes())
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2))
    print(args.output)
    print(f"{len(paths)} saves, {len(saves)} distinct, {sum(s['can_start_new_riku'] for s in saves)} unlocked headers")
    for record in saves:
        if record['can_start_new_riku'] or any(f['actor']=='riku' for f in record['files']):
            print(json.dumps({k:record[k] for k in ('path','compatible_signatures','header_flags','files')}))


if __name__ == '__main__':
    main()
