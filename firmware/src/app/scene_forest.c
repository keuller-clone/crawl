/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 the Crawl contributors */
/* Forest Ruins: the arcade of a fallen abbey in a wood. Two or three arches stand on their piers with a stump of
 * wall at one end, the forest behind them and two old trees leaning in from the sides; leaves fall through the
 * arches onto a floor of litter and fallen stones, crows keep the wall, fog lies low among the piers, and the wind
 * comes through the arches. What is heard is what is seen:
 *
 *   a leaf lands            a note (part 1), its pitch where it lands across; on the wall, a soft touch (part 3)
 *   a crow calls            its own recorded caw (CC0: a hooded crow, a rook), heard where it perches or flies
 *   wind through an arch    the arch sings: a bowed string (part 5: PHYS, STRING, bowed), one note an arch, the
 *                           wider the lower, rising a little with the gust; leaves rush through it
 *   a fog bank passes through an arch   a swell (part 4)
 *   lightning               the ruin lit white, a bolt; the thunder after it (part 7: PHYS, MEMBRANE, low), the
 *                           crows up off the wall all at once (the flock's recording)
 *   a stone falls from the wall         a dull knock where it lands (part 6: PHYS, MODAL, inharmonic), dust
 *   a bat crosses the moon  a faint high tick (part 3)
 *   under them              the drone (part 2: a pad, or VOICE: a choir in the nave; at night one hollow voice
 *                           whose vowel the wind opens), the air (the breeze or dry leaves, recordings; at night a
 *                           synthesized one), in a storm rain and a deep roar
 *
 *   KNOB 1 Mood   2 Activity   3 Instrument (Drop, Mallet, Bell, Bamboo)   4 Space (reverb)
 *   Volume: Notes, Drone, Air, Harp (the arches' singing).   Nature: Voices (Pad, Solo, Monks, Nuns, Cathedral),
 *   Crows (how often they call; 0: never), Fog, Wind.   Music: Echo (the tape echo), Warmth, Tape, Tide
 *   White keys  a note each, rising; a leaf lets go above it and falls
 *   Black keys  the first four: a crow each calls (one not here calls far off); the next three: an arch each
 *               sings; then a gust; a fog bank rolling in; a stone falling; the top one: lightning
 *
 * Moods, each its own sound and picture (FR_MOOD and FR_MOODS, data):
 *   Dawn Mist     a pale sun low behind thick fog among the piers; the fog's banks through the arches are swells
 *                 over a soft pad; a leaf now and then; one crow far off, crossing behind the ruin; no wind
 *   Overgrown     ivy on the piers, the forest in leaf, saplings in the floor; leaves fall thick and each is a
 *                 note; three crows on the wall hopping and calling; a cathedral choir in the nave; dry leaves
 *                 in the air
 *   Storm         a slate sky, rain, the wind driving the leaves straight through; the arches sing without
 *                 pause; lightning, thunder, the crows wheeling and crying; a deep pad, a roar under it
 *   Owl Moon      the moon and the stars through bare arches, bats, thin wisps of fog, no crows: an owl; stones
 *                 fall from the wall; one hollow voice, its vowel the wind's; few notes, low, in the cloud
 *   Long Night    a day: Dawn Mist, Overgrown, Storm, Owl Moon, half an hour in all, then dawn again
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
#define FR_NA 3                                 /* arches at most */
#define FR_NPERCH 6
#define FR_NTONE 4
#define FR_NCHOIR 5
#define FR_SOUND 1
#define FR_DARK 80                              /* the panel's dark curve (render8.c r8_dark), as Crypt's */
#define FR_GROUND 176.0f                        /* the floor begins here */
#define FR_SPRING 120.0f                        /* the arches spring from here */
#define FR_NSKY 32
#define FR_NST 32                               /* the stone's shades */
enum { FR_SKY = 0, FR_FAR = 32, FR_CAN = 40, FR_STONE = 48, FR_IVY = 80, FR_LIT = 84, FR_GRASS = 92, FR_TRUNK = 96,
       FR_LEAF = 100, FR_FOG = 116, FR_CROW = 132, FR_MOON = 136, FR_HALO = 140, FR_STAR = 144, FR_SUN = 148,
       FR_RAIN = 152, FR_BOLT = 154, FR_BAT = 155, FR_DUST = 156 };
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
    uint32_t sky0, sky1, stone, lit, fogc;      /* the sky's top and its horizon; lit stone; the leaf litter; the fog */
    uint8_t leafpal;                            /* the leaves' colors (FR_LEAFPAL's row) */
    uint8_t light;                              /* % */
    uint8_t fog, wind, gust;                    /* % the banks' thickness; % the wind; s between gusts (0: none) */
    uint8_t leaves, crows, rain;                /* % how many fall; crows seen; % */
    uint8_t moon, sun, ivy, bats, stones, thunder, canopy;   /* (ivy and canopy %) */
    uint8_t vowel, buzz, breath;                /* the voice's (VOICE's knobs, 0..127) */
    uint8_t cloud;                              /* % GRAIN */
} fr_mood_t;
static const fr_mood_t FR_MOOD[FR_NMOOD] = {
    {"Long Night", -1, 0, 4, 9, 0x7E8CA0u, 0xD8C8ACu, 0xA0A4A4u, 0x6A5844u, 0xD4D8DCu, 0, 90, 100, 8, 40, 12, 1, 0, 0, 1, 0, 0, 0, 0, 25, 90, 30, 0, 0},
    {"Dawn Mist", SC_AEOLIAN, 0, 5, 8, 0x7E8CA0u, 0xD8C8ACu, 0xA0A4A4u, 0x6A5844u, 0xD4D8DCu, 0, 90, 100, 8, 40, 12, 1, 0, 0, 1, 0, 0, 0, 0, 25, 90, 30, 0, 0},
    {"Overgrown", SC_DORIAN, 0, 4, 9, 0x5880B0u, 0xBCD0B8u, 0xA89C80u, 0x54502Cu, 0xC8D0B8u, 1, 110, 8, 20, 22, 70, 3, 0, 0, 0, 100, 0, 0, 0, 100, 60, 60, 0, 0},
    {"Storm", SC_IN, -700, 2, 8, 0x343C44u, 0x707A80u, 0x6C7480u, 0x3C342Cu, 0x707880u, 2, 72, 0, 65, 7, 60, 4, 70, 0, 0, 30, 0, 1, 1, 25, 40, 80, 0, 0},
    {"Owl Moon", SC_MINYO, -500, 3, 7, 0x0C1228u, 0x344468u, 0x6C7894u, 0x282C3Cu, 0x4C5674u, 3, 70, 30, 15, 28, 6, 0, 0, 1, 0, 0, 5, 1, 0, 0, 104, 20, 22, 25},
};
static const uint8_t FR_LN[4] = {1, 2, 3, 4};   /* Long Night's day: Dawn Mist, Overgrown, Storm, Owl Moon */
#define FR_NSET 14
static const sc_set_t FR_MOODS[FR_NMOOD][FR_NSET] = {
    {{FK_ACT, 25}, {FK_TONE, 0}, {FK_NOTES, 50}, {FK_DRONE, 40}, {FK_AIR, 20}, {FK_HARP, 45}, {FK_VOICES, 0}, {FK_CROWS, 40},
     {FK_FOG, 30}, {FK_WIND, 30}, {FK_ECHO, 30}, {FK_DEPTH, 20}, {FK_TAPE, 15}, {FK_ARC, 40}},
    {{FK_ACT, 12}, {FK_TONE, 2}, {FK_NOTES, 45}, {FK_DRONE, 50}, {FK_AIR, 15}, {FK_HARP, 30}, {FK_VOICES, 0}, {FK_CROWS, 25},
     {FK_FOG, 100}, {FK_WIND, 8}, {FK_ECHO, 35}, {FK_DEPTH, 30}, {FK_TAPE, 10}, {FK_ARC, 30}},   /* (still: the pad,
                                                       * the fog's swells, a bell now and then, the crow far off) */
    {{FK_ACT, 40}, {FK_TONE, 1}, {FK_NOTES, 60}, {FK_DRONE, 40}, {FK_AIR, 12}, {FK_HARP, 50}, {FK_VOICES, 4}, {FK_CROWS, 60},
     {FK_FOG, 8}, {FK_WIND, 20}, {FK_ECHO, 20}, {FK_DEPTH, 10}, {FK_TAPE, 10}, {FK_ARC, 20}},   /* (the leaves' mallets,
                                                       * the cathedral choir, the crows at their talk) */
    {{FK_ACT, 55}, {FK_TONE, 3}, {FK_NOTES, 50}, {FK_DRONE, 45}, {FK_AIR, 55}, {FK_HARP, 70}, {FK_VOICES, 0}, {FK_CROWS, 70},
     {FK_FOG, 0}, {FK_WIND, 65}, {FK_ECHO, 40}, {FK_DEPTH, 35}, {FK_TAPE, 25}, {FK_ARC, 30}},   /* (the arches singing,
                                                       * rain, thunder, the deep pad) */
    {{FK_ACT, 15}, {FK_TONE, 2}, {FK_NOTES, 40}, {FK_DRONE, 45}, {FK_AIR, 12}, {FK_HARP, 35}, {FK_VOICES, 1}, {FK_CROWS, 0},
     {FK_FOG, 30}, {FK_WIND, 15}, {FK_ECHO, 60}, {FK_DEPTH, 40}, {FK_TAPE, 25}, {FK_ARC, 45}},   /* (one hollow voice,
                                                       * the owl, the stones, the cloud) */
};
static const uint8_t FR_TONE_SLOT[FR_NTONE] = {ZN_DROP, ZN_MALLET, ZN_BELL, ZN_BAMBOO};
static const char *const FR_TONE_NAME[FR_NTONE] = {"Drop", "Mallet", "Bell", "Bamboo"};
static const char *const FR_CHOIR_NAME[FR_NCHOIR] = {"Pad", "Solo", "Monks", "Nuns", "Cathedral"};
static const uint8_t FR_AIR_STYLE[FR_NMOOD] = {FM_AIR_BREEZE, FM_AIR_BREEZE, FM_AIR_LEAVES, FM_AIR_BREEZE, FM_AIR_BREEZE_SYNTH};
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
    uint32_t at;
    int id, part, vel, pan, dur;
    float cents;
} fr_q[FR_NQ];
static struct {
    uint32_t seed, ms;
    int16_t *kv;
    float t, phase, night, dawn;                /* Long Night's day (0..1 in half an hour); the moon's night, the sun's dawn */
    float sky0[3], sky1[3], stone[3], lit[3], fogc[3], leaf[4][3];   /* the look now (eased toward the mood's) */
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
    struct {                                    /* this ruin (the picture's seed): */
        int na;                                 /* its arches */
        float cx[FR_NA], hw[FR_NA];             /* each one's middle and half width */
        float x0, x1;                           /* the wall's span */
        uint8_t pointed, broken, stump;         /* pointed arches; the arch whose crown has fallen (bit each); the
                                                 * stump of wall: 0 left, 1 right */
        float px[FR_NPERCH], py[FR_NPERCH];     /* where a crow may sit */
        int np;
    } lay;
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
/* the wall's top at wx (world): jagged, fallen low over a broken arch's crown, the stump high at one end; past the
 * wall's ends, nothing (1000) */
static float fr_top(float wx)
{
    float t, ring, best = 1000.0f;
    int a;
    if (wx < fr.lay.x0 || wx > fr.lay.x1)
        return 1000.0f;
    t = 60.0f + 16.0f * fr_n1(wx * (1.0f / 11.0f), 31) + 7.0f * fr_n1(wx * (1.0f / 3.5f), 32);
    if (fr.lay.stump ? wx > fr.lay.x1 - 24.0f : wx < fr.lay.x0 + 24.0f)
        t = 26.0f + 8.0f * fr_n1(wx * (1.0f / 5.0f), 33);   /* (the stump: the corner that still stands) */
    for (a = 0; a < fr.lay.na; a++) {
        float dx = wx - fr.lay.cx[a], hw = fr.lay.hw[a], r2 = (hw + 7.0f) * (hw + 7.0f) - dx * dx;
        if (r2 <= 0.0f)
            continue;
        ring = FR_SPRING - fm_sqrtf(r2) * (fr.lay.pointed ? 1.25f : 1.0f);   /* the ring's outer edge here */
        if (fr.lay.broken >> a & 1) {               /* its crown fallen: the wall ends at the springing's height, a
                                                     * course or two of the ring left at each side */
            float keep = FR_SPRING - 0.55f * hw * (fr.lay.pointed ? 1.25f : 1.0f);
            if (ring < keep)
                t = fm_maxf(t, keep + 3.0f * fr_n1(wx * 0.5f, 34));
        } else if (t > ring - 3.0f) {
            t = ring - 3.0f - 4.0f * fr_n1(wx * 0.4f, 35);   /* (an arch that stands keeps its ring whole) */
        }
    }
    best = t;
    return best;
}
/* inside arch a's opening (the sky, the wood and the floor show through)? its ring (1) or nothing (0): 2 inside */
static int fr_arch(float wx, float wy, int a)
{
    float dx = wx - fr.lay.cx[a], hw = fr.lay.hw[a], ey = (wy - FR_SPRING) * (fr.lay.pointed ? 0.8f : 1.0f), r2;
    if (dx > hw + 7.0f || dx < -hw - 7.0f)
        return 0;
    if (wy >= FR_SPRING)
        return dx > -hw && dx < hw ? 2 : 0;
    r2 = dx * dx + ey * ey;
    if (r2 < hw * hw)
        return 2;
    return r2 < (hw + 7.0f) * (hw + 7.0f) ? 1 : 0;
}
static void fr_moon_xy(float *mt, float *mx, float *my)   /* the moon on its arc: up all night, whatever its phase (the
                                                           * night is the mood's quarter hour, not a whole one) */
{
    *mt = fr.night;
    *mx = 20.0f + 200.0f * *mt;
    *my = fr.moon_top - 110.0f * *mt * (1.0f - *mt);
}
static int32_t fr_trunk_x(int32_t c) { return c * 13 + 3 + (int32_t)(fr_h(c, 61) % 7u); }   /* the far trunk of cell c */
static float fr_far_top(float wx) { return 92.0f + 16.0f * fr_n1(wx * (1.0f / 23.0f), 41) + 6.0f * fr_n1(wx * (1.0f / 7.0f), 42); }

/* the ruin, drawn once: the sky, the forest behind, the wall with its arches, the floor */
SC_SMALL static void fr_background(void)
{
    surf_t *s = &r8_bg;
    float k = fr_scale(), ik = 1.0f / k;
    int32_t x, y, i;
    for (y = 0; y < s->h; y++) {
        float wy = ((float)y + 0.5f) * ik;
        uint8_t *row = s->px + y * s->w;
        for (x = 0; x < s->w; x++) {
            float wx = ((float)x + 0.5f) * ik, tone, lv, wtop;
            int32_t ix = (int32_t)wx, iy = (int32_t)wy, f, a, in = 0, ring = 0;
            if (wy < FR_GROUND && wx >= fr.lay.x0 && wx <= fr.lay.x1 && wy >= (wtop = fr_top(wx))) {
                for (a = 0; a < fr.lay.na; a++) {
                    int r = fr_arch(wx, wy, a);
                    in |= r == 2;
                    ring |= r == 1;
                }
                if (!in) {                            /* stone: courses of blocks; the ring's voussoirs; the light
                                                       * from above and the pier's rounded shade */
                    int32_t bh = 9, rw = iy / bh, off = (rw & 1) * 8, cl = (ix + off) / 16;
                    float edge = fm_minf(wy - wtop, 6.0f) * (1.0f / 6.0f), side = 1.0f;
                    tone = iy % bh == 0 || (ix + off) % 16 == 0 ? 0.55f : 0.78f + 0.3f * (float)fr_h(rw, cl) * (1.0f / 255.0f);
                    if (fr_h(rw + 77, cl) < 20u)
                        tone *= 0.72f;
                    if (fr_h(ix * 7 + 3, iy) < 22u)
                        tone *= 0.88f;                /* (pitted) */
                    if (ring) {
                        float jn = 0.0f;
                        for (a = 0; a < fr.lay.na; a++)
                            if (fr_arch(wx, wy, a) == 1)
                                jn = (float)((int32_t)((wx - fr.lay.cx[a] + fr.lay.hw[a] + 7.0f) * (1.0f / 7.0f)) & 1);
                        tone = 0.95f + 0.25f * jn;
                    }
                    for (a = 0; a < fr.lay.na; a++) { /* a pier's side toward an opening darker, its edge lit */
                        float d = fm_fabsf(wx - fr.lay.cx[a]) - fr.lay.hw[a];
                        if (wy >= FR_SPRING - 2.0f && d >= 0.0f && d < 4.0f)
                            side = wx < fr.lay.cx[a] ? 0.62f : 1.18f;
                    }
                    tone *= side * (0.72f + 0.28f * (1.0f - wy * (1.0f / FR_GROUND))) * (1.0f + 0.35f * (1.0f - edge));
                    if ((fr_h(ix / 3, iy / 2 + 500) & 7u) < 3u && wy > FR_SPRING - 20.0f &&
                        fr_n1(wx * 0.09f, 51 + iy / 40) > 0.55f) {   /* ivy: patches climbing the piers (the
                                                       * palette gives them their green, or leaves them stone) */
                        row[x] = (uint8_t)(FR_IVY + (fr_h(ix, iy + 900) & 3u));
                        continue;
                    }
                    lv = tone * 0.78f;
                    f = (int32_t)(lv * (float)((FR_NST - 1) * 16));
                    f = f < 0 ? 0 : f > (FR_NST - 1) * 16 ? (FR_NST - 1) * 16 : f;
                    row[x] = (uint8_t)(FR_STONE + (f >> 4) + r8_dith(x, y, f & 15));
                    continue;
                }
            }
            if (wy >= FR_GROUND) {                    /* the floor: leaf litter, nearer rows coarser; tufts of grass */
                float v = (wy - FR_GROUND) * (1.0f / (240.0f - FR_GROUND)), cs = 2.0f + 4.0f * v;
                int32_t gx = (int32_t)(wx / cs), gy = (int32_t)((wy - FR_GROUND) / cs);
                tone = 0.35f + 0.65f * (float)fr_h(gx, gy + 300) * (1.0f / 255.0f);
                tone *= 0.8f + 0.2f * v;
                if ((fr_h(gx, gy + 301) & 31u) == 0u && v > 0.1f) {
                    row[x] = (uint8_t)(FR_GRASS + (fr_h(gx, gy + 302) & 3u));
                    continue;
                }
                f = (int32_t)(tone * 7.0f * 16.0f);
                row[x] = (uint8_t)(FR_LIT + (f >> 4) + r8_dith(x, y, f & 15));
                continue;
            }
            {   /* the sky, and the forest behind the ruin: a haze of twigs thickening downward, trunks, the canopy's
                 * masses (green in leaf, or gone into the haze) */
                float ft = fr_far_top(wx), d, sh;
                int32_t tx = fr_trunk_x(ix / 13), tr = ix == tx || (ix == tx + 1 && wy > ft + 40.0f);
                if (wy < ft) {
                    f = (int32_t)(wy * (float)(FR_NSKY - 1) * 16.0f * (1.0f / FR_GROUND));
                    row[x] = (uint8_t)(FR_SKY + (f >> 4) + r8_dith(x, y, f & 15));
                    continue;
                }
                if (tr && wy > ft + 8.0f) {
                    row[x] = FR_FAR;
                    continue;
                }
                {   /* the canopy: a disc over each trunk, dithered */
                    float cxw = (float)tx + 0.5f, cyw = fr_far_top(cxw) + 9.0f, r = 9.0f + (float)(fr_h(tx, 62) % 7u),
                          dx = wx - cxw, dy = wy - cyw;
                    int32_t t2 = fr_trunk_x(ix / 13 + (wx < cxw ? -1 : 1)), nb = 0;   /* (and the neighbor's) */
                    if (dx * dx + dy * dy < r * r && r8_dith(x, y, 10))
                        nb = 1 + (int32_t)((dy + r) / (2.0f * r) * 3.99f);
                    cxw = (float)t2 + 0.5f;
                    cyw = fr_far_top(cxw) + 9.0f;
                    r = 9.0f + (float)(fr_h(t2, 62) % 7u);
                    dx = wx - cxw;
                    dy = wy - cyw;
                    if (!nb && dx * dx + dy * dy < r * r && r8_dith(x, y, 10))
                        nb = 5 + (int32_t)((dy + r) / (2.0f * r) * 3.99f);
                    if (nb) {
                        row[x] = (uint8_t)(FR_CAN + nb - 1);
                        continue;
                    }
                }
                d = fm_clampf((wy - ft) * (1.0f / 45.0f), 0.0f, 1.0f) * 0.82f + 0.08f;
                sh = (wy - ft) / (FR_GROUND - ft);
                if (r8_dith(x, y, (int)(d * 16.0f))) {
                    row[x] = (uint8_t)(FR_FAR + 1 + (int32_t)((1.0f - sh) * 6.0f));
                } else {
                    f = (int32_t)(wy * (float)(FR_NSKY - 1) * 16.0f * (1.0f / FR_GROUND));
                    row[x] = (uint8_t)(FR_SKY + (f >> 4) + r8_dith(x, y, f & 15));
                }
            }
        }
    }
    for (i = 0; i < 4; i++) {                         /* fallen stones on the floor */
        float sx = 20.0f + 55.0f * (float)i + (float)(fr_h(i, 71) % 30u), sy = 186.0f + (float)(fr_h(i, 72) % 44u),
              sw = 7.0f + (float)(fr_h(i, 73) % 9u), shh = 3.0f + (float)(fr_h(i, 74) % 4u);
        r8_rect(s, (int32_t)(sx * k), (int32_t)(sy * k), (int32_t)(sw * k), (int32_t)(shh * k), FR_STONE + 16);
        r8_rect(s, (int32_t)(sx * k), (int32_t)(sy * k), (int32_t)(sw * k), (int32_t)(1.0f * k + 0.5f), FR_STONE + 24);
        r8_rect(s, (int32_t)((sx - 1.0f) * k), (int32_t)((sy + shh) * k), (int32_t)((sw + 2.0f) * k), (int32_t)(1.0f * k + 0.5f), FR_LIT);
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
            fr_l[i].x = fr.lay.cx[a] - fr.lay.hw[a] - 6.0f;
            fr_l[i].y = FR_SPRING + 8.0f + 36.0f * sc_frand();
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
            fr_c[j].tx = fr_c[j].x + (sc_frand() - 0.5f) * 80.0f;
            fr_c[j].ty = 40.0f + 50.0f * sc_frand();
            fr_c[j].wait = 12.0f + 10.0f * sc_frand();
        }
    if (fr.kv[FK_CROWS] > 0 && fr.ch->crows >= 2) {
        fm_shot(3, FM_SHOT_FLOCK, 0, 52, 64, 100);
        fr.flocks++;
    }
}

static void fr_stone_drop(void)
{
    float x, t;
    int i;
    if (fr.fall.on)
        return;
    for (i = 0; i < 8; i++) {                         /* (from where the wall is low: a broken place) */
        x = fr.lay.x0 + 4.0f + (fr.lay.x1 - fr.lay.x0 - 8.0f) * sc_frand();
        t = fr_top(x);
        if (t > 70.0f && t < 500.0f)
            break;
    }
    fr.fall.on = 1;
    fr.fall.x = x;
    fr.fall.y = fr_top(x) - 1.0f;
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
    uint32_t sky0 = (uint32_t)fr.sky0[0] << 16 | (uint32_t)fr.sky0[1] << 8 | (uint32_t)fr.sky0[2],
             sky1 = (uint32_t)fr.sky1[0] << 16 | (uint32_t)fr.sky1[1] << 8 | (uint32_t)fr.sky1[2],
             stone = (uint32_t)fr.stone[0] << 16 | (uint32_t)fr.stone[1] << 8 | (uint32_t)fr.stone[2],
             lit = (uint32_t)fr.lit[0] << 16 | (uint32_t)fr.lit[1] << 8 | (uint32_t)fr.lit[2],
             fogc = (uint32_t)fr.fogc[0] << 16 | (uint32_t)fr.fogc[1] << 8 | (uint32_t)fr.fogc[2],
             shadow = 0x08080Au, far = r8_mix(0x0E1014u, sky1, 0.22f), cat[4];
    float l = fr.light;
    int i, c;
    for (i = 0; i < FR_NSKY; i++)
        r8_pal_set(FR_SKY + i, r8_mix(sky0, sky1, (float)i * (1.0f / (float)(FR_NSKY - 1))));
    for (i = 0; i < 8; i++)
        r8_pal_set(FR_FAR + i, r8_mix(far, sky1, (float)i * (1.0f / 7.0f) * 0.55f));
    for (i = 0; i < 8; i++) {                         /* the canopy: two layers, green in leaf, haze when bare */
        uint32_t g = i < 4 ? r8_mix(0x2A4420u, 0x7A9A38u, (float)i * (1.0f / 3.0f)) : r8_mix(0x1C3018u, 0x4A6A28u, (float)(i - 4) * (1.0f / 3.0f)),
                 bare = r8_mix(far, sky1, i < 4 ? 0.35f : 0.15f);
        r8_pal_set(FR_CAN + i, r8_mix(bare, r8_mix(shadow, g, fm_minf(1.0f, 0.5f + 0.5f * l)), fr.canopy));
    }
    for (i = 0; i < FR_NST; i++) {
        float u = (float)i * (1.0f / (float)(FR_NST - 1)) * l;
        uint32_t cc = r8_mix(shadow, stone, fm_minf(1.0f, u));
        if (u > 1.0f)
            cc = r8_mix(cc, 0xFFFFFFu, fm_minf(0.4f, (u - 1.0f) * 0.5f));
        r8_pal_set(FR_STONE + i, cc);
    }
    for (i = 0; i < 4; i++)
        r8_pal_set(FR_IVY + i, r8_mix(r8_base[FR_STONE + 14 + i], r8_mix(shadow, r8_mix(0x1E3A16u, 0x5C8A2Cu, (float)i * (1.0f / 3.0f)), fm_minf(1.0f, l)), fr.ivy));
    for (i = 0; i < 8; i++)
        r8_pal_set(FR_LIT + i, r8_mix(0x060608u, lit, (0.25f + 0.75f * (float)i * (1.0f / 7.0f)) * fm_minf(1.0f, l)));
    for (i = 0; i < 4; i++)
        r8_pal_set(FR_GRASS + i, r8_mix(r8_base[FR_LIT + 3], r8_mix(shadow, r8_mix(0x3A5C20u, 0x6A9A30u, (float)i * (1.0f / 3.0f)), fm_minf(1.0f, l)), 0.3f + 0.6f * fr.ivy));
    r8_pal_set(FR_TRUNK, r8_mix(0x08080Au, sky1, 0.06f));
    r8_pal_set(FR_TRUNK + 1, r8_mix(0x08080Au, stone, 0.3f * fm_minf(1.0f, l)));
    for (c = 0; c < 4; c++) {
        uint32_t lc = (uint32_t)fr.leaf[c][0] << 16 | (uint32_t)fr.leaf[c][1] << 8 | (uint32_t)fr.leaf[c][2];
        for (i = 0; i < 4; i++)
            r8_pal_set(FR_LEAF + c * 4 + i, r8_mix(shadow, lc, (0.45f + 0.2f * (float)i) * fm_minf(1.2f, l)));
    }
    cat[0] = sky1;
    cat[1] = r8_base[FR_FAR + 2];
    cat[2] = r8_base[FR_STONE + 14];
    cat[3] = r8_base[FR_LIT + 3];
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
        r8_pal_set(FR_SUN, r8_mix(sky1, 0xFFF4D8u, 0.9f * fr.sunamt));
        for (i = 1; i < 4; i++)
            r8_pal_set(FR_SUN + i, r8_mix(sky1, 0xFFE8C0u, (0.45f - 0.12f * (float)i) * fr.sunamt));
    }
    r8_pal_set(FR_RAIN, r8_mix(sky1, 0xC8D0D8u, 0.45f));
    r8_pal_set(FR_RAIN + 1, r8_mix(r8_base[FR_STONE + 14], 0xC8D0D8u, 0.4f));
    r8_pal_set(FR_BOLT, 0xF0F4FFu);
    r8_pal_set(FR_BAT, 0x030304u);
    r8_pal_set(FR_DUST, r8_mix(r8_base[FR_STONE + 14], 0xC0B8A8u, 0.5f));
    r8_pal_set(FR_DUST + 1, r8_mix(r8_base[FR_STONE + 14], 0xC0B8A8u, 0.25f));
    r8_pal_commit(0xFFFFFFu, 0xE8ECFFu, (int)(fr.flash * 210.0f));
}

