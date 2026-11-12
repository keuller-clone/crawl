# Crawl's samples

Every sample in the image, with its source, author and license. CC0 only (`CLAUDE.md`, constraint 5).

The SAMPLE source (`firmware/src/dsp/samplefx.c`) plays the samples of `firmware/src/dsp/crawl_samples.h`, which
`tools/crawl_samples.py` makes from the files in `assets/samples/`. ZEN-1's nature recordings are listed apart, in
`assets/sounds/README.md`.

| No. | Name | File | Source | Author | License | Changed |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | GROAN | `assets/samples/ice-groan.wav` | "rumbling_ice", https://freesound.org/s/847807/ (sounds from ice on a lake) | aerror | CC0 1.0 Universal (`LICENSES/CC0-1.0.txt`; the Freesound page, checked 2026-11-11) | 1.6 s from 391.0 s of the recording (a groan, its fundamental about 60 Hz), mono, 22050 Hz, 16 bit, high-passed at 30 Hz and low-passed at 1.2 kHz (the recording's own hiss), a 10 ms fade in and a 150 ms fade out: `ffmpeg -ss 391.0 -t 1.6 -i 847807_2472895-hq.mp3 -ac 1 -ar 22050 -sample_fmt s16 -af highpass=f=30,lowpass=f=1200,afade=t=in:d=0.01,afade=t=out:st=1.45:d=0.15 ice-groan.wav`. The tool normalizes and codes it as IMA ADPCM (17640 B + 414 B of seek index) |

| 1 | M-SOFT (the host only) | `assets/samples/m-soft.wav` | GR!MM v2.0.2 factory sample `assets/samples/M-Soft.wav`, https://github.com/GRiMMBIENT/grimm/blob/v2.0.2/assets/samples/M-Soft.wav (commit `b519acb`) | GR!MM, Copyright (C) 2026 GR!MM | CC0 1.0 Universal (`LICENSES/CC0-1.0.txt`; stated in GR!MM's `LICENSING.md`, "The factory samples") | Its first 1.5 s, mixed to mono, 16 bit, at its own 48 kHz. The tool then resamples to 22050 Hz, normalizes and codes it as IMA ADPCM (16537 B). In the simulator, the browser build and the tests (`FM_HOST`), not in the FM-1 image |

GROAN is Frozen Lake's ice (M3, 2026-11-11): the scene plays it as a tape loop pitched to the chord's root, the first
use of the SAMPLE source by a scene. M-SOFT proved the source and its tool in M2 and is still what `sample_test`
measures the source with (smooth and tonal, as its checks assume); it left the FM-1 image with Frozen Lake.

## The scenes' recorded textures

The nature layer's recordings (loops for the beds, one-shot calls) go through ZEN-1's pipeline, not the SAMPLE
source: `tools/zen_samples.py` into `firmware/src/dsp/zen_samples.h`, played by `firmware/src/dsp/bed.c`. Each is
logged in `assets/sounds/README.md` with its source, author and license (CC0 only). Crawl's own, so far:

| Scene | Texture | File | Source | Author | License |
| --- | --- | --- | --- | --- | --- |
| Forest Ruins | the crows (four calls) | `assets/sounds/crow-741366-preview.mp3` | https://freesound.org/s/741366/ | Mish7913 | CC0 1.0 |
| Forest Ruins | a second crow, a rook (three calls) | `assets/sounds/rook-556221-preview.mp3` | https://freesound.org/s/556221/ | Walking.With.Microphones | CC0 1.0 |
| Forest Ruins | the flock taking off (one burst) | `assets/sounds/rooks-553111-preview.mp3` | https://freesound.org/s/553111/ | HighPeakEmma | CC0 1.0 |
| Frozen Lake | the ice cracking (four cracks) | `assets/sounds/ice-737896-preview.mp3` | https://freesound.org/s/737896/ | Vrymaa | CC0 1.0 |
| Frozen Lake | the keep's bell (one stroke, far) | `assets/sounds/keepbell-240586-preview.mp3` | https://freesound.org/s/240586/ | blaukreuz | CC0 1.0 |

## Adding a sample

1. Check the recording's own page: CC0 only. Put the file in `assets/samples/` (WAV, any rate, mono or stereo).
2. Add a row above, and a line to `SAMPLES` in `tools/crawl_samples.py` (file, name, root note).
3. Run `python tools/crawl_samples.py` and build. A second of sound is about 11 KB of flash.
