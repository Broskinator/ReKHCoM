#!/usr/bin/env python3
"""Read roomdump.txt (KHCOM_ROOMDUMP=1) and show a room's tile layers as text.

Each distinct tile entry gets one character; entries that fill the room
(void, clear wall) print as '.' / ','; 0000 is ' ' and ffff (no map data) '#'.
Use it to find edge strips, stairs and plain-wall tiles by eye, and --strip to
list the runs of columns whose 4 rows are all valid (candidate repeatable
strips).

  python port/tools/room_ascii.py roomdump.txt --bg 1 --rows 0 40 --cols 80 190
  python port/tools/room_ascii.py roomdump.txt --bg 1 --strip 14
"""
import argparse
from collections import Counter


def load(path):
    lines = open(path).read().split("\n")
    head = lines[0].split()
    layers, cur = {}, None
    for line in lines[1:]:
        if line.startswith("bg "):
            cur = int(line.split()[1])
            layers[cur] = []
        elif line.strip() and cur is not None:
            layers[cur].append(line.split())
    return int(head[1]), int(head[3]), head[5], layers


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("dump")
    ap.add_argument("--bg", type=int, default=1)
    ap.add_argument("--rows", type=int, nargs=2)
    ap.add_argument("--cols", type=int, nargs=2)
    ap.add_argument("--strip", type=int, help="first tile row of a 4-row strip to check")
    a = ap.parse_args()
    cols, rows, void, layers = load(a.dump)
    bg = layers[a.bg]
    count = Counter(e for r in bg for e in r)
    ids = {}

    def ch(e):
        if e == "0000":
            return " "
        if e == "ffff":
            return "#"
        if e.startswith("a11") and e[3] in "01234567":
            return "."
        if count[e] > 150:
            return ","
        return ids.setdefault(e, chr(33 + len(ids) % 90))

    r0, r1 = a.rows or (0, len(bg))
    c0, c1 = a.cols or (0, len(bg[0]))
    print(f"room {cols}x{rows} cells, {len(bg[0])}x{len(bg)} tiles, void {void}")
    for y in range(r0, min(r1, len(bg))):
        s = "".join(ch(e) for e in bg[y][c0:c1])
        if s.strip(". ,"):
            print(f"{y:3d} {s}")
    if a.strip is not None:
        y = a.strip
        runs, st = [], None
        for x in range(len(bg[0])):
            ok = all(bg[y + j][x] != "ffff" for j in range(4))
            if ok and st is None:
                st = x
            if not ok and st is not None:
                runs.append((st, x - st))
                st = None
        if st is not None:
            runs.append((st, len(bg[0]) - st))
        print("valid 4-row runs (start, width):", [r for r in runs if r[1] >= 8])


if __name__ == "__main__":
    main()
