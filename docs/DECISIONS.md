# Decisions

Every design decision and every deviation from the brief (`CLAUDE.md`), with its date and reason. Newest last.

## 2026-11-10 M0

**Base and references.** Crawl's `main` is ZEN-1 1.2 (aceyprime/zen-1 `main`, commit `ec92952`) merged with its
full history under the brief's two commits. Remotes: `origin` (keuller-clone/crawl), `upstream-zen1`,
`upstream-grimm`, and `fomni` (the platform guard needs FoMni's commits). GR!MM v2.0.2 is the annotated tag
`refs/grimm/v2.0.2` (commit `b519acb`), a read-only reference: `git show refs/grimm/v2.0.2:firmware/...`. It is not
built and not in the working tree.

**The guard's FoMni commits are pinned under `refs/fomni/`.** The two commits `tools/guard_platform.sh` diffs
against (`b81eb6b`, the platform; `6c32623`, the installers) are no longer reachable from any FoMni branch or tag,
so `git fetch fomni` alone does not bring them. They are fetched by hash and kept as `refs/fomni/platform` and
`refs/fomni/installers` (BUILDING.md), so `git gc` cannot drop them. The guard script itself is unchanged.

**Package identity family: `FM-1_5xx`.** Dev builds are `FM-1_500`, release builds `FM-1_5XXYYZZ`
(`tools/build.py`, `firmware/src/main.c`, `tools/make_pages.py`). Reason: a Crawl build must not report itself as
a ZEN-1 build (ZEN-1 is `6xx`, FuMi-1 `7xx`, FoMni `8xx`, X0X and Felucca `9xx`; there is no public registry).
The installers treat every custom `FM-1_*` identity alike and the restore path keys on stock `FM-1_015`, so the
family does not change install or restore behavior. Steven's M0 flash is the test.

**What the rename touches.** User-visible strings (the splash, the home title, the welcome card, the crash and
safe-mode screens), the USB product name (`Crawl FM-1`, the same length as `ZEN-1 FM-1`, which is the one
platform line the guard allows to differ), the build outputs (`crawl.bin`, `crawl.fwsc`, `crawl-X.Y.fwsc`,
`crawl.wasm`, `build/host/crawl_host`), the emulator and bench pages' names, and the site tools' file names and
repository. Source file names (`zen_*.c/h`, `host/zen_host.c`, `web/zen_installer.html`), in-code identifiers and
comments that name ZEN-1 stay: renaming them is churn with no effect on the device, and the comments describe
where the code came from. The splash tagline (`Ambient scenes for the FM-1`) and the landing page, guide and
installer texts stay ZEN-1's until M3 and M5, since at M0 they are still true.

**The build runs in one container, from PowerShell.** `Dockerfile` makes one Linux x86-64 image
(`emscripten/emsdk:6.0.13` plus gcc, Python with Pillow, numpy, scipy and mido, the JieLi toolchain fetched by
`tools/get_toolchain.sh`, and the three AC79 SDK files checked by hash). Inside it the host is Linux, so
`build.sh` takes ZEN-1's native path and the JieLi tools run directly: no container inside the container.
`build.ps1`, `host/build_host.ps1`, `web/emu/build.ps1`, `tests/run_tests.ps1` and `tools/guard_platform.ps1`
only call `tools/docker_run.ps1`, which mounts the tree at `/work` and runs the same shell script. None of the
build logic is forked. The host simulator binary is a Linux program and runs through `tools/docker_run.ps1` too.
The three SDK files are downloaded into the image rather than cloned as the whole SDK: the package needs only
those (BUILDING.md), and `tools/build.py` already carries their hashes.

**Line endings.** `.gitattributes` forces LF in the working tree (`* text=auto eol=lf`, `*.ps1 text eol=crlf`):
the tree is mounted into Linux and the shell scripts would not run with CRLF.

**`tools/publish_pages.sh` commits as the configured git user.** ZEN-1's copy forced its author's name and
no-reply address on the gh-pages commit; publishing Crawl's site as AceyPrime would be wrong. Publishing itself is
M5.

## 2026-11-10 M1

**`scenario-ambient`'s `expect race 0` failure is accepted (Steven).** It is inherited from ZEN-1 and fails the
same way on an untouched ZEN-1 checkout in this container. `tests/run_tests.sh` is unchanged, so the run still
reports it; a milestone's "passing tests" means every test but this one until it is fixed upstream or here.

**The M0 flash is deferred (Steven).** Steven tests in the browser emulator first. M1 therefore has no hardware
figures of its own.

**M1's method.** Memory is measured from the M0 ELF (`tools/budget_map.py`). ZEN-1's engine CPU is measured on the
host (`tests/host/budget_bench.c`) and scaled to the FM-1 by the one hardware reading ZEN-1's source records
(8 koto voices with reverb: 38 %). GR!MM's parts were first sized by reading their source. Steven then allowed building GR!MM v2.0.2 in a scratch
checkout (`build/grimm-ref`, git-ignored) for measuring only: `tools/budget_grimm.py` times its sources and effects
in its own host mode, and its FM-1 ELF gives code sizes. It adds one printf to the scratch copy's host probe.
GR!MM stays out of Crawl's build. `docs/BUDGET.md` marks each figure measured, read or estimated.

**Mode B: no go for v1, provisionally**, on measured memory (`docs/BUDGET.md`). Mode A plans for 8 voices and a
145 KB pool budget for the effects until hardware says otherwise.

**`tools/docker_run.ps1` takes its arguments raw.** With a `param` block PowerShell bound the command's own flags
(`sh -c`) as its parameters, and with `$ErrorActionPreference = "Stop"` Windows PowerShell 5.1 turned docker's
progress on stderr into a fatal error when redirected.

## 2026-11-10 M2: TAPE

**GR!MM's TAPE replaces ZEN-1's own tape.** ZEN-1 had a small tape stage (`fm_tape`), but every scene set it
to 0, so nothing audible changes at parity. `firmware/src/dsp/tapefx.c` is GR!MM's `tapefx.c` and `wear.c` with
Felucca's `softclip` and tanh table, kept in integers. `fm_tape(pct)` is now how far the mix goes through the tape
(GR!MM's TAPE tile: a crossfade), and `fm_tape_knobs(wow, sat, tone, hiss)` sets its four knobs. At amount 0
nothing runs.

**On the mix, not per track.** The brief asks for TAPE on the mix. GR!MM v2 has it per track; Crawl has no
tracks. It sits after the reverb, before Depth and the output level.

**GR!MM's 256-frame period is kept inside Crawl's 32-frame blocks**, so the curve's crossover, the hiss ramp
and the wow's timing are GR!MM's. The wow bends scene notes through the existing `tape_wow` path (cents), not the
koto instrument, as ZEN-1's did.

**Half the saturation curve is stored.** The curve is odd, so Crawl keeps x >= 0 and mirrors it: 1540 B of pool
instead of 3076. With the full curve the build failed ZEN-1's own check that 8 KiB of pool stays free. A negative
sample can differ from GR!MM's by one step of the last bit.

**No scene removal yet.** M1 suggested removing a scene to make room. TAPE did not need it, and the pool, which
is what DELAY and GRAIN need, is not held by the scenes but by the reverb and the screen. The reverb's storage is
the next thing to decide, with Steven listening.

## 2026-11-10 M2: the reverb's storage, DELAY, GRAIN

**The reverb's lines hold 16-bit samples (Steven).** `reverb.c` stored floats, 135 KB of pool. It now stores
int16 at +-2.0 full scale, rounded towards zero so a tail ends in silence: 68 KB. The arithmetic is still float. No
before and after render, at Steven's word. It costs time: each tap converts, about 8 us a block more on the host
(the idle floor went from 20 to 28 us), an estimated 5 % of an FM-1 block.

**DELAY and GRAIN share one line of 1.4 s.** Steven agreed to a shorter delay, 1.5 s proposed. After the reverb
change the pool has 68 KB to give. GR!MM's delay line at 1.5 s is 67 KB, which leaves nothing for GRAIN's ring
(GR!MM: 55 KB) or the working buffers. So `echofx.c` has one line, 1.4 s (62 KB): DELAY writes it and GRAIN
reads it as its ring. Consequences: TIME's table is scaled by 0.4 (8 to 1400 ms); the cloud is made of the send
and of its echoes; the line records while either effect is up; FREEZE holds the line for both, so the echoes fade
out over 6 ms while it is held and return when it is let go. The alternative, two separate buffers, gave a 1.0 s
delay and a 0.4 s ring, too short for grains of up to 390 ms.

**GR!MM's 256-frame blocks are kept, one period late.** fumi.c's 32-frame blocks fill a period's send and play
the last period's returns, so echoes and grains arrive 5.8 ms late. The code inside is GR!MM's, generated from
its source with the changes listed at the top of `echofx.c`.

**One send for both** (`fm_fx_send`), from the voices after the tone stage. The returns go to the output and into
the reverb (GR!MM: its REV bus), and through the tape. All off at `fm_init`: nothing runs, parity holds.

**GRAIN's lengths come from the main loop in float** (`fm_grain`), not from GR!MM's pitch table, which Crawl does
not carry.

## 2026-11-10 M2: Sakura out of the FM-1 image

**Sakura is left out of the firmware build only (Steven chose to remove a scene; the way it is done is a
deviation).** `ui.c` `CRAWL_SAKURA` is 0 for the FM-1 image and 1 for the host simulator and the browser build.
With it out the image is 489876 B (91688 B of flash free) and RAM is 54616 B of 98304 (43688 B free); its bush
warbler recordings drop out too (`bed.c`). Removing it everywhere was tried first: 12 of the 23 scenario scripts
failed, about 140 expectations, because the scripts are written around Sakura's own defaults and its place as
the first scene. Rewriting them would be thrown away in M3, when every scene is replaced. The cost: until M3 the
simulator and the emulator are not the same scene list as the firmware (three scenes against two). The engine,
the effects and the two remaining scenes are the same code in both.

## 2026-11-10 M2: VOICE

**A part has a source.** `fm_part_patch(part, FM_ENG_VOICE + TYPE)` makes a scene part GR!MM's VOICE (TYPE 0 SOLO,
1 GREGORIAN, 2 NUNS, 3 CATHEDRAL); any other slot is an FM patch as before. `fm_part_knobs` sets the source's four
knobs, `fm_part_attack` its attack. The koto instrument (part 0) stays FM. PHYS and SAMPLE will be further sources
of the same kind.

**GR!MM's code makes the block; Crawl's voice does the rest.** `voicefx.c` is generated from GR!MM's `formant.c`
with the changes listed at its top: it writes a block at full level, and fumi.c's existing voice code applies
level, velocity, pan, the release and the tape's wow, plus a new linear attack. GR!MM's track envelope is not
ported. The pitch arrives in cents; the phase increment is computed in float instead of from GR!MM's pitch table.

**Voices are still 12.** M1 planned 8. Nothing forced the change yet, and lowering it would alter the ZEN-1 scenes
still in the tree. It is set when the Crawl scenes are written, with hardware figures if there are any by then.

**The choir wavetables are CC BY 4.0** (VocalSet). `LICENSES/CC-BY-4.0.txt` came along, and the credit is in
`LICENSING.md` and `README.md`, as GR!MM has it.

## 2026-11-10 M2: PHYS

**The DSP files are GR!MM's, byte for byte.** `phys_dsp.c` (DaisySP, MIT) and `phys_symp.c` (Rings, MIT) are copied
from `refs/grimm/v2.0.2` unchanged, with `LICENSES/MIT-DaisySP.txt` and `LICENSES/MIT-Rings.txt`. Only the wrapper,
`physfx.c` (GR!MM's `phys.c`), is adapted. A later GR!MM fix to the models can be taken by copying the file.

**A part's source, as VOICE.** `fm_part_patch(part, FM_ENG_PHYS + MODEL)` (0 MODAL, 1 STRING, 2 MEMBRANE, 3 SYMPATH).
The knobs are STRUCTURE, DAMP, BOW and a fourth, BRIGHT, which GR!MM fixes at 80: dark scenes will want it lower, and
the knob was free. A part given another source takes that source's default knobs, so the knobs are set after the
patch. VOICE keeps its defaults; only a part that was PHYS and becomes VOICE behaves differently from before.

**Four PHYS notes at once (`FM_NPHYS`), in slots.** A model's state is 4.3 KB. One a voice, as GR!MM has it, is
52 KB for 12 voices, and 41 KB of RAM was free. M1 asks for at most 2 PHYS notes until hardware is measured. Four
slots (17 KB) leave room above that plan and 24 KB of RAM for SAMPLE and the scenes. A fifth note goes over the
oldest PHYS note in its own voice, so the gain ramps on and nothing clicks. The tradeoff: a long bell tail can be
cut by a fifth strike. Scenes should plan for four.

**The velocity is the accent.** Felucca's PHYS takes its accent from the velocity; GR!MM fixes the velocity at 100.
Crawl's scene notes carry a velocity, so a harder strike is brighter as well as louder.

**The level is GR!MM's.** PHYS is written at twice VOICE's scale, as in GR!MM's mixer. At A3 a struck note's peak is between
level with an FM pad's peak (MODAL) and 11 dB under it (STRING); bowed ones sit within 4 dB of the pad's level.

**`__udivdi3` is Crawl's own, for the FM-1 build only.** The firmware links no compiler runtime. At `-O2` (GR!MM
builds at `-Os`) the compiler calls it for one 64-bit division in `phys_dsp.c`, once a block. `physfx.c` supplies a
plain long division. The alternatives were linking the toolchain's `libcompiler-rt.a` (new third-party binary code in
the image) or editing the MIT file.

