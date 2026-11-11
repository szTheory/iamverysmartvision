/* SPDX-License-Identifier: MIT */
/* Typed content loading (D-10, D-11, D-13, D-24..D-27, A-06, A-08, plan 03-10): ivs_machine_load
 * and ivs_machine_unload through the public header ivs/ivs_machine.h.
 *
 * Validate-then-commit (D-10). ivs_load_validate takes a const instance and fills a small stack
 * plan (kind, lengths, units, base, the reported digest; never a copy of image bytes). The ladder
 * is one stage per class, the first failing stage wins (D-11 refined by A-08 a, b):
 *   1. BAD_ARG         argument values: kind, NULL bytes with len above 0, opts struct size
 *                      (below the v1 size or above IVS_STRUCT_SIZE_MAX, which bounds stage 3)
 *                      (a short report struct, or a report that overlaps the storage of a
 *                      valid instance, is refused by the public function, writing nothing;
 *                      plan 03-11; the overlap is not tested for a NULL or invalid handle);
 *   2. BAD_HANDLE      NULL or invalid handle;
 *   3. UNSUPPORTED     load options: an unknown flag, a nonzero tail of a larger struct;
 *   4. BAD_STATE       the machine is POWERED;
 *   5. DUPLICATE_LOAD  the slot holds an image;
 *   6. OVERSIZED       the claimed length against the kind's bound, compared in uint64_t before
 *                      any byte is read;
 *   7. BAD_ARG         the input span overlaps the instance storage, tested only on a span that
 *                      passed stage 6, with differences of uintptr_t values that cannot wrap
 *                      (never bytes + len), so a refusal status never depends on where the host
 *                      placed a buffer whose claimed length is out of bound (A-06, A-08 a);
 *   8. content         EXEC and GROM: MALFORMED for a short image (zero length included), then
 *                      for EXEC the first word with a bit of 15..10 set (its byte offset); every
 *                      GROM byte is in range; nothing is truncated or masked (D-26).
 *                      Cartridge: the IVSC v1 layer-1 parser in its spec order (D-33), then
 *                      layer 2: every segment wholly inside the profile's cartridge span (else
 *                      OUT_OF_RANGE with the segment; any start is allowed, A-01), then
 *                      required_features within cartridge.features_supported (else UNSUPPORTED
 *                      with the missing bits, D-32). Where a container has defects in two
 *                      stages, the earlier stage wins (A-08 b).
 *   9. DIGEST_MISMATCH expect_sha256 against the digest of the supplied bytes (D-27).
 * The reported digest is the SHA-256 of the exact supplied bytes (D-25). For a cartridge it is
 * the whole file, trailer included, and it comes from the parser's single pass (A-04), never
 * from the file's own trailer.
 * ivs_load_commit returns void, is copy-only and has no early return: it installs the words
 * through the memsys install primitives and writes the slot record. */
#include "common/ivs_prelude.h"

#include "common/ivs_sha256.h"
#include "ivs/ivs.h"
#include "machine/machine.h"

_Static_assert(IVS_KIND_EXEC == IVS_MEMSYS_SLOT_EXEC && IVS_KIND_GROM == IVS_MEMSYS_SLOT_GROM &&
                   IVS_KIND_CARTRIDGE == IVS_MEMSYS_SLOT_CART,
               "image kinds and memsys slots share their numbers");
_Static_assert(sizeof(ivs_load_options) == 40u, "ivs_load_options is 40 bytes");
_Static_assert(sizeof(ivs_load_report) == 88u, "ivs_load_report is 88 bytes");

/* What a validated load will commit. */
typedef struct ivs_load_plan {
  uint32_t kind;
  uint32_t length_bytes;
  uint32_t length_units;
  uint32_t unit_bits;
  uint32_t base_addr;
  uint8_t sha256[IVS_SHA256_DIGEST_BYTES]; /* of the exact supplied bytes */
  ivs_ivsc_view view;                      /* CARTRIDGE: the accepted container's segments */
} ivs_load_plan;

/* Fill the refusal detail and return the status. */
static ivs_status ivs_load_refuse(ivs_load_report *d, ivs_status st, uint32_t reason, uint32_t seg, uint64_t off,
                                  uint32_t missing)
{
  d->reason = reason;
  d->segment_index = seg;
  d->byte_offset = off;
  d->missing_features = missing;
  return st;
}

