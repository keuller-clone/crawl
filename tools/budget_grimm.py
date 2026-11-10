#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""M1 budget spike (docs/BUDGET.md): what GR!MM's sources and effects cost, on this computer, from GR!MM's own
host mode and its own per-stage timer (grimm.c cpu_stage: VOICES, MIX, GRAIN, DELAY, SPACE, TAPE, ...).

GR!MM v2.0.2 is a reference, not part of Crawl's build: this works on a scratch checkout of it, for measuring only.

  git worktree add --detach build/grimm-ref refs/grimm/v2.0.2
  cd build/grimm-ref && python3 ../../tools/budget_grimm.py          (in the build container)

It adds one printf to the scratch copy's host probe (firmware/src/probe.c: the stage timers, which host mode keeps
but does not print), builds host mode as tests/run_host.sh does, and for every factory sound plays 0, 1 and 4 keys
and reads the timers. Host mode's clock has 1 us steps and the timers average 86 blocks.
"""
import os
import statistics
import subprocess
import sys
from pathlib import Path

ENG = ["VA", "FM", "SAMPLE", "VOICE", "WHEEL", "PHYS", "PHASE"]
STAGES = ["VOICES", "MIX", "GRAIN", "DELAY", "SPACE", "TAPE", "PERFORM", "MASTER", "DRIVE", "SCAN"]
KEYS = {0: 0, 1: 0x80, 4: 0x80 | 0x800 | 0x4000 | 0x10}            # C4; C4 E4 G4 A3 (tests/latch_test.py)
PATCH_AT = "    uint32_t i;\n    printf(\"probe %u play"
PATCH = ("    uint32_t i;\n    printf(\"cpustage %u now %u\", fm1_ms, cpu_now);\n"
         "    for (i = 0; i < CS_N; i++) printf(\" %u\", cpu_stage[i]);\n    printf(\"\\n\");\n    printf(\"probe %u play")
OUT = Path("build/host/budget")


def build():
    p = Path("firmware/src/probe.c")
    s = p.read_text(encoding="utf-8")
    if "cpustage" not in s:
        if s.count(PATCH_AT) != 1:
            sys.exit("budget_grimm: probe.c is not as v2.0.2 has it")
        p.write_text(s.replace(PATCH_AT, PATCH), encoding="utf-8")
    Path("build/host").mkdir(parents=True, exist_ok=True)
    subprocess.run(["cc", "-O2", "-w", "-DDINGHY_HOST", '-DAPP_VER="2.0"', "-Ifirmware/src", "-Ifirmware/gen",
                    "-o", "build/host/grimm", "firmware/src/grimm.c"], check=True)


def run(k, nkeys, extra=()):
    """sound k (PRESETS turned k steps), then extra (the panel, 400 .. 1900 ms), nkeys held from 2 s: the stage
    timers (us), the source, chord on / off"""
    OUT.mkdir(parents=True, exist_ok=True)
    ev = [(300, f"enc 6 {k}"), *extra, (2000, f"keys {KEYS[nkeys]:#x}")] + [(t, "probe") for t in (5000, 5600, 6200, 6800)]
    p = OUT / "s.txt"
    p.write_text("".join(f"{t} {s}\n" for t, s in sorted(ev, key=lambda e: e[0])) + "6900 end\n")
    out = subprocess.run(["build/host/grimm", str(OUT / "s"), str(p)], capture_output=True, text=True, check=True,
                         env=dict(os.environ, GRIMM_MASTER="1022")).stdout
    rows, eng, chord = [], -1, 0
    for ln in out.splitlines():
        w = ln.split()
        if ln.startswith("cpustage "):
            rows.append([int(x) / 24.0 for x in w[3:]])            # now, then the stages (ticks -> us)
        elif ln.startswith("probe "):
            if " eng " in ln:
                eng = int(ln.split(" eng ", 1)[1].split()[0])
            if " ch1 " in ln:
                chord = int(ln.split(" ch1 ", 1)[1].split()[0])
    med = [statistics.median(c) for c in zip(*rows)]
    return med, eng, chord


def main():
    build()
    per = {}
    fx = {s: [] for s in STAGES}
    print(f"{'sound':>5} {'source':7} {'chord':>5} {'block us: 0 / 1 / 4 keys':>26}   {'VOICES us: 0 / 1 / 4':>22}   "
          f"{'a key':>6}   stages with 4 keys (us): " + " ".join(STAGES[1:6]))
    for k in range(1, 45):
        m = {n: run(k, n) for n in (0, 1, 4)}
        eng, chord = m[1][1], m[1][2]
        v = {n: m[n][0][1] for n in m}
        key = (v[4] - v[1]) / 3.0
        name = ENG[eng] if 0 <= eng < len(ENG) else "?"
        print(f"{k:5d} {name:7} {chord:5d} {m[0][0][0]:8.1f} {m[1][0][0]:8.1f} {m[4][0][0]:8.1f}   "
              f"{v[0]:6.1f} {v[1]:6.1f} {v[4]:6.1f}   {key:6.1f}   " + " ".join(f"{x:6.1f}" for x in m[4][0][2:7]))
        if not chord:
            per.setdefault(name, []).append((key, v[1] - v[0]))
        for i, s in enumerate(STAGES):
            fx[s].append(m[4][0][1 + i])
    print("\nper source, chord off (us a block on this computer): a further key (from 1 to 4 keys); the first key")
    for name, xs in sorted(per.items()):
        a, b = [x[0] for x in xs], [x[1] for x in xs]
        print(f"  {name:7} {len(xs):2d} sounds   a further key: median {statistics.median(a):5.1f}, {min(a):5.1f} .. "
              f"{max(a):5.1f}   the first: median {statistics.median(b):5.1f}")
    print("\nper stage, 4 keys held, over the 44 sounds (us a block): median, most")
    for s in STAGES[1:]:
        print(f"  {s:8} {statistics.median(fx[s]):6.1f} {max(fx[s]):6.1f}")
    effects()
    return 0


def effects():
    """the effects and SAMPLE, which no factory sound has on: sound 1 (a VA pad) on track 1, the panel as GR!MM's
    tests work it (tests/tapeoff_test.py, tests/fx_test.py): MIXER 1's tiles are LEVEL, DLY, REV, TAPE, GRAIN"""
    mixer = [(400, "btn 8")]
    dly = [(450, "enc 0 1"), (500, "enc 2 127")]
    tape = [(700, "btn 2")] + [(720 + 20 * i, f"enc {2 + i} {v}") for i, v in enumerate((127, 110, 90, 127))]
    cases = [
        ("the pad alone (4 keys)", 4, []),
        ("+ DLY send 127", 4, mixer + dly),
        ("+ GRAIN send 127", 4, mixer + [(450, "enc 0 4"), (500, "enc 2 127")]),
        ("+ TAPE tile 127, TAPE's four knobs up", 4, mixer + [(450, "enc 0 3"), (500, "enc 2 127")] + tape),
        ("+ DLY, GRAIN and TAPE together", 4, mixer + dly + [(550, "enc 0 2"), (600, "enc 2 127"), (620, "enc 0 1"),
                                                           (650, "enc 2 127")] + tape),
        ("SOURCE: SAMPLE, 1 key", 1, [(400, "btn 6"), (450, "btn 6"), (500, "enc 2 2")]),
        ("SOURCE: SAMPLE, 4 keys", 4, [(400, "btn 6"), (450, "btn 6"), (500, "enc 2 2")]),
    ]
    print("\nthe effects and SAMPLE on sound 1 (us a block): the block, then " + " ".join(STAGES[:6]) + "; the source")
    for name, n, extra in cases:
        m, eng, _ = run(1, n, extra)
        print(f"  {name:40} {m[0]:7.1f}   " + " ".join(f"{x:6.1f}" for x in m[1:7]) +
              f"   {ENG[eng] if 0 <= eng < len(ENG) else '?'}")


if __name__ == "__main__":
    sys.exit(main())
