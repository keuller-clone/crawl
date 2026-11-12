/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 the Crawl contributors */
/* Night Sky ("Sky" on the panel's tab): the sky over a moor at night. A ridge runs low across the picture; on its
 * brow stands a ring of old stones, two with a lintel, and a few pines at the edge. Above it the sky takes most of the
 * screen: the stars come out, the Milky Way slants across, the moon rises in the east and crosses to the west on
 * ZEN-1's own path (a full moon at dusk, a crescent low in the west, an old moon before dawn: scene.h sc_moon_*),
 * clouds drift over it, meteors fall. What is heard is what is seen:
 *
 *   a star blinks            now and then a star wavers and brightens: a note (part 1) where it stands, higher the
 *                            higher it is
 *   a star comes out         at dusk each star appears on its own, the brightest first: a soft note each
 *   a star glints            on a cold clear night the stars scintillate: a flaring star is a chime (part 3)
 *   a meteor                 a streak across the sky and a run of chimes falling (part 3); a fireball leaves a train
 *                            that fades, and a swell under it (part 4)
 *   the aurora               curtains of green light standing in the north on a cold night; a brightening is a swell
 *                            (part 4) and the choir opens with it
 *   a cloud over the moon    the moon veiled: a swell and the sound mellows (depth); the moon clearing is a note
 *   the eclipse              the full moon slides into the earth's shadow and turns the color of copper; at each
 *                            contact a gong (part 5: PHYS MODAL, low); through the partial phases a drum far off
 *                            (part 6: PHYS MEMBRANE) beats slowly, quicker toward totality, silent in it
 *   rooks going home         a skein of rooks crosses the afterglow; a far caw now and then (Forest Ruins' recording)
 *   an owl                   in the deep of the night, from the woods below, seldom (Hotaru's recording, far)
 *   under them               the drone (part 2: a pad, or VOICE: a choir), the air (the synthesized breeze)
 *
 *   KNOB 1 Mood   2 Activity   3 Instrument (Drop, Rin, Bell, Mallet)   4 Space (reverb)
 *   Volume: Notes, Drone, Air, Chimes.   Nature: Voices (Pad, Solo, Monks, Nuns, Cathedral), Meteors (how often; 0:
 *   never), Clouds, Stars (how many are out).   Music: Echo (the tape echo), Warmth, Tape, Tide
 *   White keys  a note each, rising; a star flares where it lands
 *   Black keys  the first two: a meteor, west and east; the third: a fireball; then a cloud across the moon; the
 *               aurora pulses; the gong; the drum, a roll; the eclipse begins; another moon (the phase turns); the
 *               owl; the top one: a burst of meteors
 *
 * Moods, each its own sound and picture (NS_MOOD and NS_MOODS, data):
 *   Dusk          the sky's last light: an orange band low in the west under a mauve sky, the ridge and the stones
 *                 black against it, clouds lit from below, a thin crescent moon setting over the pines; the stars
 *                 come out one by one, a soft note each; rooks go home across the glow; a warm pad, an evening breeze
 *   Cold Clear    a frosty night: every star out and scintillating, the Milky Way, a bright gibbous moon high, frost
 *                 glittering on the ground, and the aurora standing in curtains over the ridge; the glints are chimes
 *                 and the aurora's pulses swells; a high choir; still air
 *   Eclipse       midnight, the full moon at the zenith going into the earth's shadow and out again, copper red in
 *                 totality, the sky darkening and the stars coming out round it; a gong at each contact, the drum far
 *                 off through the partial phases; the monks, low; the long echo and the cloud (GRAIN)
 *   Deep Night    the small hours, moonless and darkest: the Milky Way at its brightest, a meteor shower from one
 *                 point in the sky, each a falling run of chimes, an old crescent rising late in the east; one hollow
 *                 voice, few notes, the owl far below; the air nearly still
 *   Long Night    a night: Dusk, Cold Clear, the midnight (an eclipse when the moon is full, else clouds over the
 *                 moon), Deep Night, half an hour in all, then dusk again; it starts wherever the night is
 *
 * The world is 240 x 240; drawing scales by r8_fb.w / 240 for pixel art. Every loop has a fixed bound. */
#define NS_NSTAR 90
#define NS_NMET 3                               /* meteors at once */
#define NS_NCLOUD 3
#define NS_NPUFF 4
#define NS_NBIRD 9                              /* a skein of rooks */
#define NS_NSTONE 8
#define NS_NPINE 3
#define NS_NQ 8
#define NS_NTONE 4
#define NS_NCHOIR 5
#define NS_SOUND 1
#define NS_DARK 80                              /* the panel's dark curve (render8.c r8_dark), as the other scenes' */
#define NS_HOR 176.0f                           /* the ridge's foot */
#define NS_ECL_S 240.0f                         /* the eclipse's cycle, s (first contact to last) */
#define NS_ECL_RU 2.6f                          /* the umbra's radius, in moon radii */
#define NS_MG_S 10.0f                           /* the moon's glide to a mood's phase, s */

#define NS_NSKY 48
enum { NS_SKY = 0, NS_MW = 48, NS_STW = 52, NS_STB = 58, NS_STY = 60, NS_STH = 62, NS_HALO = 63, NS_MOON = 67,
       NS_ECL = 71, NS_AUR = 75, NS_CLD = 83, NS_CLDL = 89, NS_MET = 95, NS_GLINT = 99, NS_BIRD = 102,
       NS_GROUND = 103, NS_RIM = 111, NS_STONE = 113, NS_PINE = 117, NS_FROST = 119 };
/* (NS_SKY: the zenith down to the horizon; NS_MW: the Milky Way, three densities and its own faint stars; NS_STW: white
 * stars, faint to bright,
 * NS_STB a blue-white one and its halo, NS_STY a warm one and its halo, NS_STH a white halo; NS_HALO: the moon's glow,
 * four steps; NS_MOON: lit, toward the rim, a sea, in shadow; NS_ECL: the eclipsed moon, the umbra's centre to its
 * copper edge; NS_AUR: the aurora, six greens then two violets; NS_CLD: a cloud by its thickness, NS_CLDL the same lit
 * (by the moon, or from below at dusk); NS_MET: a meteor's head, its tail, its faint tail, a train; NS_GLINT: a
 * flaring star's cross; NS_BIRD: a rook. From NS_GROUND everything stands in front of the sky: the ground, far to
 * near, the ridge's lit rim, the stones (a face, its lit edge, the lintel, frost on top), the pines, the frost) */
#define NS_ID_GONG (FM_VOICE_ID + 70)
#define NS_ID_DRUM (FM_VOICE_ID + 71)

enum { KN_MOOD, KN_ACT, KN_TONE, KN_SPACE, KN_NOTES, KN_DRONE, KN_AIR, KN_CHIME, KN_VOICES, KN_METEORS, KN_CLOUDS,
       KN_STARS, KN_ECHO, KN_DEPTH, KN_TAPE, KN_ARC };
static const int16_t NS_KNOB[SC_NKNOB][4] = {   /* lo, hi, step, default; SPACE is the reverb (P_REVERB) */
    {0, 4, 1, 0}, {0, 100, 1, 25}, {0, NS_NTONE - 1, 1, 0}, {0, 0, 0, 0},
    {0, 100, 1, 50}, {0, 100, 1, 40}, {0, 100, 1, 12}, {0, 100, 1, 50},
    {0, NS_NCHOIR - 1, 1, 0}, {0, 100, 1, 30}, {0, 100, 1, 30}, {0, 100, 1, 50},
    {0, 100, 1, 35}, {0, 100, 1, 20}, {0, 100, 1, 15}, {0, 100, 1, 40},
};
/* a mood: what it sounds in and how the sky looks (the knobs it sets: NS_MOODS) */
#define NS_NMOOD 5
typedef struct {
    const char *name;
    int8_t scale;                               /* SC_* (tuning.h); -1: the night's own (Long Night) */
    int16_t key;                                /* cents from the player's key */
    uint8_t lo, n;                              /* its notes: white keys lo .. lo + n - 1 */
    uint32_t sky0, sky1, glow, ground;          /* the zenith, the horizon, the afterglow, the ground */
    uint8_t night;                              /* % of the night it holds (the moon's time: 0 dusk .. 100 dawn) */
    int8_t moff10, mside;                       /* its moon: the terminator x 10 (-10 full .. 6 a crescent), its side */
    uint8_t mlight;                             /* % how much of it is lit */
    uint8_t band, stars, twinkle, mw, aurora, clouds, frost, wind;   /* % the afterglow; how many stars are out; how
                                                 * hard they twinkle; the Milky Way; the aurora; the clouds; the frost
                                                 * on the ground; the wind */
    uint8_t meteors, shower, glints, appear, rooks, ecl, drum, owl;  /* s between meteors (0: none); from a radiant;
                                                 * s between glints (0: none); s between stars coming out (0: all at
                                                 * once); s between skeins (0: none); the eclipse runs; the drum beats
                                                 * in its partial phases; s between owl calls (0: none) */
    uint8_t vowel, buzz, breath;                /* the voice's (VOICE's knobs, 0..127) */
    uint8_t cloud;                              /* % GRAIN */
} ns_mood_t;
static const ns_mood_t NS_MOOD[NS_NMOOD] = {
    {"Long Night", -1, 0, 5, 8, 0x1A2448u, 0x6A5A78u, 0xE89A50u, 0x1A1612u, 10, 6, 1, 30,
     100, 35, 10, 0, 0, 60, 0, 30, 150, 0, 0, 6, 70, 0, 0, 0, 60, 40, 0, 0},
    {"Dusk", SC_AEOLIAN, 0, 5, 8, 0x1A2448u, 0x6A5A78u, 0xE89A50u, 0x1A1612u, 10, 6, 1, 30,
     100, 35, 10, 0, 0, 60, 0, 30, 150, 0, 0, 6, 70, 0, 0, 0, 60, 40, 0, 0},
    {"Cold Clear", SC_DORIAN, 200, 7, 9, 0x0A1228u, 0x2C4070u, 0x000000u, 0x161C26u, 35, -5, 1, 80,
     0, 100, 100, 50, 100, 0, 100, 10, 60, 0, 3, 0, 0, 0, 0, 0, 90, 20, 0, 0},
    {"Eclipse", SC_HUNGARIAN, -500, 2, 7, 0x100C20u, 0x302444u, 0x000000u, 0x141012u, 50, -10, 1, 100,
     0, 60, 20, 20, 0, 20, 0, 8, 0, 0, 0, 0, 0, 1, 1, 0, 30, 50, 0, 40},
    {"Deep Night", SC_HARMONIC, -300, 3, 8, 0x06081Au, 0x182440u, 0x000000u, 0x0C0C12u, 84, 6, -1, 30,
     0, 100, 30, 100, 0, 10, 0, 5, 12, 1, 0, 0, 0, 0, 0, 150, 20, 30, 20, 20},
};
static const uint8_t NS_LN[4] = {1, 2, 3, 4};   /* Long Night's night: Dusk, Cold Clear, the midnight, Deep Night */
#define NS_NSET 14
static const sc_set_t NS_MOODS[NS_NMOOD][NS_NSET] = {
    {{KN_ACT, 25}, {KN_TONE, 0}, {KN_NOTES, 50}, {KN_DRONE, 40}, {KN_AIR, 12}, {KN_CHIME, 50}, {KN_VOICES, 0}, {KN_METEORS, 30},
     {KN_CLOUDS, 30}, {KN_STARS, 50}, {KN_ECHO, 35}, {KN_DEPTH, 20}, {KN_TAPE, 15}, {KN_ARC, 40}},
    {{KN_ACT, 20}, {KN_TONE, 0}, {KN_NOTES, 50}, {KN_DRONE, 45}, {KN_AIR, 18}, {KN_CHIME, 40}, {KN_VOICES, 0}, {KN_METEORS, 5},
     {KN_CLOUDS, 60}, {KN_STARS, 35}, {KN_ECHO, 30}, {KN_DEPTH, 25}, {KN_TAPE, 10}, {KN_ARC, 30}},   /* (the warm pad, the
                                                       * stars coming out, the rooks) */
    {{KN_ACT, 35}, {KN_TONE, 1}, {KN_NOTES, 50}, {KN_DRONE, 35}, {KN_AIR, 6}, {KN_CHIME, 60}, {KN_VOICES, 3}, {KN_METEORS, 20},
     {KN_CLOUDS, 0}, {KN_STARS, 100}, {KN_ECHO, 30}, {KN_DEPTH, 5}, {KN_TAPE, 5}, {KN_ARC, 25}},   /* (the glints, the aurora's
                                                       * swells, the nuns, clean and bright) */
    {{KN_ACT, 20}, {KN_TONE, 3}, {KN_NOTES, 40}, {KN_DRONE, 50}, {KN_AIR, 4}, {KN_CHIME, 35}, {KN_VOICES, 2}, {KN_METEORS, 0},
     {KN_CLOUDS, 20}, {KN_STARS, 60}, {KN_ECHO, 55}, {KN_DEPTH, 45}, {KN_TAPE, 25}, {KN_ARC, 40}},   /* (the drum and the gong,
                                                       * the monks, the long echo) */
    {{KN_ACT, 15}, {KN_TONE, 2}, {KN_NOTES, 40}, {KN_DRONE, 40}, {KN_AIR, 3}, {KN_CHIME, 55}, {KN_VOICES, 1}, {KN_METEORS, 70},
     {KN_CLOUDS, 10}, {KN_STARS, 100}, {KN_ECHO, 60}, {KN_DEPTH, 35}, {KN_TAPE, 20}, {KN_ARC, 45}},   /* (the shower, one
                                                       * voice, the owl) */
};
static const uint8_t NS_TONE_SLOT[NS_NTONE] = {ZN_DROP, ZN_RIN, ZN_BELL, ZN_MALLET};
static const char *const NS_TONE_NAME[NS_NTONE] = {"Drop", "Rin", "Bell", "Mallet"};
static const char *const NS_CHOIR_NAME[NS_NCHOIR] = {"Pad", "Solo", "Monks", "Nuns", "Cathedral"};

