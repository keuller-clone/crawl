/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 GR!MM
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments (tf_softclip and TANH_Q15: Felucca's dsp.c
 * and its tools/gen_tables.py table, as GR!MM has them)
 * Crawl changes: Copyright (C) 2026 the Crawl contributors */
/* Crawl's TAPE: GR!MM's TAPE effect (GRiMMBIENT/grimm v2.0.2, firmware/src/tapefx.c and wear.c), a worn tape, on
 * Crawl's mix. Included by fumi.c. GR!MM's own design, from its DELAY's wear (after X0X's TAPE delay); the integer
 * pieces are Felucca's. Taken over as they are, in integers: the SATURATION curve (tf_make, tf_shape, wear_sat and
 * the tanh), the WOW's warbles (GR!MM's tape_wow, here tw_wow, and tw_step: the draws, the timing, the knob's reach), TONE's low-pass, HISS.
 *   WOW         a worn motor, a slipping belt: the pitch steady, and now and then (3 .. 6.5 s apart) a warble, short
 *               (0.3 .. 1.5 s), a random walk of 1 .. 4 steps, mostly flat, then back to exactly 0. The knob is how
 *               far the steps may go (the top: ~136 cents), never how often. WOW 0: no bend at all.
 *   SATURATION  a tanh, driven 1x .. 4x, the level made up after it, blended in with the knob: 0 = clean. One
 *               curve, made by the main loop when the knob moves (tapefx_poll), crossed over to from the last
 *               within 256 frames: no step in the sound, and nothing heavy in the audio interrupt.
 *   TONE        a one-pole low-pass, darker as it turns: open .. ~1.8 kHz.
 *   HISS        noise (a draw every two frames), under the low-pass: up to ~ -41 dB rms.
 * Crawl's:
 *   - on the mix, both sides, after the reverb (GR!MM V2 has it on each track; its V1 had it on the mix as here).
 *     fm_tape's amount (0 .. 100 %) is how far the mix goes through the tape: a crossfade from the mix as it is
 *     to it taped, never the two added, as GR!MM's TAPE tile. At 0 nothing runs and the mix is untouched.
 *   - the mix is float, +-1.0 full scale: a frame goes to Q15 for the curve and the low-pass, and back.
 *   - GR!MM works in blocks of 256 frames; fumi.c's block is 32. The tape keeps GR!MM's period of 256 frames
 *     (tf.i counts them): the curve's crossover, HISS's ramp and a step of the WOW each take one period, so the
 *     timings are GR!MM's. The WOW reaches the scene's notes through fumi.c's tape_wow (cents), a block at a time.
 *   - the SATURATION curve is kept for x >= 0 only and mirrored (it is odd): half the memory. A negative x can
 *     come out one step of the last bit from GR!MM's.
 *   - the warbles' draws are stirred with the time and the voices sounding (GR!MM: its voices' notes).
 * The main loop sets the four knobs (fm_tape_knobs) and calls fm_tape_poll; the audio side calls tape_control a
 * block and tape_frame a frame. */

#define TF_NC 768u                              /* SATURATION's curve: x -98304 .. 98304 in steps of 256 */
#define TF_CQ 2                                 /* its values kept >> TF_CQ (int16: |y| <= |x| <= 98304, so <= 24576) */
#define TF_PER 256u                             /* GR!MM's block, frames: the tape's period */
#define TF_CENT_Q8 606.0f                       /* the WOW's units a cent (GR!MM's tune_f: 2.366 a cent, Q8) */

