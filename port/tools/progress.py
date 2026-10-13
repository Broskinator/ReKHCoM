#!/usr/bin/env python3
"""Generate PROGRESS.md and port/progress/progress.svg.

Function counts are measured: every function of the EU ROM (config/eu/funcmap.txt)
is either compiled from the decompiled C sources or was hand-written ARM in
asm/; the latter count as ported once a C definition exists under port/src.
Subsystem percentages come from port/progress/status.json.
"""
import json
import re
from datetime import date
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
STATUS = ROOT / "port" / "progress" / "status.json"
SVG = ROOT / "port" / "progress" / "progress.svg"
MD = ROOT / "PROGRESS.md"


# Hand-written routines whose C version lives under another name.
ALIASES = {
    "ChnVolSetAsm": "ChnVolSet",
    "clear_modM": "PortClearModM",
    "ld_r3_tp_adr_i": "ReadCmd",
    "ld_r3_tp_adr_i_unchecked": "ReadCmd",
    "SoundMainRAM": "SoundMain",          # mixer merged into SoundMain
    "func_08109AAC": "func_081213CC",     # reached through ARM/Thumb veneers
    "func_08109BE0": "func_081213C4",
    "func_08109C68": "func_081213D4",
    "MovieVideoCodecKeyFrame": "PortMovieDecodeKey",
    "MovieVideoCodecPostProcess": "PortMoviePostProcess",
    "MovieDeltaCodecDecode": "PortMovieDecodeDelta",
    "MovieAudioCodecPcm16": "PortMovieDecodeAudio",
    "MovieAudioCodecNull": "PortMovieDecodeAudio",
    "MovieAudioCodecAdpcm": "PortMovieDecodeAudio",
    # The inherited function map includes movie data/boundary labels.
    # Native routines replace their buffers and runtime patching.
    "MovieVideoCodecStart": "PortMovieSetupVideo",
    "MovieVideoCodecConstants": "PortMovieDecodeKey",
    "MovieVideoCodecEnd": "PortMovieSetupVideo",
    "MovieDeltaCodecStart": "PortMovieDecodeDelta",
    "MovieDeltaCodecOffsets": "Motion",
    "MovieAudioCodecStart": "PortMovieSetupAudio",
    "MovieAudioCodecAdpcmSteps": "PortMovieDecodeAudio",
    "MovieAudioCodecEnd": "PortMovieSetupAudio",
}


def asm_functions():
    names = set()
    for path in (ROOT / "asm").glob("*.s"):
        if path.name in ("header.s",):
            continue
        for m in re.finditer(r"^\s*\.global\s+(\w+)", path.read_text(), re.M):
            names.add(m.group(1))
    return names


def port_definitions():
    defined = set()
    for path in (ROOT / "port" / "src").glob("*.c"):
        text = path.read_text(encoding="utf-8", errors="replace")
        defined.update(re.findall(r"^[A-Za-z_][\w \*]*?\b(\w+)\s*\([^;]*?\)\s*\{", text, re.M))
    return defined


def measure():
    rom_funcs = [line.split()[0] for line in (ROOT / "config" / "eu" / "funcmap.txt").read_text().splitlines()
                 if line.strip()]
    asm = asm_functions()
    ported = port_definitions()
    in_asm = [f for f in rom_funcs if f in asm]
    asm_done = [f for f in in_asm if ALIASES.get(f, f) in ported]
    c_units = len(list((ROOT / "src").rglob("*.c")))
    return {
        "rom_functions": len(rom_funcs),
        "c_functions": len(rom_funcs) - len(in_asm),
        "asm_functions": len(in_asm),
        "asm_ported": len(asm_done),
        "asm_missing": sorted(set(in_asm) - set(asm_done)),
        "c_units": c_units,
    }


def bar_color(pct):
    if pct >= 90:
        return "#2f9e44"
    if pct >= 50:
        return "#e8a317"
    if pct > 0:
        return "#e8590c"
    return "#c92a2a"


