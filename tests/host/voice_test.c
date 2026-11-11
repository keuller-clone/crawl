/* SPDX-License-Identifier: GPL-3.0-only */
/* Crawl's VOICE (firmware/src/dsp/voicefx.c: GR!MM's formant voice and its choirs, a part's source), through the
 * engine on the host: each TYPE sounds at its pitch and in range, VOWEL changes the sound, the attack and the
 * release, polyphony, and FM parts beside it untouched. A render to listen to: build/host/voice.wav. */
#include <string.h>
#include <stdlib.h>
#include "tu.h"
#include "tu_yin.h"
#include "../../firmware/src/dsp/fumi.c"

void fm_midi_out(uint32_t st, uint32_t d1, uint32_t d2) { (void)st; (void)d1; (void)d2; }

#define SR 44100
#define N (3 * SR / 256 * 256)
static int32_t blk[2 * 256];
static float a_l[N], b_l[N];
static const char *const TY[4] = {"SOLO", "GREGORIAN", "NUNS", "CATHEDRAL"};

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
static double hf(const float *x, int from, int to)   /* the highs' share: the first difference's RMS over the RMS */
{
    double a = 0;
    int i;
    for (i = from + 1; i < to; i++)
        a += (double)(x[i] - x[i - 1]) * (x[i] - x[i - 1]);
    return sqrt(a / (to - from - 1)) / (rms(x, from, to) + 1e-12);
}
static void start(int ty, int vowel)
{
    fm_init();
    fm_set(P_REVERB, 0);
    fm_part_patch(1, FM_ENG_VOICE + ty);
    fm_part_knobs(1, vowel, 12, 64, 0);
    fm_part_release(1, 300);
}