/* a pixel; with `behind` only over palette entries below it (FR_FAR: the sky alone, for the sun and the stars;
 * FR_STONE: the sky and the far wood, for a crow far off: the ruin and the near trees stand in front) */
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

/* a crow: perched (its body, tail, head and beak, the head up as it calls), or flying (its wings beating) */
static void fr_draw_crow(surf_t *s, int i, float k)
{
    float x = fr_c[i].x, y = fr_c[i].y, d = (float)fr_c[i].dir, sz = fr_c[i].far ? 0.55f : 1.0f;
    int bh = fr_c[i].far ? FR_STONE : 0;
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
        float up = fr_c[i].call > 0.0f ? 1.6f : 0.0f;
        r8_tri(s, (x - 1.5f * d) * k, (y - 3.2f) * k, (x - 7.5f * d) * k, (y - 1.4f) * k, (x - 6.5f * d) * k, (y - 4.6f) * k, FR_CROW);
        r8_disc(s, x * k, (y - 3.0f) * k, 2.7f * k, FR_CROW);
        r8_disc(s, (x + 2.6f * d) * k, (y - 6.2f - up) * k, 1.8f * k, FR_CROW);
        r8_limb(s, (x + 3.8f * d) * k, (y - 6.3f - up) * k, 1.1f * k, (x + 6.6f * d) * k, (y - 5.6f - 1.4f * up) * k, 0.4f * k, FR_CROW + 2);
        fr_px(s, (int32_t)((x + 3.0f * d) * k), (int32_t)((y - 6.8f - up) * k), FR_CROW + 3, 0);
        fr_px(s, (int32_t)((x - 1.0f * d) * k), (int32_t)((y - 5.2f) * k), FR_CROW + 1, 0);
        fr_px(s, (int32_t)((x - 1.0f) * k), (int32_t)((y - 0.5f) * k), FR_CROW + 2, 0);
        fr_px(s, (int32_t)((x + 1.0f) * k), (int32_t)((y - 0.5f) * k), FR_CROW + 2, 0);
    }
}

