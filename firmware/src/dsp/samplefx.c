/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 GR!MM
 * Crawl changes: Copyright (C) 2026 the Crawl contributors */
/* Crawl's SAMPLE: GR!MM's SAMPLE source (GRiMMBIENT/grimm v2.0.2, firmware/src/sampler.c, with the decoder of its
 * samples.c: Felucca's IMA ADPCM decoder, Copyright (C) 2026 Leo Kuroshita, as GR!MM's two tables). Included by
 * fumi.c after physfx.c. A sample played as a loop, as a tape loop plays. GR!MM's own:
 *   varispeed: the key sets the speed, as on tape (the sample's root key plays it as recorded, an octave down at half
 *   the speed and an octave lower); no time-stretching;
 *   the loop: from START, LENGTH long (50 ms .. to the sample's end), forward or (REVERSE) backward. Two heads: before
 *   the loop's end the second one starts at its beginning and they cross over (equal power, up to 100 ms): no click.
 *   The sample, START, LENGTH and REVERSE take effect there, at the next pass: a turned knob never clicks either;
 *   reading: the ADPCM a 256-sample segment at a time into a window, from the seek index, so a head reads either way
 *   at about one decode per source sample (a speed above 2, two octaves above the root, is held there). A
 *   note keeps its last SP_NW windows: a loop of up to ~3 segments, once round, plays from them with no decoding.
 *   START     where the loop begins: 0 .. 100 % of the sample
 *   LENGTH    the loop: 50 ms .. the rest of the sample (squared: fine at the short end)
 *   REVERSE   OFF / ON: the loop backward
 * Crawl's:
 *   - the samples are in the image (crawl_samples.h, made by tools/crawl_samples.py with their seek index and the
 *     decoder's tables), not in GR!MM's flash slots: no upload, no check at boot, a sample is always ready. GR!MM's
 *     SLOT knob is the part's patch, FM_ENG_SAMPLE + the sample's number; one past the last plays nothing.
 *   - the state is a slot of SP_NSLOT, its windows in RAM (GR!MM: one a voice, the windows in the pool): fumi.c gives
 *     a sounding SAMPLE note its slot.
 *   - the speed comes from fumi.c's pitch in cents (GR!MM: its pitch table), the tape's wow in it.
 *   - sampler_block writes the block at full level (Q15); fumi.c applies the level, the pan, the attack and the
 *     release (GR!MM: its track's envelope, inside).
 * State: sp[slot]: the loop playing and the next one, the heads, the windows' keys; sp_w: the windows. */

#include "crawl_samples.h"

#define SP_NSLOT FM_NSAMPLE                     /* SAMPLE notes at once */
#define SMP_SEG_LOG2 8
#define SMP_SEG (1u << SMP_SEG_LOG2)            /* the seek index's step, samples */
#define SMP_RATE_Q16 32768u                     /* 22050 / 44100 */
#define SP_MIN_LEN 1102u                        /* the shortest loop: 50 ms at 22050 Hz */
#define SP_XF_MAX 2205u                         /* the crossing: at most 100 ms (a quarter of a short loop) */
#define SP_STEP_MAX (2u << 16)                  /* source samples an output sample: two octaves above the root */
#define SP_LV 18000                             /* a voice's level (Q15). Crawl: GR!MM's is 9000, 14 dB under an FM pad
                                                 * here; a part's level only goes down */
#define SP_NW 4u                                /* the windows a note keeps */
enum { SP_PLAY, SP_ARMED, SP_CROSS };           /* a pass: playing; the next loop chosen; the heads crossing */

/* one IMA ADPCM code (4 bits) into the decoder pred, idx (GR!MM's samples.c ima_nib) */
static inline int32_t ima_nib(uint32_t code, int32_t *pred, int32_t *idx)
{
    uint32_t k = (uint32_t)*idx * 8u + (code & 7u);
    int32_t vd = (int32_t)SMP_VD[k], m = -(int32_t)((code >> 3) & 1u);
    *pred = tf_clamp(*pred + ((vd ^ m) - m), -32768, 32767);
    *idx = SMP_NX[k];
    return *pred;
}