/* 1 when every byte of p[from, to) is zero. */
static uint32_t ivs_load_tail_zero(const uint8_t *p, size_t from, size_t to)
{
  size_t i;
  for (i = from; i < to; i++) {
    if (p[i] != 0u) {
      return 0u;
    }
  }
  return 1u;
}

/* The kind's length bound in bytes (stage 6). */
static uint64_t ivs_load_bound(uint32_t kind)
{
  switch (kind) {
  case IVS_KIND_EXEC:
    return IVS_MEMSYS_EXEC_BYTES;
  case IVS_KIND_GROM:
    return IVS_MEMSYS_GROM_BYTES;
  default:
    return IVS_IVSC_MAX_BYTES;
  }
}

uint32_t ivs_machine_cart_span_check(uint32_t lo, uint32_t hi, const ivs_ivsc_view *v)
{
  uint64_t end_limit = (uint64_t)hi + 1u; /* first word past the span */
  uint32_t i;
#ifdef IVS_FAULT_CART_SPAN_END_INCLUSIVE
  end_limit += 1u; /* a segment ending one word past the span is accepted */
#endif
  for (i = 0u; i < v->segment_count && i < IVS_IVSC_MAX_SEGMENTS; i++) {
    uint64_t start = v->segs[i].addr;
    uint64_t end = start + (uint64_t)v->segs[i].len_words;
    if (start < (uint64_t)lo || end > end_limit) {
      return i;
    }
  }
  return IVS_SEGMENT_NONE;
}

/* Stage 8 for a system image (EXEC or GROM): exact size, then the word width. */
static ivs_status ivs_load_system(const ivs_profile_desc *p, uint32_t kind, const uint8_t *bytes, size_t len,
                                  ivs_load_plan *plan, ivs_load_report *d)
{
  const ivs_slot_desc *sd = kind == IVS_KIND_EXEC ? &p->exec : &p->grom;
  uint64_t n = (uint64_t)len;
  uint32_t i;
  uint8_t wide = 0xFCu; /* bits 15..10 of a high-byte-first word */
#ifdef IVS_FAULT_ROM_DECLE_MASKED_NOT_REFUSED
  wide = 0x00u; /* a wide word is accepted and left to the 10-bit read mask, not refused */
#endif
  if (n < ivs_load_bound(kind)) {
    return ivs_load_refuse(d, IVS_E_MALFORMED, IVS_REASON_IMAGE_SHORT, IVS_SEGMENT_NONE, n, 0u);
  }
  if (kind == IVS_KIND_EXEC) {
    for (i = 0u; i < IVS_MEMSYS_EXEC_WORDS; i++) {
      if ((bytes[2u * i] & wide) != 0u) {
        return ivs_load_refuse(d, IVS_E_MALFORMED, IVS_REASON_IMAGE_WORD, IVS_SEGMENT_NONE, 2u * (uint64_t)i, 0u);
      }
    }
  }
  ivs_sha256_digest(bytes, len, plan->sha256);
  plan->length_units = sd->units;
  plan->unit_bits = sd->unit_bits;
  plan->base_addr = sd->base;
  return IVS_OK;
}

