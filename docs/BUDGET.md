# Budget (M1 spike)

2026-11-10. What Crawl has to spend on the FM-1, what it spends now, and what the GR!MM parts will cost.
Every figure says where it comes from. Three kinds: **measured** here (this tree, this toolchain), **read** from
source (exact arithmetic on declared sizes, or an upstream author's own note), and **estimated** (a measured host
figure scaled by one upstream hardware reading). Nothing here was measured on Crawl's own hardware: it has not
been flashed.

## The chip and the callback

| What | Value | Source |
| --- | --- | --- |
| MCU | JieLi AC791N (AC79 family, `wl82`), pi32v2 core `r3`, single-precision FPU | read: GR!MM `docs/RULES.md`; `tools/build.py` (`-mcpu=r3 -mfprev1`) |
| Time base | TIMER4, 24 MHz free-running, 24 ticks a microsecond | read: `firmware/hal/fm1_time.h` |
| Sample rate | 44100 Hz stereo | read: `firmware/src/dsp/fumi.h` (`FM_SR`) |
| Audio block | 256 frames a call to `fm_render` | read: `firmware/src/app/main_fm1.c` (`REND_FRAMES`) |
| Block budget | 5805 us | 256 / 44100 |
| On-device meter | `plat_cpu_pct()` (smoothed) and `plat_cpu_max()` (worst block), shown by the testing HUD | read: `main_fm1.c` `render_block` |
| Interrupts inside the block | 10 kHz key-scan timer, USB | read: `fm1_timer.h`; GR!MM `RULES.md` advises staying under half the block (2.9 ms) |

ZEN-1 uses the FPU (float DSP). GR!MM is built integer only (its rules say no float). Integer code runs
unchanged in Crawl's float engine, so the GR!MM parts port as they are, with a conversion at the mix.

## Memory: what is left today (measured)

`.\build.ps1`, then `tools/budget_map.py` on the ELF's symbols (commands at the top of that script).

| Region | Size | Used | Free |
| --- | --- | --- | --- |
| Flash, app slot (XIP) | 581564 B | 567880 B | 13684 B |
| RAM (`.data` + `.bss`) | 98304 B | 96792 B | 1512 B |
| Pool (big buffers) | 344064 B | 334020 B | 10044 B |

Crawl at M0 is full in all three. Nothing from GR!MM fits until ZEN-1 content comes out.

Where the flash goes:

| Group | Bytes |
| --- | --- |
| Nature recordings (`zen_samples.h`) | 279344 |
| Scenes: koi 47264, sakura 39120, hotaru 31752 | 118136 |
| Fonts | 54926 |
| Jump tables (switch statements) | 28234 |
| Platform | 15467 |
| UI, panel, project | 14947 |
| Kanji bitmaps | 14464 |
| FM6 core, instrument, patches, conductor, shared scene code | 33138 |
| Other | 9224 |

Where the RAM goes: Sakura 41188 B (one table, `sk_cst`, is 24800), voices, parts, beds and audio buffers
21184, platform 8139, koi 5856, hotaru 5136, shared scene state 5681, other 9432.

Where the pool goes: the reverb and echo lines `pl_buf` 135300 B (float), the 8-bit renderer's two 240 x 240
buffers and sums 129600, the UI canvas `cv_px` 69120.

## Memory: what the GR!MM parts need (read from `refs/grimm/v2.0.2`)