static const int16_t TANH_Q15[257] = {
    0, 255, 511, 767, 1023, 1279, 1534, 1790, 2045, 2300, 2554, 2809,
    3062, 3316, 3569, 3822, 4074, 4326, 4577, 4828, 5078, 5328, 5577, 5825,
    6072, 6319, 6565, 6811, 7055, 7299, 7542, 7784, 8025, 8265, 8504, 8742,
    8980, 9216, 9451, 9685, 9918, 10150, 10381, 10611, 10840, 11067, 11293, 11518,
    11742, 11964, 12186, 12405, 12624, 12841, 13057, 13272, 13485, 13697, 13908, 14117,
    14325, 14531, 14736, 14940, 15142, 15342, 15541, 15739, 15935, 16130, 16323, 16515,
    16705, 16894, 17081, 17267, 17451, 17633, 17814, 17994, 18172, 18349, 18524, 18697,
    18869, 19039, 19208, 19375, 19541, 19705, 19868, 20029, 20188, 20346, 20503, 20658,
    20811, 20963, 21114, 21263, 21410, 21556, 21701, 21844, 21985, 22125, 22264, 22401,
    22536, 22670, 22803, 22935, 23064, 23193, 23320, 23445, 23570, 23693, 23814, 23934,
    24053, 24170, 24286, 24401, 24514, 24626, 24737, 24847, 24955, 25062, 25167, 25271,
    25375, 25476, 25577, 25676, 25775, 25872, 25967, 26062, 26156, 26248, 26339, 26429,
    26518, 26606, 26692, 26778, 26862, 26946, 27028, 27109, 27190, 27269, 27347, 27424,
    27500, 27576, 27650, 27723, 27795, 27867, 27937, 28006, 28075, 28142, 28209, 28275,
    28340, 28404, 28467, 28529, 28591, 28652, 28712, 28771, 28829, 28886, 28943, 28999,
    29054, 29108, 29162, 29215, 29267, 29318, 29369, 29419, 29468, 29517, 29565, 29612,
    29658, 29704, 29750, 29794, 29838, 29882, 29925, 29967, 30008, 30049, 30090, 30130,
    30169, 30208, 30246, 30283, 30320, 30357, 30393, 30428, 30463, 30498, 30532, 30565,
    30598, 30631, 30663, 30694, 30726, 30756, 30786, 30816, 30846, 30874, 30903, 30931,
    30959, 30986, 31013, 31039, 31065, 31091, 31116, 31141, 31166, 31190, 31214, 31238,
    31261, 31284, 31306, 31328, 31350, 31371, 31393, 31413, 31434, 31454, 31474, 31494,
    31513, 31532, 31551, 31570, 31588,
};

static int16_t tf_curve[2][TF_NC / 2u + 1u] FM_POOL;   /* SATURATION as one curve: drive, tanh, make-up, blend
                                                 * (tf_make): the one in use (tf.ci) and the next / the last. Crawl:
                                                 * x >= 0 only (the curve is odd; GR!MM keeps both halves): 1540 B
                                                 * of pool instead of 3076, which the pool does not have */
static struct {
    uint32_t rng, fade, pub, i, on, live;       /* the hiss; a new curve crossed over to this period, its tf_pub; the
                                                 * frame in the period; the low-pass running; anything to do */
    int32_t lp[2], hlp[2], h0[2], csat, ci;     /* TONE's low-pass on the mix, on the hiss, L R; the last draws; the
                                                 * SATURATION the curve in use is for, and which one it is */
    int32_t hl, hn, hg, hdg;                    /* HISS's level, the last period's end, this one's; its ramp */
    int32_t wow_a, wow_b;                       /* the bend at this period's start and end (Q8 of GR!MM's units) */
    float a0, da;                               /* how far through the tape, at the block's start; a frame's step */
} tf;

/* the parameters, 0..127, written by the main loop */
static volatile int32_t tf_wow, tf_sat, tf_tone, tf_hiss;
static volatile uint32_t tf_pub, tf_ack;        /* main loop: + 1, the other curve made (for tf_psat); audio: = tf_pub,
                                                 * crossed over to it */
static volatile int32_t tf_psat;

static inline int32_t tf_clamp(int32_t x, int32_t lo, int32_t hi) { return x < lo ? lo : x > hi ? hi : x; }
static inline int32_t tf_mulq15(int32_t a, int32_t b) { return (a * b) >> 15; }
/* Felucca's softclip: a tanh from its table, continuous past it */
static inline int32_t tf_softclip(int32_t x)
{
    int32_t a = x < 0 ? -x : x, i = a >> 8, y;
    if (i >= 256)
        y = TANH_Q15[256];
    else
        y = TANH_Q15[i] + (((TANH_Q15[i + 1] - TANH_Q15[i]) * (a & 255)) >> 8);
    return x < 0 ? -y : y;
}
/* m / 2^sh rounded towards zero (Felucca's sh_tz: a product that feeds back must not floor) */
static inline int32_t wear_tz(int32_t m, uint32_t sh) { return (m + ((m >> 31) & ((1 << sh) - 1))) >> sh; }
/* the saturation: v driven by g (Q12, 4096 = 1x) through the tanh (wear_drive), made up by ig (Q15, 2^27 / g) */
static inline int32_t wear_drive(int32_t v, int32_t g) { return tf_softclip(wear_tz(v * g, 12)); }
static inline int32_t wear_sat(int32_t v, int32_t g, int32_t ig) { return wear_tz(wear_drive(v, g) * ig, 15); }

