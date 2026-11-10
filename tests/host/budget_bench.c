/* SPDX-License-Identifier: GPL-3.0-only */
/* M1 budget spike (docs/BUDGET.md): what each part of the engine costs, on this computer. fm_render's time for a
 * 256-frame block (the FM-1's own block, 5805 us of sound) in each configuration: the median of ~3 s of blocks.
 * Host figures compare the parts with each other; the FM-1's own figure is plat_cpu_pct (the HUD), on hardware.
 *
 *   cc -O2 -ffp-contract=off -std=c99 -D_POSIX_C_SOURCE=200809L -DFM_HOST -Ifirmware/src/dsp \
 *      -o build/host/budget_bench tests/host/budget_bench.c -lm && build/host/budget_bench
 * From Windows: tools\docker_run.ps1 sh -c "cc ... && build/host/budget_bench" */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../../firmware/src/dsp/fumi.c"

void fm_midi_out(uint32_t st, uint32_t d1, uint32_t d2) { (void)st; (void)d1; (void)d2; }

#define NBLK 512
static int32_t blk[2 * 256];
static double t_us[NBLK];
static int cmp(const void *a, const void *b) { double d = *(const double *)a - *(const double *)b; return d < 0 ? -1 : d > 0; }

static double now_us(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec * 1e6 + (double)t.tv_nsec * 1e-3;
}

/* the median block, in us; voices: what fm_nvoices said at the end */
static double measure(int *voices)
{
    int i;
    for (i = 0; i < NBLK; i++) {
        double t0 = now_us();
        fm_render(blk, 256, 4096);
        t_us[i] = now_us() - t0;
    }
    qsort(t_us, NBLK, sizeof t_us[0], cmp);
    if (voices)
        *voices = fm_nvoices;
    return t_us[NBLK / 2];
}

static void bq(void)
{
    int i;
    fm_init();
    fm_set(P_REVERB, 0);
    fm_tape(0);
    for (i = 0; i < FM_NBED; i++)
        fm_bed(i, 0);
    for (i = 0; i < 40; i++)                       /* let the smoothers settle */
        fm_render(blk, 256, 4096);
}

static double base;
static void row(const char *name)
{
    int v;
    double us = measure(&v);
    printf("%-44s %8.1f us  %6.2f %% of a block   %+8.1f us over idle   voices %d\n", name, us, us * 100.0 / 5805.0,
           us - base, v);
}

/* n held notes on a scene part (patch slot), an octave apart in fifths */
static void hold(int n, int part, int slot)
{
    int i;
    fm_part_patch(part, slot);
    fm_part_release(part, 8000);
    for (i = 0; i < n; i++)
        fm_voice(FM_VOICE_ID + i, part, -1200.0f + 700.0f * (float)(i % 5) - 1200.0f * (float)(i / 5), 100, 64, 0);
}

int main(void)
{
    static const int NV[] = {1, 2, 4, 8, 12};
    unsigned k;
    int i;
    bq();
    base = 0;
    base = measure(NULL);
    printf("block: 256 frames at 44100 Hz = 5805 us of sound; FM_NVOICE %d\n", FM_NVOICE);
    row("idle (no voices, reverb 0, beds 0, tape 0)");
    for (k = 0; k < sizeof NV / sizeof NV[0]; k++) {
        char name[64];
        bq();
        hold(NV[k], 1, ZN_SOFT_PAD);
        snprintf(name, sizeof name, "FM x%d (scene part, soft pad, held)", NV[k]);
        row(name);
    }
    bq();
    for (i = 0; i < 8; i++)
        fm_note(FM_NOTE_ID + i, 1, -1200.0f + 200.0f * (float)i);
    row("FM x8 (the instrument: koto, just plucked)");
    bq();
    fm_set(P_REVERB, 100);
    hold(8, 1, ZN_SOFT_PAD);
    row("FM x8 + reverb 100");
    bq();
    fm_set(P_REVERB, 100);
    for (i = 0; i < 8; i++)
        fm_note(FM_NOTE_ID + i, 1, -1200.0f + 200.0f * (float)i);
    row("koto x8 + reverb 100 (ZEN-1's K0 case)");
    bq();
    fm_set(P_REVERB, 100);
    hold(1, 1, ZN_SOFT_PAD);
    row("reverb 100 (with one voice feeding it)");
    bq();
    fm_tape_knobs(64, 110, 90, 127);
    fm_tape(100);
    hold(1, 1, ZN_SOFT_PAD);
    row("tape 100, its four knobs up (with one voice)");
    bq();
    fm_fx_send(100);
    fm_delay(88, 64, 50, 127);
    hold(1, 1, ZN_SOFT_PAD);
    row("DELAY (send 100, MIX 127, with one voice)");
    bq();
    fm_fx_send(100);
    fm_grain(80, 80, 30, 127);
    hold(1, 1, ZN_SOFT_PAD);
    row("GRAIN (send 100, MIX 127, with one voice)");
    for (i = 0; i < FM_NBED; i++) {
        static const char *const BN[] = {"bed: wind 100", "bed: stream 100", "bed: rain 100"};
        bq();
        fm_bed(i, 100);
        row(BN[i]);
    }
    bq();
    fm_bed_style(FM_BED_STREAM, FM_WATER_BROOK);
    fm_bed(FM_BED_STREAM, 100);
    row("bed: stream, the recorded brook (ADPCM loop)");
    bq();
    fm_bed_style(FM_BED_WIND, FM_AIR_INSECTS);
    fm_bed(FM_BED_WIND, 100);
    row("bed: air, the recorded insects (ADPCM loop)");
    bq();
    fm_set(P_REVERB, 100);
    fm_tape_knobs(64, 110, 90, 127);
    fm_tape(100);
    for (i = 0; i < FM_NBED; i++)
        fm_bed(i, 100);
    hold(12, 1, ZN_SOFT_PAD);
    row("everything: FM x12, reverb, tape, three beds");
    return 0;
}