/* a near tree: the trunk at the screen's edge and its limbs leaning in, swaying with the wind */
static void fr_draw_tree(surf_t *s, int side, float k)
{
    float d = side ? -1.0f : 1.0f, bx = side ? 228.0f : 12.0f, sway = (0.45f + 0.9f * fr.wind) * d * 5.0f + 1.0f * fm_sin_turns(fr.t * 0.11f + 0.3f * (float)side),
          tx = bx + d * 6.0f + sway;
    int j, salt = 80 + side * 10;
    r8_limb(s, bx * k, 240.0f * k, 11.0f * k, tx * k, -4.0f * k, 6.0f * k, FR_TRUNK);
    r8_limb(s, (bx + d * 5.0f) * k, 240.0f * k, 1.0f * k, (tx + d * 2.8f) * k, 0.0f, 0.8f * k, FR_TRUNK + 1);
    for (j = 0; j < 3; j++) {
        float y0 = 26.0f + 42.0f * (float)j + (float)(fr_h(j, salt) % 14u), len = 46.0f + (float)(fr_h(j, salt + 1) % 30u),
              rise = 8.0f + (float)(fr_h(j, salt + 2) % 20u), u = 1.0f - y0 * (1.0f / 240.0f),
              x0 = bx + d * 4.0f + (tx - bx - d * 6.0f) * u, x1 = x0 + d * len + sway * 1.6f * u, y1 = y0 - rise + 2.0f * fr.wind * u,
              mx = x0 + d * len * 0.55f + sway * 0.8f * u, my = y0 - rise * 0.45f;
        r8_limb(s, x0 * k, y0 * k, 4.2f * k, x1 * k, y1 * k, 1.1f * k, FR_TRUNK);
        r8_limb(s, mx * k, my * k, 1.6f * k, (mx + d * 14.0f + sway * u) * k, (my + 12.0f + 3.0f * fr.wind) * k, 0.6f * k, FR_TRUNK);
        r8_limb(s, (x0 + d * len * 0.8f + sway * u) * k, (y0 - rise * 0.8f) * k, 1.3f * k, (x1 + d * 9.0f + sway * 0.6f * u) * k, (y1 - 9.0f + 2.0f * fr.wind) * k, 0.5f * k, FR_TRUNK);
    }
}

