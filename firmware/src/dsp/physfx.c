/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
 * GR!MM changes: Copyright (C) 2026 GR!MM
 * Crawl changes: Copyright (C) 2026 the Crawl contributors */
/* Crawl's PHYS: GR!MM's PHYS source (GRiMMBIENT/grimm v2.0.2, firmware/src/phys.c). Included by fumi.c after
 * voicefx.c. Felucca's PHYS engine (hugelton/Felucca firmware/src/eng_phys.c), physical models. Its DSP is Felucca's
 * fixed-point port of DaisySP (Electrosmith, Emilie Gillet; MIT: phys_dsp.c) and of Rings' sympathetic strings (Emilie
 * Gillet; MIT: phys_symp.c), both as Felucca and GR!MM have them, unchanged. Taken over from eng_phys.c: the four
 * models, their parameters' meanings, the SYMP chords, the output gains and soft knee, the state reset.
 *   MODEL      MODAL (a bank of band-pass modes struck or bowed: bells, bars, plates, metal), STRING (an extended
 *              Karplus-Strong string, plucked or bowed), MEMBRANE (a struck drum head), SYMPATH (a string with three
 *              sympathetic strings ringing along)
 *   STRUCTURE  MODAL: the partials, harmonic .. more and more inharmonic (metal); STRING: a curved bridge (buzzy)
 *              .. dispersion (stiff); MEMBRANE: HARM, an ideal head .. a loaded one; SYMPATH: CHORD of its strings
 *   DAMP       the decay (at the top it rings on)
 *   BOW        MODAL, STRING: 0 struck / plucked, above sustained (bowed, blown) at that level; MEMBRANE: BEND,
 *              the pitch falling after the strike; SYMPATH: BUZZ, the curved bridge
 * Fixed at Felucca's defaults: POS 20 (SYMPATH: SYMP), ACC 90, EXC 0.
 * Crawl's:
 *   - a voice is fumi.c's, on a part whose patch is FM_ENG_PHYS + MODEL; the knobs are the part's (fm_part_knobs):
 *     STRUCTURE, DAMP, BOW, and a fourth, BRIGHT, which GR!MM fixes at 80.
 *   - the state is a slot of PH_NSLOT (4.3 KB each), not one a voice: fumi.c gives a sounding PHYS note its slot.
 *   - the pitch comes from fumi.c: the phase increment (GR!MM: from its pitch table), and the SYMPATH strings' from
 *     it by voicefx.c's fmt_ratio.
 *   - the accent follows the note's velocity (Felucca's does; GR!MM fixes the velocity at 100).
 *   - phys_block writes the block at full level (Q15, x 2 as GR!MM has PHYS against VOICE); fumi.c applies the
 *     level, the pan, the attack and the release (GR!MM: its track's envelope, inside). */

void *memcpy(void *, const void *, __SIZE_TYPE__);   /* (phys_dsp.c's; the FM-1 build has no string.h) */
#ifndef FM_HOST
/* The FM-1 build links no compiler runtime. At -O2 (GR!MM builds at -Os) the compiler calls this for one 64-bit
 * division of phys_dsp.c's (px_modal_exciter, once a block): a plain long division, no division inside it */
unsigned long long __udivdi3(unsigned long long n, unsigned long long d)
{
    unsigned long long q = 0, r = 0;
    int i = 63;
    if (!d)
        return ~0ull;
    while (i > 0 && !(n >> i))
        i--;
    for (; i >= 0; i--) {
        r = (r << 1) | ((n >> i) & 1u);
        if (r >= d) {
            r -= d;
            q |= 1ull << i;
        }
    }
    return q;
}
#endif
#include "phys_dsp.c"
#include "phys_symp.c"

enum { PM_MODAL, PM_STRING, PM_MEMB, PM_SYMP, PM_COUNT };
#define PH_NSLOT FM_NPHYS                              /* PHYS notes at once (Crawl: RAM, and the FM-1's time: docs/BUDGET.md) */
#define PHYS_POS 20                             /* Felucca's defaults for the knobs GR!MM leaves out */
#define PHYS_ACC 90
/* output: model 1.0 -> this (Felucca's VOICE_FS scale), then a soft knee; per model (a strike of all the modes in
 * phase peaks high but is short) */
static const int32_t PHYS_GAIN[PM_COUNT] = {28000, 12000, 90000, 12000};

typedef struct {
    uint8_t model;                              /* PM_* the state belongs to */
    union {
        px_modal_t m;                           /* MODAL, MEMB */
        px_string_t s;
        px_symp_t y;
    } u;
} phys_slot_t;
static phys_slot_t ph_slot[PH_NSLOT];
static uint32_t ph_rng = 0x2F6E2B1u;            /* Crawl: the seeds' random (GR!MM: the voice's) */

/* SYMP: the sympathetic strings above the note, cents (CHORD: OCT 5TH 4TH MAJ MIN SUS 7TH, and ROOT) */
static const int16_t PHYS_CHORDS[7][PX_NSYMP] = {
    {-1200, 1200, 2400}, {700, 1200, 1900}, {500, 1200, 1700}, {400, 700, 1200},
    {300, 700, 1200}, {200, 700, 1400}, {400, 700, 1000},
};
static const int8_t PHYS_DETUNE[PX_NSYMP] = {1, -1, 0};   /* 1/16 semitone (6 cents): the strings beat a little */

/* a clean state for model md */
static void phys_reset(phys_slot_t *S, uint32_t md, uint32_t seed)
{
    uint32_t i, *w = (uint32_t *)&S->u;
    for (i = 0; i < sizeof S->u / 4u; i++)
        w[i] = 0;
    S->model = (uint8_t)md;
    seed |= 1u;
    switch (md) {
    case PM_STRING:
    case PM_SYMP:                               /* (the main string is the union's px_string_t) */
        S->u.s.rng = seed;
        S->u.s.src = 1u << 30;
        break;
    default:
        S->u.m.rng = seed;
        break;
    }
}

