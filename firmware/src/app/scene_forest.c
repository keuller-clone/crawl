/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 the Crawl contributors */
/* Forest Ruins: a drowned hamlet in a marsh wood, seen from a little above. Bare trees, pale trunks and fine dark
 * crowns, stand all through the clearing and fade back into a blue haze; the ground is wet, with standing water
 * that holds the sky, and tufts of ochre marsh grass; what is left of the place is wood: a hut fallen in under its
 * thatch, a line of fence posts, a cart wheel in the water, a hollow log, and a stone well. Leaves come down and
 * land on the ground or on the water, crows keep the posts and the roof, fog lies among the trunks, the wind sings
 * in the hollow things. What is heard is what is seen:
 *
 *   a leaf lands            a note (part 1), its pitch where it lands across; on the water a ring, a plip and a
 *                           soft touch (part 3)
 *   a crow calls            its own recorded caw (CC0: a hooded crow, a rook), heard where it perches or flies
 *   wind in a hollow        the well shaft sings, the hut's doorway, the log: a bowed string (part 5: PHYS, STRING,
 *                           bowed), one note each, the wider the lower, rising a little with the gust; leaves rush
 *                           past it
 *   a fog bank drifts past the well       a swell (part 4)
 *   lightning               the marsh lit white, a bolt; the thunder after it (part 7: PHYS, MEMBRANE, low), the
 *                           crows up off the posts all at once (the flock's recording)
 *   a stone falls from the hut's wall     a dull knock where it lands (part 6: PHYS, MODAL, inharmonic), dust
 *   a bat crosses the moon  a faint high tick (part 3)
 *   under them              the drone (part 2: a pad, or VOICE: a choir; at night one hollow voice whose vowel the
 *                           wind opens), the air (the synthesized breeze: the recordings hissed), in a storm rain
 *                           and a deep roar
 *
 *   KNOB 1 Mood   2 Activity   3 Instrument (Drop, Mallet, Bell, Bamboo)   4 Space (reverb)
 *   Volume: Notes, Drone, Air, Harp (the hollows' singing).   Nature: Voices (Pad, Solo, Monks, Nuns, Cathedral),
 *   Crows (how often they call; 0: never), Fog, Wind.   Music: Echo (the tape echo), Warmth, Tape, Tide
 *   White keys  a note each, rising; a leaf lets go above it and falls
 *   Black keys  the first four: a crow each calls (one not here calls far off); the next three: the well, the
 *               doorway and the log sing; then a gust; a fog bank rolling in; a stone falling; the top one: lightning
 *
 * Moods, each its own sound and picture (FR_MOOD and FR_MOODS, data):
 *   Dawn Mist     the marsh white with mist, a pale sun low in it, the water pale; the fog's banks are swells over
 *                 a soft pad; a leaf now and then; one crow far off in the trees; no wind
 *   Overgrown     the grass green and the trees in leaf, the water green; leaves come down thick and each is a
 *                 note; three crows on the roof and the posts hopping and calling; a cathedral choir
 *   Storm         a slate sky, rain on the water, the wind driving the leaves across; the hollows sing without
 *                 pause; lightning lights the marsh white, thunder, the crows wheeling and crying; a deep pad, a
 *                 roar under it
 *   Owl Moon      the moon and the stars over the trees and in the water, bats, thin wisps of fog, no crows: an
 *                 owl; stones fall from the hut; one hollow voice, its vowel the wind's; few notes, low, in the cloud
 *   Long Night    a day: Dawn Mist, Overgrown, Storm, Owl Moon, half an hour in all, then dawn again; it starts
 *                 wherever the day is
 *
 * The world is 240 x 240; drawing scales by r8_fb.w / 240 for pixel art. Every loop has a fixed bound. */
#define FR_NL 28                                /* leaves */
#define FR_NCROW 4
#define FR_NFOG 3
#define FR_NBAT 5
#define FR_NSTAR 36
#define FR_NRAIN 36
#define FR_NDUST 8
#define FR_NQ 8
#define FR_NA 3                                 /* the hollows that sing: the well, the hut's doorway, the log */
#define FR_NTREE 22
#define FR_NWAT 4                               /* pools of standing water */
#define FR_NTUFT 8                              /* tufts of grass near us, swaying (the far ones are drawn once) */
#define FR_NRIP 6                               /* rings on the water */
#define FR_NPERCH 6
#define FR_NTONE 4
#define FR_NCHOIR 5
#define FR_SOUND 1
#define FR_DARK 80                              /* the panel's dark curve (render8.c r8_dark), as Crypt's */
#define FR_HORIZON 74.0f                        /* the far edge of the ground, where the trees go into the haze */
#define FR_NSKY 32
#define FR_NST 32                               /* the stone's shades */
enum { FR_SKY = 0, FR_TRK = 32, FR_CAN = 40, FR_GND = 48, FR_WAT = 64, FR_GRS = 72, FR_WOOD = 80, FR_STONE = 88,
       FR_LEAF = 100, FR_FOG = 116, FR_CROW = 132, FR_MOON = 136, FR_HALO = 140, FR_STAR = 144, FR_SUN = 148,
       FR_RAIN = 152, FR_BOLT = 154, FR_BAT = 155, FR_DUST = 156, FR_RIP = 158 };   /* (FR_TRK: a trunk and a twig
                                                 * shade for each of four depths, far to near; FR_CAN: a canopy pair
                                                 * each; FR_GND: four tones at four depths of haze; FR_WAT: the water,
                                                 * four depths, still and rippled; FR_GRS: the grass, near and far;
                                                 * FR_WOOD: timber, its lit side, thatch, straw, rot, iron, weathered
                                                 * posts; FR_STONE: the well; FR_RIP: a ring on the water) */
#define FR_NCAW 4                               /* the hooded crow's calls in FM_SHOT_CROW; the rook's follow (zen_samples.py) */
#define FR_ID_HARP (FM_VOICE_ID + 48)           /* an arch each (three) */
#define FR_ID_THUNDER (FM_VOICE_ID + 52)        /* (two: the clap and its roll) */
#define FR_ID_TSWELL (FM_VOICE_ID + 54)
#define FR_ID_STONE (FM_VOICE_ID + 55)
#define FR_ID_TICK (FM_VOICE_ID + 56)           /* the bats' (four) */

enum { FK_MOOD, FK_ACT, FK_TONE, FK_SPACE, FK_NOTES, FK_DRONE, FK_AIR, FK_HARP, FK_VOICES, FK_CROWS, FK_FOG, FK_WIND,
       FK_ECHO, FK_DEPTH, FK_TAPE, FK_ARC };
static const int16_t FR_KNOB[SC_NKNOB][4] = {   /* lo, hi, step, default; SPACE is the reverb (P_REVERB) */
    {0, 4, 1, 0}, {0, 100, 1, 25}, {0, FR_NTONE - 1, 1, 0}, {0, 0, 0, 0},
    {0, 100, 1, 50}, {0, 100, 1, 40}, {0, 100, 1, 20}, {0, 100, 1, 50},
    {0, FR_NCHOIR - 1, 1, 0}, {0, 100, 1, 40}, {0, 100, 1, 30}, {0, 100, 1, 30},
    {0, 100, 1, 30}, {0, 100, 1, 20}, {0, 100, 1, 15}, {0, 100, 1, 40},
};
/* a mood: what it sounds in and how the ruin looks (the knobs it sets: FR_MOODS) */
#define FR_NMOOD 5
typedef struct {
    const char *name;
    int8_t scale;                               /* SC_* (tuning.h); -1: the round's own (Long Night) */
    int16_t key;                                /* cents from the player's key */
    uint8_t lo, n;                              /* its notes: white keys lo .. lo + n - 1 */
    uint32_t sky0, sky1, gnd, wat, grs, twig, trunk, fogc;   /* the sky's top and its horizon; the wet ground; the
                                                 * water; the grass; the trees' twigs and trunks; the fog */
    uint8_t leafpal;                            /* the leaves' colors (FR_LEAFPAL's row) */
    uint8_t light;                              /* % */
    uint8_t fog, wind, gust;                    /* % the banks' thickness; % the wind; s between gusts (0: none) */
    uint8_t leaves, crows, rain;                /* % how many fall; crows seen; % */
    uint8_t moon, sun, ivy, bats, stones, thunder, canopy;   /* (ivy and canopy %) */
    uint8_t vowel, buzz, breath;                /* the voice's (VOICE's knobs, 0..127) */
    uint8_t cloud;                              /* % GRAIN */
} fr_mood_t;
static const fr_mood_t FR_MOOD[FR_NMOOD] = {
    {"Long Night", -1, 0, 4, 9, 0xB4C8DCu, 0xECF0F2u, 0x4E5A68u, 0xC4D4E0u, 0xB89050u, 0x2A2E36u, 0xC8CCD0u, 0xDCE4EAu, 0, 90, 100, 8, 40, 12, 1, 0, 0, 1, 0, 0, 0, 0, 0, 90, 30, 0, 0},
    {"Dawn Mist", SC_AEOLIAN, 0, 5, 8, 0xB4C8DCu, 0xECF0F2u, 0x4E5A68u, 0xC4D4E0u, 0xB89050u, 0x2A2E36u, 0xC8CCD0u, 0xDCE4EAu, 0, 90, 100, 8, 40, 12, 1, 0, 0, 1, 0, 0, 0, 0, 0, 90, 30, 0, 0},
    {"Overgrown", SC_DORIAN, 0, 4, 9, 0x6890B8u, 0xC8DCD8u, 0x44503Au, 0x98B8B0u, 0x7A9A40u, 0x30322Au, 0xC0C4B8u, 0xC8D4CCu, 1, 110, 8, 20, 22, 70, 3, 0, 0, 0, 100, 0, 0, 0, 100, 60, 60, 0, 0},
    {"Storm", SC_IN, -700, 2, 8, 0x3C4650u, 0x7A8690u, 0x262A30u, 0x6A7884u, 0x6A5A38u, 0x1A1C20u, 0x8A9094u, 0x707A84u, 2, 72, 0, 65, 7, 60, 4, 70, 0, 0, 30, 0, 1, 1, 25, 40, 80, 0, 0},
    {"Owl Moon", SC_MINYO, -500, 3, 7, 0x0C1228u, 0x2E3C58u, 0x12141Cu, 0x3A4C6Cu, 0x3A3424u, 0x0A0C12u, 0x6A7284u, 0x3C4860u, 3, 70, 30, 15, 28, 6, 0, 0, 1, 0, 0, 5, 1, 0, 0, 104, 20, 22, 25},
};
static const uint8_t FR_LN[4] = {1, 2, 3, 4};   /* Long Night's day: Dawn Mist, Overgrown, Storm, Owl Moon */
#define FR_NSET 14
static const sc_set_t FR_MOODS[FR_NMOOD][FR_NSET] = {
    {{FK_ACT, 25}, {FK_TONE, 0}, {FK_NOTES, 50}, {FK_DRONE, 40}, {FK_AIR, 20}, {FK_HARP, 45}, {FK_VOICES, 0}, {FK_CROWS, 40},
     {FK_FOG, 30}, {FK_WIND, 30}, {FK_ECHO, 30}, {FK_DEPTH, 20}, {FK_TAPE, 15}, {FK_ARC, 40}},
    {{FK_ACT, 12}, {FK_TONE, 2}, {FK_NOTES, 45}, {FK_DRONE, 50}, {FK_AIR, 15}, {FK_HARP, 30}, {FK_VOICES, 0}, {FK_CROWS, 25},
     {FK_FOG, 100}, {FK_WIND, 8}, {FK_ECHO, 35}, {FK_DEPTH, 30}, {FK_TAPE, 10}, {FK_ARC, 30}},   /* (still: the pad,
                                                       * the fog's swells, a bell now and then, the crow far off) */
    {{FK_ACT, 40}, {FK_TONE, 1}, {FK_NOTES, 60}, {FK_DRONE, 40}, {FK_AIR, 14}, {FK_HARP, 50}, {FK_VOICES, 4}, {FK_CROWS, 60},
     {FK_FOG, 8}, {FK_WIND, 20}, {FK_ECHO, 20}, {FK_DEPTH, 10}, {FK_TAPE, 10}, {FK_ARC, 20}},   /* (the leaves' mallets,
                                                       * the cathedral choir, the crows at their talk) */
    {{FK_ACT, 55}, {FK_TONE, 3}, {FK_NOTES, 50}, {FK_DRONE, 45}, {FK_AIR, 50}, {FK_HARP, 70}, {FK_VOICES, 0}, {FK_CROWS, 70},
     {FK_FOG, 0}, {FK_WIND, 65}, {FK_ECHO, 40}, {FK_DEPTH, 35}, {FK_TAPE, 25}, {FK_ARC, 30}},   /* (the arches singing,
                                                       * rain, thunder, the deep pad) */
    {{FK_ACT, 15}, {FK_TONE, 2}, {FK_NOTES, 40}, {FK_DRONE, 45}, {FK_AIR, 12}, {FK_HARP, 35}, {FK_VOICES, 1}, {FK_CROWS, 0},
     {FK_FOG, 30}, {FK_WIND, 15}, {FK_ECHO, 60}, {FK_DEPTH, 40}, {FK_TAPE, 25}, {FK_ARC, 45}},   /* (one hollow voice,
                                                       * the owl, the stones, the cloud) */
};
static const uint8_t FR_TONE_SLOT[FR_NTONE] = {ZN_DROP, ZN_MALLET, ZN_BELL, ZN_BAMBOO};
static const char *const FR_TONE_NAME[FR_NTONE] = {"Drop", "Mallet", "Bell", "Bamboo"};
static const char *const FR_CHOIR_NAME[FR_NCHOIR] = {"Pad", "Solo", "Monks", "Nuns", "Cathedral"};
static const uint8_t FR_AIR_STYLE[FR_NMOOD] = {FM_AIR_BREEZE_SYNTH, FM_AIR_BREEZE_SYNTH, FM_AIR_BREEZE_SYNTH, FM_AIR_BREEZE_SYNTH,
                                               FM_AIR_BREEZE_SYNTH};   /* (the recordings read as hiss: Steven, after
                                                * listening; the synthesized breeze is low-passed, and its puffs are the gusts) */
