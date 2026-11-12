/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 the Crawl contributors */
/* Frozen Lake ("Lake" on the panel's tab): a lake frozen over, seen from its shore. The ice runs from our feet to the
 * far shore in perspective, snow lying on it in wind-combed drifts; on the far shore a dark line of pines, and on a
 * headland to the right a keep, a square tower with a bell gable, one window. At the near shore reeds stand frozen
 * into the ice with snow on their heads, two mooring posts, and a rowboat frozen in. Cracks run across the ice, each
 * a dark line with a lit edge, and they grow: the ice is alive under the snow. Snow falls. What is heard is what is
 * seen:
 *
 *   the ice cracks           its own recorded crack (CC0), heard where it runs; a note (part 1) where it runs across;
 *                            by night a "pew", the fracture's whistle falling (part 3, a glide); the crack lengthens
 *                            and its newest length flashes
 *   the ice groans           the SAMPLE source (part 7): a recording of lake ice groaning, played as a tape loop
 *                            pitched to the chord's root, slow, long; the cracks' edges glow while it lasts
 *   the keep's bell          a keep's own bell, recorded across a valley at midnight (CC0), one stroke at a time, from
 *                            the keep; snow shaken from its roof; a swell answers it (part 4)
 *   snow                     silent, except a flake that lands on the near ice: a faint touch (part 3)
 *   a gust                   the wind in the reeds: a bowed string (part 5: PHYS, STRING), the root, rising with it;
 *                            the snow driven across in streaks
 *   meltwater                drips from the posts and the boat (a drop heard where it lands, a ring); a trickle at
 *                            the shore (the water bed); the pools hold the sky
 *   under them               the drone (part 2: a pad, or VOICE: a choir), the air (the synthesized breeze)
 *
 *   KNOB 1 Mood   2 Activity   3 Instrument (Drop, Rin, Bell, Mallet)   4 Space (reverb)
 *   Volume: Notes, Drone, Air, Bell.   Nature: Voices (Pad, Solo, Monks, Nuns, Cathedral), Cracks (how often; 0:
 *   never), Snow, Wind.   Music: Echo (the tape echo), Warmth, Tape, Tide
 *   White keys  a note each, rising; a crack runs where it lands
 *   Black keys  the first three: a crack near, in the middle, far; the fourth: the ice groans; the next three: the
 *               keep's bell, one, two, three strokes; then a gust; a drip; a flurry; the top one: the ice gives (a
 *               groan and the main crack runs its length)
 *
 * Moods, each its own sound and picture (LK_MOOD and LK_MOODS, data):
 *   Grey Dawn     a flat grey morning: the lake under snow, the far shore and the keep grey in the haze, a pale band
 *                 of light on the horizon, smoke from the keep; the keep's bell tolls and a swell answers; snow drifts
 *                 down and a flake on the near ice is a touch; a crack seldom; a pad, still air
 *   Thaw          the breakup: pools of meltwater spreading on the ice and holding a blue sky, the snow gone grey and
 *                 wet, water dripping from the posts and the boat, a trickle at the shore; the cracks come thick and
 *                 fast and each is a note (bright: Rin); the monks under it; no bell, no snow
 *   Whiteout      the storm: everything far gone into the white, the keep a ghost, the snow driven across in
 *                 streaks, the wind high and gusting; each gust sings the reeds (a bowed string) and the snow goes
 *                 sideways; the bell muffled through it, now and then; the cathedral choir far off, in the cloud
 *   Black Ice     night, clear and still: the snow blown off, the ice a black mirror holding the stars and the keep's
 *                 one lit window; the ice groans (the tape loop) and its fractures whistle across the lake; the bell
 *                 at midnight only; one hollow voice, few notes, low, in the long echo
 *   Long Night    a day: Grey Dawn, Thaw, Whiteout, Black Ice, half an hour in all, then dawn again; it starts
 *                 wherever the day is
 *
 * The world is 240 x 240; drawing scales by r8_fb.w / 240 for pixel art. Every loop has a fixed bound. */
#define LK_NFLAKE 48                            /* snowflakes */
#define LK_NCRACK 6                             /* cracks across the ice */
#define LK_NSEG 10                              /* a crack's segments at most */
#define LK_NSTAR 40
#define LK_NPOOL 5                              /* meltwater pools */
#define LK_NREED 14
#define LK_NPOST 2
#define LK_NSQ 24                               /* the snow's streaks in a squall */
#define LK_NDRIP 3
#define LK_NSMOKE 6
#define LK_NQ 8
#define LK_NTONE 4
#define LK_NCHOIR 5
#define LK_SOUND 1
#define LK_DARK 80                              /* the panel's dark curve (render8.c r8_dark), as the other scenes' */
#define LK_F 100.0f                             /* the eye: a point at depth z and lateral X is at 120 + LK_F X / z,
                                                 * the ice there at LK_HOR + LK_F / z */
#define LK_HOR 100.0f                           /* the horizon */
#define LK_ZSHORE 22.0f                         /* the far shore's depth */
#define LK_GROAN_F0 60.0f                       /* the groan recording's own fundamental, Hz (assets/SAMPLES.md) */

#define LK_NSKY 32
enum { LK_SKY = 0, LK_ICE = 32, LK_REFL = 64, LK_SNOW = 68, LK_PINE = 76, LK_KEEP = 80, LK_WIN = 84, LK_CRACK = 86,   /* (LK_REFL:
                                                 * one a depth band, far to near) */
       LK_POOL = 94, LK_REED = 98, LK_POST = 102, LK_BOAT = 106, LK_STAR = 110, LK_SREF = 114, LK_FLAKE = 118,
       LK_SQ = 122, LK_SMOKE = 124, LK_DRIP = 126, LK_RING = 127 };
/* (LK_SKY: the sky, its top to the horizon; LK_ICE: the ice at four depths, far to near, eight shades each (the
 * drifts' tones and the sheen); LK_SNOW: the near bank, four shades, then the snow on the wood, four; LK_PINE: the far
 * shore; LK_KEEP: its wall in shadow, lit, the roof, its openings; LK_WIN: the window lit, its reflection; LK_REFL:
 * the far shore and the keep mirrored in the ice (ice by day, a dark smear on black ice; below the snow's entries, so
 * what is drawn over the ice alone reaches it); LK_CRACK: at two depths, a crack's dark line, its lit edge, and the
 * two the flash shows; LK_POOL: meltwater, its middle to its rim) */
#define LK_ID_PEW (FM_VOICE_ID + 60)            /* a fracture's whistle (two in turn) */
#define LK_ID_GROAN (FM_VOICE_ID + 62)
#define LK_ID_SING (FM_VOICE_ID + 64)           /* the reeds */

enum { KL_MOOD, KL_ACT, KL_TONE, KL_SPACE, KL_NOTES, KL_DRONE, KL_AIR, KL_BELL, KL_VOICES, KL_CRACKS, KL_SNOW, KL_WIND,
       KL_ECHO, KL_DEPTH, KL_TAPE, KL_ARC };
static const int16_t LK_KNOB[SC_NKNOB][4] = {   /* lo, hi, step, default; SPACE is the reverb (P_REVERB) */
    {0, 4, 1, 0}, {0, 100, 1, 25}, {0, LK_NTONE - 1, 1, 0}, {0, 0, 0, 0},
    {0, 100, 1, 50}, {0, 100, 1, 40}, {0, 100, 1, 15}, {0, 100, 1, 50},
    {0, LK_NCHOIR - 1, 1, 0}, {0, 100, 1, 30}, {0, 100, 1, 30}, {0, 100, 1, 20},
    {0, 100, 1, 35}, {0, 100, 1, 20}, {0, 100, 1, 15}, {0, 100, 1, 40},
};
/* a mood: what it sounds in and how the lake looks (the knobs it sets: LK_MOODS) */
#define LK_NMOOD 5
typedef struct {
    const char *name;
    int8_t scale;                               /* SC_* (tuning.h); -1: the day's own (Long Night) */
    int16_t key;                                /* cents from the player's key */
    uint8_t lo, n;                              /* its notes: white keys lo .. lo + n - 1 */
    uint32_t sky0, sky1, ice, snow, haze, pine, water;   /* the sky's top and its horizon; the ice; the snow; the haze
                                                 * what is far goes into; the far shore; the meltwater */
    uint8_t light;                              /* % */
    uint8_t white;                              /* % the whiteout: what is far gone into the sky */
    uint8_t snowf, wind, gust;                  /* % snow falling; % the wind; s between gusts (0: none) */
    uint8_t cracks, groans, tolls;              /* % how often the ice cracks; s between groans (0: none); s between
                                                 * tolls (0: never) */
    uint8_t pool, drips, night, smoke, band;    /* % meltwater; % drips; % the night (the stars, the window lit);
                                                 * % smoke from the keep; % the pale band on the horizon */
    uint8_t vowel, buzz, breath;                /* the voice's (VOICE's knobs, 0..127) */
    uint8_t cloud;                              /* % GRAIN */
} lk_mood_t;
static const lk_mood_t LK_MOOD[LK_NMOOD] = {
    {"Long Night", -1, 0, 5, 8, 0x6E7680u, 0xCCD0D2u, 0xD8DCE0u, 0xECEEF2u, 0xB8BEC4u, 0x2A3034u, 0x6E7A86u,
     95, 25, 20, 10, 0, 15, 0, 28, 0, 0, 0, 100, 60, 50, 90, 0, 0},
    {"Grey Dawn", SC_AEOLIAN, 0, 5, 8, 0x6E7680u, 0xCCD0D2u, 0xD8DCE0u, 0xECEEF2u, 0xB8BEC4u, 0x2A3034u, 0x6E7A86u,
     95, 25, 20, 10, 0, 15, 0, 28, 0, 0, 0, 100, 60, 50, 90, 0, 0},
    {"Thaw", SC_MINYO, 300, 6, 9, 0x5A6E88u, 0xD4DCE0u, 0xC2CACEu, 0xDCE0E4u, 0xB0BAC6u, 0x24322Cu, 0x3A5068u,
     105, 10, 0, 15, 0, 70, 0, 0, 85, 80, 0, 0, 0, 70, 60, 0, 0},
    {"Whiteout", SC_IN, -500, 3, 7, 0xB2B6BAu, 0xDADCDEu, 0xD2D6DAu, 0xE2E6EAu, 0xD6D8DAu, 0x3A4044u, 0x8A9298u,
     90, 100, 100, 80, 14, 5, 0, 40, 0, 0, 0, 0, 0, 30, 40, 20, 30},
    {"Black Ice", SC_HUNGARIAN, -700, 2, 6, 0x080C1Cu, 0x34446Au, 0x10192Au, 0x6A7488u, 0x26345Au, 0x08090Eu, 0x10192Au,
     75, 0, 0, 8, 0, 35, 25, 90, 0, 0, 100, 0, 0, 40, 80, 0, 20},
};
static const uint8_t LK_LN[4] = {1, 2, 3, 4};   /* Long Night's day: Grey Dawn, Thaw, Whiteout, Black Ice */
#define LK_NSET 14
static const sc_set_t LK_MOODS[LK_NMOOD][LK_NSET] = {
    {{KL_ACT, 25}, {KL_TONE, 0}, {KL_NOTES, 50}, {KL_DRONE, 40}, {KL_AIR, 15}, {KL_BELL, 50}, {KL_VOICES, 0}, {KL_CRACKS, 30},
     {KL_SNOW, 30}, {KL_WIND, 20}, {KL_ECHO, 35}, {KL_DEPTH, 20}, {KL_TAPE, 15}, {KL_ARC, 40}},
    {{KL_ACT, 15}, {KL_TONE, 0}, {KL_NOTES, 45}, {KL_DRONE, 50}, {KL_AIR, 10}, {KL_BELL, 55}, {KL_VOICES, 0}, {KL_CRACKS, 15},
     {KL_SNOW, 30}, {KL_WIND, 10}, {KL_ECHO, 35}, {KL_DEPTH, 30}, {KL_TAPE, 10}, {KL_ARC, 30}},   /* (still: the pad,
                                                       * the bell and its swell, the snow's touches) */
    {{KL_ACT, 45}, {KL_TONE, 1}, {KL_NOTES, 60}, {KL_DRONE, 40}, {KL_AIR, 10}, {KL_BELL, 0}, {KL_VOICES, 2}, {KL_CRACKS, 70},
     {KL_SNOW, 0}, {KL_WIND, 15}, {KL_ECHO, 25}, {KL_DEPTH, 10}, {KL_TAPE, 10}, {KL_ARC, 25}},   /* (the cracks' notes,
                                                       * the drips, the trickle, the monks) */
    {{KL_ACT, 30}, {KL_TONE, 3}, {KL_NOTES, 40}, {KL_DRONE, 45}, {KL_AIR, 45}, {KL_BELL, 35}, {KL_VOICES, 4}, {KL_CRACKS, 5},
     {KL_SNOW, 100}, {KL_WIND, 75}, {KL_ECHO, 40}, {KL_DEPTH, 40}, {KL_TAPE, 25}, {KL_ARC, 30}},   /* (the wind, the
                                                       * reeds singing, the bell muffled, the choir in the cloud) */
    {{KL_ACT, 15}, {KL_TONE, 2}, {KL_NOTES, 40}, {KL_DRONE, 45}, {KL_AIR, 6}, {KL_BELL, 40}, {KL_VOICES, 1}, {KL_CRACKS, 35},
     {KL_SNOW, 0}, {KL_WIND, 8}, {KL_ECHO, 60}, {KL_DEPTH, 40}, {KL_TAPE, 20}, {KL_ARC, 45}},   /* (the groans, the
                                                       * fractures' whistles, one voice, the long echo) */
};
static const uint8_t LK_TONE_SLOT[LK_NTONE] = {ZN_DROP, ZN_RIN, ZN_BELL, ZN_MALLET};
static const char *const LK_TONE_NAME[LK_NTONE] = {"Drop", "Rin", "Bell", "Mallet"};
static const char *const LK_CHOIR_NAME[LK_NCHOIR] = {"Pad", "Solo", "Monks", "Nuns", "Cathedral"};

