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

## 2026-11-11 M3: Forest Ruins' picture, the second one

**Steven asked for a wholly different picture.** The first was the arcade seen from outside: piers and arches
across the middle, a dithered far wood, a litter floor, two trees at the sides; a soft painting. The second is
the opposite stance: we stand inside the dark nave looking out through one great pointed window that fills the
frame. The wall is black blocks with a cold rim of light along the openings' edges; through the window the
forest in flat silhouette layers (a hazy far wood, a middle wood with its canopy in leaf or bare, two near
trunks that sway); a lancet in each side wall; the window's light lies in a pool on the floor inside, the
mullion's shadow down it, among fallen blocks; the tracery is broken to a stump of mullion and a few spokes.
Flat shades and hard edges, after the brief's pixel-brutalist art. The sounds did not change: the three openings
sing (the window the root, the lancets above it), crows perch large on the sill and the mullion's stump, leaves
blow in and land in the light, fog comes in across the floor, stones fall from the tracery, the moon crosses
the window (its arc fitted to it, so it is never behind the wall), the sun rises in it at dawn, the lightning's
wash lights the whole nave. The geometry is three functions (`fr_open`, `fr_tracery`, `fr_ring_y`); what is
outside is drawn only over the palette entries below `FR_STONE`, so the wall stands in front of everything out
there without a mask.

## 2026-11-11 M3: Forest Ruins' picture, the third one

