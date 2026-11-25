/* SPDX-License-Identifier: MIT */
/* Suite state, shared helpers (plan 08-13, D-16): see state_util.h. The expected values of every row come from the specification
 * docs/formats/ivsn-v1.md and the literals of data/expected/ivsn-literals.ids; the restore code is only ever called, never consulted. */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "ivs/ivs.h"
#include "machine/machine.h"
#include "state_util.h"

#define SU_NONE 0xFFFFFFFFu /* section_index of a refusal that concerns no section */
#define SU_REP 360u         /* the v1 size of the report */

/* The input of a fault build gets a zeroed tail (see su_heap_copy); a normal build gets exactly the bytes it supplies. */
#ifdef IVS_STATE_FAULT_RUN
#define SU_TAIL 4400u
#else
#define SU_TAIL 0u
#endif

/* The build-tree images (tools/pack, ivs_pack --c-include): the boot stub contracts 1 and 2 and the three diagnostic cartridges. */
#include "ivs_state_images.inc"

/* The stand-in boot program, the same 23 decles as state_save.c (each derived in tests/machine/machine_standin.c from the printed vector
 * rows of the CP1610 manual): four PSG writes (mixer $3E, amplitude A $0F, tone A fine period 1 and coarse 0), then R1 = $0155 and the
 * loop that stores R1 at $0200 and adds 1 forever. */
const uint16_t su_boot_words[SU_BOOT_WORDS] = {0x02B8u, 0x003Eu, 0x0240u, 0x01F8u, 0x02B8u, 0x000Fu, 0x0240u, 0x01FBu,
                                               0x02B8u, 0x0001u, 0x0240u, 0x01F0u, 0x02B8u, 0x0000u, 0x0240u, 0x01F4u,
                                               0x02B9u, 0x0155u, 0x0241u, 0x0200u, 0x0009u, 0x0220u, 0x0004u};

int su_boot_at(void *storage, uint32_t images, int power, ivs_machine **mp)
{
  ivs_create_options o;
  ivs_machine *m = NULL;
  size_t cn = 0u;
  uint8_t *cart = NULL;
  int ok = 1;
  memset(&o, 0, sizeof o);
  o.struct_size = (uint32_t)sizeof o;
  o.profile_id = IVS_MS_PROFILE;
  if (ivs_machine_create(&o, storage, IVS_MACHINE_BYTES, &m) != IVS_OK) {
    ivs_test_diag("create refused");
    return 0;
  }
  if ((images & SU_IMG_CART) != 0u) {
    cart = ivs_ms_cart(0x5000u, 16u, 0u, &cn); /* its own statement: cn is an out parameter */
  }
  if ((images & SU_IMG_EXEC) != 0u) {
    ok = ok && ivs_ms_load(m, IVS_KIND_EXEC, ivs_ms_exec_words(su_boot_words, SU_BOOT_WORDS), IVS_MS_EXEC_BYTES);
  }
  if ((images & SU_IMG_GROM) != 0u) {
    ok = ok && ivs_ms_load(m, IVS_KIND_GROM, ivs_ms_grom(0u), IVS_MS_GROM_BYTES);
  }
  if ((images & SU_IMG_CART) != 0u) {
    if (ok) {
      ok = ivs_ms_load(m, IVS_KIND_CARTRIDGE, cart, cn);
    } else {
      free(cart);
    }
  }
  if (ok && power != 0) {
    ok = ivs_machine_reset(m, IVS_RESET_POWER_ON) == IVS_OK;
  }
  *mp = m;
  return ok;
}

int su_boot_new(ivs_ms_inst *x, uint32_t images, int power, uint8_t poison)
{
  x->m = NULL;
  x->storage = aligned_alloc(IVS_MACHINE_ALIGN, IVS_MACHINE_BYTES);
  if (x->storage == NULL) {
    return 0;
  }
  memset(x->storage, poison, IVS_MACHINE_BYTES);
  if (!su_boot_at(x->storage, images, power, &x->m)) {
    free(x->storage);
    x->storage = NULL;
    x->m = NULL;
    return 0;
  }
  return 1;
}