static struct {
    float x, y, vx, vy, ph, land_y, size;
    int8_t on;
} lk_f[LK_NFLAKE];
static struct {
    float z[LK_NSEG + 1], X[LK_NSEG + 1];      /* its points, near to far: depth and lateral place */
    float flash;                                /* its newest length lit (s left) */
    int8_t n;                                   /* segments run so far */
} lk_k[LK_NCRACK];
static struct {
    float x, y;
    uint8_t br;
} lk_st[LK_NSTAR];
static struct {
    float x, y, l;
    int8_t on;
} lk_s[LK_NSQ];
static struct {
    float x, y, y1, v, ring;                    /* a drip: where it hangs, the ice under it, its fall; its ring, fading */
    int8_t on;
} lk_d[LK_NDRIP];
static struct {
    float x, y, life;
} lk_sm[LK_NSMOKE];
static struct {
    uint32_t at;
    int id, part, vel, pan, dur;
    float cents;
} lk_q[LK_NQ];
static struct {
    uint32_t seed, ms;
    int16_t *kv;
    float t, phase;                             /* Long Night's day (0..1 in half an hour) */
    float sky0[3], sky1[3], ice[3], snow[3], haze[3], pine[3], water[3];   /* the look now (eased toward the mood's) */
    float light, white, snowf, pool, nightamt, glow, dripamt, smokeamt, band, flurry;
    float wind, wbase, gust, gpeak, gt, grise, ghold, gfall, gnext;   /* the wind now; its base; a gust's envelope */
    int gph;                                    /* a gust: 0 none, 1 rising, 2 blowing, 3 dying */
    int air_sent, trick_sent, vowel_sent, sing_sent;
    float crack_next, groan_next, toll_next, drip_next, flake_acc, groan_left, sing_c, sing_left, pew_c;
    int toll_n, pew_n;
    struct {                                    /* this shore (the picture's seed): */
        float kx, kw, kh, gx, gw;               /* the keep: its middle, width, height; its bell gable's middle and width */
        float kbase, ktop, chim;                /* its foot and its top on the screen; its chimney's x */
        float head0;                            /* where the headland begins (x) */
        float px[LK_NPOST], py[LK_NPOST], ph[LK_NPOST];   /* the posts: foot, height */
        float bx, by, bl, bw;                   /* the boat: its middle, its length and width */
        int boat;
        float dx[LK_NDRIP], dy[LK_NDRIP], dy1[LK_NDRIP];   /* where water drips from, and the ice under it */
        float plz[LK_NPOOL], plX[LK_NPOOL], plr[LK_NPOOL];   /* the pools: depth, place, size */
    } lay;
    uint32_t tr;                                /* the picture's own randomness */
    const lk_mood_t *ch;                        /* the mood whose events hold now (Long Night: its day's) */
    int pal_lv;
    uint32_t pal_ms;
    uint32_t cracks, groans, tolls, gusts, sings, drips, touches, pews, nights;   /* (the host's tests count them) */
    int key_note;
} lk;

static float lk_scale(void) { return (float)r8_fb.w / 240.0f; }
static int lk_mood_now(void) { return sc_mood_base(lk.kv[KL_MOOD], LK_NMOOD); }
static int lk_long(void) { return LK_MOOD[lk_mood_now()].scale < 0; }
static uint32_t lk_h(int a, int b)              /* the picture's own noise: 0..255, by the seed */
{
    uint32_t h = ((uint32_t)a * 73856093u) ^ ((uint32_t)b * 19349663u) ^ lk.seed;
    h *= 2654435761u;
    return (h ^ (h >> 13)) >> 24;
}
static float lk_n1(float x, int salt)           /* smooth noise along a line, 0..1 */
{
    int i = (int)fm_floorf(x);
    float f = x - (float)i, a = (float)lk_h(i, salt), b = (float)lk_h(i + 1, salt);
    f = f * f * (3.0f - 2.0f * f);
    return (a + (b - a) * f) * (1.0f / 255.0f);
}
static uint32_t lk_tr(void)                     /* the picture's randomness: 0..255 */
{
    lk.tr = lk.tr * 1664525u + 1013904223u;
    return lk.tr >> 24;
}
static float lk_trf(void) { return (float)lk_tr() * (1.0f / 255.0f); }

/* ------------------------------------------------------------ the lake --- */
/* a point on the ice at depth z and lateral X, on the screen (world units, 240 wide) */
static float lk_sx(float z, float X) { return 120.0f + LK_F * X / z; }
static float lk_sy(float z) { return LK_HOR + LK_F / z; }
static float lk_depth(float wy) { return LK_F / fm_maxf(wy - LK_HOR, 0.6f); }
/* the near bank: the snow at our feet on the left, below this line (x in world units) */
static float lk_bank_y(float wx)
{
    float d = wx - 10.0f;
    return wx < 118.0f ? 206.0f + 0.0024f * d * d + 3.0f * lk_n1(wx * 0.08f, 61) : 300.0f;
}

/* a pixel; with `behind` only over palette entries below it (LK_ICE: the sky alone; LK_SNOW: the sky and the ice, the
 * reflection too) */
static inline void lk_px(surf_t *s, int32_t x, int32_t y, uint8_t c, int behind)
{
    if ((uint32_t)x < (uint32_t)s->w && (uint32_t)y < (uint32_t)s->h) {
        uint8_t *p = s->px + y * s->w + x;
        if (!behind || *p < behind)
            *p = c;
    }
}
static void lk_disc(surf_t *s, float cx, float cy, float r, uint8_t c, int behind, int dith)
{
    int32_t x, y, x0 = (int32_t)(cx - r), x1 = (int32_t)(cx + r), y0 = (int32_t)(cy - r), y1 = (int32_t)(cy + r);
    float r2 = r * r;
    for (y = y0; y <= y1; y++)
        for (x = x0; x <= x1; x++) {
            float dx = (float)x + 0.5f - cx, dy = (float)y + 0.5f - cy;
            if (dx * dx + dy * dy < r2 && (dith >= 16 || r8_dith(x, y, dith)))
                lk_px(s, x, y, c, behind);
        }
}
static void lk_line(surf_t *s, float x0, float y0, float x1, float y1, uint8_t c, int behind)
{
    float dx = x1 - x0, dy = y1 - y0, l = fm_maxf(fm_fabsf(dx), fm_fabsf(dy));
    int32_t n = (int32_t)l + 1, i;
    n = n > 96 ? 96 : n;
    for (i = 0; i <= n; i++) {
        float u = (float)i / (float)n;
        lk_px(s, (int32_t)(x0 + dx * u), (int32_t)(y0 + dy * u), c, behind);
    }
}
static void lk_ellipse(surf_t *s, float cx, float cy, float rx, float ry, uint8_t c, int dith)   /* filled */
{
    int32_t y, y0 = (int32_t)fm_floorf(cy - ry), y1 = (int32_t)fm_floorf(cy + ry);
    for (y = y0; y <= y1; y++) {
        float dy = ((float)y + 0.5f - cy) / ry, w2 = 1.0f - dy * dy;
        if (w2 > 0.0f) {
            int32_t x, x0 = (int32_t)fm_floorf(cx - rx * fm_sqrtf(w2) + 0.5f), x1 = (int32_t)fm_floorf(cx + rx * fm_sqrtf(w2) - 0.5f);
            for (x = x0; x <= x1; x++)
                if (dith >= 16 || r8_dith(x, y, dith))
                    lk_px(s, x, y, c, 0);
        }
    }
}

/* the picture, drawn once: the sky, the far shore with its pines and the keep and their reflection, the ice in
 * perspective with its drifts and sheen, the near bank, the reeds, the posts, the boat (the pools come and go:
 * lk_draw) */
