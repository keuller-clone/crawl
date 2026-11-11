/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 the Crawl contributors */
/* Crypt: a stone vault by candlelight. Five candles stand on a tomb under a pointed niche where a bell hangs; dust
 * hangs in the light, water seeps from the vault. The room's light is the candles': the stone is drawn once, each
 * pixel by how much light reaches it, and the palette follows the flames, so a guttering candle dims the whole vault.
 * What is heard is what is seen:
 *
 *   a flame gutters        a note (part 1), its pitch where the candle stands
 *   wax runs to the foot   a soft note a chord tone above the last (part 3)
 *   dust in a flame        a spark, a faint high note (part 3)
 *   the bell               struck metal, low, on the chord's root (part 6: PHYS, MODAL); it swings, dust falls
 *   water from the vault   a drop heard where it lands, a ring on the wet stone
 *   under them             a choir breathing the chord (part 2: VOICE; the conductor's drone), a draught
 *
 *   KNOB 1 Mood   2 Activity   3 Instrument (Drop, Mallet, Rin, Bell)   4 Space (reverb)
 *   Volume: Notes, Choir, Air, Bell.   Nature: Tolls (how often; 0: never), Tape, Voices (Pad, Solo, Monks, Nuns,
 *   Cathedral), Drips.   Music: Echo (the tape echo), Warmth, Cloud (grains), Tide
 *   White keys  a note each, rising; the candle nearest it flares
 *   Black keys  the first five: a candle each, snuffed or lit again; the next five: the bell, a note each; the
 *               top one: a draught through the vault, the flames lean and gutter, the dust is carried off
 *
 * Moods: Long Night (the candles burn down through the others' looks in half an hour, and are set new), Vigil,
 * Procession, Sealed, Embers: each its scale, key, candles, light and choir, in CR_MOOD and CR_MOODS (data).
 *
 * The world is 240 x 240; drawing scales by r8_fb.w / 240 for pixel art. Every loop has a fixed bound. */
#define CR_NC 5                                 /* candles */
#define CR_NMOTE 16
#define CR_NWAX 3
#define CR_NDRIP 3
#define CR_NQ 8
#define CR_KLO 5                                /* the notes: white keys 5..13 (220 Hz up at the default key) */
#define CR_KN 9
#define CR_NTONE 4
#define CR_NCHOIR 5
#define CR_SOUND 1                              /* its sounds' version */
#define CR_DARK 80                              /* the panel's dark curve (render8.c r8_dark), as Hotaru's: not yet
                                                 * looked at on the FM-1 */
#define CR_FLOOR_Y 178.0f
#define CR_ALTAR_Y 148.0f                       /* the tomb's top: the candles' feet */
#define CR_LX 120.0f                            /* where the light comes from: over the tomb */
#define CR_LY 118.0f
#define CR_NLV 48                               /* the stone's shades, unlit .. fully lit */
enum { CR_WALL = 0, CR_VOID = 48, CR_WAX = 52, CR_FLAME = 56, CR_HALO = 60, CR_MOTE = 66, CR_BELL = 70, CR_DROP = 74,
       CR_WET = 76 };
#define CR_ID_BELL (FM_VOICE_ID + 48)           /* the bell's strikes (two ids, in turn: a tail under the next) */
#define CR_ID_SPARK (FM_VOICE_ID + 52)

enum { CK_MOOD, CK_ACT, CK_TONE, CK_SPACE, CK_NOTES, CK_CHOIR, CK_AIR, CK_BELL, CK_VOICES, CK_DRIPS, CK_ECHO, CK_DEPTH,
       CK_TOLLS, CK_TAPE, CK_CLOUD, CK_ARC };
static const int16_t CR_KNOB[SC_NKNOB][4] = {   /* lo, hi, step, default; SPACE is the reverb (P_REVERB) */
    {0, 4, 1, 0}, {0, 100, 1, 25}, {0, CR_NTONE - 1, 1, 0}, {0, 0, 0, 0},
    {0, 100, 1, 50}, {0, 100, 1, 45}, {0, 100, 1, 15}, {0, 100, 1, 60},
    {0, CR_NCHOIR - 1, 1, 2}, {0, 100, 1, 30}, {0, 100, 1, 40}, {0, 100, 1, 20},
    {0, 100, 1, 50}, {0, 100, 1, 35}, {0, 100, 1, 0}, {0, 100, 1, 40},
};
/* a mood: what it sounds in and how the vault looks (the knobs it sets: CR_MOODS) */
#define CR_NMOOD 5
typedef struct {
    const char *name;
    int8_t scale;                               /* SC_* (tuning.h); -1: the round's own (Long Night) */
    int16_t key;                                /* cents from the player's key */
    uint8_t lit;                                /* the candles burning, a bit each */
    uint8_t burn, gain, red;                    /* %: how far burnt down; the light; the flames' redness */
    uint8_t buzz, breath;                       /* the choir's tone and breath (VOICE's knobs, 0..127) */
    uint32_t tint;                              /* lit stone */
} cr_mood_t;
static const cr_mood_t CR_MOOD[CR_NMOOD] = {
    {"Long Night", -1, 0, 0x1F, 0, 100, 0, 40, 30, 0xB89060u},
    {"Vigil", SC_IN, 0, 0x1F, 20, 90, 0, 36, 28, 0xB08858u},
    {"Procession", SC_DORIAN, 0, 0x1F, 0, 115, 0, 60, 16, 0xD0A868u},
    {"Sealed", SC_HUNGARIAN, -500, 0x04, 85, 95, 10, 20, 70, 0x70809Cu},
    {"Embers", SC_HARMONIC, 0, 0x15, 70, 85, 75, 30, 45, 0xA84828u},
};
static const uint8_t CR_LN[4] = {2, 1, 4, 3};   /* Long Night's round: Procession, Vigil, Embers, Sealed */
#define CR_NSET 12
static const sc_set_t CR_MOODS[CR_NMOOD][CR_NSET] = {
    {{CK_ACT, 25}, {CK_NOTES, 50}, {CK_CHOIR, 45}, {CK_AIR, 15}, {CK_BELL, 60}, {CK_VOICES, 2}, {CK_DRIPS, 30},
     {CK_ECHO, 40}, {CK_DEPTH, 20}, {CK_TOLLS, 50}, {CK_TAPE, 35}, {CK_CLOUD, 0}},
    {{CK_ACT, 25}, {CK_NOTES, 50}, {CK_CHOIR, 50}, {CK_AIR, 12}, {CK_BELL, 55}, {CK_VOICES, 2}, {CK_DRIPS, 25},
     {CK_ECHO, 40}, {CK_DEPTH, 20}, {CK_TOLLS, 40}, {CK_TAPE, 35}, {CK_CLOUD, 0}},
    {{CK_ACT, 55}, {CK_NOTES, 55}, {CK_CHOIR, 55}, {CK_AIR, 10}, {CK_BELL, 70}, {CK_VOICES, 4}, {CK_DRIPS, 10},
     {CK_ECHO, 30}, {CK_DEPTH, 5}, {CK_TOLLS, 90}, {CK_TAPE, 25}, {CK_CLOUD, 0}},
    {{CK_ACT, 10}, {CK_NOTES, 40}, {CK_CHOIR, 35}, {CK_AIR, 30}, {CK_BELL, 40}, {CK_VOICES, 1}, {CK_DRIPS, 70},
     {CK_ECHO, 65}, {CK_DEPTH, 50}, {CK_TOLLS, 10}, {CK_TAPE, 70}, {CK_CLOUD, 45}},
    {{CK_ACT, 18}, {CK_NOTES, 45}, {CK_CHOIR, 45}, {CK_AIR, 20}, {CK_BELL, 50}, {CK_VOICES, 3}, {CK_DRIPS, 35},
     {CK_ECHO, 55}, {CK_DEPTH, 35}, {CK_TOLLS, 30}, {CK_TAPE, 50}, {CK_CLOUD, 15}},
};
static const uint8_t CR_TONE_SLOT[CR_NTONE] = {ZN_DROP, ZN_MALLET, ZN_RIN, ZN_BELL};
static const char *const CR_TONE_NAME[CR_NTONE] = {"Drop", "Mallet", "Rin", "Bell"};
static const char *const CR_CHOIR_NAME[CR_NCHOIR] = {"Pad", "Solo", "Monks", "Nuns", "Cathedral"};