typedef struct {
    uint32_t l0, len, xf;                       /* the loop's first sample, its length, its crossing (samples) */
    uint8_t rev, slot, ok;                      /* (slot: the sample's number) */
} sp_loop_t;
typedef struct {
    sp_loop_t cur, nxt;                         /* the loop playing; the next one (the knobs, as the crossing nears) */
    const uint8_t *wd[SP_NW];                   /* the windows: which sample's data, */
    int32_t ws[SP_NW];                          /* .. which segment (-1: none) */
    int32_t r[2];                               /* heads A and B: their place in their window, Q16 */
    uint8_t hw[2], a, st, wnext;                /* .. their windows; which head is A; SP_; the next window to reuse */
    uint32_t ui, uf;                            /* head A in its loop: whole samples, Q16 fraction */
    uint32_t ev, xs, xinv, kq;                  /* the next event (at ui); the crossing: its start, 256 / its length
                                                 * (Q16), how far it is (Q16, 0 .. 256) */
} sp_voice_t;
static sp_voice_t sp[SP_NSLOT];
static int16_t sp_w[SP_NSLOT][SP_NW][SMP_SEG + 1u];   /* a segment and the next sample */

/* the loop sample `smp` and the knobs p (START, LENGTH, REVERSE) ask for (0: there is no such sample) */
static int sp_loop(uint32_t smp, const volatile int32_t *p, sp_loop_t *L)
{
    uint32_t n, l0, av, k;
    L->ok = 0;
    if (smp >= SMP_N || SMP_TAB[smp].n < SP_MIN_LEN + 2u)
        return 0;
    n = SMP_TAB[smp].n;
    l0 = n * (uint32_t)tf_clamp(p[0], 0, 127) / 128u;
    av = n - l0;
    if (av < SP_MIN_LEN) {
        l0 = n - SP_MIN_LEN;
        av = SP_MIN_LEN;
    }
    k = (uint32_t)tf_clamp(p[1], 0, 127);
    L->len = SP_MIN_LEN + (av - SP_MIN_LEN) * (k * k) / 16129u;
    L->l0 = l0;
    L->xf = L->len / 4u < SP_XF_MAX ? L->len / 4u : SP_XF_MAX;
    L->rev = (uint8_t)(p[2] != 0);
    L->slot = (uint8_t)smp;
    L->ok = 1;
    return 1;
}