## 2026-11-10 M2: SAMPLE

Steven left both open questions to be decided from the project's context.

**The samples live in the image, not in a flash area of their own.** GR!MM keeps four 72 KiB slots at
0x97000..0xDF000, fills them at boot and takes uploads over SysEx. Crawl keeps its samples as constant data in the
app, as ZEN-1 keeps its recordings (`crawl_samples.h`, made by `tools/crawl_samples.py`). Reasons: GR!MM's area
overlaps ZEN-1's save journal (0xC0000) and panel object (0xDC000); using it needs new flash code in the frozen
platform layer; uploads are a sample editor, which v1 leaves out; and one `.fwsc` then holds all of Crawl, so install
and restore stay as they are. The cost: samples take app flash, about 11 KB a second, and changing one needs a
build. M3 frees flash when ZEN-1's recordings (279 KB) are replaced.

**What GR!MM works out at boot is made at build time.** The seek index and the decoder's two tables are in the
generated header, in flash: no boot scan, no CRC, no 6.9 KB of pool for the index, no 2.1 KB of RAM for the tables.
A sample is always ready. `samples.c`'s slots, upload and factory restore are not ported.

**The windows are in RAM, four notes at once (`FM_NSAMPLE`).** GR!MM puts a voice's four windows (2 KB) in the
pool. Crawl's pool is at its floor (276 B above the 8 KiB that must stay free) and its holders are the screen and
the effects. RAM had 23.9 KB free. Four slots take 8.6 KB with their state and leave 15.2 KB for the scenes. PHYS
and SAMPLE now share one slot mechanism in `fumi.c` (`src_*`), each with its own four slots.