static const uint32_t FR_LEAFPAL[4][4] = {       /* the leaves' four colors by the mood's palette */
    {0xC8B070u, 0xA88850u, 0x8C6A40u, 0xB09868u},   /* pale: dry yellow and tan */
    {0x5A8A30u, 0x8AA838u, 0xC8B040u, 0x6A9A48u},   /* green and yellow */
    {0x8A4A28u, 0xA86030u, 0x6A3C22u, 0x9A7038u},   /* brown and red */
    {0x2A3038u, 0x383C44u, 0x222830u, 0x30343Cu},   /* dark */
};

static struct {
    float x, y, vx, vy, rot, spin, ph, size, land_y, life;
    int8_t st, col, silent;                     /* 0 free, 1 falling, 2 landed (life s left) */
} fr_l[FR_NL];
static struct {
    float x, y, tx, ty, wing, call, wait, s;    /* where; heading to; its wings' beat; calling (s left); its next move */
    int8_t st, perch, dir, far, on;             /* 0 perched, 1 flying, 2 wheeling; its perch; facing; far behind the ruin */
} fr_c[FR_NCROW];
static struct {
    float x, y, w, hh, th, ph, lastx;
} fr_f[FR_NFOG];
static struct {
    float x, y, vx, vy, t;
} fr_b[FR_NBAT];
static struct {
    float x, y;
    uint8_t br;
} fr_s[FR_NSTAR];
static sc_drop_t fr_r[FR_NRAIN];
static struct {
    float x, y, vx, vy, life;
} fr_d[FR_NDUST];
static struct {
    float x, y, t;                              /* a ring on the water: where, how long it has to go */
} fr_rp[FR_NRIP];
static struct {
    uint32_t at;
    int id, part, vel, pan, dur;
    float cents;
} fr_q[FR_NQ];
static struct {
    uint32_t seed, ms;
    int16_t *kv;
    float t, phase, night, dawn;                /* Long Night's day (0..1 in half an hour); the moon's night, the sun's dawn */
    float sky0[3], sky1[3], gnd[3], wat[3], grs[3], twig[3], trunk[3], fogc[3], leaf[4][3];   /* the look now (eased
                                                 * toward the mood's) */
    float light, ivy, canopy, moonamt, sunamt, rain, fogamt, flash;
    float wind, wbase, gust, gpeak, gt, grise, ghold, gfall, gnext;   /* the wind now; its base; a gust's envelope */
    int gph;                                    /* a gust: 0 none, 1 rising, 2 blowing, 3 dying */
    int air_sent, air_style, air_want, rain_sent, stream_sent, vowel_sent, harp_sent;
    float air_switch, fog_key, rain_amt;
    float bolt_t, bolt[8][2], crow_next, owl_next, stone_next, bat_wait, bolt_next, leaf_acc;
    uint32_t thunder_at;
    struct {
        float x, y, v;
        int8_t on;
    } fall;                                     /* a stone falling from the wall */
    float harp_c[FR_NA], harp_left[FR_NA];
    sc_moon_t mn;
    float moon_top;
    struct {                                    /* this clearing (the picture's seed): */
        struct {
            float x, y, h;                      /* a tree: its foot, its height */
            uint8_t band;                       /* how far: 0 in the haze .. 3 near */
        } tree[FR_NTREE];
        int ntree;
        struct {
            float x, y, rx, ry;                 /* a pool of water: an ellipse */
        } wat[FR_NWAT];
        float hx, hy;                           /* the hut's middle and foot */
        float fx0, fy;                          /* the fence: its first post, their feet */
        float wx, wy;                           /* the well's top */
        float cx0, cy0;                         /* the cart wheel's hub */
        float lx0, ly0, lx1, ly1;               /* the log, hollow end first */
        struct {
            float x, y, h;
            int n;
        } tuft[FR_NTUFT];                       /* the grass near us */
        int na;                                 /* the hollows that sing: the well, the doorway, the log */
        float cx[FR_NA], cy[FR_NA], hw[FR_NA];  /* each one's place and width */
        float px[FR_NPERCH], py[FR_NPERCH], ps[FR_NPERCH];   /* where a crow may sit, and how near (its size) */
        int np;
    } lay;
    uint32_t tr;                                /* the trees' own randomness (the picture's, not the world's) */
    const fr_mood_t *ch;                        /* the mood whose events hold now (Long Night: its day's) */
    int pal_lv, set;
    uint32_t pal_ms;
    uint32_t leaves, caws, flocks, gusts, harps, swells, thunders, bolts, stones, owls, batmoon, nights;   /* (the host's
                                                 * tests count them) */
    int key_note;
} fr;

static float fr_scale(void) { return (float)r8_fb.w / 240.0f; }
static int fr_mood_now(void) { return sc_mood_base(fr.kv[FK_MOOD], FR_NMOOD); }
static int fr_long(void) { return FR_MOOD[fr_mood_now()].scale < 0; }
static uint32_t fr_h(int a, int b)              /* the picture's own noise: 0..255, by the seed */
{
    uint32_t h = ((uint32_t)a * 73856093u) ^ ((uint32_t)b * 19349663u) ^ fr.seed;
    h *= 2654435761u;
    return (h ^ (h >> 13)) >> 24;
}
static float fr_n1(float x, int salt)           /* smooth noise along a line, 0..1 */
{
    int i = (int)fm_floorf(x);
    float f = x - (float)i, a = (float)fr_h(i, salt), b = (float)fr_h(i + 1, salt);
    f = f * f * (3.0f - 2.0f * f);
    return (a + (b - a) * f) * (1.0f / 255.0f);
}

/* ------------------------------------------------------------ the ruin --- */
static void fr_moon_xy(float *mt, float *mx, float *my)   /* the moon on its arc over the trees: up all night */
{
    *mt = fr.night;
    *mx = 24.0f + 192.0f * *mt;
    *my = fr.moon_top - 90.0f * *mt * (1.0f - *mt);
}
static float fr_horizon(float wx) { return FR_HORIZON + 3.0f * fr_n1(wx * (1.0f / 19.0f), 41); }
static int fr_water(float wx, float wy)        /* the pool at (wx, wy), or -1 */
{
    int i;
    for (i = 0; i < FR_NWAT; i++) {
        float dx = (wx - fr.lay.wat[i].x) / fr.lay.wat[i].rx, dy = (wy - fr.lay.wat[i].y) / fr.lay.wat[i].ry;
        if (dx * dx + dy * dy < 1.0f + 0.12f * fm_sin_turns(wx * 0.05f + wy * 0.03f))
            return i;
    }
    return -1;
}
static uint32_t fr_tr(void)                     /* the trees' randomness: 0..255 */
{
    fr.tr = fr.tr * 1664525u + 1013904223u;
    return fr.tr >> 24;
}
static float fr_trf(void) { return (float)fr_tr() * (1.0f / 255.0f); }

/* a pixel; with `behind` only over palette entries below it (FR_TRK: the sky alone, for the sun, the moon and the
 * stars; FR_GND: the sky and the trees, for a crow far off) */
static inline void fr_px(surf_t *s, int32_t x, int32_t y, uint8_t c, int behind)
{
    if ((uint32_t)x < (uint32_t)s->w && (uint32_t)y < (uint32_t)s->h) {
        uint8_t *p = s->px + y * s->w + x;
        if (!behind || *p < behind)
            *p = c;
    }
}
static void fr_disc(surf_t *s, float cx, float cy, float r, uint8_t c, int behind, int dith)
{
    int32_t x, y, x0 = (int32_t)(cx - r), x1 = (int32_t)(cx + r), y0 = (int32_t)(cy - r), y1 = (int32_t)(cy + r);
    float r2 = r * r;
    for (y = y0; y <= y1; y++)
        for (x = x0; x <= x1; x++) {
            float dx = (float)x + 0.5f - cx, dy = (float)y + 0.5f - cy;
            if (dx * dx + dy * dy < r2 && (dith >= 16 || r8_dith(x, y, dith)))
                fr_px(s, x, y, c, behind);
        }
}
static void fr_line(surf_t *s, float x0, float y0, float x1, float y1, uint8_t c, int behind)
{
    float dx = x1 - x0, dy = y1 - y0, l = fm_maxf(fm_fabsf(dx), fm_fabsf(dy));
    int32_t n = (int32_t)l + 1, i;
    n = n > 64 ? 64 : n;
    for (i = 0; i <= n; i++) {
        float u = (float)i / (float)n;
        fr_px(s, (int32_t)(x0 + dx * u), (int32_t)(y0 + dy * u), c, behind);
    }
}
static void fr_ellipse(surf_t *s, float cx, float cy, float rx, float ry, uint8_t c)   /* filled */
{
    int32_t y, y0 = (int32_t)fm_floorf(cy - ry), y1 = (int32_t)fm_floorf(cy + ry);
    for (y = y0; y <= y1; y++) {
        float dy = ((float)y + 0.5f - cy) / ry, w2 = 1.0f - dy * dy;
        if (w2 > 0.0f)
            r8_hspan(s, y, (int32_t)fm_floorf(cx - rx * fm_sqrtf(w2) + 0.5f), (int32_t)fm_floorf(cx + rx * fm_sqrtf(w2) - 0.5f), c);
    }
}

/* a branch (a turns from straight up, len long, w wide at its root) and, while depth lasts, two or three from its
 * end, each a little to one side and thinner; the thin ones in the twig shade, the thick in the trunk's */
static void fr_branch(surf_t *s, float k, float x, float y, float a, float len, float w, int depth, int band)
{
    float ex = x + len * fm_sin_turns(a), ey = y - len * fm_sin_turns(a + 0.25f);
    int j, n;
    r8_limb(s, x * k, y * k, w * k, ex * k, ey * k, w * 0.55f * k, (uint8_t)(FR_TRK + 2 * band + (w < 1.4f)));
    if (depth <= 0)
        return;
    n = 2 + (int)(fr_tr() & 1u);
    for (j = 0; j < n; j++) {
        float a2 = a + ((float)j - 0.5f * (float)(n - 1)) * (0.07f + 0.05f * fr_trf()) + (fr_trf() - 0.5f) * 0.04f;
        fr_branch(s, k, ex, ey, a2, len * (0.58f + 0.1f * fr_trf()), w * 0.62f, depth - 1, band);
    }
}
/* a bare tree: a pale trunk with dark marks, two side branches, a crown of fine twigs; its canopy (a pair of
 * dithered discs, in leaf or not by the palette) */
static void fr_tree(surf_t *s, float k, float x, float y, float h, int band)
{
    float w = 1.0f + h * 0.03f, lean = (fr_trf() - 0.5f) * 0.05f, tl = h * 0.6f, tx = x + tl * fm_sin_turns(lean),
          ty = y - tl * fm_sin_turns(lean + 0.25f);
    int j, depth = band == 0 ? 2 : 3;
    r8_limb(s, x * k, y * k, w * k, tx * k, ty * k, w * 0.6f * k, (uint8_t)(FR_TRK + 2 * band));
    for (j = 0; j < 3; j++) {                         /* (the birch's marks) */
        float u = 0.15f + 0.25f * (float)j + 0.1f * fr_trf(), mx = x + (tx - x) * u, my = y + (ty - y) * u;
        r8_hspan(s, (int32_t)(my * k), (int32_t)((mx - w * 0.5f) * k), (int32_t)((mx + w * 0.3f) * k), (uint8_t)(FR_TRK + 2 * band + 1));
    }
    for (j = 0; j < 2; j++) {
        float u = 0.5f + 0.3f * (float)j, bx = x + (tx - x) * u, by = y + (ty - y) * u, side = j ? 1.0f : -1.0f;
        fr_branch(s, k, bx, by, lean + side * (0.1f + 0.08f * fr_trf()), h * 0.28f, w * 0.5f, depth - 1, band);
    }
    fr_branch(s, k, tx, ty, lean, h * 0.34f, w * 0.7f, depth, band);
    for (j = 0; j < 2; j++)
        fr_disc(s, (tx + (fr_trf() - 0.5f) * h * 0.3f) * k, (ty - h * 0.24f - fr_trf() * h * 0.12f) * k, h * (0.09f + 0.05f * fr_trf()) * k,
                (uint8_t)(FR_CAN + 2 * band + j), 0, 5);
}

/* the clearing, drawn once: the sky, the wet ground and its pools, the trees from the haze forward with the
 * hamlet's leavings among them, the far grass */
