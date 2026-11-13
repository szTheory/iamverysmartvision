/* SPDX-License-Identifier: MIT */
/* Suite video, family probe (plan 06-10, VID-04, D-18, D-19): the single-bit sweep of the Foreground/Background word. The roles of the 16
 * bits are read from the fgbg rows of data/video/cards.literal.txt through the generated include (never from the core's decode). For every
 * bit a one-bit word is rendered in cell (5, 9) over self-identifying stand-in graphics (graphics RAM card n has byte 0 = n and byte 7 =
 * 5A, the other bytes 00; the graphics ROM is the zero image), and the rendered 8 x 8 block must equal the block the roles predict:
 * foreground, background, graphics select and card read back, a bit whose role is ign changing nothing (bits 15 and 14 included).
 *
 * Deviation from the plan text, recorded in the SUMMARY: a one-bit word over the base word 0000 cannot show most roles (the graphics ROM
 * is zero, so a card number or a foreground color is invisible, and a lone select bit with colors 0 and 0 shows nothing). Each bit is
 * therefore probed twice: alone (base 0000, as the plan says) and toggled on top of a visible base word whose roles select graphics RAM
 * card 37 with foreground 5 and background 10, so that every role changes what is drawn. The expected block is computed from the literal
 * roles by an independent decoder here. */
#include <stdint.h>
#include <string.h>

#include "video_cases.h"

#define VP_CELL_R 5u
#define VP_CELL_C 9u

static uint8_t vpr_plane[VF_PLANE_BYTES];
static uint16_t vpr_words[240];

/* The fgbg roles by bit (null-terminated names pointing into the generated table). */
void vpr_roles(const char *role[16])
{
  size_t n = 0u;
  const vf_role *t = vf_role_table(&n);
  size_t i;
  for (i = 0u; i < 16u; i++) {
    role[i] = "";
  }
  for (i = 0u; i < n; i++) {
    if (strcmp(t[i].variant, "fgbg") == 0 && t[i].bit < 16u) {
      role[t[i].bit] = t[i].role;
    }
  }
}

/* The bit whose role is `name`, or 16 when none. */
static uint32_t vpr_bit_of(const char *const role[16], const char *name)
{
  uint32_t b;
  for (b = 0u; b < 16u; b++) {
    if (strcmp(role[b], name) == 0) {
      return b;
    }
  }
  return 16u;
}

/* The fields of a word read through the roles: a digit after fg, bg or card is the field bit; sel is the select; anything else is ignored. */
static void vpr_decode(const char *const role[16], uint32_t word, uint32_t *fg, uint32_t *bg, uint32_t *card, uint32_t *sel)
{
  uint32_t b;
  *fg = 0u;
  *bg = 0u;
  *card = 0u;
  *sel = 0u;
  for (b = 0u; b < 16u; b++) {
    const char *r = role[b];
    if (((word >> b) & 1u) == 0u) {
      continue;
    }
    if (strcmp(r, "sel") == 0) {
      *sel = 1u;
    } else if (strncmp(r, "fg", 2u) == 0 && r[2] >= '0' && r[2] <= '9' && r[3] == '\0') {
      *fg |= 1u << (r[2] - '0');
    } else if (strncmp(r, "bg", 2u) == 0 && r[2] >= '0' && r[2] <= '9' && r[3] == '\0') {
      *bg |= 1u << (r[2] - '0');
    } else if (strncmp(r, "card", 4u) == 0 && r[4] >= '0' && r[4] <= '9' && r[5] == '\0') {
      *card |= 1u << (r[4] - '0');
    }
  }
}

/* The stand-in graphics byte y of a card: graphics RAM card n is n at row 0 and 5A at row 7, graphics ROM is zero. */
static uint32_t vpr_byte(uint32_t sel, uint32_t card, uint32_t y)
{
  if (sel == 0u) {
    return 0u;
  }
  return y == 0u ? card : (y == 7u ? 0x5Au : 0u);
}