def esc(text):
    return text.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def render_svg(status, m, overall):
    subs = status["subsystems"]
    row = 26
    top = 150
    width = 860
    label_w = 270
    bar_w = width - label_w - 90
    height = top + row * len(subs) + 30
    native = m["c_functions"] + m["asm_ported"]
    func_pct = 100.0 * native / m["rom_functions"]
    out = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" '
        f'viewBox="0 0 {width} {height}" font-family="Segoe UI, Helvetica, Arial, sans-serif">',
        f'<rect width="{width}" height="{height}" rx="10" fill="#14161c"/>',
        '<text x="24" y="38" font-size="22" font-weight="700" fill="#f1f3f5">ReKHCoM - PC port progress</text>',
        f'<text x="24" y="60" font-size="13" fill="#adb5bd">Kingdom Hearts: Chain of Memories (EU) - {date.today().isoformat()}</text>',
    ]

    def big_bar(y, label, pct, detail):
        out.append(f'<text x="24" y="{y + 15}" font-size="14" font-weight="600" fill="#f1f3f5">{esc(label)}</text>')
        out.append(f'<rect x="{label_w}" y="{y}" width="{bar_w}" height="20" rx="4" fill="#2b2f38"/>')
        out.append(f'<rect x="{label_w}" y="{y}" width="{bar_w * pct / 100:.1f}" height="20" rx="4" '
                   f'fill="{bar_color(pct)}"/>')
        out.append(f'<text x="{label_w + bar_w + 10}" y="{y + 15}" font-size="14" font-weight="700" '
                   f'fill="#f1f3f5">{pct:.1f}%</text>')
        out.append(f'<text x="{label_w + 8}" y="{y + 15}" font-size="11" fill="#f8f9fa">{esc(detail)}</text>')

    big_bar(78, "Overall port", overall, "weighted by subsystem")
    big_bar(108, "Functions running natively", func_pct,
            f'{native} / {m["rom_functions"]} ({m["asm_ported"]}/{m["asm_functions"]} hand-written ARM ported)')

    for i, s in enumerate(subs):
        y = top + i * row
        pct = s["done"]
        out.append(f'<text x="24" y="{y + 13}" font-size="12" fill="#ced4da">{esc(s["name"])}</text>')
        out.append(f'<rect x="{label_w}" y="{y + 2}" width="{bar_w}" height="14" rx="3" fill="#2b2f38"/>')
        if pct:
            out.append(f'<rect x="{label_w}" y="{y + 2}" width="{bar_w * pct / 100:.1f}" height="14" rx="3" '
                       f'fill="{bar_color(pct)}"/>')
        out.append(f'<text x="{label_w + bar_w + 10}" y="{y + 13}" font-size="12" fill="#ced4da">{pct}%</text>')
    out.append("</svg>")
    return "\n".join(out) + "\n"


def render_md(status, m, overall):
    native = m["c_functions"] + m["asm_ported"]
    lines = [
        "# ReKHCoM progress",
        "",
        "![progress](port/progress/progress.svg)",
        "",
        f"**Overall: {overall:.1f}%** - generated by `python port/tools/progress.py` on {date.today().isoformat()}.",
        "",
        "## Functions",
        "",
        "Counts follow the inherited function map, including movie table and codec boundary labels.",
        "Native coverage measures implemented replacements, not gameplay verification.",
        "",
        "| | Count |",
        "|---|---:|",
        f"| Functions in the EU ROM | {m['rom_functions']} |",
        f"| Decompiled C, compiled natively | {m['c_functions']} |",
        f"| Hand-written ARM in the ROM | {m['asm_functions']} |",
        f"| ...ported to C for the PC build | {m['asm_ported']} |",
        f"| **Running natively** | **{native} ({100.0 * native / m['rom_functions']:.2f}%)** |",
        f"| Game C units compiled for x86 | {m['c_units']} / {m['c_units']} |",
        "",
    ]
    if m["asm_missing"]:
        lines += ["Hand-written ARM still to port: " + ", ".join(f"`{n}`" for n in m["asm_missing"]), ""]
    lines += ["## Subsystems", "", "| Subsystem | Weight | Done | Notes |", "|---|---:|---:|---|"]
    for s in status["subsystems"]:
        lines.append(f"| {s['name']} | {s['weight']} | {s['done']}% | {s['notes']} |")
    lines += ["", "## Milestones", ""]
    for ms in status["milestones"]:
        lines.append(f"- [{'x' if ms['done'] else ' '}] {ms['name']}")
    return "\n".join(lines) + "\n"


def main():
    status = json.loads(STATUS.read_text())
    m = measure()
    total = sum(s["weight"] for s in status["subsystems"])
    overall = sum(s["weight"] * s["done"] for s in status["subsystems"]) / total
    SVG.write_text(render_svg(status, m, overall), newline="\n")
    MD.write_text(render_md(status, m, overall), newline="\n")
    print(f"overall {overall:.1f}%, native functions {m['c_functions'] + m['asm_ported']}/{m['rom_functions']}")


if __name__ == "__main__":
    main()