SC_SMALL static void fr_background(void)
{
    surf_t *s = &r8_bg;
    float k = fr_scale(), ik = 1.0f / k;
    int32_t x, y, i, b;
    for (y = 0; y < s->h; y++) {
        float wy = ((float)y + 0.5f) * ik;
        uint8_t *row = s->px + y * s->w;
        for (x = 0; x < s->w; x++) {
            float wx = ((float)x + 0.5f) * ik, hz = fr_horizon(wx), v, hzl, tone;
            int32_t f, hl, w;
            if (wy < hz) {                            /* the sky, pale toward the horizon */
                f = (int32_t)(wy * (float)(FR_NSKY - 1) * 16.0f * (1.0f / FR_HORIZON));
                f = f > (FR_NSKY - 1) * 16 ? (FR_NSKY - 1) * 16 : f;
                row[x] = (uint8_t)(FR_SKY + (f >> 4) + r8_dith(x, y, f & 15));
                continue;
            }
            v = fm_clampf((wy - hz) * (1.0f / (240.0f - FR_HORIZON)), 0.0f, 1.0f);
            hzl = (1.0f - v) * (1.0f - v) * 3.99f;    /* the haze: thick far off, none near */
            hl = (int32_t)hzl;
            hl += r8_dith(x, y, (int)((hzl - (float)hl) * 16.0f));
            hl = hl > 3 ? 3 : hl;
            w = fr_water(wx, wy);
            if (w >= 0) {                             /* standing water: the sky in it, a ripple here and there */
                int rp = fr_n1(wx * 0.3f + wy * 0.9f, 71 + w) > 0.72f;
                row[x] = (uint8_t)(FR_WAT + hl * 2 + rp);
                continue;
            }
            {   /* the ground: wet earth, its texture coarser near us */
                float cs = 2.0f + 5.0f * v;
                int32_t gx = (int32_t)(wx / cs), gy = (int32_t)((wy - FR_HORIZON) / cs);
                tone = (float)fr_h(gx, gy + 300) * (1.0f / 255.0f) * 0.7f + 0.3f * fr_n1(wx * 0.04f + wy * 0.02f, 72);
                f = (int32_t)(tone * 3.0f * 16.0f);
                row[x] = (uint8_t)(FR_GND + hl * 4 + (f >> 4) + r8_dith(x + 1, y, f & 15));
            }
        }
    }
    for (b = 0; b < 4; b++) {                         /* the trees, far to near, and the hamlet's leavings at their
                                                       * depths; the grass far off, a few strokes each tuft */
        for (i = 0; i < fr.lay.ntree; i++)
            if (fr.lay.tree[i].band == b)
                fr_tree(s, k, fr.lay.tree[i].x, fr.lay.tree[i].y, fr.lay.tree[i].h, b);
        if (b == 0) {                                 /* the fence: posts, some gone, a rail still on a few */
            float fy = fr.lay.fy;
            int j;
            for (j = 0; j < 9; j++) {
                float px = fr.lay.fx0 + 11.0f * (float)j;
                if ((fr_h(j, 76) % 5u) == 0u)
                    continue;
                r8_rect(s, (int32_t)(px * k), (int32_t)((fy - 7.0f) * k), (int32_t)(2.0f * k + 0.5f), (int32_t)(7.0f * k), FR_WOOD + 6);
                if (j >= 2 && j <= 5)
                    r8_limb(s, px * k, (fy - 5.0f) * k, 1.0f * k, (px + 11.0f) * k, (fy - 4.5f) * k, 1.0f * k, FR_WOOD + 6);
            }
        }
        if (b == 1) {                                 /* the hut: four posts, the roof fallen in between them */
            float hx = fr.lay.hx, hy = fr.lay.hy;
            int j;
            for (j = 0; j < 4; j++) {
                float px = hx - 27.0f + 18.0f * (float)j, ph = 12.0f + (float)(fr_h(j, 77) % 9u);
                r8_rect(s, (int32_t)(px * k), (int32_t)((hy - ph) * k), (int32_t)(3.0f * k + 0.5f), (int32_t)(ph * k), FR_WOOD);
            }
            r8_rect(s, (int32_t)((hx - 9.0f) * k), (int32_t)((hy - 13.0f) * k), (int32_t)(10.0f * k), (int32_t)(13.0f * k), FR_WOOD + 4);   /* (the doorway) */
            r8_tri(s, (hx - 32.0f) * k, (hy - 15.0f) * k, (hx + 30.0f) * k, (hy - 23.0f) * k, (hx + 24.0f) * k, (hy - 3.0f) * k, FR_WOOD + 2);
            r8_tri(s, (hx - 32.0f) * k, (hy - 15.0f) * k, (hx + 24.0f) * k, (hy - 3.0f) * k, (hx - 20.0f) * k, (hy - 6.0f) * k, FR_WOOD + 2);
            r8_tri(s, (hx - 20.0f) * k, (hy - 6.0f) * k, (hx + 2.0f) * k, (hy - 20.0f) * k, (hx + 8.0f) * k, (hy - 2.0f) * k, FR_WOOD + 4);   /* (where it caved) */
            for (j = 0; j < 7; j++) {
                float u = (float)j * (1.0f / 6.0f), sx0 = hx - 30.0f + 58.0f * u, sy0 = hy - 15.0f - 8.0f * u;
                r8_limb(s, sx0 * k, sy0 * k, 0.8f * k, (sx0 - 4.0f) * k, (sy0 + 9.0f) * k, 0.8f * k, FR_WOOD + 3);
            }
            r8_limb(s, (hx - 32.0f) * k, (hy - 15.0f) * k, 1.2f * k, (hx + 30.0f) * k, (hy - 23.0f) * k, 1.2f * k, FR_WOOD + 3);   /* (the ridge) */
        }
        if (b == 2) {                                 /* the well, and the cart wheel in the water */
            float wx = fr.lay.wx, wy = fr.lay.wy, cx = fr.lay.cx0, cy = fr.lay.cy0;
            int j;
            fr_ellipse(s, wx * k, (wy + 6.0f) * k, 13.0f * k, 4.5f * k, FR_STONE + 2);
            r8_rect(s, (int32_t)((wx - 13.0f) * k), (int32_t)(wy * k), (int32_t)(26.0f * k), (int32_t)(6.0f * k), FR_STONE + 3);
            for (j = 0; j < 6; j++)
                r8_rect(s, (int32_t)((wx - 13.0f + 4.4f * (float)j) * k), (int32_t)(wy * k), (int32_t)(1.0f * k + 0.5f), (int32_t)(6.0f * k), FR_STONE + 1);
            fr_ellipse(s, wx * k, wy * k, 13.0f * k, 4.5f * k, FR_STONE + 6);
            fr_ellipse(s, wx * k, wy * k, 9.0f * k, 3.0f * k, FR_STONE);
            r8_ring(s, cx * k, cy * k, 11.0f * k, 9.5f * k, FR_WOOD + 5);
            r8_ring(s, cx * k, cy * k, 10.0f * k, 8.5f * k, FR_WOOD + 5);
            for (j = 0; j < 6; j++)
                fr_line(s, cx * k, cy * k, (cx + 9.5f * fm_sin_turns((float)j * (1.0f / 6.0f))) * k, (cy - 8.0f * fm_sin_turns((float)j * (1.0f / 6.0f) + 0.25f)) * k,
                        FR_WOOD + 5, 0);
            r8_disc(s, cx * k, cy * k, 2.2f * k, FR_WOOD + 5);
        }
        if (b == 3) {                                 /* the log, hollow at its near end */
            r8_limb(s, fr.lay.lx1 * k, fr.lay.ly1 * k, 6.0f * k, fr.lay.lx0 * k, fr.lay.ly0 * k, 9.0f * k, FR_WOOD);
            r8_limb(s, fr.lay.lx1 * k, (fr.lay.ly1 - 2.5f) * k, 1.2f * k, fr.lay.lx0 * k, (fr.lay.ly0 - 3.8f) * k, 1.4f * k, FR_WOOD + 1);
            r8_disc(s, fr.lay.lx0 * k, fr.lay.ly0 * k, 4.5f * k, FR_WOOD + 1);
            r8_disc(s, fr.lay.lx0 * k, fr.lay.ly0 * k, 3.0f * k, FR_WOOD + 4);
        }
        for (i = 0; i < 24; i++) {                    /* the grass at this depth, a few strokes a tuft */
            float gx = (float)(fr_h(i + 30 * b, 78) % 240u), gv = ((float)b + (float)fr_h(i + 30 * b, 79) * (1.0f / 255.0f)) * 0.25f,
                  gy = fr_horizon(gx) + 4.0f + (240.0f - FR_HORIZON - 8.0f) * gv * gv, gh = 3.0f + 9.0f * gv;
            int j, n = 3 + (int)(fr_h(i, 80 + b) % 3u);
            if (fr_water(gx, gy) >= 0 || b == 3)
                continue;
            for (j = 0; j < n; j++)
                r8_limb(s, (gx + 1.2f * (float)j) * k, gy * k, 1.0f * k, (gx + 1.2f * (float)j + (float)(j - n / 2) * 1.5f) * k, (gy - gh * (0.7f + 0.3f * (float)(j & 1))) * k,
                        0.6f * k, (uint8_t)(FR_GRS + (b < 2 ? 4 : 0) + (j % 3)));
        }
    }
}

/* ------------------------------------------------------------- sounds --- */
static void fr_q_add(uint32_t delay_ms, int id, int part, float cents, int vel, int pan, int dur)
{
    int i;
    for (i = 0; i < FR_NQ; i++)
        if (!fr_q[i].at) {
            fr_q[i].at = fr.ms + delay_ms + 1u;
            fr_q[i].id = id;
            fr_q[i].part = part;
            fr_q[i].cents = cents;
            fr_q[i].vel = vel;
            fr_q[i].pan = pan;
            fr_q[i].dur = dur;
            return;
        }
}
static float fr_low(float c, float hi, float lo)     /* c brought into the octave hi .. lo (cents from A4, hi > lo) */
{
    while (c > hi)
        c -= 1200.0f;
    while (c < lo)
        c += 1200.0f;
    return c;
}

/* arch a sings: a bowed string on its chord tone (the widest arch the root), 147..294 Hz, as strong as the gust;
 * it goes on as long as the gust blows and rises a little with it (fr_step glides it); leaves rush through */
static void fr_harp(int a, float peak)
{
    int j, wide = 0, i, n = 0;
    float c;
    if (fr.kv[FK_HARP] <= 0 || fr.lay.na < 1)
        return;
    a = a < 0 ? 0 : a >= fr.lay.na ? fr.lay.na - 1 : a;
    for (j = 1; j < fr.lay.na; j++)
        if (fr.lay.hw[j] > fr.lay.hw[wide])
            wide = j;
    c = fr_low(cd_cents(CD_CHORDS[cd.chord][a == wide ? 0 : a < wide ? 1 : 2]), -700.0f, -1900.0f);
    if (a != wide && c < fr_low(cd_cents(CD_CHORDS[cd.chord][0]), -700.0f, -1900.0f))
        c += 1200.0f;                                 /* (the others above the root) */
    fr.harp_c[a] = c;
    fr.harp_left[a] = 3.0f + 5.0f * peak;
    fm_voice(FR_ID_HARP + a, 5, c, 36 + (int)(70.0f * peak), cd_pan(fr.lay.cx[a] * (1.0f / 240.0f)), (int)(fr.harp_left[a] * 1000.0f));
    fr.harps++;
    for (i = 0; i < FR_NL && n < 3; i++)              /* (three leaves through the arch) */
        if (!fr_l[i].st) {
            fr_l[i].st = 1;
            fr_l[i].x = fr.lay.cx[a] - 18.0f - 10.0f * sc_frand();
            fr_l[i].y = fr.lay.cy[a] - 16.0f - 14.0f * sc_frand();
            fr_l[i].vx = 40.0f + 30.0f * sc_frand();
            fr_l[i].vy = 2.0f;
            fr_l[i].rot = sc_frand();
            fr_l[i].spin = 1.5f + sc_frand();
            fr_l[i].ph = sc_frand();
            fr_l[i].size = 2.0f + 1.2f * sc_frand();
            fr_l[i].land_y = 300.0f;
            fr_l[i].col = (int8_t)(sc_rnd() & 3u);
            fr_l[i].silent = 1;
            n++;
        }
}

/* a crow calls: its own bird's recording (crows 0 and 2 the hooded crow, 1 and 3 the rook), a little faster or
 * slower each time, heard where it is */
static void fr_caw(int i, int vel, float x)
{
    int n = fm_shot_count(FM_SHOT_CROW), seg;
    n = n > FR_NCAW ? n : FR_NCAW + 1;
    seg = i & 1 ? FR_NCAW + (int)(sc_rnd() % (uint32_t)(n - FR_NCAW)) : (int)(sc_rnd() % (uint32_t)FR_NCAW);
    fm_shot(i & 3, FM_SHOT_CROW, seg, vel, cd_pan(x * (1.0f / 240.0f)), 93 + (int)(sc_rnd() % 15u));
    fr.caws++;
}

/* lightning: the flash, a bolt down to the wall; the thunder a while after (the distance); the crows up */
static void fr_bolt(void)
{
    int i, j;
    float x = 50.0f + 140.0f * sc_frand(), y = 0.0f;
    fr.flash = 1.0f;
    fr.bolt_t = 0.13f;
    fr.bolts++;
    for (i = 0; i < 8; i++) {
        fr.bolt[i][0] = x;
        fr.bolt[i][1] = y;
        x += (sc_frand() - 0.5f) * 26.0f;
        y += 7.0f + 6.0f * sc_frand();
    }
    fr.thunder_at = fr.ms + 500u + (uint32_t)(2600.0f * sc_frand());
    for (j = 0; j < FR_NCROW; j++)
        if (fr_c[j].on && fr_c[j].st == 0) {
            fr_c[j].st = 2;
            fr_c[j].wing = sc_frand();
            fr_c[j].tx = 20.0f + 200.0f * sc_frand();
            fr_c[j].ty = 10.0f + 60.0f * sc_frand();
            fr_c[j].wait = 12.0f + 10.0f * sc_frand();
        }
    if (fr.kv[FK_CROWS] > 0 && fr.ch->crows >= 2) {
        fm_shot(3, FM_SHOT_FLOCK, 0, 52, 64, 100);
        fr.flocks++;
    }
}

static void fr_stone_drop(void)
{
    if (fr.fall.on)
        return;
    fr.fall.on = 1;
    fr.fall.x = fr.lay.hx + (sc_frand() - 0.5f) * 44.0f;
    fr.fall.y = fr.lay.hy - 19.0f;
    fr.fall.v = 0.0f;
}

static void fr_drone(void)                      /* Voices: the drone's source, a pad or one of GR!MM's choirs */
{
    int t = fr.kv[FK_VOICES];
    if (t <= 0) {
        fm_part_patch(2, ZN_DEEP_PAD);
    } else {
        fm_part_patch(2, FM_ENG_VOICE + t - 1);
        fm_part_attack(2, 2200);
        fr.vowel_sent = -100;
    }
    fm_part_release(2, 4000);
}

static void fr_fx(void)
{
    int t = fr.kv[FK_TAPE], e = fr.kv[FK_ECHO], c = fr.ch ? fr.ch->cloud : 0;
    fm_tape(t > 50 ? 100 : t * 2);
    fm_tape_knobs(t * 70 / 100, t * 60 / 100, 20 + t * 50 / 100, t * 45 / 100);
    fm_fx_send(e || c ? 70 : 0);
    fm_delay(104, 50 + e * 30 / 100, 70, e * 90 / 100);
    fm_grain(92, 46, 60, c);
}