/* Renders the word in the probe cell and returns 1 when its block equals the block the roles predict. */
static int vpr_one(const char *const role[16], uint32_t word, uint32_t bit, int quiet)
{
  uint32_t fg;
  uint32_t bg;
  uint32_t card;
  uint32_t sel;
  uint32_t y;
  uint32_t x;
  memset(vpr_words, 0, sizeof vpr_words);
  vpr_words[20u * VP_CELL_R + VP_CELL_C] = (uint16_t)word;
  vf_render_words(vpr_words, 0u, vpr_plane);
  vpr_decode(role, word, &fg, &bg, &card, &sel);
  for (y = 0u; y < 8u; y++) {
    uint32_t byte = vpr_byte(sel, card, y);
    for (x = 0u; x < 8u; x++) {
      uint8_t want = (uint8_t)(((byte >> (7u - x)) & 1u) != 0u ? fg : bg);
      uint8_t got = vpr_plane[(8u * VP_CELL_R + y) * VF_PLANE_W + 8u * VP_CELL_C + x];
      if (got != want) {
        if (quiet == 0) {
          ivs_test_diag("bit %u (role %s), word %04X: pixel (%u,%u) read %u, the roles predict %u (fg %u bg %u sel %u card %u)", (unsigned)bit,
                        role[bit], (unsigned)word, (unsigned)x, (unsigned)y, (unsigned)got, (unsigned)want, (unsigned)fg, (unsigned)bg,
                        (unsigned)sel, (unsigned)card);
        }
        return 0;
      }
    }
  }
  return 1;
}

/* The sweep over the roles given: the number of failed probes (0 is a pass). */
uint32_t vpr_sweep(const char *const role[16], int quiet)
{
  static const char *const vis_roles[] = {"fg0", "fg2", "bg1", "bg3", "sel", "card0", "card2", "card5"};
  uint32_t vis = 0u;
  uint32_t fails = 0u;
  uint32_t card;
  uint32_t b;
  size_t i;
  vf_mem_clear();
  for (card = 0u; card < 64u; card++) {
    uint8_t g[8];
    uint32_t y;
    for (y = 0u; y < 8u; y++) {
      g[y] = (uint8_t)vpr_byte(1u, card, y);
    }
    vf_poke_gram_card(vf_mem(), card, g);
  }
  for (i = 0u; i < sizeof vis_roles / sizeof vis_roles[0]; i++) {
    uint32_t bit = vpr_bit_of(role, vis_roles[i]);
    if (bit < 16u) {
      vis |= 1u << bit;
    }
  }
  for (b = 0u; b < 16u; b++) {
    fails += vpr_one(role, 1u << b, b, quiet) ? 0u : 1u;
    fails += vpr_one(role, vis ^ (1u << b), b, quiet) ? 0u : 1u;
  }
  fails += vpr_one(role, vis, 0u, quiet) ? 0u : 1u;
  return fails;
}

/* ---- the Color Stack sweep (plan 06-11): the 48 rows of the cs-grom, cs-gram and cs-sq variants of the literal ----
 * Every bit of a variant is probed alone (word 1 << b) and toggled on a visible base word of that variant. The class of a word, hence the
 * variant whose roles decode it, follows its bits 12 and 11 as the literal's fixed rows state: 00 cs-grom, 01 and 11 cs-gram, 10 cs-sq. The
 * expected block of the probe cell and one pixel of the cell after it (the pointer persists) are computed from the roles by an independent
 * decoder here; a stack {2, 5, 9, 15} makes the advance bit show as background 5. The stand-in graphics: graphics RAM card n and graphics ROM
 * card n (a project-authored synthetic pattern poked into the test memory only, not an image of any ROM, so the graphics ROM card number
 * and foreground bits are observable) are n at row 0, 5A at row 7, 00 elsewhere. */
#define VPC_VARIANTS 3u
static const char *const vpc_names[VPC_VARIANTS] = {"cs-grom", "cs-gram", "cs-sq"};
static const uint32_t vpc_stack[4] = {2u, 5u, 9u, 15u};

static void vpc_roles(const char *role[VPC_VARIANTS][16])
{
  size_t n = 0u;
  const vf_role *t = vf_role_table(&n);
  size_t i;
  uint32_t v;
  for (v = 0u; v < VPC_VARIANTS; v++) {
    uint32_t b;
    for (b = 0u; b < 16u; b++) {
      role[v][b] = "";
    }
  }
  for (i = 0u; i < n; i++) {
    for (v = 0u; v < VPC_VARIANTS; v++) {
      if (strcmp(t[i].variant, vpc_names[v]) == 0 && t[i].bit < 16u) {
        role[v][t[i].bit] = t[i].role;
      }
    }
  }
}

static uint32_t vpc_variant_index(const char *variant)
{
  uint32_t v;
  for (v = 0u; v < VPC_VARIANTS; v++) {
    if (strcmp(vpc_names[v], variant) == 0) {
      return v;
    }
  }
  return VPC_VARIANTS;
}