| Part | Pool | RAM | Flash data | From |
| --- | --- | --- | --- | --- |
| DELAY (tape echo, 3.5 s at 22.05 kHz) | 154912 B | small | small | `tape.c`: `dl_buf[77456]` int16 |
| GRAIN (1.25 s ring, 8 grains) | 55126 B | small | 514 B window | `grain.c`: `ge_buf[27563]` int16 |
| SAMPLE (per voice: 4 windows of 257) | 2056 B a voice | small | none in the image | `sampler.c`: `sp_w[NV][4][257]` int16 |
| SAMPLE slots (4 x 72 KiB ADPCM, 6.6 s each) | 6864 B seek index | small | 288 KiB **outside** the app slot, at 0x97000..0xDF000 | `samples.c`, `platform.c` |
| PHYS (per voice: the largest model's state) | none | about 2.7 KB a voice | small | `phys_dsp.c`: string line 512 + 128 int32; `phys_symp.c`: 3 x 256 int16 |
| VOICE (formant choir) | none | 28 B a voice | 28800 B choir table | `formant.c`; `grimm_choir.h` `CHOIR_T[2][3][2][5][480]` |
| TAPE (wow, saturation, hiss) | none | 3076 B curve | small | `tapefx.c`: `tf_curve[2][769]` int16 |

Code sizes are in the CPU section below. GR!MM's own build confirms the buffers: `dl_buf` 154914 B, `ge_buf`
55126 B, `sp_w` 12336 B (6 voices), `ph_slot` 25776 B of RAM (6 voices, 4296 B each, more than the 2.7 KB read
from the largest line alone).

As GR!MM has them, DELAY and GRAIN alone want 210 KB of pool. The pool is 344 KB and the screen needs 199 KB of
it (the renderer and the canvas), so the effects have about 145 KB between them, and ZEN-1's reverb now holds
135 KB of that.

Two flash-map conflicts to settle in M2: GR!MM's sample area (0x97000..0xDF000) overlaps ZEN-1's save journal
(0xC0000, 8 sectors) and its panel object (0xDC000). Crawl cannot take both layouts as they are.

## CPU: ZEN-1's engine (measured on the host, then estimated for the FM-1)

`tests/host/budget_bench.c`, in the build container (gcc 13.3, `-O2 -ffp-contract=off`), median of 512 blocks,
two runs within 0.2 us of each other.

| Configuration | Host, us a block | Over idle |
| --- | --- | --- |
| Idle: no voices, reverb 0, beds 0, tape 0 | 20.3 | 0 |
| FM x1 / x2 / x4 / x8 / x12 (soft pad, held) | 29.1 / 34.3 / 44.0 / 63.6 / 83.1 | 8.8 / 14.0 / 23.7 / 43.3 / 62.8 |
| Koto x8 + reverb 100 | 63.2 | 42.9 |
| Reverb 100 against reverb 0, 8 voices | 63.8 against 63.6 | no difference: the reverb runs either way |
| ZEN-1's tape at 100 (one voice) | 42.7 | 13.6 over one voice |
| Bed: wind / stream / recorded brook / recorded insects | 26.7 / 33.6 / 28.1 / 24.5 | 6.4 / 13.3 / 7.8 / 4.2 |
| Everything: FM x12, reverb, tape, three beds | 113.0 | 92.7 |

An FM voice costs about 5 us a block on the host after the first (the first carries about 4 us of shared work).

One hardware reading exists, and it is ZEN-1's author's, not ours: `fumi.h` records that 8 koto voices with
reverb measured 38 % on the FM-1, and puts 12 at about 60 %. The host reads 63.2 us for the same case, so one
host microsecond is about 0.6 % of an FM-1 block (a factor near 35). Scaling the table by that:

| Configuration | FM-1, estimated |
| --- | --- |
| Idle floor (reverb, mix, smoothing) | about 12 % |
| Each FM voice | about 3 % |
| FM x12 | about 50 % (the author's own figure: about 60 %) |
| ZEN-1's tape | about 8 % |
| A nature bed | 3 to 8 % |
| Everything on, 12 voices | about 68 % |

These are estimates. The host compiler and the pi32v2 compiler make different code, and GR!MM's rules warn
that a host count can land in the wrong place. They are good for ranking, not for a limit.

## CPU: the GR!MM parts (measured on the host, then estimated for the FM-1)

`tools/budget_grimm.py`, on a scratch checkout of GR!MM v2.0.2 (`build/grimm-ref`, not part of Crawl's build,
allowed by Steven for measuring). It builds GR!MM's host mode in the same container, plays every factory sound
with 0, 1 and 4 keys, and reads GR!MM's own per-stage timers. Host mode's clock has 1 us steps; each reading is
the median of four half-second averages.

Sources, per voice, chord off:

| Source | Host, us a voice | Sounds measured | FM-1, estimated |
| --- | --- | --- | --- |
| FM (GR!MM's, the same msfa core) | 5.1 (up to 7.0) | 12 | about 3 to 4 % |
| VOICE (formant choir) | 7.2 (6.8 to 7.6) | 10 | about 4 %, see the caution below |
| PHYS (physical models) | 18.8 (9.6 to 20.7, by model) | 7 | about 6 to 12 % |
| SAMPLE (tape loop) | 1.8 (2.1 for the first) | sound 1 with its source set to SAMPLE | about 1 % |

GR!MM's FM voice costs the same on the host as ZEN-1's (5 us), which supports using one scale for both.

Effects, switched on over a four-key pad (each is skipped whole when off):

| Effect | Host, us a block | FM-1, estimated |
| --- | --- | --- |
| DELAY (send 127) | 3.3 in its stage, 0.4 in the mix | about 2 % |
| GRAIN (send 127) | 1.5 in its stage, 0.4 in the mix | about 1 % |
| TAPE (tile 127, four knobs up) | 1.1 in its stage, 2.1 in the mix | about 2 % |
| All three | 8.1 over the pad alone | about 5 % |
| GR!MM's SPACE reverb, for scale (not ported) | 10.1, always | about 6 % |

The estimates use the same factor as ZEN-1's table (0.6 % of an FM-1 block per host microsecond). Caution:
that factor comes from float code, and GR!MM's code is integer. GR!MM's own source says six choir voices are
the most the FM-1 has time for (`formant.c`, `VT_SING`), and GR!MM runs 6 voices in all against ZEN-1's 12.
That fits a hardware cost for VOICE and PHYS two to three times this estimate. Until hardware says otherwise,
plan with the higher figure.

GRAIN was measured at its default size and density, without FREEZE.

Code and table sizes in GR!MM's FM-1 image (measured: its ELF, built with the same toolchain; GR!MM is one
compilation unit, so small functions are inlined and counted with their caller):

| Part | Code | Tables |
| --- | --- | --- |
| VOICE | 4028 B | 28856 B (the choir waves) |
| PHYS | 8838 B | 4510 B |
| SAMPLE engine | 3598 B | 186 B |
| TAPE | 628 B | none |
| GRAIN, DELAY | inlined into the 2662 B audio callback | 532 B, 256 B |
| GR!MM's four factory samples | none | 291676 B in the image, copied to the sample area at boot |

Without samples, everything Crawl ports is under 20 KB of code plus the 29 KB choir table.

## Recommendation

**Mode B (sequencer): no go for v1, provisionally.** The reason is memory, which is measured, not CPU. All three
regions are full today, and Mode A's own effects need more pool than exists once the screen is paid for. A
sequencer adds pattern storage, a second UI and a second voice path on top of that. The brief says not to
degrade Mode A to make room. Revisit only if M2 ends with pool and flash to spare.

**Mode A plan.**

1. Free memory before porting. The recordings (279 KB of flash) and the three scenes (118 KB of flash, 52 KB
   of RAM) are replaced in M3 anyway. Removing a scene and its recordings early gives M2 room to work.
2. Give the effects a fixed pool budget of about 145 KB. A workable split: keep ZEN-1's reverb but store it as
   int16 instead of float (135 KB becomes about 68 KB, to be confirmed by listening), DELAY at 1.5 s (about
   66 KB) instead of 3.5 s, and GRAIN's ring shortened or sharing DELAY's line. As GR!MM sizes them they do not
   fit.
3. Polyphony: plan for 8 voices in all, not 12, shared across FM, VOICE, PHYS and SAMPLE, with at most 4
   VOICE and 2 PHYS at once, until hardware figures say otherwise. A worked case with the estimates: idle 12 %,
   three effects 5 %, 4 FM 12 %, 2 VOICE 8 to 24 %, 2 PHYS 12 to 36 %: 49 % at the low figures, 89 % at the high
   ones. GR!MM's rules ask for half the block kept free, so PHYS is the voice to ration, and each PHYS voice
   also needs 4.3 KB of RAM. SAMPLE voices are nearly free in time.
4. The effects are cheap in time (about 5 % together). Their cost is pool, as above.
5. SAMPLE: decide in M2 whether samples live in the image (as ZEN-1's recordings do) or in a flash area outside
   it (as GR!MM's do), since the two layouts collide.

## Open: what still has to be measured

- **GRAIN under FREEZE and at full density, and PHYS by model.** Measured only at defaults and across the
  factory sounds.
- **Hardware figures.** Every FM-1 percentage here is an estimate from one upstream reading. Smallest next
  step: with Steven's go-ahead, flash the M0 build and read the HUD's load for the bench's cases, then add a
  per-stage split like GR!MM's.
- **The reverb's true cost.** The bench shows it always running; its share of the idle floor is not separated.

## Audit, 2026-11-12: will Crawl as it stands fit and run on the FM-1?

Tree at `41cb0cb`, four scenes (Crypt, Forest, Lake, Sky), 16 moods and Long Night. One build (`.\build.ps1`),
then `tools/budget_map.py build/crawl.syms --top 400`. The build writes no linker map, so "the map" here is the
ELF's symbol table, as in M1. Nothing was changed, trimmed or flashed. Every FM-1 percentage is still an estimate.

### Verdict

| Budget | Verdict | Basis |
| --- | --- | --- |
| Flash | **Fits with margin.** | 537548 B of 581564, 44016 B free (measured). 95976 B more is in the image and used by no mood. |
| RAM | **Fits, tight.** | `.data` + `.bss` 83784 B of 98304, 14520 B free; pool 335596 B of 344064, 8468 B free (measured). All of it is static. Stack depth with PHYS running is not measured. |
| Audio CPU | **Cannot be known without hardware.** | At the estimates a full 12-voice pass is 56 to 99 % of a block by mood, over GR!MM's half-block advice in every mood; at the cautious figures 13 of 16 moods pass 100 %. |

### Build, confirmed

| Region | Size | Used | Free | Source |
| --- | --- | --- | --- | --- |
| Flash, app slot | 581564 B | 537548 B | 44016 B | build output |
| RAM (`.data` + `.bss`) | 98304 B | 83784 B | 14520 B | build output |
| Pool | 344064 B | 335596 B | 8468 B | build output |

The same as the last recorded build. The symbol table accounts for 534500 B of flash (3048 B fewer than the
image; not looked into, the build's figure is used) and 83624 B of RAM.

Where the tables above this section disagree with today's symbols, the symbols win: the reverb line `pl_buf` is
67652 B (it was 135300 B as float), the recordings are 196239 B (279344), RAM is no longer full. And the code
allows more than M1's plan: `FM_NVOICE` is 12 (the plan said 8), `FM_NPHYS` is 4 (the plan said 2), and VOICE has
no cap of its own below 12.

### Flash: the 15 largest symbols

| # | Bytes | Symbol | What | Mark |
| --- | --- | --- | --- | --- |
| 1 | 47988 | `ZN_BREEZE` | recorded breeze (air styles 1, 2) | **ZEN-1 leftover, no mood uses it** |
| 2 | 47988 | `ZN_INSECTS` | recorded insects (air style 3) | **ZEN-1 leftover, no mood uses it** |
| 3 | 28802 | `CHOIR_T` | VOICE's choir waves | Crawl needs it |
| 4 | 23736 | `ZN_OWL_0` | owl call | Crawl needs it (Owl Moon, Deep Night) |
| 5 | 17640 | `SMP_D0` | SAMPLE 0, the ice's groan | Crawl needs it (Lake) |
| 6 | 16832 | `FONT_L_DATA` | 31 px font | needed by the UI; use by size not checked |
| 7 | 16660 | `.GJTIE95_0_0_` | one switch's jump table | unsure: its switch was not identified |
| 8 | 14352 | `FONT_JPL_DATA` | the big kanji | ZEN-1 leftover, kept on purpose (PROGRESS: "the kanji bitmaps stay") |
| 9 | 12900 | `ZN_FLOCK_0` | flock call | Crawl needs it (by name; not traced) |
| 10 | 11832 | `FONT_M_DATA` | 23 px font | Crawl needs it |
| 11 | 11500 | `render_voices` | the audio render | Crawl needs it |
| 12 | 10124 | `fr_step` | Forest | Crawl needs it |
| 13 | 8772 | `ZN_TOLL_0` | toll recording | Crawl needs it (by name; not traced) |
| 14 | 8772 | `ZN_OWL2_1` | second owl | Crawl needs it (by name; not traced) |
| 15 | 8752 | `FONT_B_DATA` | 15 px bold font | Crawl needs it |

By group: recordings 196239 B, fonts 54926, jump tables 27730, UI 15723, platform 15670, kanji 14464, instrument
and render 11282, FM6 core 9333, conductor 5973, shared scene code 4708, patches 3112, everything else 175340
(the four scenes, the GR!MM engines, the choir table, the SAMPLE data).

Still in the image and used by no Crawl mood:

| Item | Bytes | Why it is still linked |
| --- | --- | --- |
| `ZN_BREEZE`, `ZN_INSECTS` | 95976 | `bed.c` picks them for air styles 1 to 3. All four scenes ask for `FM_AIR_BREEZE_SYNTH`. Forest falls back to `FM_AIR_BREEZE` when `air_want` is negative (`scene_forest.c:1144`); `FR_AIR_STYLE` is all synth, so that path looks dead, not proven. |
| Kanji (`FONT_JPL` and its tables) | 14464 | `ui.c:694` draws the instrument patch's Japanese name. |
| `FM_PATCH_JA`, `FM_PATCH_JA_LCD` | 112 | the same names |

Not leftovers, though they look it: the conductor (5973 B) holds every scene's drone; the Japanese scales are
moods' scales (`SC_IN` in Vigil, Storm, Whiteout; `SC_MINYO` in Thaw, Owl Moon); `ZN_PATCH` (1152 B) carries the
scenes' pads, rin, swell and furin. The koto instrument's own share (`FM_PATCH` 1792 B plus UI code inside shared
functions) could not be separated by symbol: not determined.

**What M3 step 6 would free, counted:** 95976 B of flash for certain, 14576 B more if the kanji decision is
reversed, the koto instrument's share unknown. No RAM: none of the ten largest RAM items is a leftover.

### RAM and pool

| # | Bytes | Symbol | What | Mark |
| --- | --- | --- | --- | --- |
| 1 | 17184 | `ph_slot` | PHYS, 4 notes at 4296 B | Crawl needs it; sized for 4 where the scenes plan 2 or 3 |
| 2 | 8228 | `sp_w` | SAMPLE windows, 4 notes at 2056 B | Crawl needs it; one part in one scene uses SAMPLE |
| 3 | 8192 | `ua_ring` | USB audio ring | platform (frozen) |
| 4 | 4560 | `vc` | 12 voices at 380 B | Crawl needs it |
| 5 | 4096 | `abuf` | audio buffer | Crawl needs it |
| 6 | 3840 | `st_buf` | storage payload | platform (frozen) |
| 7 | 3520 | `parts` | the parts | Crawl needs it |
| 8 | 2304 | `sc_mst` | the moon stamp | Crawl needs it (Sky, Forest) |
| 9 | 2048 | `r8_pal` | renderer palette | Crawl needs it |
| 10 | 1728 | `vx_c` | VOICE choir state, 12 at 144 B | Crawl needs it |

Pool, all of it Crawl's: `cv_px` 69120 (UI canvas), `pl_buf` 67652 (reverb), `td_buf` 62244 (not traced; by its
size and place the echo line), `r8_bg_px` and `r8_fb_px` 57600 each, `r8_sum` and `r8_sumf` 7200 each, `fxp`
3080. No leftovers.

### CPU: worst case by mood (estimates)

Basis: this file's host figures times 0.6 % of an FM-1 block a host microsecond. Floor 12 %; FM voice 3 %;
VOICE voice 4.3 %; PHYS note 11.3 % (the mean of the models); SAMPLE note 1.1 %; wind bed 3.8 %; stream bed 8 %;
DELAY 2.2 %, TAPE 1.9 %, GRAIN 1.1 %. "Cautious" multiplies VOICE and PHYS by 3, the caution already written
above for integer code.

Assumptions, each one a guess that the code was not read far enough to prove:

1. All 12 voices sound. The engine allows it; how often a mood gets there is not known.
2. The drone is 6 of them: the conductor's three voices, each with a second id while it breathes
   (`conductor.c:46`). It is VOICE when the mood's Voices knob is above 0, an FM pad at 0.
3. PHYS notes: one per struck part the mood uses, two for a bowed part (Lake's reeds, Forest's arches in Storm),
   read from the moods' tables and comments. Forest's arches are assumed to sound once in every Forest mood.
4. The rest are FM (notes, touches, swells).
5. The rain bed costs what the stream does (not measured). A one-shot recording costs 1 % (not measured).
6. DELAY and TAPE are on in every mood (every mood sets Echo and Tape above 0); GRAIN where the mood's cloud is
   above 0. The tape is GR!MM's (2 %), not ZEN-1's (8 %).
7. The floor is M1's 12 %, measured with the float reverb; the reverb has changed since.
8. Handover: the caps, 4 PHYS and 6 VOICE, the old scene's three beds still easing out. The seven extra LFOs a
   block are not counted.

| Risk | Mood | FM | VOICE | PHYS | SAMPLE | Shots | Beds, % | Effects | Estimate, % | Cautious, % |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | Long Night handover | 2 | 6 | 4 | 0 | 2 | 20 | DELAY, TAPE, GRAIN | 116 | 258 |
| 2 | Forest / Owl Moon | 4 | 6 | 2 | 0 | 1 | 4 | DELAY, TAPE, GRAIN | 82 | 179 |
| 3 | Sky / Eclipse | 4 | 6 | 2 | 0 | 0 | 4 | DELAY, TAPE, GRAIN | 81 | 178 |
| 4 | Lake / Whiteout | 4 | 6 | 2 | 0 | 0 | 4 | DELAY, TAPE, GRAIN | 81 | 178 |
| 5 | Crypt / Procession | 4 | 6 | 2 | 0 | 0 | 0 | DELAY, TAPE | 76 | 173 |
| 6 | Forest / Storm | 9 | 0 | 3 | 0 | 2 | 20 | DELAY, TAPE | 99 | 167 |
| 7 | Forest / Overgrown | 5 | 6 | 1 | 0 | 2 | 4 | DELAY, TAPE | 74 | 148 |
| 8 | Crypt / Embers | 5 | 6 | 1 | 0 | 0 | 4 | DELAY, TAPE, GRAIN | 73 | 147 |
| 9 | Crypt / Vigil | 5 | 6 | 1 | 0 | 0 | 0 | DELAY, TAPE | 68 | 142 |
| 10 | Lake / Thaw | 6 | 6 | 0 | 0 | 0 | 12 | DELAY, TAPE | 72 | 123 |
| 11 | Sky / Deep Night | 6 | 6 | 0 | 0 | 1 | 4 | DELAY, TAPE, GRAIN | 66 | 117 |
| 12 | Sky / Cold Clear | 6 | 6 | 0 | 0 | 0 | 4 | DELAY, TAPE | 64 | 115 |
| 13 | Lake / Black Ice | 5 | 6 | 0 | 1 | 0 | 4 | DELAY, TAPE, GRAIN | 63 | 114 |
| 14 | Forest / Dawn Mist | 11 | 0 | 1 | 0 | 1 | 4 | DELAY, TAPE | 65 | 88 |
| 15 | Crypt / Sealed | 12 | 0 | 0 | 0 | 0 | 4 | DELAY, TAPE, GRAIN | 57 | 57 |
| 16 | Sky / Dusk | 12 | 0 | 0 | 0 | 1 | 4 | DELAY, TAPE | 57 | 57 |
| 17 | Lake / Grey Dawn | 12 | 0 | 0 | 0 | 0 | 4 | DELAY, TAPE | 56 | 56 |

Ranked by the cautious figure (`build/tmp/audit_cpu.py` did the sums). What the table says: twelve FM voices
alone are already past half a block; PHYS decides the ranking; a VOICE drone moves a mood from the bottom group
to the middle. These are ceilings under assumption 1, not what a mood does minute to minute. A steady state of 3
drone voices and 3 or 4 notes is 35 to 45 % at the estimates.

Display costs flagged as unmeasured (from PROGRESS; none can be estimated from the host):

- Crypt: the palette changes up to four times a second and each change sends the whole screen.
- Sky / Cold Clear: the aurora's draw, 240 columns by up to 70 rows, the costliest part of a frame.
- Sky: the moon stamp is remade up to about 25 times in a mood's 10 s phase glide, each about 5.9 ms.
- Long Night: how long `enter` takes while the picture is dark, and whether the 4 s fades look smooth.

### Trim list, most freed per unit of risk first

| # | Trim | Gives back | What is lost |
| --- | --- | --- | --- |
| 1 | Take `ZN_BREEZE` and `ZN_INSECTS` out of the FM-1 image (step 6) | 95976 B flash | Nothing a mood plays. Prove the Forest fallback dead first. |
| 2 | `FM_NSAMPLE` 4 to 2 | 4112 B RAM | Nothing today: one SAMPLE part exists. |
| 3 | `FM_NPHYS` 4 to 3 | 4296 B RAM; 11 % (cautious 34 %) off the handover's ceiling | A fourth PHYS note steals the oldest, during handovers only. |
| 4 | `FM_NPHYS` 3 to 2 (M1's plan) | 4296 B RAM; the same again off Storm | Storm's arches, thunder and stones can no longer all ring. |
| 5 | `FM_NVOICE` 12 to 10, then 8 (M1's plan) | 3 % a voice (cautious 13 % if VOICE); about 550 B RAM a voice | Earlier stealing in dense passages; the handover's overlap thins. |
| 6 | One id a drone voice (no doubled breath) on VOICE | up to 13 % (cautious 39 %) | The choir's breaths stop overlapping. |
| 7 | Storm without the stream bed's roar | 8 % in Storm | Part of the storm's body. |
| 8 | Kanji out | 14576 B flash | The patch's Japanese name; reverses a decision. |
| 9 | `VT_SING` 6 to 4 | not measured | Thinner choirs. |

Flash needs none of this. RAM needs none of it to link; 2 and 3 are free headroom. CPU: do nothing before
hardware figures.

### Hardware measurement plan

One flash of the testing build as it is settles everything below. Read the HUD's load (`plat_cpu_pct`, smoothed)
and worst block (`plat_cpu_max`). Nothing is prepared.

| # | Where | Read | It gives |
| --- | --- | --- | --- |
| 1 | Instrument, no keys | load | the floor (estimate 12 %) |
| 2 | Instrument, 4 then 8 keys held | load | an FM voice; checks the author's 38 % at 8 |
| 3 | Lake / Grey Dawn, 5 minutes | load, worst | an all-FM scene, with the display running |
| 4 | Crypt, any mood, Voices from Pad to Cathedral | the change in load | a VOICE voice (divide by the drone's voices) |
| 5 | Crypt / Procession, at a toll | the step in load | a PHYS note (MODAL, MEMBRANE) |
| 6 | Lake / Whiteout in a gust | the step in load | a bowed PHYS note (STRING) |
| 7 | Forest / Storm, Forest / Owl Moon, Sky / Eclipse, 5 minutes each | worst | the risky moods' real peaks |
| 8 | Long Night through two handovers | worst, and listen | the overlap; the 0.11 s burst |
| 9 | Crypt / Embers, Sky / Cold Clear, a Sky mood change | worst, frame rate, clicks | the palette resend, the aurora, the moon stamp |

Numbers that mean "trim" (proposed here, not from a measurement): the worst block at 85 % or more in any mood,
or the smoothed load above 50 % for minutes (GR!MM's half-block rule). By source: a PHYS note above 12 % means
trim 4, a VOICE voice above 6 % means trims 5 and 6, a floor above 20 % means the reverb needs its own look.
A second flash is needed only to check a trim.

### Not determined

- Stack depth; the 3048 B between the symbol table and the image; what `td_buf` and the 16660 B jump table are.
- The koto instrument's own flash share.
- How many voices each mood really reaches (assumption 1), and whether Forest's arches sound outside Storm.
- The rain bed's and the one-shots' cost; the reverb's share of the floor since it changed.