**A part's source, as VOICE and PHYS.** `fm_part_patch(part, FM_ENG_SAMPLE + n)`; GR!MM's SLOT knob is the patch.
The knobs are START, LENGTH, REVERSE; the fourth is unused. One past the last sample plays nothing.

**The level is twice GR!MM's.** At GR!MM's 9000 a normalized sample sat 14 dB under an FM pad, and a part's level
only goes down. At 18000 it is 8 dB under, with its peak below the mixer's knee.

**M-SOFT is a stand-in.** One sample, the first 1.5 s of GR!MM's CC0 factory sample `M-Soft.wav`, proves the source
and the tool (`assets/SAMPLES.md`). Which samples ship is M3's choice, with the scenes. GR!MM's four factory samples
whole are 292 KB and do not fit.

## 2026-11-10 M2: end

**Voices stay 12, with caps by source.** PHYS 4, SAMPLE 4, the rest FM or VOICE. M1's plan of 8 in all and 2 PHYS
is a time budget, and time is not measured on hardware yet. The caps here are memory's. M3's scene tables should
keep to M1's plan until a flash gives CPU figures.

**Whether DELAY replaces ZEN-1's echo is not decided here.** The brief allows it. ZEN-1's code is untouched on
this point; it is settled when the Crawl scenes are written.