static void fr_mix_levels(void)
{
    fm_part_level(1, fr.kv[FK_NOTES]);
    fm_part_level(3, fr.kv[FK_NOTES]);
    fm_part_level(4, fr.kv[FK_NOTES]);
    cd_drone_level(fr.kv[FK_DRONE]);
    fm_part_level(5, fr.kv[FK_HARP]);
    if (fr.air_switch <= 0.0f)
        fm_bed(FM_BED_WIND, fr.kv[FK_AIR]);
}

/* the mood whose events hold from now (a mood chosen; Long Night passing into the next): its range, its air */
static void fr_char(const fr_mood_t *m)
{
    int i, want;
    if (m == fr.ch)
        return;
    fr.ch = m;
    cd_range(m->lo, m->n);
    want = FR_AIR_STYLE[(int)(m - FR_MOOD)];
    if (want != fr.air_style && fr.air_want != want) {   /* another recording: down, over, up again (no step) */
        fr.air_want = want;
        fm_bed(FM_BED_WIND, 0);
        fr.air_switch = 1.3f;
    }
    for (i = 0; i < FR_NCROW; i++) {                  /* the crows here (the others fly off); one alone keeps far off */
        int want = i < m->crows;
        if (want && !fr_c[i].on) {
            fr_c[i].on = 1;
            fr_c[i].st = 1;
            fr_c[i].far = (int8_t)(m->crows == 1);
            fr_c[i].x = -30.0f;
            fr_c[i].y = 50.0f + 40.0f * sc_frand();
            fr_c[i].perch = (int8_t)(sc_rnd() % (uint32_t)fr.lay.np);
            if (fr_c[i].far) {                        /* (far off: it crosses behind the ruin, now and then) */
                fr_c[i].tx = 270.0f;
                fr_c[i].ty = fr_c[i].y + (sc_frand() - 0.5f) * 30.0f;
                fr_c[i].wait = 8.0f + 20.0f * sc_frand();
            } else {
                fr_c[i].tx = fr.lay.px[fr_c[i].perch];
                fr_c[i].ty = fr.lay.py[fr_c[i].perch];
                fr_c[i].wait = 20.0f + 30.0f * sc_frand();
            }
        } else if (want) {
            int far = m->crows == 1;
            if (far != fr_c[i].far) {                 /* (near now, or far: in from the side again) */
                fr_c[i].far = (int8_t)far;
                fr_c[i].on = 0;
                i--;
            }
        } else if (fr_c[i].on) {
            if (fr_c[i].x < -20.0f || fr_c[i].x > 250.0f) {
                fr_c[i].on = 0;
            } else {
                fr_c[i].st = 1;
                fr_c[i].tx = 300.0f;
                fr_c[i].ty = 30.0f + 40.0f * sc_frand();
            }
        }
    }
    fr_fx();
}

/* ------------------------------------------------------------ picture --- */
static void fr_palette(void)
{
#define FR_RGB(v) ((uint32_t)(v)[0] << 16 | (uint32_t)(v)[1] << 8 | (uint32_t)(v)[2])
    uint32_t sky0 = FR_RGB(fr.sky0), sky1 = FR_RGB(fr.sky1), gnd = FR_RGB(fr.gnd), wat = FR_RGB(fr.wat), grs = FR_RGB(fr.grs),
             twig = FR_RGB(fr.twig), trunk = FR_RGB(fr.trunk), fogc = FR_RGB(fr.fogc), shadow = 0x08090Cu,
             haze = r8_mix(sky1, fogc, 0.5f), cat[4];
    static const float HZ[4] = {0.74f, 0.52f, 0.3f, 0.08f};   /* how far into the haze each depth of trees stands */
    float l = fr.light, lm = fm_minf(1.0f, l);
    int i, b, c;
#undef FR_RGB
    for (i = 0; i < FR_NSKY; i++)
        r8_pal_set(FR_SKY + i, r8_mix(sky0, sky1, (float)i * (1.0f / (float)(FR_NSKY - 1))));
    for (b = 0; b < 4; b++) {
        uint32_t tk = r8_mix(r8_mix(shadow, trunk, 0.4f + 0.6f * lm), haze, HZ[b]), tw = r8_mix(r8_mix(shadow, twig, lm), haze, HZ[b]);
        r8_pal_set(FR_TRK + 2 * b, tk);
        r8_pal_set(FR_TRK + 2 * b + 1, tw);
        for (c = 0; c < 2; c++)                       /* the canopy: in leaf, or a thickening of twigs */
            r8_pal_set(FR_CAN + 2 * b + c, r8_mix(r8_mix(haze, tw, 0.3f), r8_mix(r8_mix(shadow, c ? 0x7A9A38u : 0x3E5A26u, lm), haze, HZ[b] * 0.8f), fr.canopy));
    }
    for (b = 0; b < 4; b++)
        for (i = 0; i < 4; i++)
            r8_pal_set(FR_GND + b * 4 + i, r8_mix(r8_mix(shadow, gnd, (0.45f + 0.55f * (float)i * (1.0f / 3.0f)) * lm), haze, 0.26f * (float)b));
    for (b = 0; b < 4; b++)
        for (i = 0; i < 2; i++)
            r8_pal_set(FR_WAT + b * 2 + i, r8_mix(r8_mix(r8_mix(shadow, wat, lm), sky1, 0.2f + 0.3f * (float)i), haze, 0.22f * (float)b));
    for (i = 0; i < 4; i++) {
        uint32_t g = r8_mix(shadow, grs, (0.4f + 0.2f * (float)i) * lm);
        r8_pal_set(FR_GRS + i, g);
        r8_pal_set(FR_GRS + 4 + i, r8_mix(g, haze, 0.42f));
    }
    {
        static const uint32_t WD[8] = {0x5A4A38u, 0x8A7A60u, 0x3A3022u, 0x9A8458u, 0x0A0A0Cu, 0x3C3C40u, 0x8C8880u, 0x5A4A38u};
        for (i = 0; i < 8; i++)
            r8_pal_set(FR_WOOD + i, r8_mix(r8_mix(shadow, WD[i], (i == 4 ? 1.0f : 0.75f) * lm), haze, 0.12f));
    }
    for (i = 0; i < 8; i++)
        r8_pal_set(FR_STONE + i, r8_mix(shadow, 0x8A8E90u, (0.1f + 0.9f * (float)i * (1.0f / 7.0f)) * lm));
    for (c = 0; c < 4; c++) {
        uint32_t lc = (uint32_t)fr.leaf[c][0] << 16 | (uint32_t)fr.leaf[c][1] << 8 | (uint32_t)fr.leaf[c][2];
        for (i = 0; i < 4; i++)
            r8_pal_set(FR_LEAF + c * 4 + i, r8_mix(shadow, lc, (0.45f + 0.2f * (float)i) * fm_minf(1.2f, l)));
    }
    cat[0] = sky1;
    cat[1] = r8_base[FR_TRK + 3];
    cat[2] = r8_base[FR_GND + 9];
    cat[3] = r8_base[FR_WAT + 2];
    for (c = 0; c < 4; c++)
        for (i = 0; i < 4; i++)
            r8_pal_set(FR_FOG + c * 4 + i, r8_mix(cat[c], fogc, 0.3f + 0.19f * (float)i));
    r8_pal_set(FR_CROW, 0x0A0A0Cu);
    r8_pal_set(FR_CROW + 1, r8_mix(0x0A0A0Cu, sky1, 0.28f));
    r8_pal_set(FR_CROW + 2, 0x3A3028u);
    r8_pal_set(FR_CROW + 3, 0x0A0A0Cu);
    {   /* the moon and the stars by night, the sun at dawn: in the sky's own colors when they are not up */
        static const uint32_t MOON[4] = {0xE8E4D0u, 0xC8C4B0u, 0xA8A490u, 0x404048u};
        for (i = 0; i < 4; i++) {
            r8_pal_set(FR_MOON + i, r8_mix(sky0, MOON[i], fr.moonamt));
            r8_pal_set(FR_HALO + i, r8_mix(sky0, 0xB0B8C8u, (0.08f + 0.1f * (float)i) * fr.moonamt));
            r8_pal_set(FR_STAR + i, r8_mix(sky0, 0xE0E4F0u, (0.3f + 0.23f * (float)i) * fr.moonamt));
        }
        r8_pal_set(FR_SUN, r8_mix(sky1, 0xFFF4D8u, 0.8f * fr.sunamt));
        for (i = 1; i < 4; i++)
            r8_pal_set(FR_SUN + i, r8_mix(sky1, 0xFFE8C0u, (0.4f - 0.11f * (float)i) * fr.sunamt));
    }
    r8_pal_set(FR_RAIN, r8_mix(sky1, 0xC8D0D8u, 0.45f));
    r8_pal_set(FR_RAIN + 1, r8_mix(r8_base[FR_GND + 6], 0xC8D0D8u, 0.4f));
    r8_pal_set(FR_BOLT, 0xF0F4FFu);
    r8_pal_set(FR_BAT, 0x030304u);
    r8_pal_set(FR_DUST, r8_mix(r8_base[FR_GND + 2], 0xC0B8A8u, 0.5f));
    r8_pal_set(FR_DUST + 1, r8_mix(r8_base[FR_GND + 2], 0xC0B8A8u, 0.25f));
    r8_pal_set(FR_RIP, r8_mix(r8_base[FR_WAT + 1], 0xE8EEF4u, 0.55f));
    r8_pal_set(FR_RIP + 1, r8_mix(r8_base[FR_WAT + 1], 0xE8EEF4u, 0.25f));
    r8_pal_commit(0xFFFFFFu, 0xE8ECFFu, (int)(fr.flash * 210.0f));
}

static void fr_ripple(float x, float y)
{
    int i, old = 0;
    for (i = 1; i < FR_NRIP; i++)
        if (fr_rp[i].t < fr_rp[old].t)
            old = i;
    fr_rp[old].x = x;
    fr_rp[old].y = y;
    fr_rp[old].t = 1.4f;
}

/* a crow: perched (its body, tail, head and beak, the head up as it calls; sized by how near its perch is), or
 * flying (its wings beating; a far one small, behind the trees) */
static void fr_draw_crow(surf_t *s, int i, float k)
{
    float x = fr_c[i].x, y = fr_c[i].y, d = (float)fr_c[i].dir, sz = fr_c[i].far ? 0.5f : 0.9f;
    int bh = fr_c[i].far ? FR_GND : 0;
    if (fr_c[i].st) {
        float fl = fm_sin_turns(fr_c[i].wing), wy = -5.0f * sz * fl, wx = 7.5f * sz, bx = x * k, by = y * k;
        fr_line(s, bx, by, (x - wx) * k, (y + wy - 1.0f) * k, FR_CROW, bh);
        fr_line(s, bx, by + k, (x - wx) * k, (y + wy) * k, FR_CROW, bh);
        fr_line(s, bx, by, (x + wx) * k, (y + wy - 1.0f) * k, FR_CROW, bh);
        fr_line(s, bx, by + k, (x + wx) * k, (y + wy) * k, FR_CROW, bh);
        fr_disc(s, bx, by + 0.5f * k, 1.6f * sz * k, FR_CROW, bh, 16);
        fr_px(s, (int32_t)((x + 2.6f * d * sz) * k), (int32_t)(y * k), FR_CROW, bh);
        fr_px(s, (int32_t)((x - 3.0f * d * sz) * k), (int32_t)((y + 1.5f) * k), FR_CROW, bh);
        return;
    }
    {
        float g = fr.lay.ps[fr_c[i].perch] * k, up = fr_c[i].call > 0.0f ? 1.6f : 0.0f, xk = x * k, yk = y * k;
        r8_tri(s, xk - 1.5f * d * g, yk - 3.2f * g, xk - 7.5f * d * g, yk - 1.4f * g, xk - 6.5f * d * g, yk - 4.6f * g, FR_CROW);
        r8_disc(s, xk, yk - 3.0f * g, 2.7f * g, FR_CROW);
        r8_disc(s, xk + 2.6f * d * g, yk - (6.2f + up) * g, 1.8f * g, FR_CROW);
        r8_limb(s, xk + 3.8f * d * g, yk - (6.3f + up) * g, 1.1f * g, xk + 6.6f * d * g, yk - (5.6f + 1.4f * up) * g, 0.4f * g, FR_CROW + 2);
        fr_px(s, (int32_t)(xk + 3.0f * d * g), (int32_t)(yk - (6.8f + up) * g), FR_CROW + 3, 0);
        fr_disc(s, xk - 1.0f * d * g, yk - 4.4f * g, 0.55f * g, FR_CROW + 1, 0, 16);   /* (the light on its back) */
        fr_px(s, (int32_t)(xk - 1.0f * g), (int32_t)(yk - 0.5f * g), FR_CROW + 2, 0);
        fr_px(s, (int32_t)(xk + 1.0f * g), (int32_t)(yk - 0.5f * g), FR_CROW + 2, 0);
    }
}