/* Stage 8 for a cartridge: layer 1 (the parser), then layer 2 (span, then features). */
static ivs_status ivs_load_cart(const struct ivs_machine *im, const ivs_profile_desc *p, const uint8_t *bytes,
                                size_t len, ivs_load_plan *plan, ivs_load_report *d)
{
  ivs_ivsc_report irep;
  ivs_status st;
  uint32_t seg;
  uint32_t missing;
  uint32_t units = 0u;
  uint32_t i;
  memset(&irep, 0, sizeof irep);
  st = ivs_ivsc_parse(bytes, len, &plan->view, &irep);
  if (st != IVS_OK) {
    return ivs_load_refuse(d, st, irep.reason, irep.segment_index, irep.byte_offset, 0u);
  }
#ifdef IVS_FAULT_CART_COPY_BEFORE_VALIDATE
  /* the first segment is installed before layer 2 has accepted the container */
  ivs_memsys_install_cart_segment(&((struct ivs_machine *)(uintptr_t)im)->mem, plan->view.segs[0].addr,
                                  bytes + plan->view.segs[0].data_byte_offset, plan->view.segs[0].len_words);
#endif
  (void)im;
  seg = p->has_cart_span != 0u ? ivs_machine_cart_span_check(p->cart_lo, p->cart_hi, &plan->view) : 0u;
  if (seg != IVS_SEGMENT_NONE) {
    return ivs_load_refuse(d, IVS_E_OUT_OF_RANGE, IVS_REASON_OUT_OF_SPAN, seg,
                           IVS_IVSC_PREFIX_BYTES + (uint64_t)seg * IVS_IVSC_ENTRY_BYTES, 0u);
  }
  missing = plan->view.required_features & ~p->features_supported;
  if (missing != 0u) {
    return ivs_load_refuse(d, IVS_E_UNSUPPORTED, IVS_REASON_FEATURES, IVS_SEGMENT_NONE, IVS_IVSC_FEATURES_OFFSET,
                           missing);
  }
  for (i = 0u; i < plan->view.segment_count; i++) {
    units += plan->view.segs[i].len_words;
  }
  memcpy(plan->sha256, plan->view.file_sha256, sizeof plan->sha256);
#ifdef IVS_FAULT_CART_REPORT_TRAILER_DIGEST
  memcpy(plan->sha256, bytes + len - IVS_IVSC_TRAILER_BYTES, sizeof plan->sha256); /* the trailer */
#endif
  plan->length_units = units;
  plan->unit_bits = IVS_IVSC_WIDTH_BITS;
  plan->base_addr = plan->view.segs[0].addr;
  return IVS_OK;
}