## 2026-11-10 M3: plan

Steven left the open design calls of M3 to be made from the project's context.

**Koi comes out of the FM-1 image first, the way Sakura did.** Crypt needs flash (a ZEN-1 scene is 32 to 47 KB;
21 KB is free). Koi is the largest scene left and nothing of Crawl reuses it. Hotaru stays until Night Sky, which
takes over its moon and stars. `ui.c` `CRAWL_KOI` is 0 for the FM-1 image and 1 for the simulator and the browser
build, for the reason given under "Sakura out of the FM-1 image": the scenario scripts are written around the
ZEN-1 scenes and are rewritten once, when the last of them goes.

**The new scenes go after ZEN-1's in the simulator's list**, so the scripts' scene numbers hold. Two scripts that
went round the end of the list were changed by one step each. In the FM-1 image Crypt is first.

**Crypt takes project slot 1**, free since ZEN-1 dropped its Bench scene. The project format is unchanged until
the ZEN-1 scenes go; then the four Crawl scenes own slots 0 to 3 and the magic changes.

**The scales stay five notes an octave.** The brief names seven-note modes. ZEN-1's keys (sixteen white keys,
five an octave), its conductor (chords of three of five degrees, the motif, the loops) and the drone are all built
on five. A seven-note scale would change every one of them and the key map. So each dark mode is a five-note set
that keeps the mode's colour notes: Aeolian 1 2 b3 5 b6, Dorian 1 b3 4 5 6, harmonic minor 1 b3 5 b6 7, Hungarian
minor 1 2 b3 #4 5. ZEN-1's In is already Phrygian's 1 b2 4 5 b6 and Min'yo the minor pentatonic; they stay, under
their old names until the instrument's Japanese naming goes (step 6). The sets are rows of `TN_SCALE`; a mood
names its scale in the scene's mood table. The instrument's own scale knob keeps to ZEN-1's three (`SC_NINST`).