int su_boot_stepped(ivs_ms_inst *x, uint32_t units, uint8_t poison)
{
  if (!su_boot_new(x, SU_IMG_ALL, 1, poison)) {
    return 0;
  }
  if (units > 0u && !ivs_ms_step(x->m, units, NULL)) {
    ivs_ms_free(x);
    return 0;
  }
  return 1;
}

int su_nine_digests(const ivs_machine *m, uint8_t out[9][32])
{
  static const uint32_t doms[9] = {IVS_DIGEST_COMPOSITE, IVS_DIGEST_CPU,   IVS_DIGEST_MEM,   IVS_DIGEST_IMAGES, IVS_DIGEST_INSTANCE,
                                   IVS_DIGEST_VIDEO,     IVS_DIGEST_AUDIO, IVS_DIGEST_INPUT, IVS_DIGEST_PSGLOG};
  uint32_t i;
  for (i = 0u; i < 9u; i++) {
    if (ivs_machine_state_digest(m, doms[i], out[i]) != IVS_OK) {
      return 0;
    }
  }
  return 1;
}

void su_poison(ivs_machine *m, uint8_t byte)
{
  uint8_t *b = (uint8_t *)m;
  size_t n = ivs_machine_digest_field_count();
  size_t i;
  for (i = 0u; i < n; i++) {
    const ivs_digest_field *f = ivs_machine_digest_field(i);
    if (f != NULL && (f->snap == IVS_SNAP_STATE || f->snap == IVS_SNAP_PAD)) {
      memset(b + f->offset, byte, (size_t)f->width * f->count);
    }
  }
}

void state_poison(ivs_machine *m)
{
  su_poison(m, 0xA5u);
}

int su_snapshot(const ivs_machine *m, uint8_t *buf, size_t cap, size_t *n)
{
  uint64_t w = 0u;
  if (ivs_machine_snapshot_save(m, buf, cap, &w) != IVS_OK || w > cap) {
    return 0;
  }
  *n = (size_t)w;
  return 1;
}

void su_reseal(uint8_t *snap, size_t n)
{
  ivs_sha256_digest(snap, n - IVS_SNAP_TRAILER_BYTES, snap + n - IVS_SNAP_TRAILER_BYTES);
}

uint64_t su_get(const uint8_t *p, uint32_t width)
{
  uint64_t v = 0u;
  uint32_t i;
  for (i = 0u; i < width; i++) {
    v = (v << 8) | (uint64_t)p[i];
  }
  return v;
}

void su_put(uint8_t *p, uint64_t v, uint32_t width)
{
  uint32_t i;
  for (i = 0u; i < width; i++) {
    p[i] = (uint8_t)(v >> (8u * (width - 1u - i)));
  }
}

uint8_t *su_heap_copy(const uint8_t *src, size_t len)
{
  uint8_t *p = (uint8_t *)malloc((len > 0u ? len : 1u) + SU_TAIL);
  if (p == NULL) {
    return NULL;
  }
  if (len > 0u) {
    memcpy(p, src, len);
  }
  if (SU_TAIL > 0u) {
    memset(p + len, 0, SU_TAIL);
  }
  return p;
}

uint32_t su_section_offset(uint32_t i)
{
  char key[48];
  uint32_t o = 0u;
  if (i >= ST_SECTIONS) {
    return 0u;
  }
  (void)snprintf(key, sizeof key, "section-offset.%s", st_section_name[i]);
  return ivsn_lit_u32(key, &o) ? o : 0u;
}