static struct {
    float x, h;                                 /* where it stands; its height, new */
    float on, inten, gut, flare;                /* burning (0..1, eased); its flame now; a gutter and a flare, fading */
    float next, ph, ember;                      /* its next gutter (s); its own flicker; a wick's glow after it is out */
    uint8_t lit;
} cr_c[CR_NC];
static struct {
    float x, y, ph, fall;                       /* (fall: shaken from the vault, falling this long more) */
} cr_m[CR_NMOTE];
static struct {
    float y;                                    /* a drop of wax: how far down its candle */
    int8_t c, side, on;
} cr_w[CR_NWAX];
static struct {
    float x, fy, y, v, ring;                    /* a seep: where, the floor under it; its drop; its ring, fading */
    uint8_t on;
} cr_d[CR_NDRIP];
static struct {
    uint32_t at;
    int id, part, vel, pan, dur;
    float cents;
} cr_q[CR_NQ];
static struct {
    uint32_t seed, ms;
    int16_t *kv;
    float t, phase;                             /* Long Night's round, 0..1 in half an hour */
    float gain, burn, red, tint[3];             /* the look now (eased toward the mood's) */
    float light;                                /* the vault's light now: the look's gain, the flames */
    float dr, dr_to, lean;                      /* a draught: now, rising to; the flames' lean */
    float wax_next, drip_next, bell_next, spark_wait, lit_t;
    float bell_sw, bell_ph;                     /* the bell's swing: how far (fading), where */
    int lit_want, lit_auto;                     /* the candles that should burn; Long Night's own last choice */
    int pal_lv, air_sent, vowel_sent, bell_n, spark_n, set;
    uint32_t pal_ms;
    uint32_t gutters, waxdrops, sparks, tolls, drips, draughts, nights;   /* (the host's tests count them) */
    int key_note;
} cr;

static float cr_scale(void) { return (float)r8_fb.w / 240.0f; }
static int cr_mood_now(void) { return sc_mood_base(cr.kv[CK_MOOD], CR_NMOOD); }
static float cr_candle_h(int i) { return fm_maxf(5.0f, cr_c[i].h * (1.0f - 0.78f * cr.burn)); }
static uint32_t cr_h(int a, int b)              /* the picture's own noise: 0..255, by the seed */
{
    uint32_t h = ((uint32_t)a * 73856093u) ^ ((uint32_t)b * 19349663u) ^ cr.seed;
    h *= 2654435761u;
    return (h ^ (h >> 13)) >> 24;
}

/* ------------------------------------------------------------ sounds --- */
static void cr_q_add(uint32_t delay_ms, int id, int part, float cents, int vel, int pan, int dur)
{
    int i;
    for (i = 0; i < CR_NQ; i++)
        if (!cr_q[i].at) {
            cr_q[i].at = cr.ms + delay_ms + 1u;
            cr_q[i].id = id;
            cr_q[i].part = part;
            cr_q[i].cents = cents;
            cr_q[i].vel = vel;
            cr_q[i].pan = pan;
            cr_q[i].dur = dur;
            return;
        }
}

/* the bell: n strikes 2.6 s apart (a swing each), on white key k's note or the chord's root (k < 0), brought into
 * 147..294 Hz: low, the partials carry it on the FM-1's speaker */
static void cr_toll(int k, int n)
{
    float c = cd_cents(k < 0 ? CD_CHORDS[cd.chord][0] : k);
    int i;
    while (c > -700.0f)
        c -= 1200.0f;
    while (c < -1900.0f)
        c += 1200.0f;
    for (i = 0; i < n; i++)
        cr_q_add((uint32_t)(i * 2600), CR_ID_BELL + (cr.bell_n++ & 1), 6, c, 104 - 10 * i + (int)(sc_rnd() % 8u),
                 58 + (int)(sc_rnd() % 12u), 3000);
    cr.bell_sw = 1.0f;
    cr.bell_ph = 0.0f;
    cr.tolls++;
    for (i = 0; i < 5; i++) {                         /* dust shaken from the vault */
        int j = (int)(sc_rnd() % CR_NMOTE);
        cr_m[j].x = 90.0f + 60.0f * sc_frand();
        cr_m[j].y = 30.0f + 30.0f * sc_frand();
        cr_m[j].fall = 4.0f + 3.0f * sc_frand();
    }
}

static void cr_choir(void)                      /* Voices: the drone's source, a pad or one of GR!MM's choirs */
{
    int t = cr.kv[CK_VOICES];
    if (t <= 0) {
        fm_part_patch(2, ZN_DEEP_PAD);
    } else {
        fm_part_patch(2, FM_ENG_VOICE + t - 1);
        fm_part_attack(2, 1800);
        cr.vowel_sent = -100;                         /* (its knobs: at the next step) */
    }
    fm_part_release(2, 4000);
}

/* the effects: the tape (more wow, wear and hiss as Tape turns), the echo (GR!MM's DELAY: the conductor's own
 * note echoes are off here), the cloud (GRAIN, from the same line) */