/* the window holding segment seg of the sample: one kept, or the oldest decoded anew (never `keep`: the other head's) */
static uint32_t sp_win(sp_voice_t *t, uint32_t vi, uint32_t slot, int32_t seg, uint32_t keep)
{
    const uint8_t *d = SMP_TAB[slot].data;
    uint32_t k, q, m, b, n = SMP_TAB[slot].n;
    int32_t pred, idx;
    int16_t *w;
    for (k = 0; k < SP_NW; k++)
        if (t->ws[k] == seg && t->wd[k] == d)
            return k;
    k = t->wnext == keep ? (keep + 1u) % SP_NW : t->wnext;
    t->wnext = (uint8_t)((k + 1u) % SP_NW);
    w = sp_w[vi][k];
    pred = SMP_TAB[slot].ipred[seg];            /* from the index: a segment starts on a whole byte */
    idx = SMP_TAB[slot].iidx[seg];
    q = (uint32_t)seg << SMP_SEG_LOG2;
    m = n - q < SMP_SEG + 1u ? n - q : SMP_SEG + 1u;
    for (b = 0; b + 1u < m; b += 2u) {          /* two samples a byte */
        uint32_t c = d[(q + b) >> 1];
        w[b] = (int16_t)ima_nib(c & 15u, &pred, &idx);
        w[b + 1u] = (int16_t)ima_nib(c >> 4, &pred, &idx);
    }
    if (b < m) {
        w[b] = (int16_t)ima_nib(d[(q + b) >> 1] & 15u, &pred, &idx);
        b++;
    }
    for (; b <= SMP_SEG; b++)                   /* (past the sample's end: its last sample) */
        w[b] = w[b - 1u];
    t->ws[k] = seg;
    t->wd[k] = d;
    return k;
}
/* head h to (u + f / 65536) from the beginning (forward) or the end (REVERSE) of loop L */
static void sp_seat(sp_voice_t *t, uint32_t vi, uint32_t h, const sp_loop_t *L, uint32_t u, uint32_t f)
{
    int32_t i = (int32_t)(L->rev ? L->l0 + L->len - 1u - u : L->l0 + u), seg = i >> SMP_SEG_LOG2;
    int32_t r = ((i & (int32_t)(SMP_SEG - 1u)) << 16) + (L->rev ? -(int32_t)f : (int32_t)f);
    if (r < 0) {                                /* (backward, between a segment's first sample and the one before) */
        if (seg > 0) {
            seg--;
            r += (int32_t)SMP_SEG << 16;
        } else
            r = 0;
    }
    t->hw[h] = (uint8_t)sp_win(t, vi, L->slot, seg, t->hw[h ^ 1u]);
    t->r[h] = r;
}
/* head h past its window's end (or before its beginning): the next segment's window */
static void sp_move(sp_voice_t *t, uint32_t vi, uint32_t h, uint32_t slot)
{
    int32_t r = t->r[h], d = (r >> 16) >> SMP_SEG_LOG2, seg = t->ws[t->hw[h]] + d,
            last = (int32_t)((SMP_TAB[slot].n - 1u) >> SMP_SEG_LOG2);
    r -= d * ((int32_t)SMP_SEG << 16);
    if (seg < 0) {
        seg = 0;
        r = 0;
    } else if (seg > last) {
        seg = last;
        r = (int32_t)(SMP_SEG - 1u) << 16;
    }
    t->hw[h] = (uint8_t)sp_win(t, vi, slot, seg, t->hw[h ^ 1u]);
    t->r[h] = r;
}
/* head A at the next event (ui >= ev): the next loop chosen; head B in, the crossing begun; the pass over */
static void sp_event(sp_voice_t *t, uint32_t vi, uint32_t smp, const volatile int32_t *p)
{
    while (t->ui >= t->ev) {
        if (t->st == SP_PLAY) {                 /* the next pass, from the knobs as they are now */
            uint32_t x;
            if (!sp_loop(smp, p, &t->nxt))
                t->nxt = t->cur;
            x = t->cur.xf < t->nxt.len / 2u ? t->cur.xf : t->nxt.len / 2u;
            x = x ? x : 1u;
            t->xs = t->cur.len - x;
            t->xinv = (256u << 16) / x;
            t->st = SP_ARMED;
            t->ev = t->xs;
        } else if (t->st == SP_ARMED) {         /* head B in at the next loop's beginning */
            uint32_t u = t->ui - t->xs;
            sp_seat(t, vi, t->a ^ 1u, &t->nxt, u, t->uf);
            t->kq = u * t->xinv + (((t->uf >> 4) * (t->xinv >> 4)) >> 8);
            t->st = SP_CROSS;
            t->ev = t->cur.len;
        } else {                                /* A out: B goes on as A, in the next loop */
            t->ui -= t->xs;
            t->a ^= 1u;
            t->cur = t->nxt;
            t->st = SP_PLAY;
            t->ev = t->cur.len - t->cur.xf;
        }
    }
}

static void sampler_begin(uint32_t vi)          /* a note from silence: the loop from the knobs at the first block */
{
    sp_voice_t *t = &sp[vi];
    uint32_t k;
    t->cur.ok = t->nxt.ok = 0;
    for (k = 0; k < SP_NW; k++)
        t->ws[k] = -1;
    t->hw[0] = t->hw[1] = 0;
    t->a = 0;
    t->wnext = 0;
    t->st = SP_PLAY;
}

/* BLK frames of the note in slot vi into mono[], at full level, Q15. Crawl: the slot, the sample's number, the knobs
 * (START, LENGTH, REVERSE) and the pitch (cents, MIDI x 100) given; the level is fumi.c's */