**Steven gave a reference: a misty marsh clearing seen from a little above**, bare pale trees with fine crowns
standing all through it and fading back into a blue haze, wet ground with standing water that holds the sky,
ochre marsh grass, and the ruins wooden: a hut fallen in under its thatch, fence posts, a cart wheel in the
water, a fallen log. The picture is built to that and the stone nave is gone. Trees are grown at enter (a trunk,
two side branches, a crown of twigs three deep, from the picture's own random stream) in four depths, each depth
its own trunk and twig shades mixed toward the haze (`FR_TRK`), so the wood recedes; the ground's haze is four
steps by depth too (`FR_GND`). The hamlet's leavings sit at their depths among the trees. The three hollows that
sing are now the stone well (the root, the widest), the hut's doorway and the log; stones fall from the hut; a
leaf on the water makes a ring, a plip (the rain bed's drop) and a soft touch instead of a note. Nothing in the
sound changed otherwise, and `forest.zen` passes unchanged.

**The canopy is a palette switch, not a redraw.** Each crown carries two dithered discs in its depth's canopy
entries; in Overgrown they are green, in the other moods they are the haze thickened a little, so the trees are
in leaf or bare without the picture being made again.

## 2026-11-11 M3: Forest Ruins' picture, the fourth one: a redwood path

**Steven asked for a pivot: redwoods flanking a path.** The marsh and everything before it are gone. The picture
is a path from our feet to a vanishing point in the mist, with eight redwoods a side placed in true perspective
(a point at depth z and lateral X is at 120 + 100 X / z, the ground there at 118 + 100 / z): fluted columns that go
off the top, their lit side toward the path, furrows down them and a flare of roots, moss low on the shaded
side, tufts of foliage on the farther ones and a dark fringe of foliage overhead; the far ones fade into the
haze by depth (`FR_TRK`, eight shades at four depths); sword ferns at their feet; the path and the duff in
four depths of haze each. Shafts of light come down from the upper left through the mist in Dawn Mist, less in
Overgrown, as moonbeams at night (the mood row's `beam`), drawn with the fog's own veil: a pixel takes a lighter
shade of its kind. The ruin is small among the trees: a goose pen (a burnt hollow) in the second trunk on the
left, a fallen log, a stump, mossy stones by the path. The three hollows that sing are the hollow trunk (the
root), the log and the stump; what falls from above is a cone, with the stone's knock. A leaf on the path is a
note, in the ferns a touch. The moon crosses low over the path's end and shows through the ghost trunks of the
two farthest depths, as the dawn sun does; otherwise the sounds and the script are unchanged.

## 2026-11-11 M3: Frozen Lake

Built from the plan's step 4 with the open calls settled here.

**The groan is the SAMPLE source's first use by a scene.** A crack cannot be synthesized and is a one-shot through
ZEN-1's pipeline, as the crows were; a groan of ice is a pitched texture, and the brief's SAMPLE is a tape loop
whose key is its speed. So Frozen Lake's part 7 is SAMPLE on Crawl's sample 0, a 1.6 s cut of lake ice groaning
(its fundamental about 60 Hz, `LK_GROAN_F0`), and a groan is a note on it: the chord's root brought into 73..147 Hz
is turned into the cents that play the sample at that speed from its root of 60 (the sample's root stays C4, as
`sample_test` and the tool assume; the scene knows the recording's own pitch). START, LENGTH and REVERSE are rolled
each time (a groan from the attack or from the tail, forward or backward), so no two are the same. M-SOFT, which
proved the source in M2, left the image for it: 16.5 KB back.

**The keep's bell is a recording, not PHYS.** Crypt's bell is PHYS MODAL; a bell heard across a frozen lake from a
keep is what one CC0 recording literally is (Neuenburg Castle's hour bell tolling midnight across the Unstrut valley,
blaukreuz), so one stroke of it is the toll, played a little slower or faster each time, and PHYS keeps to the reeds
(one STRING note), well inside M1's two. The cost is 8.8 KB of flash.

**No wind recording.** Steven's rule from Forest Ruins: recordings of wind read as hiss; the synthesized breeze is
the air in every mood, and Whiteout is its puffs with the Air knob up. The snow is silent but for a flake on the
near ice.

**Vrymaa's "Ice - Fracture" file is not ice** (its page: pitched-down leather), so it was not used, clean as it is.

**Each mood has its own event source, sources and sight.** Grey Dawn: the bell is the clock and a swell answers it,
a flake landing on the near ice is a touch, the horizon's pale band and the keep's smoke. Thaw: the cracks come
thick and each is a note, meltwater drips and trickles, the pools hold the sky; no snow, no bell. Whiteout: the
gusts are the events, each singing the reeds (PHYS STRING) and driving the snow in streaks; the far shore and the
keep go into the white in the palette (what is far mixes toward the sky by depth band, nothing redrawn); the bell
muffled through it; GRAIN on. Black Ice: the groans (SAMPLE) and the fractures' whistles (a Rin note gliding down a
fifteenth in a third of a second, the "pew" of lake ice); the stars and the keep's window in the ice (the far shore's
reflection is palette: ice by day, a smear by night); no snow; the bell at midnight only. Long Night is a day in that
order, started wherever the day is.

**"Grey Morning" is "Grey Dawn" on the panel.** Twelve characters do not fit the knob bar's 11 (as "Hollow Night"
did not); Grey Dawn is what it shows.

**Frozen Lake takes project slot 0, Sakura's.** The project has four slots; in the FM-1 image Sakura is out, so slot
0 is free there; in the simulator the two share it as Koi and Forest Ruins share slot 2 (the saved mood and tweaks
are read by whichever enters; the scripts do not switch between them with saved state). When the ZEN-1 scenes go,
the four Crawl scenes own slots 0 to 3.

**The ice's haze bands run the other way from the trees'.** Forest Ruins' depth band 0 is the haze and 3 is near; the
ice follows that, and the first picture had it inverted (the near ice hazy, the far ice clear, the keep's reflection
a pale block by day): the reflection's entries are now one a depth band, so by day it is the ice there at every
depth.

**The scene list grew again, and the mood render scripts count too.** `crypt_moods.zen` was not given its extra
click when Forest Ruins came in, so it had been rendering Forest Ruins under Crypt's names: now every script that
steps round the list from Sakura is in `docs/PROGRESS.md`'s list, the render tools included.

**The composition was not confirmed first.** Steven asked for one round on the view before the moods were built;
the session was not interactive, so the picture went ahead with the moods on it, built so that a change of view
costs the layout and the background alone (the cracks, snow, pools, reflections, whiteout and night are drawn over
it or live in the palette).

## 2026-11-12 M3: Hotaru out of the FM-1 image

**Hotaru leaves the image the way Koi and Sakura did**: `ui.c` `CRAWL_HOTARU`, 0 for the FM-1 image, 1 for the host
simulator and the browser build, its frog recordings with it (`bed.c`, `FM_SHOT_FROG` under `FM_HOST`, as the knock).
Night Sky did not fit beside it (12 KB free against about 25 KB), and Night Sky takes over what of Hotaru Crawl keeps:
the moon (`scene.h` `sc_moon_*`, shared code, which stays) and project slot 3. The owl recordings stay: Forest Ruins
uses them, and Night Sky does now. Measured: 57.7 KB of flash and 5.6 KB of RAM back (the plan's estimate was 32 + 24).
The simulator's scripts that count on Hotaru (`hotaru.zen`, `ambient.zen`, `scenes.zen`, `gallery.zen`) keep working
there; the simulator's list is unchanged until step 6.

## 2026-11-12 M3: Night Sky

Built from the plan's step 4 with the open calls settled here.

**The view: a moor under the sky, a ring of stones on the brow of a ridge.** The brief's scene is the sky, so the sky
takes three quarters of the picture: the horizon at 176 of 240, the ridge rising to a brow on the left where seven or
eight standing stones stand in a ring (two under a lintel, now and then one fallen), three pines at the right edge, the
ground below nearly black. The stones give the moon something to rise behind and the eclipse a place (a red moon over
a stone ring); they cost nothing each frame, being background. Steven asked for one round on the composition before
the moods were built; the session could not wait, so the picture went ahead with the moods on it, built so that a
change of view costs `ns_layout` and `ns_background` alone: the moon, stars, Milky Way, aurora, clouds, meteors,
rooks, frost and eclipse are drawn over the background or live in the palette, and nothing of them depends on the
ridge but its height at an x (`ns_ridge_y`).

**ZEN-1's lunar phase logic is reused whole.** `sc_moon_pick`, `sc_moon_t_of` and `sc_moon_draw` give each picture a
moon of its own size and phase and put it on the path its phase sets (a full moon up all night, a waxing crescent low
in the west at dusk, an old moon rising late). A mood holds a time of night (`night`: Dusk 0.10, Cold Clear 0.35,
Eclipse 0.50, Deep Night 0.84) reached the shorter way round over 30 s (`sc_glide`), and its own moon (a thin waxing
crescent, a gibbous, the full, an old crescent) reached over 10 s with the shadow sweeping across its face, through
the full moon when the side changes, after Hotaru's glide. Long Night runs the night 0 to 1 in half an hour with the
night's own moon, repicked each dawn.

**The eclipse is the Eclipse mood's event source, and Long Night's midnight only on a full-moon night.** The earth's
umbra (2.6 moon radii) crosses the moon over four minutes: first contact, totality (copper, the umbra's heart darkest,
a dithered penumbral fringe), last contact; a gong (PHYS MODAL, low, ringing) at each contact and once at mid-totality,
a drum far off (PHYS MEMBRANE) through the partial phases, slower at the limb and quicker toward totality, silent in it
(the drums beaten at an eclipse to drive the dragon off). In the Eclipse mood the cycle repeats; in Long Night it runs
once across the midnight quarter when the night's moon is full (two nights in five), and on the other nights that
quarter sends its clouds toward the moon instead, so the brief's "clouds over a moon" has its home there: a cloud
covering more than half the disc is a swell and darkens the sound (depth), the moon clearing is a note. A black key
starts an eclipse in any mood; it ends with the mood (a crescent in the umbra is nonsense).

**Each mood has its own event source, sources and sight.** Dusk: the stars come out one by one, a soft note each
(the brightest first; when all under the mood's cap are out, one of the faintest goes back to come out again), rooks
cross the afterglow in a skein with a far caw, clouds lit from below, the warm pad, an evening breeze. Cold Clear: the
stars scintillate (a flaring star is a chime, part 3), the aurora stands in curtains over the ridge and its pulses are
swells with the nuns opening on them, frost glitters on the ground, a gibbous moon high, no clouds. Eclipse: above.
Deep Night: the meteor shower from one radiant, each a falling run of chimes, a fireball in five with a train that
fades and a swell, the Milky Way at its brightest, no moon until an old crescent rises, one hollow voice, the owl.
Under every mood the stars blink: now and then one wavers and is a note, the scene's heartbeat, so no quarter of Long
Night is without notes. "Chimes" (part 3) has its own Volume knob, since the meteors and glints are the loudest things
in the sky.

**No new recording, and no use for the SAMPLE source.** Nothing in a night sky makes a sound that cannot be
synthesized; the air is the synthesized breeze (Steven's rule from Forest Ruins); the rooks are Forest Ruins' crows,
far and quiet (velocity 16 to 26, slowed a little), the owl is Hotaru's, far, in Deep Night only, every two to four
minutes. Both were in the image already. Part 7 is free. `assets/SAMPLES.md` says so.

**VOICE's BREATH is used once**: Deep Night's solo voice at 20. The other choirs have none.

**"Sky" on the panel's tab, "Night Sky" nowhere the bar must fit.** The mood names (Long Night, Dusk, Cold Clear,
Eclipse, Deep Night) are the brief's, all within 11 characters; `textfit` is checked in `sky.zen`.

**Night Sky takes project slot 3, Hotaru's**, free in the FM-1 image, shared in the simulator as the other three
Crawl scenes share theirs. Night Sky is last in the simulator's list; every script that steps round the list from
Sakura took one more click (`crypt.zen`, `forest.zen`, `lake.zen`, `ambient.zen`, `scenes.zen` twice, and the three
mood render tools).

**The night skies were brightened after the first pictures.** Under the panel's dark curve (80) the first zenith and
horizon colors went to black in the simulator's pictures; they were raised to about Frozen Lake's Black Ice (horizon
0x2C4070, 0x302444, 0x182440), and totality darkens the sky by a quarter, not a third. The Milky Way's own dust of
faint stars has a palette entry of its own, seen only with the band (drawn in the stars' white it was a line of dots
across the dusk).

## 2026-11-12 M3: Long Night across the scenes

Step 5 of the plan, settled from the project's context before building.

**It is one night that passes from scene to scene, and each scene's own long form is a chapter of it.** A scene's
Long Night was already a half-hour round through its four moods. The cross-scene mode does not replace that with
something flatter: the night holds a scene for one whole round of its own long form and then passes to the next
scene, which starts its round at its first quarter. So nothing of a scene's moods is touched, and the night is two
hours before it comes back to where it was.

**The order and the time are a table in the glue** (`ui.c` `AMB_NIGHT`: a scene and its minutes, a row each): Crypt,
Night Sky, Forest Ruins, Frozen Lake, 30 minutes each. The order is not the PRESETS list's, because every join then
reads forward: Sealed gives way to Dusk (out of the tomb at nightfall), Deep Night to Dawn Mist, Owl Moon to Grey
Dawn, Black Ice to Vigil. In the list's own order Black Ice would run back into Dusk. A row's minutes are the length
of that scene's round while the night is in it, so the time held and the round cannot disagree.

**The door is each scene's mood 0, as the plan said.** Choosing a scene whose mood is Long Night enters the night at
that scene, wherever its round is (the seed's, as before, but never in the round's last tenth: a scene just chosen
does not pass on within three minutes). The first pass comes when that round ends.

**The night is a guest in the scenes it passes into.** `proj.scene` stays the door: a power-on starts where the
player last chose to be, and a pass writes nothing to flash (no save, so no breath of silence). A guest scene's own
saved mood is left alone (it may be Storm; the night shows its Long Night and goes on). Knobs turned in a guest are
that scene's Long Night's tweaks, kept in its own slot as ever. Turning the Mood in a guest makes the scene the
player's choice (it becomes `proj.scene`, saved soon): another mood ends the night there, Long Night again takes it
up from there. PRESETS works as ever, counted from the scene on the screen.

**The handover: the picture through dark, the sound across.** The picture fades to dark over 4 s before the round
ends (the scene and its sound going on), the scenes change in the dark, and the next fades in over 4 s. The glue's
PRESETS change calls the old scene's `leave` (every note let go, the echo, tape, cloud and beds to nothing) and then
the new scene's `enter`, which gives the parts new patches at once; the engine applies a part's patch to its
sounding notes, so released notes change sound mid-release, and the echo's tail is cut. For the night that is not
good enough. So a pass does not call `leave`:

- The engine learns one thing, `fm_voice_pass`: the parts as they are now are copied to seven spare parts, every
  scene note moves onto its copy and is let go there. The old drone (4.5 s release), the last notes and swells ring
  out with their own patches, levels and releases while the next scene's first drone breath comes in at once.
  Recordings that are sounding (a toll, a caw) play out instead of being cut.
- The echo line, the tape and the reverb are not reset: the next scene sets its own, and the engine glides the
  echo's time, the sends and the tape's amount. The beds are set to nothing and to the next scene's in the same
  frame (they ease over a quarter of a second).

**A pass is the dawn.** The scenes called `cd_dawn` at the end of a round (the key may move a fourth or a fifth,
by the Key page's Drift). The pass comes just before the round's end and calls it once; the key it leaves and the
count of dawns are carried into the next scene's conductor (`cd_pass`, `cd_init`), so the night keeps one key from
scene to scene and Drift's "every third dawn" counts across them. The next scene starts in that key at once, on
its first quarter's scale, so there is no glide a moment after the join.

**Drift: Off holds the night in its scene.** That is what a scene's own long form becomes when it is wanted alone:
with the Key page's Drift at Off nothing moves on at the end of a round, not the key and not the scene, and the
round goes on for good as it did before step 5. No new knob, mood place or project field.

**What persists.** The project is unchanged in format: the four slots, each scene's moods and tweaks, the Key page.
The sleep timer runs across a pass (its time, its dimming, its fade); the knob page and the panel lights' auto-off
are not disturbed (a pass is not a touch). Asleep, the night goes on unheard, so waking finds it where it would be.

**A scene learns one thing: its long form's clock is kept through `scene.h`** (`sc_long_begin`, `sc_long_step`), so
the glue can say where a round starts and how long it is, and can see where it is. Each of the four scenes changes
two lines for it (Night Sky a third: its midnight eclipse is timed from the round's length).

**ZEN-1's scenes are not in the table**, so their Natural is as it was until they go (step 6).

## 2026-11-12 M3: Long Night across the scenes, after building and measuring

**The old drone holds three seconds into the next scene before it lets go** (`fumi.c` `FM_PASS_HOLD_S`). With every
note let go at the pass, the first renders dipped 12 to 16 dB for about two seconds, one to three seconds after each
join: the old drone falls 13 dB a second and the new scene's first breath takes two seconds to come in. Now a kept
part's held notes (the drone) hold for 3 s on their copies and then release; the lowest half-second within 3 s of a
join is within the scenes' own breathing (below).

**A round that ends does not turn back.** A scene's long form blends toward the next mood in the last 30 % of each
quarter, and in the last quarter that is its first mood again: at the end of its round Crypt was already Vigil and
Night Sky already Dusk, so the joins were not the ones the order was chosen for. While the night will pass on, the
last quarter holds (`scene.h` `sc_long_ends`, `sc_long_next`; one more line in each scene, part of the same one thing
a scene learns: its long form's clock and end are the glue's). With Drift at Off the round goes round as before.

**The kanji bitmaps stay in the FM-1 image.** Step 5 cost 1072 B of flash and 1776 B of RAM (the seven spare parts);
44 KB of flash and 14.5 KB of RAM are free, so nothing had to leave.

**Not done on purpose.** PRESETS changes still go through `leave` and `enter` as before (released notes take the new
patches, the echo is cut): the pass could carry those too, but that changes every scene change and every script that
counts voices after one, so it is left for its own step. No name is shown at a pass: the night is for a dark room;
HOME's status card says where it is.

## 2026-11-12 M5: the browser emulator, for testing by ear

Steven asked for ZEN-1's emulator reused as it is, with Crawl's content, before anything is heard on hardware.

**The browser build has Crawl's four scenes only.** `web/emu/build.sh` passes `CRAWL_SAKURA`, `CRAWL_KOI` and
`CRAWL_HOTARU` as 0, as the FM-1 image has them, so the page plays what the FM-1 will. The simulator keeps the three
and its scripts. Two guards in `host/zen_host.c` were needed for that to compile: the script runner (`expect` and
`main`, which read ZEN-1's scenes' state) is now all under `#ifndef OM_WEB` (only `main` was; the browser build never
ran scripts), and the moon-jump tracking in `run_ms` is under `CRAWL_HOTARU`. The simulator's build is unchanged by
both. The wasm went from 976105 B to 761621 B.

**The page is a text swap.** Scene names, the moods of each, what the white and black keys do in each scene, the
new-picture line, the tooltips for the keys, the tagline, a credits line (ZEN-1, GR!MM, FoMni, Groove OS) and the
"unofficial" line the brief requires. The layout, panel, input handling, worklet and `zen_web.c` are ZEN-1's,
untouched. The "koto instrument" section stays: the instrument is still in the firmware under HOME, and its Japanese
naming goes with step 6 of M3. The page's "About" and "guide" links point at pages M5 has not made yet.

**An emulator-only site.** ZEN-1's `tools/make_pages.py` and `tools/publish_pages.sh` make the whole site and need a
release package, the installer and the landing page, none of which exist for Crawl yet (`web/site` is still ZEN-1's
text). So `make_pages.py - LABEL OUT` writes `emu/` and a root page that goes to it, and `publish_pages.sh --emu LABEL`
pushes that. The script's remote is now `origin` by default (Crawl has one repository; ZEN-1 had a second, public
one), Python is `$PYTHON`, and without `emcc` on the path (Git Bash on Windows) it publishes `build/emu` as
`web\emu\build.ps1` built it. The full-site path is as it was.

**Not published: GitHub Pages is not available for this private repository on the account's plan.** Asked through
`gh` (`POST repos/keuller-clone/crawl/pages` with `build_type=workflow`, which deploys nothing): "Your current plan
does not support GitHub Pages for this repository" (HTTP 422). Nothing was changed: no `gh-pages` branch, the
repository private. The emulator runs from a local server meanwhile (`docs/PROGRESS.md`). Two things to weigh before
it is published from a private repository on a paid plan: the site itself is public whatever the repository is
(access control for Pages is an Enterprise Cloud feature), and the page then hands out a build of GPL-3.0 code whose
"Source" link nobody else can open, which the license's source-offer terms do not allow for long.

## 2026-11-12 M5: the repository public, the emulator on Pages, the README

**The repository is public now, by Steven's word** ("Let's just make this project public. I don't care about hiding
it"), not at release as the brief had it. Pages was still refused for the private repository, and a public one
settles the source question above. It is not a release: nothing is flashed, there is no installer and no package,
and the README says so in its Status.

**Pages serves the `gh-pages` branch, the emulator alone** (`tools/publish_pages.sh --emu m3`; GitHub enabled Pages on that branch by itself at the first push). The root goes to `emu/`.

**The README is Crawl's, and the page says what it says.** The README was ZEN-1's under a Crawl preface. It now
describes the four scenes, every mood, Long Night, the sources and effects, every control, the keys of each scene
and the knob pages scene by scene (read from each scene's knob table: Crypt's Nature and Music pages are laid out
differently from the others'), the koto instrument and the browser's mouse, keyboard and shortcuts. The emulator
page keeps ZEN-1's layout; its lines were brought to the README's wording where they differed (the sleep timer, the
Long Night order, Crypt's white keys), and its About and guide links go to the README, since there is no landing
page or guide yet. `web/site/` (ZEN-1's landing page and guide) is not rewritten and not published; the README is
the guide until M5 makes one.