int su_row_pos(const char *row, uint32_t *section, uint32_t *value_off, uint32_t *count_off)
{
  size_t rows = ivs_machine_digest_field_count();
  uint32_t i;
  for (i = 0u; i < ST_SECTIONS; i++) {
    uint32_t base = su_section_offset(i);
    uint32_t pos = IVS_SNAP_FRAME_BYTES;
    size_t k;
    for (k = 0u; k < rows; k++) {
      const ivs_digest_field *f = ivs_machine_digest_field(k);
      uint32_t cpos = 0u;
      if (f == NULL || f->domain != st_section_domain[i] || f->snap != IVS_SNAP_STATE || f->kind != IVS_DIGEST_KIND_UINT) {
        continue;
      }
      if (f->count > 1u) {
        cpos = base + pos;
        pos += 4u;
      }
      if (strcmp(f->name, row) == 0) {
        *section = i;
        *value_off = base + pos;
        *count_off = cpos;
        return base != 0u;
      }
      pos += f->width * f->count;
    }
  }
  return 0;
}

ivs_status su_status_of(uint32_t reason)
{
  switch (reason) {
  case 58u: /* SNAP_BUF_NULL */
  case 59u: /* SNAP_REPORT_OVERLAP */
  case 81u: /* SNAP_ARG_OVERLAP_INSTANCE */
    return IVS_E_BAD_ARG;
  case 60u: /* SNAP_OVERSIZED */
    return IVS_E_OVERSIZED;
  case 61u: /* SNAP_TRUNCATED */
  case 62u: /* SNAP_MAGIC */
  case 67u: /* SNAP_TOTAL_LENGTH */
  case 68u: /* SNAP_SECTION_COUNT */
  case 69u: /* SNAP_SECTION_TABLE */
  case 71u: /* SNAP_PROFILE_ID_FORM */
  case 77u: /* SNAP_ROW_COUNT */
  case 78u: /* SNAP_FIELD_RANGE */
  case 79u: /* SNAP_INVARIANT */
  case 80u: /* SNAP_SECTION_LENGTH */
    return IVS_E_MALFORMED;
  case 63u: /* SNAP_FORMAT_VERSION */
  case 64u: /* SNAP_FLAGS */
  case 65u: /* SNAP_SCHEMA_VERSION */
  case 66u: /* SNAP_SCHEMA_DIGEST */
    return IVS_E_UNSUPPORTED;
  case 70u: /* SNAP_TRAILER */
    return IVS_E_DIGEST_MISMATCH;
  case 72u: /* SNAP_UNKNOWN_PROFILE */
    return IVS_E_UNKNOWN_PROFILE;
  case 73u: /* SNAP_PROFILE_MISMATCH */
  case 74u: /* SNAP_PROFILE_PARAMS */
  case 76u: /* SNAP_SLOT_MISMATCH */
    return IVS_E_STATE_MISMATCH;
  case 75u: /* SNAP_SLOT_MISSING */
    return IVS_E_MISSING_SYSTEM_IMAGE;
  default:
    return IVS_OK;
  }
}

/* ---- the table of rows ---- */

int su_tab_init(su_tab *t, size_t cap)
{
  memset(t, 0, sizeof *t);
  t->row = (su_row *)calloc(cap, sizeof *t->row);
  t->cap = cap;
  return t->row != NULL;
}

void su_tab_free(su_tab *t)
{
  free(t->row);
  memset(t, 0, sizeof *t);
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 2, 3)))
#endif
su_row *su_add(su_tab *t, const char *fmt, ...)
{
  static su_row spare;
  su_row *r;
  va_list ap;
  if (t->n >= t->cap) {
    t->bad = 1;
    memset(&spare, 0, sizeof spare);
    return &spare;
  }
  r = &t->row[t->n++];
  memset(r, 0, sizeof *r);
  va_start(ap, fmt);
  (void)vsnprintf(r->name, sizeof r->name, fmt, ap);
  va_end(ap);
  r->sec = SU_NONE;
  return r;
}

static void su_patch_add(su_tab *t, su_row *r, uint32_t off, uint8_t op, uint8_t v)
{
  if (r->np >= SU_PATCHES || off > 0xFFFFu) {
    t->bad = 1;
    return;
  }
  r->p[r->np].off = (uint16_t)off;
  r->p[r->np].op = op;
  r->p[r->np].v = v;
  r->np++;
}

void su_xor(su_tab *t, su_row *r, uint32_t off, uint8_t m)
{
  su_patch_add(t, r, off, 1u, m);
}