/* WOW: the warbles, a period on: tf.wow_b, the bend at this period's end; its start the last one's end. No shapes
 * and no oscillator: a warble a random walk, steps straight to a target drawn each time (tw.g, Q15 of the knob's
 * most), quick or slow (tw.m periods), sometimes held there (tw.h), then the next (tw.n left, the last one back to
 * exactly 0), all in the warble's time (tw.q). The draws never depend on the knob (it only scales them: the same
 * warbles, the same moments, at every WOW) */
#define TW_BPS 172                              /* periods a second (44100 / 256) */
static struct {
    uint32_t rng;                               /* the draws */
    int32_t gap, q, t, n, m, h, r, s, g, f;     /* periods still steady before the next (-1: draw them); the warble's
                                                 * periods left; the step's period (-1: none), the steps left, its
                                                 * move's periods, held, the way back's; from, to, now (Q15 of the
                                                 * most, - flat) */
} tw;
static uint32_t tw_rnd(uint32_t n) { tw.rng = tw.rng * 1664525u + 1013904223u; return (tw.rng >> 8) % n; }
static int32_t tw_move(void) { return tw_rnd(2) ? 4 + (int32_t)tw_rnd(4) : 52 + (int32_t)tw_rnd(52); }   /* (a jerk, 23
                                                 * .. 40 ms; or a sag, 300 .. 600 ms) */
static void tw_step(int32_t back)               /* the next step: a target (or 0), how fast, how long held there; within
                                                 * the warble's time (the way back's kept), a jerk kept for each target
                                                 * after it, the last held for what is left */
{
    int32_t m = tw_move(), x;
    tw.s = tw.f, tw.t = 0;
    if (back) {
        tw.g = 0, tw.h = 0, tw.m = tw.r;
        return;
    }                                           /* (leaning flat, a motor losing power: 2 in 3 flat, 0.2 .. 1 of the
                                                 * most; or sharp, 0.2 .. 0.7) */
    tw.g = tw_rnd(3) ? -(6554 + (int32_t)tw_rnd(26214)) : 6554 + (int32_t)tw_rnd(16384);
    tw.h = tw_rnd(5) < 3u ? 9 + (int32_t)tw_rnd(112) : 0;   /* (3 in 5 held, 50 .. 700 ms) */
    x = tw.q - 4 * (tw.n - 1);
    tw.m = m < x ? m : x;
    if (tw.n == 1 || tw.h > x - tw.m) tw.h = x - tw.m;   /* (the last: held the rest) */
    tw.q -= tw.m + tw.h;
}
static void tw_wow(uint32_t stir)   /* (GR!MM's tape_wow) */
{
    int32_t w = tf_clamp(tf_wow, 0, 127), e;
    tf.wow_a = tf.wow_b;
    if (!w) {                                   /* (WOW 0: none, at once; the next gap drawn when it turns up) */
        tw.t = -1, tw.gap = -1, tw.f = 0, tf.wow_b = 0;
        return;
    }
    if (tw.t < 0) {                             /* steady: exactly 0 */
        if (tw.gap < 0)                         /* (3 .. 6.5 s, mostly near the middle: one every ~5.5 s, evenly) */
            tw.gap = 3 * TW_BPS + (int32_t)tw_rnd(301) + (int32_t)tw_rnd(301);
        if (tw.gap > 0) tw.gap--;
        else {                                  /* a warble: 1 .. 4 steps in 0.3 .. 1.5 s (the draws stirred with what
                                                 * plays, and when) */
            tw.rng ^= stir * 2654435761u;
            tw.n = 1 + (int32_t)tw_rnd(4), tw.q = 52 + (int32_t)tw_rnd(207), tw.r = tw_move();
            if (tw.r > tw.q / 3) tw.r = tw.q / 3;   /* (the way back: a jerk or a sag, a third of it at most) */
            tw.q -= tw.r;
            tw_step(0);
        }
        if (tw.t < 0) {
            tf.wow_b = 0;
            return;
        }
    }
    e = ++tw.t;                                 /* the step: straight there, then held */
    tw.f = e < tw.m ? tw.s + (tw.g - tw.s) * e / tw.m : tw.g;
    if (e >= tw.m + tw.h) {
        if (tw.n > 0) tw_step(--tw.n == 0);     /* (the next; after the last target, back to 0) */
        else {                                  /* (back at 0: done, steady again) */
            tw.t = -1, tw.gap = -1, tw.f = 0, tf.wow_b = 0;
            return;
        }
    }
    tf.wow_b = (int32_t)(((int64_t)(82416 * w / 127) * tw.f) >> 15);   /* (the knob: the most, straight, to 136 cents) */
}

