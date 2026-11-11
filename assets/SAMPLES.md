# Crawl's samples

Every sample in the image, with its source, author and license. CC0 only (`CLAUDE.md`, constraint 5).

The SAMPLE source (`firmware/src/dsp/samplefx.c`) plays the samples of `firmware/src/dsp/crawl_samples.h`, which
`tools/crawl_samples.py` makes from the files in `assets/samples/`. ZEN-1's nature recordings are listed apart, in
`assets/sounds/README.md`.

| No. | Name | File | Source | Author | License | Changed |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | M-SOFT | `assets/samples/m-soft.wav` | GR!MM v2.0.2 factory sample `assets/samples/M-Soft.wav`, https://github.com/GRiMMBIENT/grimm/blob/v2.0.2/assets/samples/M-Soft.wav (commit `b519acb`) | GR!MM, Copyright (C) 2026 GR!MM | CC0 1.0 Universal (`LICENSES/CC0-1.0.txt`; stated in GR!MM's `LICENSING.md`, "The factory samples") | Its first 1.5 s, mixed to mono, 16 bit, at its own 48 kHz. The tool then resamples to 22050 Hz, normalizes and codes it as IMA ADPCM (16537 B) |

M-SOFT is here to prove the SAMPLE source and its tool in M2. The samples Crawl ships with are chosen in M3, with the
scenes.

## The scenes' recorded textures

The nature layer's recordings (loops for the beds, one-shot calls) go through ZEN-1's pipeline, not the SAMPLE
source: `tools/zen_samples.py` into `firmware/src/dsp/zen_samples.h`, played by `firmware/src/dsp/bed.c`. Each is
logged in `assets/sounds/README.md` with its source, author and license (CC0 only). Crawl's own, so far:

| Scene | Texture | File | Source | Author | License |
| --- | --- | --- | --- | --- | --- |
| Forest Ruins | Air: Leaves (dry leaves in the wind, a loop) | `assets/sounds/leaves-457318-preview.mp3` | https://freesound.org/s/457318/ | Stek59 | CC0 1.0 |
| Forest Ruins | the crows (four calls) | `assets/sounds/crow-741366-preview.mp3` | https://freesound.org/s/741366/ | Mish7913 | CC0 1.0 |
| Forest Ruins | a second crow, a rook (three calls) | `assets/sounds/rook-556221-preview.mp3` | https://freesound.org/s/556221/ | Walking.With.Microphones | CC0 1.0 |
| Forest Ruins | the flock taking off (one burst) | `assets/sounds/rooks-553111-preview.mp3` | https://freesound.org/s/553111/ | HighPeakEmma | CC0 1.0 |

## Adding a sample

1. Check the recording's own page: CC0 only. Put the file in `assets/samples/` (WAV, any rate, mono or stereo).
2. Add a row above, and a line to `SAMPLES` in `tools/crawl_samples.py` (file, name, root note).
3. Run `python tools/crawl_samples.py` and build. A second of sound is about 11 KB of flash.