SC_SMALL static void lk_background(void)
{
    surf_t *s = &r8_bg;
    float k = lk_scale(), ik = 1.0f / k, ys = lk_sy(LK_ZSHORE);
    int32_t x, y, i;
    lk.tr = lk.seed * 747796405u + 2891336453u;
    for (y = 0; y < s->h; y++) {
        float wy = ((float)y + 0.5f) * ik;
        uint8_t *row = s->px + y * s->w;
        for (x = 0; x < s->w; x++) {
            float wx = ((float)x + 0.5f) * ik;
            int32_t f, hl;
            if (wy < ys) {                            /* the sky: its top down to the horizon */
                float g = fm_clampf(wy / ys, 0.0f, 1.0f);
                g = g * g;
                f = (int32_t)(g * (float)(LK_NSKY - 1) * 16.0f);
                row[x] = (uint8_t)(LK_SKY + (f >> 4) + r8_dith(x, y, f & 15));
                continue;
            }
            {                                         /* the ice: drifts combed by the wind, and the sheen */
                float z = lk_depth(wy), X = (wx - 120.0f) * z * (1.0f / LK_F), cs = 0.5f + 0.25f * z, hzf, tone, lz, sheen;
                int32_t gx = (int32_t)fm_floorf(X * 1.2f / cs), gz = (int32_t)fm_floorf(z * 26.0f / cs);
                hzf = (1.0f - fm_clampf((z - 0.8f) * (1.0f / 10.0f), 0.0f, 1.0f)) * 3.99f;   /* (band 3 near, 0 far) */
                hl = (int32_t)hzf;
                hl += r8_dith(x, y, (int)((hzf - (float)hl) * 16.0f));
                hl = hl > 3 ? 3 : hl;
                tone = (float)lk_h(gx, gz + 300) * (1.0f / 255.0f);
                tone = 0.3f * tone + 0.7f * lk_n1(X * 2.2f + z * 0.6f, 77);
                lz = fm_log2f(z);
                sheen = fm_maxf(0.0f, 1.0f - fm_fabsf(lz - 2.0f) * (1.0f / 1.5f)) * (0.6f + 0.4f * lk_n1(X * 2.0f, 78));
                f = (int32_t)((tone * 5.0f + sheen * 2.0f) * 16.0f);
                f = f > 7 * 16 ? 7 * 16 : f;
                row[x] = (uint8_t)(LK_ICE + hl * 8 + (f >> 4) + r8_dith(x + 1, y, f & 15));
            }
        }
    }
    {   /* the far shore: pines along it, and the headland rising to the keep on the right */
        float h0 = lk.lay.head0;
        for (x = 0; x < s->w; x++) {
            float wx = ((float)x + 0.5f) * ik, rise = wx > h0 ? fm_minf(1.0f, (wx - h0) * (1.0f / 40.0f)) : 0.0f,
                  top = ys - 1.5f - 5.0f * rise - 2.0f * lk_n1(wx * 0.3f, 62);
            int32_t y0 = (int32_t)(top * k), y1 = (int32_t)(ys * k) + 1;
            for (y = y0; y <= y1 && y < s->h; y++)
                if (y >= 0)
                    s->px[y * s->w + x] = LK_PINE + (y == y0 ? 1 : 0);
        }
        for (i = 0; i < 40; i++) {                     /* the pines: a spike each, taller on the headland */
            float wx = 2.0f + 236.0f * lk_trf(), rise = wx > h0 ? fm_minf(1.0f, (wx - h0) * (1.0f / 40.0f)) : 0.0f,
                  base = ys - 1.0f - 5.0f * rise, ht = 2.5f + 4.0f * lk_trf() + 2.0f * rise, hw = 0.6f + 0.8f * lk_trf();
            r8_tri(s, (wx - hw) * k, base * k, (wx + hw) * k, base * k, wx * k, (base - ht) * k, LK_PINE + (lk_tr() & 1u ? 0 : 2));
        }
    }
    {   /* the keep: a square tower, its lit side toward the dawn, battlements, a bell gable, the window; a stump of
         * curtain wall to one side; the reflection of it all below the shore */
        float kx = lk.lay.kx, kw = lk.lay.kw, kb = lk.lay.kbase, kt = lk.lay.ktop, gx = lk.lay.gx, gw = lk.lay.gw;
        int32_t y0 = (int32_t)(kt * k), y1 = (int32_t)(kb * k);
        r8_rect(s, (int32_t)((kx - kw * 0.5f) * k), y0, (int32_t)(kw * k + 0.5f), y1 - y0 + 1, LK_KEEP);
        r8_rect(s, (int32_t)((kx - kw * 0.5f) * k), y0, (int32_t)(kw * 0.3f * k + 0.5f), y1 - y0 + 1, LK_KEEP + 1);
        for (i = 0; i < 5; i++)                       /* the merlons */
            if (i & 1)
                r8_rect(s, (int32_t)((kx - kw * 0.5f + kw * 0.2f * (float)i) * k), (int32_t)((kt - 2.0f) * k), (int32_t)(kw * 0.2f * k + 0.5f), (int32_t)(2.0f * k + 1.0f), LK_KEEP);
        r8_tri(s, (gx - gw) * k, kt * k, (gx + gw) * k, kt * k, gx * k, (kt - gw * 2.2f) * k, LK_KEEP + 2);   /* the gable */
        lk_px(s, (int32_t)(gx * k), (int32_t)((kt - gw * 0.8f) * k), LK_KEEP + 3, 0);   /* (its opening, the bell in it) */
        r8_rect(s, (int32_t)((kx + kw * 0.5f) * k), (int32_t)((kb - 6.0f) * k), (int32_t)(kw * 0.9f * k), (int32_t)(6.0f * k + 1.0f), LK_KEEP);   /* the wall */
        r8_rect(s, (int32_t)((kx + kw * 0.5f) * k), (int32_t)((kb - 6.0f) * k), (int32_t)(kw * 0.9f * k), (int32_t)(1.0f * k + 0.5f), LK_KEEP + 1);
        r8_rect(s, (int32_t)((kx + kw * 0.12f) * k), (int32_t)((kb - 12.0f) * k), (int32_t)(1.0f * k + 0.5f), (int32_t)(3.0f * k + 0.5f), LK_WIN);   /* the window */
        for (y = (int32_t)(ys * k); y < (int32_t)((2.0f * ys - kt + 2.0f) * k) && y < s->h; y++) {   /* the reflection */
            float wy = ((float)y + 0.5f) * ik, my = 2.0f * ys - wy;   /* (mirrored in the shore line) */
            int32_t sy = (int32_t)(my * k), xa = (int32_t)((kx - kw * 0.5f) * k) - 1, xb = (int32_t)((kx + kw * 1.4f) * k) + 1;
            uint8_t *row = s->px + y * s->w;
            if (sy < 0)
                continue;
            for (x = xa < 0 ? 0 : xa; x <= xb && x < s->w; x++) {
                uint8_t v = s->px[sy * s->w + x];
                if (v >= LK_PINE && v <= LK_WIN + 1 && row[x] < LK_REFL)
                    row[x] = (uint8_t)(LK_REFL + (row[x] - LK_ICE) / 8);   /* (its band: the ice's there) */
            }
        }
        for (x = 0; x < s->w; x++) {                 /* the pines' reflection: a dark fringe under the shore */
            float wx = ((float)x + 0.5f) * ik, rise = wx > lk.lay.head0 ? fm_minf(1.0f, (wx - lk.lay.head0) * (1.0f / 40.0f)) : 0.0f,
                  d = 2.0f + 4.0f * rise + 3.0f * lk_n1(wx * 0.3f, 62);
            for (y = (int32_t)(ys * k) + 1; y <= (int32_t)((ys + d) * k) && y < s->h; y++)
                if (s->px[y * s->w + x] < LK_REFL)
                    s->px[y * s->w + x] = (uint8_t)(LK_REFL + (s->px[y * s->w + x] - LK_ICE) / 8);
        }
    }
    for (y = 0; y < s->h; y++) {                      /* the near bank: snow, its crest lit */
        float wy = ((float)y + 0.5f) * ik;
        uint8_t *row = s->px + y * s->w;
        for (x = 0; x < s->w; x++) {
            float wx = ((float)x + 0.5f) * ik, by = lk_bank_y(wx), tone;
            int32_t f;
            if (wy < by)
                continue;
            tone = fm_clampf((wy - by) * 0.25f, 0.0f, 1.0f) * 0.6f + 0.4f * lk_n1(wx * 0.15f + wy * 0.1f, 63);
            f = (int32_t)((1.0f - tone) * 3.0f * 16.0f);
            row[x] = (uint8_t)(LK_SNOW + (f >> 4) + r8_dith(x, y, f & 15));
        }
    }
    for (i = 0; i < LK_NREED; i++) {                  /* the reeds, frozen in along the bank's edge, snow on their heads */
        float wx = 4.0f + 100.0f * ((float)i + 0.2f + 0.6f * lk_trf()) * (1.0f / (float)LK_NREED), wy = lk_bank_y(wx) + 1.0f + 6.0f * lk_trf(),
              ht = 14.0f + 16.0f * lk_trf(), lean = (lk_trf() - 0.5f) * 10.0f, tx = wx + lean, ty = wy - ht;
        if (wy > 238.0f)
            wy = 238.0f;
        r8_limb(s, wx * k, wy * k, 1.0f * k, tx * k, ty * k, 0.6f * k, LK_REED + (lk_tr() & 1u));
        if (lk_tr() & 1u)
            r8_limb(s, tx * k, ty * k, 0.8f * k, (tx + lean * 0.5f + 2.0f) * k, (ty + 4.0f) * k, 0.4f * k, LK_REED + 1);   /* (bent) */
        lk_px(s, (int32_t)(tx * k), (int32_t)((ty - 1.0f) * k), LK_REED + 2, 0);   /* (its snow) */
        lk_px(s, (int32_t)((tx + 1.0f) * k), (int32_t)((ty - 1.0f) * k), LK_REED + 3, 0);
    }
    for (i = 0; i < LK_NPOST; i++) {                  /* the mooring posts, snow on their tops */
        float px = lk.lay.px[i], py = lk.lay.py[i], ph = lk.lay.ph[i];
        r8_rect(s, (int32_t)((px - 1.5f) * k), (int32_t)((py - ph) * k), (int32_t)(3.0f * k + 0.5f), (int32_t)(ph * k + 1.0f), LK_POST);
        r8_rect(s, (int32_t)((px - 1.5f) * k), (int32_t)((py - ph) * k), (int32_t)(1.0f * k + 0.5f), (int32_t)(ph * k + 1.0f), LK_POST + 1);
        r8_rect(s, (int32_t)((px - 2.0f) * k), (int32_t)((py - ph - 1.5f) * k), (int32_t)(4.0f * k + 0.5f), (int32_t)(1.5f * k + 1.0f), LK_POST + 2);
        lk_ellipse(s, px * k, py * k, 3.0f * k, 1.0f * k, LK_POST + 3, 16);   /* (the ice heaved round it) */
    }
    if (lk.lay.boat) {                                /* the rowboat frozen in, seen a little from above, snow in it */
        float bx = lk.lay.bx, by = lk.lay.by, bl = lk.lay.bl, bw = lk.lay.bw;
        lk_ellipse(s, bx * k, by * k, bl * 0.5f * k, bw * 0.5f * k, LK_BOAT, 16);
        lk_ellipse(s, bx * k, (by - bw * 0.1f) * k, bl * 0.42f * k, bw * 0.32f * k, LK_BOAT + 2, 16);   /* (the snow inside) */
        r8_limb(s, (bx - bl * 0.5f) * k, by * k, 0.8f * k, (bx + bl * 0.5f) * k, by * k, 0.8f * k, LK_BOAT + 1);   /* (the gunwale) */
        r8_limb(s, (bx - bl * 0.1f) * k, (by - bw * 0.3f) * k, 0.7f * k, (bx - bl * 0.1f) * k, (by + bw * 0.3f) * k, 0.7f * k, LK_BOAT + 3);   /* (a thwart) */
        r8_tri(s, (bx + bl * 0.5f) * k, (by - bw * 0.5f) * k, (bx + bl * 0.62f) * k, (by - bw * 0.1f) * k, (bx + bl * 0.5f) * k, (by + bw * 0.1f) * k, LK_BOAT);   /* (the prow) */
    }
}

/* ------------------------------------------------------------- sounds --- */
static void lk_q_add(uint32_t delay_ms, int id, int part, float cents, int vel, int pan, int dur)
{
    int i;
    for (i = 0; i < LK_NQ; i++)
        if (!lk_q[i].at) {
            lk_q[i].at = lk.ms + delay_ms + 1u;
            lk_q[i].id = id;
            lk_q[i].part = part;
            lk_q[i].cents = cents;
            lk_q[i].vel = vel;
            lk_q[i].pan = pan;
            lk_q[i].dur = dur;
            return;
        }
}
static float lk_low(float c, float hi, float lo)     /* c brought into the octave hi .. lo (cents from A4, hi > lo) */
{
    while (c > hi)
        c -= 1200.0f;
    while (c < lo)
        c += 1200.0f;
    return c;
}
static float lk_root(void) { return cd_cents(CD_CHORDS[cd.chord][0]); }