int vpr_cs_roles_complete(const char *variant, uint32_t *ign)
{
  const char *role[VPC_VARIANTS][16];
  uint32_t v = vpc_variant_index(variant);
  uint32_t b;
  vpc_roles(role);
  *ign = 0u;
  if (v >= VPC_VARIANTS) {
    return 0;
  }
  for (b = 0u; b < 16u; b++) {
    if (role[v][b][0] == '\0') {
      return 0;
    }
    if (strcmp(role[v][b], "ign") == 0) {
      (*ign)++;
    }
  }
  return 1;
}

typedef struct vpc_dec {
  uint32_t cls; /* 0 graphics ROM, 1 graphics RAM, 2 squares */
  uint32_t adv;
  uint32_t card;
  uint32_t fg;
  uint32_t q[4];
} vpc_dec;

static void vpc_decode(const char *const role[VPC_VARIANTS][16], uint32_t word, vpc_dec *d)
{
  uint32_t b;
  uint32_t b12 = (word >> 12) & 1u;
  uint32_t b11 = (word >> 11) & 1u;
  memset(d, 0, sizeof *d);
  d->cls = (b12 == 1u && b11 == 0u) ? 2u : (b11 == 1u ? 1u : 0u);
  for (b = 0u; b < 16u; b++) {
    const char *r = role[d->cls][b];
    if (((word >> b) & 1u) == 0u) {
      continue;
    }
    if (strcmp(r, "adv") == 0) {
      d->adv = 1u;
    } else if (strncmp(r, "card", 4u) == 0 && r[4] >= '0' && r[4] <= '9' && r[5] == '\0') {
      d->card |= 1u << (r[4] - '0');
    } else if (strncmp(r, "fg", 2u) == 0 && r[2] >= '0' && r[2] <= '9' && r[3] == '\0') {
      d->fg |= 1u << (r[2] - '0'); /* fg0..fg2 and, in the graphics RAM word, fg3 */
    } else if (r[0] == 's' && r[1] == 'q' && r[2] >= '0' && r[2] <= '3' && r[3] == 'c' && r[4] >= '0' && r[4] <= '2' && r[5] == '\0') {
      d->q[r[2] - '0'] |= 1u << (r[4] - '0');
    }
  }
}

static uint32_t vpc_byte(uint32_t card, uint32_t y)
{
  return y == 0u ? (card & 0xFFu) : (y == 7u ? 0x5Au : 0u);
}

/* The probe cell (5, 9) holds the word; one pixel of the next cell (5, 10), a fill cell, shows the pointer after it. 1 when both match the roles. */
static int vpc_one(const char *const role[VPC_VARIANTS][16], uint32_t word, const char *what, uint32_t bit, int quiet)
{
  vpc_dec d;
  uint32_t pos;
  uint32_t y;
  uint32_t x;
  memset(vpr_words, 0, sizeof vpr_words);
  vpr_words[20u * VP_CELL_R + VP_CELL_C] = (uint16_t)word;
  vf_render_cs(vpr_words, 1u, vpc_stack, vpr_plane, NULL);
  vpc_decode(role, word, &d);
  pos = d.cls == 2u ? 0u : (d.adv != 0u ? 1u : 0u);
  for (y = 0u; y < 8u; y++) {
    for (x = 0u; x < 8u; x++) {
      uint32_t want;
      uint32_t got = vpr_plane[(8u * VP_CELL_R + y) * VF_PLANE_W + 8u * VP_CELL_C + x];
      if (d.cls == 2u) {
        uint32_t col = d.q[(y >> 2) * 2u + (x >> 2)];
        want = col == 7u ? vpc_stack[pos] : col;
      } else {
        want = ((vpc_byte(d.card, y) >> (7u - x)) & 1u) != 0u ? d.fg : vpc_stack[pos];
      }
      if (got != want) {
        if (quiet == 0) {
          ivs_test_diag("%s bit %u, word %04X: pixel (%u,%u) read %u, the roles predict %u (class %u adv %u card %u fg %u)", what, (unsigned)bit,
                        (unsigned)word, (unsigned)x, (unsigned)y, (unsigned)got, (unsigned)want, (unsigned)d.cls, (unsigned)d.adv,
                        (unsigned)d.card, (unsigned)d.fg);
        }
        return 0;
      }
    }
  }
  if (vpr_plane[(8u * VP_CELL_R) * VF_PLANE_W + 8u * (VP_CELL_C + 1u)] != vpc_stack[pos]) {
    if (quiet == 0) {
      ivs_test_diag("%s bit %u, word %04X: the cell after it reads %u, the pointer predicts %u", what, (unsigned)bit, (unsigned)word,
                    (unsigned)vpr_plane[(8u * VP_CELL_R) * VF_PLANE_W + 8u * (VP_CELL_C + 1u)], (unsigned)vpc_stack[pos]);
    }
    return 0;
  }
  return 1;
}