/* a SATURATION curve into c: the tanh, driven 1x .. 4x, made up, blended in with the knob (0: x itself) */
static void tf_make(int16_t *c, int32_t sat)
{
    int32_t g = 4096 + sat * 97, ig = (int32_t)(134217728u / (uint32_t)g), sw = sat * 129, x, y;   /* sw: Q14 */
    uint32_t j;
    for (j = 0; j <= TF_NC / 2u; j++) {         /* (odd: one half; Crawl keeps only it, tf_shape mirrors) */
        x = (int32_t)(j << 8);
        y = x + (((wear_sat(x, g, ig) - x) * sw) >> 14);
        c[j] = (int16_t)tf_clamp((y + (1 << (TF_CQ - 1))) >> TF_CQ, -32767, 32767);
    }
}
/* main loop: the curve for SATURATION as it is now, into the one not in use, once the last has been taken */
static void tapefx_poll(void)
{
    int32_t sat = tf_clamp(tf_sat, 0, 127);
    if (tf_ack != tf_pub || sat == tf_psat)
        return;
    tf_make(tf_curve[tf.ci ^ 1], sat);
    tf_psat = sat;
    tf_pub++;
}
static inline int32_t tf_shape(const int16_t *c, int32_t x)   /* x through curve c, between its points */
{
    int32_t a = tf_clamp(x < 0 ? -x : x, 0, (int32_t)(TF_NC << 7) - 1), i = a >> 8;   /* (Crawl: |x|, the half kept) */
    int32_t y0 = (int32_t)c[i] * (1 << TF_CQ), y1 = (int32_t)c[i + 1] * (1 << TF_CQ);
    int32_t y = y0 + (((y1 - y0) * (a & 255)) >> 8);
    return x < 0 ? -y : y;
}

static void tapefx_init(void)                   /* fm_init: the tape clean, the knobs at 0, the curve x itself */
{
    uint32_t k;
    tf_wow = tf_sat = tf_tone = tf_hiss = 0;
    tf_psat = 0;
    tf_pub = tf_ack = 0;
    for (k = 0; k < sizeof tf; k++)
        ((uint8_t *)&tf)[k] = 0;
    for (k = 0; k < sizeof tw; k++)
        ((uint8_t *)&tw)[k] = 0;
    tw.rng = 0x2545F491u, tw.gap = -1, tw.t = -1;
    tf_make(tf_curve[0], 0);
}

/* audio, a period's start: HISS's level by how far the mix is on the tape (a: Q15), the new SATURATION curve taken
 * (crossed over to this period, the old one let go the next); then WOW */
static void tape_begin(int32_t a, uint32_t stir)
{
    int32_t h = tf_clamp(tf_hiss, 0, 127);
    tf.hl = tf.hn;
    tf.hn = (((h * h) >> 5) * a) >> 15;         /* (not on tape, or HISS 0: none) */
    tf.hg = tf.hl << 8, tf.hdg = ((tf.hn - tf.hl) << 8) / (int32_t)TF_PER;
    if (tf.fade)
        tf_ack = tf.pub, tf.fade = 0;           /* (the last curve free for the next) */
    else if (tf_pub != tf_ack)
        tf.fade = 1, tf.pub = tf_pub, tf.ci ^= 1, tf.csat = tf_psat;
    tw_wow(stir);
}

/* audio, a block of n frames (fumi.c's, 32): amt0 .. amt1 how far the mix goes through the tape over it (0 .. 1).
 * Returns the WOW at this block, cents (0: none), for the scene's notes. Off (and nothing dying away): nothing runs,
 * and a curve waiting is taken at once */