static void phys_strike(uint32_t slot)          /* the same state struck again (a note over a sounding one) */
{
    phys_slot_t *S = &ph_slot[slot];
    if (S->model == PM_MODAL || S->model == PM_MEMB)
        S->u.m.trig = 1;
    else
        S->u.s.trig = 1;
}

static void phys_begin(uint32_t slot, uint32_t md)   /* a note from silence: from rest, struck */
{
    phys_reset(&ph_slot[slot], md < PM_COUNT ? md : PM_MODAL, px_rand(&ph_rng));
    phys_strike(slot);
}

/* SYMP: the sympathetic strings' pitches (cycles Q32) by CHORD: a chord above the note (f0), or ROOT (C) */
static void phys_symp_pitches(int32_t chord, uint32_t f0, uint32_t *f)
{
    uint32_t c = (uint32_t)tf_clamp(chord, 0, 127) >> 4, k;   /* 0..7 */
    for (k = 0; k < PX_NSYMP; k++) {
        int32_t p16 = PHYS_DETUNE[k];
        if (c < 7u) {
            p16 += (PHYS_CHORDS[c][k] * 41) >> 8;      /* cents -> 1/16 semitones (0.16) */
            f[k] = (uint32_t)(((uint64_t)f0 * fmt_ratio(p16)) >> 16);
        } else {                                    /* C3, G3, C4 from A4 (2^32 x 440 / 44100) */
            p16 += (48 + (k == 1u ? 7 : k == 2u ? 12 : 0) - 69) * 16;
            f[k] = (uint32_t)(((uint64_t)42852281u * fmt_ratio(p16)) >> 16);
        }
    }
}

/* BLK frames of the note in slot into mono[] (eng_phys.c phys_render), at full level, Q15. Crawl: the slot, MODEL,
 * the phase increment f0 and the velocity given; p: STRUCTURE, DAMP, BOW, BRIGHT; the level is fumi.c's */
static void phys_block(uint32_t slot, uint32_t md, const volatile int32_t *p, uint32_t f0, int32_t vel, int32_t *mono)
{
    phys_slot_t *S = &ph_slot[slot];
    uint32_t i;
    int32_t y[BLK], ax[BLK];
    int32_t strc = tf_clamp(p[0], 0, 127) * 516, brit = tf_clamp(p[3], 0, 127) * 516, dmp = tf_clamp(p[1], 0, 127) * 516;
    int32_t pos = PHYS_POS, k4 = tf_clamp(p[2], 0, 127), bow = k4 * 206;   /* BOW: Dust at up to 0.4 */
    int32_t acc = tf_clamp(PHYS_ACC * tf_clamp(vel, 1, 127) * 4, 0, 65536);   /* ACC at the velocity, Q16 */
    int32_t g, sh;
    if (md >= PM_COUNT)
        md = PM_MODAL;
    g = PHYS_GAIN[md], sh = bow && (md == PM_MODAL || md == PM_STRING) ? 18 : 20;   /* (bowed: x 4, as loud as the
                                                                                    * other sources) */
    if (S->model != md) {                       /* MODEL changed under a sounding note: struck anew */
        phys_reset(S, md, px_rand(&ph_rng));
        phys_strike(slot);
    }
    switch (md) {
    case PM_MODAL: {
        px_modal_blk_t B;
        px_modal_block(&B, &S->u.m, f0, strc, brit, dmp, acc, pos * 129, bow);   /* pos 0..1/4 */
        px_modal_run(&B, &S->u.m, y, ax, BLK);
        break;
    }
    case PM_MEMB: {
        px_modal_blk_t B;
        px_memb_block(&B, &S->u.m, f0, strc, brit, dmp, acc, pos * 516, k4 * 6192);   /* BEND 0..12 st */
        px_modal_run(&B, &S->u.m, y, ax, BLK);
        break;
    }
    case PM_STRING: {
        px_string_blk_t B;
        px_string_block(&B, &S->u.s, f0, strc, brit, dmp, acc, bow, pos * 516);   /* 0..1/2 period */
        px_string_excite(&B, &S->u.s, ax, BLK);
        px_string_run(&B, &S->u.s, ax, y, BLK);
        break;
    }
    default: {                                  /* SYMP */
        px_string_blk_t B;
        uint32_t f[PX_NSYMP];
        phys_symp_pitches(p[0], f0, f);
        px_string_block(&B, &S->u.y.s, f0, 16384 - k4 * 129, brit, dmp, acc, 0, 0);   /* BUZZ: STRC 0.25 .. 0 */
        px_string_excite(&B, &S->u.y.s, ax, BLK);
        px_string_run(&B, &S->u.y.s, ax, y, BLK);
        px_symp_block(&S->u.y, f, brit, dmp, pos * 516);
        px_symp_run(&S->u.y, y, y, BLK);
        break;
    }
    }
    for (i = 0; i < (uint32_t)BLK; i++) {       /* (EXC 0: the exciter is not heard); the knee caps a strike */
        int32_t s = px_m(y[i], g, sh), a = s < 0 ? -s : s;
        if (a > 24000) {                        /* soft knee: only peaks saturate */
            a = 24000 + (tf_softclip(tf_clamp(a - 24000, 0, 1 << 20) * 2) >> 1);
            s = s < 0 ? -a : a;
        }
        mono[i] = s * 2;                        /* (GR!MM: s x amp >> 14, twice VOICE's s x amp >> 15) */
    }
}
