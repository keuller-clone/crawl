# Progress

A short note at the end of each milestone. Newest last.

## M0 Parity (2026-11-10): built, one test flaky, not yet flashed

**Done**

- Repo: ZEN-1 1.2 merged with full history; remotes `upstream-zen1`, `upstream-grimm`, `fomni`; GR!MM v2.0.2 at
  `refs/grimm/v2.0.2`; the guard's two FoMni commits pinned under `refs/fomni/`.
- Renamed to Crawl: splash, home title, welcome card, crash and safe-mode screens, USB product name
  (`Crawl FM-1`), package identity (`FM-1_500` for dev builds), outputs `crawl.bin`, `crawl.fwsc`, `crawl.wasm`,
  `build/host/crawl_host`. Behavior is otherwise ZEN-1 1.2. Scope and reasons are in `docs/DECISIONS.md`.
- Build from PowerShell through Docker: `Dockerfile` (image `crawl-build`, about 4 GB) and thin wrappers
  `build.ps1`, `host/build_host.ps1`, `web/emu/build.ps1`, `tests/run_tests.ps1`, `tools/guard_platform.ps1`,
  all through `tools/docker_run.ps1`. No WSL distribution is needed.

**Results (Windows 11, Docker Desktop 4.99, image from `emscripten/emsdk:6.0.13`, gcc 13.3, JieLi toolchain 20260730.1)**

| Step | Command | Result |
| --- | --- | --- |
| Firmware | `.\build.ps1` | `build/crawl.fwsc`, 609657 B, identity `FM-1_500`; app 570936 B; loader `ota.bin` 6501 B |
| Build checks | (same) | RAM .data+.bss 96792 B of 98304; pool 334020 B of 344064; no soft-double; register access in `hal/` only |
| Host simulator | `.\host\build_host.ps1` | `build/host/crawl_host` |
| Emulator | `.\web\emu\build.ps1` | `build/emu/crawl.wasm`, 650671 B, plus the page and worklet |
| Platform guard | `tools\guard_platform.ps1` | clean: platform files match FoMni `b81eb6b`, installers `6c32623` |
| Tests | `.\tests\run_tests.ps1` | 62 pass, 1 skipped (`kanji`: needs a Noto Sans CJK font), 1 fails (`scenario-ambient`, below) |
| Host-sim audio render | (part of the tests) | `build/scenarios/first/first.wav`, `build/scenarios/midi/midi.wav` |

SHA-256 of this run's `build/crawl.fwsc`: `cfd35d881ab928eafa0e317662c22bcfa0b0be4104c3edf7064426013a5cb5de`
(a dev build; it is not checked in).

**Open**

- `scenario-ambient` fails its last check, `expect race 0` (got 1), in every full test run here. It is a timing
  flake inherited from ZEN-1, not an effect of the rename: an untouched ZEN-1 1.2 checkout built in the same
  container fails the same check, and the same binary passes and fails from run to run (roughly half of
  single runs pass). The host simulator's screen-transfer check depends on real time (`plat_us` in
  `host/zen_host.c`). Every other expectation in that scenario passes. Smallest next step: find which real-time
  path writes a blit buffer before its transfer ends, and make the simulator's clock deterministic there.
- Not flashed. Steven flashes once to confirm install and restore (brief, constraint 4). `tools/fm1_install.py`
  has not been run on Windows; it needs Windows Python with `mido` and `python-rtmidi`.
- The image holds the only copy of the JieLi toolchain tarball and the three SDK files. ZEN-1's notes advise
  keeping copies in case the downloads disappear.
- The site, guide and web installer texts still say ZEN-1 (M5). The splash tagline is ZEN-1's (M3).
- Nothing is pushed: the commits are local on `main`.
