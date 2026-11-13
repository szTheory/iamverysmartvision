/* SPDX-License-Identifier: MIT */
/* Test-side word-versus-fields encoder (plan 06-10, D-19; suite video). ENCODE direction only: from the fields of a Foreground/Background
 * word (foreground 0..7, background 0..15, graphics select, card 0..63) to the word, written from the bit roles of data/video/cards.literal.txt
 * (variant fgbg: bits 2..0 foreground, 8..3 card, 11 select, bits 12 13 10 9 the background from high to low). It includes nothing from
 * core/src, so it shares no code with the renderer. Every hand-typed word of the frame literals is checked against its annotation string
 * ("fgbg fg=1 bg=2 gram card=5"); a mismatch means the fixture is wrong and is reported as FIXTURE, never as a renderer failure. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "video_cases.h"

uint32_t vid_encode_fgbg(uint32_t fg, uint32_t bg, uint32_t sel, uint32_t card)
{
  uint32_t w = fg & 7u;
  w |= (card & 0x3Fu) << 3;
  w |= (sel & 1u) << 11;
  w |= (bg & 1u) << 9;
  w |= ((bg >> 1) & 1u) << 10;
  w |= ((bg >> 2) & 1u) << 13;
  w |= ((bg >> 3) & 1u) << 12;
  return w;
}

/* The unsigned number after key (for example "fg=") in s, or -1 when key is absent. */
static long vid_field(const char *s, const char *key)
{
  const char *p = strstr(s, key);
  if (p == NULL) {
    return -1;
  }
  return (long)strtoul(p + strlen(key), NULL, 10);
}

int vid_check_cells(const vf_frame *f)
{
  uint32_t i;
  int ok = 1;
  for (i = 0u; i < f->n_cells; i++) {
    const vf_cell *c = &f->cells[i];
    long fg = vid_field(c->ann, "fg=");
    long bg = vid_field(c->ann, "bg=");
    long card = vid_field(c->ann, "card=");
    int gram = strstr(c->ann, " gram ") != NULL;
    int grom = strstr(c->ann, " grom ") != NULL;
    uint32_t want;
    if (strncmp(c->ann, "fgbg ", 5u) != 0 || fg < 0 || bg < 0 || card < 0 || gram == grom) {
      ivs_test_diag("FIXTURE: %s line %u: annotation outside the fgbg grammar: %s", f->name, (unsigned)c->line, c->ann);
      ok = 0;
      continue;
    }
    want = vid_encode_fgbg((uint32_t)fg, (uint32_t)bg, gram ? 1u : 0u, (uint32_t)card);
    if (want != c->word) {
      ivs_test_diag("FIXTURE: %s line %u: cell (%u,%u) word %04X but its annotation encodes %04X (%s)", f->name, (unsigned)c->line,
                    (unsigned)c->r, (unsigned)c->c, (unsigned)c->word, (unsigned)want, c->ann);
      ok = 0;
    }
  }
  return ok;
}