void su_set(su_tab *t, su_row *r, uint32_t off, uint8_t v)
{
  su_patch_add(t, r, off, 2u, v);
}

void su_setn(su_tab *t, su_row *r, uint32_t off, uint32_t width, uint64_t v)
{
  uint32_t i;
  for (i = 0u; i < width; i++) {
    su_set(t, r, off + i, (uint8_t)(v >> (8u * (width - 1u - i))));
  }
}

void su_expect(su_row *r, uint32_t reason, uint64_t off, uint32_t sec, uint32_t kind)
{
  r->reason = reason;
  r->off = off;
  r->sec = sec;
  r->kind = kind;
}

/* ---- the run ---- */

void su_run_init(su_run *u, const uint8_t *base, size_t base_n, int poisoned)
{
  memset(u, 0, sizeof *u);
  u->base = base;
  u->base_n = base_n;
  u->poisoned = poisoned;
  u->digests = 1;
}

void su_run_free(su_run *u)
{
  uint32_t k;
  for (k = 0u; k < SU_T_COUNT; k++) {
    if (u->have[k]) {
      ivs_ms_free(&u->target[k]);
      free(u->ref[k]);
    }
  }
  memset(u, 0, sizeof *u);
}

/* The target of a row, built on first use: tsel picks which images it holds and whether it is powered; a poisoned run poisons its STATE
 * and PAD rows. Its bytes (and nine digests) before the first row are kept to prove that no row changed it. */
static ivs_ms_inst *su_target(su_run *u, uint32_t tsel)
{
  static const uint32_t images[SU_T_COUNT] = {SU_IMG_ALL, SU_IMG_EXEC | SU_IMG_GROM, SU_IMG_GROM | SU_IMG_CART, SU_IMG_EXEC | SU_IMG_CART};
  static const int power[SU_T_COUNT] = {1, 1, 0, 0};
  ivs_ms_inst *x;
  if (tsel >= SU_T_COUNT) {
    return NULL;
  }
  x = &u->target[tsel];
  if (u->have[tsel]) {
    return x;
  }
  if (!su_boot_new(x, images[tsel], power[tsel], 0x5Au)) {
    return NULL;
  }
  if (power[tsel] != 0 && !ivs_ms_step(x->m, tsel == SU_T_FULL ? 777u : 300u, NULL)) {
    ivs_ms_free(x);
    return NULL;
  }
  if (u->poisoned) {
    state_poison(x->m);
  }
  u->ref[tsel] = (uint8_t *)malloc(IVS_MACHINE_BYTES);
  if (u->ref[tsel] == NULL) {
    ivs_ms_free(x);
    return NULL;
  }
  memcpy(u->ref[tsel], x->storage, IVS_MACHINE_BYTES);
  /* The reference digests are taken here, once, for either digest mode: a sampled run still needs the before value on the rows it
   * does sample, and row 0 is always one of them. */
  if ((u->digests || u->digest_sampled) && !su_nine_digests(x->m, u->dig[tsel])) {
    ivs_ms_free(x);
    free(u->ref[tsel]);
    u->ref[tsel] = NULL;
    return NULL;
  }
  u->have[tsel] = 1;
  return x;
}

static void su_put32(uint8_t *exp, size_t at, uint32_t v)
{
  memcpy(exp + at, &v, sizeof v);
}

static void su_put64(uint8_t *exp, size_t at, uint64_t v)
{
  memcpy(exp + at, &v, sizeof v);
}

/* Edits exp, the report as the caller gave it, into the report the contract of ivs/ivs_state.h allows after a refusal with the given
 * reason, byte offset, section index and slot kind: only those members, and the header facts that the readable bytes of src let the reader
 * see (the format version from 12 bytes, the total from 16 and the schema version from 24). */