static void fr_draw(void)
{
    surf_t *s = &r8_fb;
    float k = fr_scale();
    int i, j;
    r8_copy(s, &r8_bg);
    if (fr.moonamt > 0.02f) {                         /* the night sky: the stars, the moon on its way over the trees */
        float mt, mx, my;
        fr_moon_xy(&mt, &mx, &my);
        for (i = 0; i < FR_NSTAR; i++) {
            int tw = (int)(fr.t * 2.0f + (float)i * 0.37f) & 3;
            fr_px(s, (int32_t)(fr_s[i].x * k), (int32_t)(fr_s[i].y * k), (uint8_t)(FR_STAR + (fr_s[i].br + (tw == 0)) % 4), FR_TRK);
        }
        if (mt > -0.05f && mt < 1.05f)
            sc_moon_draw(s, k, &fr.mn, mx * k, my * k, FR_TRK, FR_MOON, FR_HALO, 4);
    }
    if (fr.sunamt > 0.02f) {                          /* the sun at dawn, low in the haze behind the trees */
        float sx = 196.0f, sy = 66.0f - 34.0f * fr.dawn;
        fr_disc(s, sx * k, sy * k, 22.0f * k, FR_SUN + 3, FR_TRK, 7);
        fr_disc(s, sx * k, sy * k, 16.0f * k, FR_SUN + 2, FR_TRK, 9);
        fr_disc(s, sx * k, sy * k, 12.0f * k, FR_SUN + 1, FR_TRK, 12);
        fr_disc(s, sx * k, sy * k, 9.0f * k, FR_SUN, FR_TRK, 16);
    }
    for (i = 0; i < FR_NCROW; i++)                    /* the crows far off, among the trees */
        if (fr_c[i].on && fr_c[i].st && fr_c[i].far)
            fr_draw_crow(s, i, k);
    for (i = 0; i < FR_NFOG; i++) {                   /* the fog: banks lying among the trunks and over the water, each
                                                       * pixel a shade of what it covers by how thick the fog is there */
        float w = fr_f[i].w, hh = fr_f[i].hh, th = fr_f[i].th * fr.fogamt;
        int32_t x, y, x0 = (int32_t)((fr_f[i].x - w) * k), x1 = (int32_t)((fr_f[i].x + w) * k), y0 = (int32_t)((fr_f[i].y - hh) * k),
                y1 = (int32_t)((fr_f[i].y + hh) * k);
        if (th < 0.03f)
            continue;
        for (y = y0 < 0 ? 0 : y0; y <= y1 && y < s->h; y++) {
            float yw = (float)y / k, dy = (yw - fr_f[i].y) / hh, vy = 1.0f - dy * dy;
            uint8_t *row = s->px + y * s->w;
            if (vy <= 0.0f)
                continue;
            for (x = x0 < 0 ? 0 : x0; x <= x1 && x < s->w; x++) {
                float xw = (float)x / k, dx = (xw - fr_f[i].x) / w, vx = 1.0f - dx * dx, a, st;
                int lv, cat, old;
                uint8_t p = row[x];
                if (vx <= 0.0f)
                    continue;
                st = fm_sin_turns(xw * 0.017f + yw * 0.09f + fr.t * 0.012f + fr_f[i].ph) + 0.5f * fm_sin_turns(xw * 0.041f - fr.t * 0.02f);
                a = fm_sqrtf(vx * vy) * th * (1.0f + 0.4f * st) * 5.2f;
                lv = (int)a;
                lv += r8_dith(x, y, (int)((a - (float)lv) * 16.0f));
                if (lv <= 0)
                    continue;
                lv = lv > 4 ? 4 : lv;
                old = p >= FR_FOG && p < FR_FOG + 16 ? ((p - FR_FOG) & 3) + 1 : 0;
                if (old >= lv)
                    continue;
                if (old)
                    cat = (p - FR_FOG) >> 2;
                else if (p < FR_TRK)
                    cat = 0;
                else if (p < FR_GND)
                    cat = 1;
                else if (p < FR_WAT)
                    cat = 2;
                else if (p < FR_GRS)
                    cat = 3;
                else if (p < FR_LEAF)
                    cat = 2;
                else
                    continue;                         /* (what was drawn in front stays in front) */
                row[x] = (uint8_t)(FR_FOG + cat * 4 + lv - 1);
            }
        }
    }
    for (i = 0; i < FR_NRIP; i++)                     /* rings on the water */
        if (fr_rp[i].t > 0.0f) {
            float r = (1.4f - fr_rp[i].t) * 7.0f + 1.0f;
            r8_ring(s, fr_rp[i].x * k, fr_rp[i].y * k, r * k, r * 0.35f * k, (uint8_t)(FR_RIP + (fr_rp[i].t < 0.6f)));
        }
    for (i = 0; i < FR_NTUFT; i++) {                  /* the grass near us, leaning with the wind */
        float gx = fr.lay.tuft[i].x, gy = fr.lay.tuft[i].y, gh = fr.lay.tuft[i].h,
              bend = gh * (0.45f * fr.wind + 0.06f * fm_sin_turns(fr.t * 0.5f + gx * 0.01f));
        for (j = 0; j < fr.lay.tuft[i].n; j++) {
            float bx = gx + 1.5f * (float)(j - fr.lay.tuft[i].n / 2), h = gh * (0.7f + 0.3f * (float)((j * 7) % 3) * 0.5f);
            fr_line(s, bx * k, gy * k, (bx + bend + (float)(j % 3 - 1) * 2.5f) * k, (gy - h) * k, (uint8_t)(FR_GRS + j % 3), 0);
            fr_px(s, (int32_t)((bx + bend + (float)(j % 3 - 1) * 2.5f) * k), (int32_t)((gy - h) * k), FR_GRS + 3, 0);
        }
    }
    for (i = 0; i < FR_NCROW; i++)                    /* the crows on the posts and the roof, and in the air near */
        if (fr_c[i].on && (!fr_c[i].st || !fr_c[i].far))
            fr_draw_crow(s, i, k);
    if (fr.fall.on)
        r8_rect(s, (int32_t)((fr.fall.x - 1.5f) * k), (int32_t)(fr.fall.y * k), (int32_t)(3.0f * k + 0.5f), (int32_t)(2.0f * k + 0.5f), FR_STONE + 5);
    for (i = 0; i < FR_NDUST; i++)
        if (fr_d[i].life > 0.0f)
            fr_px(s, (int32_t)(fr_d[i].x * k), (int32_t)(fr_d[i].y * k), (uint8_t)(FR_DUST + (fr_d[i].life < 0.6f)), 0);
    for (i = 0; i < FR_NL; i++) {                     /* the leaves: tumbling as they fall, flat where they lie */
        float hh;
        if (!fr_l[i].st)
            continue;
        hh = fr_l[i].st == 1 ? fm_fabsf(fm_sin_turns(fr_l[i].rot + 0.25f)) * 0.6f + 0.25f : 0.3f;
        r8_quad(s, fr_l[i].x * k, fr_l[i].y * k, fr_l[i].size * k, hh * fr_l[i].size * k, fr_l[i].st == 1 ? fr_l[i].rot : 0.02f,
                (uint8_t)(FR_LEAF + fr_l[i].col * 4 + (fr_l[i].st == 1 ? 1 + (int)(2.9f * hh) : 1)));
    }
    if (fr.rain > 0.02f)                              /* the rain: streaks, slanted by the wind */
        for (i = 0; i < FR_NRAIN; i++)
            if (fr_r[i].on) {
                float sl = (0.15f + 0.6f * fr.wind) * 3.0f;
                fr_px(s, (int32_t)(fr_r[i].x * k), (int32_t)(fr_r[i].y * k), FR_RAIN, 0);
                fr_px(s, (int32_t)((fr_r[i].x - sl) * k), (int32_t)((fr_r[i].y - 3.0f) * k), FR_RAIN + 1, 0);
            }
    if (fr.bolt_t > 0.0f)                             /* the bolt, a moment */
        for (i = 0; i < 7; i++)
            r8_limb(s, fr.bolt[i][0] * k, fr.bolt[i][1] * k, 1.5f * k, fr.bolt[i + 1][0] * k, fr.bolt[i + 1][1] * k, 0.9f * k, FR_BOLT);
    if (fr.ch && fr.ch->bats)                         /* the bats: a flicker of wings over the trees */
        for (i = 0; i < fr.ch->bats && i < FR_NBAT; i++) {
            float x = fr_b[i].x * k, y = fr_b[i].y * k, w = ((int)(fr_b[i].t * 14.0f) & 1) ? 1.0f : -0.5f;
            fr_px(s, (int32_t)x, (int32_t)y, FR_BAT, 0);
            fr_px(s, (int32_t)(x - 1.5f * k), (int32_t)(y - w * k), FR_BAT, 0);
            fr_px(s, (int32_t)(x + 1.5f * k), (int32_t)(y - w * k), FR_BAT, 0);
            fr_px(s, (int32_t)(x - 2.5f * k), (int32_t)(y - 0.3f * w * k), FR_BAT, 0);
            fr_px(s, (int32_t)(x + 2.5f * k), (int32_t)(y - 0.3f * w * k), FR_BAT, 0);
        }
}

/* -------------------------------------------------------------- world --- */
/* the clearing's own shape: where the trees stand and how tall, the pools, where the hamlet's leavings lie; the
 * perches; the stars; the fog */
SC_SMALL static void fr_layout(void)
{
    static const int NB[4] = {9, 6, 4, 3};
    static const float Y0[4] = {76.0f, 90.0f, 118.0f, 160.0f}, Y1[4] = {84.0f, 110.0f, 150.0f, 205.0f},
                       H0[4] = {30.0f, 58.0f, 98.0f, 136.0f}, H1[4] = {44.0f, 78.0f, 126.0f, 180.0f};
    int i, b, n = 0;
    fr.tr = fr.seed * 747796405u + 2891336453u;
    for (b = 0; b < 4; b++)
        for (i = 0; i < NB[b] && n < FR_NTREE; i++, n++) {
            float u = ((float)i + 0.5f) / (float)NB[b];
            fr.lay.tree[n].x = u * 240.0f + ((float)fr_h(n, 1) * (1.0f / 255.0f) - 0.5f) * 240.0f / (float)NB[b] * 0.9f;
            fr.lay.tree[n].y = Y0[b] + (Y1[b] - Y0[b]) * (float)fr_h(n, 2) * (1.0f / 255.0f);
            fr.lay.tree[n].h = H0[b] + (H1[b] - H0[b]) * (float)fr_h(n, 3) * (1.0f / 255.0f);
            fr.lay.tree[n].band = (uint8_t)b;
        }
    fr.lay.ntree = n;
    for (i = 0; i < FR_NWAT; i++) {                   /* the pools: wide and flat, nearer ones larger */
        float v = 0.15f + 0.8f * (float)i / (float)(FR_NWAT - 1);
        fr.lay.wat[i].x = 30.0f + 180.0f * (float)fr_h(i, 11) * (1.0f / 255.0f);
        fr.lay.wat[i].y = FR_HORIZON + 10.0f + (240.0f - FR_HORIZON - 20.0f) * v + ((float)fr_h(i, 12) * (1.0f / 255.0f) - 0.5f) * 20.0f;
        fr.lay.wat[i].rx = 18.0f + 40.0f * v + (float)(fr_h(i, 13) % 20u);
        fr.lay.wat[i].ry = fr.lay.wat[i].rx * (0.18f + 0.12f * v);
    }
    fr.lay.hx = 150.0f + (float)(fr_h(10, 21) % 50u);   /* the hut, mid-distance right; the fence far left to the
                                                       * middle; the well mid right; the cart wheel in a pool, left;
                                                       * the log in the near left corner */
    fr.lay.hy = 118.0f + (float)(fr_h(10, 22) % 12u);
    fr.lay.fx0 = 20.0f + (float)(fr_h(10, 23) % 30u);
    fr.lay.fy = 100.0f + (float)(fr_h(10, 24) % 8u);
    fr.lay.wx = 130.0f + (float)(fr_h(10, 25) % 40u);
    fr.lay.wy = 158.0f + (float)(fr_h(10, 26) % 16u);
    fr.lay.cx0 = 40.0f + (float)(fr_h(10, 27) % 40u);
    fr.lay.cy0 = 150.0f + (float)(fr_h(10, 28) % 20u);
    fr.lay.wat[1].x = fr.lay.cx0 + 6.0f;              /* (one pool round the wheel) */
    fr.lay.wat[1].y = fr.lay.cy0 + 6.0f;
    fr.lay.wat[1].rx = 34.0f;
    fr.lay.wat[1].ry = 8.0f;
    fr.lay.lx0 = 14.0f + (float)(fr_h(10, 29) % 20u);
    fr.lay.ly0 = 208.0f + (float)(fr_h(10, 30) % 16u);
    fr.lay.lx1 = fr.lay.lx0 + 56.0f + (float)(fr_h(10, 31) % 20u);
    fr.lay.ly1 = fr.lay.ly0 - 10.0f - (float)(fr_h(10, 32) % 8u);
    for (i = 0; i < FR_NTUFT; i++) {
        fr.lay.tuft[i].x = 8.0f + 224.0f * (float)fr_h(i, 33) * (1.0f / 255.0f);
        fr.lay.tuft[i].y = 168.0f + 66.0f * (float)fr_h(i, 34) * (1.0f / 255.0f);
        fr.lay.tuft[i].h = 10.0f + 10.0f * (fr.lay.tuft[i].y - 168.0f) * (1.0f / 66.0f) + (float)(fr_h(i, 35) % 5u);
        fr.lay.tuft[i].n = 5 + (int)(fr_h(i, 36) % 4u);
        if (fr_water(fr.lay.tuft[i].x, fr.lay.tuft[i].y) >= 0)
            fr.lay.tuft[i].n = 0;
    }
    fr.lay.na = FR_NA;                                /* what sings: the well (the root), the doorway, the log */
    fr.lay.cx[0] = fr.lay.wx;
    fr.lay.cy[0] = fr.lay.wy;
    fr.lay.hw[0] = 13.0f;
    fr.lay.cx[1] = fr.lay.hx - 4.0f;
    fr.lay.cy[1] = fr.lay.hy - 7.0f;
    fr.lay.hw[1] = 5.0f;
    fr.lay.cx[2] = fr.lay.lx0;
    fr.lay.cy[2] = fr.lay.ly0;
    fr.lay.hw[2] = 3.0f;
    n = 0;                                            /* the perches: the roof's ridge, a fence post, the well's rim,
                                                       * the log (near: large), the wheel */
    fr.lay.px[n] = fr.lay.hx + 8.0f;
    fr.lay.py[n] = fr.lay.hy - 21.0f;
    fr.lay.ps[n++] = 1.0f;
    fr.lay.px[n] = fr.lay.fx0 + 34.0f;
    fr.lay.py[n] = fr.lay.fy - 7.0f;
    fr.lay.ps[n++] = 0.8f;
    fr.lay.px[n] = fr.lay.wx - 7.0f;
    fr.lay.py[n] = fr.lay.wy - 3.0f;
    fr.lay.ps[n++] = 1.25f;
    fr.lay.px[n] = fr.lay.lx0 + 24.0f;
    fr.lay.py[n] = fr.lay.ly0 - 7.0f;
    fr.lay.ps[n++] = 1.6f;
    fr.lay.px[n] = fr.lay.cx0;
    fr.lay.py[n] = fr.lay.cy0 - 10.0f;
    fr.lay.ps[n++] = 1.3f;
    fr.lay.np = n;
    for (i = 0; i < FR_NSTAR; i++) {
        fr_s[i].x = 2.0f + 236.0f * (float)fr_h(i, 41) * (1.0f / 255.0f);
        fr_s[i].y = 2.0f + 60.0f * (float)fr_h(i, 42) * (1.0f / 255.0f);
        fr_s[i].br = (uint8_t)(fr_h(i, 43) % 3u);
    }
    sc_moon_repick(&fr.mn, 1.0f, fr.seed * 7u + 3u);
    fr.moon_top = 46.0f + (float)(fr_h(10, 44) % 16u);
    sc_mst_k = -1.0f;
    for (i = 0; i < FR_NFOG; i++) {
        fr_f[i].x = 40.0f + 80.0f * (float)i;
        fr_f[i].y = 96.0f + 18.0f * (float)i + (float)(fr_h(i, 45) % 10u);
        fr_f[i].w = 60.0f + (float)(fr_h(i, 46) % 50u);
        fr_f[i].hh = 12.0f + (float)(fr_h(i, 47) % 10u);
        fr_f[i].ph = (float)fr_h(i, 48) * (1.0f / 255.0f);
        fr_f[i].lastx = fr_f[i].x;
    }
}