static struct {
    int16_t x, y;                               /* where (world px) */
    uint8_t lv, tint, appear;                   /* its brightness, 0 faint .. 5 the brightest; its tint (0 white, 1
                                                 * blue-white, 2 warm); its place in the order of coming out */
    uint8_t want;                               /* out, or not */
    float vis, flare;                           /* how far out it is (eased); a flare, fading */
} ns_st[NS_NSTAR];
static struct {
    float x, y, vx, vy, t, life, x0, y0, train;   /* its head, velocity, age, span; where it began; its train's time left */
    int8_t on, big;
} ns_m[NS_NMET];
static struct {
    float x, y, v;
    int8_t on, np;
    float px[NS_NPUFF], py[NS_NPUFF], rx[NS_NPUFF], ry[NS_NPUFF];
} ns_c[NS_NCLOUD];
static struct {
    uint32_t at;
    int id, part, vel, pan, dur;
    float cents;
} ns_q[NS_NQ];
static struct {
    uint32_t seed, ms;
    int16_t *kv;
    float t, phase;                             /* Long Night's night (0..1 in half an hour) */
    float night;                                /* the night now, the moon's time (a mood's, reached gently) */
    sc_glide_t glide;
    float sky0[3], sky1[3], glow[3], ground[3];   /* the look now (eased toward the mood's) */
    float band, starsamt, twinkle, mw, aur, cloudamt, frost, wind, light;
    float aur_pulse;                            /* the aurora brightening, fading */
    float ecl, ecl_t, ecl_d;                    /* the eclipse: how deep (0..1, eased); the cycle's time; the umbra's
                                                 * offset from the moon, in moon radii */
    int ecl_on, ecl_ph, ecl_mid, ecl_force;     /* it runs; its phase (0 clear, 1 partial, 2 total); mid-totality
                                                 * passed; begun by a key */
    sc_moon_t mn;                               /* the moon: its size and phase (scene.h) */
    float moon_top;                             /* its arc's height */
    float nat_light, nat_off, nat_side;         /* the night's own moon (Long Night's; a mood shows its own) */
    float mn_vis, mn_cov, veil;                 /* how much of its disc is seen; how much a cloud covers; the veil
                                                 * (eased) */
    int veiled;
    struct {
        float t, x0, y0, l0, o0, s0, l1, o1, s1, o_made;
    } mg;                                       /* its glide to a mood's phase and place */
    float blink_next, appear_next, glint_next, met_next, rook_next, owl_next, pulse_next, drum_next, cloud_next;
    int air_sent, vowel_sent, out_cap;
    struct {                                    /* this moor (the picture's seed): */
        float bx, bw, bh;                       /* the brow: its middle, half width, height */
        float sx[NS_NSTONE], sy[NS_NSTONE], sh[NS_NSTONE], sw[NS_NSTONE];   /* the stones: foot, height, width */
        int8_t nst, lintel;                     /* how many; the pair under the lintel (the first of two) */
        float rx, ry;                           /* the shower's radiant */
        float mwc, mws;                         /* the Milky Way's height and slant */
    } lay;
    struct {                                    /* the rooks: a skein crossing */
        float x, y, dir, caw;
        int8_t on;
    } sk;
    uint32_t tr;                                /* the picture's own randomness */
    const ns_mood_t *ch;                        /* the mood whose events hold now (Long Night: its night's) */
    int pal_lv;
    uint32_t pal_ms;
    uint32_t blinks, appears, glints, meteors, showers, trains, veils, pulses, drums, gongs, caws, skeins, owls, moons,
             nights;                            /* (the host's tests count them) */
    int key_note;
} ns;

static float ns_scale(void) { return (float)r8_fb.w / 240.0f; }
static int ns_mood_now(void) { return sc_mood_base(ns.kv[KN_MOOD], NS_NMOOD); }
static int ns_long(void) { return NS_MOOD[ns_mood_now()].scale < 0; }
static uint32_t ns_h(int a, int b)              /* the picture's own noise: 0..255, by the seed */
{
    uint32_t h = ((uint32_t)a * 73856093u) ^ ((uint32_t)b * 19349663u) ^ ns.seed;
    h *= 2654435761u;
    return (h ^ (h >> 13)) >> 24;
}
static float ns_n1(float x, int salt)           /* smooth noise along a line, 0..1 */
{
    int i = (int)fm_floorf(x);
    float f = x - (float)i, a = (float)ns_h(i, salt), b = (float)ns_h(i + 1, salt);
    f = f * f * (3.0f - 2.0f * f);
    return (a + (b - a) * f) * (1.0f / 255.0f);
}
static uint32_t ns_tr(void)                     /* the picture's randomness: 0..255 */
{
    ns.tr = ns.tr * 1664525u + 1013904223u;
    return ns.tr >> 24;
}
static float ns_trf(void) { return (float)ns_tr() * (1.0f / 255.0f); }

/* ------------------------------------------------------------- the moor --- */
/* the ridge's crest at x (world units): the brow rises toward the stones, the rest low and uneven */
static float ns_ridge_y(float wx)
{
    float d = (wx - ns.lay.bx) / ns.lay.bw, b = fm_maxf(0.0f, 1.0f - d * d);
    return NS_HOR - ns.lay.bh * b * b - 2.5f * ns_n1(wx * 0.06f, 3) - 1.0f * ns_n1(wx * 0.25f, 4);
}

/* a pixel; with `under` only over palette entries below it (NS_HALO: the sky, the Milky Way and the stars; NS_GROUND:
 * everything in the sky) */
static inline void ns_px(surf_t *s, int32_t x, int32_t y, uint8_t c, int under)
{
    if ((uint32_t)x < (uint32_t)s->w && (uint32_t)y < (uint32_t)s->h) {
        uint8_t *p = s->px + y * s->w + x;
        if (!under || *p < under)
            *p = c;
    }
}

/* the moon's path, by its phase, this night (scene.h sc_moon_t_of): x across the sky (east left), y on its arc;
 * below the ridge when it is down. In a glide (a mood's moon) between where it was and where the new phase stands */
static float ns_moon_x_of(float off, float side) { return 16.0f + 208.0f * fm_clampf(sc_moon_t_of(ns.night, off, side), -0.05f, 1.05f); }
static float ns_moon_y_of(float off, float side)
{
    float t = sc_moon_t_of(ns.night, off, side);
    if (t < 0.0f || t > 1.0f)
        return 260.0f;
    return 184.0f - ns.moon_top * fm_sin_turns(t * 0.5f);
}
static float ns_mg_u(void)
{
    float u = fm_clampf(ns.mg.t / NS_MG_S, 0.0f, 1.0f);
    return u * u * (3.0f - 2.0f * u);
}
static float ns_moon_x(void)
{
    if (ns.mg.t < NS_MG_S)
        return ns.mg.x0 + (ns_moon_x_of(ns.mg.o1, ns.mg.s1) - ns.mg.x0) * ns_mg_u();
    return ns_moon_x_of(ns.mn.off, ns.mn.side);
}
static float ns_moon_y(void)
{
    if (ns.mg.t < NS_MG_S)
        return ns.mg.y0 + (ns_moon_y_of(ns.mg.o1, ns.mg.s1) - ns.mg.y0) * ns_mg_u();
    return ns_moon_y_of(ns.mn.off, ns.mn.side);
}
/* the moon paling into the twilight at dusk and dawn (0 gone .. 1 its own) */
static float ns_moon_tw(void) { return fm_clampf(fm_minf(ns.night, 1.0f - ns.night) * 25.0f + 0.1f, 0.0f, 1.0f); }
/* a moon's phase, reached at once (a scene starting) or gliding over NS_MG_S (a mood turned): its shadow sweeps across
 * its face to the new phase, through the full moon if it changes side */
static void ns_moon_to(float l, float o, float sd, int glide)
{
    if (!glide) {
        ns.mn.light = l;
        ns.mn.off = o;
        ns.mn.side = sd;
        ns.mg.t = 99.0f;
        sc_mst_k = -1.0f;                             /* (its face made again) */
        return;
    }
    ns.mg.x0 = ns_moon_x();                           /* (from where it is now, even mid-glide) */
    ns.mg.y0 = ns_moon_y();
    ns.mg.l0 = ns.mn.light;
    ns.mg.o0 = ns.mn.off;
    ns.mg.s0 = ns.mn.side;
    ns.mg.l1 = l;
    ns.mg.o1 = o;
    ns.mg.s1 = sd;
    ns.mg.o_made = ns.mn.off;
    ns.mg.t = 0.0f;
}
static void ns_mood_moon(int m, int glide)
{
    const ns_mood_t *md = &NS_MOOD[m];
    if (md->scale < 0)
        ns_moon_to(ns.nat_light, ns.nat_off, ns.nat_side, glide);
    else
        ns_moon_to((float)md->mlight * 0.01f, (float)md->moff10 * 0.1f, (float)md->mside, glide);
}
static void ns_moon_glide(float dt)             /* (each step) */
{
    float u, o, l;
    if (ns.mg.t >= NS_MG_S)
        return;
    ns.mg.t += dt;
    u = ns_mg_u();
    l = ns.mg.l0 + (ns.mg.l1 - ns.mg.l0) * u;
    if (ns.mg.s0 != ns.mg.s1) {                       /* the side changes: through the full moon */
        if (u < 0.5f) {
            o = ns.mg.o0 + (-1.0f - ns.mg.o0) * (u * 2.0f);
            ns.mn.side = ns.mg.s0;
        } else {
            o = -1.0f + (ns.mg.o1 + 1.0f) * (u * 2.0f - 1.0f);
            ns.mn.side = ns.mg.s1;
        }
    } else {
        o = ns.mg.o0 + (ns.mg.o1 - ns.mg.o0) * u;
        ns.mn.side = ns.mg.s1;
    }
    if (ns.mg.t >= NS_MG_S) {
        o = ns.mg.o1;
        l = ns.mg.l1;
        ns.mn.side = ns.mg.s1;
    }
    ns.mn.light = l;
    if (fm_fabsf(o - ns.mg.o_made) > 0.04f || ns.mg.t >= NS_MG_S) {
        ns.mn.off = o;
        ns.mg.o_made = o;
        sc_mst_k = -1.0f;
    }
}

/* the picture, drawn once: the sky, the Milky Way, the ridge and its ground, the stones, the pines (the moon, the
 * stars, the aurora, the clouds and the meteors move: ns_draw) */
