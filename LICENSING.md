# Crawl licensing

Crawl is free software under the GNU General Public License, version 3 only (`GPL-3.0-only`, full text
in `LICENSE`). It is built on [ZEN-1](https://github.com/aceyprime/zen-1), which is built on FuMi-1, FoMni and
Felucca's platform layer (through X0X), and keeps their license. If you distribute Crawl, or firmware derived from
it, you must give your recipients its complete corresponding source under the same license.

## Where the code comes from

| What | Origin | License |
| --- | --- | --- |
| Platform: `firmware/hal/`, `firmware/loader/`, `firmware/src/{libc,lcd,gfx,usb,storage,ota,midi_uart}.c`, `tools/` (build, package, install, rescue, fonts), `web/fm1*.js`, `tests/{ota,ldr,storage}_test.c`, `tests/install_test.py` | [Felucca](https://github.com/hugelton/Felucca), Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments, as changed for [X0X](https://github.com/charlesvestal/fm1-x0x) and [FoMni](https://github.com/charlesvestal/fm1-omnichord) | GPL-3.0-only |
| `firmware/src/app/{panel,plat_fm1,main_fm1}.c`, `plat.h`, `host/zen_host.c`, `web/emu/` (the worklet, the page, the device clock driven by its audio), `tests/host/{fastmath,encoder,uac,trs}_test.c` | FoMni (from X0X, from Felucca's `panel.c`, `main.c`, `audio.c`), adapted | GPL-3.0-only |
| `firmware/src/dsp/reverb.c`: Dattorro's plate | FoMni's `omni.c` (Charles Vestal) | GPL-3.0-only |
| `firmware/src/dsp/fm6_core.c`: the 6-operator FM synthesis, msfa (Dexed) ported to integer C | Felucca's port of msfa, Copyright 2012 Google Inc., 2016–2025 Pascal Gauthier; only msfa is used (Dexed itself is GPL-3.0) | Apache-2.0 (`LICENSES/Apache-2.0-msfa.txt`) |
| `firmware/src/dsp/fm6_patch.c` (patch formats), `tools/gen_fm6_tables.py` and the generated `fm6_tables.h`, the helpers in `tools/fumi_patches.py` | Felucca's `eng_fm6.c`, `tools/gen_tables.py`, `tools/gen_fm6_patches.py` | GPL-3.0-only |
| `firmware/src/dsp/fastmath.h` | Felucca, via FoMni | GPL-3.0-only |
| The pitch core, the instrument (`firmware/src/dsp/{tuning,smooth,bend,fumi}.*`, `fumi_patches.h`), `web/bench/`, `tools/{analyze_ref,syx_dump,fumi_patches,check_no_double}.py`, `tools/koto_search.c`, `tools/guard_platform.sh` | [FuMi-1](https://github.com/cartesive/fumi-1) | GPL-3.0-only |
| Everything else: the scenes, the conductor, the renderer and UI (`firmware/src/app/`), the nature beds (`firmware/src/dsp/bed.c`, `zen_*.h`), `tools/zen_*`, the site and guide (`web/site/`), `tests/`, `docs/` | ZEN-1 (AceyPrime) | GPL-3.0-only |
| `firmware/src/dsp/tapefx.c`: the TAPE effect (wow, saturation, tone, hiss), with Felucca's `softclip` and its tanh table | [GR!MM](https://github.com/GRiMMBIENT/grimm) v2.0.2 (`firmware/src/tapefx.c`, `wear.c`, `dsp.c`), Copyright (C) 2026 GR!MM; Felucca's parts Copyright (C) 2026 Leo Kuroshita; adapted to Crawl's mix | GPL-3.0-only |
| `firmware/src/dsp/echofx.c`: DELAY (a tape echo) and GRAIN (a granular cloud), with Felucca's `soft_knee` | [GR!MM](https://github.com/GRiMMBIENT/grimm) v2.0.2 (`firmware/src/tape.c`, `grain.c`, `wear.c`, `dsp.c`), Copyright (C) 2026 GR!MM; GRAIN from Felucca's `eng_grain.c`, Copyright (C) 2026 Leo Kuroshita; DELAY's design after [X0X](https://github.com/charlesvestal/fm1-x0x)'s TAPE delay; adapted to share one line | GPL-3.0-only |
| `firmware/src/dsp/voicefx.c`: VOICE, a formant voice and three choirs | [GR!MM](https://github.com/GRiMMBIENT/grimm) v2.0.2 (`firmware/src/formant.c`), from Felucca's `eng_formant.c`, Copyright (C) 2026 Leo Kuroshita, GR!MM changes Copyright (C) 2026 GR!MM; after Klatt (1980), formant data from Klatt and Hillenbrand et al. (1995); the choirs are GR!MM's own | GPL-3.0-only |
| `firmware/src/dsp/physfx.c`: PHYS, four physical models (MODAL, STRING, MEMBRANE, SYMPATH) | [GR!MM](https://github.com/GRiMMBIENT/grimm) v2.0.2 (`firmware/src/phys.c`), from Felucca's `eng_phys.c`, Copyright (C) 2026 Leo Kuroshita, GR!MM changes Copyright (C) 2026 GR!MM; adapted to Crawl's parts and voices. Its DSP: `phys_dsp.c`, `phys_symp.c` (below) | GPL-3.0-only |
| `firmware/src/dsp/phys_dsp.c`: PHYS's modal and string models and the resonator | [DaisySP](https://github.com/electro-smith/DaisySP) by Electrosmith, Corp and Emilie Gillet, ported to fixed point by Leo Kuroshita for Felucca; as GR!MM v2.0.2 has it (`firmware/src/phys_dsp.c`), unchanged | MIT (`LICENSES/MIT-DaisySP.txt`) |
| `firmware/src/dsp/phys_symp.c`: PHYS's sympathetic strings | Rings by Emilie Gillet (<https://github.com/pichenettes/eurorack>), ported to fixed point by Leo Kuroshita for Felucca; as GR!MM v2.0.2 has it (`firmware/src/phys_symp.c`), unchanged | MIT (`LICENSES/MIT-Rings.txt`) |
| `firmware/src/dsp/samplefx.c`: SAMPLE, a sample played as a tape loop | [GR!MM](https://github.com/GRiMMBIENT/grimm) v2.0.2 (`firmware/src/sampler.c`, and the decoder of `samples.c`), Copyright (C) 2026 GR!MM; the IMA ADPCM decoder is Felucca's (`eng_grain.c`), Copyright (C) 2026 Leo Kuroshita; adapted to samples kept in the image | GPL-3.0-only |
| `tools/sampleio.py`: WAV reading, resampling, IMA ADPCM coding | Felucca's, Copyright (C) 2026 Leo Kuroshita, as GR!MM v2.0.2 has it, unchanged | GPL-3.0-only |
| `tools/crawl_samples.py` and its output's layout (`firmware/src/dsp/crawl_samples.h`) | Crawl | GPL-3.0-only |
| Crawl's own changes: the name, `Dockerfile`, the PowerShell wrappers, `docs/DECISIONS.md`, `docs/PROGRESS.md` | Crawl | GPL-3.0-only |

## Third-party material

| What | License | Where |
| --- | --- | --- |
| Barlow Semi Condensed (The Barlow Project Authors), the UI face | SIL OFL 1.1 | `assets/fonts/BarlowSemiCondensed-*.ttf`, `assets/fonts/Barlow-OFL.txt` |
| Terminus (Dimitar Toshkov Zhekov), an alternative font set | SIL OFL 1.1 | `assets/fonts/ter-u*.bdf`, `assets/fonts/Terminus-LICENSE.txt` |
| Noto Sans CJK, the seventeen kanji of the instruments' names, as bitmaps (`tools/gen_kanji.py`) | SIL OFL 1.1 | `firmware/src/app/zen_kanji.h` |
| The choirs' wavetables (`firmware/src/dsp/grimm_choir.h`, made by GR!MM's `tools/gen_choir.py`): single cycles measured from the long tones of six singers of [VocalSet: A Singing Voice Dataset](https://doi.org/10.5281/zenodo.1442513) by Julia Wilkins, Prem Seetharaman, Alison Wahl and Bryan Pardo (Northwestern University, 2018), changed as that script says | CC BY 4.0 (`LICENSES/CC-BY-4.0.txt`) | `firmware/src/dsp/grimm_choir.h`; in the firmware and the browser build |
| The SAMPLE source's samples (`assets/samples/`, coded into `firmware/src/dsp/crawl_samples.h`): M-SOFT, the first 1.5 s of GR!MM's factory sample `M-Soft.wav`, Copyright (C) 2026 GR!MM. Each sample's source, author and license: `assets/SAMPLES.md` | CC0 1.0 (`LICENSES/CC0-1.0.txt`) | `assets/samples/`, `firmware/src/dsp/crawl_samples.h`; in the firmware and the browser build |
| Nature recordings from Freesound (BurghRecords, Anya_Media, Defelozedd94, shaunhillyard, warukunai, afleetingspeck, Breviceps, thegoosington) | CC0 | `assets/sounds/` (sources in its README), coded into `firmware/src/dsp/zen_samples.h` |
| The Moon's near side, NASA Scientific Visualization Studio CGI Moon Kit (LRO) | Public domain | `assets/images/`, coded into `firmware/src/app/zen_moon.h` |
| JieLi AC79 SDK: `uboot.boot`, `cfg_tool.bin`, `eq_cfg_hw.bin` are read from your SDK checkout at build time and placed in the package; no SDK files are in this tree | Apache-2.0 | <https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK> |

## What is deliberately not here

- No Yamaha DX7 factory patch data. FuMi's own patches (`tools/fumi_patches.py`) and ZEN-1's (`tools/zen_patches.py`)
  are written as operator settings; the community patches among the instruments name their bank and author. The
  audition bench reads any `.syx` you have, from `refs/` (git-ignored), for listening only.
- No Suiko ST-50 audio. The recordings used to measure the tuning stay in `refs/`.
- No M-VAVE firmware files.

## Trademarks

Suiko, 水光トレーナー and ST-50 are names of 水光社 (Suikohsha), used here only to say what inspired FuMi-1, whose instrument ZEN-1 keeps.
"Felucca" and "Hügelton Instruments" are names of Hügelton Instruments. "M-VAVE" and "FM-1" are trademarks
of their respective owners. DX7 is a trademark of Yamaha. Crawl is independent, unofficial firmware; it is not
affiliated with, endorsed by or supported by any of them, nor by ZEN-1's author.

## Radio

Crawl never enables the Bluetooth / Wi-Fi radio of the hardware.