static void cr_fx(void)
{
    int t = cr.kv[CK_TAPE], e = cr.kv[CK_ECHO], c = cr.kv[CK_CLOUD];
    fm_tape(t > 50 ? 100 : t * 2);
    fm_tape_knobs(t * 70 / 100, t * 60 / 100, 20 + t * 50 / 100, t * 45 / 100);
    fm_fx_send(e || c ? 70 : 0);
    fm_delay(104, 50 + e * 30 / 100, 70, e * 90 / 100);
    fm_grain(92, 46, 60, c);
}

static void cr_mix_levels(void)
{
    fm_part_level(1, cr.kv[CK_NOTES]);
    fm_part_level(3, cr.kv[CK_NOTES]);
    cd_drone_level(cr.kv[CK_CHOIR]);
    fm_part_level(6, cr.kv[CK_BELL]);
    fm_bed(FM_BED_WIND, cr.kv[CK_AIR]);
}

/* --------------------------------------------------------------- picture --- */
/* the palette: the stone from unlit to the mood's tint by the light now; the rest by the light too */
static void cr_palette(void)
{
    uint32_t tint = (uint32_t)cr.tint[0] << 16 | (uint32_t)cr.tint[1] << 8 | (uint32_t)cr.tint[2], shadow = 0x06060Au,
             fl = r8_mix(0xFFC860u, 0xE05828u, cr.red);
    float l = cr.light;
    int i;
    for (i = 0; i < CR_NLV; i++)
        r8_pal_set(CR_WALL + i, r8_mix(shadow, tint, (float)i * (1.0f / (float)(CR_NLV - 1)) * l));
    for (i = 0; i < 4; i++) {
        r8_pal_set(CR_VOID + i, r8_mix(0x020203u, tint, 0.035f * (float)i * l));
        r8_pal_set(CR_WAX + i, r8_mix(shadow, r8_mix(0xE8DCC0u, tint, 0.3f), (0.95f - 0.22f * (float)i) * fm_minf(1.0f, l + 0.1f)));
        r8_pal_set(CR_MOTE + i, r8_mix(shadow, 0xFFE8B0u, (0.14f + 0.2f * (float)i) * fm_minf(1.0f, l + 0.2f)));
        r8_pal_set(CR_BELL + i, r8_mix(0x030304u, r8_mix(0x9A7840u, tint, 0.4f), (0.75f - 0.2f * (float)i) * l));
    }
    r8_pal_set(CR_FLAME, r8_mix(0xFFF6D8u, 0xFFD890u, cr.red));   /* the flame: its core, body, tip; an ember */
    r8_pal_set(CR_FLAME + 1, fl);
    r8_pal_set(CR_FLAME + 2, r8_mix(0xE87830u, 0xB03018u, cr.red));
    r8_pal_set(CR_FLAME + 3, 0xA03018u);
    for (i = 0; i < 6; i++)                           /* its light on the stone behind, faint .. strong */
        r8_pal_set(CR_HALO + i, r8_mix(r8_base[CR_WALL + 38], fl, 0.05f + 0.07f * (float)i));
    r8_pal_set(CR_DROP, r8_mix(shadow, 0xA8C0D8u, fm_minf(1.0f, 0.3f + l)));
    r8_pal_set(CR_DROP + 1, r8_mix(shadow, 0xF4EAD0u, fm_minf(1.0f, 0.3f + l)));
    r8_pal_set(CR_WET, r8_mix(shadow, 0x8098B0u, 0.5f * l));
    r8_pal_set(CR_WET + 1, r8_mix(shadow, 0x8098B0u, 0.25f * l));
    r8_pal_commit(0xFFFFFFu, 0, 0);
}

/* the vault, drawn once: every pixel a shade of stone by the light that reaches it (the distance from the flames)
 * and what the stone is there: the wall's blocks, the niche and its arch, two pillars, the tomb, the flags */
SC_SMALL static void cr_background(void)
{
    surf_t *s = &r8_bg;
    float k = cr_scale(), ik = 1.0f / k;
    int32_t x, y;
    for (y = 0; y < s->h; y++) {
        float wy = ((float)y + 0.5f) * ik;
        uint8_t *row = s->px + y * s->w;
        for (x = 0; x < s->w; x++) {
            float wx = ((float)x + 0.5f) * ik, dx = wx - CR_LX, dy = wy - CR_LY, ax = fm_fabsf(wx - 120.0f), tone, l;
            int32_t f, iy = (int32_t)wy, ix = (int32_t)wx;
            if (wy >= CR_FLOOR_Y && !(ax < 58.0f && wy < 188.0f)) {   /* the flags: seams toward a point behind the
                                                       * tomb, rows closer together with the distance */
                float v = (wy - CR_FLOOR_Y) * (1.0f / (240.0f - CR_FLOOR_Y)), q = fm_sqrtf(v) * 4.0f,
                      u = (wx - 120.0f) / (wy - 140.0f) * 3.0f + ((int32_t)q & 1 ? 0.5f : 0.0f);
                float fq = q - fm_floorf(q), fu = u - fm_floorf(u);
                dy = (wy - 168.0f) * 2.0f;
                tone = fq < 0.07f || fu < 0.035f ? 0.5f
                                                 : 0.7f + 0.2f * (float)cr_h((int32_t)q + 40, (int32_t)fm_floorf(u)) * (1.0f / 255.0f);
                if (ax < 62.0f && wy < 198.0f)
                    tone *= 0.55f + 0.045f * (wy - 188.0f);   /* (the tomb's shadow at its foot) */
            } else if (ax < 60.0f && wy >= CR_ALTAR_Y - 3.0f && wy < CR_ALTAR_Y + 1.0f) {
                tone = 1.5f;                              /* the tomb's lid, its edge to the light */
            } else if (ax < 58.0f && wy >= CR_ALTAR_Y + 1.0f) {   /* its front, turned from the light: a panel, a cross */
                int cross = (ax < 2.5f && wy > 158.0f && wy < 181.0f) || (ax < 9.0f && wy > 163.5f && wy < 167.5f);
                tone = cross ? 0.8f : ax < 50.0f && wy > 155.0f && wy < 184.0f ? 0.36f : 0.5f;
                dy = 0.0f;
            } else if (ax > 76.0f && ax < 104.0f) {       /* a pillar: round (its side to the tomb lit), a capital
                                                       * and a base, fluted */
                float u = (ax - 76.0f) * (1.0f / 28.0f);
                tone = 1.25f * (1.0f - 0.6f * u);
                if ((ix + (wx < 120.0f ? 2 : 0)) % 7 == 0)
                    tone *= 0.82f;
                if ((wy > 22.0f && wy < 32.0f) || wy > 168.0f)
                    tone = 1.2f - 0.4f * u;
            } else if (ax > 72.0f && ax < 108.0f && ((wy > 22.0f && wy < 32.0f) || (wy > 168.0f && wy < CR_FLOOR_Y))) {
                tone = 1.1f;                              /* (the capital and the base stand out) */
            } else {
                float aw = 34.0f, au = ax * (1.0f / 34.0f), top = 40.0f + 52.0f * au * fm_sqrtf(au);   /* the niche: a
                                                       * pointed arch, dark, the light from below just reaching in */
                if (ax < aw && wy > top && wy < CR_ALTAR_Y) {
                    f = (int32_t)(fm_clampf((wy - 70.0f) * (1.0f / 80.0f), 0.0f, 1.0f) * 3.0f * 16.0f);
                    row[x] = (uint8_t)(CR_VOID + (f >> 4) + r8_dith(x, y, f & 15));
                    continue;
                }
                if (ax < aw + 7.0f && wy > 40.0f + 52.0f * fm_maxf(0.0f, (ax - 7.0f) * (1.0f / 34.0f)) *
                                                       fm_sqrtf(fm_maxf(0.0f, (ax - 7.0f) * (1.0f / 34.0f))) - 7.0f &&
                    wy < CR_ALTAR_Y) {                    /* its arch's stones */
                    tone = iy % 11 == 0 ? 0.75f : 1.22f;
                } else {                                  /* the wall: courses of blocks, each its own gray */
                    int32_t rw = iy / 16, off = (rw & 1) * 16, cl = (ix + off) / 32;
                    tone = iy % 16 == 0 || (ix + off) % 32 == 0 ? 0.6f : 0.82f + 0.3f * (float)cr_h(rw, cl) * (1.0f / 255.0f);
                    if (cr_h(rw + 99, cl) < 14u)
                        tone *= 0.7f;                     /* (a darker stone here and there) */
                    if (cr_h(ix * 7 + 3, iy) < 26u)
                        tone *= 0.9f;                     /* (pitted) */
                    if (wy < 16.0f)
                        tone *= 0.45f + wy * (0.55f / 16.0f);   /* (the vault's shadow) */
                }
            }
            l = 1.0f / (1.0f + (dx * dx + dy * dy * 1.3f) * (1.0f / (64.0f * 64.0f)));
            f = (int32_t)(l * tone * (float)((CR_NLV - 1) * 16));
            f = f < 0 ? 0 : f > (CR_NLV - 1) * 16 ? (CR_NLV - 1) * 16 : f;
            row[x] = (uint8_t)(CR_WALL + (f >> 4) + r8_dith(x, y, f & 15));
        }
    }
}