SC_SMALL static void ns_background(void)
{
    surf_t *s = &r8_bg;
    float k = ns_scale(), ik = 1.0f / k;
    int32_t x, y, i;
    ns.tr = ns.seed * 747796405u + 2891336453u;
    r8_vgrad_wide(s, 0, (int32_t)((NS_HOR + 6.0f) * k), NS_SKY, NS_NSKY, 8);
    for (x = 0; x < s->w; x++) {                      /* the Milky Way: a soft band slanting across, lumpy, with a dark
                                                       * rift along it; its faint stars thickest in it */
        float wx = ((float)x + 0.5f) * ik, cy = ns.lay.mwc + ns.lay.mws * (wx - 120.0f) + 7.0f * fm_sin_turns(wx / 160.0f + 0.2f),
              w = 15.0f + 5.0f * fm_sin_turns(wx / 57.0f + 0.4f), lump = 0.6f + 0.4f * ns_n1(wx * 0.045f, 21) * ns_n1(wx * 0.11f + 3.0f, 22);
        int32_t ya = (int32_t)((cy - w) * k), yb = (int32_t)((cy + w) * k);
        for (y = ya < 0 ? 0 : ya; y <= yb && y < s->h; y++) {
            float v = (((float)y + 0.5f) * ik - cy) / w, a = (1.0f - v * v) * lump, rift = v - 0.12f;
            int t, l;
            if (a <= 0.0f)
                continue;
            a *= 1.0f - 0.65f * fm_clampf(1.0f - rift * rift * 30.0f, 0.0f, 1.0f);
            t = (int)(a * 3.0f * 16.0f);
            l = (t >> 4) + r8_dith(x, y, t & 15);
            if (l > 0)
                s->px[y * s->w + x] = (uint8_t)(NS_MW + (l > 3 ? 2 : l - 1));
        }
    }
    for (i = 0; i < 160; i++) {                       /* the faintest stars, most along the band (the palette fades
                                                       * them with the others) */
        uint32_t h = r8_mix32((uint32_t)i * 2654435761u, ns.seed ^ 0x5D3Bu);
        float sx = (float)(h % 2400u) * 0.1f, sy;
        if (i % 3)
            sy = ns.lay.mwc + ns.lay.mws * (sx - 120.0f) + 7.0f * fm_sin_turns(sx / 160.0f + 0.2f) + ((float)((h >> 12) & 255u) - 127.5f) * 0.13f;
        else
            sy = 3.0f + (float)((h >> 12) % 1500u) * 0.1f;
        if (sy < 3.0f || sy > ns_ridge_y(sx) - 6.0f)
            continue;
        ns_px(s, (int32_t)(sx * k), (int32_t)(sy * k), NS_MW + 3, NS_STW);
    }
    for (x = 0; x < s->w; x++) {                      /* the ridge and the ground below it: heather, dark, its crest lit */
        float wx = ((float)x + 0.5f) * ik, ry = ns_ridge_y(wx);
        int32_t y0 = (int32_t)(ry * k);
        for (y = y0 < 0 ? 0 : y0; y < s->h; y++) {
            float wy = ((float)y + 0.5f) * ik, d = wy - ry, tone;
            int32_t f;
            if (d < 1.0f * ik + 0.5f) {
                s->px[y * s->w + x] = (uint8_t)(NS_RIM + (d >= 0.5f * ik + 0.25f));
                continue;
            }
            tone = fm_clampf(d * (1.0f / 60.0f), 0.0f, 1.0f) * 0.55f + 0.45f * ns_n1(wx * 0.12f + wy * 0.07f, 5) * ns_n1(wy * 0.3f, 6);
            f = (int32_t)(tone * 7.0f * 16.0f);
            f = f > 7 * 16 ? 7 * 16 : f;
            s->px[y * s->w + x] = (uint8_t)(NS_GROUND + (f >> 4) + r8_dith(x, y, f & 15));
        }
    }
    {   /* the stones, the far ones first (an index sort by their feet, eight at most): a face, its edge lit on the
         * west (the afterglow's, the moon's), frost on its top; the lintel across its pair */
        int order[NS_NSTONE], n = ns.lay.nst, a, b;
        for (i = 0; i < n; i++)
            order[i] = i;
        for (a = 0; a < n; a++)
            for (b = a + 1; b < n; b++)
                if (ns.lay.sy[order[b]] < ns.lay.sy[order[a]]) {
                    int t = order[a];
                    order[a] = order[b];
                    order[b] = t;
                }
        for (a = 0; a < n; a++) {
            float sx, sy, sh, sw;
            i = order[a];
            sx = ns.lay.sx[i];
            sy = ns.lay.sy[i];
            sh = ns.lay.sh[i];
            sw = ns.lay.sw[i];
            int32_t X = (int32_t)((sx - sw * 0.5f) * k), Y = (int32_t)((sy - sh) * k), W = (int32_t)(sw * k + 0.5f), H = (int32_t)(sh * k + 1.0f);
            r8_rect(s, X, Y, W < 1 ? 1 : W, H, NS_STONE);
            r8_rect(s, X + W - (int32_t)(k + 0.5f), Y, (int32_t)(k + 0.5f), H, NS_STONE + 1);
            r8_rect(s, X, Y, W < 1 ? 1 : W, (int32_t)(k + 0.5f), NS_STONE + 3);
            if (i == ns.lay.lintel && a + 1 <= n) {   /* the lintel across this stone and the next in the ring */
                int o = (i + 1) % n;
                float x0 = fm_minf(sx, ns.lay.sx[o]) - sw * 0.6f, x1 = fm_maxf(sx, ns.lay.sx[o]) + sw * 0.6f,
                      top = fm_minf(sy - sh, ns.lay.sy[o] - ns.lay.sh[o]);
                r8_rect(s, (int32_t)(x0 * k), (int32_t)((top - 2.2f) * k), (int32_t)((x1 - x0) * k + 0.5f), (int32_t)(2.2f * k + 1.0f), NS_STONE + 2);
                r8_rect(s, (int32_t)(x0 * k), (int32_t)((top - 2.2f) * k), (int32_t)((x1 - x0) * k + 0.5f), (int32_t)(k + 0.5f), NS_STONE + 3);
            }
        }
    }
    for (i = 0; i < NS_NPINE; i++) {                  /* the pines at the edge of the moor, on the right */
        float wx = 204.0f + 12.0f * (float)i + 6.0f * ns_trf(), base = ns_ridge_y(wx) + 1.0f, ht = 12.0f + 10.0f * ns_trf(),
              hw = 2.6f + 1.8f * ns_trf();
        r8_tri(s, (wx - hw) * k, base * k, (wx + hw) * k, base * k, wx * k, (base - ht) * k, NS_PINE + (i & 1));
        r8_tri(s, (wx - hw * 0.7f) * k, (base - ht * 0.45f) * k, (wx + hw * 0.7f) * k, (base - ht * 0.45f) * k, wx * k, (base - ht - 2.0f) * k, NS_PINE + (i & 1));
    }
}

/* ------------------------------------------------------------- sounds --- */
static void ns_q_add(uint32_t delay_ms, int id, int part, float cents, int vel, int pan, int dur)
{
    int i;
    for (i = 0; i < NS_NQ; i++)
        if (!ns_q[i].at) {
            ns_q[i].at = ns.ms + delay_ms + 1u;
            ns_q[i].id = id;
            ns_q[i].part = part;
            ns_q[i].cents = cents;
            ns_q[i].vel = vel;
            ns_q[i].pan = pan;
            ns_q[i].dur = dur;
            return;
        }
}
static float ns_low(float c, float hi, float lo)     /* c brought into the octave hi .. lo (cents from A4, hi > lo) */
{
    while (c > hi)
        c -= 1200.0f;
    while (c < lo)
        c += 1200.0f;
    return c;
}
static float ns_root(void) { return cd_cents(CD_CHORDS[cd.chord][0]); }
static float ns_star_pos(int i) { return fm_clampf((float)ns_st[i].x * (1.0f / 240.0f), 0.0f, 1.0f); }

/* a star blinks: one of the stars that are out wavers and brightens for a moment, a note where it stands (higher the
 * higher it is: the position across picks the note, its height the strength) */
static void ns_blink(void)
{
    int i, t, pick = -1;
    for (t = 0; t < 12; t++) {
        i = (int)(sc_rnd() % NS_NSTAR);
        if (ns_st[i].vis > 0.6f && ns_st[i].flare < 0.2f) {
            pick = i;
            break;
        }
    }
    if (pick < 0)
        return;
    ns_st[pick].flare = 0.8f;
    if (!cd_quiet() && cd_pluck(ns_star_pos(pick), 0.3f + 0.5f * (1.0f - (float)ns_st[pick].y / NS_HOR), 1800))
        ns.blinks++;
}
/* a star comes out (dusk): the next in the order of appearing among those under the cap; a soft note. When all are out,
 * one of the faintest goes back (the eye wanders), to come out again */
static void ns_appear(void)
{
    int i, best = -1, cap = ns.out_cap, nout = 0;
    for (i = 0; i < NS_NSTAR; i++) {
        if (ns_st[i].want)
            nout++;
        else if ((int)ns_st[i].appear < cap && (best < 0 || ns_st[i].appear < ns_st[best].appear))
            best = i;
    }
    if (best < 0) {
        for (i = 0; i < 8; i++) {
            int j = (int)(sc_rnd() % NS_NSTAR);
            if (ns_st[j].want && ns_st[j].lv <= 1) {
                ns_st[j].want = 0;
                break;
            }
        }
        return;
    }
    ns_st[best].want = 1;
    ns_st[best].flare = 0.5f;
    if (!cd_quiet() && cd_pluck(ns_star_pos(best), 0.25f + 0.1f * (float)ns_st[best].lv, 2000))
        ns.appears++;
}
/* a star glints (a cold clear night): one of the brighter ones flares, a chime (part 3) */
static void ns_glint(int heard)
{
    int i, t, pick = -1;
    for (t = 0; t < 12; t++) {
        i = (int)(sc_rnd() % 20u);                    /* (the brighter ones are first in the table) */
        if (ns_st[i].vis > 0.6f && ns_st[i].flare < 0.3f) {
            pick = i;
            break;
        }
    }
    if (pick < 0)
        return;
    ns_st[pick].flare = 1.0f;
    if (heard && !cd_quiet()) {
        cd_chime(ns_star_pos(pick), 1, 0.3f + 0.1f * (float)ns_st[pick].lv);
        ns.glints++;
    }
}
/* a meteor: a streak across the sky and a run of chimes falling (part 3). dir: -1 west to east (leftward), 1 the other
 * way, 0 as the night has it (from the shower's radiant, or anywhere); a big one is a fireball: slower, longer, a swell
 * under it (part 4) and a train left behind */
static void ns_meteor(int dir, int big, float strength, int heard)
{
    int i, n;
    float pos, sp;
    for (i = 0; i < NS_NMET; i++)
        if (!ns_m[i].on && ns_m[i].train <= 0.0f)
            break;
    if (i >= NS_NMET)
        return;
    if (ns.ch && ns.ch->shower && dir == 0) {        /* from the radiant, outward, mostly downward */
        float a = 0.08f + 0.34f * sc_frand(), d = 10.0f + 30.0f * sc_frand(), cs, sn;
        if (sc_frand() < 0.5f)
            a = 0.5f - a;
        cs = fm_sin_turns(a + 0.25f);                 /* (cos) */
        sn = fm_sin_turns(a);
        sp = 150.0f + 110.0f * sc_frand();
        ns_m[i].x = ns.lay.rx + cs * d;
        ns_m[i].y = ns.lay.ry + sn * d;
        ns_m[i].vx = cs * sp;
        ns_m[i].vy = sn * sp;
        ns.showers++;
    } else {
        dir = dir ? dir : sc_frand() < 0.5f ? -1 : 1;
        sp = 140.0f + 80.0f * sc_frand();
        ns_m[i].x = 30.0f + 180.0f * sc_frand();
        ns_m[i].y = 8.0f + 55.0f * sc_frand();
        ns_m[i].vx = (float)dir * sp;
        ns_m[i].vy = 50.0f + 60.0f * sc_frand();
    }
    if (big) {
        ns_m[i].vx *= 0.75f;
        ns_m[i].vy *= 0.75f;
    }
    ns_m[i].t = 0.0f;
    ns_m[i].life = big ? 1.1f + 0.4f * sc_frand() : 0.35f + 0.4f * sc_frand();
    ns_m[i].x0 = ns_m[i].x;
    ns_m[i].y0 = ns_m[i].y;
    ns_m[i].on = 1;
    ns_m[i].big = (int8_t)big;
    ns.meteors++;
    pos = fm_clampf(ns_m[i].x * (1.0f / 240.0f), 0.0f, 1.0f);
    if (heard || !cd_quiet()) {
        n = (big ? 5 : 3) + (int)(sc_rnd() % (big ? 3u : 2u));
        cd_run(pos, -1, n, big ? 110u : 80u, 0.35f + 0.45f * strength + (big ? 0.2f : 0.0f));
        if (big) {
            ns_q_add(400u, 0, 4, pos, 45, 0, 4500);    /* (a swell under it: ns_step plays it as cd_swell) */
            ns.trains++;
        }
    }
}
/* the aurora pulses: a curtain brightens, a swell (part 4) */
static void ns_pulse(float strength)
{
    ns.aur_pulse = fm_maxf(ns.aur_pulse, 0.6f + 0.4f * strength);
    if (!cd_quiet() && cd_swell(0.25f + 0.5f * sc_frand(), 0.3f + 0.3f * strength, 5000))
        ns.pulses++;
}
/* the eclipse's gong: struck metal, low (the root in 65..130 Hz), inharmonic, ringing on (part 5: PHYS MODAL) */
static void ns_gong(float strength)
{
    float c = ns_low(ns_root(), -2100.0f, -3300.0f);
    fm_voice(NS_ID_GONG, 5, c, 50 + (int)(45.0f * strength), 60 + (int)(sc_rnd() % 9u), 7000);
    ns.gongs++;
}
/* the drum far off (part 6: PHYS MEMBRANE), the root in 55..110 Hz, soft */
static void ns_drum(float strength)
{
    float c = ns_low(ns_root(), -2400.0f, -3600.0f);
    fm_voice(NS_ID_DRUM, 6, c, 34 + (int)(44.0f * strength), 50 + (int)(sc_rnd() % 7u), 600);
    ns.drums++;
}
/* a far caw (the rooks going home: Forest Ruins' crows, quiet, a little slow for the distance) */
static void ns_caw(float pos)
{
    int n = fm_shot_count(FM_SHOT_CROW);
    if (n <= 0)
        return;
    fm_shot(2, FM_SHOT_CROW, (int)(sc_rnd() % (uint32_t)n), 16 + (int)(sc_rnd() % 10u), cd_pan(pos), 84 + (int)(sc_rnd() % 8u));
    ns.caws++;
}
/* the owl, from the woods below the ridge, far (Hotaru's recording) */
static void ns_owl(void)
{
    int n = fm_shot_count(FM_SHOT_OWL);
    if (n <= 0)
        return;
    fm_shot(3, FM_SHOT_OWL, (int)(sc_rnd() % (uint32_t)n), 26 + (int)(sc_rnd() % 10u), cd_pan(0.15f + 0.7f * sc_frand()), 92 + (int)(sc_rnd() % 7u));
    ns.owls++;
}
/* a skein of rooks sets out across the sky, low, toward their roost (right, or left), with a caw or two */
static void ns_skein(void)
{
    ns.sk.on = 1;
    ns.sk.dir = sc_frand() < 0.75f ? 1.0f : -1.0f;
    ns.sk.x = ns.sk.dir > 0.0f ? -30.0f : 270.0f;
    ns.sk.y = 95.0f + 45.0f * sc_frand();
    ns.sk.caw = 1.5f + 3.0f * sc_frand();
    ns.skeins++;
}

