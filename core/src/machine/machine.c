/* SPDX-License-Identifier: MIT */
/* Machine lifecycle (D-01..D-04, D-08, A-06, plan 03-09): storage requirements, create, destroy
 * and info through the public header ivs/ivs_machine.h. The core never allocates: the host owns
 * the storage and the handle is its address (D-01).
 *
 * create validates everything before its first write to the storage (D-08, D-10), in this order:
 *   BAD_ARG          out inside the instance bytes of the storage (refused before *out is
 *                    written, so the storage stays untouched; plan 03-11); NULL opts, storage or
 *                    out; opts struct_size below the v1 size; profile id NULL or without a NUL in
 *                    its first 64 bytes; storage not 16-byte aligned;
 *   UNSUPPORTED      nonzero flags; a nonzero byte in a larger caller struct's unknown tail;
 *   UNKNOWN_PROFILE  the id is not in the registry (exact, case-sensitive match);
 *   BUFFER_TOO_SMALL storage_bytes below the instance size.
 * Only then does it zero the instance bytes (sizeof the instance; bytes past them belong to the
 * host and are never touched) and set the fields of a new instance. A refusal writes *out = NULL
 * and nothing else. destroy zero-scrubs the same bytes, so the magic check refuses a stale
 * handle with BAD_HANDLE. ivs_status_name and ivs_api_version live here too (D-04, D-09).
 * Profile-id matching is a hand-written loop bounded by 64 bytes (A-06):
 * no strlen or strcmp, which the core may not import. */
#include "common/ivs_prelude.h"

#include "ivs/ivs.h"
#include "machine/machine.h"

_Static_assert(IVS_PROFILE_ID_MAX_BYTES == IVS_MEMSYS_PROFILE_ID_CAP, "profile id bound");
_Static_assert(IVS_SLOT_COUNT == 3u, "three image slots");
_Static_assert(sizeof(ivs_image_report) == 64u, "ivs_image_report is 64 bytes");

const struct ivs_machine *ivs_machine_from_handle(const ivs_machine *m)
{
  if (m == NULL || m->magic != IVS_MACHINE_MAGIC) {
    return NULL;
  }
  return m;
}

struct ivs_machine *ivs_machine_from_handle_mut(ivs_machine *m)
{
  if (m == NULL || m->magic != IVS_MACHINE_MAGIC) {
    return NULL;
  }
  return m;
}

uint32_t ivs_machine_span_overlaps(const void *p, size_t n, const void *inst, size_t inst_bytes)
{
  uintptr_t a = (uintptr_t)p;
  uintptr_t s = (uintptr_t)inst;
  if (n == 0u || inst_bytes == 0u) {
    return 0u;
  }
  if (a >= s) {
    return (uint32_t)(a - s < (uintptr_t)inst_bytes);
  }
  return (uint32_t)(s - a < (uintptr_t)n);
}

ivs_status ivs_machine_profile_id_length(const char *id, uint32_t *len)
{
  uint32_t i;
  if (id == NULL) {
    return IVS_E_BAD_ARG;
  }
  for (i = 0u; i < IVS_PROFILE_ID_MAX_BYTES; i++) {
    if (id[i] == '\0') {
      *len = i;
      return IVS_OK;
    }
  }
  return IVS_E_BAD_ARG;
}

ivs_status ivs_machine_profile_lookup(const char *id, uint32_t *index)
{
  uint32_t len = 0u;
  uint32_t p;
  uint32_t n = ivs_profile_count();
  ivs_status st = ivs_machine_profile_id_length(id, &len);
  if (st != IVS_OK) {
    return st;
  }
  for (p = 0u; p < n; p++) {
    const ivs_profile_desc *d = ivs_profile_get(p);
    uint32_t i;
    uint8_t same = 1u;
    /* Compare len bytes and the NUL; len < 64, so i stays inside both arrays. */
    for (i = 0u; i <= len; i++) {
      if (d->id[i] != id[i]) {
        same = 0u;
        break;
      }
    }
    if (same != 0u) {
      *index = p;
      return IVS_OK;
    }
  }
  return IVS_E_UNKNOWN_PROFILE;
}