static void fr_leaf_new(int i, int from_left)
{
    float u = sc_frand();
    fr_l[i].st = 1;
    fr_l[i].x = from_left ? -8.0f : 10.0f + 220.0f * sc_frand();
    fr_l[i].y = from_left ? 20.0f + 120.0f * sc_frand() : -4.0f - 6.0f * sc_frand();
    fr_l[i].vx = 0.0f;
    fr_l[i].vy = 9.0f + 8.0f * sc_frand();
    fr_l[i].rot = sc_frand();
    fr_l[i].spin = (0.3f + 0.5f * sc_frand()) * (sc_rnd() & 1u ? 1.0f : -1.0f);
    fr_l[i].ph = sc_frand();
    fr_l[i].size = 1.5f + 1.6f * u;                   /* (the nearer it lands, the larger) */
    fr_l[i].land_y = fr.wind > 0.5f ? 300.0f : FR_HORIZON + 8.0f + 150.0f * u;
    fr_l[i].col = (int8_t)(sc_rnd() & 3u);
    fr_l[i].silent = 0;
}

/* the look the mood asks for now (Long Night: between two of its day's), and what holds now (fr_char) */
static void fr_look(float dt, int at_once)
{
    const fr_mood_t *a = &FR_MOOD[fr_mood_now()], *b = a;
    float u = 0.0f, e = at_once ? 1.0f : fm_minf(1.0f, dt * 0.3f), v;
    int c;
    if (a->scale < 0) {
        float f = fr.phase * 4.0f;
        int si = (int)f;
        si = si > 3 ? 3 : si;
        u = fm_clampf((f - (float)si - 0.7f) * (1.0f / 0.3f), 0.0f, 1.0f);
        u = u * u * (3.0f - 2.0f * u);
        a = &FR_MOOD[FR_LN[si]];
        b = &FR_MOOD[FR_LN[(si + 1) & 3]];
        fr.dawn = si == 0 ? fm_minf(1.0f, (f - (float)si) * 1.6f) : 1.0f;
        fr.night = si == 3 ? f - 3.0f : 0.0f;
        cd_light(a->scale);                           /* (taken when the notes rest, within a minute) */
    } else {
        fr.dawn = fm_minf(1.0f, fr.dawn + dt * (1.0f / 300.0f));
        fr.night += dt * (1.0f / 1800.0f);
        fr.night -= fm_floorf(fr.night);
    }
    fr_char(u < 0.5f ? a : b);
#define FR_EASE(dst, ca, cb) do { \
        uint32_t m_ = r8_mix(ca, cb, u); \
        (dst)[0] += ((float)(m_ >> 16 & 255u) - (dst)[0]) * e; \
        (dst)[1] += ((float)(m_ >> 8 & 255u) - (dst)[1]) * e; \
        (dst)[2] += ((float)(m_ & 255u) - (dst)[2]) * e; \
    } while (0)
    FR_EASE(fr.sky0, a->sky0, b->sky0);
    FR_EASE(fr.sky1, a->sky1, b->sky1);
    FR_EASE(fr.gnd, a->gnd, b->gnd);
    FR_EASE(fr.wat, a->wat, b->wat);
    FR_EASE(fr.grs, a->grs, b->grs);
    FR_EASE(fr.twig, a->twig, b->twig);
    FR_EASE(fr.trunk, a->trunk, b->trunk);
    FR_EASE(fr.fogc, a->fogc, b->fogc);
    for (c = 0; c < 4; c++)
        FR_EASE(fr.leaf[c], FR_LEAFPAL[a->leafpal][c], FR_LEAFPAL[b->leafpal][c]);
#undef FR_EASE
#define FR_EASE1(dst, fa, fb, sc) do { v = ((float)(fa) + ((float)(fb) - (float)(fa)) * u) * (sc); (dst) += (v - (dst)) * e; } while (0)
    FR_EASE1(fr.light, a->light, b->light, 0.01f);
    FR_EASE1(fr.ivy, a->ivy, b->ivy, 0.01f);
    FR_EASE1(fr.canopy, a->canopy, b->canopy, 0.01f);
    FR_EASE1(fr.moonamt, a->moon, b->moon, 1.0f);
    FR_EASE1(fr.sunamt, a->sun, b->sun, 1.0f);
    FR_EASE1(fr.rain, a->rain, b->rain, 0.01f);
    {   /* the fog's thickness and the wind's base: the knobs (a mood sets them); in Long Night its day's, scaled by
         * the knobs from their defaults */
        float fk = fr_long() ? (float)fr.kv[FK_FOG] * (1.0f / 30.0f) : 1.0f, wk = fr_long() ? (float)fr.kv[FK_WIND] * (1.0f / 30.0f) : 1.0f;
        float ft = fr_long() ? ((float)a->fog + ((float)b->fog - (float)a->fog) * u) * 0.01f * fk : (float)fr.kv[FK_FOG] * 0.01f,
              wt = fr_long() ? ((float)a->wind + ((float)b->wind - (float)a->wind) * u) * 0.01f * wk : (float)fr.kv[FK_WIND] * 0.01f;
        ft = fr.fog_key > 0.0f ? 1.0f : fm_minf(1.0f, ft);
        fr.fogamt += (ft - fr.fogamt) * e;
        fr.wbase += (fm_minf(1.0f, wt) - fr.wbase) * e;
    }
#undef FR_EASE1
    {   /* the voice's vowel: the mood's, opened by the wind (the arches' hollow); its tone and breath */
        int vw = (int)((float)a->vowel - 34.0f * fr.wind + 6.0f * fm_sin_turns(fr.t * (1.0f / 41.0f)));
        vw = vw < 0 ? 0 : vw > 127 ? 127 : vw;
        if (fr.kv[FK_VOICES] > 0 && (vw - fr.vowel_sent >= 4 || fr.vowel_sent - vw >= 4)) {
            fm_part_knobs(2, vw, 12, a->buzz, a->breath);
            fr.vowel_sent = vw;
        }
    }
}

static void fr_mood_tune(void)
{
    const fr_mood_t *m = &FR_MOOD[fr_mood_now()];
    cd_tune(m->scale, (float)m->key);
}

SC_SMALL static void fr_defaults(int16_t *kv)
{
    int i;
    for (i = 0; i < SC_NKNOB; i++)
        kv[i] = FR_KNOB[i][3];
}

SC_SMALL static void fr_enter(uint32_t seed, int16_t *kv)
{
    int i;
    memset(&fr, 0, sizeof fr);
    memset(fr_l, 0, sizeof fr_l);
    memset(fr_c, 0, sizeof fr_c);
    memset(fr_q, 0, sizeof fr_q);
    memset(fr_r, 0, sizeof fr_r);
    memset(fr_d, 0, sizeof fr_d);
    memset(fr_rp, 0, sizeof fr_rp);
    fr.kv = kv;
    for (i = 0; i < SC_NKNOB; i++)
        if (FR_KNOB[i][1] > FR_KNOB[i][0] && (kv[i] < FR_KNOB[i][0] || kv[i] > FR_KNOB[i][1]))
            kv[i] = FR_KNOB[i][3];
    fr.seed = seed | 1u;
    sc_seed(fr.seed * 2654435761u + 11u);
    fr_layout();
    fr.phase = (float)(seed % 1000u) * 0.001f;        /* (Long Night: wherever the day is) */
    fr.night = 0.2f;
    fr.pal_lv = fr.air_sent = fr.rain_sent = fr.stream_sent = -1;
    fr.air_style = fr.air_want = -1;
    fr.vowel_sent = -100;
    fr.gnext = 6.0f + 10.0f * sc_frand();
    fr.crow_next = 8.0f + 10.0f * sc_frand();
    fr.owl_next = 30.0f + 30.0f * sc_frand();
    fr.stone_next = 40.0f + 50.0f * sc_frand();
    fr.bolt_next = 18.0f + 30.0f * sc_frand();
    fr.dawn = 0.0f;
    fr.thunder_at = 0;
    for (i = 0; i < FR_NBAT; i++) {
        fr_b[i].x = 20.0f + 200.0f * sc_frand();
        fr_b[i].y = 20.0f + 60.0f * sc_frand();
        fr_b[i].vx = 30.0f;
        fr_b[i].vy = 0.0f;
        fr_b[i].t = sc_frand() * 10.0f;
    }
    for (i = 0; i < FR_NCROW; i++)
        fr_c[i].dir = 1;
    r8_dark(FR_DARK);
    cd_init(seed);
    cd_activity((float)kv[FK_ACT] * 0.01f);
    cd_echo_amount(0.0f);                             /* (the echo is the tape echo: fr_fx) */
    cd_change_mode(1);
    fr_mood_tune();
    fm_part_patch(1, FR_TONE_SLOT[kv[FK_TONE] % FR_NTONE]);
    fm_part_release(1, 2400);
    fr_drone();
    fm_part_patch(3, ZN_RIN);                         /* touches, the bats' ticks */
    fm_part_release(3, 2200);
    fm_part_patch(4, ZN_SOFT_SWELL);                  /* the fog's swells, the thunder's depth */
    fm_part_release(4, 3500);
    fm_part_patch(5, FM_ENG_PHYS + 1);                /* the arches: STRING, bowed, a little stiff, long */
    fm_part_knobs(5, 44, 92, 70, 46);
    fm_part_attack(5, 900);
    fm_part_release(5, 2600);
    fm_part_patch(6, FM_ENG_PHYS + 0);                /* a stone: MODAL, inharmonic, short, dull */
    fm_part_knobs(6, 98, 28, 0, 26);
    fm_part_release(6, 500);
    fm_part_patch(7, FM_ENG_PHYS + 2);                /* thunder: MEMBRANE, loaded, ringing on, its pitch falling, dark */
    fm_part_knobs(7, 60, 118, 72, 8);
    fm_part_release(7, 2600);
    fm_part_level(6, 60);
    fm_part_level(7, 40);
    fr_look(0.0f, 1);
    fr_mix_levels();
    cd_drone(1);
    fm_bed_style(FM_BED_STREAM, FM_WATER_DEEP);       /* (the storm's roar) */
    fm_bed(FM_BED_STREAM, 0);
    fm_bed(FM_BED_RAIN, 0);
    fm_bed_style(FM_BED_WIND, fr.air_want >= 0 ? fr.air_want : FM_AIR_BREEZE);
    fr.air_style = fr.air_want >= 0 ? fr.air_want : FM_AIR_BREEZE;
    fr.air_switch = 0.0f;
    fm_bed(FM_BED_WIND, kv[FK_AIR]);
    cd_melody(0);
    cd_arc_amount((float)kv[FK_ARC] * 0.01f);
    cd_depth_send(kv[FK_DEPTH]);
}

SC_SMALL static void fr_leave(void)
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

