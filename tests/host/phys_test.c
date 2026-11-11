/* SPDX-License-Identifier: GPL-3.0-only */
/* Crawl's PHYS (firmware/src/dsp/physfx.c: GR!MM's physical models, a part's source), through the engine on the
 * host: each MODEL sounds at its pitch and in range, a struck note dies away and DAMP sets how fast, a bowed one
 * holds, STRUCTURE and the velocity change the sound, the slots (FM_NPHYS notes at once), the release, and FM parts
 * beside it untouched. A render to listen to: build/host/phys.wav. */
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
static const char *const MD[4] = {"MODAL", "STRING", "MEMBRANE", "SYMPATH"};

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
static double db(double v) { return 20.0 * log10(v + 1e-12); }
static void start(int md, int strc, int damp, int bow)
{
    fm_init();
    fm_set(P_REVERB, 0);
    fm_part_patch(1, FM_ENG_PHYS + md);
    fm_part_knobs(1, strc, damp, bow, 80);
    fm_part_release(1, 300);
}

int main(void)
{
    /* each model's plain setting: MODAL's harmonic partials, STRING's straight string, an ideal head, the octaves */
    static const int STRC[4] = {35, 32, 127, 0};   /* (MEMBRANE: the loaded head, its modes harmonic: a pitch to find) */
    int md, i;
    double fm_level, fm_peak, v;

    /* for scale: an FM pad on the same part, the same note */
    fm_init();
    fm_set(P_REVERB, 0);
    fm_part_patch(1, ZN_SOFT_PAD);
    fm_voice(FM_VOICE_ID, 1, -1200.0f, 100, 64, 0);
    render(a_l, N);
    fm_level = rms(a_l, SR, 2 * SR);
    fm_peak = peak(a_l, SR, 2 * SR);

    for (md = 0; md < 4; md++) {
        char what[128];
        double hz, early, late;
        start(md, STRC[md], 80, 0);
        fm_voice(FM_VOICE_ID, 1, -1200.0f, 100, 64, 0);   /* A3, 220 Hz, struck and held */
        render(a_l, N);
        early = rms(a_l, 0, SR / 4), late = rms(a_l, 2 * SR, N);
        hz = tu_yin(a_l + 2048, 8192, 44100.0);
        printf("  %s: first 250 ms %.1f dB, peak %.1f dB (the FM pad %.1f dB, peak %.1f), after 2 s %.1f dB, %.1f Hz\n", MD[md],
               db(early), db(peak(a_l, 0, N)), db(fm_level), db(fm_peak), db(late), hz);
        snprintf(what, sizeof what, "%s: sounds, its strike's peak within 12 dB of the FM pad's", MD[md]);
        CHECK(what, peak(a_l, 0, N) > fm_peak * 0.25 && peak(a_l, 0, N) < fm_peak * 4.0);
        snprintf(what, sizeof what, "%s: A3 at 220 Hz", MD[md]);
        CHECK(what, fabs(tu_cents(hz, 220.0)) < 25.0);
        snprintf(what, sizeof what, "%s: far from the output's ceiling", MD[md]);
        CHECK(what, peak(a_l, 0, N) < 0.5);
        snprintf(what, sizeof what, "%s: struck, it dies away", MD[md]);
        CHECK(what, late < early * 0.5);
        start(md, STRC[md], 20, 0);                /* DAMP down: shorter */
        fm_voice(FM_VOICE_ID, 1, -1200.0f, 100, 64, 0);
        render(b_l, N);
        snprintf(what, sizeof what, "%s: DAMP 20 against 80: gone sooner (%.1f dB against %.1f at 0.5 to 1 s)", MD[md],
                 db(rms(b_l, SR / 2, SR)), db(rms(a_l, SR / 2, SR)));
        CHECK(what, rms(b_l, SR / 2, SR) < rms(a_l, SR / 2, SR) * 0.5);
    }

    /* STRUCTURE: MODAL's partials move (another sound), the pitch's place too */
    start(0, 35, 80, 0);
    fm_voice(FM_VOICE_ID, 1, -1200.0f, 100, 64, 0);
    render(a_l, N);
    start(0, 110, 80, 0);
    fm_voice(FM_VOICE_ID, 1, -1200.0f, 100, 64, 0);
    render(b_l, N);
    for (i = 0; i < SR; i++)
        b_l[i] -= a_l[i];
    printf("  MODAL STRUCTURE 35 / 110: the difference %.1f dB over the first\n", db(rms(b_l, 0, SR)) - db(rms(a_l, 0, SR)));
    CHECK("MODAL: STRUCTURE 110 against 35: another sound", rms(b_l, 0, SR) > 0.5 * rms(a_l, 0, SR));

    /* BOW: sustained, as long as the note is held */
    for (md = 0; md < 2; md++) {
        char what[128];
        start(md, STRC[md], 60, 90);
        fm_voice(FM_VOICE_ID, 1, -1200.0f, 100, 64, 0);
        render(a_l, N);
        printf("  %s bowed: %.1f dB at 0.5 to 1 s, %.1f dB at 2 to 3 s, peak %.3f, %.1f Hz\n", MD[md], db(rms(a_l, SR / 2, SR)),
               db(rms(a_l, 2 * SR, N)), peak(a_l, 0, N), tu_yin(a_l + SR, 8192, 44100.0));
        snprintf(what, sizeof what, "%s bowed: it holds (within 6 dB from 0.5 s to 3 s)", MD[md]);
        CHECK(what, rms(a_l, 2 * SR, N) > rms(a_l, SR / 2, SR) * 0.5 && rms(a_l, 2 * SR, N) < rms(a_l, SR / 2, SR) * 2.0);
        snprintf(what, sizeof what, "%s bowed: within 12 dB of the FM pad, far from the ceiling", MD[md]);
        CHECK(what, rms(a_l, SR, 2 * SR) > fm_level * 0.25 && rms(a_l, SR, 2 * SR) < fm_level * 4.0 && peak(a_l, 0, N) < 0.5);
        snprintf(what, sizeof what, "%s bowed: A3 at 220 Hz", MD[md]);
        CHECK(what, fabs(tu_cents(tu_yin(a_l + SR, 8192, 44100.0), 220.0)) < 25.0);
    }

    /* the velocity: the accent (a harder strike is brighter and louder) */
    start(1, 32, 80, 0);
    fm_voice(FM_VOICE_ID, 1, -1200.0f, 40, 64, 0);
    render(a_l, N);
    start(1, 32, 80, 0);
    fm_voice(FM_VOICE_ID, 1, -1200.0f, 120, 64, 0);
    render(b_l, N);
    printf("  STRING at velocity 40 / 120: %.1f / %.1f dB, the highs' share %.3f / %.3f\n", db(rms(a_l, 0, SR / 2)),
           db(rms(b_l, 0, SR / 2)), hf(a_l, 0, SR / 2), hf(b_l, 0, SR / 2));
    CHECK("STRING: velocity 120 louder than 40", rms(b_l, 0, SR / 2) > rms(a_l, 0, SR / 2) * 1.5);
    CHECK("STRING: ... and brighter", hf(b_l, 0, SR / 2) > hf(a_l, 0, SR / 2) * 1.05);

    /* the release: let go, it dies away and the voice is free */
    start(1, 32, 127, 0);                          /* (DAMP at the top: it rings on while held) */
    fm_voice(FM_VOICE_ID, 1, 0.0f, 100, 64, 0);
    render(a_l, N);
    CHECK("STRING, DAMP at the top, held: one voice, still ringing after 2 s", fm_nvoices == 1 && rms(a_l, 2 * SR, N) > 1e-4);
    fm_voice_off(FM_VOICE_ID);
    render(a_l, N);
    CHECK("let go (release 300 ms): silent after it, the voice free", fm_nvoices == 0 && rms(a_l, SR, 2 * SR) < 1e-6);
    CHECK("... not cut off: still there 100 ms on", rms(a_l, 4096, 4096 + 2048) > 1e-5);

    /* the same note again over itself: one voice, struck again as it rings */
    start(0, 35, 90, 0);
    fm_voice(FM_VOICE_ID, 1, -1200.0f, 100, 64, 0);
    render(a_l, SR / 256 * 256);
    v = rms(a_l, SR / 256 * 256 - 4096, SR / 256 * 256);
    fm_voice(FM_VOICE_ID, 1, -1200.0f, 100, 64, 0);
    render(b_l, SR / 256 * 256);
    CHECK("MODAL struck again while it rings: one voice, louder again", fm_nvoices == 1 && rms(b_l, 0, 4096) > v * 1.2);

    /* the slots: FM_NPHYS notes at once; more go over the oldest; an FM note beside them */
    start(0, 35, 100, 0);
    fm_part_patch(2, ZN_SOFT_PAD);
    for (i = 0; i < FM_NPHYS + 2; i++) {
        fm_voice(FM_VOICE_ID + i, 1, -1700.0f + 300.0f * (float)i, 90, 20 + 16 * i, 0);
        render(NULL, 2048);
    }
    fm_voice(FM_VOICE_ID + 10, 2, -2400.0f, 100, 64, 0);
    render(a_l, N);
    printf("  %d MODAL notes asked for and an FM pad: %d voices, peak %.3f\n", FM_NPHYS + 2, fm_nvoices, peak(a_l, 0, N));
    CHECK("six PHYS notes asked for: FM_NPHYS sound, and the FM note", fm_nvoices == FM_NPHYS + 1);
    CHECK("... the last ones asked for are the ones held", fm_voice_held(FM_VOICE_ID + FM_NPHYS + 1) &&
          fm_voice_held(FM_VOICE_ID + FM_NPHYS) && !fm_voice_held(FM_VOICE_ID) && !fm_voice_held(FM_VOICE_ID + 1));
    CHECK("... in range (the output's soft clip not reached)", peak(a_l, 0, N) < 0.9);
    fm_voice_all_off();
    render(NULL, 4 * N);
    CHECK("all off: all free", fm_nvoices == 0);
    for (i = 0; i < FM_NPHYS; i++)                 /* (the slots are free again: all of them sound) */
        fm_voice(FM_VOICE_ID + i, 1, -1700.0f + 300.0f * (float)i, 90, 64, 0);
    render(a_l, SR / 256 * 256);
    CHECK("FM_NPHYS notes again: all sound", fm_nvoices == FM_NPHYS && rms(a_l, 0, SR / 4) > 1e-3);
    fm_voice_all_off();
    render(NULL, 4 * N);

    /* a part back to an FM patch plays FM again; a VOICE part after PHYS has VOICE's own knobs */
    fm_part_patch(1, ZN_SOFT_PAD);
    fm_voice(FM_VOICE_ID, 1, -1200.0f, 100, 64, 0);
    render(a_l, N);
    v = rms(a_l, SR, 2 * SR);
    CHECK("the part given an FM patch again: the FM pad, as before", fabs(20.0 * log10(v / fm_level)) < 1.0);
    start(0, 127, 127, 127);
    fm_part_patch(1, FM_ENG_VOICE);
    fm_voice(FM_VOICE_ID, 1, -1200.0f, 100, 64, 0);
    render(a_l, N);
    fm_init();
    fm_set(P_REVERB, 0);
    fm_part_patch(1, FM_ENG_VOICE);
    fm_part_release(1, 300);
    fm_voice(FM_VOICE_ID, 1, -1200.0f, 100, 64, 0);
    render(b_l, N);
    CHECK("VOICE after PHYS on a part: as VOICE on a new part",
          fabs(db(rms(a_l, SR, 2 * SR)) - db(rms(b_l, SR, 2 * SR))) < 1.0 && fabs(hf(a_l, SR, 2 * SR) - hf(b_l, SR, 2 * SR)) < 0.02);

    /* to listen to (build/host/phys.wav, 16-bit stereo, 48 s), with the reverb: 0 s a bell line (MODAL, struck),
     * 10 s inharmonic metal, 18 s bowed metal, 26 s bowed strings, 34 s SYMPATH (MIN, then 5TH), 42 s the drum */
    {
        FILE *f = fopen("build/host/phys.wav", "wb");
        if (f) {
            uint32_t periods = 48u * 172u, bytes = periods * 256u * 4u, v32, t;
            uint16_t v16;
            int k;
            static const float LINE[8] = {-1200.0f, -500.0f, -900.0f, -200.0f, -1400.0f, -700.0f, -900.0f, -1200.0f};
            fwrite("RIFF", 1, 4, f), v32 = 36u + bytes, fwrite(&v32, 4, 1, f), fwrite("WAVEfmt ", 1, 8, f);
            v32 = 16, fwrite(&v32, 4, 1, f), v16 = 1, fwrite(&v16, 2, 1, f), v16 = 2, fwrite(&v16, 2, 1, f);
            v32 = 44100, fwrite(&v32, 4, 1, f), v32 = 176400, fwrite(&v32, 4, 1, f);
            v16 = 4, fwrite(&v16, 2, 1, f), v16 = 16, fwrite(&v16, 2, 1, f);
            fwrite("data", 1, 4, f), fwrite(&bytes, 4, 1, f);
            fm_init();
            fm_set(P_REVERB, 60);
            fm_set(P_REV_SIZE, 90);
            fm_part_release(1, 2500);
            for (t = 0; t < periods; t++) {
                uint32_t s = t / 172u, at = t % 172u, n = t / 215u;   /* a note every 1.25 s */
                int note = t % 215u == 0;
                if (at == 0 && s == 0)
                    fm_part_patch(1, FM_ENG_PHYS + 0), fm_part_knobs(1, 35, 95, 0, 70);
                if (at == 0 && s == 10)
                    fm_part_knobs(1, 100, 100, 0, 60);
                if (at == 0 && s == 18)
                    fm_part_knobs(1, 60, 90, 70, 50), fm_part_attack(1, 1200);
                if (at == 0 && s == 26)
                    fm_part_patch(1, FM_ENG_PHYS + 1), fm_part_knobs(1, 32, 90, 80, 55);
                if (at == 0 && s == 34)
                    fm_part_patch(1, FM_ENG_PHYS + 3), fm_part_knobs(1, 4 * 16 + 8, 100, 0, 75), fm_part_attack(1, 0);
                if (at == 0 && s == 38)
                    fm_part_knobs(1, 1 * 16 + 8, 100, 20, 75);
                if (at == 0 && s == 42)
                    fm_part_patch(1, FM_ENG_PHYS + 2), fm_part_knobs(1, 40, 70, 40, 60);
                if (note && s < 18u)               /* struck: a line */
                    fm_voice(FM_VOICE_ID + (int)(n % 3u), 1, LINE[n % 8u] - (s >= 10u ? 1200.0f : 0.0f), 70 + 10 * (int)(n % 4u),
                             40 + 24 * (int)(n % 3u), 1000);
                if (at == 0 && s >= 18u && s < 34u && (s - 18u) % 4u == 0)   /* bowed: two notes every 4 s */
                    for (k = 0; k < 2; k++)
                        fm_voice(FM_VOICE_ID + k, 1, LINE[(s / 4u + (uint32_t)k * 3u) % 8u] - 1200.0f, 90, 40 + 48 * k, 3500);
                if (note && s >= 34u && s < 42u)
                    fm_voice(FM_VOICE_ID + (int)(n % 2u), 1, LINE[n % 8u] - 1200.0f, 85, 50 + 28 * (int)(n % 2u), 1000);
                if (t % 86u == 0 && s >= 42u && s < 47u)             /* the drum: every half second, low */
                    fm_voice(FM_VOICE_ID, 1, t % 344u == 0 ? -3100.0f : -2400.0f, t % 172u == 0 ? 110 : 70, 64, 200);
                fm_render(blk, 256, 4096);
                for (k = 0; k < 512; k++) {
                    int16_t smp = (int16_t)(blk[k] >> 8);
                    fwrite(&smp, 2, 1, f);
                }
            }
            fclose(f);
            printf("  render -> build/host/phys.wav\n");
        }
    }
    return tu_done("phys");
}