/* a flame's light on the stone behind it (only over stone: the wax and the bell stand in front) */
static void cr_glow(surf_t *s, float cx, float cy, float r)
{
    int32_t x, y, x0 = (int32_t)(cx - r), x1 = (int32_t)(cx + r), y0 = (int32_t)(cy - r), y1 = (int32_t)(cy + r);
    float r2 = r * r, ir2 = 1.0f / r2;
    for (y = y0 < 0 ? 0 : y0; y <= y1 && y < s->h; y++) {
        float dy = (float)y + 0.5f - cy;
        uint8_t *row = s->px + y * s->w;
        for (x = x0 < 0 ? 0 : x0; x <= x1 && x < s->w; x++) {
            float dx = (float)x + 0.5f - cx, u = (dx * dx + dy * dy) * ir2;
            int f, lv;
            if (u >= 1.0f || row[x] >= CR_WAX)
                continue;
            f = (int)((1.0f - u) * (1.0f - u) * 6.0f * 16.0f);
            lv = (f >> 4) + r8_dith(x, y, f & 15);
            if (lv > 0)
                row[x] = (uint8_t)(CR_HALO + (lv > 6 ? 6 : lv) - 1);
        }
    }
}

static void cr_draw(void)
{
    surf_t *s = &r8_fb;
    float k = cr_scale();
    int i;
    r8_copy(s, &r8_bg);
    {   /* the bell in the niche: its rope, its body swinging from the arch's point, the clapper a little behind */
        float a = 0.085f * cr.bell_sw * fm_sin_turns(cr.bell_ph), ux = fm_sin_turns(a), uy = fm_sin_turns(a + 0.25f),
              px = uy, py = -ux, ox = 120.0f, oy = 50.0f, tx = ox + 16.0f * ux, ty = oy + 16.0f * uy,
              mx = ox + 34.0f * ux, my = oy + 34.0f * uy;
        r8_limb(s, ox * k, oy * k, 1.2f * k, tx * k, ty * k, 1.2f * k, CR_BELL + 3);
        r8_tri(s, (tx - 3.5f * px) * k, (ty - 3.5f * py) * k, (tx + 3.5f * px) * k, (ty + 3.5f * py) * k,
               (mx + 10.0f * px) * k, (my + 10.0f * py) * k, CR_BELL + 1);
        r8_tri(s, (tx - 3.5f * px) * k, (ty - 3.5f * py) * k, (mx + 10.0f * px) * k, (my + 10.0f * py) * k,
               (mx - 10.0f * px) * k, (my - 10.0f * py) * k, CR_BELL + 1);
        r8_disc(s, tx * k, ty * k, 3.5f * k, CR_BELL + 1);
        r8_limb(s, (tx - 1.5f * px) * k, (ty - 1.5f * py) * k, 1.6f * k, (mx - 6.5f * px) * k, (my - 6.5f * py) * k, 2.2f * k,
                CR_BELL);                             /* (the light along its side) */
        r8_limb(s, (mx - 10.0f * px) * k, (my - 10.0f * py) * k, 1.5f * k, (mx + 10.0f * px) * k, (my + 10.0f * py) * k, 1.5f * k,
                CR_BELL + 2);                         /* (its lip) */
        r8_disc(s, (mx + 2.5f * ux - 40.0f * a * px) * k, (my + 2.5f * uy) * k, 1.8f * k, CR_BELL + 2);
    }
    for (i = 0; i < CR_NDRIP; i++) {                  /* water: a ring where a drop fell; a drop falling */
        if (cr_d[i].ring > 0.0f) {
            float rr = (1.2f - cr_d[i].ring) * 9.0f + 1.0f;
            r8_ring(s, cr_d[i].x * k, cr_d[i].fy * k, rr * k, rr * 0.32f * k, (uint8_t)(CR_WET + (cr_d[i].ring < 0.6f)));
        }
        if (cr_d[i].on) {
            r8_pset(s, (int32_t)(cr_d[i].x * k), (int32_t)(cr_d[i].y * k), CR_DROP);
            r8_pset(s, (int32_t)(cr_d[i].x * k), (int32_t)((cr_d[i].y - 2.0f) * k), CR_DROP);
        }
    }
    for (i = 0; i < CR_NC; i++) {                     /* the candles: the wax, its runs, the flame and its light */
        float h = cr_candle_h(i), cx = cr_c[i].x, top = CR_ALTAR_Y - 3.0f - h, in = cr_c[i].inten * cr_c[i].on;
        int32_t x0 = (int32_t)((cx - 3.0f) * k), x1 = (int32_t)((cx + 3.0f) * k) - 1, y0 = (int32_t)(top * k),
                y1 = (int32_t)((CR_ALTAR_Y - 3.0f) * k), y;
        int j;
        if (in > 0.02f)
            cr_glow(s, cx * k, (top - 5.0f) * k, (7.0f + 6.0f * in) * k);
        for (y = y0; y < y1; y++) {
            r8_hspan(s, y, x0, x1, CR_WAX + 1);
            r8_pset(s, x0, y, CR_WAX + 2);
        }
        r8_hspan(s, y0, x0, x1, CR_WAX);
        r8_hspan(s, y1 - 1, x0 - 1, x1 + 1, CR_WAX + 2);   /* (the wax pooled at its foot) */
        for (j = 0; j < 2; j++) {                     /* (old runs, set) */
            int32_t ry = y0 + 2 + (int32_t)(cr_h(i, j + 7) % 9u), rx = j ? x1 : x0 + 1;
            if (ry < y1 - 1) {
                r8_pset(s, rx, ry, CR_WAX);
                r8_pset(s, rx, ry + 1, CR_WAX);
            }
        }
        for (j = 0; j < CR_NWAX; j++)
            if (cr_w[j].on && cr_w[j].c == i)
                r8_pset(s, cr_w[j].side > 0 ? x1 : x0, y0 + (int32_t)(cr_w[j].y * k), CR_DROP + 1);
        r8_pset(s, (int32_t)(cx * k), y0 - 1, cr_c[i].ember > 0.0f ? CR_FLAME + 3 : CR_VOID);
        if (in > 0.02f) {
            float fh = (4.5f + 5.0f * in) * (1.0f - 0.35f * cr.red), bx = cx + cr.lean * 1.5f, by = top - 3.2f,
                  tipx = bx + cr.lean * 5.0f + 0.7f * fm_sin_turns(cr.t * 2.3f + cr_c[i].ph);
            r8_disc(s, bx * k, by * k, 2.3f * fm_minf(1.0f, 0.5f + in) * k, CR_FLAME + 1);
            r8_tri(s, (bx - 1.9f) * k, (by - 0.8f) * k, (bx + 1.9f) * k, (by - 0.8f) * k, tipx * k, (by - fh) * k, CR_FLAME + 1);
            r8_pset(s, (int32_t)(tipx * k), (int32_t)((by - fh) * k), CR_FLAME + 2);
            r8_disc(s, bx * k, (by + 0.3f) * k, 1.15f * k, CR_FLAME);
        }
    }
    for (i = 0; i < CR_NMOTE; i++) {                  /* the dust: seen where the light reaches it */
        float dx = cr_m[i].x - CR_LX, dy = cr_m[i].y - CR_LY;
        int lv = 3 - (int)(fm_sqrtf(dx * dx + dy * dy) * (1.0f / 34.0f));
        if (lv >= 0) {
            int32_t mx = (int32_t)(cr_m[i].x * k), my = (int32_t)(cr_m[i].y * k);
            if ((uint32_t)mx < (uint32_t)s->w && (uint32_t)my < (uint32_t)s->h && s->px[my * s->w + mx] < CR_WAX)
                s->px[my * s->w + mx] = (uint8_t)(CR_MOTE + lv);
        }
    }
}