/* 1 when every byte of p[from, to) is zero. */
static uint8_t ivs_machine_tail_zero(const uint8_t *p, size_t from, size_t to)
{
  size_t i;
  for (i = from; i < to; i++) {
    if (p[i] != 0u) {
      return 0u;
    }
  }
  return 1u;
}

ivs_status ivs_machine_storage_requirements(const char *profile_id, ivs_storage_requirements *req)
{
  ivs_storage_requirements r;
  uint32_t index = 0u;
  uint32_t len = 0u;
  ivs_status st;
  if (req == NULL || req->struct_size < (uint32_t)sizeof(ivs_storage_requirements)) {
    return IVS_E_BAD_ARG;
  }
  if (ivs_machine_profile_id_length(profile_id, &len) != IVS_OK) {
    return IVS_E_BAD_ARG;
  }
  st = ivs_machine_profile_lookup(profile_id, &index);
  if (st != IVS_OK) {
    return st;
  }
  memset(&r, 0, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  r.size_bytes = (uint64_t)sizeof(struct ivs_machine);
  r.align_bytes = (uint64_t)IVS_MACHINE_ALIGN;
  *req = r;
  return IVS_OK;
}

ivs_status ivs_machine_create(const ivs_create_options *opts, void *storage, size_t storage_bytes,
                              ivs_machine **out)
{
  struct ivs_machine *im;
  uint32_t index = 0u;
  uint32_t len = 0u;
  uint32_t i;
  size_t clear;
  size_t span = storage_bytes < sizeof(struct ivs_machine) ? storage_bytes : sizeof(struct ivs_machine);
  ivs_status st;
  /* BAD_ARG: an out pointer inside the bytes create may write is refused before *out is set. */
  if (out != NULL && storage != NULL && ivs_machine_span_overlaps(out, sizeof *out, storage, span) != 0u) {
    return IVS_E_BAD_ARG;
  }
  if (out != NULL) {
    *out = NULL;
  }
  /* BAD_ARG */
  if (opts == NULL || storage == NULL || out == NULL) {
    return IVS_E_BAD_ARG;
  }
  if (opts->struct_size < (uint32_t)sizeof(ivs_create_options)) {
    return IVS_E_BAD_ARG;
  }
  if (ivs_machine_profile_id_length(opts->profile_id, &len) != IVS_OK) {
    return IVS_E_BAD_ARG;
  }
  if (((uintptr_t)storage % (uintptr_t)IVS_MACHINE_ALIGN) != 0u) {
    return IVS_E_BAD_ARG;
  }
  /* UNSUPPORTED */
  if (opts->flags != 0u) {
    return IVS_E_UNSUPPORTED;
  }
  if (ivs_machine_tail_zero((const uint8_t *)opts, sizeof(ivs_create_options), (size_t)opts->struct_size) == 0u) {
    return IVS_E_UNSUPPORTED;
  }
  /* UNKNOWN_PROFILE */
#ifdef IVS_FAULT_CREATE_MAGIC_BEFORE_PROFILE_CHECK
  ((struct ivs_machine *)storage)->magic = IVS_MACHINE_MAGIC;
#endif
  st = ivs_machine_profile_lookup(opts->profile_id, &index);
  if (st != IVS_OK) {
    return st;
  }
  /* BUFFER_TOO_SMALL */
  if (storage_bytes < sizeof(struct ivs_machine)) {
    return IVS_E_BUFFER_TOO_SMALL;
  }
  /* Commit: every instance byte is written. */
  im = (struct ivs_machine *)storage;
  clear = sizeof *im;
#ifdef IVS_FAULT_INIT_SKIPS_MEMSET
  clear = 0u;
#endif
  memset(im, 0, clear);
  im->magic = IVS_MACHINE_MAGIC;
  im->profile = index;
  im->phase = IVS_PHASE_OFF;
  im->time_ticks = 0u;
  (void)ivs_memsys_init(&im->mem, index);
  for (i = 0u; i < IVS_SLOT_COUNT; i++) {
    im->slot[i].kind = i + 1u;
  }
  *out = im;
  return IVS_OK;
}

ivs_status ivs_machine_destroy(ivs_machine *m)
{
  struct ivs_machine *im = ivs_machine_from_handle_mut(m);
  size_t scrub;
  if (im == NULL) {
    return IVS_E_BAD_HANDLE;
  }
  scrub = sizeof *im;
#ifdef IVS_FAULT_DESTROY_NO_SCRUB
  scrub = 0u;
#endif
  memset(im, 0, scrub);
  return IVS_OK;
}

ivs_status ivs_machine_info(const ivs_machine *m, ivs_info *out)
{
  const struct ivs_machine *im;
  ivs_info r;
  uint32_t i;
  if (out == NULL || out->struct_size < (uint32_t)sizeof(ivs_info) ||
      ivs_machine_span_overlaps(out, sizeof *out, m, sizeof(struct ivs_machine)) != 0u) {
    return IVS_E_BAD_ARG;
  }
  im = ivs_machine_from_handle(m);
  if (im == NULL) {
    return IVS_E_BAD_HANDLE;
  }
  memset(&r, 0, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  r.phase = im->phase;
  r.time_ticks = im->time_ticks;
  r.profile_index = im->profile;
  for (i = 0u; i < IVS_SLOT_COUNT; i++) {
    ivs_machine_slot_report(&im->slot[i], &r.slots[i]);
  }
  *out = r;
  return IVS_OK;
}

void ivs_machine_slot_report(const ivs_machine_slot *s, ivs_image_report *o)
{
  memset(o, 0, sizeof *o);
  o->struct_size = (uint32_t)sizeof *o;
  o->kind = s->kind;
  o->loaded = s->loaded;
  o->length_bytes = s->length_bytes;
  o->length_units = s->length_units;
  o->unit_bits = s->unit_bits;
  o->base_addr = s->base_addr;
  memcpy(o->sha256, s->sha256, sizeof o->sha256);
}

uint32_t ivs_api_version(void)
{
  return IVS_API_VERSION;
}

const char *ivs_status_name(ivs_status s)
{
  switch (s) {
  case IVS_OK:
    return "IVS_OK";
  case IVS_E_BAD_ARG:
    return "IVS_E_BAD_ARG";
  case IVS_E_BAD_STATE:
    return "IVS_E_BAD_STATE";
  case IVS_E_BAD_HANDLE:
    return "IVS_E_BAD_HANDLE";
  case IVS_E_UNSUPPORTED:
    return "IVS_E_UNSUPPORTED";
  case IVS_E_BUFFER_TOO_SMALL:
    return "IVS_E_BUFFER_TOO_SMALL";
  case IVS_E_UNKNOWN_PROFILE:
    return "IVS_E_UNKNOWN_PROFILE";
  case IVS_E_MALFORMED:
    return "IVS_E_MALFORMED";
  case IVS_E_OVERSIZED:
    return "IVS_E_OVERSIZED";
  case IVS_E_OVERLAP:
    return "IVS_E_OVERLAP";
  case IVS_E_OUT_OF_RANGE:
    return "IVS_E_OUT_OF_RANGE";
  case IVS_E_MISSING_SYSTEM_IMAGE:
    return "IVS_E_MISSING_SYSTEM_IMAGE";
  case IVS_E_DUPLICATE_LOAD:
    return "IVS_E_DUPLICATE_LOAD";
  case IVS_E_DIGEST_MISMATCH:
    return "IVS_E_DIGEST_MISMATCH";
  default:
    return "IVS_E_UNKNOWN";
  }
}
