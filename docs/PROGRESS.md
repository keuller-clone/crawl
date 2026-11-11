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
- Pushed to `origin/main` on 2026-11-10. The emulator was checked in Chrome from a local server: it boots into
  Sakura with the "Welcome to Crawl" card and no console errors.

## M1 Budget spike (2026-11-10): memory measured, CPU partly, hardware and GR!MM builds open

`docs/BUDGET.md` has the figures and the recommendation.

- **Memory (measured):** Crawl at M0 is full. Free: flash 13684 B, RAM 1512 B, pool 10044 B.
  `tools/budget_map.py` breaks each region down by owner.
- **ZEN-1 engine CPU (measured on the host):** `tests/host/budget_bench.c`. About 5 us a block for an FM voice,
  20 us idle, 113 us with everything on, of a 5805 us block.
- **FM-1 CPU (estimated):** scaled from ZEN-1's one recorded hardware reading. About 3 % a voice, 12 % idle.
- **GR!MM parts:** pool, RAM and table sizes read from source. DELAY and GRAIN as GR!MM sizes them need 210 KB
  of pool; about 145 KB exists for all effects.
- **Recommendation:** Mode B no go for v1, provisionally. Mode A: 8 voices, a fixed effects pool budget, free
  ZEN-1 content before porting.

**Not done, and why**

- GR!MM's parts are now measured on the host (`tools/budget_grimm.py`, with Steven's leave to build GR!MM in a
  scratch checkout): VOICE 7.2 us a voice, PHYS 10 to 21, SAMPLE 1.8, DELAY 3.7, GRAIN 1.9, TAPE 3.2 a block.
  All ported code is under 20 KB plus a 29 KB choir table.
- No hardware measurements: not flashed.
- Tests: unchanged from M0 (62 pass, `kanji` skipped, `scenario-ambient` fails as accepted). The bench is not
  part of `run_tests.sh`. Guard clean.

## M2 DSP port (in progress)

**TAPE: ported (2026-11-10).** `firmware/src/dsp/tapefx.c`, from GR!MM v2.0.2.

- Unit test `tests/host/tape_test.c` (in `run_tests.sh`): 18 checks pass. Clean tape is bit-exact, saturation
  rounds a loud sine and leaves a quiet one within 0.03 dB, tone takes 6 kHz down 10.5 dB, hiss is faint and
  stops when the tape lets go, a knob turn makes no step, wow gives 7 warbles in 40 s within 136 cents.
- Render to listen to: `build/host/tape.wav` (20 s: a pad for 4 s, 12 s on the tape, 4 s after), written by the
  test.
- Cost on the host: 9 us a block with all four knobs up (ZEN-1's old tape: 13.6). Estimated about 5 % of an
  FM-1 block.
- Build: image 571908 B, RAM 96936 B of 98304, pool 335560 B of 344064 (8504 B free, 312 B above the limit).
- Tests: 63 pass, `kanji` skipped, `scenario-ambient` fails as accepted. Guard clean. Emulator builds.
- Not audible yet in the emulator or on a scene: no scene turns the tape up until M3.

**Next:** DELAY and GRAIN need about 120 KB of pool that is not there. The reverb's 135 KB buffer is the place
to find it, and that changes the reverb's storage, so it needs a listening check.

**The reverb's storage, DELAY, GRAIN: done (2026-11-10).** `firmware/src/dsp/reverb.c`, `echofx.c`.

- The reverb's lines are 16-bit: 68 KB of pool freed. It costs about 8 us a block more on the host.
- DELAY and GRAIN are GR!MM's, on one shared 1.4 s line (`docs/DECISIONS.md` says why). API: `fm_fx_send`,
  `fm_delay`, `fm_grain`, `fm_grain_freeze`.
- Unit test `tests/host/echo_test.c` (in `run_tests.sh`): 19 checks pass. The echo returns at TIME (298 ms
  measured for a 286 ms setting plus the burst and a period), repeats 5.3 dB down at FEEDBACK 64, stays bounded at
  full FEEDBACK (peak 1.34), ends in exact silence and stops; the cloud sounds and is panned; FREEZE holds it for
  6 s and more, and letting go writes it over; echoes fade without a step when frozen.
- Render to listen to: `build/host/echo.wav` (32 s: dry, DELAY from 8 s, GRAIN from 16 s, FREEZE at 24 s,
  let go at 30 s).
- Cost on the host, over one voice: DELAY 4.4 us a block, GRAIN 5.6. Estimated about 3 % of an FM-1 block each.
- Build: image 576212 B (5352 B of flash left), RAM 96984 B of 98304, pool 335596 B of 344064 (8468 B free,
  276 B above the limit).
- Tests: 64 pass, `kanji` skipped, `scenario-ambient` fails as accepted. Guard clean. Emulator builds.
- Not audible yet in a scene: nothing turns the send up until M3.

**Left in M2:** VOICE, PHYS, SAMPLE. All three memory regions are at their limits again: flash 5 KB, RAM 1.3 KB,
pool 0.3 KB above its floor. VOICE needs 33 KB of flash, PHYS 13 KB of flash and 4.3 KB of RAM a voice. They do
not fit until ZEN-1 content goes (a scene, its recordings, or the koto instrument).

**Sakura out of the FM-1 image, and VOICE: done (2026-11-10).**

- Sakura is left out of the firmware build only (`docs/DECISIONS.md`): 86 KB of flash and 42 KB of RAM freed.
- VOICE is GR!MM's formant voice and its three choirs, as a part's source: `firmware/src/dsp/voicefx.c`,
  `grimm_choir.h`. API: `fm_part_patch(part, FM_ENG_VOICE + TYPE)`, `fm_part_knobs`, `fm_part_attack`.
- Unit test `tests/host/voice_test.c` (in `run_tests.sh`): 25 checks pass. Each TYPE sounds at 220 Hz for A3
  (within 0.9 Hz) and 2 to 5 dB above the FM pad, VOWEL changes the sound, the attack and release work, six
  choir notes and an FM note play together, a part returns to FM.
- Render to listen to: `build/host/voice.wav` (40 s: SOLO through A E I O U, then a chord on each choir).
- Cost on the host: about 8 us a voice a block, SOLO or a choir (an FM voice: 5). Estimated about 5 % of an
  FM-1 block a voice.
- Build: image 524072 B (57492 B of flash free), RAM 57208 B of 98304 (41096 B free), pool 335596 B of 344064.
- Tests: 65 pass, `kanji` skipped, `scenario-ambient` fails as accepted. Guard clean. Emulator builds.

**Left in M2:** PHYS, then SAMPLE. PHYS fits in RAM now (4.3 KB a voice). SAMPLE needs pool for its windows and a
decision on where samples live in flash.
