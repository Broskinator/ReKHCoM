"""The launcher rewrites kh_com.ini: every key the game saves must be in its
list, or turning an option on in the game is lost after the next launch
(experimental_save_point_slot_select was, 18/10/2026).

usage: python port/tools/test_settings_keys.py   (from the repo root)
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    config = (ROOT / 'port/src/port_config.c').read_text(encoding='utf-8')
    launcher = (ROOT / 'port/decompui/decompui.cpp').read_text(encoding='utf-8')
    start = launcher.index('kSettings[] = {')
    table = launcher[start:launcher.index('};', start)]
    game = set(re.findall(r'\{ "([a-z0-9_]+)", &gPortConfig', config))
    known = set(re.findall(r'\{ "([a-z0-9_]+)",', table))
    missing = sorted(game - known)
    if missing:
        print('FAIL: the launcher drops these game settings:', ', '.join(missing))
        return 1
    if 'known = known || kv.first == s.key' not in launcher:
        print('FAIL: SaveIni no longer keeps keys it does not know')
        return 1
    print(f'PASS: {len(game)} game settings are kept by the launcher')
    return 0


if __name__ == '__main__':
    sys.exit(main())
