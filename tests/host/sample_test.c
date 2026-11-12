/* SPDX-License-Identifier: GPL-3.0-only */
/* Crawl's SAMPLE (firmware/src/dsp/samplefx.c: GR!MM's tape-loop sample player, a part's source), through the engine
 * on the host, against the sample decoded here: at its root it plays as recorded, an octave down at half the speed,
 * START and REVERSE place the loop, LENGTH sets its period and its seam has no step, a part with no sample is silent,
 * the slots (FM_NSAMPLE notes at once, beside PHYS's own), the release, and FM parts beside it untouched. A render to
 * listen to: build/host/sample.wav. */
#include <string.h>
#include <stdlib.h>
#include "tu.h"
#include "../../firmware/src/dsp/fumi.c"

void fm_midi_out(uint32_t st, uint32_t d1, uint32_t d2) { (void)st; (void)d1; (void)d2; }

#define SR 44100
#define N (3 * SR / 256 * 256)
#define ROOT (-900.0f)                             /* C4, the sample's root, in cents from A4 */
#define TS ((int)SMP_N - 1)                        /* the sample under test: M-SOFT, smooth and tonal, the last (the host's
                                                    * own, out of the FM-1 image: crawl_samples.py) */
static int32_t blk[2 * 256];
static float a_l[N], b_l[N], e_l[N];
static float *ref;                                 /* the sample under test, decoded */
static int ref_n;

static void render(float *dst, int n)
{
    int i, k;
    for (i = 0; i < n; i += 256) {
        fm_render(blk, 256, 4096);
        if (dst)
            for (k = 0; k < 256; k++)
                dst[i + k] = (float)blk[2 * k] / 8388608.0f;
    }
}
static double rms(const float *x, int from, int to)
{
    double a = 0;
    int i;
    for (i = from; i < to; i++)
        a += (double)x[i] * x[i];
    return sqrt(a / (to - from));
}
static double peak(const float *x, int from, int to)
{
    double m = 0;
    int i;
    for (i = from; i < to; i++)
        if (fabs((double)x[i]) > m)
            m = fabs((double)x[i]);
    return m;
}
static double db(double v) { return 20.0 * log10(v + 1e-12); }
/* the correlation of x[from + lag ..] with y[from ..] over n samples, the best of lags 0 .. 64 (the engine's few
 * samples of delay) */
static double corr(const float *x, const float *y, int from, int n)
{
    double best = -1.0;
    int lag, i;
    for (lag = 0; lag <= 64; lag++) {
        double xy = 0, xx = 0, yy = 0, c;
        for (i = from; i < from + n; i++) {
            xy += (double)x[i + lag] * y[i];
            xx += (double)x[i + lag] * x[i + lag];
            yy += (double)y[i] * y[i];
        }
        c = xy / (sqrt(xx * yy) + 1e-20);
        if (c > best)
            best = c;
    }
    return best;
}
/* what a head reads: the sample from `from` on at `step` samples an output sample (backward: step < 0), n of them */
static void expect(float *dst, double from, double step, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        double p = from + step * i;
        int k = (int)floor(p);
        double f = p - k;
        dst[i] = k >= 0 && k + 1 < ref_n ? (float)(ref[k] * (1.0 - f) + ref[k + 1] * f) : 0.0f;
    }
}
static double step_max(const float *x, int from, int to)   /* the largest step between two samples */
{
    double m = 0;
    int i;
    for (i = from + 1; i < to; i++)
        if (fabs((double)x[i] - x[i - 1]) > m)
            m = fabs((double)x[i] - x[i - 1]);
    return m;
}
static void start(int smp, int st, int len, int rev)
{
    fm_init();
    fm_set(P_REVERB, 0);
    fm_set(P_LOWCUT, fm_param_info(P_LOWCUT)->lo);      /* (the output's own cuts out of the way: a sample with its
                                                     * energy low, as the ice's groan, would lose it to the low cut) */
    fm_set(P_HIGHCUT, fm_param_info(P_HIGHCUT)->hi);
    fm_part_patch(1, FM_ENG_SAMPLE + smp);
    fm_part_knobs(1, st, len, rev, 0);
    fm_part_release(1, 300);
}