static void fr_draw(void)
{
    surf_t *s = &r8_fb;
    float k = fr_scale();
    int i;
    r8_copy(s, &r8_bg);
    if (fr.moonamt > 0.02f) {                         /* the night sky: the stars, the moon on its way (only over the
                                                       * sky: the wood and the ruin stand in front) */
        float mt, mx, my;
        fr_moon_xy(&mt, &mx, &my);
        for (i = 0; i < FR_NSTAR; i++) {
            int tw = (int)(fr.t * 2.0f + (float)i * 0.37f) & 3;
            fr_px(s, (int32_t)(fr_s[i].x * k), (int32_t)(fr_s[i].y * k), (uint8_t)(FR_STAR + (fr_s[i].br + (tw == 0)) % 4), FR_FAR);
        }
        if (mt > -0.05f && mt < 1.05f)
            sc_moon_draw(s, k, &fr.mn, mx * k, my * k, FR_FAR, FR_MOON, FR_HALO, 4);
    }
    if (fr.sunamt > 0.02f) {                          /* the sun at dawn, low behind the mist, its glow on the sky */
        float sx = 176.0f, sy = 150.0f - 62.0f * fr.dawn;
        fr_disc(s, sx * k, sy * k, 19.0f * k, FR_SUN + 3, FR_FAR, 7);
        fr_disc(s, sx * k, sy * k, 14.0f * k, FR_SUN + 2, FR_FAR, 9);
        fr_disc(s, sx * k, sy * k, 10.5f * k, FR_SUN + 1, FR_FAR, 12);
        fr_disc(s, sx * k, sy * k, 8.0f * k, FR_SUN, FR_FAR, 16);
    }
    for (i = 0; i < FR_NCROW; i++)                    /* the crows far off, behind the ruin */
        if (fr_c[i].on && fr_c[i].far)
            fr_draw_crow(s, i, k);
    for (i = 0; i < FR_NFOG; i++) {                   /* the fog: banks lying low among the piers, each pixel a shade
                                                       * of what it covers by how thick the fog is there */
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
                else if (p < FR_FAR)
                    cat = 0;
                else if (p < FR_STONE)
                    cat = 1;
                else if (p < FR_LIT)
                    cat = 2;
                else if (p < FR_TRUNK)
                    cat = 3;
                else
                    continue;                         /* (what was drawn in front stays in front) */
                row[x] = (uint8_t)(FR_FOG + cat * 4 + lv - 1);
            }
        }
    }
    for (i = 0; i < FR_NCROW; i++)
        if (fr_c[i].on && !fr_c[i].far)
            fr_draw_crow(s, i, k);
    if (fr.fall.on)
        r8_rect(s, (int32_t)((fr.fall.x - 1.5f) * k), (int32_t)(fr.fall.y * k), (int32_t)(3.0f * k + 0.5f), (int32_t)(2.0f * k + 0.5f), FR_STONE + 18);
    for (i = 0; i < FR_NDUST; i++)
        if (fr_d[i].life > 0.0f)
            fr_px(s, (int32_t)(fr_d[i].x * k), (int32_t)(fr_d[i].y * k), (uint8_t)(FR_DUST + (fr_d[i].life < 0.6f)), 0);
    fr_draw_tree(s, 0, k);
    fr_draw_tree(s, 1, k);
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
    if (fr.ch && fr.ch->bats)                         /* the bats: a flicker of wings across the sky */
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
/* the ruin's own shape: its arches, where the wall ends, which crown has fallen, the stump; the perches; the stars */
SC_SMALL static void fr_layout(void)
{
    int i, a, n;
    float x;
    fr.lay.na = 2 + (int)(fr_h(10, 1) % 2u);
    fr.lay.pointed = (uint8_t)(fr_h(10, 2) % 3u == 0u);
    fr.lay.stump = (uint8_t)(fr_h(10, 3) & 1u);
    fr.lay.broken = (uint8_t)(1u << (fr_h(10, 4) % (uint32_t)fr.lay.na));
    if (fr_h(10, 5) % 4u == 0u)
        fr.lay.broken = 0;
    x = fr.lay.na == 3 ? 24.0f : 44.0f;
    for (a = 0; a < fr.lay.na; a++) {
        float hw = (fr.lay.na == 3 ? 24.0f : 34.0f) + (float)(fr_h(a, 6) % 9u);
        fr.lay.cx[a] = x + 10.0f + hw;
        fr.lay.hw[a] = hw;
        x += 10.0f + 2.0f * hw + 2.0f;
    }
    fr.lay.x0 = (fr.lay.na == 3 ? 24.0f : 44.0f) - 8.0f + (float)(fr_h(10, 7) % 6u);
    fr.lay.x1 = x + 6.0f + (float)(fr_h(10, 8) % 6u);
    if (fr.lay.x1 > 232.0f)
        fr.lay.x1 = 232.0f;
    n = 0;
    for (a = 0; a <= fr.lay.na && n < FR_NPERCH; a++) {   /* a perch on each pier's top, and on the stump */
        float px = a < fr.lay.na ? fr.lay.cx[a] - fr.lay.hw[a] - 5.0f : fr.lay.cx[a - 1] + fr.lay.hw[a - 1] + 5.0f, t = fr_top(px);
        if (t < 500.0f) {
            fr.lay.px[n] = px;
            fr.lay.py[n++] = t - 0.5f;
        }
    }
    if (n < FR_NPERCH) {
        float px = fr.lay.stump ? fr.lay.x1 - 12.0f : fr.lay.x0 + 12.0f;
        fr.lay.px[n] = px;
        fr.lay.py[n++] = fr_top(px) - 0.5f;
    }
    fr.lay.np = n;
    for (i = 0; i < FR_NSTAR; i++) {
        fr_s[i].x = 4.0f + 232.0f * (float)fr_h(i, 21) * (1.0f / 255.0f);
        fr_s[i].y = 3.0f + 78.0f * (float)fr_h(i, 22) * (1.0f / 255.0f);
        fr_s[i].br = (uint8_t)(fr_h(i, 23) % 3u);
    }
    sc_moon_repick(&fr.mn, 1.0f, fr.seed * 7u + 3u);
    fr.moon_top = 38.0f + (float)(fr_h(10, 9) % 20u);
    sc_mst_k = -1.0f;
    for (i = 0; i < FR_NFOG; i++) {
        fr_f[i].x = 40.0f + 80.0f * (float)i;
        fr_f[i].y = 142.0f + 14.0f * (float)i + (float)(fr_h(i, 24) % 10u);
        fr_f[i].w = 60.0f + (float)(fr_h(i, 25) % 50u);
        fr_f[i].hh = 16.0f + (float)(fr_h(i, 26) % 12u);
        fr_f[i].ph = (float)fr_h(i, 27) * (1.0f / 255.0f);
        fr_f[i].lastx = fr_f[i].x;
    }
}