void su_expect_report(uint8_t *exp, const uint8_t *src, size_t readable, uint32_t reason, uint64_t off, uint32_t sec, uint32_t kind)
{
  su_put32(exp, offsetof(ivs_snapshot_report, reason), reason);
  su_put64(exp, offsetof(ivs_snapshot_report, byte_offset), off);
  su_put32(exp, offsetof(ivs_snapshot_report, section_index), sec);
  if (kind != 0u) {
    su_put32(exp, offsetof(ivs_snapshot_report, kind), kind);
  }
  if (readable >= 12u) {
    su_put32(exp, offsetof(ivs_snapshot_report, format_version), (uint32_t)su_get(src + IVS_SNAP_OFF_FORMAT, 2u));
  }
  if (readable >= 16u) {
    su_put64(exp, offsetof(ivs_snapshot_report, total_bytes), su_get(src + IVS_SNAP_OFF_TOTAL, 4u));
  }
  if (readable >= 24u) {
    su_put32(exp, offsetof(ivs_snapshot_report, schema_version), (uint32_t)su_get(src + IVS_SNAP_OFF_SCHEMA, 4u));
  }
}

/* One row against its target.  r is the row's index in the table and n the table's row count, which are what the sampled digest mode
 * selects on; neither is otherwise used. */
static int su_call_row(su_run *u, const su_row *w, const uint8_t *work, uint8_t *h, uint64_t claimed, size_t readable, size_t r, size_t n)
{
  static const uint8_t poison[2] = {0xA5u, 0x5Au};
  ivs_ms_inst *x = su_target(u, w->tsel);
  ivs_status want = su_status_of(w->reason);
  uint8_t after[9][32];
  int ok = 1;
  uint32_t k;
  if (x == NULL) {
    return 0;
  }
  for (k = 0u; k < 2u; k++) {
    su_rep rep;
    uint8_t exp[SU_CALLER];
    ivs_status st;
    memset(rep.b, poison[k], sizeof rep.b);
    rep.r.struct_size = (uint32_t)sizeof rep.b;
    memcpy(exp, rep.b, sizeof exp);
    su_expect_report(exp, work, readable, w->reason, w->off, w->sec, w->kind);
    st = ivs_machine_snapshot_restore(x->m, h, claimed, &rep.r);
    if (st != want) {
      if (!u->quiet) {
        ivs_test_diag("row '%s': status %s (want %s)", w->name, ivs_status_name(st), ivs_status_name(want));
      }
      ok = 0;
    }
    if (memcmp(rep.b, exp, sizeof exp) != 0) {
      size_t i = 0u;
      while (rep.b[i] == exp[i]) {
        i++;
      }
      if (!u->quiet) {
        ivs_test_diag("row '%s' (poison %02X): report byte %u is %02X, the contract says %02X", w->name, (unsigned)poison[k],
                      (unsigned)i, (unsigned)rep.b[i], (unsigned)exp[i]);
      }
      ok = 0;
    }
  }
  if (readable > 0u && memcmp(h, work, readable) != 0) {
    if (!u->quiet) {
      ivs_test_diag("row '%s': the supplied bytes were changed", w->name);
    }
    ok = 0;
  }
  if (memcmp(x->storage, u->ref[w->tsel], IVS_MACHINE_BYTES) != 0) {
    if (!u->quiet) {
      ivs_test_diag("row '%s': the instance bytes changed by a refusal", w->name);
    }
    ok = 0;
  }
  /* The whole-instance comparison above and the produced accumulation below are outside this branch: the digest mode, every-row or
   * sampled, changes only how often the nine digests are computed, never whether the instance itself is compared (plan 09-02, trim (c)). */
  if (u->digests || (u->digest_sampled && (r == 0u || r % 8u == 0u || r + 1u == n))) {
    if (!su_nine_digests(x->m, after) || memcmp(after, u->dig[w->tsel], sizeof after) != 0) {
      if (!u->quiet) {
        ivs_test_diag("row '%s': a digest of the instance changed by a refusal", w->name);
      }
      ok = 0;
    }
  }
  if (ok && w->reason >= 58u && w->reason <= 81u) {
    u->produced |= 1u << (w->reason - 58u);
  }
  return ok;
}