static void ns_cloud_new(int c, float x, float y)
{
    int j;
    float sc = 0.8f + 0.6f * sc_frand(), ext = 0.0f;
    ns_c[c].on = 1;
    ns_c[c].np = 3 + (int)(sc_rnd() % 2u);
    ns_c[c].y = y;
    ns_c[c].v = (2.5f + 3.0f * sc_frand()) * (0.6f + 1.2f * ns.wind);
    for (j = 0; j < ns_c[c].np; j++) {
        ns_c[c].px[j] = (float)j * 13.0f * sc + 4.0f * sc_frand();
        ns_c[c].py[j] = (sc_frand() - 0.5f) * 5.0f;
        ns_c[c].rx[j] = (11.0f + 9.0f * sc_frand()) * sc;
        ns_c[c].ry[j] = (3.5f + 2.5f * sc_frand()) * sc * (j == 1 ? 1.3f : 1.0f);
        ext = fm_maxf(ext, ns_c[c].px[j] + ns_c[c].rx[j]);
    }
    ns_c[c].x = x < -1e8f ? -ext - 4.0f : x;          /* (from beyond the left edge, or where asked) */
}

static void ns_drone(void)                      /* Voices: the drone's source, a pad or one of GR!MM's choirs */
{
    int t = ns.kv[KN_VOICES];
    if (t <= 0) {
        fm_part_patch(2, ZN_SOFT_PAD);
    } else {
        fm_part_patch(2, FM_ENG_VOICE + t - 1);
        fm_part_attack(2, 2600);
        ns.vowel_sent = -100;
    }
    fm_part_release(2, 5000);
}

static void ns_fx(void)
{
    int t = ns.kv[KN_TAPE], e = ns.kv[KN_ECHO], c = ns.ch ? ns.ch->cloud : 0;
    fm_tape(t > 50 ? 100 : t * 2);
    fm_tape_knobs(t * 70 / 100, t * 60 / 100, 20 + t * 50 / 100, t * 40 / 100);
    fm_fx_send(e || c ? 70 : 0);
    fm_delay(128, 50 + e * 30 / 100, 70, e * 90 / 100);
    fm_grain(120, 36, 70, c);
}

static void ns_mix_levels(void)
{
    fm_part_level(1, ns.kv[KN_NOTES]);
    fm_part_level(3, ns.kv[KN_CHIME]);
    fm_part_level(4, ns.kv[KN_NOTES]);
    fm_part_level(5, ns.kv[KN_NOTES]);
    fm_part_level(6, ns.kv[KN_NOTES]);
    cd_drone_level(ns.kv[KN_DRONE]);
    fm_bed(FM_BED_WIND, ns.kv[KN_AIR]);
}

/* the mood whose events hold from now (a mood chosen; Long Night passing into the next): its range, its clocks */
static void ns_char(const ns_mood_t *m)
{
    if (m == ns.ch)
        return;
    ns.ch = m;
    cd_range(m->lo, m->n);
    ns.blink_next = fm_minf(ns.blink_next, 3.0f + 5.0f * sc_frand());
    ns.met_next = m->meteors ? fm_minf(ns.met_next, 3.0f + 6.0f * sc_frand()) : ns.met_next;
    ns.glint_next = m->glints ? fm_minf(ns.glint_next, 2.0f + 3.0f * sc_frand()) : ns.glint_next;
    ns.appear_next = m->appear ? fm_minf(ns.appear_next, 2.0f + 3.0f * sc_frand()) : ns.appear_next;
    ns.pulse_next = m->aurora ? fm_minf(ns.pulse_next, 4.0f + 6.0f * sc_frand()) : ns.pulse_next;
    ns.rook_next = m->rooks ? fm_minf(ns.rook_next, 8.0f + 20.0f * sc_frand()) : ns.rook_next;
    ns.owl_next = m->owl ? fm_minf(ns.owl_next, 40.0f + 60.0f * sc_frand()) : ns.owl_next;
    if (m->ecl && !ns_long() && !ns.ecl_force)       /* (the Eclipse mood: first contact in a few seconds) */
        ns.ecl_t = NS_ECL_S * (0.5f - (NS_ECL_RU + 1.0f) / (2.0f * (NS_ECL_RU + 1.2f))) - 6.0f;
    ns_fx();
}

/* ------------------------------------------------------------ picture --- */
static void ns_palette(void)
{
#define NS_RGB(v) ((uint32_t)(v)[0] << 16 | (uint32_t)(v)[1] << 8 | (uint32_t)(v)[2])
    uint32_t sky0 = NS_RGB(ns.sky0), sky1 = NS_RGB(ns.sky1), glow = NS_RGB(ns.glow), ground = NS_RGB(ns.ground), skym, skyl,
             black = 0x020306u, moonlit;
    float l = ns.light, ec = ns.ecl, bd = ns.band, tw = ns_moon_tw(),
          ml = ns.mn.light * ns.mn_vis * (1.0f - 0.9f * ec) * tw * (1.0f - ns.veil * 0.8f),   /* the moon's light on the world */
          n;
    int i;
#undef NS_RGB
    for (i = 0; i < NS_NSKY; i++) {                   /* the sky: the zenith to the horizon; the afterglow low down at
                                                       * dusk; darker in totality */
        float u = (float)i * (1.0f / (float)(NS_NSKY - 1)), g = bd * fm_maxf(0.0f, u - 0.35f) * (1.0f / 0.65f);
        uint32_t c = r8_mix(r8_mix(sky0, sky1, u), glow, 0.95f * g * fm_sqrtf(g));
        r8_pal_set(NS_SKY + i, r8_mix(c, black, 0.25f * ec));
    }
    skym = r8_base[NS_SKY + 20];
    skyl = r8_base[NS_SKY + 38];
    for (i = 0; i < 3; i++)                           /* the Milky Way: the high sky a little lighter, cool; its dust
                                                       * of faint stars */
        r8_pal_set(NS_MW + i, r8_mix(r8_base[NS_SKY + 12], 0x7A82A0u, ns.mw * (0.1f + 0.1f * (float)i)));
    r8_pal_set(NS_MW + 3, r8_mix(r8_base[NS_SKY + 12], 0xD0D8F0u, ns.mw * 0.5f));
    {   /* the stars: out with the dark (dimmer in the afterglow, brighter in totality); each its own steady
         * brightness; a few tinted */
        static const float B[6] = {0.42f, 0.52f, 0.62f, 0.72f, 0.86f, 1.0f};
        n = fm_clampf((0.55f + 0.45f * (1.0f - bd)) * (1.0f - 0.45f * bd) + 0.15f * ec, 0.0f, 1.0f);
        for (i = 0; i < 6; i++)
            r8_pal_set(NS_STW + i, r8_mix(skym, 0xF0F2FFu, B[i] * n));
        r8_pal_set(NS_STB, r8_mix(skym, 0xD2E2FFu, n));
        r8_pal_set(NS_STB + 1, r8_mix(skym, 0xD2E2FFu, 0.26f * n));
        r8_pal_set(NS_STY, r8_mix(skym, 0xFFE4B8u, n));
        r8_pal_set(NS_STY + 1, r8_mix(skym, 0xFFE4B8u, 0.26f * n));
        r8_pal_set(NS_STH, r8_mix(skym, 0xF0F2FFu, 0.26f * n));
    }
    for (i = 0; i < 4; i++)                           /* the moon's halo: the sky, lighter, with how much is lit; gone
                                                       * in the umbra, and under cloud */
        r8_pal_set(NS_HALO + i, r8_mix(skym, 0xA8B4D0u, (0.03f + 0.045f * (float)(3 - i)) * (0.45f + 0.5f * ns.mn.light) *
                                                             (1.0f - 0.9f * ec) * tw * (1.0f - 0.7f * ns.veil)));
    r8_pal_set(NS_MOON, r8_mix(skym, 0xFBF6E2u, tw));
    r8_pal_set(NS_MOON + 1, r8_mix(skym, 0xDDD7C0u, tw));
    r8_pal_set(NS_MOON + 2, r8_mix(skym, 0xC6C0AAu, tw));
    r8_pal_set(NS_MOON + 3, r8_mix(skym, 0x8A96B4u, 0.22f * tw));
    r8_pal_set(NS_ECL, 0x3A0A06u);                    /* the eclipsed moon: the umbra's heart to its copper edge */
    r8_pal_set(NS_ECL + 1, 0x6A1A0Au);
    r8_pal_set(NS_ECL + 2, 0x9C3414u);
    r8_pal_set(NS_ECL + 3, 0xC8683Au);
    for (i = 0; i < 6; i++)                           /* the aurora: greens, faint to bright; its violet top */
        r8_pal_set(NS_AUR + i, r8_mix(skym, 0x48F0A0u, (0.12f + 0.15f * (float)i)));
    r8_pal_set(NS_AUR + 6, r8_mix(skym, 0x9A5AD0u, 0.4f));
    r8_pal_set(NS_AUR + 7, r8_mix(skym, 0xB070E0u, 0.6f));
    moonlit = r8_mix(0xC8CCDCu, glow, bd > 0.3f ? bd * 0.8f : 0.0f);
    for (i = 0; i < 6; i++) {                         /* the clouds by thickness: the low sky grayed; lit by the moon, or
                                                       * from below at dusk */
        uint32_t c = r8_mix(skyl, 0x5A5E70u, 0.18f + 0.13f * (float)i);
        r8_pal_set(NS_CLD + i, c);
        r8_pal_set(NS_CLDL + i, r8_mix(c, moonlit, (0.25f + 0.08f * (float)i) * (bd > 0.3f ? 1.0f : 0.4f + 0.6f * ml)));
    }
    r8_pal_set(NS_MET, 0xFFFFFFu);
    r8_pal_set(NS_MET + 1, r8_mix(skym, 0xE8EEFFu, 0.85f));
    r8_pal_set(NS_MET + 2, r8_mix(skym, 0xE0E8FFu, 0.45f));
    r8_pal_set(NS_MET + 3, r8_mix(skym, 0xC8D0E8u, 0.3f));
    r8_pal_set(NS_GLINT, 0xFFFFFFu);
    r8_pal_set(NS_GLINT + 1, r8_mix(skym, 0xFFFFFFu, 0.6f));
    r8_pal_set(NS_GLINT + 2, r8_mix(skym, 0xFFFFFFu, 0.3f));
    r8_pal_set(NS_BIRD, r8_mix(r8_base[NS_SKY + 34], 0x000000u, 0.8f));
    for (i = 0; i < 8; i++)                           /* the ground, far to near: heather, nearly black, the moon's cool
                                                       * light on it; frost whitens it */
        r8_pal_set(NS_GROUND + i, r8_mix(r8_mix(r8_mix(0x000000u, ground, (0.55f + 0.06f * (float)i) * l), 0x3A4458u, 0.3f * ml),
                                         0x9AA8BCu, 0.12f * ns.frost));
    r8_pal_set(NS_RIM, r8_mix(r8_mix(r8_base[NS_GROUND + 7], glow, 0.4f * bd), 0x6A7490u, 0.45f * ml + 0.2f * ns.frost));
    r8_pal_set(NS_RIM + 1, r8_mix(r8_base[NS_GROUND + 7], r8_base[NS_RIM], 0.5f));
    {
        uint32_t st = r8_mix(0x000000u, ground, 0.3f * l), lit = r8_mix(glow, 0x8090B0u, 1.0f - bd);
        r8_pal_set(NS_STONE, st);
        r8_pal_set(NS_STONE + 1, r8_mix(st, lit, 0.35f * fm_minf(1.0f, bd + ml)));
        r8_pal_set(NS_STONE + 2, r8_mix(st, 0x000000u, 0.2f));
        r8_pal_set(NS_STONE + 3, r8_mix(r8_mix(st, lit, 0.2f * fm_minf(1.0f, bd + ml)), 0xC0D0E0u, 0.55f * ns.frost));
    }
    r8_pal_set(NS_PINE, r8_mix(0x000000u, ground, 0.28f * l));
    r8_pal_set(NS_PINE + 1, r8_mix(0x000000u, ground, 0.36f * l));
    r8_pal_set(NS_FROST, r8_mix(r8_base[NS_GROUND + 5], 0xE0F0FFu, 0.75f));
    r8_pal_set(NS_FROST + 1, r8_mix(r8_base[NS_GROUND + 5], 0xE0F0FFu, 0.4f));
    r8_pal_commit(0xFFFFFFu, 0xFFFFFFu, 0);
}