int main(void)
{
    int i;
    double fm_level, v, c, c2;

    {                                              /* the sample as the firmware decodes it */
        int32_t pred = 0, idx = 0;
        ref_n = (int)SMP_TAB[TS].n;
        ref = malloc(sizeof *ref * (size_t)ref_n);
        for (i = 0; i < ref_n; i++)
            ref[i] = (float)ima_nib((SMP_TAB[TS].data[i >> 1] >> (4 * (i & 1))) & 15u, &pred, &idx) / 32768.0f;
        printf("  sample %d: %s, %d samples (%.2f s), root %d\n", TS, SMP_TAB[TS].name, ref_n, ref_n / 22050.0, SMP_TAB[TS].root16 / 16);
        for (i = 0; i * 256 < ref_n; i++)          /* the seek index against a decode from the beginning */
            if (i == 0 && (SMP_TAB[TS].ipred[0] != 0 || SMP_TAB[TS].iidx[0] != 0))
                break;
        CHECK("the seek index starts at predictor 0, step index 0", i * 256 >= ref_n);
    }

    /* for scale: an FM pad on the same part */
    fm_init();
    fm_set(P_REVERB, 0);
    fm_set(P_LOWCUT, fm_param_info(P_LOWCUT)->lo);      /* (the output's own cuts out of the way: a sample with its
                                                     * energy low, as the ice's groan, would lose it to the low cut) */
    fm_set(P_HIGHCUT, fm_param_info(P_HIGHCUT)->hi);
    fm_part_patch(1, ZN_SOFT_PAD);
    fm_voice(FM_VOICE_ID, 1, -1200.0f, 100, 64, 0);
    render(a_l, N);
    fm_level = rms(a_l, SR, 2 * SR);

    /* at the root: as recorded (22050 Hz: half a sample an output sample) */
    start(TS, 0, 127, 0);
    fm_voice(FM_VOICE_ID, 1, ROOT, 100, 64, 0);
    render(a_l, N);
    expect(e_l, 0.0, 0.5, SR);
    c = corr(a_l, e_l, 2048, SR - 4096);
    printf("  at the root: %.1f dB (the FM pad %.1f dB), peak %.3f, correlation with the sample %.3f\n", db(rms(a_l, 0, SR)),
           db(fm_level), peak(a_l, 0, N), c);
    CHECK("at its root the sample plays as recorded", c > 0.95);
    CHECK("... within 12 dB of the FM pad, far from the output's ceiling",
          rms(a_l, 0, SR) > fm_level * 0.25 && rms(a_l, 0, SR) < fm_level * 4.0 && peak(a_l, 0, N) < 0.5);
    CHECK("... the loop goes on: still sounding after its length (1.5 s)", rms(a_l, 2 * SR, N) > rms(a_l, 0, SR) * 0.3);

    /* an octave down: half the speed; an octave up: twice */
    start(TS, 0, 127, 0);
    fm_voice(FM_VOICE_ID, 1, ROOT - 1200.0f, 100, 64, 0);
    render(b_l, N);
    expect(e_l, 0.0, 0.25, SR);
    c = corr(b_l, e_l, 2048, SR - 4096);
    expect(e_l, 0.0, 0.5, SR);
    c2 = corr(b_l, e_l, 2048, SR - 4096);
    printf("  an octave down: correlation with the sample at half speed %.3f, at full speed %.3f\n", c, c2);
    CHECK("an octave under the root: at half the speed", c > 0.95 && c2 < 0.5);
    start(TS, 0, 127, 0);
    fm_voice(FM_VOICE_ID, 1, ROOT + 1200.0f, 100, 64, 0);
    render(b_l, N);
    expect(e_l, 0.0, 1.0, SR / 2);
    c = corr(b_l, e_l, 2048, SR / 2 - 4096);
    CHECK("an octave over the root: at twice the speed", c > 0.95);

    /* START: the loop begins there; REVERSE: from the loop's end backward */
    start(TS, 64, 127, 0);
    fm_voice(FM_VOICE_ID, 1, ROOT, 100, 64, 0);
    render(b_l, N);
    expect(e_l, (double)(ref_n * 64 / 128), 0.5, SR / 2);
    c = corr(b_l, e_l, 2048, SR / 2 - 4096);
    expect(e_l, 0.0, 0.5, SR / 2);
    c2 = corr(b_l, e_l, 2048, SR / 2 - 4096);
    printf("  START 64: correlation with the sample from its middle %.3f, from its beginning %.3f\n", c, c2);
    CHECK("START 64: the loop begins at the sample's middle", c > 0.95 && c2 < 0.5);
    start(TS, 0, 127, 1);
    fm_voice(FM_VOICE_ID, 1, ROOT, 100, 64, 0);
    render(b_l, N);
    expect(e_l, (double)(ref_n - 1), -0.5, SR);
    c = corr(b_l, e_l, 2048, SR - 4096);
    expect(e_l, 0.0, 0.5, SR);
    c2 = corr(b_l, e_l, 2048, SR - 4096);
    printf("  REVERSE: correlation with the sample backward %.3f, forward %.3f\n", c, c2);
    CHECK("REVERSE: the loop backward from its end", c > 0.95 && c2 < 0.5);

    /* LENGTH: a short loop repeats at its length less its crossing, and its seam has no step */
    {
        uint32_t len = 1102u + ((uint32_t)ref_n - 1102u) * (40u * 40u) / 16129u, xf = len / 4u < 2205u ? len / 4u : 2205u;
        int per = (int)((len - xf) * 2u);
        start(TS, 0, 40, 0);
        fm_voice(FM_VOICE_ID, 1, ROOT, 100, 64, 0);
        render(a_l, N);
        c = corr(a_l + per, a_l, SR, SR / 2);      /* (a period on: the same again) */
        c2 = corr(a_l + per * 3 / 4, a_l, SR, SR / 2);
        printf("  LENGTH 40: a loop of %u samples, its period %d output samples: correlation a period on %.3f, three quarters on %.3f\n",
               len, per, c, c2);
        CHECK("LENGTH 40: the loop repeats at its length less its crossing", c > 0.9 && c2 < 0.6);
        expect(e_l, 0.0, 0.5, (int)len * 2);
        printf("  the largest step: %.4f over two seconds of loops, %.4f inside the sample itself (at the output's level)\n",
               step_max(a_l, SR / 2, 5 * SR / 2), step_max(e_l, 0, (int)len * 2) * peak(a_l, SR, 2 * SR) / (peak(e_l, 0, (int)len * 2) + 1e-12));
        CHECK("... its seam has no step larger than the sample's own (x 1.5)",
              step_max(a_l, SR / 2, 5 * SR / 2) < 1.5 * step_max(e_l, 0, (int)len * 2) * peak(a_l, SR, 2 * SR) / (peak(e_l, 0, (int)len * 2) + 1e-12));
    }

    /* LENGTH turned while it plays: the next pass takes it, nothing steps */
    start(TS, 0, 40, 0);
    fm_voice(FM_VOICE_ID, 1, ROOT, 100, 64, 0);
    render(a_l, SR / 256 * 256);
    v = step_max(a_l, SR / 4, SR / 256 * 256);
    fm_part_knobs(1, 90, 20, 1, 0);
    render(b_l, N);
    printf("  the knobs turned in a loop: the largest step %.4f before, %.4f after\n", v, step_max(b_l, 0, N));
    CHECK("START, LENGTH and REVERSE turned while it plays: it goes on, no step (x 2 the loop's own)",
          rms(b_l, SR, 2 * SR) > 1e-4 && step_max(b_l, 0, N) < 2.0 * v);

    /* no such sample: silent */
    start((int)SMP_N, 0, 127, 0);
    fm_voice(FM_VOICE_ID, 1, ROOT, 100, 64, 0);
    render(a_l, N);
    CHECK("a part with no sample (one past the last): silent", rms(a_l, 0, N) < 1e-9);

    /* the release: let go, it dies away and the voice is free */
    start(TS, 0, 127, 0);
    fm_voice(FM_VOICE_ID, 1, ROOT, 100, 64, 0);
    render(NULL, SR / 256 * 256);
    CHECK("held: one voice", fm_nvoices == 1);
    fm_voice_off(FM_VOICE_ID);
    render(a_l, N);
    CHECK("let go (release 300 ms): silent after it, the voice free", fm_nvoices == 0 && rms(a_l, SR, 2 * SR) < 1e-6);
    CHECK("... not cut off: still there 100 ms on", rms(a_l, 4096, 4096 + 2048) > 1e-5);

    /* the slots: FM_NSAMPLE notes at once, more go over the oldest; PHYS's slots are its own; an FM note beside */
    start(TS, 0, 127, 0);
    fm_part_patch(2, FM_ENG_PHYS + 0);
    fm_part_knobs(2, 35, 127, 0, 80);
    fm_part_patch(3, ZN_SOFT_PAD);
    for (i = 0; i < FM_NSAMPLE + 2; i++) {
        fm_voice(FM_VOICE_ID + i, 1, ROOT - 1200.0f + 300.0f * (float)i, 90, 20 + 16 * i, 0);
        render(NULL, 2048);
    }
    CHECK("six SAMPLE notes asked for: FM_NSAMPLE sound, the last ones asked for", fm_nvoices == FM_NSAMPLE &&
          fm_voice_held(FM_VOICE_ID + FM_NSAMPLE + 1) && !fm_voice_held(FM_VOICE_ID) && !fm_voice_held(FM_VOICE_ID + 1));
    for (i = 0; i < FM_NPHYS; i++)
        fm_voice(FM_VOICE_ID + 20 + i, 2, -1200.0f + 300.0f * (float)i, 90, 64, 0);
    fm_voice(FM_VOICE_ID + 30, 3, -2400.0f, 100, 64, 0);
    render(a_l, N);
    printf("  %d SAMPLE notes, %d PHYS notes and an FM pad: %d voices, peak %.3f\n", FM_NSAMPLE, FM_NPHYS, fm_nvoices, peak(a_l, 0, N));
    CHECK("FM_NPHYS PHYS notes beside them, and an FM note: all sound", fm_nvoices == FM_NSAMPLE + FM_NPHYS + 1);
    CHECK("... in range (the output's soft clip not reached)", peak(a_l, 0, N) < 0.9);
    fm_voice_all_off();
    render(NULL, 4 * N);
    CHECK("all off: all free", fm_nvoices == 0);

    /* a part back to an FM patch plays FM again */
    fm_part_patch(1, ZN_SOFT_PAD);
    fm_voice(FM_VOICE_ID, 1, -1200.0f, 100, 64, 0);
    render(a_l, N);
    v = rms(a_l, SR, 2 * SR);
    CHECK("the part given an FM patch again: the FM pad, as before", fabs(20.0 * log10(v / fm_level)) < 1.0);

    /* to listen to (build/host/sample.wav, 16-bit stereo, 36 s), with the reverb: 0 s at the root; 6 s an octave
     * down; 12 s two octaves down, backward; 18 s a short loop, its START moving every second; 26 s three notes */
    {
        FILE *f = fopen("build/host/sample.wav", "wb");
        if (f) {
            uint32_t periods = 36u * 172u, bytes = periods * 256u * 4u, v32, t;
            uint16_t v16;
            int k;
            fwrite("RIFF", 1, 4, f), v32 = 36u + bytes, fwrite(&v32, 4, 1, f), fwrite("WAVEfmt ", 1, 8, f);
            v32 = 16, fwrite(&v32, 4, 1, f), v16 = 1, fwrite(&v16, 2, 1, f), v16 = 2, fwrite(&v16, 2, 1, f);
            v32 = 44100, fwrite(&v32, 4, 1, f), v32 = 176400, fwrite(&v32, 4, 1, f);
            v16 = 4, fwrite(&v16, 2, 1, f), v16 = 16, fwrite(&v16, 2, 1, f);
            fwrite("data", 1, 4, f), fwrite(&bytes, 4, 1, f);
            fm_init();
            fm_set(P_REVERB, 50);
            fm_set(P_REV_SIZE, 90);
            fm_part_patch(1, FM_ENG_SAMPLE + TS);
            fm_part_attack(1, 300);
            fm_part_release(1, 1500);
            for (t = 0; t < periods; t++) {
                uint32_t s = t / 172u, at = t % 172u;
                if (at == 0 && s == 0)
                    fm_part_knobs(1, 0, 127, 0, 0), fm_voice(FM_VOICE_ID, 1, ROOT, 100, 64, 5000);
                if (at == 0 && s == 6)
                    fm_voice(FM_VOICE_ID, 1, ROOT - 1200.0f, 100, 64, 5000);
                if (at == 0 && s == 12)
                    fm_part_knobs(1, 0, 127, 1, 0), fm_voice(FM_VOICE_ID, 1, ROOT - 2400.0f, 100, 64, 5000);
                if (at == 0 && s == 18)
                    fm_part_knobs(1, 0, 30, 0, 0), fm_voice(FM_VOICE_ID, 1, ROOT - 700.0f, 100, 64, 7000);
                if (at == 0 && s > 18u && s < 26u)
                    fm_part_knobs(1, (int)(s - 18u) * 14, 30, 0, 0);
                if (at == 0 && s == 26) {
                    fm_part_knobs(1, 20, 100, 0, 0);
                    for (k = 0; k < 3; k++)
                        fm_voice(FM_VOICE_ID + 1 + k, 1, ROOT - 1900.0f + 700.0f * (float)k, 90, 24 + 40 * k, 7000);
                }
                fm_render(blk, 256, 4096);
                for (k = 0; k < 512; k++) {
                    int16_t smp = (int16_t)(blk[k] >> 8);
                    fwrite(&smp, 2, 1, f);
                }
            }
            fclose(f);
            printf("  render -> build/host/sample.wav\n");
        }
    }
    free(ref);
    return tu_done("sample");
}