uint32_t su_run_rows(su_run *u, const su_tab *t)
{
  static uint8_t work[IVS_SNAPSHOT_MAX_BYTES + 64u];
  uint32_t bad = 0u;
  size_t r;
  if (t->bad) {
    ivs_test_diag("a row table overflowed: raise its capacity");
    return 1u;
  }
  for (r = 0u; r < t->n; r++) {
    const su_row *w = &t->row[r];
    uint8_t *h;
    uint64_t claimed;
    size_t readable;
    size_t i;
    memset(work, 0, sizeof work);
    memcpy(work, u->base, u->base_n);
    for (i = 0u; i < w->np; i++) {
      uint8_t *b = &work[w->p[i].off];
      *b = w->p[i].op == 1u ? (uint8_t)(*b ^ w->p[i].v) : w->p[i].v;
    }
    if (w->reseal != 0u) {
      su_reseal(work, u->base_n);
    }
    if (w->big_len != 0u) {
      h = su_heap_copy(work, 1u);
      claimed = w->big_len;
      readable = 0u;
    } else {
      readable = w->trunc != 0u ? (size_t)w->trunc : u->base_n;
      h = su_heap_copy(work, readable);
      claimed = readable;
    }
    if (h == NULL) {
      bad++;
      continue;
    }
    if (!su_call_row(u, w, work, h, claimed, readable, r, t->n)) {
      bad++;
    }
    free(h);
  }
  return bad;
}

/* ---- the scenario programs ---- */

typedef struct su_scn {
  const char *name;
  const uint8_t *exec;
  size_t exec_n;
  const uint8_t *cart;
  size_t cart_n;
  uint32_t n_events;
  ivs_input_event ev[6];
} su_scn;

#define SU_EV(t, c0, c1) {(t), 0u, 0u, {(c0), (c1), 0u, 0u}}

/* The events of data/scenarios/demo-lamp-tone-pass.ivss, in its order and at its ticks (c0 is the port read at $01FE, c1 the port read at
 * $01FF), and the selector words of the diag-stic scenarios (KEY_n closed from tick 0). */
static const su_scn su_scns[SU_SC_COUNT] = {
    {"lamp-idle", ivs_img_boot_stub_2, IVS_IMG_BOOT_STUB_2_BYTES, ivs_img_lamp_tone, IVS_IMG_LAMP_TONE_BYTES, 0u, {{0}}},
    {"lamp-pass", ivs_img_boot_stub_2, IVS_IMG_BOOT_STUB_2_BYTES, ivs_img_lamp_tone, IVS_IMG_LAMP_TONE_BYTES, 6u,
     {SU_EV(100000u, IVS_CTRL_KEY_1, 0u), SU_EV(220000u, IVS_CTRL_KEY_1 | IVS_CTRL_KEY_9, 0u), SU_EV(340000u, 0u, IVS_CTRL_DISC_E),
      SU_EV(460000u, IVS_CTRL_SIDE_TOP, IVS_CTRL_DISC_E), SU_EV(580000u, 0u, 0u),
      SU_EV(700000u, IVS_CTRL_KEY_1 | IVS_CTRL_KEY_9, IVS_CTRL_SIDE_LOWER_LEFT)}},
    {"stub2", ivs_img_boot_stub_2, IVS_IMG_BOOT_STUB_2_BYTES, NULL, 0u, 0u, {{0}}},
    {"stic1-fgbg", ivs_img_boot_stub, IVS_IMG_BOOT_STUB_BYTES, ivs_img_diag_stic_1, IVS_IMG_DIAG_STIC_1_BYTES, 0u, {{0}}},
    {"stic1-cstack", ivs_img_boot_stub, IVS_IMG_BOOT_STUB_BYTES, ivs_img_diag_stic_1, IVS_IMG_DIAG_STIC_1_BYTES, 1u,
     {SU_EV(0u, IVS_CTRL_KEY_1, 0u)}},
    {"stic2-idle", ivs_img_boot_stub_2, IVS_IMG_BOOT_STUB_2_BYTES, ivs_img_diag_stic_2, IVS_IMG_DIAG_STIC_2_BYTES, 0u, {{0}}},
    {"stic2-mob", ivs_img_boot_stub_2, IVS_IMG_BOOT_STUB_2_BYTES, ivs_img_diag_stic_2, IVS_IMG_DIAG_STIC_2_BYTES, 1u,
     {SU_EV(0u, IVS_CTRL_KEY_1, 0u)}},
    {"stic2-border", ivs_img_boot_stub_2, IVS_IMG_BOOT_STUB_2_BYTES, ivs_img_diag_stic_2, IVS_IMG_DIAG_STIC_2_BYTES, 1u,
     {SU_EV(0u, IVS_CTRL_KEY_2, 0u)}},
    {"stic2-hdelay", ivs_img_boot_stub_2, IVS_IMG_BOOT_STUB_2_BYTES, ivs_img_diag_stic_2, IVS_IMG_DIAG_STIC_2_BYTES, 1u,
     {SU_EV(0u, IVS_CTRL_KEY_3, 0u)}},
    {"stic2-vdelay", ivs_img_boot_stub_2, IVS_IMG_BOOT_STUB_2_BYTES, ivs_img_diag_stic_2, IVS_IMG_DIAG_STIC_2_BYTES, 1u,
     {SU_EV(0u, IVS_CTRL_KEY_4, 0u)}},
    {"stic2-collision", ivs_img_boot_stub_2, IVS_IMG_BOOT_STUB_2_BYTES, ivs_img_diag_stic_2, IVS_IMG_DIAG_STIC_2_BYTES, 1u,
     {SU_EV(0u, IVS_CTRL_KEY_5, 0u)}},
};