static void sampler_block(uint32_t vi, uint32_t smp, const volatile int32_t *p, float cents, int32_t *mono)
{
    uint32_t i, step, ui, uf, ev, kq, kinc, a;
    sp_voice_t *t = &sp[vi];
    int32_t buf[BLK], ra, rb, dsa, dsb;
    const int16_t *wa, *wb;
    float oct;
    for (i = 0; i < (uint32_t)BLK; i++)
        mono[i] = 0;
    if (!t->cur.ok) {                           /* (the first block, or there was no such sample: is there now?) */
        if (!sp_loop(smp, p, &t->cur))
            return;                             /* no such sample: nothing at all */
        t->ui = t->uf = 0;
        t->st = SP_PLAY;
        t->ev = t->cur.len - t->cur.xf;
        sp_seat(t, vi, t->a, &t->cur, 0, 0);
    }
    /* the speed: the key against the root (varispeed), 12 octaves down .. 2 up as GR!MM's */
    oct = (cents - (float)SMP_TAB[t->cur.slot].root16 * 6.25f) * (1.0f / 1200.0f);
    oct = oct < -12.0f ? -12.0f : oct > 2.0f ? 2.0f : oct;
    step = (uint32_t)((float)SMP_RATE_Q16 * fm_exp2f(oct));
    step = step > SP_STEP_MAX ? SP_STEP_MAX : step < 256u ? 256u : step;

    /* the heads: A, and B as they cross; their places move by the step, backward in a REVERSE loop */
#define SP_LOAD()                                                                                                      \
    do {                                                                                                               \
        a = t->a, ui = t->ui, uf = t->uf, ev = t->ev, kq = t->kq;                                                      \
        kinc = ((step >> 4) * (t->xinv >> 4)) >> 8;                                                                    \
        ra = t->r[a], rb = t->r[a ^ 1u];                                                                               \
        wa = sp_w[vi][t->hw[a]], wb = sp_w[vi][t->hw[a ^ 1u]];                                                         \
        dsa = t->cur.rev ? -(int32_t)step : (int32_t)step, dsb = t->nxt.rev ? -(int32_t)step : (int32_t)step;          \
    } while (0)
#define SP_STORE() (t->ui = ui, t->uf = uf, t->kq = kq, t->r[a] = ra, t->r[a ^ 1u] = rb)
    /* in runs: as far as no event and no window's edge can come (a step is at most 2 source samples: a run of m moves
     * a head at most 2 m, so m <= distance x 31 / 64 is safe), with no tests inside */
#define SP_RUN(m, r, ds)                                                                                               \
    do {                                                                                                               \
        uint32_t room_ = (ds) < 0 ? (uint32_t)((r) >> 16) : SMP_SEG - 1u - (uint32_t)((r) >> 16);                      \
        room_ = ((room_ * 31u) >> 6) + 1u;                                                                             \
        m = m < room_ ? m : room_;                                                                                     \
    } while (0)
    SP_LOAD();
    for (i = 0; i < (uint32_t)BLK;) {
        uint32_t m = (uint32_t)BLK - i, j, e;
        int32_t n;
        if (ui >= ev) {
            SP_STORE();
            sp_event(t, vi, smp, p);
            SP_LOAD();
        }
        if ((uint32_t)(ra >> 16) >= SMP_SEG) {  /* (into the next segment) */
            SP_STORE();
            sp_move(t, vi, a, t->cur.slot);
            SP_LOAD();
        }
        e = ((ev - ui) * 31u) >> 6;             /* (to the event: at least one sample, as ui < ev) */
        m = m < e + 1u ? m : e + 1u;
        SP_RUN(m, ra, dsa);
        if (t->st != SP_CROSS) {
            for (j = 0; j < m; j++, ra += dsa) {
                n = ra >> 16;
                buf[i + j] = wa[n] + (((wa[n + 1] - wa[n]) * ((ra >> 1) & 0x7FFF)) >> 15);
            }
        } else {                                /* equal power: A out, B in */
            if ((uint32_t)(rb >> 16) >= SMP_SEG) {
                SP_STORE();
                sp_move(t, vi, a ^ 1u, t->nxt.slot);
                SP_LOAD();
            }
            SP_RUN(m, rb, dsb);
            for (j = 0; j < m; j++, ra += dsa, rb += dsb, kq += kinc) {
                uint32_t k = kq >> 16;
                int32_t y, yb;
                n = ra >> 16;
                y = wa[n] + (((wa[n + 1] - wa[n]) * ((ra >> 1) & 0x7FFF)) >> 15);
                n = rb >> 16;
                yb = wb[n] + (((wb[n + 1] - wb[n]) * ((rb >> 1) & 0x7FFF)) >> 15);
                k = k > 256u ? 256u : k;
                buf[i + j] = tf_mulq15(y, SINE[256u - k]) + tf_mulq15(yb, SINE[k]);
            }
        }
        i += m;
        uf += m * step;                         /* (head A in its loop) */
        ui += uf >> 16;
        uf &= 0xFFFFu;
    }
    SP_STORE();
#undef SP_LOAD
#undef SP_STORE
#undef SP_RUN

    for (i = 0; i < (uint32_t)BLK; i++)
        mono[i] = (buf[i] * SP_LV) >> 15;
}