/* a crack's points: from (z0, X0) across the ice in a direction, wavering; crack 0 runs from our feet toward the keep */
static void lk_crack_make(int i, float z0, float X0, float dz, float dX)
{
    int j;
    float z = z0, X = X0;
    for (j = 0; j <= LK_NSEG; j++) {
        lk_k[i].z[j] = z;
        lk_k[i].X[j] = X;
        z *= 1.0f + dz * (0.7f + 0.6f * sc_frand());
        X += dX * (0.7f + 0.6f * sc_frand()) * z * 0.12f + (sc_frand() - 0.5f) * 0.08f * z;
        z = fm_clampf(z, 0.62f, 28.0f);
    }
    lk_k[i].n = 0;
    lk_k[i].flash = 0.0f;
}
static void lk_crack_new(int i)                 /* a crack starting afresh somewhere on the ice */
{
    float z0 = 0.7f * fm_exp2f(3.2f * sc_frand()), X0 = (sc_frand() - 0.5f) * 1.6f * z0 * 0.9f;
    if (i == 0)
        lk_crack_make(0, 0.66f, -0.25f + 0.2f * sc_frand(), 0.32f, (lk.lay.kx - 120.0f) * (1.0f / LK_F) * 0.9f);
    else
        lk_crack_make(i, z0, X0, 0.18f + 0.2f * sc_frand(), (sc_frand() - 0.5f) * 2.0f);
}
static float lk_crack_x(int i, int seg) { return lk_sx(lk_k[i].z[seg], lk_k[i].X[seg]); }

/* the ice cracks: crack i runs a segment farther (a new one if it is spent), heard where it runs: its recorded crack,
 * then a note where it runs across (by night: a fracture's whistle, falling); strength: 0..1 */
static void lk_crack(int i, float strength)
{
    float x, z, pos;
    int vel, pan, n = fm_shot_count(FM_SHOT_ICE);
    i = i < 0 ? 0 : i >= LK_NCRACK ? LK_NCRACK - 1 : i;
    if (lk_k[i].n >= LK_NSEG)
        lk_crack_new(i);
    lk_k[i].n++;
    lk_k[i].flash = 0.35f;
    z = lk_k[i].z[lk_k[i].n];
    x = lk_crack_x(i, lk_k[i].n);
    pos = fm_clampf(x * (1.0f / 240.0f), 0.0f, 1.0f);
    pan = cd_pan(pos);
    vel = (int)(34.0f + 30.0f * strength * fm_clampf(1.4f - 0.3f * fm_log2f(z), 0.4f, 1.0f));   /* (the nearer the louder) */
    if (n > 0 && lk.kv[KL_CRACKS] > 0)
        fm_shot(i & 3, FM_SHOT_ICE, (int)(sc_rnd() % (uint32_t)n), vel, pan, 90 + (int)(sc_rnd() % 21u));
    if (lk.ch && lk.ch->night >= 50) {                 /* the whistle: high, falling a fifteenth in a third of a second */
        float c = lk_low(lk_root(), 2600.0f, 1400.0f);
        int id = LK_ID_PEW + (lk.pew_n++ & 1);
        if (!cd_quiet()) {
            fm_voice(id, 3, c, 26 + (int)(24.0f * strength), pan, 700);
            fm_voice_glide(id, c - 1900.0f, 320);
            lk.pews++;
        }
    } else {
        cd_pluck(pos, 0.35f + 0.4f * strength, 1600);
    }
    lk.cracks++;
}

/* the ice groans: the tape loop (part 7: SAMPLE) pitched to the chord's root, low (its octave in 73..147 Hz), from a
 * place in the recording, forward or back, long; the cracks' edges glow while it lasts */
static void lk_groan(float strength)
{
    float fr = 440.0f * fm_exp2f(lk_low(lk_root(), -1900.0f, -3100.0f) * (1.0f / 1200.0f)),
          c = -900.0f + 1200.0f * fm_log2f(fr / LK_GROAN_F0);   /* (the sample at its root plays as recorded) */
    int dur = 5000 + (int)(6000.0f * strength) + (int)(sc_rnd() % 2000u);
    fm_part_knobs(7, (int)(sc_rnd() % 50u), 80 + (int)(sc_rnd() % 48u), sc_rnd() % 3u == 0, 0);
    fm_voice(LK_ID_GROAN, 7, c, 50 + (int)(40.0f * strength), 44 + (int)(sc_rnd() % 40u), dur);
    lk.groan_left = (float)dur * 0.001f;
    lk.groans++;
}

/* the keep's bell: n strokes, 2.4 s apart, its recording (a stroke, far across a valley) from the keep, a little slower
 * or faster each time (another hour, another bell); snow shaken from its roof; in a mood with a swell, one answers the
 * first stroke. The strokes go through the queue as part 0 notes: lk_step plays each as a shot */
static void lk_toll(int n, int far)
{
    float kpos = fm_clampf(lk.lay.kx * (1.0f / 240.0f), 0.0f, 1.0f);
    int i, vel = (far ? 22 : 30) + lk.kv[KL_BELL] * 30 / 100;
    if (lk.kv[KL_BELL] <= 0 || fm_shot_count(FM_SHOT_TOLL) < 1)
        return;
    for (i = 0; i < n && i < 3; i++)
        lk_q_add((uint32_t)(i * 2400), 1, 0, 0.0f, vel - 3 * i + (int)(sc_rnd() % 5u), cd_pan(kpos), 92 + (int)(sc_rnd() % 12u));
    if (lk.ch && lk.ch->band > 0)
        lk_q_add(700u, 0, 4, 0.0f, 0, 0, 0);          /* (a swell, part 4: lk_step plays it as cd_swell) */
    for (i = 0; i < 4; i++) {
        int j = (int)(sc_rnd() % LK_NFLAKE);
        lk_f[j].on = 1;
        lk_f[j].x = lk.lay.kx - lk.lay.kw * 0.5f + lk.lay.kw * sc_frand();
        lk_f[j].y = lk.lay.ktop + 1.0f;
        lk_f[j].vx = 0.0f;
        lk_f[j].vy = 6.0f + 4.0f * sc_frand();
        lk_f[j].size = 0.0f;
        lk_f[j].land_y = lk.lay.kbase + 1.0f;
        lk_f[j].ph = sc_frand();
    }
    lk.tolls++;
}

/* a gust sings the reeds: a bowed string on the chord's root, 147..294 Hz, as strong as the gust, as long */
static void lk_sing(float peak)
{
    float c = lk_low(lk_root(), -700.0f, -1900.0f);
    if (lk.kv[KL_WIND] <= 0)
        return;
    lk.sing_c = c;
    lk.sing_left = 3.0f + 5.0f * peak;
    fm_voice(LK_ID_SING, 5, c, 30 + (int)(64.0f * peak), cd_pan(0.2f), (int)(lk.sing_left * 1000.0f));
    lk.sings++;
}

static void lk_drone(void)                      /* Voices: the drone's source, a pad or one of GR!MM's choirs */
{
    int t = lk.kv[KL_VOICES];
    if (t <= 0) {
        fm_part_patch(2, ZN_DEEP_PAD);
    } else {
        fm_part_patch(2, FM_ENG_VOICE + t - 1);
        fm_part_attack(2, 2400);
        lk.vowel_sent = -100;
    }
    fm_part_release(2, 4500);
}

static void lk_fx(void)
{
    int t = lk.kv[KL_TAPE], e = lk.kv[KL_ECHO], c = lk.ch ? lk.ch->cloud : 0;
    fm_tape(t > 50 ? 100 : t * 2);
    fm_tape_knobs(t * 70 / 100, t * 60 / 100, 20 + t * 50 / 100, t * 45 / 100);
    fm_fx_send(e || c ? 70 : 0);
    fm_delay(118, 50 + e * 30 / 100, 70, e * 90 / 100);
    fm_grain(110, 40, 70, c);
}

static void lk_mix_levels(void)
{
    fm_part_level(1, lk.kv[KL_NOTES]);
    fm_part_level(3, lk.kv[KL_NOTES]);
    fm_part_level(4, lk.kv[KL_NOTES]);
    cd_drone_level(lk.kv[KL_DRONE]);
    fm_part_level(5, lk.kv[KL_WIND] * 70 / 100);
    fm_part_level(7, 40 + lk.kv[KL_CRACKS] * 30 / 100);
    fm_bed(FM_BED_WIND, lk.kv[KL_AIR]);
}

/* the mood whose events hold from now (a mood chosen; Long Night passing into the next): its range */
static void lk_char(const lk_mood_t *m)
{
    if (m == lk.ch)
        return;
    lk.ch = m;
    cd_range(m->lo, m->n);
    lk.crack_next = fm_minf(lk.crack_next, 4.0f + 8.0f * sc_frand());
    lk.toll_next = m->tolls ? fm_minf(lk.toll_next, 6.0f + 10.0f * sc_frand()) : lk.toll_next;
    lk.groan_next = m->groans ? fm_minf(lk.groan_next, 8.0f + 10.0f * sc_frand()) : lk.groan_next;
    lk_fx();
}