static void fr_leaf_new(int i, int from_left)
{
    fr_l[i].st = 1;
    fr_l[i].x = from_left ? -8.0f : 10.0f + 220.0f * sc_frand();
    fr_l[i].y = from_left ? 20.0f + 120.0f * sc_frand() : -4.0f - 6.0f * sc_frand();
    fr_l[i].vx = 0.0f;
    fr_l[i].vy = 9.0f + 8.0f * sc_frand();
    fr_l[i].rot = sc_frand();
    fr_l[i].spin = (0.3f + 0.5f * sc_frand()) * (sc_rnd() & 1u ? 1.0f : -1.0f);
    fr_l[i].ph = sc_frand();
    fr_l[i].size = 1.8f + 1.4f * sc_frand();
    fr_l[i].land_y = fr.wind > 0.5f ? 300.0f : FR_GROUND + 6.0f + 54.0f * sc_frand();
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
    FR_EASE(fr.stone, a->stone, b->stone);
    FR_EASE(fr.lit, a->lit, b->lit);
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
    fr.kv = kv;
    for (i = 0; i < SC_NKNOB; i++)
        if (FR_KNOB[i][1] > FR_KNOB[i][0] && (kv[i] < FR_KNOB[i][0] || kv[i] > FR_KNOB[i][1]))
            kv[i] = FR_KNOB[i][3];
    fr.seed = seed | 1u;
    sc_seed(fr.seed * 2654435761u + 11u);
    fr_layout();
    fr.phase = 0.05f + 0.15f * (float)(seed % 100u) * 0.01f;   /* (Long Night: early in the morning) */
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
        int r = (int)(fr.rain * 34.0f), st = fr.ch->thunder ? 18 : 0;
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
        if (fr_l[i].y >= fr_l[i].land_y) {             /* on the floor */
            fr_l[i].st = 2;
            fr_l[i].life = 6.0f + 6.0f * sc_frand();
            fr.leaves++;
            if (!fr_l[i].silent)
                cd_pluck(fr_l[i].x * (1.0f / 240.0f), 0.3f + 0.12f * fr_l[i].size, 1400);
        } else if (fr_l[i].y < FR_GROUND && fr_l[i].x >= fr.lay.x0 && fr_l[i].x <= fr.lay.x1 && fr_l[i].vy > 0.0f) {
            float t = fr_top(fr_l[i].x);
            int in = 0;
            for (j = 0; j < fr.lay.na; j++)
                in |= fr_arch(fr_l[i].x, fr_l[i].y + 1.0f, j) == 2;
            if (!in && fr_l[i].y >= t - 1.0f && fr_l[i].y < t + 3.0f && sc_frand() < 0.7f) {   /* on the wall */
                fr_l[i].st = 2;
                fr_l[i].y = t - 1.0f;
                fr_l[i].life = 5.0f + 4.0f * sc_frand();
                fr.leaves++;
                if (!fr_l[i].silent)
                    cd_touch(fr_l[i].x * (1.0f / 240.0f), 0.5f);
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
                        fr_c[i].tx = 30.0f + 180.0f * sc_frand();
                        fr_c[i].ty = 20.0f + 70.0f * sc_frand();
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
                        fr_c[i].y = 46.0f + 40.0f * sc_frand();
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
            fr_c[i].tx = fr_c[i].x + (sc_frand() - 0.5f) * 60.0f;
            fr_c[i].ty = 30.0f + 50.0f * sc_frand();
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
                fr_c[i].tx = fr_c[i].x + (sc_frand() - 0.5f) * 40.0f;
                fr_c[i].ty = fr_c[i].y - 10.0f - 20.0f * sc_frand();
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
        float fy = FR_GROUND + 12.0f + 20.0f * fr_n1(fr.fall.x * 0.3f, 91);
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
            if (fr_b[i].y > fr_far_top(fr_b[i].x) - 6.0f)
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
                 (int)(fr.sunamt * 8.0f) * 256 + (int)(fr.flash * 6.0f) * 4096) * 4096 + ((int)fr.stone[2] / 8) * 65536 * 64,
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
