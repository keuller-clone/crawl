/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 GR!MM
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments (GRAIN: Felucca's eng_grain.c; soft_knee:
 * Felucca's dsp.c; as GR!MM has them)
 * Crawl changes: Copyright (C) 2026 the Crawl contributors */
/* Crawl's DELAY and GRAIN: GR!MM's tape echo and granular cloud (GRiMMBIENT/grimm v2.0.2, firmware/src/tape.c,
 * grain.c, wear.c), in integers as GR!MM has them. Included by fumi.c after tapefx.c (its wear_tz, wear_sat,
 * tf_softclip, tf_clamp, tf_mulq15). GR!MM's DELAY follows X0X's TAPE delay (charlesvestal/fm1-x0x, GPL-3.0, from
 * 9W9's er99_dly_tick); its GRAIN is Felucca's.
 *   DELAY   at half the rate: the send -> the line -> read TIME back, with wow and flutter, the right side 9 ms
 *           later -> the loop: a low-pass (WEAR), FEEDBACK (0 .. 110 %), a tanh saturation, a DC block -> back into
 *           the line with the input through a soft knee. TIME glides (the pitch bends, as a tape's).
 *   GRAIN   up to 8 grains read from the ring, each a Hann window, at its own place behind the newest sample and
 *           its own pan: SIZE, DENSITY, SCATTER, MIX; FREEZE stops the recording, the grains go on from what is held.
 * Crawl's:
 *   - one line for both. The pool cannot hold GR!MM's 155 KB line and its 55 KB ring: DELAY's line is 1.4 s
 *     (62 KB; TIME's table x 0.4, 8 .. 1400 ms) and GRAIN reads that same line as its ring. So the cloud is made
 *     of what was sent and of its echoes, the line records while either is up, and FREEZE holds the line for
 *     both: the echoes fade out (a period, 6 ms) while it is held and come back when it is let go.
 *   - the return's level ramps over a period (GR!MM: a step), for that fade.
 *   - no REV bus: fumi.c sends the returns into its reverb itself.
 *   - GRAIN's lengths come from the main loop in float (fm_grain), not from GR!MM's pitch table.
 *   - both work in GR!MM's blocks of 256 frames (fx_period below): fumi.c's blocks of 32 fill a period's send and
 *     play the last period's returns, so the returns are one period (5.8 ms) late.
 * Off (both MIX 0, or the line silent with nothing sent): nothing runs. */

static volatile int32_t ge_mix;                 /* GRAIN's MIX and FREEZE (below): DELAY looks at them, since GRAIN
                                                 * reads its line */
static volatile uint32_t ge_freeze;

/* TIME's table, ms (GR!MM's tools/gen_tables.py: 20 .. 3500, Crawl uses it x 0.4) */
static const uint16_t TD_MS[128] = {
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    33, 34, 35, 37, 38, 40, 42, 43, 45, 47, 49, 51,
    53, 55, 58, 60, 62, 65, 68, 71, 73, 77, 80, 83,
    86, 90, 94, 98, 102, 106, 110, 115, 120, 125, 130, 135,
    141, 147, 153, 159, 166, 173, 180, 187, 195, 203, 212, 220,
    229, 239, 249, 259, 270, 281, 293, 305, 318, 331, 345, 359,
    374, 389, 406, 422, 440, 458, 477, 497, 518, 539, 561, 585,
    609, 634, 661, 688, 717, 746, 777, 810, 843, 878, 915, 953,
    992, 1033, 1076, 1121, 1167, 1216, 1266, 1319, 1374, 1431, 1490, 1552,
    1616, 1683, 1753, 1826, 1902, 1981, 2063, 2148, 2238, 2331, 2427, 2528,
    2633, 2742, 2856, 2975, 3098, 3227, 3361, 3500,
};

/* the wow's sine: a full cycle over 2^32 -> -32767..32767 (a refined parabola, ~0.1 %: Dinghy's) */
static int32_t wear_sine(uint32_t ph)
{
    int32_t x = (int32_t)ph >> 16, ax = x < 0 ? -x : x, y, ay;
    y = (x * (32768 - ax)) >> 13;
    ay = y < 0 ? -y : y;
    y += ((((y * ay) >> 15) - y) * 7373) >> 15;
    return y > 32767 ? 32767 : y < -32767 ? -32767 : y;
}
/* the wow and flutter at phases p1 (0.5 Hz), p2 (0.19 Hz), p3 (6.1 Hz), reach a: about +-0.9 a */
static inline int32_t wear_wow(uint32_t p1, uint32_t p2, uint32_t p3, int32_t a)
{
    return (((wear_sine(p1) + tf_mulq15(wear_sine(p2), 19661)) >> 1) * a + tf_mulq15(wear_sine(p3), 3277) * a) >> 15;
}
/* linear up to k, then a soft knee: only the peaks above k saturate (|y| - k must stay < 2^30) */
static inline int32_t soft_knee(int32_t y, int32_t k)
{
    int32_t a = y < 0 ? -y : y;
    if (a <= k)
        return y;
    a = k + (tf_softclip((a - k) * 2) >> 1);
    return y < 0 ? -a : a;
}


/* ------------------------------------------------------------ DELAY (GR!MM's tape.c) --- */
#define TD_NC 128u                              /* a block at half the rate: app_audio's 256 frames */
#define TD_LEN 31122u                           /* Crawl: 1400 ms (30870 samples) + the wow's reach + the right
                                                 * side's 9 ms (GR!MM: 3500 ms, 77456: the pool has not that) */
#define TD_SPREAD (198 << 8)                    /* the right side reads 9 ms further back (Q8 samples) */
#define TD_IN 8192                              /* the voices into the line, Q15: a quarter (headroom for the loop; V2:
                                                 * half of 1.0's, its return twice: loud chords never reach its knee) */
#define TD_DCK 280                              /* the DC block: 30 Hz at 22.05 kHz, Q15 */
#define TD_W1 12467522u                         /* the wow: 0.5 Hz, a block's step (2^32 * 256 / 44100 * Hz) */
#define TD_W2 4737658u                          /* .. and 0.19 Hz beside it: not quite periodic */
#define TD_W3 152103765u                        /* the flutter: 6.1 Hz */
_Static_assert(30870u + 51u + 198u + 2u < TD_LEN, "DELAY: the longest read stays in the line");

static int16_t td_buf[TD_LEN] FM_POOL;
static struct {
    uint32_t w, quiet;                          /* the write index; half-rate samples with silence in and written */
    uint32_t p1, p2, p3;                        /* the wow's and the flutter's phases */
    int32_t d, m;                               /* TIME gliding to its target, the wow at the block's end (Q8 samples) */
    int32_t wg;                                 /* Crawl: the return's level at the last period's end */
    int32_t x, lp, le, dc, de, ml, mr;          /* the input's last sample; the loop's low-pass and DC block (and
                                                 * their steps' remainders); the last wet out, left / right */
    int32_t l[TD_NC], r[TD_NC];                 /* the block's echoes at half the rate */
} td FM_POOL;

/* the parameters, 0..127, written by the main loop */
static volatile int32_t td_time = 88, td_fb = 64, td_wear = 50, td_mix;   /* (Crawl: MIX 0 until a scene asks) */

/* main loop: the delay gives nothing out (MIX 0, or the line silent and nothing coming in) */
static int tape_idle(void) { return td_mix <= 0 || td.quiet >= TD_LEN; }

/* a read d (Q8 samples) behind the write index w, between samples */
static inline int32_t tape_read(uint32_t w, int32_t d)
{
    int32_t i0 = (int32_t)w - (d >> 8), f = d & 255, b0, b1;
    i0 = i0 < 0 ? i0 + (int32_t)TD_LEN : i0;
    b0 = td_buf[i0];
    b1 = td_buf[i0 ? i0 - 1 : (int32_t)TD_LEN - 1];   /* one further back */
    return b0 + (((b1 - b0) * f + 128) >> 8);
}

/* the audio ISR: n (= 2 TD_NC) frames of the DLY bus in (mono) -> the echoes added to out (L R) and to the REV bus */
static void tape_render(const int32_t *in, int32_t *out, uint32_t n)
{
    uint32_t i, any = 0, w;
    int32_t mix = td_mix, wear = td_wear, fbk = td_fb;
    int32_t target, d0, d1, m1, a, tc, g, ig, fb;
    int32_t xp = td.x, lp = td.lp, le = td.le, dc = td.dc, de = td.de, ml = td.ml, mr = td.mr;
    int32_t wg1;
    if (n != 2u * TD_NC || (mix <= 0 && ge_mix <= 0 && !td.wg))
        return;                                 /* MIX 0 (and GRAIN's, which reads this line): the line waits */
    wg1 = ge_freeze || mix <= 0 ? 0 : mix * 258;   /* (Crawl: FREEZE holds the line, the echoes fade out) */
    if (ge_freeze && !td.wg)
        return;                                 /* held, and faded: nothing to do */
    for (i = 0; i < 2u * TD_NC && !any; i++)
        any = in[i] != 0;
    if (!any && td.quiet >= TD_LEN)
        return;                                 /* the line silent, nothing coming in: nothing to do */
    wear = tf_clamp(wear, 0, 127);
    fbk = tf_clamp(fbk, 0, 127);

    /* this block's settings: TIME (gliding), the wow, the loop's tone, drive and feedback */
    target = (int32_t)(TD_MS[tf_clamp(td_time, 0, 127)] * 2u / 5u) * 5645;   /* ms (Crawl: x 0.4, 8 .. 1400) -> Q8 samples
                                                 * at 22.05 kHz */
    if (!td.d)
        td.d = target;                          /* (the first block: no glide from nothing) */
    d0 = td.d;
    d1 = d0 + ((target - d0) >> 3);             /* ~50 ms to a new TIME */
    td.d = d1;
    td.p1 += TD_W1, td.p2 += TD_W2, td.p3 += TD_W3;
    a = (10 + wear) * 94;                       /* the wow's reach, Q8 samples: ~0.17 .. ~2.3 ms */
    m1 = wear_wow(td.p1, td.p2, td.p3, a);      /* (wear.c) */
    tc = 28300 - wear * 148;                    /* the loop's low-pass, Q15: ~7 kHz .. ~1.2 kHz */
    g = 4096 + wear * 26;                       /* the saturation's drive, Q12: 1x .. 1.8x */
    ig = (int32_t)(134217728u / (uint32_t)g) >> 1;   /* .. and the level made up after it, Q15 (V2: the line at half:
                                                 * driven twice as hard, half back: saturating where it did) */
    g *= 2;
    fb = fbk * 284;                             /* FEEDBACK, Q15: 0 .. 1.1 */

    w = td.w;
    {
        int32_t D0 = d0 + td.m, dD = (d1 + m1) - D0;
        for (i = 0; i < TD_NC; i++) {
            int32_t x = (xp + 2 * in[2u * i] + in[2u * i + 1u]) >> 2, D = D0 + ((dD * (int32_t)i) >> 7);
            int32_t y = tape_read(w, D), v, e;
            xp = in[2u * i + 1u];
            x = tf_mulq15(tf_clamp(x, -65535, 65535), TD_IN);
            td.l[i] = y;
            td.r[i] = tape_read(w, D + TD_SPREAD);
            e = (y - lp) * tc + le, le = e & 32767, lp += e >> 15;          /* darker each time round */
            v = wear_tz(lp * fb, 15);                                         /* FEEDBACK */
            v = wear_sat(v, g, ig);                                         /* the tape's saturation (wear.c) */
            e = (v - dc) * TD_DCK + de, de = e & 32767, dc += e >> 15;      /* the DC block */
            v = tf_clamp(soft_knee(x + v - dc, 16384), -32767, 32767);         /* in with the input: never clips */
            if (!ge_freeze) {                   /* (Crawl: FREEZE: the line as it is, read on while it fades) */
                td_buf[w] = (int16_t)v;
                td.quiet = (v | x) ? 0u : td.quiet + 1u;
                if (++w == TD_LEN) w = 0;
            }
        }
    }
    td.w = w, td.m = m1, td.x = xp, td.lp = lp, td.le = le, td.dc = dc, td.de = de;

    /* back to 44.1 kHz, between samples, at the return's level (MIX), into out and into the REV bus */
    {
        int32_t wg0 = td.wg;
        td.wg = wg1;
        for (i = 0; i < TD_NC; i++) {
            int32_t wg = wg0 + (((wg1 - wg0) * (int32_t)(i + 1u)) >> 7);   /* (Crawl: the level ramped) */
            int32_t wl = td.l[i], wr = td.r[i], *o = out + 4u * i, a = 2 * tf_mulq15((ml + wl) >> 1, wg),
                    b = 2 * tf_mulq15((mr + wr) >> 1, wg), c = 2 * tf_mulq15(wl, wg), d = 2 * tf_mulq15(wr, wg);   /* (V2: x 2) */
            o[0] += a, o[1] += b, o[2] += c, o[3] += d;
            ml = wl;
            mr = wr;
        }
    }
    td.ml = ml, td.mr = mr;
}

/* ------------------------------------------------------------ GRAIN (GR!MM's grain.c) --- */
#define GE_LEN TD_LEN                           /* Crawl: the ring is DELAY's line (GR!MM: its own, 1.25 s) */
#define ge_buf td_buf
#define GE_NC 128u                              /* a block at half the rate: app_audio's 256 frames */
#define GE_NG 8u                                /* grains at once */

/* sin^2(pi i / 256), Q15: the grain window */
static const int16_t GR_HANN[257] = {
    0, 5, 20, 44, 79, 123, 177, 241, 315, 398, 491, 593, 705, 827, 958, 1098,
    1247, 1406, 1573, 1749, 1935, 2128, 2331, 2542, 2761, 2989, 3224, 3468, 3719, 3978, 4244, 4518,
    4799, 5086, 5381, 5682, 5990, 6304, 6624, 6950, 7281, 7618, 7961, 8308, 8660, 9017, 9379, 9744,
    10114, 10487, 10864, 11244, 11628, 12014, 12403, 12794, 13187, 13583, 13980, 14378, 14778, 15178, 15580, 15981,
    16383, 16786, 17187, 17589, 17989, 18389, 18787, 19184, 19580, 19973, 20364, 20753, 21139, 21523, 21903, 22280,
    22653, 23023, 23388, 23750, 24107, 24459, 24806, 25149, 25486, 25817, 26143, 26463, 26777, 27085, 27386, 27681,
    27968, 28249, 28523, 28789, 29048, 29299, 29543, 29778, 30006, 30225, 30436, 30639, 30832, 31018, 31194, 31361,
    31520, 31669, 31809, 31940, 32062, 32174, 32276, 32369, 32452, 32526, 32590, 32644, 32688, 32723, 32747, 32762,
    32767, 32762, 32747, 32723, 32688, 32644, 32590, 32526, 32452, 32369, 32276, 32174, 32062, 31940, 31809, 31669,
    31520, 31361, 31194, 31018, 30832, 30639, 30436, 30225, 30006, 29778, 29543, 29299, 29048, 28789, 28523, 28249,
    27968, 27681, 27386, 27085, 26777, 26463, 26143, 25817, 25486, 25149, 24806, 24459, 24107, 23750, 23388, 23023,
    22653, 22280, 21903, 21523, 21139, 20753, 20364, 19973, 19580, 19184, 18787, 18389, 17989, 17589, 17187, 16786,
    16384, 15981, 15580, 15178, 14778, 14378, 13980, 13583, 13187, 12794, 12403, 12014, 11628, 11244, 10864, 10487,
    10114, 9744, 9379, 9017, 8660, 8308, 7961, 7618, 7281, 6950, 6624, 6304, 5990, 5682, 5381, 5086,
    4799, 4518, 4244, 3978, 3719, 3468, 3224, 2989, 2761, 2542, 2331, 2128, 1935, 1749, 1573, 1406,
    1247, 1098, 958, 827, 705, 593, 491, 398, 315, 241, 177, 123, 79, 44, 20, 5,
    0,
};

/* grain level by overlap (grains sounding at once on average): 1 / sqrt(n), Q15 (Felucca's, to GE_NG) */
static const int16_t GR_NORM[GE_NG + 1] = {32767, 32767, 23170, 18918, 16384, 14654, 13377, 12385, 11585};

typedef struct {
    uint32_t pos;                               /* the next sample it reads in the ring */
    uint32_t wph, winc;                         /* window phase: one turn = the grain */
    uint32_t left, wait;                        /* half-rate samples to go; to wait before it starts (this block) */
    int32_t gl, gr;                             /* its level by overlap, panned (Q15) */
} ge_grain_t;
static struct {
    ge_grain_t g[GE_NG];
    uint32_t on;                                /* bit k: grain k sounds */
    uint32_t w, quiet;                          /* the write index; silent samples written in a row */
    int32_t cd;                                 /* half-rate samples to the next grain */
    int32_t x, ml, mr;                          /* the input's last sample; the last wet out, left / right */
    int32_t l[GE_NC], r[GE_NC];                 /* the block's cloud at half the rate */
} ge FM_POOL;
static uint32_t gr_rng = 0x2545F491u;

/* the parameters, 0..127, and FREEZE, written by the main loop */
static volatile int32_t ge_size = 80, ge_dens = 80, ge_scat = 30, ge_mix;   /* (Crawl: MIX 0 until a scene asks) */
static volatile uint32_t ge_freeze;

static inline uint32_t gr_rnd(void)             /* xorshift32 */
{
    uint32_t s = gr_rng;
    s ^= s << 13;
    s ^= s >> 17;
    s ^= s << 5;
    return gr_rng = s;
}
static inline uint32_t gr_scale(uint32_t n, uint32_t f16) { return (n >> 16) * f16 + (((n & 0xFFFFu) * f16) >> 16); }
/* Crawl: the length and the interval come from the main loop (fm_grain: 441 x 2^(SIZE / 24) and
 * 44100 x 2^(-DENSITY x 10 / 192), 44.1 kHz samples, as GR!MM's gr_len and gr_iv make them from its pitch table) */
static volatile uint32_t ge_len44 = 4445u, ge_iv44 = 2456u;
static uint32_t gr_len(int32_t size) { (void)size; return ge_len44; }
static uint32_t gr_iv(int32_t dens) { (void)dens; return ge_iv44; }
static inline int32_t gr_win(uint32_t wph)      /* the Hann window, Q15 */
{
    uint32_t wi = wph >> 24;
    return GR_HANN[wi] + (((GR_HANN[wi + 1u] - GR_HANN[wi]) * (int32_t)((wph >> 9) & 0x7FFFu)) >> 15);
}

/* main loop: the cloud gives nothing out (MIX 0, or the ring silent) */
static int grain_idle(void) { return ge_mix <= 0 || td.quiet >= GE_LEN; }

/* a grain, starting `off` samples into this block (write index w0 at the block's start) */
static void ge_spawn(uint32_t off, uint32_t w0, uint32_t iv, int32_t sc)
{
    uint32_t k, len, span, d, ov;
    int32_t pan, gain;
    ge_grain_t *g;
    for (k = 0; k < GE_NG && (ge.on >> k & 1u); k++)
        ;
    if (k == GE_NG)
        return;                                 /* all GE_NG sound: this one is left out */
    g = &ge.g[k];
    len = gr_len(ge_size) >> 1;                 /* (at half the rate) */
    len = len < 16u ? 16u : len;
    /* where: len + 1 .. the whole ring behind the newest sample (as the ring records, the grain stays as far behind; a
     * frozen one never reaches the newest sample, nor a recording one the samples written over in this block) */
    span = GE_LEN - GE_NC - 2u - len;
    d = len + 1u + gr_scale(gr_scale(span, (uint32_t)sc * 516u), gr_rnd() & 0xFFFFu);
    w0 += ge_freeze ? 0u : off;
    g->pos = w0 >= d ? w0 - d : w0 + GE_LEN - d;
    g->wph = 0;
    g->winc = 0xFFFFFFFFu / len;
    g->left = len;
    g->wait = off;
    ov = len / (iv ? iv : 1u);
    gain = GR_NORM[ov < 1u ? 1u : ov > GE_NG ? GE_NG : ov];
    pan = (((int32_t)(gr_rnd() & 0xFFFFu) - 32768) * sc) / 127;   /* +-SCATTER: from the middle to the sides */
    g->gl = tf_mulq15(gain, tf_mulq15(23170, 32768 - pan));
    g->gr = tf_mulq15(gain, tf_mulq15(23170, 32768 + pan));
    ge.on |= 1u << k;
}

/* one grain into the block's cloud (Felucca's gr_run: the window over the grain; here a ring read at the source's rate) */
static void ge_run(ge_grain_t *g, uint32_t k)
{
    uint32_t i = g->wait, pos = g->pos, wph = g->wph, winc = g->winc, end = i + g->left;
    int32_t gl = g->gl, gr = g->gr;
    end = end < GE_NC ? end : GE_NC;
    g->left -= end - i;
    g->wait = 0;
    while (i < end) {                           /* in runs up to the ring's end */
        uint32_t m = end - i < GE_LEN - pos ? end - i : GE_LEN - pos, j;
        const int16_t *s = ge_buf + pos;
        for (j = 0; j < m; j++, i++, wph += winc) {
            int32_t y = (s[j] * gr_win(wph)) >> 15;
            ge.l[i] += (y * gl) >> 15;
            ge.r[i] += (y * gr) >> 15;
        }
        pos += m;
        pos = pos == GE_LEN ? 0u : pos;
    }
    g->pos = pos;
    g->wph = wph;
    if (!g->left)
        ge.on &= ~(1u << k);
}

/* the audio ISR: n (= 2 GE_NC) frames of the GRAIN bus in (mono) -> the cloud added to out (L R) and to the REV bus */
static void grain_render(const int32_t *in, int32_t *out, uint32_t n)
{
    uint32_t i, k, any = 0, w0 = td.w, iv;      /* (Crawl: DELAY's write index; it records after this) */
    int32_t mix = ge_mix, sc = tf_clamp(ge_scat, 0, 127), ml = ge.ml, mr = ge.mr;
    if (n != 2u * GE_NC || mix <= 0)
        return;                                 /* MIX 0: the ring waits as it is */
    for (i = 0; i < 2u * GE_NC && !any; i++)
        any = in[i] != 0;
    if (!any && td.quiet >= GE_LEN) {           /* nothing sent, the ring silent: nothing to do */
        ge.on = 0;
        return;
    }
    /* the grains due in this block: DENSITY a second, SCATTER jitters the interval (eng_grain.c) */
    iv = gr_iv(ge_dens) >> 1;
    iv = iv ? iv : 1u;
    for (k = 0; k < 4u && ge.cd < (int32_t)GE_NC; k++) {
        int32_t j = (((int32_t)(gr_rnd() & 0xFFFFu) - 32768) * sc) / 254;   /* +-SCATTER / 2, Q15 */
        ge_spawn((uint32_t)(ge.cd > 0 ? ge.cd : 0), w0, iv, sc);
        ge.cd += (int32_t)iv + tf_mulq15((int32_t)iv, j);
    }
    ge.cd = ge.cd - (int32_t)GE_NC > 0 ? ge.cd - (int32_t)GE_NC : 0;

    for (i = 0; i < GE_NC; i++)
        ge.l[i] = ge.r[i] = 0;
    for (k = 0; k < GE_NG; k++)
        if (ge.on >> k & 1u)
            ge_run(&ge.g[k], k);

    /* back to 44.1 kHz, between samples, at the return's level (MIX), into out and into the REV bus (as DELAY) */
    {
        int32_t wg = mix * 258;
        for (i = 0; i < GE_NC; i++) {
            /* (Crawl: x 4, the line holds the bus at a quarter, TD_IN) */
            int32_t wl = tf_clamp(ge.l[i] * 4, -65535, 65535), wr = tf_clamp(ge.r[i] * 4, -65535, 65535), *o = out + 4u * i,
                    a = tf_mulq15((ml + wl) >> 1, wg),
                    b = tf_mulq15((mr + wr) >> 1, wg), c = tf_mulq15(wl, wg), d = tf_mulq15(wr, wg);
            o[0] += a, o[1] += b, o[2] += c, o[3] += d;
            ml = wl;
            mr = wr;
        }
    }
    ge.ml = ml, ge.mr = mr;
}

/* ---- Crawl: the period. fumi.c's frames in (the send, mono, +-1.0), the last period's returns out (L R, +-1.0) */
#define FX_PER 256u
static volatile float fx_send_target;         /* main loop: the send into the line, 0 .. 1 (fumi.c glides to it) */
static struct {
    int32_t in[FX_PER], out[2u * FX_PER];       /* this period's send (Q15); the returns playing (Q15) */
    uint32_t i, live;                           /* the frame in the period; anything sounding or sent */
} fxp FM_POOL;

static void echofx_init(void)
{
    uint32_t k;
    for (k = 0; k < sizeof fxp; k++)
        ((uint8_t *)&fxp)[k] = 0;
    for (k = 0; k < sizeof td; k++)
        ((uint8_t *)&td)[k] = 0;
    for (k = 0; k < sizeof ge; k++)
        ((uint8_t *)&ge)[k] = 0;
    for (k = 0; k < TD_LEN; k++)
        td_buf[k] = 0;
    td.quiet = TD_LEN;
    td_time = 88, td_fb = 64, td_wear = 50, td_mix = 0;
    ge_size = 80, ge_dens = 80, ge_scat = 30, ge_mix = 0, ge_freeze = 0;
    gr_rng = 0x2545F491u;
}
/* a frame: send (+-1.0) in; the returns added to l and r. 1 while there is anything to do */
static inline void echofx_frame(float send, float *l, float *r)
{
    uint32_t i = fxp.i;
    float x = send * 32768.0f;
    fxp.in[i] = (int32_t)(x > 65535.0f ? 65535.0f : x < -65535.0f ? -65535.0f : x);
    *l += (float)fxp.out[2u * i] * (1.0f / 32768.0f);
    *r += (float)fxp.out[2u * i + 1u] * (1.0f / 32768.0f);
    if (++i == FX_PER) {                        /* a period is in: its returns, for the next */
        uint32_t k, any = 0;
        for (k = 0; k < 2u * FX_PER; k++)
            fxp.out[k] = 0;
        grain_render(fxp.in, fxp.out, FX_PER);  /* (GR!MM's order: GRAIN before DELAY) */
        tape_render(fxp.in, fxp.out, FX_PER);
        for (k = 0; k < 2u * FX_PER && !any; k++)
            any = fxp.out[k] != 0;
        fxp.live = any || ((td_mix > 0 || ge_mix > 0) && td.quiet < TD_LEN);   /* (the line silent: done) */
        i = 0;
    }
    fxp.i = i;
}