/* ------------------------------------------------------------ picture --- */
static void lk_palette(void)
{
#define LK_RGB(v) ((uint32_t)(v)[0] << 16 | (uint32_t)(v)[1] << 8 | (uint32_t)(v)[2])
    uint32_t sky0 = LK_RGB(lk.sky0), sky1 = LK_RGB(lk.sky1), ice = LK_RGB(lk.ice), snow = LK_RGB(lk.snow), haze = LK_RGB(lk.haze),
             pine = LK_RGB(lk.pine), water = LK_RGB(lk.water), shadow = 0x08090Cu, white = r8_mix(sky1, 0xF4F6F8u, 0.5f), far4;
    static const float HZ[4] = {0.75f, 0.5f, 0.25f, 0.05f};   /* how far into the haze each depth of ice lies */
    float l = lk.light, lm = fm_minf(1.0f, l), w = lk.white, nt = lk.nightamt;
    int i, b;
#undef LK_RGB
    for (i = 0; i < LK_NSKY; i++) {                   /* the sky, and the pale band on the horizon at dawn */
        float u = (float)i * (1.0f / (float)(LK_NSKY - 1)), bd = lk.band * fm_maxf(0.0f, u - 0.7f) * (1.0f / 0.3f);
        r8_pal_set(LK_SKY + i, r8_mix(r8_mix(sky0, sky1, u), 0xF2F0E8u, 0.5f * bd * bd));
    }
    for (b = 0; b < 4; b++) {                         /* the ice at each depth: dark between the drifts to the sheen */
        uint32_t dark = r8_mix(r8_mix(shadow, ice, 0.74f * lm + 0.1f), haze, HZ[b]), lit = r8_mix(r8_mix(shadow, ice, lm), haze, HZ[b]),
                 sheen = r8_mix(r8_mix(ice, sky1, 0.6f), haze, HZ[b] * 0.5f);
        for (i = 0; i < 8; i++) {
            uint32_t c = i < 6 ? r8_mix(dark, lit, (float)i * (1.0f / 5.0f)) : r8_mix(lit, sheen, (float)(i - 5) * 0.5f);
            r8_pal_set(LK_ICE + b * 8 + i, r8_mix(c, white, w * HZ[b] * 1.2f));
        }
    }
    far4 = r8_mix(r8_mix(pine, haze, 0.55f), white, w);   /* the far shore, into the haze and the white */
    for (i = 0; i < 4; i++)
        r8_pal_set(LK_SNOW + i, r8_mix(shadow, snow, (0.55f + 0.15f * (float)i) * lm));
    r8_pal_set(LK_SNOW + 4, r8_mix(shadow, snow, 0.9f * lm));   /* the snow on the wood */
    r8_pal_set(LK_SNOW + 5, r8_mix(shadow, snow, 0.75f * lm));
    r8_pal_set(LK_SNOW + 6, r8_mix(shadow, snow, 1.0f * lm));
    r8_pal_set(LK_SNOW + 7, r8_mix(shadow, snow, 0.6f * lm));
    r8_pal_set(LK_PINE, far4);
    r8_pal_set(LK_PINE + 1, r8_mix(far4, snow, 0.35f * (1.0f - nt)));   /* (snow along the shore line) */
    r8_pal_set(LK_PINE + 2, r8_mix(far4, shadow, 0.25f));
    r8_pal_set(LK_PINE + 3, far4);
    r8_pal_set(LK_KEEP, r8_mix(r8_mix(pine, 0x4A4C52u, 0.4f * lm), white, w));
    r8_pal_set(LK_KEEP + 1, r8_mix(r8_mix(pine, 0x8C8A84u, 0.5f * lm), white, w));
    r8_pal_set(LK_KEEP + 2, r8_mix(r8_mix(pine, 0x5A5450u, 0.5f * lm), white, w));
    r8_pal_set(LK_KEEP + 3, r8_mix(shadow, white, w));
    r8_pal_set(LK_WIN, r8_mix(r8_mix(r8_base[LK_KEEP], 0xFFD890u, nt), white, w * 0.9f));
    r8_pal_set(LK_WIN + 1, r8_mix(r8_base[LK_ICE + 2], 0xE8C070u, 0.7f * nt));
    for (b = 0; b < 4; b++)                           /* the reflection: the ice by day, a dark smear in the black ice */
        r8_pal_set(LK_REFL + b, r8_mix(r8_base[LK_ICE + b * 8 + 3], r8_mix(pine, ice, 0.5f), nt * (0.9f - 0.15f * (float)b)));
    for (b = 0; b < 2; b++) {                         /* the cracks: a dark line and its lit edge; the flash; the glow */
        uint32_t base = r8_base[LK_ICE + (b ? 24 : 8) + 3], gl = r8_mix(0xC8E0FFu, 0xF0F8FFu, lk.glow);
        r8_pal_set(LK_CRACK + b * 4, r8_mix(base, r8_mix(0x1A2230u, water, 0.3f), b ? 0.75f : 0.5f));
        r8_pal_set(LK_CRACK + b * 4 + 1, r8_mix(r8_mix(base, 0xF8FCFFu, b ? 0.45f : 0.25f), gl, 0.6f * lk.glow));
        r8_pal_set(LK_CRACK + b * 4 + 2, r8_mix(base, 0x5A7AA0u, 0.6f));
        r8_pal_set(LK_CRACK + b * 4 + 3, 0xF0F8FFu);
    }
    for (i = 0; i < 4; i++) {                         /* the pools: the sky held in them, their rims toward the ice */
        float show = fm_clampf((lk.pool - 0.18f * (float)i) * 3.0f, 0.0f, 1.0f);
        r8_pal_set(LK_POOL + i, r8_mix(r8_base[LK_ICE + 27], r8_mix(water, sky1, 0.3f + 0.12f * (float)i), 0.3f + 0.7f * show));
    }
    r8_pal_set(LK_REED, r8_mix(shadow, 0x6A5A3Au, 0.6f * lm));
    r8_pal_set(LK_REED + 1, r8_mix(shadow, 0x9A8858u, 0.6f * lm));
    r8_pal_set(LK_REED + 2, r8_base[LK_SNOW + 4]);
    r8_pal_set(LK_REED + 3, r8_base[LK_SNOW + 5]);
    r8_pal_set(LK_POST, r8_mix(shadow, 0x3A3028u, 0.7f * lm));
    r8_pal_set(LK_POST + 1, r8_mix(shadow, 0x6A5A48u, 0.7f * lm));
    r8_pal_set(LK_POST + 2, r8_base[LK_SNOW + 4]);
    r8_pal_set(LK_POST + 3, r8_mix(r8_base[LK_ICE + 26], 0x3A3028u, 0.3f));
    r8_pal_set(LK_BOAT, r8_mix(shadow, 0x3A2E26u, 0.7f * lm));
    r8_pal_set(LK_BOAT + 1, r8_mix(shadow, 0x7A6A54u, 0.7f * lm));
    r8_pal_set(LK_BOAT + 2, r8_base[LK_SNOW + 6]);
    r8_pal_set(LK_BOAT + 3, r8_mix(shadow, 0x5A4A3Au, 0.7f * lm));
    {
        uint32_t sk = r8_base[LK_SKY + 4];
        for (i = 0; i < 4; i++) {
            r8_pal_set(LK_STAR + i, r8_mix(sk, 0xE0E8FFu, (0.3f + 0.23f * (float)i) * nt));
            r8_pal_set(LK_SREF + i, r8_mix(r8_base[LK_ICE + 3], 0xC8D4F0u, (0.15f + 0.15f * (float)i) * nt));
        }
    }
    r8_pal_set(LK_FLAKE, r8_mix(sky1, 0xFFFFFFu, 0.7f));
    r8_pal_set(LK_FLAKE + 1, r8_mix(r8_base[LK_SKY + 24], 0xFFFFFFu, 0.45f));
    r8_pal_set(LK_FLAKE + 2, r8_mix(r8_base[LK_ICE + 28], 0xFFFFFFu, 0.5f));
    r8_pal_set(LK_FLAKE + 3, r8_mix(r8_base[LK_ICE + 12], 0xFFFFFFu, 0.35f));
    r8_pal_set(LK_SQ, r8_mix(sky1, 0xFFFFFFu, 0.5f));
    r8_pal_set(LK_SQ + 1, r8_mix(r8_base[LK_ICE + 20], 0xFFFFFFu, 0.4f));
    r8_pal_set(LK_SMOKE, r8_mix(r8_base[LK_SKY + 24], 0x8A8C90u, 0.5f));
    r8_pal_set(LK_SMOKE + 1, r8_mix(r8_base[LK_SKY + 24], 0x8A8C90u, 0.25f));
    r8_pal_set(LK_DRIP, r8_mix(water, 0xE0ECF8u, 0.6f));
    r8_pal_set(LK_RING, r8_mix(r8_base[LK_ICE + 27], 0xF0F8FFu, 0.5f));
    r8_pal_commit(0xFFFFFFu, 0xFFFFFFu, 0);
}

static void lk_draw(void)
{
    surf_t *s = &r8_fb;
    float k = lk_scale();
    int i, j;
    r8_copy(s, &r8_bg);
    if (lk.nightamt > 0.02f) {                        /* the night: the stars, and the stars in the ice */
        float wob = 1.2f * fm_sin_turns(lk.t * 0.11f);
        for (i = 0; i < LK_NSTAR; i++) {
            int tw = (int)(lk.t * 2.0f + (float)i * 0.37f) & 3, br = (lk_st[i].br + (tw == 0)) % 4;
            lk_px(s, (int32_t)(lk_st[i].x * k), (int32_t)(lk_st[i].y * k), (uint8_t)(LK_STAR + br), LK_ICE);
            if (lk_st[i].br >= 1)
                lk_px(s, (int32_t)((lk_st[i].x + wob * (float)(i & 3)) * k), (int32_t)((2.0f * lk_sy(LK_ZSHORE) - lk_st[i].y) * k),
                      (uint8_t)(LK_SREF + br), LK_SNOW);
        }
        if (lk.nightamt > 0.3f) {                     /* the window's light down the ice, wavering */
            float x0 = lk.lay.kx + lk.lay.kw * 0.12f + 0.5f, ys = lk_sy(LK_ZSHORE);
            for (j = 0; j < 14; j++) {
                float y = ys + 1.0f + (float)j * 2.2f, x = x0 + 1.5f * fm_sin_turns(lk.t * 0.3f + (float)j * 0.17f);
                if ((j & 1) == 0 || r8_dith(j, (int)(lk.t * 3.0f), 8))
                    lk_px(s, (int32_t)(x * k), (int32_t)(y * k), LK_WIN + 1, LK_SNOW);
            }
        }
    }
    if (lk.pool > 0.02f)                              /* the thaw's pools: ellipses in perspective, spreading, their
                                                       * rims dithered */
        for (i = 0; i < LK_NPOOL; i++) {
            float z = lk.lay.plz[i], cx = lk_sx(z, lk.lay.plX[i]), cy = lk_sy(z), rx = lk.lay.plr[i] * LK_F / z * (0.35f + 0.65f * lk.pool),
                  ry = rx * 0.28f;
            lk_ellipse(s, cx * k, cy * k, rx * k, ry * k, LK_POOL + 3, 10);
            lk_ellipse(s, cx * k, cy * k, rx * 0.85f * k, ry * 0.85f * k, LK_POOL + 2, 16);
            lk_ellipse(s, cx * k, cy * k, rx * 0.65f * k, ry * 0.65f * k, LK_POOL + 1, 16);
            lk_ellipse(s, cx * k, cy * k, rx * 0.4f * k, ry * 0.4f * k, LK_POOL, 16);
        }
    for (i = 0; i < LK_NSMOKE; i++)                   /* smoke from the keep */
        if (lk_sm[i].life > 0.0f)
            lk_px(s, (int32_t)(lk_sm[i].x * k), (int32_t)(lk_sm[i].y * k), (uint8_t)(LK_SMOKE + (lk_sm[i].life < 2.0f)), LK_ICE);
    for (i = 0; i < LK_NCRACK; i++) {                 /* the cracks: a dark line, its edge lit; the newest length flashing */
        int n = lk_k[i].n, d;
        for (j = 0; j < n; j++) {
            float x0 = lk_crack_x(i, j), y0 = lk_sy(lk_k[i].z[j]), x1 = lk_crack_x(i, j + 1), y1 = lk_sy(lk_k[i].z[j + 1]);
            int fl = j == n - 1 && lk_k[i].flash > 0.0f;
            d = lk_k[i].z[j] < 2.4f;
            lk_line(s, x0 * k, y0 * k, x1 * k, y1 * k, (uint8_t)(LK_CRACK + d * 4 + (fl ? 2 : 0)), LK_SNOW);
            lk_line(s, x0 * k, (y0 - 1.0f) * k, x1 * k, (y1 - 1.0f) * k, (uint8_t)(LK_CRACK + d * 4 + (fl ? 3 : 1)), LK_SNOW);
        }
    }
    for (i = 0; i < LK_NDRIP; i++)                    /* a drip falling; its ring on the ice */
        if (lk_d[i].on) {
            lk_px(s, (int32_t)(lk_d[i].x * k), (int32_t)(lk_d[i].y * k), LK_DRIP, 0);
            if (lk_d[i].ring > 0.0f) {
                float r = (1.0f - lk_d[i].ring) * 6.0f + 1.0f;
                r8_ring(s, lk_d[i].x * k, lk_d[i].y1 * k, r * k, r * 0.35f * k, LK_RING);
            }
        }
    for (i = 0; i < LK_NSQ; i++)                      /* the snow driven across in streaks */
        if (lk_s[i].on)
            lk_line(s, lk_s[i].x * k, lk_s[i].y * k, (lk_s[i].x - lk_s[i].l) * k, (lk_s[i].y - lk_s[i].l * 0.12f) * k,
                    (uint8_t)(LK_SQ + (lk_s[i].y > LK_HOR)), 0);
    for (i = 0; i < LK_NFLAKE; i++)                   /* the snow coming down */
        if (lk_f[i].on) {
            int32_t x = (int32_t)(lk_f[i].x * k), y = (int32_t)(lk_f[i].y * k);
            uint8_t c = (uint8_t)(LK_FLAKE + (lk_f[i].y > LK_HOR ? 2 : 0) + (lk_f[i].size < 1.0f));
            lk_px(s, x, y, c, 0);
            if (lk_f[i].size >= 1.0f)
                lk_px(s, x + 1, y, c, 0);
        }
}

