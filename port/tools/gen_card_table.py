"""Card table for the launcher's save editor: every battle card id (kind,
value, category, CP cost, flags) and where its small collection sprite and
palette sit in each cartridge, read from the decompilation itself
(src/card/card_catalog.c, include/card/card_ids.h, config/assets).

usage: python port/tools/gen_card_table.py   (from the repo root)
writes port/decompui/card_table.h
"""
import re
from pathlib import Path

import yaml

ROOT = Path(__file__).resolve().parents[2]
VERSIONS = ('eu', 'us', 'jp')  # the launcher's order


def kinds():
    text = (ROOT / 'include/card/card_ids.h').read_text(encoding='utf-8')
    first = text.index('CARD_KIND_KINGDOM_KEY')
    body = text[text.rfind('{', 0, first) + 1:text.index('}', first)]
    names, value = {}, -1
    for item in body.split(','):
        item = item.split('//')[0].strip()
        if not item.startswith('CARD_KIND_'):
            continue
        if '=' in item:
            name, number = (s.strip() for s in item.split('='))
            value = int(number, 0)
        else:
            name, value = item, value + 1
        names.setdefault(name, value)
    return names


def pretty(kind):
    return ' '.join(w.capitalize() for w in kind[len('CARD_KIND_'):].split('_'))


def sprite_addresses():
    """symbol -> {version: address} for small card tiles and palettes"""
    out = {}
    data = yaml.safe_load((ROOT / 'config/assets/sprites_card_pictures.yaml').read_text(encoding='utf-8'))
    entries = data['entries']
    for e in entries:
        if e.get('format') != 'sprite_sheet' or not str(e.get('name', '')).endswith('SmallCardTiles'):
            continue
        skip = 0
        for frame in e['frames']:
            pieces = len(frame.get('pieces', []))
            skip += 2 + 6 * pieces + (2 if pieces else 0)
        for anim in e.get('animations', []):
            skip += 6 + 4 * len(anim['frames'])
        out[e['name']] = {v: e[v]['address'] + skip if v in e else 0 for v in VERSIONS}
    pal = yaml.safe_load((ROOT / 'config/assets/sprite_palettes.yaml').read_text(encoding='utf-8'))
    for e in pal['entries']:
        if str(e.get('name', '')).endswith('SmallCardPalette'):
            out[e['name']] = {v: e[v]['address'] if v in e else 0 for v in VERSIONS}
    return out


FLAGS = {'CARD_DEF_FLAG_ITEM': 0x2, 'CARD_DEF_FLAG_SUMMON': 0x4, 'CARD_DEF_FLAG_FRIEND': 0x8,
         'CARD_DEF_FLAG_GIMMICK': 0x10}


def number(text):
    total = 0
    for part in text.split('|'):
        part = part.strip().strip('()')
        total |= FLAGS[part] if part in FLAGS else int(part, 0)
    return total


def preprocess(text, version):
    """the file's #if VERSION_* blocks, resolved for one cartridge"""
    out, stack = [], []  # stack of (this branch taken, any branch taken)
    macro = 'VERSION_' + version.upper()

    def test(line):
        names = re.findall(r'VERSION_[A-Z]+', line)
        hit = macro in names
        return not hit if line.startswith('#ifndef') else hit

    for line in text.splitlines():
        t = line.strip()
        if t.startswith(('#ifdef', '#ifndef', '#if ')):
            ok = test(t)
            stack.append([ok, ok])
        elif t.startswith('#elif'):
            ok = not stack[-1][1] and test(t)
            stack[-1] = [ok, stack[-1][1] or ok]
        elif t.startswith('#else'):
            stack[-1] = [not stack[-1][1], True]
        elif t.startswith('#endif'):
            stack.pop()
        elif all(level[0] for level in stack):
            out.append(line)
    return '\n'.join(out)