static void fr_step(uint32_t dt_ms)
{
    float dt = (float)dt_ms * 0.001f, a = (float)fr.kv[FK_ACT] * 0.01f;
    int i, j;
    fr.t += dt;
    fr.ms += dt_ms;
    if (fr_long()) {                                  /* Long Night: the day goes round */
        fr.phase += dt * (1.0f / 1800.0f);
        if (fr.phase >= 1.0f) {
            fr.phase -= 1.0f;
            fr.nights++;
            cd_dawn();
        }
    }
    fr_look(dt, 0);
    /* the wind: its base and the gusts, each rising over 1.5..3 s, blowing 1..3 s, dying over 5..9 s; a gust's
     * peak sings an arch (two in a mood without thunder: PHYS's room) */
    if (fr.gph == 0) {
        float gap = fr.ch->gust ? (float)fr.ch->gust : 0.0f;
        if (gap > 0.0f && (fr.gnext -= dt) <= 0.0f) {
            fr.gph = 1;
            fr.gt = 0.0f;
            fr.grise = 1.5f + 1.5f * sc_frand();
            fr.ghold = 1.0f + 2.0f * sc_frand();
            fr.gfall = 5.0f + 4.0f * sc_frand();
            fr.gpeak = (0.35f + 0.65f * sc_frand()) * (0.5f + 0.5f * fr.wbase + 0.3f);
            fr.gpeak = fm_minf(1.0f, fr.gpeak);
            fr.gnext = gap * (0.6f + 0.8f * sc_frand()) * (fr_long() ? 1.0f : 30.0f / (float)(fr.kv[FK_WIND] + 5));
        }
    } else {
        fr.gt += dt;
        if (fr.gph == 1) {
            fr.gust = fr.gpeak * (0.5f - 0.5f * fm_sin_turns(0.25f + 0.5f * fr.gt / fr.grise));
            if (fr.gt >= fr.grise) {
                int na = fr.ch->thunder ? 1 : 2, a0 = (int)(sc_rnd() % (uint32_t)fr.lay.na);
                fr.gph = 2;
                fr.gt = 0.0f;
                fr.gusts++;
                if (fr.gpeak >= 0.4f) {
                    fr_harp(a0, fr.gpeak);
                    if (na > 1 && fr.lay.na > 1 && fr.gpeak > 0.75f)
                        fr_harp((a0 + 1) % fr.lay.na, fr.gpeak * 0.8f);
                }
            }
        } else if (fr.gph == 2) {
            fr.gust = fr.gpeak;
            if (fr.gt >= fr.ghold) {
                fr.gph = 3;
                fr.gt = 0.0f;
            }
        } else {
            fr.gust = fr.gpeak * (0.5f + 0.5f * fm_sin_turns(0.25f + 0.5f * fr.gt / fr.gfall));
            if (fr.gt >= fr.gfall) {
                fr.gph = 0;
                fr.gust = 0.0f;
            }
        }
    }
    fr.wind = fm_minf(1.0f, fr.wbase * 0.6f + fr.gust * (0.5f + 0.5f * fr.wbase) + 0.04f * fm_sin_turns(fr.t * 0.07f));
    {
        int m = (int)(8.0f + 80.0f * fr.wind);
        if (m / 4 != fr.air_sent / 4) {
            fm_bed_mod(FM_BED_WIND, m);
            fr.air_sent = m;
        }
    }
    if (fr.air_switch > 0.0f && (fr.air_switch -= dt) <= 0.0f) {   /* (the air's recording changes, down) */
        fm_bed_style(FM_BED_WIND, fr.air_want);
        fr.air_style = fr.air_want;
        fm_bed(FM_BED_WIND, fr.kv[FK_AIR]);
    }
    for (j = 0; j < FR_NA; j++)                       /* the arches singing: each rises a little with the wind */
        if (fr.harp_left[j] > 0.0f) {
            fr.harp_left[j] -= dt;
            if (fr.harp_left[j] <= 0.0f)
                fr.harp_left[j] = 0.0f;
        }
    {
        int hs = (int)(fr.wind * 12.0f);
        if (hs != fr.harp_sent) {
            fr.harp_sent = hs;
            for (j = 0; j < FR_NA; j++)
                if (fr.harp_left[j] > 0.0f)
                    fm_voice_glide(FR_ID_HARP + j, fr.harp_c[j] + 55.0f * fr.wind - 10.0f, 600);
        }
    }
    /* the storm's beds: the rain, the roar (sent as they change) */
    {
        int r = (int)(fr.rain * 14.0f), st = fr.ch->thunder ? 14 : 0;
        if (r / 3 != fr.rain_sent / 3) {
            fr.rain_sent = r;
            fm_bed(FM_BED_RAIN, r);
            fm_bed_mod(FM_BED_RAIN, (int)(fr.rain * 40.0f));
        }
        if (st != fr.stream_sent) {
            fr.stream_sent = st;
            fm_bed(FM_BED_STREAM, st);
        }
        sc_rain_step(fr_r, FR_NRAIN, &fr.rain_amt, fr.rain, 40.0f + 180.0f * fr.wind, 270.0f, dt);
    }
    /* the leaves: let go above (in a storm, in from the left too), tumbling down and across with the wind; one that
     * lands on the floor is a note where it lands, on the wall a soft touch; where it lies it stays a while */
    fr.leaf_acc += dt * (0.1f + 1.4f * (float)fr.ch->leaves * 0.01f * (0.4f + 1.2f * a)) * (1.0f + 1.5f * fr.gust);
    while (fr.leaf_acc >= 1.0f) {
        fr.leaf_acc -= 1.0f;
        for (i = 0; i < FR_NL; i++)
            if (!fr_l[i].st) {
                fr_leaf_new(i, fr.wind > 0.5f && sc_frand() < 0.6f);
                break;
            }
    }
    for (i = 0; i < FR_NL; i++) {
        if (fr_l[i].st == 2) {
            if ((fr_l[i].life -= dt) <= 0.0f)
                fr_l[i].st = 0;
            continue;
        }
        if (!fr_l[i].st)
            continue;
        fr_l[i].vx += ((fr.wind * 75.0f + 10.0f * fm_sin_turns(fr.t * 0.6f + fr_l[i].ph)) - fr_l[i].vx) * fm_minf(1.0f, dt * 1.5f);
        fr_l[i].vy += (fr_l[i].size * 4.0f + 6.0f + 6.0f * fm_fabsf(fm_sin_turns(fr.t * 0.8f + fr_l[i].ph * 2.0f)) - 9.0f * fr.wind - fr_l[i].vy) * fm_minf(1.0f, dt * 2.0f);
        fr_l[i].x += fr_l[i].vx * dt;
        fr_l[i].y += fr_l[i].vy * dt;
        fr_l[i].rot += fr_l[i].spin * dt * (1.0f + 2.0f * fr.wind);
        fr_l[i].rot -= fm_floorf(fr_l[i].rot);
        if (fr_l[i].x > 252.0f || fr_l[i].x < -14.0f || fr_l[i].y > 244.0f) {
            fr_l[i].st = 0;
            continue;
        }
        if (fr_l[i].y >= fr_l[i].land_y) {             /* on the ground: a note; on the water: a ring, a plip, a touch */
            int w = fr_water(fr_l[i].x, fr_l[i].y);
            fr_l[i].st = 2;
            fr_l[i].life = 6.0f + 6.0f * sc_frand();
            fr.leaves++;
            if (w >= 0) {
                fr_ripple(fr_l[i].x, fr_l[i].y);
                if (!fr_l[i].silent) {
                    fm_drop(26 + (int)(sc_rnd() % 20u), cd_pan(fr_l[i].x * (1.0f / 240.0f)), 0);
                    cd_touch(fr_l[i].x * (1.0f / 240.0f), 0.6f);
                }
            } else if (!fr_l[i].silent) {
                cd_pluck(fr_l[i].x * (1.0f / 240.0f), 0.3f + 0.12f * fr_l[i].size, 1400);
            }
        }
    }
    /* the crows: on the wall they wait, hop to another perch, call; a far one crosses behind the ruin; in a storm
     * they wheel over the arches and cry */
    fr.crow_next -= dt;
    for (i = 0; i < FR_NCROW; i++) {
        if (!fr_c[i].on)
            continue;
        if (fr_c[i].call > 0.0f)
            fr_c[i].call -= dt;
        if (fr_c[i].st) {                             /* flying: toward where it heads, wings beating */
            float dx = fr_c[i].tx - fr_c[i].x, dy = fr_c[i].ty - fr_c[i].y, d = fm_sqrtf(dx * dx + dy * dy),
                  sp = fr_c[i].far ? 22.0f : 48.0f + 30.0f * fr.wind;
            fr_c[i].wing += dt * (fr_c[i].far ? 2.2f : 3.2f);
            fr_c[i].wing -= fm_floorf(fr_c[i].wing);
            if (dx != 0.0f)
                fr_c[i].dir = (int8_t)(dx > 0.0f ? 1 : -1);
            if (d < sp * dt + 0.5f) {
                if (fr_c[i].st == 2) {                /* wheeling: another turn, or down to the wall when it is over */
                    if ((fr_c[i].wait -= 2.0f) <= 0.0f) {
                        fr_c[i].st = 1;
                        fr_c[i].perch = (int8_t)(sc_rnd() % (uint32_t)fr.lay.np);
                        fr_c[i].tx = fr.lay.px[fr_c[i].perch];
                        fr_c[i].ty = fr.lay.py[fr_c[i].perch];
                    } else {
                        fr_c[i].tx = 20.0f + 200.0f * sc_frand();
                        fr_c[i].ty = 10.0f + 60.0f * sc_frand();
                        if (fr.kv[FK_CROWS] > 0 && sc_frand() < 0.3f) {
                            fr_caw(i, 46 + (int)(sc_rnd() % 16u), fr_c[i].x);
                            fr_c[i].call = 0.4f;
                        }
                    }
                    continue;
                }
                if (fr_c[i].tx >= 250.0f || fr_c[i].tx <= -20.0f) {   /* off: gone (a far one comes again) */
                    fr_c[i].on = (int8_t)(i < fr.ch->crows);
                    if (fr_c[i].on && fr_c[i].far) {
                        fr_c[i].x = -30.0f;
                        fr_c[i].y = 30.0f + 60.0f * sc_frand();
                        fr_c[i].tx = 270.0f;
                        fr_c[i].ty = fr_c[i].y + (sc_frand() - 0.5f) * 30.0f;
                        fr_c[i].wait = 40.0f + 50.0f * sc_frand();
                    } else if (fr_c[i].on) {
                        fr_c[i].x = -30.0f;
                        fr_c[i].y = 50.0f + 30.0f * sc_frand();
                        fr_c[i].perch = (int8_t)(sc_rnd() % (uint32_t)fr.lay.np);
                        fr_c[i].tx = fr.lay.px[fr_c[i].perch];
                        fr_c[i].ty = fr.lay.py[fr_c[i].perch];
                    }
                    continue;
                }
                fr_c[i].st = 0;
                fr_c[i].x = fr_c[i].tx;
                fr_c[i].y = fr_c[i].ty;
                fr_c[i].wait = 12.0f + 30.0f * sc_frand();
                fr_c[i].dir = (int8_t)(sc_rnd() & 1u ? 1 : -1);
                continue;
            }
            if (fr_c[i].far && fr_c[i].wait > 0.0f) {  /* (a far one waits off-screen for its next crossing) */
                if (fr_c[i].x < -20.0f) {
                    fr_c[i].wait -= dt;
                    if (fr_c[i].wait > 0.0f)
                        continue;
                }
            }
            fr_c[i].x += dx / d * sp * dt;
            fr_c[i].y += dy / d * sp * dt + 1.5f * fm_sin_turns(fr_c[i].wing) * dt * 10.0f;
            if (fr_c[i].far && fr.kv[FK_CROWS] > 0 && fr_c[i].x > 100.0f && fr_c[i].x < 100.0f + sp * dt + 0.1f) {
                fr_caw(i, 26 + (int)(sc_rnd() % 8u), fr_c[i].x);   /* (far off: once, as it passes) */
                fr_c[i].call = 0.5f;
            }
            continue;
        }
        if (fr.ch->thunder && fr.gust > 0.7f && sc_frand() < dt * 0.3f) {   /* a strong gust: up and wheeling */
            fr_c[i].st = 2;
            fr_c[i].tx = 20.0f + 200.0f * sc_frand();
            fr_c[i].ty = 10.0f + 60.0f * sc_frand();
            fr_c[i].wait = 8.0f + 8.0f * sc_frand();
            continue;
        }
        fr_c[i].wait -= dt;
        if (fr_c[i].wait <= 0.0f) {                   /* another perch, or a turn in the air and back */
            float r = sc_frand();
            if (r < 0.5f && fr.lay.np > 1) {
                int p = (fr_c[i].perch + 1 + (int)(sc_rnd() % (uint32_t)(fr.lay.np - 1))) % fr.lay.np;
                fr_c[i].perch = (int8_t)p;
                fr_c[i].st = 1;
                fr_c[i].tx = fr.lay.px[p];
                fr_c[i].ty = fr.lay.py[p];
            } else if (r < 0.65f) {
                fr_c[i].st = 1;
                fr_c[i].tx = fr_c[i].x + (sc_frand() - 0.5f) * 60.0f;
                fr_c[i].ty = fr_c[i].y - 15.0f - 30.0f * sc_frand();
                fr_c[i].wait = 0.0f;
            } else {
                fr_c[i].dir = (int8_t)-fr_c[i].dir;
                fr_c[i].wait = 6.0f + 20.0f * sc_frand();
            }
        }
        if (fr.crow_next <= 0.0f && fr.kv[FK_CROWS] > 0 && !fr_c[i].far) {   /* its turn to call */
            fr_caw(i, 44 + (int)(sc_rnd() % 18u), fr_c[i].x);
            fr_c[i].call = 0.45f;
            fr.crow_next = (14.0f + 1.5f * (float)(100 - fr.kv[FK_CROWS])) * (0.5f + sc_frand()) / (float)(fr.ch->crows > 0 ? fr.ch->crows : 1);
            if (sc_frand() < 0.4f)                    /* (another answers) */
                for (j = 0; j < FR_NCROW; j++)
                    if (j != i && fr_c[j].on && !fr_c[j].st && !fr_c[j].far) {
                        fr.crow_next = fm_minf(fr.crow_next, 1.0f + 0.8f * sc_frand());   /* (soon: the other's turn) */
                        break;
                    }
        }
    }
    /* the fog: banks drifting with the wind, round again from the left; one through an arch is a swell */
    if (fr.fog_key > 0.0f)
        fr.fog_key -= dt;
    for (i = 0; i < FR_NFOG; i++) {
        fr_f[i].lastx = fr_f[i].x;
        fr_f[i].x += (2.5f + 16.0f * fr.wind + 1.5f * fm_sin_turns(fr.t * 0.013f + fr_f[i].ph)) * dt;
        fr_f[i].th += ((0.6f + 0.4f * fm_sin_turns(fr.t * 0.004f + fr_f[i].ph * 3.0f)) - fr_f[i].th) * fm_minf(1.0f, dt * 0.2f);
        if (fr_f[i].x - fr_f[i].w > 240.0f) {
            fr_f[i].x = -fr_f[i].w - 10.0f * sc_frand();
            fr_f[i].lastx = fr_f[i].x;
            fr_f[i].y = 140.0f + 36.0f * sc_frand();
        }
        if (fr.fogamt * fr_f[i].th > 0.25f)
            for (j = 0; j < fr.lay.na; j++)
                if (fr_f[i].lastx < fr.lay.cx[j] && fr_f[i].x >= fr.lay.cx[j]) {
                    if (cd_swell(fr.lay.cx[j] * (1.0f / 240.0f), 0.3f + 0.5f * fr.fogamt * fr_f[i].th, 7000))
                        fr.swells++;
                }
    }
    /* lightning and thunder (a storm), the stones falling (the night, the storm), the owl, the bats */
    if (fr.flash > 0.0f)
        fr.flash = fm_maxf(0.0f, fr.flash - dt * 5.0f);
    if (fr.bolt_t > 0.0f)
        fr.bolt_t -= dt;
    if (fr.ch->thunder && (fr.bolt_next -= dt) <= 0.0f) {
        fr_bolt();
        fr.bolt_next = 22.0f + 40.0f * sc_frand();
    }
    if (fr.thunder_at && (int32_t)(fr.ms - fr.thunder_at) >= 0) {
        float c = fr_low(cd_cents(CD_CHORDS[cd.chord][0]), -1400.0f, -2600.0f);   /* 98..196 Hz */
        int pan = 30 + (int)(sc_rnd() % 68u);
        fr.thunder_at = 0;
        fm_voice(FR_ID_THUNDER, 7, c, 86, pan, 1600);
        fr_q_add(380u, FR_ID_THUNDER + 1, 7, c - 200.0f, 62, 127 - pan, 2200);
        fr_q_add(120u, FR_ID_TSWELL, 4, c + 1200.0f, 50, 64, 4500);
        fr.thunders++;
    }
    if (fr.ch->stones && (fr.stone_next -= dt) <= 0.0f) {
        fr.stone_next = (fr.ch->thunder ? 30.0f : 55.0f) + 60.0f * sc_frand();
        fr_stone_drop();
    }
    if (fr.fall.on) {
        float fy = fr.lay.hy + 2.0f + 8.0f * fr_n1(fr.fall.x * 0.3f, 91);
        fr.fall.v += 400.0f * dt;
        fr.fall.y += fr.fall.v * dt;
        if (fr.fall.y >= fy) {
            float c = fr_low(cd_cents(CD_CHORDS[cd.chord][0]), -1100.0f, -2300.0f);
            fr.fall.on = 0;
            fr.stones++;
            fm_voice(FR_ID_STONE, 6, c, 72 + (int)(sc_rnd() % 30u), cd_pan(fr.fall.x * (1.0f / 240.0f)), 500);
            for (i = 0; i < FR_NDUST; i++) {
                fr_d[i].x = fr.fall.x + (sc_frand() - 0.5f) * 4.0f;
                fr_d[i].y = fy - 1.0f;
                fr_d[i].vx = (sc_frand() - 0.5f) * 24.0f + 10.0f * fr.wind;
                fr_d[i].vy = -8.0f - 10.0f * sc_frand();
                fr_d[i].life = 0.8f + 0.8f * sc_frand();
            }
        }
    }
    for (i = 0; i < FR_NRIP; i++)
        if (fr_rp[i].t > 0.0f)
            fr_rp[i].t -= dt;
    for (i = 0; i < FR_NDUST; i++)
        if (fr_d[i].life > 0.0f) {
            fr_d[i].life -= dt;
            fr_d[i].vy += 6.0f * dt;
            fr_d[i].x += fr_d[i].vx * dt;
            fr_d[i].y += fr_d[i].vy * dt;
        }
    if (fr.ch->moon && (fr.owl_next -= dt) <= 0.0f) {
        fr.owl_next = 45.0f + 70.0f * sc_frand();
        fm_shot(3, FM_SHOT_OWL, (int)(sc_rnd() % (uint32_t)fm_shot_count(FM_SHOT_OWL)), 44 + (int)(sc_rnd() % 18u),
                sc_rnd() & 1u ? 24 : 104, 95 + (int)(sc_rnd() % 9u));
        fr.owls++;
    }
    fr.bat_wait -= dt;
    if (fr.ch->bats) {
        float mt, mx, my;
        fr_moon_xy(&mt, &mx, &my);
        for (i = 0; i < fr.ch->bats && i < FR_NBAT; i++) {
            float dx, dy;
            fr_b[i].t += dt;
            if (sc_frand() < dt * 3.0f) {             /* (a new dart every third of a second or so) */
                fr_b[i].vx = (sc_frand() < 0.7f ? 1.0f : -1.0f) * (fr_b[i].vx > 0.0f ? 1.0f : -1.0f) * (30.0f + 40.0f * sc_frand());
                fr_b[i].vy = (sc_frand() - 0.5f) * 50.0f;
            }
            fr_b[i].x += fr_b[i].vx * dt;
            fr_b[i].y += fr_b[i].vy * dt;
            if (fr_b[i].y < 10.0f)
                fr_b[i].vy = fm_fabsf(fr_b[i].vy);
            if (fr_b[i].y > FR_HORIZON - 4.0f)
                fr_b[i].vy = -fm_fabsf(fr_b[i].vy);
            if (fr_b[i].x > 250.0f)
                fr_b[i].x = -8.0f;
            if (fr_b[i].x < -10.0f)
                fr_b[i].x = 248.0f;
            dx = fr_b[i].x - mx;
            dy = fr_b[i].y - my;
            if (mt > 0.0f && mt < 1.0f && dx * dx + dy * dy < fr.mn.r * fr.mn.r && fr.bat_wait <= 0.0f) {
                fr.bat_wait = 4.0f + 7.0f * sc_frand();
                fr.batmoon++;
                if (!cd_quiet())
                    fr_q_add(0u, FR_ID_TICK + (int)(fr.batmoon & 3u), 3, cd_cents(cd.lo + cd.n - 1 - (int)(sc_rnd() % 3u)) + 1200.0f,
                             16 + (int)(sc_rnd() % 8u), cd_pan(fr_b[i].x * (1.0f / 240.0f)), 1200);
            }
        }
    }
    cd_step(dt_ms);
    for (i = 0; i < FR_NQ; i++)
        if (fr_q[i].at && (int32_t)(fr.ms - fr_q[i].at) >= 0) {
            fm_voice(fr_q[i].id, fr_q[i].part, fr_q[i].cents, fr_q[i].vel, fr_q[i].pan, fr_q[i].dur);
            fr_q[i].at = 0;
        }
    cd_depth_send(fr.kv[FK_DEPTH]);
    {   /* the palette, when the look has moved (a step of 1/32, at most four times a second: every color changes,
         * so the whole screen is sent), or at once for a flash */
        int lv = (int)(fr.light * 32.0f) + ((int)fr.sky1[1] / 6) * 64 + ((int)(fr.ivy * 8.0f) + (int)(fr.moonamt * 8.0f) * 16 +
                 (int)(fr.sunamt * 8.0f) * 256 + (int)(fr.flash * 6.0f) * 4096) * 4096 + ((int)fr.gnd[2] / 8) * 65536 * 64,
            d = lv - fr.pal_lv;
        if (fr.pal_lv < 0 || !dt_ms || (lv != fr.pal_lv && (fr.ms - fr.pal_ms >= 250u || d > 2 || d < -2 || fr.flash > 0.0f))) {
            fr.pal_lv = lv;
            fr.pal_ms = fr.ms;
            fr_palette();
        }
    }
}