const char *su_scenario_name(uint32_t sc)
{
  return sc < SU_SC_COUNT ? su_scns[sc].name : "";
}

const ivs_input_event *su_scenario_events(uint32_t sc, uint32_t *count)
{
  if (sc >= SU_SC_COUNT || su_scns[sc].n_events == 0u) {
    *count = 0u;
    return NULL;
  }
  *count = su_scns[sc].n_events;
  return su_scns[sc].ev;
}

static uint8_t *su_copy(const uint8_t *src, size_t n)
{
  uint8_t *b = (uint8_t *)malloc(n);
  if (b != NULL) {
    memcpy(b, src, n);
  }
  return b;
}

int su_scenario_new(ivs_ms_inst *x, uint32_t sc, int power)
{
  const su_scn *s;
  int ok;
  if (sc >= SU_SC_COUNT) {
    return 0;
  }
  s = &su_scns[sc];
  if (!ivs_ms_new(x, 0x5Au)) {
    return 0;
  }
  ok = ivs_ms_load(x->m, IVS_KIND_EXEC, su_copy(s->exec, s->exec_n), s->exec_n) &&
       ivs_ms_load(x->m, IVS_KIND_GROM, (uint8_t *)calloc(IVS_MS_GROM_BYTES, 1u), IVS_MS_GROM_BYTES);
  if (ok && s->cart != NULL) {
    ok = ivs_ms_load(x->m, IVS_KIND_CARTRIDGE, su_copy(s->cart, s->cart_n), s->cart_n);
  }
  if (ok && power != 0) {
    ok = ivs_machine_reset(x->m, IVS_RESET_POWER_ON) == IVS_OK;
  }
  if (!ok) {
    ivs_ms_free(x);
    return 0;
  }
  return 1;
}

int su_run_to(ivs_machine *m, uint64_t target, const ivs_input_event *ev, uint32_t n, uint32_t *consumed)
{
  ivs_run_request q;
  ivs_run_report r;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = target;
  q.events = n > *consumed ? ev + *consumed : NULL;
  q.event_count = n > *consumed ? n - *consumed : 0u;
  memset(&r, 0, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  if (ivs_machine_run(m, &q, &r) != IVS_OK) {
    return 0;
  }
  *consumed += r.input_consumed;
  return 1;
}

int su_run_unit(ivs_machine *m, const ivs_input_event *ev, uint32_t n, uint32_t *consumed)
{
  const struct ivs_machine *im = m;
  return su_run_to(m, im->time_ticks + 1u, ev, n, consumed);
}