def cards(kind_values, version):
    text = preprocess((ROOT / 'src/card/card_catalog.c').read_text(encoding='utf-8'), version)
    body = text[text.index('const CardDef gCardDefs[950] = {'):]
    body = body[body.index('{') + 1:]
    rows, depth, start = [], 0, 0
    for i, ch in enumerate(body):
        if ch == '{':
            depth += 1
            if depth == 1:
                start = i + 1
        elif ch == '}':
            depth -= 1
            if depth == 0:
                rows.append(body[start:i])
            elif depth < 0:
                break
    out = []
    for row in rows[:950]:
        fields = [f.strip() for f in re.sub(r'\{[^{}]*\}', '', row).split(',') if f.strip()]
        small_tiles, small_pal = fields[5], fields[6]
        kind, flags, value = fields[7], number(fields[8]), number(fields[9])
        category, cost = fields[12], number(fields[13])
        out.append((kind_values.get(kind, -1), kind, flags, value, category, cost, small_tiles, small_pal))
    if len(out) != 950:
        raise SystemExit(f'error: {len(out)} card definitions, expected 950')
    return out


def main():
    kind_values = kinds()
    addresses = sprite_addresses()
    tables = [cards(kind_values, v) for v in VERSIONS]
    table = tables[0]
    for other in tables[1:]:
        for a, b in zip(table, other):
            if a[:6] != b[:6]:
                raise SystemExit(f'error: card fields differ between versions: {a} {b}')
    sprites = []
    index = {}
    for t in tables:
        for c in t:
            key = (c[6], c[7])
            if key not in index and c[6] in addresses and c[7] in addresses:
                index[key] = len(sprites)
                sprites.append(key)
    categories = {'CARD_CATEGORY_ATTACK': 0, 'CARD_CATEGORY_MAGIC': 1, 'CARD_CATEGORY_ITEM': 2, 'CARD_CATEGORY_ENEMY': 3}
    lines = [
        '/* Generated by port/tools/gen_card_table.py from the decompilation; do not edit. */',
        '#ifndef REKHCOM_CARD_TABLE_H',
        '#define REKHCOM_CARD_TABLE_H',
        'namespace RekhcomCards {',
        '/* sprite: index in kSprites for each cartridge (eu, us, jp), -1 none */',
        'struct Card { short kind; unsigned short flags; unsigned char value, category; unsigned short cost; short sprite[3]; };',
        '/* ROM offsets (from 0x08000000) of a 16x32 sprite: 256 bytes of 4bpp tiles, 32 of palette */',
        'struct Sprite { unsigned tiles[3], palette[3]; };',
        f'static const int kCardCount = {len(table)};',
        f'static const int kKindCount = {max(kind_values.values()) + 1};',
        'static const Card kCards[kCardCount] = {',
    ]
    for i, c in enumerate(table):
        sprite = ', '.join(str(index.get((t[i][6], t[i][7]), -1)) for t in tables)
        lines.append(f'    {{ {c[0]}, 0x{c[2]:x}, {c[3]}, {categories.get(c[4], 255)}, {c[5]}, {{ {sprite} }} }},')
    lines.append('};')
    lines.append(f'static const Sprite kSprites[{len(sprites)}] = {{')
    for tiles, pal in sprites:
        t = addresses[tiles]
        p = addresses[pal]
        lines.append('    { { ' + ', '.join(f'0x{max(t[v] - 0x08000000, 0):07X}' for v in VERSIONS) + ' }, { ' +
                     ', '.join(f'0x{max(p[v] - 0x08000000, 0):07X}' for v in VERSIONS) + ' } },')
    lines.append('};')
    names = {}
    for name, value in kind_values.items():
        names.setdefault(value, pretty(name))
    lines.append('static const char* const kKindNames[kKindCount] = {')
    for k in range(max(kind_values.values()) + 1):
        lines.append(f'    "{names.get(k, "")}",')
    lines.append('};')
    ids = (ROOT / 'include/card/card_ids.h').read_text(encoding='utf-8')
    groups = ids[ids.index('enum MapCardGroup {'):]
    groups = re.findall(r'MAP_CARD_GROUP_(\w+)', groups[:groups.index('};')])
    regular = groups.index('KEY_OF_BEGINNINGS')
    lines.append(f'/* map cards: id = group * 10 + value; the {len(groups) - regular} key groups after these are story items */')
    lines.append(f'static const int kMapGroupCount = {regular};')
    lines.append('static const char* const kMapGroupNames[kMapGroupCount] = {')
    for g in groups[:regular]:
        lines.append(f'    "{" ".join(w.capitalize() for w in g.split("_"))}",')
    lines.append('};')
    lines += ['}', '#endif', '']
    (ROOT / 'port/decompui/card_table.h').write_text('\n'.join(lines), encoding='utf-8')
    print(f'card_table.h: {len(table)} cards, {len(sprites)} sprites')


if __name__ == '__main__':
    main()