/* the stars: each at its own brightness (times how far out it is), fewer low in the haze, outshone near the moon; the
 * cold night's twinkle; the brightest with a soft halo; a flaring one a cross */
static void ns_stars_draw(surf_t *s, float k)
{
    float mx = ns_moon_x(), my = ns_moon_y(),
          glare = (ns.mn.r * (1.4f + 2.0f * ns.mn.light) + 10.0f) * ns.mn_vis * (1.0f - 0.8f * ns.ecl) * (1.0f - ns.veil);
    int i, slot = (int)(ns.t * 9.0f);
    for (i = 0; i < NS_NSTAR; i++) {
        float x = (float)ns_st[i].x, y = (float)ns_st[i].y, dx = x - mx, dy = y - my, d2 = dx * dx + dy * dy, lv, fl = ns_st[i].flare;
        int32_t px = (int32_t)(x * k), py = (int32_t)(y * k), l;
        uint8_t *p;
        if (ns_st[i].vis < 0.05f)
            continue;
        lv = (float)ns_st[i].lv * ns_st[i].vis + 0.5f - 2.0f * fm_clampf((y - 0.6f * NS_HOR) / (0.35f * NS_HOR), 0.0f, 1.0f);
        if (ns.twinkle > 0.02f)
            lv += ns.twinkle * ((float)((r8_mix32((uint32_t)i * 2654435761u, (uint32_t)slot) >> 8) & 7u) - 3.5f) * 0.32f;
        lv += 2.5f * fl;
        if (d2 < glare * glare * 2.5f)
            lv -= d2 < glare * glare ? 3.0f : 1.5f;
        l = (int32_t)lv;
        if (l < 0 || px < 0 || py < 0 || px >= s->w || py >= s->h)
            continue;
        p = s->px + py * s->w + px;
        if (*p >= NS_HALO)
            continue;                                 /* (only over the sky, the Milky Way, another star) */
        *p = (uint8_t)(l >= 4 && ns_st[i].tint ? (ns_st[i].tint == 1 ? NS_STB : NS_STY) : NS_STW + (l > 5 ? 5 : l));
        if (l >= 4 || fl > 0.3f) {                    /* a soft halo round the brightest; a flare's cross */
            static const int8_t NB[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
            int j;
            uint8_t h = fl > 0.3f ? (uint8_t)(NS_GLINT + (fl > 0.75f ? 0 : fl > 0.5f ? 1 : 2))
                                  : (uint8_t)(ns_st[i].tint == 1 ? NS_STB + 1 : ns_st[i].tint == 2 ? NS_STY + 1 : NS_STH);
            for (j = 0; j < 4; j++)
                ns_px(s, px + NB[j][0], py + NB[j][1], h, NS_STW);
            if (fl > 0.6f)
                *p = NS_GLINT;
        }
    }
}

/* the eclipse: the earth's shadow across the moon's face, from its eastern limb; inside the umbra the copper, its
 * heart darkest; a soft penumbral fringe */
static void ns_eclipse_draw(surf_t *s, float k)
{
    float r = ns.mn.r * k, mx = ns_moon_x() * k, my = ns_moon_y() * k, ad = fm_fabsf(ns.ecl_d),
          ux = mx + ns.ecl_d * r * 0.94f, uy = my + ns.ecl_d * r * 0.3f, ru = NS_ECL_RU * r, fr = 0.35f * r;
    int32_t x, y, x0 = (int32_t)(mx - r - 1.0f), x1 = (int32_t)(mx + r + 1.0f), y0 = (int32_t)(my - r - 1.0f), y1 = (int32_t)(my + r + 1.0f);
    if (!ns.ecl_on || ad > NS_ECL_RU + 1.4f || my > 250.0f)
        return;
    for (y = y0 < 0 ? 0 : y0; y <= y1 && y < s->h; y++)
        for (x = x0 < 0 ? 0 : x0; x <= x1 && x < s->w; x++) {
            uint8_t *p = s->px + y * s->w + x;
            float dx = (float)x + 0.5f - ux, dy = (float)y + 0.5f - uy, e = fm_sqrtf(dx * dx + dy * dy), sh;
            int lv;
            if (*p < NS_MOON || *p >= NS_MOON + 4)
                continue;
            if (e >= ru + fr)
                continue;
            if (e >= ru) {                            /* the fringe: dithered into the lit face */
                if (r8_dith(x, y, (int)((e - ru) / fr * 16.0f)))
                    continue;
                *p = NS_ECL + 3;
                continue;
            }
            sh = e / ru;                              /* 0 the heart .. 1 the edge */
            lv = (int)(sh * 3.0f * 16.0f);
            lv = (lv >> 4) + r8_dith(x, y, lv & 15);
            *p = (uint8_t)(NS_ECL + (lv > 3 ? 3 : lv));
        }
}

/* the aurora: curtains of light standing over the ridge, their rays wandering, brightest along their lower edge,
 * violet at the top when strong; only over the sky and the stars */
static void ns_aurora_draw(surf_t *s, float k)
{
    float a0 = ns.aur * (0.55f + 0.45f * ns.aur_pulse), ik = 1.0f / k, t = ns.t;
    int32_t x, y;
    if (a0 < 0.03f)
        return;
    for (x = 0; x < s->w; x++) {
        float wx = ((float)x + 0.5f) * ik,
              base = 96.0f + 12.0f * fm_sin_turns(wx / 230.0f + 0.011f * t) + 8.0f * ns_n1(wx * 0.02f + 0.25f * t, 11),
              ray = ns_n1(wx * 0.26f + 1.1f * t, 13) * (0.35f + 0.65f * ns_n1(wx * 0.05f - 0.2f * t, 14)), h, b;
        int32_t ya, yb;
        ray = ray * ray * ray;
        h = 22.0f + 46.0f * ray + 10.0f * ns_n1(wx * 0.015f - 0.08f * t, 12);
        b = fm_minf(1.0f, a0 * (0.12f + 1.1f * ray) * (1.0f + 0.5f * ns.aur_pulse));
        if (b < 0.04f)
            continue;
        ya = (int32_t)((base - h) * k);
        yb = (int32_t)(base * k);
        for (y = ya < 0 ? 0 : ya; y <= yb && y < s->h; y++) {
            float v = (((float)y + 0.5f) * ik - (base - h)) / h, I;
            uint8_t *p = s->px + y * s->w + x;
            int tt, lv;
            if (v < 0.0f || v > 1.0f || *p >= NS_HALO)
                continue;
            I = b * fm_minf(1.0f, v * 1.4f) * (v < 0.88f ? 1.0f : (1.0f - v) * (1.0f / 0.12f));
            tt = (int)(I * 6.0f * 16.0f);
            lv = (tt >> 4) + r8_dith(x, y, tt & 15);
            if (lv <= 0)
                continue;
            lv = lv > 6 ? 6 : lv;
            *p = (uint8_t)(v < 0.3f && lv >= 3 ? NS_AUR + 6 + (lv > 4) : NS_AUR + lv - 1);
        }
    }
}

/* a cloud: soft puffs, each a dithered ellipse; by its thickness one of six shades, lit near the moon (and from below at
 * dusk); across the moon the lit cloud hides it where it is thick, and what it covers is counted (the veil) */
static void ns_cloud_draw(surf_t *s, float k, int c, float mx, float my, float mr, int *covered)
{
    int j;
    float lit_r2 = (mr * 2.6f + 4.0f * k) * (mr * 2.6f + 4.0f * k);
    for (j = 0; j < ns_c[c].np; j++) {
        float cx = (ns_c[c].x + ns_c[c].px[j]) * k, cy = (ns_c[c].y + ns_c[c].py[j]) * k, rx = ns_c[c].rx[j] * k, ry = ns_c[c].ry[j] * k;
        int32_t x, y, x0 = (int32_t)(cx - rx), x1 = (int32_t)(cx + rx), y0 = (int32_t)(cy - ry), y1 = (int32_t)(cy + ry);
        for (y = y0 < 0 ? 0 : y0; y <= y1 && y < s->h; y++) {
            float dy = ((float)y + 0.5f - cy) / ry, vy = 1.0f - dy * dy;
            uint8_t *row = s->px + y * s->w;
            if (vy <= 0.0f)
                continue;
            for (x = x0 < 0 ? 0 : x0; x <= x1 && x < s->w; x++) {
                float dx = ((float)x + 0.5f - cx) / rx, a = vy - dx * dx, v, ex, ey;
                uint8_t *p = row + x;
                int lv, lit;
                if (a <= 0.0f || *p >= NS_GROUND)
                    continue;
                v = a * a * (0.8f + 0.3f * ns_n1((float)x * 0.3f / k + (float)y * 0.17f / k, 31)) * 6.4f;
                lv = (int)v;
                lv += r8_dith(x, y, (int)((v - (float)lv) * 16.0f));
                if (lv <= 0)
                    continue;
                lv = lv > 6 ? 6 : lv;
                if (*p >= NS_CLD && *p < NS_CLDL + 6) {   /* a cloud already: the thicker */
                    int old = (*p - NS_CLD) % 6 + 1;
                    if (lv <= old)
                        continue;
                }
                if ((*p >= NS_MOON && *p < NS_MOON + 4) || (*p >= NS_ECL && *p < NS_ECL + 4)) {   /* across the moon */
                    if (lv < 3)
                        continue;                     /* (thin: the moon shows through) */
                    (*covered)++;
                    *p = (uint8_t)(NS_CLDL + lv - 1);
                    continue;
                }
                ex = (float)x + 0.5f - mx;
                ey = (float)y + 0.5f - my;
                lit = (ns.mn_vis > 0.05f && ex * ex + ey * ey < lit_r2) || (ns.band > 0.3f && dy > 0.1f);
                *p = (uint8_t)((lit ? NS_CLDL : NS_CLD) + lv - 1);
            }
        }
    }
}

static void ns_draw(void)
{
    surf_t *s = &r8_fb;
    float k = ns_scale(), mx = ns_moon_x(), my = ns_moon_y(), mr = ns.mn.r * k;
    int i, j, covered = 0;
    r8_copy(s, &r8_bg);
    ns.mn_vis = sc_moon_draw(s, k, &ns.mn, mx * k, my * k, NS_GROUND, NS_MOON, NS_HALO, 4);
    ns_eclipse_draw(s, k);
    ns_stars_draw(s, k);
    ns_aurora_draw(s, k);
    for (i = 0; i < NS_NMET; i++) {                   /* the meteors: a bright head and a fading tail; a fireball's train */
        if (ns_m[i].train > 0.0f) {
            float dx = ns_m[i].x - ns_m[i].x0, dy = ns_m[i].y - ns_m[i].y0, l = fm_maxf(fm_fabsf(dx), fm_fabsf(dy));
            int n = (int)l + 1, d = (int)(ns_m[i].train * (16.0f / 3.5f));
            n = n > 120 ? 120 : n;
            for (j = 0; j <= n; j++) {
                float u = (float)j / (float)n, wob = 0.8f * fm_sin_turns(u * 3.0f + ns.t * 0.2f);
                int32_t px = (int32_t)((ns_m[i].x0 + dx * u) * k), py = (int32_t)((ns_m[i].y0 + dy * u + wob) * k);
                if (r8_dith(px, py, d))
                    ns_px(s, px, py, NS_MET + 3, NS_HALO);
            }
        }
        if (ns_m[i].on) {
            float u = ns_m[i].t / ns_m[i].life, sp = fm_sqrtf(ns_m[i].vx * ns_m[i].vx + ns_m[i].vy * ns_m[i].vy) + 1e-3f,
                  tx = ns_m[i].vx / sp, ty = ns_m[i].vy / sp;
            int n = ns_m[i].big ? 22 : 12, q = n / 4;
            for (j = 0; j < n; j++) {
                float b = (float)j * 2.0f;
                int c = j / q + (u > 0.65f ? 1 : 0);
                if (c > 3)
                    continue;
                ns_px(s, (int32_t)((ns_m[i].x - tx * b) * k), (int32_t)((ns_m[i].y - ty * b) * k), (uint8_t)(NS_MET + c), NS_MOON);
            }
        }
    }
    if (ns.sk.on)                                     /* the rooks: a skein in a loose V, wings beating */
        for (i = 0; i < NS_NBIRD; i++) {
            float back = (float)((i + 1) / 2) * 4.5f, side = (float)((i + 1) / 2) * 2.2f * (i & 1 ? 1.0f : -1.0f),
                  bx = ns.sk.x - ns.sk.dir * back, by = ns.sk.y + side + 1.2f * fm_sin_turns(ns.t * 0.4f + (float)i * 0.1f);
            int flap = ((int)(ns.t * 7.0f) + i) & 1;
            ns_px(s, (int32_t)(bx * k), (int32_t)(by * k), NS_BIRD, NS_GROUND);
            ns_px(s, (int32_t)((bx + (flap ? 1.0f : -1.0f)) * k), (int32_t)((by - (flap ? 0.0f : 1.0f)) * k), NS_BIRD, NS_GROUND);
        }
    for (i = 0; i < NS_NCLOUD; i++)
        if (ns_c[i].on)
            ns_cloud_draw(s, k, i, mx * k, my * k, mr, &covered);
    {   /* how much of the moon a cloud hides this frame */
        float disc = 3.1416f * mr * mr * ns.mn_vis;
        ns.mn_cov = disc > 4.0f ? fm_clampf((float)covered / disc, 0.0f, 1.0f) : 0.0f;
    }
    if (ns.frost > 0.05f) {                           /* frost glittering on the ground: points that catch the light in
                                                       * turn */
        int slot = (int)(ns.t * 3.0f);
        for (i = 0; i < 48; i++) {
            uint32_t h = r8_mix32((uint32_t)i * 2246822519u, ns.seed ^ 0x3F1u);
            float fx = (float)(h % 2400u) * 0.1f, fy;
            if ((int)((h >> 12) % 7u) != (slot + i) % 7 || (float)((h >> 20) & 15u) > ns.frost * 16.0f)
                continue;
            fy = ns_ridge_y(fx) + 2.0f + (float)((h >> 24) & 255u) * (1.0f / 255.0f) * (238.0f - ns_ridge_y(fx));
            {
                int32_t px = (int32_t)(fx * k), py = (int32_t)(fy * k);
                if (px >= 0 && py >= 0 && px < s->w && py < s->h && s->px[py * s->w + px] >= NS_GROUND && s->px[py * s->w + px] < NS_RIM)
                    s->px[py * s->w + px] = (uint8_t)(NS_FROST + ((h >> 9) & 1u));
            }
        }
    }
}

/* -------------------------------------------------------------- world --- */
/* this moor's own shape: the brow and its stones, the pines, the Milky Way's line, the shower's radiant, the stars,
 * the night's own moon */
SC_SMALL static void ns_layout(void)
{
    int i, n;
    float cx, cy, rx, ry;
    ns.lay.bx = 58.0f + 40.0f * (float)ns_h(1, 1) * (1.0f / 255.0f);
    ns.lay.bw = 55.0f + 25.0f * (float)ns_h(1, 2) * (1.0f / 255.0f);
    ns.lay.bh = 10.0f + 7.0f * (float)ns_h(1, 3) * (1.0f / 255.0f);
    n = ns.lay.nst = 7 + (int)(ns_h(1, 4) & 1u);
    cx = ns.lay.bx + 6.0f - 12.0f * (float)ns_h(1, 5) * (1.0f / 255.0f);
    cy = ns_ridge_y(cx) + 2.0f;
    rx = 26.0f + 10.0f * (float)ns_h(1, 6) * (1.0f / 255.0f);
    ry = 4.5f + 2.0f * (float)ns_h(1, 7) * (1.0f / 255.0f);
    for (i = 0; i < n; i++) {                         /* the ring, seen from a little below its level: the near stones
                                                       * lower and larger */
        float a = ((float)i + 0.15f + 0.7f * (float)ns_h(i, 8) * (1.0f / 255.0f)) / (float)n, sn = fm_sin_turns(a), cs = fm_sin_turns(a + 0.25f),
              near = 1.0f + 0.3f * sn;
        ns.lay.sx[i] = cx + rx * cs;
        ns.lay.sy[i] = cy + ry * sn;
        ns.lay.sh[i] = (10.0f + 8.0f * (float)ns_h(i, 9) * (1.0f / 255.0f)) * near;
        ns.lay.sw[i] = (2.8f + 1.8f * (float)ns_h(i, 10) * (1.0f / 255.0f)) * near;
        if (ns_h(i, 11) < 36u) {                      /* (a fallen one) */
            ns.lay.sh[i] = 2.2f * near;
            ns.lay.sw[i] = 7.0f * near;
        }
    }
    {   /* the lintel: over the nearest pair of standing stones, side by side in the ring */
        int best = -1;
        float by = -1e9f;
        for (i = 0; i < n; i++) {
            int o = (i + 1) % n;
            float y2 = ns.lay.sy[i] + ns.lay.sy[o];
            if (ns.lay.sh[i] > 5.0f && ns.lay.sh[o] > 5.0f && y2 > by && fm_fabsf(ns.lay.sx[i] - ns.lay.sx[o]) < 16.0f) {
                by = y2;
                best = i;
            }
        }
        ns.lay.lintel = (int8_t)best;
        if (best >= 0) {                              /* (the pair the same height) */
            int o = (best + 1) % n;
            float top = fm_minf(ns.lay.sy[best] - ns.lay.sh[best], ns.lay.sy[o] - ns.lay.sh[o]);
            ns.lay.sh[best] = ns.lay.sy[best] - top;
            ns.lay.sh[o] = ns.lay.sy[o] - top;
        }
    }
    ns.lay.mwc = 66.0f + 30.0f * (float)ns_h(2, 12) * (1.0f / 255.0f);
    ns.lay.mws = (ns_h(2, 13) & 1u ? 1.0f : -1.0f) * (0.2f + 0.25f * (float)ns_h(2, 14) * (1.0f / 255.0f));
    ns.lay.rx = 70.0f + 100.0f * (float)ns_h(2, 15) * (1.0f / 255.0f);
    ns.lay.ry = 18.0f + 30.0f * (float)ns_h(2, 16) * (1.0f / 255.0f);
    {   /* the stars: scattered, with small clumps and gaps, thinning toward the horizon; most faint, about ten
         * medium, a few bright, those faintly blue-white or warm; the brightest come out first */
        float cxs[4], cys[4];
        for (i = 0; i < 4; i++) {
            cxs[i] = 20.0f + 200.0f * sc_frand();
            cys[i] = 10.0f + 90.0f * sc_frand();
        }
        for (i = 0; i < NS_NSTAR; i++) {
            float sx = 120.0f, sy = 40.0f, u;
            int tries;
            for (tries = 0; tries < 8; tries++) {
                if (i % 3 == 0) {
                    int c = (int)(sc_rnd() % 4u);
                    sx = cxs[c] + (sc_frand() + sc_frand() - 1.0f) * 24.0f;
                    sy = cys[c] + (sc_frand() + sc_frand() - 1.0f) * 18.0f;
                } else {
                    sx = 240.0f * sc_frand();
                    sy = 3.0f + (NS_HOR - 20.0f) * sc_frand();
                }
                if (sx >= 1.0f && sx < 239.0f && sy >= 3.0f && sy < ns_ridge_y(sx) - 8.0f && sc_frand() < 1.25f - sy / (NS_HOR - 12.0f))
                    break;
            }
            u = sc_frand();
            ns_st[i].x = (int16_t)fm_clampf(sx, 1.0f, 238.0f);
            ns_st[i].y = (int16_t)fm_clampf(sy, 3.0f, ns_ridge_y(sx) - 8.0f);
            ns_st[i].lv = (uint8_t)(i < 3 ? 5 : i < 6 ? 4 : i < 18 ? 2 + (u < 0.5f) : u < 0.5f ? 0 : 1);
            ns_st[i].tint = (uint8_t)(i == 1 ? 1 : i == 2 ? 2 : i == 4 && u < 0.5f ? 1 : 0);
            ns_st[i].appear = (uint8_t)(ns_st[i].lv >= 4 ? 20.0f * sc_frand() : (float)(5 - ns_st[i].lv) * 40.0f + 70.0f * sc_frand());
            ns_st[i].vis = 0.0f;
            ns_st[i].flare = 0.0f;
            ns_st[i].want = 0;
        }
    }
    {   /* the night's own moon (Long Night's): its size and phase, its arc's height */
        sc_moon_pick(&ns.mn, 1.15f);
        ns.nat_light = ns.mn.light;
        ns.nat_off = ns.mn.off;
        ns.nat_side = ns.mn.side;
        ns.moon_top = 120.0f + 32.0f * sc_frand();
        sc_mst_k = -1.0f;
    }
}

/* the look the mood asks for now (Long Night: between two of its night's), and what holds now (ns_char) */
static void ns_look(float dt, int at_once)
{
    const ns_mood_t *a = &NS_MOOD[ns_mood_now()], *b = a;
    float u = 0.0f, e = at_once ? 1.0f : fm_minf(1.0f, dt * 0.3f), v;
    if (a->scale < 0) {
        float f = ns.phase * 4.0f;
        int si = (int)f;
        si = si > 3 ? 3 : si;
        u = fm_clampf((f - (float)si - 0.7f) * (1.0f / 0.3f), 0.0f, 1.0f);
        u = u * u * (3.0f - 2.0f * u);
        a = &NS_MOOD[NS_LN[si]];
        b = &NS_MOOD[NS_LN[sc_long_next(si)]];
        cd_light(a->scale);                           /* (taken when the notes rest, within a minute) */
    }
    ns_char(u < 0.5f ? a : b);
#define NS_EASE(dst, ca, cb) do { \
        uint32_t m_ = r8_mix(ca, cb, u); \
        (dst)[0] += ((float)(m_ >> 16 & 255u) - (dst)[0]) * e; \
        (dst)[1] += ((float)(m_ >> 8 & 255u) - (dst)[1]) * e; \
        (dst)[2] += ((float)(m_ & 255u) - (dst)[2]) * e; \
    } while (0)
    NS_EASE(ns.sky0, a->sky0, b->sky0);
    NS_EASE(ns.sky1, a->sky1, b->sky1);
    NS_EASE(ns.glow, a->glow ? a->glow : b->glow, b->glow ? b->glow : a->glow);
    NS_EASE(ns.ground, a->ground, b->ground);
#undef NS_EASE
#define NS_EASE1(dst, fa, fb, sc) do { v = ((float)(fa) + ((float)(fb) - (float)(fa)) * u) * (sc); (dst) += (v - (dst)) * e; } while (0)
    NS_EASE1(ns.band, a->band, b->band, 0.01f);
    NS_EASE1(ns.twinkle, a->twinkle, b->twinkle, 0.01f);
    NS_EASE1(ns.mw, a->mw, b->mw, 0.01f);
    NS_EASE1(ns.aur, a->aurora, b->aurora, 0.01f);
    NS_EASE1(ns.frost, a->frost, b->frost, 0.01f);
    NS_EASE1(ns.wind, a->wind, b->wind, 0.01f);
    {   /* the stars out and the clouds: the knobs (a mood sets them); in Long Night its night's, scaled by the knobs
         * from their defaults */
        float sk = ns_long() ? (float)ns.kv[KN_STARS] * (1.0f / 50.0f) : 1.0f, ck = ns_long() ? (float)ns.kv[KN_CLOUDS] * (1.0f / 30.0f) : 1.0f;
        float st = ns_long() ? ((float)a->stars + ((float)b->stars - (float)a->stars) * u) * 0.01f * sk : (float)ns.kv[KN_STARS] * 0.01f,
              cl = ns_long() ? ((float)a->clouds + ((float)b->clouds - (float)a->clouds) * u) * 0.01f * ck : (float)ns.kv[KN_CLOUDS] * 0.01f;
        ns.starsamt += (fm_minf(1.0f, st) - ns.starsamt) * e;
        ns.cloudamt += (fm_minf(1.0f, cl) - ns.cloudamt) * e;
        ns.out_cap = (int)(255.0f * fm_minf(1.0f, st) + 0.5f);
    }
    ns.light = 1.0f - 0.35f * ns.ecl + 0.2f * ns.band;
#undef NS_EASE1
    {   /* the voice's vowel: the mood's, opened by the aurora, closed by the eclipse; its tone and breath */
        int vw = (int)((float)a->vowel + 35.0f * ns.aur * (0.5f + 0.5f * ns.aur_pulse) - 20.0f * ns.ecl + 5.0f * fm_sin_turns(ns.t * (1.0f / 53.0f)));
        vw = vw < 0 ? 0 : vw > 127 ? 127 : vw;
        if (ns.kv[KN_VOICES] > 0 && (vw - ns.vowel_sent >= 4 || ns.vowel_sent - vw >= 4)) {
            fm_part_knobs(2, vw, 12, a->buzz, a->breath);
            ns.vowel_sent = vw;
        }
    }
}