/* ----------------------------------------------------------------- world --- */
/* where the candles stand and how tall, where the water seeps: the picture's own (its seed) */
SC_SMALL static void cr_layout(void)
{
    int i;
    for (i = 0; i < CR_NC; i++) {
        cr_c[i].x = 76.0f + 22.0f * (float)i + (float)(cr_h(i, 1) % 7u) - 3.0f;
        cr_c[i].h = 24.0f + (float)(cr_h(i, 2) % 18u);
        cr_c[i].ph = (float)cr_h(i, 3) * (1.0f / 255.0f);
    }
    for (i = 0; i < CR_NDRIP; i++) {
        static const float X0[CR_NDRIP] = {22.0f, 150.0f, 196.0f};
        cr_d[i].x = X0[i] + (float)(cr_h(i, 4) % 24u);
        cr_d[i].fy = 198.0f + (float)(cr_h(i, 5) % 30u);
        cr_d[i].on = 0;
        cr_d[i].ring = 0.0f;
    }
}

static void cr_mote_new(int i, int anywhere)
{
    cr_m[i].x = 30.0f + 180.0f * sc_frand();
    cr_m[i].y = anywhere ? 30.0f + 130.0f * sc_frand() : 150.0f + 20.0f * sc_frand();
    cr_m[i].ph = sc_frand();
    cr_m[i].fall = 0.0f;
}

/* the look the mood asks for now (Long Night: between two of its round's), and the candles to burn */
static void cr_look(float dt, int at_once)
{
    const cr_mood_t *a = &CR_MOOD[cr_mood_now()], *b = a;
    float u = 0.0f, e = at_once ? 1.0f : fm_minf(1.0f, dt * 0.25f), g, bn, rd;
    uint32_t tint;
    if (a->scale < 0) {
        float f = cr.phase * 4.0f;
        int si = (int)f, want;
        si = si > 3 ? 3 : si;
        u = fm_clampf((f - (float)si - 0.7f) * (1.0f / 0.3f), 0.0f, 1.0f);
        u = u * u * (3.0f - 2.0f * u);
        a = &CR_MOOD[CR_LN[si]];
        b = &CR_MOOD[CR_LN[(si + 1) & 3]];
        want = u < 0.5f ? a->lit : b->lit;
        if (want != cr.lit_auto)                      /* (the round's own; a candle a key changed stays until then) */
            cr.lit_auto = cr.lit_want = want;
        cd_light(a->scale);                           /* (taken when the notes rest, within a minute) */
    }
    g = ((float)a->gain + ((float)b->gain - (float)a->gain) * u) * 0.01f;
    bn = ((float)a->burn + ((float)b->burn - (float)a->burn) * u) * 0.01f;
    rd = ((float)a->red + ((float)b->red - (float)a->red) * u) * 0.01f;
    tint = r8_mix(a->tint, b->tint, u);
    cr.gain += (g - cr.gain) * e;
    cr.burn += (bn - cr.burn) * e;
    cr.red += (rd - cr.red) * e;
    cr.tint[0] += ((float)(tint >> 16 & 255u) - cr.tint[0]) * e;
    cr.tint[1] += ((float)(tint >> 8 & 255u) - cr.tint[1]) * e;
    cr.tint[2] += ((float)(tint & 255u) - cr.tint[2]) * e;
    {   /* the choir's vowel wanders (A .. O); its tone and breath are the mood's */
        int v = (int)(46.0f + 30.0f * fm_sin_turns(cr.t * (1.0f / 53.0f)) + 12.0f * fm_sin_turns(cr.t * (1.0f / 17.0f) + 0.3f));
        if (cr.kv[CK_VOICES] > 0 && (v - cr.vowel_sent >= 4 || cr.vowel_sent - v >= 4)) {
            fm_part_knobs(2, v, 12, a->buzz, a->breath);
            cr.vowel_sent = v;
        }
    }
}

