# CRAWL: Dark Generative Ambient Firmware for the M-VAVE FM-1

This file is the standing brief for every Claude Code session in this repo. Read it fully before acting. When this file and a later instruction from Steven conflict, Steven wins; record the change in `docs/DECISIONS.md`.

## 1. Goal

Build **Crawl**, custom firmware for the M-VAVE FM-1 that:

- Auto-plays evolving, never-looping soundscapes driven by animated on-screen scenes (the ZEN-1 model).
- Sounds like dark ambient / dungeon synth (the GR!MM palette): formant choirs, bowed and struck metal, tape-worn textures, granular clouds, echoes.
- Ships with a browser emulator, a web installer, and a landing page + guide on GitHub Pages, so anyone can try it without hardware.

Repo: `keuller-clone/crawl` (private until release, then public). License: **GPL-3.0-only** (required, since both parents are GPL-3.0).

## 2. Source projects

| Project | Role | Where |
|---|---|---|
| ZEN-1 (aceyprime) | **Base fork.** Keep its app structure, scene/event engine, host simulator, Emscripten emulator, web installer, tools | https://github.com/aceyprime/zen-1 |
| GR!MM v2.0.2 (GRiMMBIENT) | **DSP donor.** Port selected voices and effects | https://github.com/GRiMMBIENT/grimm/tree/v2.0.2 |

Setup: clone ZEN-1 with full history as the starting point, add `upstream-zen1` and `upstream-grimm` remotes, and check out GR!MM at tag `v2.0.2` under `refs/grimm/` (read-only reference, not built). Do not assume file paths or APIs from this brief; read both codebases and confirm before porting.

## 3. Hard constraints

1. **Platform layer is frozen.** ZEN-1 keeps the Felucca-derived platform code byte-for-byte and checks it with `tools/guard_platform.sh`. Keep that guarantee. Run the guard before every commit.
2. **Radios stay off.** Never enable Bluetooth or Wi-Fi.
3. **Restore path must always work.** The installer must be able to restore M-VAVE stock firmware. Test this before any public release.
4. **Never flash hardware without Steven's explicit go-ahead in the current session.** Prepare the `.fwsc`, state what changed, and wait.
5. **Licensing hygiene.** Preserve every upstream copyright notice and `LICENSES/` entry. GR!MM components carry their own licenses (msfa Apache-2.0, DaisySP and Mutable Instruments Rings MIT, OFL fonts); bring the matching license text along with any ported code. Samples are **CC0 only**; log each one (source URL, author, license) in `assets/SAMPLES.md`.
6. **Trademark.** State that Crawl is unofficial and not affiliated with or endorsed by M-VAVE or other named trademark holders.
7. **Measure, don't guess.** CPU, RAM, and flash budgets come from measurements on the host sim and on hardware, recorded in `docs/BUDGET.md`.

## 4. Scope for v1

### 4.1 Sound engines
- Keep: ZEN-1's 6-op FM (msfa) and its three-voice breathing drone.
- Port from GR!MM: **VOICE** (formant choir), **PHYS** (bowed/struck physical models), **SAMPLE** (tape-loop playback).
- Out of scope for v1: VA, WHEEL, PHASE.