static void ns_mood_tune(void)
{
    const ns_mood_t *m = &NS_MOOD[ns_mood_now()];
    cd_tune(m->scale, (float)m->key);
}

SC_SMALL static void ns_defaults(int16_t *kv)
{
    int i;
    for (i = 0; i < SC_NKNOB; i++)
        kv[i] = NS_KNOB[i][3];
}

SC_SMALL static void ns_enter(uint32_t seed, int16_t *kv)
{
    int i, m;
    memset(&ns, 0, sizeof ns);
    memset(ns_m, 0, sizeof ns_m);
    memset(ns_c, 0, sizeof ns_c);
    memset(ns_q, 0, sizeof ns_q);
    ns.kv = kv;
    for (i = 0; i < SC_NKNOB; i++)
        if (NS_KNOB[i][1] > NS_KNOB[i][0] && (kv[i] < NS_KNOB[i][0] || kv[i] > NS_KNOB[i][1]))
            kv[i] = NS_KNOB[i][3];
    ns.seed = seed | 1u;
    sc_seed(ns.seed * 2654435761u + 13u);
    ns_layout();
    m = ns_mood_now();
    ns.phase = sc_long_begin((float)(seed % 1000u) * 0.001f);   /* (Long Night: wherever the night is) */
    ns.night = ns_long() ? ns.phase : (float)NS_MOOD[m].night * 0.01f;
    sc_glide_at(&ns.glide, ns.night);
    ns_mood_moon(m, 0);
    ns.pal_lv = ns.air_sent = -1;
    ns.vowel_sent = -100;
    ns.blink_next = 4.0f + 6.0f * sc_frand();
    ns.appear_next = 3.0f + 4.0f * sc_frand();
    ns.glint_next = 2.0f + 3.0f * sc_frand();
    ns.met_next = 8.0f + 12.0f * sc_frand();
    ns.rook_next = 10.0f + 30.0f * sc_frand();
    ns.owl_next = 60.0f + 90.0f * sc_frand();
    ns.pulse_next = 6.0f + 8.0f * sc_frand();
    ns.cloud_next = 2.0f + 6.0f * sc_frand();
    ns.ecl_t = 0.0f;
    r8_dark(NS_DARK);
    cd_init(seed);
    cd_activity((float)kv[KN_ACT] * 0.01f);
    cd_echo_amount(0.0f);                             /* (the echo is the tape echo: ns_fx) */
    cd_change_mode(1);
    ns_mood_tune();
    fm_part_patch(1, NS_TONE_SLOT[kv[KN_TONE] % NS_NTONE]);
    fm_part_release(1, 2800);
    ns_drone();
    fm_part_patch(3, ZN_FURIN);                       /* the chimes: meteors, glints */
    fm_part_release(3, 2600);
    fm_part_patch(4, ZN_SOFT_SWELL);                  /* the swells: the aurora, the veil, a fireball */
    fm_part_release(4, 4000);
    fm_part_patch(5, FM_ENG_PHYS + 0);                /* the gong: MODAL, inharmonic, ringing on */
    fm_part_knobs(5, 78, 112, 0, 34);
    fm_part_release(5, 6000);
    fm_part_patch(6, FM_ENG_PHYS + 2);                /* the drum: MEMBRANE, loaded, its pitch falling a little */
    fm_part_knobs(6, 62, 58, 36, 22);
    fm_part_release(6, 1200);
    ns_look(0.0f, 1);
    {   /* the stars already out, as the mood has them (Dusk's come out one by one from here) */
        for (i = 0; i < NS_NSTAR; i++) {
            ns_st[i].want = (uint8_t)((int)ns_st[i].appear < ns.out_cap);
            if (ns.ch && ns.ch->appear && ns_st[i].appear >= 20)
                ns_st[i].want = 0;
            ns_st[i].vis = ns_st[i].want ? 1.0f : 0.0f;
        }
        for (i = 0; i < NS_NCLOUD; i++)               /* a cloud or two already in the sky */
            if (sc_frand() < ns.cloudamt)
                ns_cloud_new(i, 20.0f + 180.0f * sc_frand(), 18.0f + 110.0f * sc_frand());
    }
    ns_mix_levels();
    cd_drone(1);
    fm_bed(FM_BED_STREAM, 0);
    fm_bed(FM_BED_RAIN, 0);
    fm_bed_style(FM_BED_WIND, FM_AIR_BREEZE_SYNTH);   /* (the air: synthesized; the recordings hissed) */
    fm_bed(FM_BED_WIND, kv[KN_AIR]);
    cd_melody(0);
    cd_arc_amount((float)kv[KN_ARC] * 0.01f);
    cd_depth_send(kv[KN_DEPTH]);
}