static ivs_status ivs_load_validate(const ivs_machine *m, uint32_t kind, const uint8_t *bytes, size_t len,
                                    const ivs_load_options *opts, ivs_load_plan *plan, ivs_load_report *d)
{
  const struct ivs_machine *im;
  const ivs_profile_desc *p;
  uint64_t n = (uint64_t)len;
  uint64_t bound;
  ivs_status st;
  uint32_t occupied;
  uint32_t powered;
  uint32_t over;
  uint32_t i;

  /* 1. BAD_ARG: argument values. */
  if (kind < IVS_KIND_EXEC || kind > IVS_KIND_CARTRIDGE) {
    return ivs_load_refuse(d, IVS_E_BAD_ARG, IVS_REASON_ARG_KIND, IVS_SEGMENT_NONE, 0u, 0u);
  }
  if (bytes == NULL && len != 0u) {
    return ivs_load_refuse(d, IVS_E_BAD_ARG, IVS_REASON_ARG_NULL_BYTES, IVS_SEGMENT_NONE, 0u, 0u);
  }
  if (opts != NULL && opts->struct_size < (uint32_t)sizeof(ivs_load_options)) {
    return ivs_load_refuse(d, IVS_E_BAD_ARG, IVS_REASON_ARG_STRUCT_SIZE, IVS_SEGMENT_NONE, 0u, 0u);
  }
  /* The cap bounds the stage-3 tail scan: a garbage struct_size is refused, never followed (WR-02). */
  over = (uint32_t)(opts != NULL && opts->struct_size > IVS_STRUCT_SIZE_MAX);
#ifdef IVS_FAULT_STRUCT_SIZE_UNCAPPED
  over = 0u; /* any struct_size is followed by the tail scan */
#endif
  if (over != 0u) {
    return ivs_load_refuse(d, IVS_E_BAD_ARG, IVS_REASON_ARG_STRUCT_SIZE, IVS_SEGMENT_NONE, 0u, 0u);
  }
  /* 2. BAD_HANDLE. */
  im = ivs_machine_from_handle(m);
  if (im == NULL) {
    return ivs_load_refuse(d, IVS_E_BAD_HANDLE, IVS_REASON_HANDLE, IVS_SEGMENT_NONE, 0u, 0u);
  }
  p = ivs_profile_get(im->profile);
  /* 3. UNSUPPORTED: load options. */
  if (opts != NULL) {
    if ((opts->flags & ~IVS_LOAD_F_EXPECT_SHA256) != 0u) {
      return ivs_load_refuse(d, IVS_E_UNSUPPORTED, IVS_REASON_OPTION_FLAGS, IVS_SEGMENT_NONE, 0u, 0u);
    }
    if (ivs_load_tail_zero((const uint8_t *)opts, sizeof(ivs_load_options), (size_t)opts->struct_size) == 0u) {
      return ivs_load_refuse(d, IVS_E_UNSUPPORTED, IVS_REASON_OPTION_TAIL, IVS_SEGMENT_NONE, 0u, 0u);
    }
  }
  /* 4. BAD_STATE. */
  powered = (uint32_t)(im->phase != IVS_PHASE_OFF);
#ifdef IVS_FAULT_STATE_CHECK_OMITTED
  powered = 0u; /* a load while POWERED goes on to the later stages */
#endif
  if (powered != 0u) {
    return ivs_load_refuse(d, IVS_E_BAD_STATE, IVS_REASON_STATE_POWERED, IVS_SEGMENT_NONE, 0u, 0u);
  }
  /* 5. DUPLICATE_LOAD. */
  occupied = im->slot[kind - 1u].loaded;
#ifdef IVS_FAULT_DUPLICATE_REPLACES
  occupied = 0u; /* an occupied slot is validated and overwritten by the commit */
#endif
  if (occupied != 0u) {
    return ivs_load_refuse(d, IVS_E_DUPLICATE_LOAD, IVS_REASON_SLOT_OCCUPIED, IVS_SEGMENT_NONE, 0u, 0u);
  }
  bound = ivs_load_bound(kind);
#ifdef IVS_FAULT_CHECK_ORDER_MALFORMED_FIRST
  if (kind != IVS_KIND_CARTRIDGE && n != bound) { /* a wrong size is refused as a shape fault first */
    return ivs_load_refuse(d, IVS_E_MALFORMED, IVS_REASON_IMAGE_SHORT, IVS_SEGMENT_NONE, n, 0u);
  }
#endif
#ifdef IVS_FAULT_OVERLAP_BEFORE_LENGTH
  if (ivs_machine_span_overlaps(bytes, len, im, sizeof *im) != 0u) { /* on the unbounded span */
    return ivs_load_refuse(d, IVS_E_BAD_ARG, IVS_REASON_ARG_INPUT_OVERLAP, IVS_SEGMENT_NONE, 0u, 0u);
  }
#endif
  /* 6. OVERSIZED: the claimed length, before any byte is read. */
  if (n > bound) {
    return ivs_load_refuse(d, IVS_E_OVERSIZED, kind == IVS_KIND_CARTRIDGE ? IVS_REASON_TOO_LARGE : IVS_REASON_IMAGE_LONG,
                           IVS_SEGMENT_NONE, bound, 0u);
  }
  /* 7. BAD_ARG: the bounded input span against the instance storage. */
  if (ivs_machine_span_overlaps(bytes, len, im, sizeof *im) != 0u) {
    return ivs_load_refuse(d, IVS_E_BAD_ARG, IVS_REASON_ARG_INPUT_OVERLAP, IVS_SEGMENT_NONE, 0u, 0u);
  }
  /* 8. Content. */
  st = kind == IVS_KIND_CARTRIDGE ? ivs_load_cart(im, p, bytes, len, plan, d)
                                  : ivs_load_system(p, kind, bytes, len, plan, d);
  if (st != IVS_OK) {
    return st;
  }
  /* 9. DIGEST_MISMATCH: expect_sha256, compared over all 32 bytes. */
  if (opts != NULL && (opts->flags & IVS_LOAD_F_EXPECT_SHA256) != 0u) {
    uint32_t diff = 0u;
    for (i = 0u; i < IVS_SHA256_DIGEST_BYTES; i++) {
      diff |= (uint32_t)(opts->expect_sha256[i] ^ plan->sha256[i]);
    }
    if (diff != 0u) {
      return ivs_load_refuse(d, IVS_E_DIGEST_MISMATCH, IVS_REASON_EXPECT_DIGEST, IVS_SEGMENT_NONE, 0u, 0u);
    }
  }
  plan->kind = kind;
  plan->length_bytes = (uint32_t)n;
  return IVS_OK;
}

/* Copy-only commit of a validated plan: no check, no early return. */
static void ivs_load_commit(struct ivs_machine *im, const ivs_load_plan *plan, const uint8_t *bytes)
{
  ivs_machine_slot *s = &im->slot[plan->kind - 1u];
  uint32_t i;
  switch (plan->kind) {
  case IVS_KIND_EXEC:
    ivs_memsys_install_exec(&im->mem, bytes);
    break;
  case IVS_KIND_GROM:
    ivs_memsys_install_grom(&im->mem, bytes);
    break;
  default:
    for (i = 0u; i < plan->view.segment_count; i++) {
      const ivs_ivsc_segment *g = &plan->view.segs[i];
      ivs_memsys_install_cart_segment(&im->mem, g->addr, bytes + g->data_byte_offset, g->len_words);
    }
    break;
  }
  s->loaded = 1u;
  s->length_bytes = plan->length_bytes;
  s->length_units = plan->length_units;
  s->unit_bits = plan->unit_bits;
  s->base_addr = plan->base_addr;
  memcpy(s->sha256, plan->sha256, sizeof s->sha256);
}