### 4.2 Effects
- Port from GR!MM: **TAPE** (wow, saturation, hiss on the mix), **GRAIN** (granular cloud with FREEZE), **DELAY** (tape echo, may replace ZEN-1's echo).
- Reverb: **keep ZEN-1's reverb**, retuned darker (longer, lower damping cutoff). Do not port SPACE.

### 4.3 Scenes (four, each with four moods)
Visual style: ZEN-1's scene-driven event model rendered in GR!MM-flavored pixel-brutalist art. Screen events trigger notes, as in ZEN-1. Keep ZEN-1's painted/pixel toggle only if both art sets are produced; otherwise repurpose the button.

| Scene | Event sources (suggested) | Moods (suggested) |
|---|---|---|
| Crypt | Candle flicker, wax drips, dust motes, a distant bell | Vigil, Procession, Sealed, Embers |
| Forest Ruins | Falling leaves, crows, fog banks, wind through arches | Dawn Mist, Overgrown, Storm, Hollow Night |
| Frozen Lake | Ice cracks, snowfall, distant tolls from a keep | Grey Morning, Whiteout, Thaw, Black Ice |
| Night Sky | Stars, meteors, clouds over a moon (reuse ZEN-1 lunar phase logic) | Dusk, Cold Clear, Eclipse, Deep Night |

Also carry over ZEN-1's "Natural" mode as a cross-scene long-form mode (rename it, e.g. **Long Night**).

Scales: replace Japanese pentatonics with dark sets, e.g. Aeolian, Phrygian, Dorian, harmonic minor, Hungarian minor, and a sparse minor pentatonic. Map scenes and moods to scales in data, not code.

Nature/texture layer: CC0 recordings matched to scenes (wind, crows, fire crackle, dripping water, ice, owls, distant bells).

### 4.4 Interaction
- **Mode A, Vigil (required):** ZEN-1 control model. Scene, mood, activity level, volume, new picture; white keys add scale-locked notes; black keys do scene functions; knob pages for scene, volume, nature, music; sleep timer; settings persist.
- **Mode B, Sequencer (conditional):** GR!MM-style multi-track step sequencer using Crawl's voices, switchable from Vigil. Build only if the M1 budget spike shows headroom. If it does not fit, ship Mode A only and document why in `docs/DECISIONS.md`. Do not degrade Mode A to make room for Mode B.

### 4.5 Web (GitHub Pages)
- **Emulator:** Emscripten build of the real firmware with audio and an on-screen FM-1 panel, playable by mouse and computer keyboard.
- **Web installer:** WebMIDI in Chrome/Edge; install Crawl and restore stock firmware.
- **Landing page + guide:** what it is, scene and mood descriptions, control map, install/uninstall/rescue steps, audio clips rendered by the host sim, credits and licenses.
- Out of scope: sound/patch editor.

## 5. Development environment

- **Windows 11 is the host. This is a Windows project.** VS Code with Claude Code, repo cloned on the Windows filesystem, FM-1 on USB to Windows.
- Do not require a user-installed WSL Linux distribution or a WSL-side clone.
- The JieLi firmware toolchain is Linux x86_64 only, so the firmware build runs inside a **Docker container launched from Windows** (Docker Desktop). This is ZEN-1's own non-Linux path. Linux is a container detail, not a workspace.
- Every build entry point must be runnable from PowerShell. If `build.sh`, `host/build_host.sh`, or `web/emu/build.sh` cannot run from Windows as-is, add thin PowerShell wrappers (`build.ps1`, etc.) that invoke the same Docker image and commands. Do not fork the build logic.
- Host sim and Emscripten emulator: build them in the same Docker image unless a native Windows build is simpler and verified.
- Flashing and USB stay on Windows: browser installer in Chrome/Edge, or `tools/fm1_install.py` run with Windows Python. USB never needs to reach Linux.
- Add `.gitattributes` with `* text=auto eol=lf` (and `*.ps1 text eol=crlf`) so shell scripts keep LF endings when mounted into the container.
- Never install system software (Docker, WSL distros, drivers, Python) yourself. Check what is present, and if something is missing, stop and give Steven the install command.
- When any step depends on the environment, give each command in its own code block.

## 6. Milestones

Each milestone ends with a passing build, passing tests, guard script clean, a host-sim audio render, and a short note in `docs/PROGRESS.md`. Stop at the end of each milestone for Steven's review.

- **M0 Parity.** Rename to Crawl (binary `crawl.fwsc`, strings, splash), keep behavior identical to ZEN-1. Firmware, host sim, and emulator all build from PowerShell via Docker. Steven flashes once to confirm install and restore.
- **M1 Budget spike.** Identify the MCU and audio callback budget. Measure CPU per voice for FM, VOICE, PHYS, SAMPLE, and per effect (TAPE, GRAIN, DELAY, reverb); measure RAM and flash headroom. Output `docs/BUDGET.md` with a go/no-go recommendation for Mode B and a max-polyphony plan for Mode A.
- **M2 DSP port.** Port one engine or effect at a time, with a unit test and a host-sim render for each. Integrate into the voice allocator and mixer.
- **M3 Scenes.** Build the four scenes, moods, scales, nature layer, Long Night mode, and art. Tune by rendering 10-minute host-sim sessions per mood and listening for loops, clipping, and dead air.
- **M4 Sequencer (conditional).** Only if M1 said go.
- **M5 Web.** Emulator, installer, landing page + guide, publish script.
- **M6 Release.** README, MANUAL, CREDITS, licensing review, restore test on hardware, tagged release with `crawl.fwsc`, repo made public.

## 7. Working rules

- Read before writing. Confirm APIs, build flags, and file layout in the actual code.
- Small, reviewable commits with clear messages. One concern per commit.
- Record every design decision and every deviation from this brief in `docs/DECISIONS.md` with date and reason.
- Prefer data tables (scenes, moods, scales, presets) over hard-coded logic.
- If a task is blocked by missing information or hardware, say so and propose the smallest next step; don't fabricate measurements or results.
- Credits: keep and extend ZEN-1's credits; add GR!MM and every ported upstream.
- Writing style for docs: plain, concise, no em-dashes.

## 8. Definition of done (v1)

- Crawl installs and uninstalls cleanly from the web installer, and stock firmware restores.
- Four scenes × four moods plus Long Night auto-play indefinitely without audible loops, clicks, or dropouts on hardware.
- VOICE, PHYS, SAMPLE, TAPE, GRAIN, DELAY, and darkened reverb all work within measured budgets.
- Emulator, installer, and guide are live on GitHub Pages.
- Licensing, credits, and sample provenance are complete.
