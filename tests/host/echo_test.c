/* SPDX-License-Identifier: GPL-3.0-only */
/* Crawl's DELAY and GRAIN (firmware/src/dsp/echofx.c: GR!MM's tape echo and granular cloud on one line), on the
 * host: the echo's time and feedback, no clipping at full feedback, silence when done, the cloud, FREEZE, and
 * through the engine: off is off. A render to listen to: build/host/echo.wav. */
#include <string.h>
#include <stdlib.h>
#include "tu.h"
#include "../../firmware/src/dsp/fumi.c"

void fm_midi_out(uint32_t st, uint32_t d1, uint32_t d2) { (void)st; (void)d1; (void)d2; }

#define SR 44100
#define NMAX (12 * SR)
static float snd[NMAX], ol[NMAX], orr[NMAX];
static int32_t blk[2 * 256];

/* the effects alone: n frames of the send in, the returns out */
static void fx_run(int from, int n)
{
    int i;
    for (i = from; i < from + n; i++) {
        float l = 0.0f, r = 0.0f;
        echofx_frame(snd[i], &l, &r);
        ol[i] = l, orr[i] = r;
    }
}
static void tone_in(int from, int n, float hz, float amp)
{
    int i;
    for (i = 0; i < n; i++)
        snd[from + i] = amp * (float)sin(6.283185307179586 * hz * i / SR);
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
static double jerk(const float *x, int from, int to)
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
static void fresh(void)
{
    fm_init();
    memset(snd, 0, sizeof snd);
    memset(ol, 0, sizeof ol);
    memset(orr, 0, sizeof orr);
}
static int ms(double t) { return (int)(t * SR / 1000.0); }

int main(void)
{
    int i, at, want;
    double e1, e2, v;

    /* DELAY: a 20 ms burst comes back after TIME (the knob's ms x 0.4), a period late */
    fresh();
    fm_delay(88, 64, 0, 127);
    tone_in(0, ms(20), 440.0f, 0.5f);
    fx_run(0, 2 * SR);
    want = (int)(TD_MS[88] * 2u / 5u);
    at = 0;
    for (i = ms(40); i < SR; i++)
        if (fabs((double)ol[i]) > fabs((double)ol[at]))
            at = i;
    printf("  TIME 88: the echo's peak at %.0f ms (the knob: %d ms, + the burst and a period)\n", at * 1000.0 / SR, want);
    CHECK("DELAY: the echo comes back at TIME", fabs(at * 1000.0 / SR - (want + 10 + 6)) < 25.0);
    e1 = rms(ol, ms(want), ms(want + 60));
    e2 = rms(ol, ms(2 * want), ms(2 * want + 60));
    printf("  FEEDBACK 64: the second echo %.1f dB under the first\n", 20.0 * log10(e1 / (e2 > 1e-12 ? e2 : 1e-12)));
    CHECK("... and again, quieter (FEEDBACK 64)", e2 < e1 * 0.9 && e2 > e1 * 0.05);
    CHECK("... the right side too, its own (9 ms later)", rms(orr, ms(want), ms(want + 60)) > e1 * 0.3 &&
          memcmp(ol, orr, sizeof(float) * (size_t)SR) != 0);

    fresh();
    fm_delay(88, 0, 0, 127);
    tone_in(0, ms(20), 440.0f, 0.5f);
    fx_run(0, 2 * SR);
    e1 = rms(ol, ms(want), ms(want + 60));
    e2 = rms(ol, ms(2 * want), ms(2 * want + 60));
    CHECK("FEEDBACK 0: one echo only", e1 > 1e-3 && e2 < e1 * 0.02);

    /* full FEEDBACK (110 %), a loud send for 10 s: the loop holds, nothing clips or runs away */
    fresh();
    fm_delay(60, 127, 20, 127);
    tone_in(0, 10 * SR, 220.0f, 0.9f);
    fx_run(0, 10 * SR);
    v = peak(ol, 0, 10 * SR);
    printf("  FEEDBACK 127, a send at 0.9 for 10 s: the return's peak %.2f\n", v);
    CHECK("full FEEDBACK: bounded (under 2.1)", v < 2.1 && v > 0.1);
    CHECK("... the last second no louder than the fifth", rms(ol, 9 * SR, 10 * SR) < rms(ol, 4 * SR, 5 * SR) * 1.2);

    /* silence in: the echoes die away and the effects stop */
    fresh();
    fm_delay(88, 64, 50, 127);
    tone_in(0, SR / 2, 330.0f, 0.5f);
    fx_run(0, 2 * SR);
    CHECK("sounding: live", fxp.live);
    memset(snd, 0, sizeof snd);
    for (i = 0; i < 10; i++)
        fx_run(0, 2 * SR);
    CHECK("20 s of silence in: exact silence out, nothing runs", !fxp.live && peak(ol, 0, 2 * SR) == 0.0);

    /* GRAIN alone (DELAY's MIX 0): a cloud of what was sent, the two sides different */
    fresh();
    fm_grain(80, 100, 127, 127);
    tone_in(0, SR, 330.0f, 0.5f);
    fx_run(0, 3 * SR);
    e1 = rms(ol, SR / 2, 2 * SR), e2 = rms(orr, SR / 2, 2 * SR);
    printf("  GRAIN: the cloud %.1f dB left, %.1f dB right, peak %.2f\n", 20.0 * log10(e1 + 1e-12), 20.0 * log10(e2 + 1e-12),
           peak(ol, 0, 3 * SR));
    CHECK("GRAIN: a cloud from the line, DELAY's return off", e1 > 1e-3 && e2 > 1e-3);
    CHECK("... panned (SCATTER): the sides differ", memcmp(ol, orr, sizeof(float) * (size_t)(2 * SR)) != 0);
    CHECK("... in range", peak(ol, 0, 3 * SR) < 2.1 && peak(orr, 0, 3 * SR) < 2.1);

    /* without FREEZE (FEEDBACK 0) the line is written over: the cloud is gone; with it, it stays */
    fresh();
    fm_delay(88, 0, 0, 0);
    fm_grain(80, 100, 127, 127);
    tone_in(0, SR, 330.0f, 0.5f);
    fx_run(0, SR);
    memset(snd, 0, sizeof snd);
    for (i = 0; i < 3; i++)
        fx_run(0, 2 * SR);
    CHECK("not frozen: 6 s on, the cloud is gone", rms(ol, SR, 2 * SR) < 1e-5);
    fresh();
    fm_delay(88, 0, 0, 0);
    fm_grain(80, 100, 127, 127);
    tone_in(0, SR, 330.0f, 0.5f);
    fx_run(0, SR);
    fm_grain_freeze(1);
    memset(snd, 0, sizeof snd);
    for (i = 0; i < 3; i++)
        fx_run(0, 2 * SR);
    v = rms(ol, SR, 2 * SR);
    printf("  FREEZE: 6 s on, the cloud at %.1f dB\n", 20.0 * log10(v + 1e-12));
    CHECK("FREEZE: 6 s on, the cloud still sounds", v > 1e-3);
    tone_in(0, SR, 880.0f, 0.5f);                  /* (sent while frozen: not recorded) */
    fx_run(0, SR);
    fm_grain_freeze(0);
    memset(snd, 0, sizeof snd);
    for (i = 0; i < 3; i++)
        fx_run(0, 2 * SR);
    CHECK("let go: the line records again, the held sound is written over", rms(ol, SR, 2 * SR) < 1e-5);

    /* FREEZE while echoes sound: they fade over a period, no step */
    fresh();
    fm_delay(70, 100, 0, 127);
    tone_in(0, 4 * SR, 220.0f, 0.4f);
    fx_run(0, 2 * SR);
    e1 = jerk(ol, SR, 2 * SR);
    fm_grain_freeze(1);
    fx_run(2 * SR, SR);
    e2 = jerk(ol, 2 * SR - 2, 3 * SR);
    printf("  FREEZE on sounding echoes: biggest second difference %.5f (before %.5f)\n", e2, e1);
    CHECK("FREEZE: the echoes fade without a step (a step would be near 0.1)", e2 < 0.01);
    CHECK("... to nothing", peak(ol, 2 * SR + 1024, 3 * SR) == 0.0);

    /* through the engine: all off by default; the send and a MIX up, the echoes run; off again, they stop */
    fm_init();
    fm_set(P_REVERB, 0);
    fm_part_patch(1, ZN_SOFT_PAD);
    fm_voice(FM_VOICE_ID, 1, 0.0f, 110, 64, 400);
    for (i = 0; i < 172; i++)
        fm_render(blk, 256, 4096);
    CHECK("engine: by default nothing of it runs", !fxp.live && fx_send == 0.0f);
    fm_fx_send(100);
    fm_delay(88, 64, 50, 127);
    fm_voice(FM_VOICE_ID + 1, 1, 700.0f, 110, 64, 400);
    for (i = 0; i < 172; i++)
        fm_render(blk, 256, 4096);
    CHECK("engine: the send and DELAY's MIX up: echoes", fxp.live);
    fm_fx_send(0);
    fm_voice_all_off();
    for (i = 0; i < 60 * 172; i++)
        fm_render(blk, 256, 4096);
    CHECK("engine: the send back to 0, a minute on: it has stopped", !fxp.live);

    /* to listen to (build/host/echo.wav, 16-bit stereo, 32 s): pad notes every 2 s. 0-8 s dry; 8-16 s DELAY;
     * 16-24 s GRAIN too; 24-30 s FREEZE (no new notes); then let go */
    {
        FILE *f = fopen("build/host/echo.wav", "wb");
        if (f) {
            uint32_t periods = 32u * 172u, bytes = periods * 256u * 4u, v32, t;
            uint16_t v16;
            int k, note = 0;
            static const float CENTS[4] = {0.0f, 300.0f, 700.0f, -500.0f};
            fwrite("RIFF", 1, 4, f), v32 = 36u + bytes, fwrite(&v32, 4, 1, f), fwrite("WAVEfmt ", 1, 8, f);
            v32 = 16, fwrite(&v32, 4, 1, f), v16 = 1, fwrite(&v16, 2, 1, f), v16 = 2, fwrite(&v16, 2, 1, f);
            v32 = 44100, fwrite(&v32, 4, 1, f), v32 = 176400, fwrite(&v32, 4, 1, f);
            v16 = 4, fwrite(&v16, 2, 1, f), v16 = 16, fwrite(&v16, 2, 1, f);
            fwrite("data", 1, 4, f), fwrite(&bytes, 4, 1, f);
            fm_init();
            fm_set(P_REVERB, 30);
            fm_part_patch(1, ZN_SOFT_PAD);
            fm_part_release(1, 600);
            for (t = 0; t < periods; t++) {
                if (t % 344u == 0 && t < 24u * 172u)
                    fm_voice(FM_VOICE_ID + (note & 3), 1, CENTS[note & 3], 110, 40 + 16 * (note & 3), 500), note++;
                if (t == 8u * 172u)
                    fm_fx_send(100), fm_delay(96, 80, 60, 110);
                if (t == 16u * 172u)
                    fm_grain(90, 90, 110, 110);
                if (t == 24u * 172u)
                    fm_grain_freeze(1);
                if (t == 30u * 172u)
                    fm_grain_freeze(0), fm_fx_send(0);
                fm_render(blk, 256, 4096);
                for (k = 0; k < 512; k++) {
                    int16_t s = (int16_t)(blk[k] >> 8);
                    fwrite(&s, 2, 1, f);
                }
            }
            fclose(f);
            printf("  render -> build/host/echo.wav\n");
        }
    }
    return tu_done("echo");
}