/* -------------------------------------------------------------- world --- */
/* the shore's own shape: where the keep stands and how big, the headland, the posts, the boat, the pools, the cracks,
 * the stars, where the water drips from */
SC_SMALL static void lk_layout(void)
{
    int i;
    float ys = lk_sy(LK_ZSHORE);
    lk.lay.kx = 150.0f + 40.0f * (float)lk_h(1, 1) * (1.0f / 255.0f);
    lk.lay.kw = 14.0f + 10.0f * (float)lk_h(1, 2) * (1.0f / 255.0f);
    lk.lay.kh = 28.0f + 16.0f * (float)lk_h(1, 3) * (1.0f / 255.0f);
    lk.lay.kbase = ys - 4.0f;
    lk.lay.ktop = lk.lay.kbase - lk.lay.kh;
    lk.lay.gw = 2.5f + 1.5f * (float)lk_h(1, 4) * (1.0f / 255.0f);
    lk.lay.gx = lk.lay.kx + (lk_h(1, 5) & 1u ? 1.0f : -1.0f) * (lk.lay.kw * 0.5f - lk.lay.gw);
    lk.lay.chim = lk.lay.kx - (lk.lay.gx - lk.lay.kx) * 0.6f;
    lk.lay.head0 = lk.lay.kx - 30.0f - 20.0f * (float)lk_h(1, 6) * (1.0f / 255.0f);
    for (i = 0; i < LK_NPOST; i++) {
        lk.lay.px[i] = 30.0f + 28.0f * (float)i + 8.0f * (float)lk_h(i, 7) * (1.0f / 255.0f);
        lk.lay.py[i] = lk_bank_y(lk.lay.px[i]) + 8.0f + 6.0f * (float)lk_h(i, 8) * (1.0f / 255.0f);
        lk.lay.ph[i] = 16.0f + 10.0f * (float)lk_h(i, 9) * (1.0f / 255.0f);
    }
    lk.lay.boat = lk_h(2, 10) % 4u != 0;              /* (three shores in four have the boat) */
    lk.lay.bx = 170.0f + 30.0f * (float)lk_h(2, 11) * (1.0f / 255.0f);
    lk.lay.by = 198.0f + 16.0f * (float)lk_h(2, 12) * (1.0f / 255.0f);
    lk.lay.bl = 40.0f + 10.0f * (float)lk_h(2, 13) * (1.0f / 255.0f);
    lk.lay.bw = 13.0f + 3.0f * (float)lk_h(2, 14) * (1.0f / 255.0f);
    for (i = 0; i < LK_NDRIP; i++) {                  /* water drips from the posts' tops and the boat's gunwale */
        if (i < LK_NPOST) {
            lk.lay.dx[i] = lk.lay.px[i] + 2.0f;
            lk.lay.dy[i] = lk.lay.py[i] - lk.lay.ph[i];
            lk.lay.dy1[i] = lk.lay.py[i] + 2.0f;
        } else {
            lk.lay.dx[i] = lk.lay.boat ? lk.lay.bx - lk.lay.bl * 0.3f : lk.lay.px[0] - 2.0f;
            lk.lay.dy[i] = lk.lay.boat ? lk.lay.by : lk.lay.py[0] - lk.lay.ph[0];
            lk.lay.dy1[i] = lk.lay.boat ? lk.lay.by + lk.lay.bw * 0.5f + 3.0f : lk.lay.py[0] + 2.0f;
        }
    }
    for (i = 0; i < LK_NPOOL; i++) {
        lk.lay.plz[i] = 1.1f + 3.5f * (float)lk_h(i, 15) * (1.0f / 255.0f);
        lk.lay.plX[i] = (0.2f + 0.8f * (float)lk_h(i, 16) * (1.0f / 255.0f)) * (i & 1 ? 1.0f : -1.0f) * lk.lay.plz[i] * 0.5f;
        lk.lay.plr[i] = 0.25f + 0.3f * (float)lk_h(i, 17) * (1.0f / 255.0f);
    }
    for (i = 0; i < LK_NSTAR; i++) {
        lk_st[i].x = 2.0f + 236.0f * (float)lk_h(i, 41) * (1.0f / 255.0f);
        lk_st[i].y = 4.0f + (ys - 16.0f) * (float)lk_h(i, 42) * (1.0f / 255.0f);
        lk_st[i].br = (uint8_t)(lk_h(i, 43) % 3u);
    }
    for (i = 0; i < LK_NCRACK; i++) {                 /* the old cracks: each run some way already */
        lk_crack_new(i);
        lk_k[i].n = (int8_t)(1 + (int)(sc_rnd() % 5u));
    }
}

static void lk_flake_new(int i, int from_left)
{
    float u = sc_frand();
    lk_f[i].on = 1;
    lk_f[i].x = from_left ? -6.0f : -10.0f + 260.0f * sc_frand();
    lk_f[i].y = from_left ? 10.0f + 160.0f * sc_frand() : -3.0f - 4.0f * sc_frand();
    lk_f[i].vx = 0.0f;
    lk_f[i].vy = 8.0f + 10.0f * u;
    lk_f[i].ph = sc_frand();
    lk_f[i].size = u > 0.55f ? 1.0f : 0.0f;          /* (the nearer it lands, the larger, the faster) */
    lk_f[i].land_y = lk.wind > 0.55f ? 300.0f : LK_HOR + 3.0f + 150.0f * u * u;
}

/* the look the mood asks for now (Long Night: between two of its day's), and what holds now (lk_char) */
static void lk_look(float dt, int at_once)
{
    const lk_mood_t *a = &LK_MOOD[lk_mood_now()], *b = a;
    float u = 0.0f, e = at_once ? 1.0f : fm_minf(1.0f, dt * 0.3f), v;
    if (a->scale < 0) {
        float f = lk.phase * 4.0f;
        int si = (int)f;
        si = si > 3 ? 3 : si;
        u = fm_clampf((f - (float)si - 0.7f) * (1.0f / 0.3f), 0.0f, 1.0f);
        u = u * u * (3.0f - 2.0f * u);
        a = &LK_MOOD[LK_LN[si]];
        b = &LK_MOOD[LK_LN[(si + 1) & 3]];
        cd_light(a->scale);                           /* (taken when the notes rest, within a minute) */
    }
    lk_char(u < 0.5f ? a : b);
#define LK_EASE(dst, ca, cb) do { \
        uint32_t m_ = r8_mix(ca, cb, u); \
        (dst)[0] += ((float)(m_ >> 16 & 255u) - (dst)[0]) * e; \
        (dst)[1] += ((float)(m_ >> 8 & 255u) - (dst)[1]) * e; \
        (dst)[2] += ((float)(m_ & 255u) - (dst)[2]) * e; \
    } while (0)
    LK_EASE(lk.sky0, a->sky0, b->sky0);
    LK_EASE(lk.sky1, a->sky1, b->sky1);
    LK_EASE(lk.ice, a->ice, b->ice);
    LK_EASE(lk.snow, a->snow, b->snow);
    LK_EASE(lk.haze, a->haze, b->haze);
    LK_EASE(lk.pine, a->pine, b->pine);
    LK_EASE(lk.water, a->water, b->water);
#undef LK_EASE
#define LK_EASE1(dst, fa, fb, sc) do { v = ((float)(fa) + ((float)(fb) - (float)(fa)) * u) * (sc); (dst) += (v - (dst)) * e; } while (0)
    LK_EASE1(lk.light, a->light, b->light, 0.01f);
    LK_EASE1(lk.white, a->white, b->white, 0.01f);
    LK_EASE1(lk.pool, a->pool, b->pool, 0.01f);
    LK_EASE1(lk.nightamt, a->night, b->night, 0.01f);
    LK_EASE1(lk.dripamt, a->drips, b->drips, 0.01f);
    LK_EASE1(lk.smokeamt, a->smoke, b->smoke, 0.01f);
    LK_EASE1(lk.band, a->band, b->band, 0.01f);
    {   /* the snow and the wind's base: the knobs (a mood sets them); in Long Night its day's, scaled by the knobs
         * from their defaults */
        float sk = lk_long() ? (float)lk.kv[KL_SNOW] * (1.0f / 30.0f) : 1.0f, wk = lk_long() ? (float)lk.kv[KL_WIND] * (1.0f / 20.0f) : 1.0f;
        float st = lk_long() ? ((float)a->snowf + ((float)b->snowf - (float)a->snowf) * u) * 0.01f * sk : (float)lk.kv[KL_SNOW] * 0.01f,
              wt = lk_long() ? ((float)a->wind + ((float)b->wind - (float)a->wind) * u) * 0.01f * wk : (float)lk.kv[KL_WIND] * 0.01f;
        st = fm_maxf(st, lk.flurry > 0.0f ? 1.0f : 0.0f);
        lk.snowf += (fm_minf(1.0f, st) - lk.snowf) * e;
        lk.wbase += (fm_minf(1.0f, wt) - lk.wbase) * e;
    }
#undef LK_EASE1
    {   /* the voice's vowel: the mood's, opened by the wind; its tone and breath */
        int vw = (int)((float)a->vowel + 30.0f * lk.wind + 5.0f * fm_sin_turns(lk.t * (1.0f / 47.0f)));
        vw = vw < 0 ? 0 : vw > 127 ? 127 : vw;
        if (lk.kv[KL_VOICES] > 0 && (vw - lk.vowel_sent >= 4 || lk.vowel_sent - vw >= 4)) {
            fm_part_knobs(2, vw, 12, a->buzz, a->breath);
            lk.vowel_sent = vw;
        }
    }
}

static void lk_mood_tune(void)
{
    const lk_mood_t *m = &LK_MOOD[lk_mood_now()];
    cd_tune(m->scale, (float)m->key);
}

SC_SMALL static void lk_defaults(int16_t *kv)
{
    int i;
    for (i = 0; i < SC_NKNOB; i++)
        kv[i] = LK_KNOB[i][3];
}