SC_SMALL static void ns_leave(void)
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

static void ns_step(uint32_t dt_ms)
{
    float dt = (float)dt_ms * 0.001f, a = (float)ns.kv[KN_ACT] * 0.01f;
    int i;
    ns.t += dt;
    ns.ms += dt_ms;
    if (ns_long()) {                                  /* Long Night: the night goes on; at dawn, dusk again with a
                                                       * new moon of its own */
        if (sc_long_step(&ns.phase, dt)) {
            ns.nights++;
            cd_dawn();
            sc_moon_repick(&ns.mn, 1.15f, ns.seed ^ (ns.nights * 0x9E3779B1u));
            ns.nat_light = ns.mn.light;
            ns.nat_off = ns.mn.off;
            ns.nat_side = ns.mn.side;
            ns.moon_top = 120.0f + 32.0f * sc_frand();
            ns.mg.t = 99.0f;
            sc_mst_k = -1.0f;
        }
        ns.night = ns.phase;
        sc_glide_at(&ns.glide, ns.night);
    } else
        ns.night = sc_glide(&ns.glide, ns.night, (float)NS_MOOD[ns_mood_now()].night * 0.01f, dt);
    ns_moon_glide(dt);
    ns_look(dt, 0);
    /* the eclipse: in its mood on its cycle; in Long Night at the midnight when the moon is full; from a key once */
    {
        int on = 0;
        float et = -1.0f, want = 0.0f;
        if (ns.ecl_force) {
            ns.ecl_t += dt;
            et = ns.ecl_t;
            on = 1;
            if (et > NS_ECL_S) {
                ns.ecl_force = 0;
                on = 0;
            }
        } else if (ns_long()) {
            if (ns.phase >= 0.5f && ns.phase < 0.75f && ns.mn.off < -0.99f) {
                on = 1;
                et = (ns.phase - 0.625f) * sc_long_s + NS_ECL_S * 0.5f;
            }
        } else if (ns.ch && ns.ch->ecl) {
            ns.ecl_t += dt;
            if (ns.ecl_t >= NS_ECL_S)
                ns.ecl_t -= NS_ECL_S;
            et = ns.ecl_t;
            on = 1;
        }
        ns.ecl_on = on && et >= 0.0f && et <= NS_ECL_S;
        if (ns.ecl_on) {
            float ad;
            int ph;
            ns.ecl_d = (et / NS_ECL_S * 2.0f - 1.0f) * (NS_ECL_RU + 1.2f);
            ad = fm_fabsf(ns.ecl_d);
            ph = ad > NS_ECL_RU + 1.0f ? 0 : ad > NS_ECL_RU - 1.0f ? 1 : 2;
            want = fm_clampf((NS_ECL_RU + 1.0f - ad) * 0.5f, 0.0f, 1.0f);
            if (ph != ns.ecl_ph) {                    /* a contact: the gong */
                ns_gong(ph == 2 || ns.ecl_ph == 2 ? 1.0f : 0.7f);
                ns.ecl_ph = ph;
                ns.ecl_mid = 0;
                ns.drum_next = 1.0f;
            }
            if (ph == 2 && !ns.ecl_mid && ad < 0.1f) {   /* mid-totality: once, softer */
                ns.ecl_mid = 1;
                ns_gong(0.5f);
            }
            if (ph == 1 && (ns.ch->drum || ns.ecl_force) && (ns.drum_next -= dt) <= 0.0f) {
                ns.drum_next = (4.2f - 2.2f * want) * (0.85f + 0.3f * sc_frand());
                ns_drum(0.35f + 0.55f * want);
            }
        } else
            ns.ecl_ph = 0;
        ns.ecl += (want - ns.ecl) * fm_minf(1.0f, dt * 2.0f);
    }
    {   /* the air: the mood's wind, breathing a little */
        int m = (int)(6.0f + 60.0f * ns.wind * (0.8f + 0.2f * fm_sin_turns(ns.t * 0.05f)));
        if (m / 4 != ns.air_sent / 4) {
            fm_bed_mod(FM_BED_WIND, m);
            ns.air_sent = m;
        }
    }
    /* the stars: each eases out or back; a flare fades; now and then one blinks (a note); at dusk they come out one by
     * one; on a cold night they glint */
    for (i = 0; i < NS_NSTAR; i++) {
        if (!ns.ch->appear)
            ns_st[i].want = (uint8_t)((int)ns_st[i].appear < ns.out_cap);
        ns_st[i].vis += ((ns_st[i].want ? 1.0f : 0.0f) - ns_st[i].vis) * fm_minf(1.0f, dt * 0.4f);
        if (ns_st[i].flare > 0.0f)
            ns_st[i].flare -= dt * 0.7f;
    }
    if ((ns.blink_next -= dt) <= 0.0f) {
        ns.blink_next = (6.0f + 6.0f * sc_frand()) / (0.5f + a);
        ns_blink();
    }
    if (ns.ch->appear && (ns.appear_next -= dt) <= 0.0f) {
        ns.appear_next = (float)ns.ch->appear * (0.6f + 0.8f * sc_frand()) / (0.6f + a);
        ns_appear();
    }
    if (ns.ch->glints && (ns.glint_next -= dt) <= 0.0f) {
        ns.glint_next = (float)ns.ch->glints * (0.5f + sc_frand()) / (0.6f + a);
        ns_glint(1);
    }
    /* the meteors: on the mood's clock and the knob's; each on its way, then gone (a fireball's train lingers) */
    if (ns.ch->meteors && ns.kv[KN_METEORS] > 0 && (ns.met_next -= dt) <= 0.0f) {
        float rate = (ns_long() ? 1.0f : (float)ns.kv[KN_METEORS] * (1.0f / 30.0f)) * (0.5f + a);
        ns.met_next = (float)ns.ch->meteors * (0.6f + 0.8f * sc_frand()) / fm_maxf(0.2f, rate);
        ns_meteor(0, sc_frand() < 0.18f, 0.4f + 0.6f * sc_frand(), 0);
    }
    for (i = 0; i < NS_NMET; i++) {
        if (ns_m[i].train > 0.0f)
            ns_m[i].train -= dt;
        if (!ns_m[i].on)
            continue;
        ns_m[i].t += dt;
        ns_m[i].x += ns_m[i].vx * dt;
        ns_m[i].y += ns_m[i].vy * dt;
        if (ns_m[i].t >= ns_m[i].life || ns_m[i].y > ns_ridge_y(ns_m[i].x) - 3.0f || ns_m[i].x < -10.0f || ns_m[i].x > 250.0f) {
            ns_m[i].on = 0;
            if (ns_m[i].big)
                ns_m[i].train = 3.5f;
        }
    }
    /* the aurora's pulses; the veil of cloud over the moon: a swell when it closes, a note when it clears */
    if (ns.aur_pulse > 0.0f)
        ns.aur_pulse -= dt * 0.25f;
    if (ns.ch->aurora && (ns.pulse_next -= dt) <= 0.0f) {
        ns.pulse_next = (9.0f + 12.0f * sc_frand()) / (0.6f + a);
        ns_pulse(sc_frand());
    }
    ns.veil += (ns.mn_cov - ns.veil) * fm_minf(1.0f, dt * 1.5f);
    if (!ns.veiled && ns.veil > 0.5f) {
        ns.veiled = 1;
        ns.veils++;
        if (!cd_quiet())
            cd_swell(fm_clampf(ns_moon_x() * (1.0f / 240.0f), 0.0f, 1.0f), 0.3f, 4000);
    } else if (ns.veiled && ns.veil < 0.15f) {
        ns.veiled = 0;
        if (!cd_quiet())
            cd_pluck(fm_clampf(ns_moon_x() * (1.0f / 240.0f), 0.0f, 1.0f), 0.45f, 1800);
    }
    /* the clouds: new ones in from the west on the mood's amount (at the midnight without an eclipse, toward the
     * moon), drifting with the wind, gone past the east */
    if (ns.cloudamt > 0.02f && (ns.cloud_next -= dt) <= 0.0f) {
        ns.cloud_next = (14.0f / (0.1f + ns.cloudamt)) * (0.6f + 0.8f * sc_frand()) / (0.6f + 0.8f * ns.wind);
        for (i = 0; i < NS_NCLOUD; i++)
            if (!ns_c[i].on) {
                float my = ns_moon_y(), y = 18.0f + 110.0f * sc_frand();
                if (my < 170.0f && ns.ch->ecl && !ns.ecl_on && sc_frand() < 0.6f)
                    y = my - 3.0f + 6.0f * sc_frand();
                ns_cloud_new(i, -1e9f, y);
                break;
            }
    }
    for (i = 0; i < NS_NCLOUD; i++)
        if (ns_c[i].on) {
            ns_c[i].x += ns_c[i].v * (0.6f + 1.2f * ns.wind) * dt;
            if (ns_c[i].x > 262.0f)
                ns_c[i].on = 0;
        }
    /* the rooks going home, and their caws; the owl in the deep of the night */
    if (ns.ch->rooks && (ns.rook_next -= dt) <= 0.0f && !ns.sk.on) {
        ns.rook_next = (float)ns.ch->rooks * (0.6f + 0.8f * sc_frand());
        ns_skein();
    }
    if (ns.sk.on) {
        ns.sk.x += ns.sk.dir * 24.0f * dt;
        if ((ns.sk.caw -= dt) <= 0.0f) {
            ns.sk.caw = 5.0f + 9.0f * sc_frand();
            if (ns.sk.x > -10.0f && ns.sk.x < 250.0f)
                ns_caw(fm_clampf(ns.sk.x * (1.0f / 240.0f), 0.0f, 1.0f));
        }
        if (ns.sk.x > 290.0f || ns.sk.x < -50.0f)
            ns.sk.on = 0;
    }
    if (ns.ch->owl && (ns.owl_next -= dt) <= 0.0f) {
        ns.owl_next = (float)ns.ch->owl * (0.6f + 0.8f * sc_frand());
        ns_owl();
    }
    cd_step(dt_ms);
    for (i = 0; i < NS_NQ; i++)
        if (ns_q[i].at && (int32_t)(ns.ms - ns_q[i].at) >= 0) {
            if (ns_q[i].id == 0)                      /* (a swell: its place in cents, its strength in vel) */
                cd_swell(ns_q[i].cents, (float)ns_q[i].vel * 0.01f, ns_q[i].dur);
            else if (ns_q[i].id == 1)                 /* (a meteor of a burst) */
                ns_meteor(0, 0, 0.6f, 1);
            else if (ns_q[i].id == 2)                 /* (a beat of the roll) */
                ns_drum((float)ns_q[i].vel * 0.01f);
            else
                fm_voice(ns_q[i].id, ns_q[i].part, ns_q[i].cents, ns_q[i].vel, ns_q[i].pan, ns_q[i].dur);
            ns_q[i].at = 0;
        }
    cd_depth_send(ns.kv[KN_DEPTH] + (int)(30.0f * ns.veil) + (int)(25.0f * ns.ecl));
    {   /* the palette, when the look has moved (at most four times a second: every color changes, so the whole
         * screen is sent) */
        int lv = (int)(ns.light * 24.0f) + ((int)ns.sky1[1] / 6) * 32 + ((int)(ns.band * 12.0f) + (int)(ns.aur * 10.0f) * 16 +
                 (int)(ns.ecl * 16.0f) * 256 + (int)(ns.mw * 8.0f) * 4096) * 2048 + ((int)ns.sky0[2] / 8) * 65536 * 32 +
                 (int)(ns.veil * 6.0f) + (int)(ns.frost * 8.0f) * 8 + (int)(ns.mn_vis * 4.0f) * 64 + (int)(ns.aur_pulse * 4.0f) * 128,
            d = lv - ns.pal_lv;
        if (ns.pal_lv < 0 || !dt_ms || (lv != ns.pal_lv && (ns.ms - ns.pal_ms >= 250u || d > 2 || d < -2))) {
            ns.pal_lv = lv;
            ns.pal_ms = ns.ms;
            ns_palette();
        }
    }
}