uint32_t vpr_sweep_cs(const char *variant, int swap, int quiet)
{
  static const char *const vis_roles[] = {"fg0", "fg2", "fg3", "card0", "card2", "card5", "card7", "adv", "sel", "fixed1", "sq0c0", "sq1c1",
                                          "sq2c2", "sq3c0", "sq3c2"};
  const char *role[VPC_VARIANTS][16];
  uint32_t v = vpc_variant_index(variant);
  uint32_t base = 0u;
  uint32_t fails = 0u;
  uint32_t card;
  uint32_t b;
  size_t i;
  if (v >= VPC_VARIANTS) {
    return 1u;
  }
  vpc_roles(role);
  if (swap != 0) {
    const char *tmp = role[v][0];
    role[v][0] = role[v][1];
    role[v][1] = tmp;
  }
  vf_mem_clear();
  for (card = 0u; card < 256u; card++) {
    uint32_t y;
    for (y = 0u; y < 8u; y++) {
      vf_mem()->grom[8u * card + y] = (uint16_t)vpc_byte(card, y);
      if (card < 64u) {
        vf_mem()->gram[8u * card + y] = (uint16_t)vpc_byte(card, y);
      }
    }
  }
  for (i = 0u; i < sizeof vis_roles / sizeof vis_roles[0]; i++) {
    for (b = 0u; b < 16u; b++) {
      if (strcmp(role[v][b], vis_roles[i]) == 0) {
        base |= 1u << b;
      }
    }
  }
  if (v == 0u) {
    base &= ~(1u << 11); /* the graphics ROM variant has its select bit clear: the base stays in the class */
  }
  for (b = 0u; b < 16u; b++) {
    fails += vpc_one(role, 1u << b, variant, b, quiet) ? 0u : 1u;
    fails += vpc_one(role, base ^ (1u << b), variant, b, quiet) ? 0u : 1u;
  }
  fails += vpc_one(role, base, variant, 16u, quiet) ? 0u : 1u;
  return fails;
}

static int vpc_variant_test(const char *variant, uint32_t want_ign)
{
  uint32_t ign = 0u;
  VC_CHECK(vpr_cs_roles_complete(variant, &ign)); /* every bit has a role row */
  VC_CHECK(ign == want_ign);
  VC_CHECK(vpr_sweep_cs(variant, 0, 0) == 0u);
  return IVS_TEST_PASS;
}

static int vpc_bit_roles_cs_grom(void)
{
  return vpc_variant_test("cs-grom", 2u); /* bits 15 and 14 */
}

static int vpc_bit_roles_cs_gram(void)
{
  return vpc_variant_test("cs-gram", 4u); /* bits 15, 14, 10 and 9 */
}

static int vpc_bit_roles_cs_sq(void)
{
  return vpc_variant_test("cs-sq", 2u); /* bits 15 and 14 */
}

/* The judge of the three sweeps: with the roles of bits 0 and 1 swapped in a copy of the literal, each sweep must report failures. */
int vpr_cs_judge(void)
{
  uint32_t v;
  for (v = 0u; v < VPC_VARIANTS; v++) {
    VC_CHECK(vpr_sweep_cs(vpc_names[v], 0, 1) == 0u); /* the control passes */
    VC_CHECK(vpr_sweep_cs(vpc_names[v], 1, 1) != 0u); /* the swapped roles fail */
  }
  return IVS_TEST_PASS;
}

static int vpr_bit_roles_fgbg(void)
{
  const char *role[16];
  uint32_t b;
  uint32_t ign = 0u;
  vpr_roles(role);
  for (b = 0u; b < 16u; b++) {
    VC_CHECK(role[b][0] != '\0'); /* every bit has a role row */
    if (strcmp(role[b], "ign") == 0) {
      ign++;
    }
  }
  VC_CHECK(ign == 2u && strcmp(role[15], "ign") == 0 && strcmp(role[14], "ign") == 0); /* bits 15 and 14 are the only ignored bits */
  VC_CHECK(vpr_sweep(role, 0) == 0u);
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_video_probe_cases[] = {
    {"video.probe.bit-roles-fgbg", vpr_bit_roles_fgbg, NULL},
    {"video.probe.bit-roles-cs-grom", vpc_bit_roles_cs_grom, NULL},
    {"video.probe.bit-roles-cs-gram", vpc_bit_roles_cs_gram, NULL},
    {"video.probe.bit-roles-cs-sq", vpc_bit_roles_cs_sq, NULL},
};
const size_t ivs_video_probe_count = sizeof ivs_video_probe_cases / sizeof ivs_video_probe_cases[0];