SC_SMALL static void fr_key(int key, int on)
{
    int b, i;
    if (!on)
        return;
    for (b = 0; b < FM_NBLACK; b++)
        if (BLACK_K[b] == key) {
            if (b < 4) {                              /* a crow calls (one not here: far off) */
                if (fr_c[b].on && !fr_c[b].st) {
                    fr_caw(b, 58, fr_c[b].x);
                    fr_c[b].call = 0.45f;
                } else {
                    fr_caw(b, 30, 30.0f + 60.0f * (float)b);
                }
            } else if (b < 7) {                       /* an arch sings */
                fr_harp((b - 4) % (fr.lay.na > 0 ? fr.lay.na : 1), 0.9f);
            } else if (b == 7) {                      /* a gust */
                fr.gph = 1;
                fr.gt = 0.0f;
                fr.grise = 1.4f;
                fr.ghold = 2.0f;
                fr.gfall = 7.0f;
                fr.gpeak = 1.0f;
            } else if (b == 8) {                      /* a fog bank rolls in */
                int lo = 0;
                for (i = 1; i < FR_NFOG; i++)
                    if (fr_f[i].x > fr_f[lo].x)
                        lo = i;
                fr_f[lo].x = -fr_f[lo].w + 4.0f;
                fr_f[lo].lastx = fr_f[lo].x;
                fr_f[lo].th = 1.0f;
                fr.fog_key = 25.0f;
            } else if (b == 9) {
                fr_stone_drop();
            } else {
                fr_bolt();
            }
            fr.key_note = b;
            return;
        }
    for (b = 0; b < FM_NWHITE; b++)
        if (WHITE_K[b] == key) {
            int k = b + 2;                            /* (sixteen notes, each its own, rising) */
            float x = 12.0f + 216.0f * (float)b / (float)(FM_NWHITE - 1);
            cd_note_key(k, x * (1.0f / 240.0f), 0.5f + 0.25f * (1.0f - (float)b / (float)(FM_NWHITE - 1)), 1400, 1, 0u);
            fr.key_note = k;
            for (i = 0; i < FR_NL; i++)
                if (!fr_l[i].st) {
                    fr_leaf_new(i, 0);
                    fr_l[i].x = x;
                    fr_l[i].silent = 1;
                    break;
                }
            return;
        }
}

SC_SMALL static void fr_knob(int i, int32_t e)
{
    if (i == FK_MOOD) {                               /* a mood: every knob to the sound, its key, scale, look */
        (void)e;
        fr_mood_tune();
        cd_activity((float)fr.kv[FK_ACT] * 0.01f);
        cd_arc_amount((float)fr.kv[FK_ARC] * 0.01f);
        fm_part_patch(1, FR_TONE_SLOT[fr.kv[FK_TONE] % FR_NTONE]);
        fr_drone();
        fr_mix_levels();
        fr.ch = 0;                                    /* (fr_look takes the new mood's events at the next step) */
        fr_look(0.0f, 0);
        fr_fx();
        return;
    }
    if (i == FK_SPACE) {
        knob_set(P_REVERB, proj.par[P_REVERB] + (int)(e * sc_knob_mul));
        return;
    }
    if (i < 0 || i >= SC_NKNOB || FR_KNOB[i][1] <= FR_KNOB[i][0])
        return;
    if (i == FK_TONE || i == FK_VOICES)
        sc_knob_wrap(&fr.kv[i], e, FR_KNOB[i][0], FR_KNOB[i][1]);
    else
        sc_knob_turn(&fr.kv[i], e, FR_KNOB[i][0], FR_KNOB[i][1], FR_KNOB[i][2]);
    switch (i) {
    case FK_ACT: cd_activity((float)fr.kv[FK_ACT] * 0.01f); break;
    case FK_TONE: fm_part_patch(1, FR_TONE_SLOT[fr.kv[FK_TONE] % FR_NTONE]); break;
    case FK_VOICES: fr_drone(); break;
    case FK_ECHO: case FK_TAPE: fr_fx(); break;
    case FK_ARC: cd_arc_amount((float)fr.kv[FK_ARC] * 0.01f); break;
    case FK_NOTES: case FK_DRONE: case FK_AIR: case FK_HARP: fr_mix_levels(); break;
    default: break;                                   /* (Crows, Fog, Wind, Warmth: read each step) */
    }
}

SC_SMALL static void fr_knob_text(int i, char *name, char *val)
{
    static const char *const N[SC_NUSED] = {"Mood", "Activity", "Instrument", "Space", "Notes", "Drone", "Air", "Harp",
                                            "Voices", "Crows", "Fog", "Wind", "Echo", "Warmth", "Tape", "Tide"};
    if (i < 0 || i >= SC_NUSED || (i != FK_SPACE && FR_KNOB[i][1] <= FR_KNOB[i][0])) {
        name[0] = val[0] = 0;
        return;
    }
    copy_str(name, N[i], 11);
    switch (i) {
    case FK_MOOD: copy_str(val, FR_MOOD[fr_mood_now()].name, 11); break;
    case FK_TONE: copy_str(val, FR_TONE_NAME[fr.kv[FK_TONE] % FR_NTONE], 11); break;
    case FK_VOICES: copy_str(val, FR_CHOIR_NAME[fr.kv[FK_VOICES] % FR_NCHOIR], 11); break;
    case FK_SPACE: itoa_u((uint32_t)proj.par[P_REVERB], val); break;
    default: itoa_u((uint32_t)fr.kv[i], val); break;
    }
}

/* a new ruin (PLAY; the glue fades through dark): other arches, another wall, another moon; the world goes on */
SC_SMALL static void fr_regrow(uint32_t seed)
{
    int i;
    fr.seed = seed | 1u;
    fr_layout();
    fr_background();
    for (i = 0; i < FR_NCROW; i++)
        if (fr_c[i].on && !fr_c[i].st) {
            fr_c[i].perch = (int8_t)(i % fr.lay.np);
            fr_c[i].x = fr.lay.px[fr_c[i].perch];
            fr_c[i].y = fr.lay.py[fr_c[i].perch];
        }
}

SC_SMALL static void fr_mood_kv(int m, int16_t *kv)
{
    fr_defaults(kv);
    m = sc_mood_base(m, FR_NMOOD);
    kv[FK_MOOD] = (int16_t)m;
    sc_mood_set(kv, FR_MOODS[m], FR_NSET);
}
static const char *fr_mood_name(int m) { return FR_MOOD[sc_mood_base(m, FR_NMOOD)].name; }

static const scene_t SCENE_FOREST = {"Ruins", 2, fr_defaults, fr_enter, fr_leave, fr_background, fr_step, fr_draw, fr_key,
                                     fr_knob, fr_knob_text, fr_regrow, FR_SOUND, 0, 0, FR_NMOOD, fr_mood_kv, fr_mood_name, 0};
