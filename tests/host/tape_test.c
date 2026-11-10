/* SPDX-License-Identifier: GPL-3.0-only */
/* Crawl's TAPE (firmware/src/dsp/tapefx.c: GR!MM's TAPE effect on the mix), through the whole engine on the host:
 * off is off to the bit, SATURATION, TONE, HISS, the curve's crossover without a step, WOW's warbles, letting go. */
#include <string.h>
#include <stdlib.h>
#include "tu.h"
#include "../../firmware/src/dsp/fumi.c"

void fm_midi_out(uint32_t st, uint32_t d1, uint32_t d2) { (void)st; (void)d1; (void)d2; }

#define N (2 * 44100 / 256 * 256)                  /* about 2 s, whole blocks */
static int32_t blk[2 * 256];
static float a_l[N], b_l[N];

/* a fresh engine, a held pad on a scene part (so the WOW reaches it), reverb off */
static void start(void)
{
    fm_init();
    fm_set(P_REVERB, 0);
    fm_part_patch(1, ZN_SOFT_PAD);
    fm_voice(FM_VOICE_ID, 1, 0.0f, 110, 64, 0);
    fm_voice(FM_VOICE_ID + 1, 1, 700.0f, 110, 64, 0);
}
static void render(float *dst, int n)              /* n frames (whole 256s): the left side, +-1 */
{
    int i, k;
    for (i = 0; i < n; i += 256) {
        fm_tape_poll();                            /* (the main loop's part) */
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
static double hf(const float *x, int from, int to)   /* the first difference's RMS: the highs */
{
    double a = 0;
    int i;
    for (i = from + 1; i < to; i++)
        a += (double)(x[i] - x[i - 1]) * (x[i] - x[i - 1]);
    return sqrt(a / (to - from - 1));
}
static double jerk(const float *x, int from, int to)   /* the biggest second difference: a step shows here */
{
    double m = 0;
    int i;
    for (i = from + 2; i < to; i++) {
        double d = fabs((double)x[i] - 2.0 * x[i - 1] + x[i - 2]);
        if (d > m)
            m = d;
    }
    return m;
}
static double db(double x) { return 20.0 * log10(x > 1e-12 ? x : 1e-12); }

/* the tape alone: x (n frames, whole 32s, both sides the same) through it at amount amt, in place */
static void tape_run(float *x, int n, float amt)
{
    int i, k;
    for (i = 0; i < n; i += 32) {
        fm_tape_poll();
        tape_control(amt, amt, 32, (uint32_t)i);
        for (k = 0; k < 32; k++) {
            float l = x[i + k], r = x[i + k];
            if (tf.live)
                tape_frame(&l, &r);
            x[i + k] = l;
        }
    }
}
static void sine(float *x, int n, float hz, float amp)
{
    int i;
    for (i = 0; i < n; i++)
        x[i] = amp * (float)sin(6.283185307179586 * hz * i / 44100.0);
}

int main(void)
{
    double clean, v, lo, hi;
    int i, same;

    /* through the engine: the four knobs up, the amount 0: nothing runs, no bend */
    start();
    fm_tape_knobs(127, 127, 127, 127);
    render(a_l, N);
    CHECK("the pad sounds", rms(a_l, N / 2, N) > 0.001);
    CHECK("knobs up, amount 0: the tape does not run, no bend", tape_wow == 0.0f && !tf.live);

    /* the tape alone, a 220 Hz sine at half scale. The four knobs 0: a clean tape, the sound to the bit */
    fm_init();
    sine(a_l, N, 220.0f, 0.5f);
    memcpy(b_l, a_l, sizeof a_l);
    tape_run(b_l, N, 1.0f);
    CHECK("amount 100 %, the four knobs 0: a clean tape, to the bit as it was", !memcmp(a_l, b_l, sizeof a_l));
    clean = rms(a_l, N / 2, N);

    /* SATURATION: the tanh driven 4x and made up by 1/4: a quiet sound as loud as it was, a loud one held under a
     * quarter of full scale (GR!MM's curve: the make-up is for small signals) */
    fm_init();
    fm_tape_knobs(0, 127, 0, 0);
    memcpy(b_l, a_l, sizeof a_l);
    tape_run(b_l, N, 1.0f);
    v = rms(b_l, N / 2, N);
    hi = 0;
    for (i = N / 2; i < N; i++)
        if (fabs((double)b_l[i]) > hi)
            hi = fabs((double)b_l[i]);
    printf("  saturation 127, a sine at 0.5: rms %.1f dB (clean %.1f dB), peak %.3f\n", db(v), db(clean), hi);
    CHECK("SATURATION: another sound", memcmp(a_l, b_l, sizeof a_l) != 0);
    CHECK("... a loud sine rounded, held under 0.26", hi < 0.26 && hi > 0.2);
    fm_init();
    fm_tape_knobs(0, 127, 0, 0);
    sine(b_l, N, 220.0f, 0.02f);
    lo = rms(b_l, N / 2, N);
    tape_run(b_l, N, 1.0f);
    printf("  ... a sine at 0.02: %.2f dB from clean\n", db(rms(b_l, N / 2, N)) - db(lo));
    CHECK("... a quiet sine within 1 dB of what it was", fabs(db(rms(b_l, N / 2, N)) - db(lo)) < 1.0);

    /* half the amount: between the two (a crossfade, never the two added) */
    fm_init();
    fm_tape_knobs(0, 127, 0, 0);
    memcpy(b_l, a_l, sizeof a_l);
    tape_run(b_l, N, 0.5f);
    lo = rms(b_l, N / 2, N);
    CHECK("amount 50 %: between the clean sound and the taped one", lo <= (clean > v ? clean : v) * 1.001 &&
          lo >= (clean < v ? clean : v) * 0.999);

    /* TONE: a 6 kHz sine well down, a 220 Hz one nearly as it was */
    fm_init();
    fm_tape_knobs(0, 0, 127, 0);
    sine(b_l, N, 6000.0f, 0.5f);
    hi = rms(b_l, N / 2, N);
    tape_run(b_l, N, 1.0f);
    lo = rms(b_l, N / 2, N);
    printf("  tone 127: 6 kHz %.1f dB down\n", db(hi) - db(lo));
    CHECK("TONE: 6 kHz at least 9 dB down", lo < hi * 0.355);
    memcpy(b_l, a_l, sizeof a_l);
    tape_run(b_l, N, 1.0f);
    CHECK("... 220 Hz within 1 dB", rms(b_l, N / 2, N) > clean * 0.89);

    /* SATURATION turned while the sine sounds: the new curve crossed over to, no step */
    fm_init();
    fm_tape_knobs(0, 20, 0, 0);
    memcpy(b_l, a_l, sizeof a_l);
    tape_run(b_l, 44032, 1.0f);
    lo = jerk(b_l, 22016, 44032);
    fm_tape_knobs(0, 127, 0, 0);
    tape_run(b_l + 44032, 22016, 1.0f);
    hi = jerk(b_l, 44032 - 2, 44032 + 22016);
    printf("  the knob turned 20 -> 127: biggest second difference %.6f (before %.6f)\n", hi, lo);
    CHECK("SATURATION turned: the curve taken", tf.csat == 127);
    CHECK("... with no step in the sound", hi < lo * 2.0);

    /* HISS: in silence, faint; gone when the tape lets go */
    fm_init();
    fm_set(P_REVERB, 0);
    render(NULL, 22016);
    render(b_l, 22016);
    CHECK("silence without the tape", rms(b_l, 0, 22016) < 1e-7);
    fm_tape_knobs(0, 0, 0, 127);
    fm_tape(100);
    render(NULL, 44032);
    render(b_l, 22016);
    v = rms(b_l, 0, 22016);
    printf("  hiss 127: %.1f dB\n", db(v));
    CHECK("HISS: there, and faint (under -35 dB)", v > 1e-5 && v < 0.0178);
    fm_tape(0);
    render(NULL, 88064);
    render(b_l, 22016);
    CHECK("the amount back to 0: silence, the tape let go", rms(b_l, 0, 22016) < 1e-7 && !tf.live);

    /* WOW: steady, then warbles, then exactly 0 again; never past the knob's reach (136 cents) */
    start();
    fm_tape_knobs(127, 0, 0, 0);
    fm_tape(100);
    {
        int bent = 0, back = 0, was = 0;
        float most = 0.0f;
        for (i = 0; i < 40 * 172; i++) {           /* 40 s */
            render(NULL, 256);
            if (tape_wow != 0.0f) {
                bent++, was = 1;
                if (fm_fabsf(tape_wow) > most)
                    most = fm_fabsf(tape_wow);
            } else if (was)
                back++, was = 0;
        }
        printf("  wow 127, 40 s: bent in %d periods, %d warbles, at most %.1f cents\n", bent, back, (double)most);
        CHECK("WOW: warbles come (2 at least in 40 s)", back >= 2);
        CHECK("... short, the pitch mostly steady", bent < 40 * 172 / 2);
        CHECK("... within the knob's reach", most > 5.0f && most <= 137.0f);
    }
    start();
    fm_tape_knobs(0, 127, 127, 127);
    fm_tape(100);
    same = 1;
    for (i = 0; i < 20 * 172; i++) {
        render(NULL, 256);
        if (tape_wow != 0.0f)
            same = 0;
    }
    CHECK("WOW 0: no bend at all", same);

    /* to listen to (build/host/tape.wav, 16-bit stereo): the pad for 4 s as it is, then 12 s on the tape (WOW 90,
     * SATURATION 100, TONE 70, HISS 90), then 4 s after the tape lets go */
    {
        FILE *f = fopen("build/host/tape.wav", "wb");
        if (f) {
            uint32_t frames = 20u * 172u * 256u, bytes = frames * 4u, v32;
            uint16_t v16;
            uint32_t t;
            int k;
            fwrite("RIFF", 1, 4, f), v32 = 36u + bytes, fwrite(&v32, 4, 1, f), fwrite("WAVEfmt ", 1, 8, f);
            v32 = 16, fwrite(&v32, 4, 1, f), v16 = 1, fwrite(&v16, 2, 1, f), v16 = 2, fwrite(&v16, 2, 1, f);
            v32 = 44100, fwrite(&v32, 4, 1, f), v32 = 176400, fwrite(&v32, 4, 1, f);
            v16 = 4, fwrite(&v16, 2, 1, f), v16 = 16, fwrite(&v16, 2, 1, f);
            fwrite("data", 1, 4, f), fwrite(&bytes, 4, 1, f);
            start();
            fm_tape_knobs(90, 100, 70, 90);
            for (t = 0; t < 20u * 172u; t++) {
                if (t == 4u * 172u)
                    fm_tape(100);
                if (t == 16u * 172u)
                    fm_tape(0);
                fm_tape_poll();
                fm_render(blk, 256, 4096);
                for (k = 0; k < 512; k++) {
                    int16_t s = (int16_t)(blk[k] >> 8);
                    fwrite(&s, 2, 1, f);
                }
            }
            fclose(f);
            printf("  render -> build/host/tape.wav\n");
        }
    }
    return tu_done("tape");
}
