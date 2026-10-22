#!/usr/bin/env python3
"""Which story event each Re:CoM HD scene belongs to, by its dialogue: the movie's Spanish subtitles
(hd/subs/<movie>.es.txt, from the player's import) against every event's Spanish text
(assets/eu/text/msg_localized_text.txt, gEvent<NNN>Text<MM>Spanish = EVENT_<NNN>). The Re:CoM script rewrites lines,
so each subtitle line counts by its best similarity to any line of the event.

    python port/tools/match_hd_scenes.py --subs D:/ReKHCoM-build/hd-completo/subs [--check]

--check compares the result with port/src/port_hdscenes.inc. Prints, per movie, the best events and their scores.
"""
import argparse
import re
import unicodedata
from difflib import SequenceMatcher
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def norm(text):
    text = unicodedata.normalize('NFKD', text.replace('\\n', ' ').replace('\n', ' ').lower())
    text = ''.join(c for c in text if not unicodedata.combining(c))
    text = re.sub(r'\{[^}]*\}', ' ', text)
    return ' '.join(re.findall(r'[a-z0-9]+', text))


def event_texts():
    src = (ROOT / 'assets/eu/text/msg_localized_text.txt').read_text(encoding='utf-8')
    events = {}
    for label, body in re.findall(r'@gEvent(\d+)Text\d+Spanish\n(.*?)(?=\n@|\Z)', src, re.S):
        line = norm(body)
        if len(line) > 8:
            events.setdefault(int(label), []).append(line)
    return events


def movie_lines(path):
    out = []
    for raw in path.read_text(encoding='utf-8', errors='replace').splitlines():
        parts = raw.split(' ', 2)
        if len(parts) == 3 and parts[0].isdigit():
            line = norm(parts[2])
            if len(line) > 8:
                out.append(line)
    return out


def score(lines, event_lines):
    total = 0.0
    for line in lines:
        best = max((SequenceMatcher(None, line, e).ratio() for e in event_lines), default=0)
        total += best if best >= 0.55 else 0
    return total / max(1, len(lines))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--subs', required=True)
    ap.add_argument('--check', action='store_true')
    args = ap.parse_args()
    names = {}
    for line in (ROOT / 'include/evt/event_ids.h').read_text(encoding='utf-8').splitlines():
        m = re.match(r'\s*(EVENT_(\d+)_\w+)', line)
        if m:
            names[int(m.group(2))] = m.group(1)
    current = dict((m, e) for e, m in re.findall(r'\{ (EVENT_\w+), "(zz_event_\d+)" \}',
                                                  (ROOT / 'port/src/port_hdscenes.inc').read_text(encoding='utf-8')))
    events = event_texts()
    for sub in sorted(Path(args.subs).glob('zz_event_*.es.txt')):
        movie = sub.name.split('.')[0]
        lines = movie_lines(sub)
        ranked = sorted(((score(lines, el), n) for n, el in events.items()), reverse=True)[:3]
        best = names.get(ranked[0][1], ranked[0][1])
        mark = ''
        if args.check and movie in current:
            mark = 'ok' if current[movie] == best else f'TABLE SAYS {current[movie]}'
        elif args.check:
            mark = 'NEW'
        print(f'{movie} {len(lines):3d} lines  ' + '  '.join(f'{names.get(n, n)} {s:.2f}' for s, n in ranked) + f'  {mark}')


if __name__ == '__main__':
    main()