int main(void)
{
    int ty, i;
    double fm_level, v;

    /* for scale: an FM pad on the same part, the same note */
    fm_init();
    fm_set(P_REVERB, 0);
    fm_part_patch(1, ZN_SOFT_PAD);
    fm_voice(FM_VOICE_ID, 1, -1200.0f, 100, 64, 0);
    render(a_l, N);
    fm_level = rms(a_l, SR, 2 * SR);

    for (ty = 0; ty < 4; ty++) {
        char what[96];
        double hz;
        start(ty, 0);
        fm_voice(FM_VOICE_ID, 1, -1200.0f, 100, 64, 0);   /* A3, 220 Hz */
        render(a_l, N);
        v = rms(a_l, SR, 2 * SR);
        hz = tu_yin(a_l + SR, 8192, 44100.0);
        printf("  %s: %.1f dB (the FM pad %.1f dB), peak %.3f, %.1f Hz\n", TY[ty], 20.0 * log10(v + 1e-12),
               20.0 * log10(fm_level + 1e-12), peak(a_l, 0, N), hz);
        snprintf(what, sizeof what, "%s: sounds, within 12 dB of the FM pad", TY[ty]);
        CHECK(what, v > fm_level * 0.25 && v < fm_level * 4.0);
        snprintf(what, sizeof what, "%s: A3 at 220 Hz (a choir's women or its octave: 440)", TY[ty]);
        CHECK(what, fabs(tu_cents(hz, 220.0)) < 25.0 || fabs(tu_cents(hz, 440.0)) < 25.0);
        snprintf(what, sizeof what, "%s: far from the output's ceiling", TY[ty]);
        CHECK(what, peak(a_l, 0, N) < 0.5);
        start(ty, 127);                            /* VOWEL: U instead of A */
        fm_voice(FM_VOICE_ID, 1, -1200.0f, 100, 64, 0);
        render(b_l, N);
        snprintf(what, sizeof what, "%s: VOWEL A against U: another sound (the highs' share %.3f, %.3f)", TY[ty],
                 hf(a_l, SR, 2 * SR), hf(b_l, SR, 2 * SR));
        CHECK(what, fabs(hf(a_l, SR, 2 * SR) - hf(b_l, SR, 2 * SR)) > 0.1 * hf(a_l, SR, 2 * SR));
    }

    /* the release: let go, it dies away and the voice is free */
    start(1, 40);
    fm_voice(FM_VOICE_ID, 1, 0.0f, 100, 64, 0);
    render(NULL, SR / 256 * 256);
    CHECK("held: one voice", fm_nvoices == 1);
    fm_voice_off(FM_VOICE_ID);
    render(a_l, N);
    CHECK("let go (release 300 ms): silent after it, the voice free", fm_nvoices == 0 && rms(a_l, SR, 2 * SR) < 1e-6);
    CHECK("... not cut off: still there 100 ms on", rms(a_l, 4096, 4096 + 2048) > 1e-5);

    /* the attack: 800 ms to full level */
    start(2, 40);
    fm_part_attack(1, 800);
    fm_voice(FM_VOICE_ID, 1, 0.0f, 100, 64, 0);
    render(a_l, N);
    printf("  attack 800 ms: %.1f dB in the first 100 ms, %.1f dB after 1 s\n", 20.0 * log10(rms(a_l, 0, 4410) + 1e-12),
           20.0 * log10(rms(a_l, SR, SR + 4410) + 1e-12));
    CHECK("the attack: quiet at first, full after it", rms(a_l, 0, 4410) < rms(a_l, SR, SR + 4410) * 0.2);
    start(2, 40);
    fm_voice(FM_VOICE_ID, 1, 0.0f, 100, 64, 0);
    render(b_l, N);
    CHECK("no attack set: at full level in the first 100 ms", rms(b_l, 0, 4410) > rms(b_l, SR, SR + 4410) * 0.5);

    /* six choir notes at once, and an FM part beside them */
    start(3, 60);
    fm_part_patch(2, ZN_SOFT_PAD);
    for (i = 0; i < 6; i++)
        fm_voice(FM_VOICE_ID + i, 1, -1700.0f + 400.0f * (float)i, 90, 20 + 16 * i, 0);
    fm_voice(FM_VOICE_ID + 10, 2, -2400.0f, 100, 64, 0);
    render(a_l, N);
    printf("  six CATHEDRAL notes and an FM pad: peak %.3f\n", peak(a_l, 0, N));
    CHECK("six choir notes and an FM note: seven voices", fm_nvoices == 7);
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

    /* to listen to (build/host/voice.wav, 16-bit stereo, 40 s): SOLO through A E I O U, then a chord on each choir,
     * with the reverb */
    {
        FILE *f = fopen("build/host/voice.wav", "wb");
        if (f) {
            uint32_t periods = 40u * 172u, bytes = periods * 256u * 4u, v32, t;
            uint16_t v16;
            int k;
            static const float CH[4] = {-1200.0f, -500.0f, -200.0f, 300.0f};
            fwrite("RIFF", 1, 4, f), v32 = 36u + bytes, fwrite(&v32, 4, 1, f), fwrite("WAVEfmt ", 1, 8, f);
            v32 = 16, fwrite(&v32, 4, 1, f), v16 = 1, fwrite(&v16, 2, 1, f), v16 = 2, fwrite(&v16, 2, 1, f);
            v32 = 44100, fwrite(&v32, 4, 1, f), v32 = 176400, fwrite(&v32, 4, 1, f);
            v16 = 4, fwrite(&v16, 2, 1, f), v16 = 16, fwrite(&v16, 2, 1, f);
            fwrite("data", 1, 4, f), fwrite(&bytes, 4, 1, f);
            fm_init();
            fm_set(P_REVERB, 60);
            fm_set(P_REV_SIZE, 90);
            fm_part_attack(1, 600);
            fm_part_release(1, 2500);
            for (t = 0; t < periods; t++) {
                uint32_t s = t / 172u, at = t % 172u;
                if (at == 0 && s < 10u && s % 2u == 0) {            /* SOLO: a vowel every 2 s */
                    fm_part_patch(1, FM_ENG_VOICE);
                    fm_part_knobs(1, (int)(s / 2u) * 127 / 4, 12, 70, 10);
                    fm_voice(FM_VOICE_ID, 1, -1200.0f, 100, 64, 1500);
                }
                if (at == 0 && s >= 10u && (s - 10u) % 10u == 0 && s < 40u) {   /* a choir every 10 s */
                    fm_part_patch(1, FM_ENG_VOICE + 1 + (int)((s - 10u) / 10u));
                    fm_part_knobs(1, 30, 12, 80, 15);
                    for (k = 0; k < 4; k++)
                        fm_voice(FM_VOICE_ID + 1 + k, 1, CH[k], 95, 30 + 22 * k, 6500);
                }
                if (at == 0 && s >= 13u && (s - 13u) % 10u == 0)    /* ... its vowel moving on */
                    fm_part_knobs(1, 100, 12, 80, 15);
                fm_render(blk, 256, 4096);
                for (k = 0; k < 512; k++) {
                    int16_t smp = (int16_t)(blk[k] >> 8);
                    fwrite(&smp, 2, 1, f);
                }
            }
            fclose(f);
            printf("  render -> build/host/voice.wav\n");
        }
    }
    return tu_done("voice");
}