ivs_status ivs_machine_load(ivs_machine *m, uint32_t kind, const uint8_t *bytes, size_t len,
                            const ivs_load_options *opts, ivs_load_report *report)
{
  ivs_load_plan plan;
  ivs_load_report r;
  ivs_status st;
  if (report != NULL) {
    /* A report aimed into the instance would be written over it after the commit. Only a valid
     * instance has bytes to overlap: with a NULL or invalid handle the ladder below answers
     * BAD_HANDLE (or an earlier BAD_ARG) wherever the report lies (A-08 a). */
    const struct ivs_machine *live = ivs_machine_from_handle(m);
    uint32_t aliased = live != NULL ? ivs_machine_span_overlaps(report, sizeof *report, live, sizeof *live) : 0u;
#ifdef IVS_FAULT_REPORT_INSIDE_UNCHECKED
    aliased = 0u; /* a report inside the instance is accepted and written over it */
#endif
    if (aliased != 0u || report->struct_size < (uint32_t)sizeof(ivs_load_report)) {
      return IVS_E_BAD_ARG;
    }
  }
  memset(&plan, 0, sizeof plan);
  memset(&r, 0, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  r.segment_index = IVS_SEGMENT_NONE;
#ifdef IVS_FAULT_LEN_TRUNCATED_32
  len = (size_t)(uint32_t)len; /* the length narrowed to 32 bits before the size check */
#endif
#ifdef IVS_FAULT_LOAD_LEN_BEFORE_CHECK
  if (ivs_machine_from_handle_mut(m) != NULL && kind >= IVS_KIND_EXEC && kind <= IVS_KIND_CARTRIDGE) {
    ivs_machine_from_handle_mut(m)->slot[kind - 1u].length_bytes = (uint32_t)len; /* recorded before checks */
  }
#endif
#ifdef IVS_FAULT_LOAD_TABLE_BEFORE_VALIDATE
  if (ivs_machine_from_handle_mut(m) != NULL && kind == IVS_KIND_EXEC) {
    ivs_machine_from_handle_mut(m)->mem.exec_loaded = 1u; /* the decode table marks the slot loaded first */
  } else if (ivs_machine_from_handle_mut(m) != NULL && kind == IVS_KIND_GROM) {
    ivs_machine_from_handle_mut(m)->mem.grom_loaded = 1u;
  }
#endif
  st = ivs_load_validate(m, kind, bytes, len, opts, &plan, &r);
  if (st == IVS_OK) {
    struct ivs_machine *im = ivs_machine_from_handle_mut(m);
    ivs_load_commit(im, &plan, bytes);
    ivs_machine_slot_report(&im->slot[kind - 1u], &r.image);
  }
  if (report != NULL) {
    *report = r;
  }
  return st;
}

ivs_status ivs_machine_unload(ivs_machine *m, uint32_t kind)
{
  struct ivs_machine *im;
  ivs_machine_slot *s;
  uint32_t mem_slot;
  if (kind < IVS_KIND_EXEC || kind > IVS_KIND_CARTRIDGE) {
    return IVS_E_BAD_ARG;
  }
  im = ivs_machine_from_handle_mut(m);
  if (im == NULL) {
    return IVS_E_BAD_HANDLE;
  }
  if (im->phase != IVS_PHASE_OFF) {
    return IVS_E_BAD_STATE;
  }
  s = &im->slot[kind - 1u];
  if (s->loaded == 0u) {
    return IVS_OK;
  }
  memset(s, 0, sizeof *s);
  s->kind = kind;
  mem_slot = kind;
#ifdef IVS_FAULT_UNLOAD_KEEPS_IMAGE
  mem_slot = 0u; /* the memsys keeps the slot's words and loaded flag: only the slot record clears */
#endif
  ivs_memsys_clear_slot(&im->mem, mem_slot);
  return IVS_OK;
}