SC_SMALL static void lk_enter(uint32_t seed, int16_t *kv)
{
    int i;
    memset(&lk, 0, sizeof lk);
    memset(lk_f, 0, sizeof lk_f);
    memset(lk_k, 0, sizeof lk_k);
    memset(lk_s, 0, sizeof lk_s);
    memset(lk_d, 0, sizeof lk_d);
    memset(lk_sm, 0, sizeof lk_sm);
    memset(lk_q, 0, sizeof lk_q);
    lk.kv = kv;
    for (i = 0; i < SC_NKNOB; i++)
        if (LK_KNOB[i][1] > LK_KNOB[i][0] && (kv[i] < LK_KNOB[i][0] || kv[i] > LK_KNOB[i][1]))
            kv[i] = LK_KNOB[i][3];
    lk.seed = seed | 1u;
    sc_seed(lk.seed * 2654435761u + 13u);
    lk_layout();
    lk.phase = (float)(seed % 1000u) * 0.001f;        /* (Long Night: wherever the day is) */
    lk.pal_lv = lk.air_sent = lk.trick_sent = lk.sing_sent = -1;
    lk.vowel_sent = -100;
    lk.gnext = 6.0f + 10.0f * sc_frand();
    lk.crack_next = 6.0f + 10.0f * sc_frand();
    lk.groan_next = 12.0f + 10.0f * sc_frand();
    lk.toll_next = 8.0f + 12.0f * sc_frand();
    lk.drip_next = 2.0f + 3.0f * sc_frand();
    r8_dark(LK_DARK);
    cd_init(seed);
    cd_activity((float)kv[KL_ACT] * 0.01f);
    cd_echo_amount(0.0f);                             /* (the echo is the tape echo: lk_fx) */
    cd_change_mode(1);
    lk_mood_tune();
    fm_part_patch(1, LK_TONE_SLOT[kv[KL_TONE] % LK_NTONE]);
    fm_part_release(1, 2600);
    lk_drone();
    fm_part_patch(3, ZN_RIN);                         /* touches, the fractures' whistles */
    fm_part_release(3, 2400);
    fm_part_patch(4, ZN_SOFT_SWELL);                  /* the swell after the bell */
    fm_part_release(4, 3500);
    fm_part_patch(5, FM_ENG_PHYS + 1);                /* the reeds: STRING, bowed, a little stiff, long */
    fm_part_knobs(5, 50, 92, 70, 40);
    fm_part_attack(5, 1100);
    fm_part_release(5, 2800);
    fm_part_patch(7, FM_ENG_SAMPLE + 0);              /* the ice's groan: SAMPLE, the recording as a tape loop */
    fm_part_knobs(7, 0, 110, 0, 0);
    fm_part_attack(7, 1800);
    fm_part_release(7, 3200);
    lk_look(0.0f, 1);
    lk_mix_levels();
    cd_drone(1);
    fm_bed_style(FM_BED_STREAM, FM_WATER_TRICKLE);    /* (the meltwater at the shore) */
    fm_bed(FM_BED_STREAM, 0);
    fm_bed(FM_BED_RAIN, 0);
    fm_bed_style(FM_BED_WIND, FM_AIR_BREEZE_SYNTH);   /* (the air: synthesized; the recordings hissed) */
    fm_bed(FM_BED_WIND, kv[KL_AIR]);
    cd_melody(0);
    cd_arc_amount((float)kv[KL_ARC] * 0.01f);
    cd_depth_send(kv[KL_DEPTH]);
}

SC_SMALL static void lk_leave(void)
{
    int b;
    cd_stop();
    fm_tape(0);
    fm_tape_knobs(0, 0, 0, 0);
    fm_fx_send(0);
    fm_delay(88, 64, 50, 0);                          /* (as fm_init leaves them: nothing runs) */
    fm_grain(92, 46, 60, 0);
    fm_grain_freeze(0);
    fm_depth(0);
    r8_dark(0);
    for (b = 0; b < FM_NBED; b++)
        fm_bed(b, 0);
}

static void lk_step(uint32_t dt_ms)
{
    float dt = (float)dt_ms * 0.001f, a = (float)lk.kv[KL_ACT] * 0.01f;
    int i;
    lk.t += dt;
    lk.ms += dt_ms;
    if (lk_long()) {                                  /* Long Night: the day goes round */
        lk.phase += dt * (1.0f / 1800.0f);
        if (lk.phase >= 1.0f) {
            lk.phase -= 1.0f;
            lk.nights++;
            cd_dawn();
        }
    }
    if (lk.flurry > 0.0f)
        lk.flurry -= dt;
    lk_look(dt, 0);
    /* the wind: its base and the gusts, each rising over 1.5..3 s, blowing 1..3 s, dying over 5..9 s; a gust's peak
     * sings the reeds */
    if (lk.gph == 0) {
        float gap = lk.ch->gust ? (float)lk.ch->gust : 0.0f;
        if (gap > 0.0f && (lk.gnext -= dt) <= 0.0f) {
            lk.gph = 1;
            lk.gt = 0.0f;
            lk.grise = 1.5f + 1.5f * sc_frand();
            lk.ghold = 1.0f + 2.0f * sc_frand();
            lk.gfall = 5.0f + 4.0f * sc_frand();
            lk.gpeak = fm_minf(1.0f, (0.35f + 0.65f * sc_frand()) * (0.5f + 0.5f * lk.wbase + 0.3f));
            lk.gnext = gap * (0.6f + 0.8f * sc_frand()) * (lk_long() ? 1.0f : 20.0f / (float)(lk.kv[KL_WIND] + 5));
        }
    } else {
        lk.gt += dt;
        if (lk.gph == 1) {
            lk.gust = lk.gpeak * (0.5f - 0.5f * fm_sin_turns(0.25f + 0.5f * lk.gt / lk.grise));
            if (lk.gt >= lk.grise) {
                lk.gph = 2;
                lk.gt = 0.0f;
                lk.gusts++;
                if (lk.gpeak >= 0.4f)
                    lk_sing(lk.gpeak);
            }
        } else if (lk.gph == 2) {
            lk.gust = lk.gpeak;
            if (lk.gt >= lk.ghold) {
                lk.gph = 3;
                lk.gt = 0.0f;
            }
        } else {
            lk.gust = lk.gpeak * (0.5f + 0.5f * fm_sin_turns(0.25f + 0.5f * lk.gt / lk.gfall));
            if (lk.gt >= lk.gfall) {
                lk.gph = 0;
                lk.gust = 0.0f;
            }
        }
    }
    lk.wind = fm_minf(1.0f, lk.wbase * 0.6f + lk.gust * (0.5f + 0.5f * lk.wbase) + 0.04f * fm_sin_turns(lk.t * 0.07f));
    {
        int m = (int)(8.0f + 80.0f * lk.wind);
        if (m / 4 != lk.air_sent / 4) {
            fm_bed_mod(FM_BED_WIND, m);
            lk.air_sent = m;
        }
    }
    if (lk.sing_left > 0.0f) {                        /* the reeds singing: rising a little with the wind */
        int hs = (int)(lk.wind * 12.0f);
        lk.sing_left -= dt;
        if (hs != lk.sing_sent) {
            lk.sing_sent = hs;
            fm_voice_glide(LK_ID_SING, lk.sing_c + 50.0f * lk.wind - 10.0f, 600);
        }
    }
    {   /* the meltwater's trickle at the shore (sent as it changes) */
        int tr = (int)(lk.dripamt * 10.0f);              /* (at 8 at most: its drops are broadband) */
        if (tr / 2 != lk.trick_sent / 2) {
            lk.trick_sent = tr;
            fm_bed(FM_BED_STREAM, tr);
            fm_bed_mod(FM_BED_STREAM, 30);
        }
    }
    /* the snow: flakes let go above (in a wind, in from the left too), drifting down and across; one that lands on the
     * near ice is a faint touch, where the snow is light */
    lk.flake_acc += dt * (0.2f + 16.0f * lk.snowf) * (1.0f + 1.5f * lk.gust);
    while (lk.flake_acc >= 1.0f) {
        lk.flake_acc -= 1.0f;
        for (i = 0; i < LK_NFLAKE; i++)
            if (!lk_f[i].on) {
                lk_flake_new(i, lk.wind > 0.5f && sc_frand() < 0.6f);
                break;
            }
    }
    for (i = 0; i < LK_NFLAKE; i++) {
        if (!lk_f[i].on)
            continue;
        lk_f[i].vx += ((lk.wind * 90.0f + 6.0f * fm_sin_turns(lk.t * 0.5f + lk_f[i].ph)) - lk_f[i].vx) * fm_minf(1.0f, dt * 1.5f);
        lk_f[i].vy += ((8.0f + 10.0f * lk_f[i].size + 4.0f * fm_fabsf(fm_sin_turns(lk.t * 0.7f + lk_f[i].ph * 2.0f))) - lk_f[i].vy) * fm_minf(1.0f, dt * 2.0f);
        lk_f[i].x += lk_f[i].vx * dt;
        lk_f[i].y += lk_f[i].vy * dt;
        if (lk_f[i].x > 252.0f || lk_f[i].x < -14.0f || lk_f[i].y > 244.0f) {
            lk_f[i].on = 0;
            continue;
        }
        if (lk_f[i].y >= lk_f[i].land_y) {
            lk_f[i].on = 0;
            if (lk_f[i].land_y > 150.0f && lk.ch->snowf > 0 && lk.ch->snowf < 60 && !cd_quiet() && sc_frand() < 0.05f + 0.25f * a) {
                if (cd_touch(lk_f[i].x * (1.0f / 240.0f), 0.35f))
                    lk.touches++;
            }
        }
    }
    /* the snow's streaks in a squall: across, fast, a little down */
    {
        int want = lk.wind > 0.45f ? (int)((lk.wind - 0.45f) * (1.0f / 0.55f) * (float)LK_NSQ * fm_minf(1.0f, lk.snowf * 2.0f)) : 0;
        for (i = 0; i < LK_NSQ; i++) {
            if (lk_s[i].on) {
                lk_s[i].x += (120.0f + 200.0f * lk.wind) * dt;
                lk_s[i].y += 20.0f * dt;
                if (lk_s[i].x - lk_s[i].l > 242.0f || lk_s[i].y > 242.0f)
                    lk_s[i].on = 0;
            } else if (i < want && sc_frand() < dt * 2.0f) {
                lk_s[i].on = 1;
                lk_s[i].x = -4.0f;
                lk_s[i].y = 4.0f + 230.0f * sc_frand();
                lk_s[i].l = 6.0f + 10.0f * sc_frand();
            }
        }
    }
    /* the ice: the cracks (how often: the mood's, the knob's and the activity's), the groans, the whistles */
    for (i = 0; i < LK_NCRACK; i++)
        if (lk_k[i].flash > 0.0f)
            lk_k[i].flash -= dt;
    if (lk.kv[KL_CRACKS] > 0 && lk.ch->cracks > 0 && (lk.crack_next -= dt) <= 0.0f) {
        float rate = (float)lk.ch->cracks * 0.01f * (lk_long() ? 1.0f : (float)lk.kv[KL_CRACKS] * (1.0f / 30.0f)) * (0.5f + a);
        lk.crack_next = (6.0f + 70.0f / (0.15f + rate)) * (0.5f + sc_frand()) * 0.1f;
        lk_crack((int)(sc_rnd() % LK_NCRACK), 0.3f + 0.7f * sc_frand());
    }
    if (lk.groan_left > 0.0f)
        lk.groan_left -= dt;
    lk.glow += ((lk.groan_left > 0.0f ? 1.0f : 0.0f) - lk.glow) * fm_minf(1.0f, dt * 0.5f);
    if (lk.ch->groans && (lk.groan_next -= dt) <= 0.0f) {
        float st = 0.3f + 0.7f * sc_frand();
        lk.groan_next = (float)lk.ch->groans * (0.7f + 0.6f * sc_frand());
        lk_groan(st);
        if (st > 0.6f)                                /* (a strong one: a fracture runs with it, silent) */
            lk_q_add(1500u + (uint32_t)(2000.0f * sc_frand()), -1, 0, 0.0f, 0, 0, 0);
    }
    /* the keep's bell: on its time; its swell */
    if (lk.ch->tolls && lk.kv[KL_BELL] > 0 && (lk.toll_next -= dt) <= 0.0f) {
        lk.toll_next = (float)lk.ch->tolls * (0.7f + 0.6f * sc_frand());
        lk_toll(1 + (int)(sc_rnd() % 3u), lk.ch->white >= 50);
    }
    /* the meltwater: drips from the posts and the boat, each heard where it lands, a ring there; smoke from the keep */
    if (lk.dripamt > 0.05f && (lk.drip_next -= dt) <= 0.0f) {
        lk.drip_next = (1.5f + 6.0f * (1.0f - lk.dripamt)) * (0.5f + sc_frand()) / (0.4f + a);
        for (i = 0; i < LK_NDRIP; i++)
            if (!lk_d[i].on) {
                lk_d[i].on = 1;
                lk_d[i].x = lk.lay.dx[i];
                lk_d[i].y = lk.lay.dy[i];
                lk_d[i].y1 = lk.lay.dy1[i];
                lk_d[i].v = 0.0f;
                lk_d[i].ring = 0.0f;
                break;
            }
    }
    for (i = 0; i < LK_NDRIP; i++)
        if (lk_d[i].on) {
            if (lk_d[i].ring > 0.0f) {
                lk_d[i].ring -= dt * 1.6f;
                if (lk_d[i].ring <= 0.0f)
                    lk_d[i].on = 0;
                continue;
            }
            lk_d[i].v += 300.0f * dt;
            lk_d[i].y += lk_d[i].v * dt;
            if (lk_d[i].y >= lk_d[i].y1) {
                lk_d[i].y = lk_d[i].y1;
                lk_d[i].ring = 1.0f;
                lk.drips++;
                fm_drop(40 + (int)(sc_rnd() % 22u), cd_pan(lk_d[i].x * (1.0f / 240.0f)), 0);
                if (sc_frand() < 0.3f && !cd_quiet())
                    cd_touch(lk_d[i].x * (1.0f / 240.0f), 0.4f);
            }
        }
    for (i = 0; i < LK_NSMOKE; i++) {
        if (lk_sm[i].life > 0.0f) {
            lk_sm[i].life -= dt;
            lk_sm[i].y -= (3.0f + 2.0f * sc_frand()) * dt;
            lk_sm[i].x += (2.0f + 20.0f * lk.wind) * dt;
        } else if (lk.smokeamt > 0.1f && sc_frand() < dt * 0.5f * lk.smokeamt) {
            lk_sm[i].life = 5.0f + 3.0f * sc_frand();
            lk_sm[i].x = lk.lay.chim;
            lk_sm[i].y = lk.lay.ktop - 3.0f;
        }
    }
    cd_step(dt_ms);
    for (i = 0; i < LK_NQ; i++)
        if (lk_q[i].at && (int32_t)(lk.ms - lk_q[i].at) >= 0) {
            if (lk_q[i].id == 0)                      /* (the swell after the bell) */
                cd_swell(fm_clampf(lk.lay.kx * (1.0f / 240.0f), 0.0f, 1.0f), 0.45f, 6000);
            else if (lk_q[i].id == 1)                 /* (a stroke of the bell: vel, pan, its rate in dur) */
                fm_shot(3, FM_SHOT_TOLL, 0, lk_q[i].vel, lk_q[i].pan, lk_q[i].dur);
            else if (lk_q[i].id < 0) {                /* (a fracture with the groan: the crack runs, silent) */
                int c = (int)(sc_rnd() % LK_NCRACK);
                if (lk_k[c].n >= LK_NSEG)
                    lk_crack_new(c);
                lk_k[c].n++;
                lk_k[c].flash = 0.5f;
            } else
                fm_voice(lk_q[i].id, lk_q[i].part, lk_q[i].cents, lk_q[i].vel, lk_q[i].pan, lk_q[i].dur);
            lk_q[i].at = 0;
        }
    cd_depth_send(lk.kv[KL_DEPTH]);
    {   /* the palette, when the look has moved (a step of 1/32, at most four times a second: every color changes,
         * so the whole screen is sent) */
        int lv = (int)(lk.light * 32.0f) + ((int)lk.sky1[1] / 6) * 64 + ((int)(lk.white * 12.0f) + (int)(lk.nightamt * 12.0f) * 16 +
                 (int)(lk.pool * 8.0f) * 256 + (int)(lk.glow * 6.0f) * 4096) * 4096 + ((int)lk.ice[2] / 8) * 65536 * 64 + (int)(lk.band * 8.0f),
            d = lv - lk.pal_lv;
        if (lk.pal_lv < 0 || !dt_ms || (lv != lk.pal_lv && (lk.ms - lk.pal_ms >= 250u || d > 2 || d < -2))) {
            lk.pal_lv = lv;
            lk.pal_ms = lk.ms;
            lk_palette();
        }
    }
}