static void cr_mood_tune(void)
{
    const cr_mood_t *m = &CR_MOOD[cr_mood_now()];
    cd_tune(m->scale, (float)m->key);
    if (m->scale >= 0)
        cr.lit_want = m->lit;
    else
        cr.lit_auto = -1;                             /* (Long Night: its round's candles, at the next step) */
}

SC_SMALL static void cr_defaults(int16_t *kv)
{
    int i;
    for (i = 0; i < SC_NKNOB; i++)
        kv[i] = CR_KNOB[i][3];
}

SC_SMALL static void cr_enter(uint32_t seed, int16_t *kv)
{
    int i;
    memset(&cr, 0, sizeof cr);
    memset(cr_c, 0, sizeof cr_c);
    memset(cr_w, 0, sizeof cr_w);
    memset(cr_q, 0, sizeof cr_q);
    cr.kv = kv;
    for (i = 0; i < SC_NKNOB; i++)
        if (CR_KNOB[i][1] > CR_KNOB[i][0] && (kv[i] < CR_KNOB[i][0] || kv[i] > CR_KNOB[i][1]))
            kv[i] = CR_KNOB[i][3];
    cr.seed = seed | 1u;
    sc_seed(cr.seed * 2654435761u + 7u);
    cr_layout();
    cr.phase = 0.2f * (float)(seed % 100u) * 0.01f;   /* (Long Night: early in its round) */
    cr.lit_auto = -1;
    cr.wax_next = 9.0f;
    cr.drip_next = 4.0f;
    cr.bell_next = 20.0f + 10.0f * sc_frand();
    cr.spark_wait = 8.0f;
    cr.pal_lv = cr.air_sent = -1;
    cr.vowel_sent = -100;
    for (i = 0; i < CR_NMOTE; i++)
        cr_mote_new(i, 1);
    r8_dark(CR_DARK);
    cd_init(seed);
    cd_activity((float)kv[CK_ACT] * 0.01f);
    cd_echo_amount(0.0f);                             /* (the echo is the tape echo: cr_fx) */
    cd_change_mode(1);
    cr_mood_tune();
    cr_look(0.0f, 1);
    for (i = 0; i < CR_NC; i++) {                     /* the mood's candles burning from the first */
        cr_c[i].lit = (uint8_t)(cr.lit_want >> i & 1);
        cr_c[i].on = cr_c[i].inten = (float)cr_c[i].lit;
        cr_c[i].next = 3.0f + 14.0f * sc_frand();
    }
    cd_range(CR_KLO, CR_KN);
    fm_part_patch(1, CR_TONE_SLOT[kv[CK_TONE] % CR_NTONE]);
    fm_part_release(1, 2600);
    cr_choir();
    fm_part_patch(3, ZN_RIN);                         /* wax, sparks */
    fm_part_release(3, 2600);
    fm_part_patch(6, FM_ENG_PHYS + 0);                /* the bell: MODAL, a little inharmonic, long, struck, dark */
    fm_part_knobs(6, 48, 112, 0, 44);
    fm_part_release(6, 8000);
    cr_mix_levels();
    cd_drone(1);
    cr_fx();
    fm_bed_style(FM_BED_WIND, FM_AIR_BREEZE_SYNTH);   /* (the draught: synthesized, no recording) */
    fm_bed(FM_BED_STREAM, 0);
    fm_bed(FM_BED_RAIN, 0);
    cd_melody(0);
    cd_arc_amount((float)kv[CK_ARC] * 0.01f);
    cd_depth_send(kv[CK_DEPTH]);
}

SC_SMALL static void cr_leave(void)
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

