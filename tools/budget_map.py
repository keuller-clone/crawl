#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Where the flash, the RAM and the pool go (docs/BUDGET.md): the symbols of build/crawl.elf, sized by the gap to
the next symbol in the same section (the toolchain's objdump gives data objects no size), grouped.

  objdump -t build/crawl.elf > build/crawl.syms       (the toolchain's common/bin/objdump)
  tools/budget_map.py build/crawl.syms [--top N] [--section .text]

From Windows:
  tools\\docker_run.ps1 sh -c "/opt/jieli/toolchain/common/bin/objdump -t build/crawl.elf > build/crawl.syms"
  tools\\docker_run.ps1 python3 tools/budget_map.py build/crawl.syms
"""
import argparse
import re
import sys

# section: (start, length) as firmware/app.ld has them
REGIONS = {".text": (0x02000120, 0x8DFBC), ".bss": (0x01C08000, 96 * 1024), ".pool": (0x01C20000, 0x54000)}
LINE = re.compile(r"^([0-9a-f]{8})\s+(.{7})\s+(\.\w+)\s+([0-9a-f]{8})\s+(\S+)$")

# first match wins: (group, regex on the symbol's name)
GROUPS = [
    ("patches (koto, scenes)", r"^(FM_PATCH|ZN_PATCH|fm_patch|zn_patch|FUMI_|fumi_)"),
    ("recordings (zen_samples.h)", r"^ZN_[A-Z0-9_]+$"),
    ("kanji bitmaps (zen_kanji.h)", r"^(FONT_JPL|ZK_|KJ_|kj_)"),
    ("reverb and echo lines", r"^(pl_|rv_|RV_|reverb|plate|echo|ec_)"),
    ("voices, parts, beds, audio buffers", r"^(vc|parts|q|bed|abuf|ua_ring)$"),
    ("moon map (zen_moon.h)", r"^(ZM_|zm_|MOON|moon_)"),
    ("fonts (felucca_font.h)", r"^(FONT_|font_|GLYPH|glyph)"),
    ("scene: sakura", r"^(sk_|SK_)"),
    ("scene: koi", r"^(kp_|KP_)"),
    ("scene: hotaru", r"^(ht_|HT_)"),
    ("scenes: shared (scene.h, render8.c)", r"^(sc_|SC_|r8_|R8_|cv8_|pal_|PAL_)"),
    ("conductor (the music)", r"^(cd_|CD_)"),
    ("FM6 core and tables", r"^(fm6_|FM6_|msfa_|exp2|sin_|SIN|EXP|freqlut|pitchenv|env_|lfo_|LFO_)"),
    ("instrument and render (fumi.c)", r"^(fm_|FM_|render_block|pluck|note_|voice|tn_|TN_|bend_|sm_)"),
    ("nature beds (bed.c)", r"^(bed_|BED_|wind_|rain_|stream_)"),
    ("UI, panel, project", r"^(ui_|UI_|amb_|AMB_|hud_|panel|PANEL|proj|PROJ|VIEW_|view_|draw_|text_|cv_|CV_|apply|input)"),
    ("platform (hal, usb, ota, storage, lcd, gfx, libc)", r"^(fm1_|usb|USB|ota_|OTA|st_|ST_|lcd_|gfx_|flash|uac_|UAC|ua_|midi|mem|str|STR|isr_|_start|__|trs_|boot|safe_|enter_)"),
]


def load(path):
    by = {}
    for ln in open(path, encoding="utf-8", errors="replace"):
        m = LINE.match(ln.rstrip())
        if not m:
            continue
        addr, flags, sect, size, name = int(m[1], 16), m[2], m[3], int(m[4], 16), m[5]
        if sect not in REGIONS or name.startswith(".") and name == sect:
            continue
        by.setdefault(sect, []).append((addr, size, name, "F" in flags))
    return by


def sized(syms, end):
    """(name, bytes, is_function): a function keeps its own size; the bytes up to the next symbol go to it
    too when it has none (data), or to '(after NAME)' when a function is followed by unnamed bytes"""
    out = []
    syms = sorted(set(syms))
    for i, (addr, size, name, fn) in enumerate(syms):
        nxt = syms[i + 1][0] if i + 1 < len(syms) else end
        gap = nxt - addr
        if gap <= 0:
            continue
        if fn and size and size < gap:
            out.append((name, size, True))
            out.append((name + " (its constants)", gap - size, False))
        else:
            out.append((name, gap, fn))
    return out


def group_of(name):
    base = name.replace(" (its constants)", "")
    if base.startswith(".GJTIE"):
        return "jump tables"
    for g, rx in GROUPS:
        if re.search(rx, base):
            return g
    return "other"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("syms")
    ap.add_argument("--top", type=int, default=12, help="the biggest symbols to list per section")
    ap.add_argument("--section", help="one section only (.text, .bss, .pool)")
    a = ap.parse_args()
    by = load(a.syms)
    for sect, (start, length) in REGIONS.items():
        if a.section and a.section != sect:
            continue
        syms = by.get(sect, [])
        if not syms:
            continue
        used_end = max(s[0] + s[1] for s in syms)
        # .text's end is the image's end (the last symbol's constants run to .ram_text / .data's load address)
        items = sized(syms, used_end)
        total = sum(n for _, n, _ in items)
        print(f"\n{sect}: {total} B in symbols, region {length} B, free {length - (used_end - start)} B")
        groups = {}
        for name, n, fn in items:
            g = group_of(name)
            c, d = groups.get(g, (0, 0))
            groups[g] = (c + n, d) if fn else (c, d + n)
        print(f"  {'group':52} {'code':>8} {'data':>8} {'total':>8}")
        for g, (c, d) in sorted(groups.items(), key=lambda kv: -sum(kv[1])):
            print(f"  {g:52} {c:8} {d:8} {c + d:8}")
        print(f"  the {a.top} biggest:")
        for name, n, fn in sorted(items, key=lambda t: -t[1])[:a.top]:
            print(f"    {n:8}  {name}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
