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

## Memory: what the GR!MM parts need (read from `refs/grimm/v2.0.2`, not built)

| Part | Pool | RAM | Flash data | From |
| --- | --- | --- | --- | --- |
| DELAY (tape echo, 3.5 s at 22.05 kHz) | 154912 B | small | small | `tape.c`: `dl_buf[77456]` int16 |
| GRAIN (1.25 s ring, 8 grains) | 55126 B | small | 514 B window | `grain.c`: `ge_buf[27563]` int16 |
| SAMPLE (per voice: 4 windows of 257) | 2056 B a voice | small | none in the image | `sampler.c`: `sp_w[NV][4][257]` int16 |
| SAMPLE slots (4 x 72 KiB ADPCM, 6.6 s each) | 6864 B seek index | small | 288 KiB **outside** the app slot, at 0x97000..0xDF000 | `samples.c`, `platform.c` |
| PHYS (per voice: the largest model's state) | none | about 2.7 KB a voice | small | `phys_dsp.c`: string line 512 + 128 int32; `phys_symp.c`: 3 x 256 int16 |
| VOICE (formant choir) | none | 28 B a voice | 28800 B choir table | `formant.c`; `grimm_choir.h` `CHOIR_T[2][3][2][5][480]` |
| TAPE (wow, saturation, hiss) | none | 3076 B curve | small | `tapefx.c`: `tf_curve[2][769]` int16 |

Code size for each part is unknown: it needs a build with the JieLi toolchain (see Open).

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

## CPU: the GR!MM parts (not measured)

No CPU figure for VOICE, PHYS, SAMPLE, TAPE, GRAIN or DELAY exists yet. What the source says:

- GR!MM runs 6 voices in all (`voices.c`, `NV 6`), against ZEN-1's 12.
- `formant.c` caps the choir at six singers and says that is the most the FM-1's time has for six voices.
- DELAY, GRAIN and GR!MM's reverb run at half rate (22.05 kHz) to halve their work.
- GR!MM ships its own on-device profiler (`tools/fm1_cpu.py`, a per-stage split read over USB), which is the
  design to copy for Crawl's hardware measurements.

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
3. Polyphony: plan for 8 voices in all, not 12, shared across FM, VOICE, PHYS and SAMPLE, with at most 4 of
   the heavy kinds (VOICE, PHYS) at once, until hardware figures say otherwise. The estimate above leaves about
   40 % of a block for three new effects and the heavier voices if FM stays near 8 voices, and GR!MM's rules ask
   for half the block kept free.
4. SAMPLE: decide in M2 whether samples live in the image (as ZEN-1's recordings do) or in a flash area outside
   it (as GR!MM's do), since the two layouts collide.

## Open: what still has to be measured

- **GR!MM CPU and code size.** Needs the GR!MM modules compiled, on the host for timing and with the JieLi
  toolchain for size. The brief marks the GR!MM checkout as not built, and an attempt to build it in the container
  was blocked by the session's permissions. Smallest next step: Steven allows building `refs/grimm/v2.0.2` in a
  scratch folder for measurement only.
- **Hardware figures.** Every FM-1 percentage here is an estimate from one upstream reading. Smallest next
  step: with Steven's go-ahead, flash the M0 build and read the HUD's load for the bench's cases, then add a
  per-stage split like GR!MM's.
- **The reverb's true cost.** The bench shows it always running; its share of the idle floor is not separated.