static void cr_step(uint32_t dt_ms)
{
    float dt = (float)dt_ms * 0.001f, a = (float)cr.kv[CK_ACT] * 0.01f, n = 0.0f;
    int i, j, nlit = 0;
    cr.t += dt;
    cr.ms += dt_ms;
    if (CR_MOOD[cr_mood_now()].scale < 0) {           /* Long Night: the candles burn down, and are set new */
        cr.phase += dt * (1.0f / 1800.0f);
        if (cr.phase >= 1.0f) {
            cr.phase -= 1.0f;
            cr.nights++;
            cd_dawn();
        }
    }
    cr_look(dt, 0);
    /* a draught (the top black key): it rises over ~2.5 s and dies away over ~8 s */
    if (cr.dr_to > 0.0f) {
        cr.dr = fm_minf(1.0f, cr.dr + dt * 0.4f);
        if (cr.dr >= 1.0f)
            cr.dr_to = 0.0f;
    } else if (cr.dr > 0.0f) {
        cr.dr = fm_maxf(0.0f, cr.dr - dt * 0.125f);
    }
    cr.lean += (0.9f * cr.dr + 0.08f * fm_sin_turns(cr.t * 0.23f) - cr.lean) * fm_minf(1.0f, dt * 1.5f);
    {
        int m = (int)(10.0f + 65.0f * cr.dr);
        if (m / 4 != cr.air_sent / 4) {
            fm_bed_mod(FM_BED_WIND, m);
            cr.air_sent = m;
        }
    }
    /* the candles: one lit or snuffed at a time toward those that should burn; each flame's own flicker; its
     * gutters, each a note where it stands */
    cr.lit_t -= dt;
    for (i = 0; i < CR_NC; i++) {
        float fl;
        if (cr.lit_t <= 0.0f && cr_c[i].lit != (cr.lit_want >> i & 1)) {
            cr_c[i].lit ^= 1u;
            if (!cr_c[i].lit)
                cr_c[i].ember = 3.0f;
            cr.lit_t = 1.8f;
        }
        cr_c[i].on += fm_clampf((float)cr_c[i].lit - cr_c[i].on, -dt * 2.5f, dt * 1.2f);
        cr_c[i].gut *= fm_maxf(0.0f, 1.0f - dt * 2.2f);
        cr_c[i].flare *= fm_maxf(0.0f, 1.0f - dt * 1.6f);
        cr_c[i].ember = fm_maxf(0.0f, cr_c[i].ember - dt);
        fl = 1.0f + 0.07f * fm_sin_turns(cr.t * 1.31f + cr_c[i].ph) + 0.05f * fm_sin_turns(cr.t * 3.17f + cr_c[i].ph * 2.3f) +
             0.5f * cr_c[i].flare - 0.3f * cr.dr * fm_fabsf(fm_sin_turns(cr.t * 2.6f + cr_c[i].ph));
        cr_c[i].inten = fm_maxf(0.1f, fl * (1.0f - 0.7f * cr_c[i].gut));
        if (cr_c[i].lit) {
            nlit++;
            cr_c[i].next -= dt * (0.5f + 1.5f * a) * (1.0f + 2.0f * cr.dr);
            if (cr_c[i].next <= 0.0f) {
                cr_c[i].next = 10.0f + 20.0f * sc_frand();
                cr_c[i].gut = 1.0f;
                cr.gutters++;
                cd_pluck(cr_c[i].x * (1.0f / 240.0f), 0.3f + 0.3f * sc_frand(), 1400);
            }
        }
        n += cr_c[i].on * cr_c[i].inten;
    }
    cr.light = cr.gain * (0.45f + 0.55f * fm_minf(n, 5.5f) * (1.0f / 5.0f));   /* (one candle still shows the vault) */
    /* wax: a drop runs down a burning candle; at its foot, a soft note */
    for (j = 0; j < CR_NWAX; j++)
        if (cr_w[j].on) {
            cr_w[j].y += 6.0f * dt;
            if (cr_w[j].y >= cr_candle_h(cr_w[j].c)) {
                cr_w[j].on = 0;
                cr.waxdrops++;
                cd_touch(cr_c[cr_w[j].c].x * (1.0f / 240.0f), 0.5f);
            }
        }
    cr.wax_next -= dt * (0.5f + a);
    if (cr.wax_next <= 0.0f && nlit) {
        cr.wax_next = 15.0f + 25.0f * sc_frand();
        i = (int)(sc_rnd() % CR_NC);
        for (j = 0; j < CR_NC && !cr_c[i].lit; j++)
            i = (i + 1) % CR_NC;
        for (j = 0; j < CR_NWAX; j++)
            if (!cr_w[j].on) {
                cr_w[j].on = 1;
                cr_w[j].c = (int8_t)i;
                cr_w[j].y = 0.0f;
                cr_w[j].side = (int8_t)(sc_rnd() & 1u ? 1 : -1);
                break;
            }
    }
    /* water from the vault: a drop falls, lands (heard there), a ring spreads */
    if (cr.kv[CK_DRIPS] > 0 && (cr.drip_next -= dt) <= 0.0f) {
        cr.drip_next = (5.0f + 10.0f * sc_frand()) * 40.0f / (float)(cr.kv[CK_DRIPS] + 10);
        i = (int)(sc_rnd() % CR_NDRIP);
        if (!cr_d[i].on) {
            cr_d[i].on = 1;
            cr_d[i].y = 0.0f;
            cr_d[i].v = 0.0f;
        }
    }
    for (i = 0; i < CR_NDRIP; i++) {
        if (cr_d[i].ring > 0.0f)
            cr_d[i].ring -= dt;
        if (!cr_d[i].on)
            continue;
        cr_d[i].v += 420.0f * dt;
        cr_d[i].y += cr_d[i].v * dt;
        if (cr_d[i].y >= cr_d[i].fy) {
            cr_d[i].on = 0;
            cr_d[i].ring = 1.2f;
            cr.drips++;
            fm_drop(50 + (int)(sc_rnd() % 40u), cd_pan(cr_d[i].x * (1.0f / 240.0f)), 0);
        }
    }
    /* the dust: adrift, rising in the warm air over the flames, carried by a draught; one that meets a flame is a
     * spark, a faint high note */
    cr.spark_wait -= dt;
    for (i = 0; i < CR_NMOTE; i++) {
        float vx = 2.0f * fm_sin_turns(cr.t * 0.07f + cr_m[i].ph) + 26.0f * cr.dr,
              vy = -1.0f + 1.6f * fm_sin_turns(cr.t * 0.11f + cr_m[i].ph * 3.0f);
        if (cr_m[i].fall > 0.0f) {
            cr_m[i].fall -= dt;
            vy += 9.0f;
        }
        for (j = 0; j < CR_NC; j++) {
            float dx = cr_c[j].x - cr_m[i].x, top = CR_ALTAR_Y - 3.0f - cr_candle_h(j), dy = top - 7.0f - cr_m[i].y;
            if (!cr_c[j].lit || cr_c[j].on < 0.9f)
                continue;
            if (dx > -16.0f && dx < 16.0f && cr_m[i].y < top + 4.0f && cr_m[i].y > top - 46.0f) {
                vy -= 4.0f;                           /* (the warm air above it) */
                vx += dx * 0.12f;
            }
            if (dx > -2.5f && dx < 2.5f && dy > -3.0f && dy < 3.0f && cr.spark_wait <= 0.0f) {
                cr.spark_wait = 9.0f + 12.0f * sc_frand();
                cr_c[j].flare = 0.6f;
                cr.sparks++;
                if (!cd_quiet())
                    cr_q_add(0u, CR_ID_SPARK + (cr.spark_n++ & 3), 3, cd_cents(CR_KLO + CR_KN - 1 - (int)(sc_rnd() % 4u)) + 1200.0f,
                             20 + (int)(sc_rnd() % 9u), cd_pan(cr_c[j].x * (1.0f / 240.0f)), 1500);
                cr_mote_new(i, 0);
            }
        }
        cr_m[i].x += vx * dt;
        cr_m[i].y += vy * dt;
        if (cr_m[i].y < 18.0f || cr_m[i].y > 176.0f || cr_m[i].x < 4.0f || cr_m[i].x > 236.0f)
            cr_mote_new(i, 0);
    }
    /* the bell, now and then (Tolls), one to three strikes */
    cr.bell_ph += dt * (1.0f / 5.2f);
    cr.bell_ph -= fm_floorf(cr.bell_ph);
    cr.bell_sw *= fm_maxf(0.0f, 1.0f - dt * 0.14f);
    if (cr.kv[CK_TOLLS] > 0 && (cr.bell_next -= dt) <= 0.0f) {
        cr_toll(-1, cr.kv[CK_TOLLS] >= 80 ? 3 : 1 + (int)(sc_rnd() % 3u));
        cr.bell_next = (40.0f + 2.2f * (float)(100 - cr.kv[CK_TOLLS])) * (0.7f + 0.6f * sc_frand());
    }
    cd_step(dt_ms);
    for (i = 0; i < CR_NQ; i++)
        if (cr_q[i].at && (int32_t)(cr.ms - cr_q[i].at) >= 0) {
            fm_voice(cr_q[i].id, cr_q[i].part, cr_q[i].cents, cr_q[i].vel, cr_q[i].pan, cr_q[i].dur);
            cr_q[i].at = 0;
        }
    cd_depth_send(cr.kv[CK_DEPTH]);
    {   /* the palette, when the light has moved: a step of 1/32 at most four times a second (every color changes,
         * so the whole screen is sent), or at once when a flame gutters or the look is on its way */
        int lv = (int)(cr.light * 32.0f + 0.5f) + ((int)cr.tint[0] / 6) * 64 + ((int)(cr.red * 16.0f)) * 4096, d = lv - cr.pal_lv;
        if (cr.pal_lv < 0 || !dt_ms || (lv != cr.pal_lv && (cr.ms - cr.pal_ms >= 250u || d > 2 || d < -2))) {
            cr.pal_lv = lv;
            cr.pal_ms = cr.ms;
            cr_palette();
        }
    }
}