static float tape_control(float amt0, float amt1, uint32_t n, uint32_t stir)
{
    int32_t w;
    if (amt0 <= 0.0f && amt1 <= 0.0f && !tf.hn && !tf.hl) {
        if (tf.live) {                          /* (let go: the next time from the start, clean) */
            tf.live = 0, tf.i = 0, tf.on = 0, tf.fade = 0, tf.wow_a = tf.wow_b = 0;
            tw.t = -1, tw.gap = -1, tw.f = 0;
            tf_ack = tf.pub;
        }
        if (tf_pub != tf_ack)
            tf.pub = tf_pub, tf.ci ^= 1, tf.csat = tf_psat, tf_ack = tf_pub;
        return 0.0f;
    }
    tf.live = 1;
    if (tf.i == 0)
        tape_begin((int32_t)(amt1 * 32768.0f), stir);
    tf.a0 = amt0, tf.da = (amt1 - amt0) / (float)n;
    w = tf.wow_a + (tf.wow_b - tf.wow_a) * (int32_t)(tf.i + n) / (int32_t)TF_PER;
    return amt1 * (float)w * (1.0f / TF_CENT_Q8);
}

/* audio, a frame (tf.live only): the mix's two sides through the tape as far as the block's amount: SATURATION (the
 * curve in use; a new one crossed over to in the period), TONE, crossfaded in from the mix as it is; then HISS */
static inline void tape_frame(float *l, float *r)
{
    int32_t tone = tf_clamp(tf_tone, 0, 127), tc = 32767 - ((tone * tone * 1606) >> 10);
    uint32_t i = tf.i, shape = tf.csat || tf.fade, c;
    float a = tf.a0 += tf.da, *x[2];
    x[0] = l, x[1] = r;
    if (a > 0.0f && (shape || tone)) {
        const int16_t *c1 = tf_curve[tf.ci], *c0 = tf_curve[tf.ci ^ 1];
        for (c = 0; c < 2u; c++) {
            float in = *x[c];
            int32_t y = (int32_t)((in > 3.0f ? 3.0f : in < -3.0f ? -3.0f : in) * 32768.0f);
            if (tf.fade) {                      /* (the last curve to the new one over the period) */
                int32_t o = tf_shape(c0, y);
                y = o + tf_mulq15(tf_clamp(tf_shape(c1, y) - o, -65535, 65535), (int32_t)i << 7);
            } else if (shape)
                y = tf_shape(c1, y);
            if (tone) {                         /* (the low-pass, Q15; from the sound when it starts) */
                if (!tf.on) tf.lp[c] = y;
                tf.lp[c] += (tf_clamp(y - tf.lp[c], -65535, 65535) * tc) >> 15;
                y = tf.lp[c];
            }
            *x[c] = in + ((float)y * (1.0f / 32768.0f) - in) * (a > 1.0f ? 1.0f : a);
        }
        if (tone) tf.on = 1;
    }
    if (!tone) tf.on = 0;
    if (tf.hl || tf.hn) {                       /* HISS, ramped from the last period's level, under TONE's low-pass */
        tf.hg += tf.hdg;
        if (!(i & 1u)) {                        /* (a draw a side for two frames: a little darker than white) */
            uint32_t rng = tf.rng ? tf.rng : 0x9E3779B9u;
            rng = rng * 1664525u + 1013904223u;
            tf.h0[0] = (((int32_t)rng >> 16) * (tf.hg >> 8)) >> 15;
            rng = rng * 1664525u + 1013904223u;
            tf.h0[1] = (((int32_t)rng >> 16) * (tf.hg >> 8)) >> 15;
            tf.rng = rng;
        }
        if (tone) {                             /* (TONE darkens it) */
            tf.hlp[0] += (tf_clamp(tf.h0[0] - tf.hlp[0], -65535, 65535) * tc) >> 15;
            tf.hlp[1] += (tf_clamp(tf.h0[1] - tf.hlp[1], -65535, 65535) * tc) >> 15;
        } else
            tf.hlp[0] = tf.h0[0], tf.hlp[1] = tf.h0[1];
        *l += (float)tf.hlp[0] * (1.0f / 32768.0f);
        *r += (float)tf.hlp[1] * (1.0f / 32768.0f);
    }
    tf.i = (i + 1u) & (TF_PER - 1u);
}