**A scene's first mood is its own long form for now.** The brief has four moods a scene and one cross-scene
Long Night. ZEN-1's glue has five mood places, the first "Natural". Crypt's first mood, "Long Night", burns the
candles down through the looks of its four moods in half an hour and starts again. The cross-scene mode is
step 5; when it exists this place becomes its door.

## 2026-11-10 M3: Crypt

**The vault's light is the palette.** The stone is drawn once, each pixel a shade by how much candlelight reaches
it; the 48 stone shades follow the flames, so a gutter dims the room with nothing redrawn. The cost: a palette
change sends the whole screen (ZEN-1's own note: about 20 ms). So the light moves in steps of 1/32, at most four
times a second, sooner only for a gutter or a mood on its way. To be measured on hardware.

**Which source plays what.** The choir is the conductor's drone on VOICE: three voices, inside M1's four. The
bell is PHYS MODAL on two note ids, inside M1's two. The scene's notes stay FM, on ZEN-1's Drop, Mallet, Rin and
Bell patches: PHYS notes would pass M1's plan before hardware is measured. SAMPLE is not used: there are no CC0
crypt recordings yet, so the drips (`fm_drop`) and the draught (the synthesized breeze) are synthesized and
Crypt holds no recording.

**DELAY is the echo of Crawl's scenes (M2 left this open).** ZEN-1's echo replays a note seconds later and costs
a voice each time (about 3 % of a block, estimated); DELAY costs about 2 % in all and is the tape echo the brief
asks for. Crypt turns the conductor's echo off and puts DELAY on the Echo knob. ZEN-1's scenes keep theirs until
they go.

**Each mood's scale, key and look are one table row** (`CR_MOOD`): Vigil In, Procession Dorian, Sealed Hungarian
minor a fourth down, Embers harmonic minor. Long Night takes the scale of where it is in its round, when the
notes rest.

**ZEN-1's knob pages are kept** (Scene, Volume, Nature, Music, Key), with Tape, Cloud and Echo on knobs. The
Key page's Scale still offers ZEN-1's three and holds one of them over the mood's; it gets Crawl's list in step
6, with the Key page's own script.

**Black keys:** five candles, five bell notes, a draught. The brief leaves a scene's functions to the scene.

## 2026-11-10 M3: Crypt's moods, after listening

**A mood must differ in what is heard and in what is seen (Steven).** Changing levels, the scale and the tint was
not enough: Vigil and Procession sounded alike and three pictures looked alike. So a mood row now also holds its
note range, a pace and a shaft of light, and the moods use different sources and event sources: Procession is
driven by a clock (footfalls, a figure of notes, the bell on the bar) and shows figures walking; Sealed's notes
come from the water and it has no voice; Vigil is the choir with few notes.

**Procession takes "a consistent pace" literally** (Steven's word): 1.1 s a step, the conductor's token bucket
and rests bypassed for its notes so the pace never breaks. Its notes are three of the motif's, so they follow the
chord. The flames' own gutters are silent while it runs.

**VOICE's BREATH is used sparingly.** On the solo voice it is noise through the formants; at 70 it read as static.
Embers keeps 45 on the nuns, which Steven found fine.

**Renders are written louder than the output (`wavgain 24`).** The level itself is ZEN-1's and is left alone: the
scenes' loudness is one thing to set for all of them, on hardware. The 16-bit file is the problem, so the gain is
on the file. The peak checks stay on the true output.

**Long Night's knobs are its own; its range, pace and shaft are its round's.** Changing the choir's source inside
the round would switch a part's source under held notes, which is not tested for clicks.

## 2026-11-11 M3: Forest Ruins

Built from the plan's step 4 with the open calls settled here.

**Its textures are recordings through ZEN-1's pipeline, not the SAMPLE source.** The brief's nature layer is CC0
recordings matched to scenes; `bed.c` already plays loops (the air) and one-shot calls (the owl) from flash with
no RAM to speak of, and the SAMPLE source is a pitched tape loop, which a crow is not. So the crows are a one-shot
set (`FM_SHOT_CROW`: a hooded crow's four caws, then a rook's three, so two birds answer each other; a storm's
take-off is a rook flock's burst, `FM_SHOT_FLOCK`) and the dry leaves are the Air style Leaves, which ZEN-1 had
emptied (it played the breeze). Each is CC0 on its Freesound page, logged in `assets/sounds/README.md` and in a
table in `assets/SAMPLES.md`. SAMPLE waits for a scene whose texture wants pitch (Frozen Lake's ice, perhaps).

**The brook leaves the FM-1 image now, as the plan said.** In the FM-1 build `bed.c` no longer reads the loop
table (the breeze, the leaves and the insects by name), so the brook's 68 KB is not emitted; Hotaru's "Water is:
Brook" plays the bubbles there until Hotaru goes. The simulator keeps the brook: its scripts use it. The leaves
loop (32 KB) and the crows (34 KB) take about half of what the brook freed.

**Wind through the arches is a bowed string, one note an arch.** PHYS STRING with BOW up sustains; each arch
sings a chord tone (the widest the root, the others above it), brought into 147..294 Hz for the speaker, as
strong as the gust and as long, and glided up to 55 cents as the wind rises. Two arches at once in a mood
without thunder, one with: with the thunder (PHYS MEMBRANE) and the falling stones (PHYS MODAL) that keeps PHYS
within M1's two notes at a time, to be measured on hardware like the rest.

**Each mood has its own event source, sources and sight.** Dawn Mist: fog banks (a swell each time one passes
through an arch), a pale sun, a pad, one crow far off behind the ruin, no wind. Overgrown: the leaves (a note each
where one lands, a touch on the wall), ivy and the forest in leaf, three crows on the wall, a cathedral choir,
the dry leaves in the air. Storm: gusts back to back (the arches singing), rain and a deep roar, lightning (the
palette washed white for a flash, a bolt) with thunder after it and the crows up all at once, leaves driven
straight through. Owl Moon: the moon up all night with bats (a crossing is a faint tick), stones falling from the
wall (a dull knock), the owl's recording, one VOICE solo whose vowel the wind opens, thin wisps of fog, Min'yo a
fourth down in the cloud. Long Night is a day: the four in that order, half an hour round.

**"Hollow Night" is "Owl Moon" on the panel.** The mood's name shows in the knob bar, 11 characters and 56 px at
most; "Hollow Night" is 12 and "Hollow Moon" measured 61 px. Owl Moon is what the mood shows and sounds. The
scene is "Ruins" on its page tab for the same reason ("Forest Ruins" overflowed the tab), Forest Ruins
everywhere else.

**The levels were set by measurement, not ears.** The first renders clipped: thunder at velocity 112 and crows at
74..104 peaked 15 dB over Crypt. Now the thunder is 86 on a part at 40 %, the crows 44..62 near and 26..34 far
(a one-shot's gain is velocity squared), the owl 44..62, the flock 52; the renders peak -30 to -38 dBFS at the
output, where Crypt's are -34 to -40. The dry-leaves loop is broadband (its centroid 5.4 to 6 kHz everywhere in
the recording), so Overgrown's air sits at 12 and its 3..10 kHz floor is 18 dB under the music; Storm's rain
gives 23 dB. Both are textures, not VOICE's breath, but they are the first things to listen for.

**Forest Ruins takes project slot 2, Koi's.** Koi is out of the FM-1 image; in the simulator the two share the
slot until step 6, when the four Crawl scenes take slots 0 to 3 and the magic changes. The slot holds knobs and
the kept picture; no script switches between Koi and Forest Ruins with saved state.

## 2026-11-11 M3: Forest Ruins' air, after listening

**The air is the synthesized breeze in every mood; the dry-leaves loop went.** Steven heard too much hiss in
Dawn Mist, Long Night, Overgrown and Storm and little in Owl Moon. Owl Moon was the one mood on the synthesized
breeze (soft noise low-passed at about 520 Hz, puffs to 1 kHz); the others sat on the breeze recording, on the
dry-leaves loop (broadband, centroid 5.4 to 6 kHz everywhere in it) and, in Storm, on the rain bed. So every
mood takes the synthesized breeze, whose puffs are the gusts anyway; the leaves loop is out of the tool, the
image and the logs (its row says it was there for a day); Storm's rain bed is at a third of what it was and the
roar a little lower. Measured, the steady 3..10 kHz noise against the music went from 29, 34, 18, 23 dB under
(Long Night, Dawn, Overgrown, Storm) to 48, 54, 32, 35; Owl Moon 46. Overgrown's remaining floor does not move
with its Air knob: it is the mallets' tails at Activity 40, not noise. The breeze recording stays in the image
for Hotaru until step 6. Flash: image 519680 B, 61884 B free.

**Long Night starts wherever the day is.** It began at dawn every time, so its render was Dawn Mist again
(Steven: "sounds just like forest dawn"). A day has no natural first hour; the phase at power-on is the seed's.