SC_SMALL static void cr_key(int key, int on)
{
    int b, i;
    if (!on)
        return;
    for (b = 0; b < FM_NBLACK; b++)
        if (BLACK_K[b] == key) {
            if (b < CR_NC) {                          /* a candle, snuffed or lit again */
                cr.lit_want ^= 1 << b;
                cr.lit_t = 0.0f;
            } else if (b < FM_NBLACK - 1) {           /* the bell: five notes, low to high */
                cr_toll(b - CR_NC, 1);
                cr.key_note = b - CR_NC;
            } else {                                  /* a draught */
                cr.dr_to = 1.0f;
                cr.draughts++;
            }
            return;
        }
    for (b = 0; b < FM_NWHITE; b++)
        if (WHITE_K[b] == key) {
            int k = b + 3, best = 0;                  /* (sixteen notes, each its own, rising) */
            float x = 12.0f + 216.0f * (float)b / (float)(FM_NWHITE - 1), bd = 1e9f;
            for (i = 0; i < CR_NC; i++)
                if (cr_c[i].lit && fm_fabsf(cr_c[i].x - x) < bd) {
                    bd = fm_fabsf(cr_c[i].x - x);
                    best = i;
                }
            if (bd < 1e8f)
                cr_c[best].flare = 1.0f;
            cd_note_key(k, x * (1.0f / 240.0f), 0.5f + 0.25f * (1.0f - (float)b / (float)(FM_NWHITE - 1)), 1400, 1, 0u);
            cr.key_note = k;
            return;
        }
}

SC_SMALL static void cr_knob(int i, int32_t e)
{
    if (i == CK_MOOD) {                               /* a mood: every knob to the sound, its key, scale, candles */
        (void)e;
        cr_mood_tune();
        cd_activity((float)cr.kv[CK_ACT] * 0.01f);
        cd_arc_amount((float)cr.kv[CK_ARC] * 0.01f);
        fm_part_patch(1, CR_TONE_SLOT[cr.kv[CK_TONE] % CR_NTONE]);
        cr_choir();
        cr_mix_levels();
        cr_fx();
        cr.lit_t = 0.0f;
        return;
    }
    if (i == CK_SPACE) {
        knob_set(P_REVERB, proj.par[P_REVERB] + (int)(e * sc_knob_mul));
        return;
    }
    if (i < 0 || i >= SC_NKNOB || CR_KNOB[i][1] <= CR_KNOB[i][0])
        return;
    if (i == CK_TONE || i == CK_VOICES)
        sc_knob_wrap(&cr.kv[i], e, CR_KNOB[i][0], CR_KNOB[i][1]);
    else
        sc_knob_turn(&cr.kv[i], e, CR_KNOB[i][0], CR_KNOB[i][1], CR_KNOB[i][2]);
    switch (i) {
    case CK_ACT: cd_activity((float)cr.kv[CK_ACT] * 0.01f); break;
    case CK_TONE: fm_part_patch(1, CR_TONE_SLOT[cr.kv[CK_TONE] % CR_NTONE]); break;
    case CK_VOICES: cr_choir(); break;
    case CK_ECHO: case CK_TAPE: case CK_CLOUD: cr_fx(); break;
    case CK_ARC: cd_arc_amount((float)cr.kv[CK_ARC] * 0.01f); break;
    case CK_NOTES: case CK_CHOIR: case CK_AIR: case CK_BELL: cr_mix_levels(); break;
    default: break;                                   /* (Drips, Tolls, Warmth: read each step) */
    }
}

SC_SMALL static void cr_knob_text(int i, char *name, char *val)
{
    static const char *const N[SC_NUSED] = {"Mood", "Activity", "Instrument", "Space", "Notes", "Choir", "Air", "Bell",
                                            "Voices", "Drips", "Echo", "Warmth", "Tolls", "Tape", "Cloud", "Tide"};
    if (i < 0 || i >= SC_NUSED || (i != CK_SPACE && CR_KNOB[i][1] <= CR_KNOB[i][0])) {
        name[0] = val[0] = 0;
        return;
    }
    copy_str(name, N[i], 11);
    switch (i) {
    case CK_MOOD: copy_str(val, CR_MOOD[cr_mood_now()].name, 11); break;
    case CK_TONE: copy_str(val, CR_TONE_NAME[cr.kv[CK_TONE] % CR_NTONE], 11); break;
    case CK_VOICES: copy_str(val, CR_CHOIR_NAME[cr.kv[CK_VOICES] % CR_NCHOIR], 11); break;
    case CK_SPACE: itoa_u((uint32_t)proj.par[P_REVERB], val); break;
    default: itoa_u((uint32_t)cr.kv[i], val); break;
    }
}

/* a new vault (PLAY; the glue fades through dark): other stones, candles, seeps; the world goes on */
SC_SMALL static void cr_regrow(uint32_t seed)
{
    cr.seed = seed | 1u;
    cr_layout();
    cr_background();
}

SC_SMALL static void cr_mood_kv(int m, int16_t *kv)
{
    cr_defaults(kv);
    m = sc_mood_base(m, CR_NMOOD);
    kv[CK_MOOD] = (int16_t)m;
    sc_mood_set(kv, CR_MOODS[m], CR_NSET);
}
static const char *cr_mood_name(int m) { return CR_MOOD[sc_mood_base(m, CR_NMOOD)].name; }

static const scene_t SCENE_CRYPT = {"Crypt", 1, cr_defaults, cr_enter, cr_leave, cr_background, cr_step, cr_draw, cr_key,
                                    cr_knob, cr_knob_text, cr_regrow, CR_SOUND, 0, 0, CR_NMOOD, cr_mood_kv, cr_mood_name, 0};