SC_SMALL static void ns_key(int key, int on)
{
    int b, i;
    if (!on)
        return;
    for (b = 0; b < FM_NBLACK; b++)
        if (BLACK_K[b] == key) {
            if (b < 2) {                              /* a meteor, west and east */
                ns_meteor(b == 0 ? 1 : -1, 0, 0.8f, 1);
            } else if (b == 2) {                      /* a fireball */
                ns_meteor(0, 1, 1.0f, 1);
            } else if (b == 3) {                      /* a cloud across the moon */
                float my = ns_moon_y();
                for (i = 0; i < NS_NCLOUD; i++)
                    if (!ns_c[i].on) {
                        ns_cloud_new(i, -1e9f, my < 170.0f ? my - 2.0f + 4.0f * sc_frand() : 40.0f + 80.0f * sc_frand());
                        ns_c[i].x = ns_moon_x() - 50.0f;
                        ns_c[i].v = 7.0f;
                        break;
                    }
            } else if (b == 4) {                      /* the aurora pulses */
                ns.aur = fm_maxf(ns.aur, 0.6f);
                ns_pulse(1.0f);
            } else if (b == 5) {                      /* the gong */
                ns_gong(0.9f);
            } else if (b == 6) {                      /* the drum: a roll of three */
                ns_drum(0.8f);
                ns_q_add(420u, 2, 0, 0.0f, 60, 0, 0);
                ns_q_add(840u, 2, 0, 0.0f, 90, 0, 0);
            } else if (b == 7) {                      /* the eclipse begins (first contact at once) */
                ns.ecl_force = 1;
                ns.ecl_t = NS_ECL_S * (0.5f - (NS_ECL_RU + 1.0f) / (2.0f * (NS_ECL_RU + 1.2f))) - 1.0f;
            } else if (b == 8) {                      /* another moon: the phase turns, full to a crescent and round */
                float o = ns.mn.off, l = ns.mn.light, sd = ns.mn.side;
                if (o < -0.99f) {
                    o = -0.5f;
                    l = 0.8f;
                } else if (o < -0.4f) {
                    o = 0.0f;
                    l = 0.55f;
                } else if (o < 0.3f) {
                    o = 0.6f;
                    l = 0.3f;
                } else {
                    o = -1.0f;
                    l = 1.0f;
                    sd = -sd;
                }
                ns_moon_to(l, o, sd, 1);
                ns.nat_light = l;
                ns.nat_off = o;
                ns.nat_side = sd;
                ns.moons++;
            } else if (b == 9) {                      /* the owl */
                ns_owl();
            } else {                                  /* a burst of meteors */
                ns_meteor(0, 0, 0.8f, 1);
                for (i = 1; i < 5; i++)
                    ns_q_add((uint32_t)(i * 600 + (int)(sc_rnd() % 300u)), 1, 0, 0.0f, 0, 0, 0);
            }
            ns.key_note = b;
            return;
        }
    for (b = 0; b < FM_NWHITE; b++)
        if (WHITE_K[b] == key) {
            int k = b + 2;                            /* (sixteen notes, each its own, rising) */
            float x = 12.0f + 216.0f * (float)b / (float)(FM_NWHITE - 1), bd = 1e9f;
            int best = -1;
            cd_note_key(k, x * (1.0f / 240.0f), 0.5f + 0.25f * (1.0f - (float)b / (float)(FM_NWHITE - 1)), 1500, 1, 0u);
            ns.key_note = k;
            for (i = 0; i < NS_NSTAR; i++) {          /* the nearest star that is out flares, silent */
                float d = fm_fabsf((float)ns_st[i].x - x) + (ns_st[i].vis < 0.5f ? 1000.0f : 0.0f);
                if (d < bd) {
                    bd = d;
                    best = i;
                }
            }
            if (best >= 0)
                ns_st[best].flare = 1.0f;
            return;
        }
}

SC_SMALL static void ns_knob(int i, int32_t e)
{
    if (i == KN_MOOD) {                               /* a mood: every knob to the sound, its key, scale, look, moon */
        (void)e;
        ns_mood_tune();
        cd_activity((float)ns.kv[KN_ACT] * 0.01f);
        cd_arc_amount((float)ns.kv[KN_ARC] * 0.01f);
        fm_part_patch(1, NS_TONE_SLOT[ns.kv[KN_TONE] % NS_NTONE]);
        ns_drone();
        ns_mix_levels();
        ns.ch = 0;                                    /* (ns_look takes the new mood's events at the next step) */
        ns.ecl_force = 0;                             /* (a key's eclipse ends with the mood) */
        ns_look(0.0f, 0);
        ns_mood_moon(ns_mood_now(), 1);
        ns_fx();
        return;
    }
    if (i == KN_SPACE) {
        knob_set(P_REVERB, proj.par[P_REVERB] + (int)(e * sc_knob_mul));
        return;
    }
    if (i < 0 || i >= SC_NKNOB || NS_KNOB[i][1] <= NS_KNOB[i][0])
        return;
    if (i == KN_TONE || i == KN_VOICES)
        sc_knob_wrap(&ns.kv[i], e, NS_KNOB[i][0], NS_KNOB[i][1]);
    else
        sc_knob_turn(&ns.kv[i], e, NS_KNOB[i][0], NS_KNOB[i][1], NS_KNOB[i][2]);
    switch (i) {
    case KN_ACT: cd_activity((float)ns.kv[KN_ACT] * 0.01f); break;
    case KN_TONE: fm_part_patch(1, NS_TONE_SLOT[ns.kv[KN_TONE] % NS_NTONE]); break;
    case KN_VOICES: ns_drone(); break;
    case KN_ECHO: case KN_TAPE: ns_fx(); break;
    case KN_ARC: cd_arc_amount((float)ns.kv[KN_ARC] * 0.01f); break;
    case KN_NOTES: case KN_DRONE: case KN_AIR: case KN_CHIME: ns_mix_levels(); break;
    default: break;                                   /* (Meteors, Clouds, Stars, Warmth: read each step) */
    }
}

SC_SMALL static void ns_knob_text(int i, char *name, char *val)
{
    static const char *const N[SC_NUSED] = {"Mood", "Activity", "Instrument", "Space", "Notes", "Drone", "Air", "Chimes",
                                            "Voices", "Meteors", "Clouds", "Stars", "Echo", "Warmth", "Tape", "Tide"};
    if (i < 0 || i >= SC_NUSED || (i != KN_SPACE && NS_KNOB[i][1] <= NS_KNOB[i][0])) {
        name[0] = val[0] = 0;
        return;
    }
    copy_str(name, N[i], 11);
    switch (i) {
    case KN_MOOD: copy_str(val, NS_MOOD[ns_mood_now()].name, 11); break;
    case KN_TONE: copy_str(val, NS_TONE_NAME[ns.kv[KN_TONE] % NS_NTONE], 11); break;
    case KN_VOICES: copy_str(val, NS_CHOIR_NAME[ns.kv[KN_VOICES] % NS_NCHOIR], 11); break;
    case KN_SPACE: itoa_u((uint32_t)proj.par[P_REVERB], val); break;
    default: itoa_u((uint32_t)ns.kv[i], val); break;
    }
}

/* a new moor (PLAY; the glue fades through dark): another brow and ring of stones, other stars, the Milky Way
 * elsewhere, another moon of its own; the night goes on */
SC_SMALL static void ns_regrow(uint32_t seed)
{
    int i, m = ns_mood_now();
    ns.seed = seed | 1u;
    ns_layout();
    ns_mood_moon(m, 0);
    for (i = 0; i < NS_NSTAR; i++) {
        ns_st[i].want = (uint8_t)((int)ns_st[i].appear < ns.out_cap);
        ns_st[i].vis = ns_st[i].want ? 1.0f : 0.0f;
    }
    ns_background();
}

SC_SMALL static void ns_mood_kv(int m, int16_t *kv)
{
    ns_defaults(kv);
    m = sc_mood_base(m, NS_NMOOD);
    kv[KN_MOOD] = (int16_t)m;
    sc_mood_set(kv, NS_MOODS[m], NS_NSET);
}
static const char *ns_mood_name(int m) { return NS_MOOD[sc_mood_base(m, NS_NMOOD)].name; }

static const scene_t SCENE_SKY = {"Sky", 3, ns_defaults, ns_enter, ns_leave, ns_background, ns_step, ns_draw, ns_key,
                                  ns_knob, ns_knob_text, ns_regrow, NS_SOUND, 0, 0, NS_NMOOD, ns_mood_kv, ns_mood_name, 0};