SC_SMALL static void lk_key(int key, int on)
{
    int b, i;
    if (!on)
        return;
    for (b = 0; b < FM_NBLACK; b++)
        if (BLACK_K[b] == key) {
            if (b < 3) {                              /* a crack: near, in the middle, far */
                int best = 0;
                float want = b == 0 ? 0.9f : b == 1 ? 2.5f : 8.0f, bd = 1e9f;
                for (i = 0; i < LK_NCRACK; i++) {
                    float d = fm_fabsf(fm_log2f(lk_k[i].z[lk_k[i].n < LK_NSEG ? lk_k[i].n : LK_NSEG]) - fm_log2f(want));
                    if (d < bd) {
                        bd = d;
                        best = i;
                    }
                }
                lk_crack(best, 0.8f);
            } else if (b == 3) {                      /* the ice groans */
                lk_groan(0.7f);
            } else if (b < 7) {                       /* the bell: one, two, three strokes */
                lk_toll(b - 3, 0);
            } else if (b == 7) {                      /* a gust */
                lk.gph = 1;
                lk.gt = 0.0f;
                lk.grise = 1.4f;
                lk.ghold = 2.0f;
                lk.gfall = 7.0f;
                lk.gpeak = 1.0f;
            } else if (b == 8) {                      /* a drip */
                lk.dripamt = fm_maxf(lk.dripamt, 0.2f);
                lk.drip_next = 0.0f;
            } else if (b == 9) {                      /* a flurry */
                lk.flurry = 20.0f;
            } else {                                  /* the ice gives: a groan and the main crack runs its length */
                lk_groan(1.0f);
                lk_k[0].n = 0;
                lk_crack_new(0);
                lk_crack(0, 1.0f);
                lk_q_add(600u, -1, 0, 0.0f, 0, 0, 0);
                lk_q_add(1300u, -1, 0, 0.0f, 0, 0, 0);
            }
            lk.key_note = b;
            return;
        }
    for (b = 0; b < FM_NWHITE; b++)
        if (WHITE_K[b] == key) {
            int k = b + 2;                            /* (sixteen notes, each its own, rising) */
            float x = 12.0f + 216.0f * (float)b / (float)(FM_NWHITE - 1);
            cd_note_key(k, x * (1.0f / 240.0f), 0.5f + 0.25f * (1.0f - (float)b / (float)(FM_NWHITE - 1)), 1400, 1, 0u);
            lk.key_note = k;
            {                                         /* a crack runs where it lands, silent */
                int c = 1 + (int)(sc_rnd() % (LK_NCRACK - 1));
                if (lk_k[c].n >= LK_NSEG)
                    lk_crack_new(c);
                lk_k[c].n++;
                lk_k[c].flash = 0.35f;
            }
            return;
        }
}

SC_SMALL static void lk_knob(int i, int32_t e)
{
    if (i == KL_MOOD) {                               /* a mood: every knob to the sound, its key, scale, look */
        (void)e;
        lk_mood_tune();
        cd_activity((float)lk.kv[KL_ACT] * 0.01f);
        cd_arc_amount((float)lk.kv[KL_ARC] * 0.01f);
        fm_part_patch(1, LK_TONE_SLOT[lk.kv[KL_TONE] % LK_NTONE]);
        lk_drone();
        lk_mix_levels();
        lk.ch = 0;                                    /* (lk_look takes the new mood's events at the next step) */
        lk_look(0.0f, 0);
        lk_fx();
        return;
    }
    if (i == KL_SPACE) {
        knob_set(P_REVERB, proj.par[P_REVERB] + (int)(e * sc_knob_mul));
        return;
    }
    if (i < 0 || i >= SC_NKNOB || LK_KNOB[i][1] <= LK_KNOB[i][0])
        return;
    if (i == KL_TONE || i == KL_VOICES)
        sc_knob_wrap(&lk.kv[i], e, LK_KNOB[i][0], LK_KNOB[i][1]);
    else
        sc_knob_turn(&lk.kv[i], e, LK_KNOB[i][0], LK_KNOB[i][1], LK_KNOB[i][2]);
    switch (i) {
    case KL_ACT: cd_activity((float)lk.kv[KL_ACT] * 0.01f); break;
    case KL_TONE: fm_part_patch(1, LK_TONE_SLOT[lk.kv[KL_TONE] % LK_NTONE]); break;
    case KL_VOICES: lk_drone(); break;
    case KL_ECHO: case KL_TAPE: lk_fx(); break;
    case KL_ARC: cd_arc_amount((float)lk.kv[KL_ARC] * 0.01f); break;
    case KL_NOTES: case KL_DRONE: case KL_AIR: case KL_WIND: case KL_CRACKS: lk_mix_levels(); break;
    default: break;                                   /* (Bell, Snow, Warmth: read each step) */
    }
}

SC_SMALL static void lk_knob_text(int i, char *name, char *val)
{
    static const char *const N[SC_NUSED] = {"Mood", "Activity", "Instrument", "Space", "Notes", "Drone", "Air", "Bell",
                                            "Voices", "Cracks", "Snow", "Wind", "Echo", "Warmth", "Tape", "Tide"};
    if (i < 0 || i >= SC_NUSED || (i != KL_SPACE && LK_KNOB[i][1] <= LK_KNOB[i][0])) {
        name[0] = val[0] = 0;
        return;
    }
    copy_str(name, N[i], 11);
    switch (i) {
    case KL_MOOD: copy_str(val, LK_MOOD[lk_mood_now()].name, 11); break;
    case KL_TONE: copy_str(val, LK_TONE_NAME[lk.kv[KL_TONE] % LK_NTONE], 11); break;
    case KL_VOICES: copy_str(val, LK_CHOIR_NAME[lk.kv[KL_VOICES] % LK_NCHOIR], 11); break;
    case KL_SPACE: itoa_u((uint32_t)proj.par[P_REVERB], val); break;
    default: itoa_u((uint32_t)lk.kv[i], val); break;
    }
}

/* a new shore (PLAY; the glue fades through dark): another keep, other posts, the boat or not, other cracks and
 * pools; the world goes on */
SC_SMALL static void lk_regrow(uint32_t seed)
{
    lk.seed = seed | 1u;
    lk_layout();
    lk_background();
}

SC_SMALL static void lk_mood_kv(int m, int16_t *kv)
{
    lk_defaults(kv);
    m = sc_mood_base(m, LK_NMOOD);
    kv[KL_MOOD] = (int16_t)m;
    sc_mood_set(kv, LK_MOODS[m], LK_NSET);
}
static const char *lk_mood_name(int m) { return LK_MOOD[sc_mood_base(m, LK_NMOOD)].name; }

static const scene_t SCENE_LAKE = {"Lake", 0, lk_defaults, lk_enter, lk_leave, lk_background, lk_step, lk_draw, lk_key,
                                   lk_knob, lk_knob_text, lk_regrow, LK_SOUND, 0, 0, LK_NMOOD, lk_mood_kv, lk_mood_name, 0};
