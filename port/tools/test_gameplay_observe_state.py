#!/usr/bin/env python3
"""Read native KHST checkpoints through their exact executable's PE symbols.

Never writes to an executable, state, process or save. Rejects another build
with the same FNV layout/code check used by PortStateService.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

REGIONS = ((0x02000000, 0x40000), (0x03000000, 0x8000), (0x04000000, 0x400),
           (0x05000000, 0x400), (0x06000000, 0x18000), (0x07000000, 0x400))


class PE:
    def __init__(self, path):
        self.raw = Path(path).read_bytes()
        pe = struct.unpack_from('<I', self.raw, 0x3c)[0]
        if self.raw[pe:pe+4] != b'PE\0\0':
            raise ValueError('Expected PE executable')
        machine, count, _, symbol_offset, symbol_count, optional_size, _ = struct.unpack_from('<HHIIIHH', self.raw, pe+4)
        if machine != 0x14c or struct.unpack_from('<H', self.raw, pe+24)[0] != 0x10b:
            raise ValueError('Expected native 32-bit PE')
        self.base = struct.unpack_from('<I', self.raw, pe+24+28)[0]
        self.sections = []
        for i in range(count):
            offset = pe+24+optional_size+i*40
            name, size, rva, raw_size, raw_offset = struct.unpack_from('<8sIIII', self.raw, offset)
            self.sections.append(dict(name=name.rstrip(b'\0').decode(), size=size, address=self.base+rva,
                                      raw_size=raw_size, raw_offset=raw_offset))
        strings = symbol_offset+symbol_count*18
        self.symbols = {}
        i = 0
        while i < symbol_count:
            offset = symbol_offset+i*18
            name, value, section, _, _, auxiliaries = struct.unpack_from('<8sIhHBB', self.raw, offset)
            if name[:4] == b'\0'*4:
                start = strings+struct.unpack_from('<I', name, 4)[0]
                name = self.raw[start:self.raw.index(0, start)]
            else:
                name = name.rstrip(b'\0')
            if section > 0:
                self.symbols[name.decode(errors='replace')] = self.sections[section-1]['address']+value
            i += 1+auxiliaries

    def read(self, address, size):
        for section in self.sections:
            delta = address-section['address']
            if 0 <= delta and delta+size <= max(section['size'], section['raw_size']):
                available = max(0, min(size, section['raw_size']-delta))
                start = section['raw_offset']+delta
                return self.raw[start:start+available]+bytes(size-available)
        raise ValueError(f'Address {address:#x} absent from executable')

    def build_id(self):
        data = next(s for s in self.sections if s['name']=='khdata')
        bss = next(s for s in self.sections if s['name']=='khbss')
        start = self.base+0x1000
        code = self.read(start, self.symbols['_PortStateService']-start)
        value = 2166136261
        for byte in code:
            value = ((value ^ byte)*16777619) & 0xffffffff
        return (value ^ data['address'] ^ data['size']*31 ^ bss['address'] ^ bss['size']*7 ^ 1) & 0xffffffff


class State:
    def __init__(self, exe, state):
        self.pe = PE(exe)
        self.raw = Path(state).read_bytes()
        magic, version, self.build = struct.unpack_from('<III', self.raw)
        if magic != 0x5453484b or version != 1 or self.build != self.pe.build_id():
            raise ValueError('KHST is incompatible with supplied executable')
        self.regions = []
        offset = 12
        for address, size in REGIONS:
            self.regions.append((address, size, offset))
            offset += size
        for name in ('khdata', 'khbss'):
            section = next(s for s in self.pe.sections if s['name']==name)
            self.regions.append((section['address'], section['size'], offset))
            offset += section['size']
        if offset > len(self.raw):
            raise ValueError('Truncated KHST memory sections')

    def read(self, address, size):
        for base, length, offset in self.regions:
            if base <= address and address+size <= base+length:
                return self.raw[offset+address-base:offset+address-base+size]
        return self.pe.read(address, size)

    def value(self, address, format='<I'):
        return struct.unpack(format, self.read(address, struct.calcsize(format)))[0]

    def symbol(self, name):
        return self.pe.symbols['_'+name]

    def snapshot(self):
        game = self.symbol('gGameState')
        floor = self.symbol('gMapFloorState')
        mode = self.value(self.symbol('gCurrentMode'))
        name_ptr = self.value(mode)
        name = self.read(name_ptr, 64).split(b'\0')[0].decode(errors='replace')
        flags = self.value(game+8)
        field = self.value(self.symbol('gFieldState'))
        report = dict(build_id=f'{self.build:08x}', mode=name, game_flags=flags,
                      actor='riku' if flags & 8 else 'sora',
                      sora_clear=bool(flags&0x20), riku_clear=bool(flags&0x800),
                      floor=self.value(game+0xe,'<b'), world=self.value(game+0xc,'<B'),
                      hp=self.value(game+0x32,'<h'), level=self.value(game+0xf8+0x10,'<B'),
                      exp=self.value(game+0xf8+8), next_exp=self.value(game+0xf8+12),
                      ap=self.value(game+0xf8+6,'<h'), dp=self.value(game+0xf8+4,'<h'),
                      room=self.value(floor+6,'<B'), story=self.value(floor,'<B'),
                      event_step=self.value(floor+5,'<B'), floor_flags=self.value(floor+2,'<H'),
                      logic_frame=self.value(self.symbol('gFrameCounter')))
        report['random_seed'] = self.value(game+4)
        report['map_cards'] = [dict(id=i,count=count) for i,count in enumerate(self.read(self.symbol('gMapCardCounts'),270)) if count]
        if field and name in ('Mode_MapFld','Mode_MapFix','Mode_MapDbg'):
            report['field_position'] = list(struct.unpack('<iiii', self.read(field+0x18,16)))
            report['field_flags'] = self.value(field+0x70)
            doors = self.value(self.pe.symbols['_sMapDoors'])
            report['doors'] = [dict(zip(('flags','cell_x','cell_y','side','room'),
                                        struct.unpack('<HHHBB',self.read(doors+i*8,8)))) for i in range(4)] if doors else []
        battle = self.value(self.symbol('gBtlWork'))
        if battle and name == 'mode_battle':
            flags = self.value(battle+104,'<Q')
            report['battle'] = dict(flags=f'{flags:016x}', phase=self.value(battle+160,'<i'),
                                    step=self.value(battle+228,'<h'), id=self.value(battle+268),
                                    enemies=self.value(battle+238,'<B'),
                                    pending_enemies=self.value(battle+288,'<h'),
                                    pending_level_ups=self.value(battle+250,'<B'),
                                    tutorial=bool(flags&0x800000000),
                                    dark_mode=bool(flags&0x800000000000))
            def actor(pointer):
                return dict(kind=self.value(pointer,'<i'),position=list(struct.unpack('<iii',self.read(pointer+4,12))),
                            hp=self.value(pointer+44,'<h'),max_hp=self.value(pointer+46,'<h'),
                            flags=f"{self.value(pointer+52,'<Q'):016x}")
            player=self.value(battle+124)
            if player:
                report['battle']['player']=actor(player)
            node=self.value(battle+136)
            enemies=[]
            visited=set()
            while node and node not in visited and len(visited)<64:
                visited.add(node)
                owner=self.value(node)
                if owner:
                    enemies.append(actor(owner))
                node=self.value(node+8)
            report['battle']['opponents']=enemies
            selected_name='_sRikuSelectedCard' if report['actor']=='riku' else '_sSoraSelectedCard'
            selected=self.value(self.pe.symbols[selected_name])
            if selected:
                slot=self.value(selected+60)
                if slot:
                    card_id=self.value(slot)
                    report['battle']['selected_card']=dict(id=card_id,value=self.value(selected+165,'<B'),
                                                           reload=card_id==0xfffe)
        return report


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--exe', type=Path, required=True)
    ap.add_argument('--state', type=Path, required=True)
    ap.add_argument('--output', type=Path)
    args = ap.parse_args()
    report = State(args.exe, args.state).snapshot()
    report.update(state_sha256=hashlib.sha256(args.state.read_bytes()).hexdigest(),
                  executable_sha256=hashlib.sha256(args.exe.read_bytes()).hexdigest(), scope='Read-only checkpoint')
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(report,indent=2))
    print(json.dumps(report,indent=2))


if __name__ == '__main__':
    main()
