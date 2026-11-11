/* SPDX-License-Identifier: MIT */
/* Canonical machine state digest (D-35, plan 03-09). One field table over struct ivs_machine
 * drives the serializer; it is never a memcpy of a struct, so the value depends on the field
 * values only, not on layout, padding or host byte order (machine.h states the framing).
 * Domains: 1 CPU (registers, status and flags of the embedded CPU; its reserved bytes are not
 * serialized), 2 MEM (the profile the decode uses, the gate flags and every backing array),
 * 3 IMAGES (loaded flags, the committed cartridge segment count and table, the three slot
 * records), 4 INSTANCE (profile id, run phase, machine time). The composite hashes the four
 * domain digests under a version with the domain ids ("IVSSTATE"); RUN-05 video, audio and input
 * domains (16..18) reuse the framing later and are refused here.
 * Output is SHA-256 (core/src/common/ivs_sha256.c). Serialized bytes pass through a small stage
 * buffer that feeds the hash and, for the test seam ivs_machine_digest_preimage, a caller buffer
 * (no function pointer). */
#include "common/ivs_prelude.h"

#include "common/ivs_sha256.h"
#include "machine/machine.h"

#define IVS_DF(name, member, width, count, domain, kind)                                               \
  {name, (uint32_t)offsetof(struct ivs_machine, member), width, count, domain, kind, 0u}

#define IVS_DF_SLOT(i, label)                                                                           \
  IVS_DF("slot." label ".kind", slot[i].kind, 4u, 1u, IVS_DIGEST_IMAGES, IVS_DIGEST_KIND_UINT),            \
      IVS_DF("slot." label ".loaded", slot[i].loaded, 4u, 1u, IVS_DIGEST_IMAGES, IVS_DIGEST_KIND_UINT),    \
      IVS_DF("slot." label ".length_bytes", slot[i].length_bytes, 4u, 1u, IVS_DIGEST_IMAGES,               \
             IVS_DIGEST_KIND_UINT),                                                                     \
      IVS_DF("slot." label ".length_units", slot[i].length_units, 4u, 1u, IVS_DIGEST_IMAGES,               \
             IVS_DIGEST_KIND_UINT),                                                                     \
      IVS_DF("slot." label ".unit_bits", slot[i].unit_bits, 4u, 1u, IVS_DIGEST_IMAGES,                     \
             IVS_DIGEST_KIND_UINT),                                                                     \
      IVS_DF("slot." label ".base_addr", slot[i].base_addr, 4u, 1u, IVS_DIGEST_IMAGES,                     \
             IVS_DIGEST_KIND_UINT),                                                                     \
      IVS_DF("slot." label ".sha256", slot[i].sha256, 1u, 32u, IVS_DIGEST_IMAGES, IVS_DIGEST_KIND_UINT)

/* Bytes after the last slot up to sizeof (the 16-byte alignment's tail padding). */
#define IVS_MACHINE_SLOTS_END (offsetof(struct ivs_machine, slot) + sizeof(ivs_machine_slot) * IVS_SLOT_COUNT)

/* In instance offset order. */
static const ivs_digest_field ivs_digest_fields[] = {
    IVS_DF("magic", magic, 4u, 1u, IVS_DIGEST_INSTANCE, IVS_DIGEST_KIND_MAGIC),
    IVS_DF("profile", profile, 4u, 1u, IVS_DIGEST_INSTANCE, IVS_DIGEST_KIND_PROFILE),
    IVS_DF("phase", phase, 4u, 1u, IVS_DIGEST_INSTANCE, IVS_DIGEST_KIND_UINT),
    IVS_DF("reserved", reserved, 4u, 1u, IVS_DIGEST_INSTANCE, IVS_DIGEST_KIND_PAD),
    IVS_DF("time_ticks", time_ticks, 8u, 1u, IVS_DIGEST_INSTANCE, IVS_DIGEST_KIND_UINT),
    IVS_DF("cpu.r", cpu.r, 2u, 8u, IVS_DIGEST_CPU, IVS_DIGEST_KIND_UINT),
    IVS_DF("cpu.status", cpu.status, 1u, 1u, IVS_DIGEST_CPU, IVS_DIGEST_KIND_UINT),
    IVS_DF("cpu.intff", cpu.intff, 1u, 1u, IVS_DIGEST_CPU, IVS_DIGEST_KIND_UINT),
    IVS_DF("cpu.dbd", cpu.dbd, 1u, 1u, IVS_DIGEST_CPU, IVS_DIGEST_KIND_UINT),
    IVS_DF("cpu.prev_interruptible", cpu.prev_interruptible, 1u, 1u, IVS_DIGEST_CPU, IVS_DIGEST_KIND_UINT),
    IVS_DF("cpu.run_state", cpu.run_state, 1u, 1u, IVS_DIGEST_CPU, IVS_DIGEST_KIND_UINT),
    IVS_DF("cpu.halt_pending", cpu.halt_pending, 1u, 1u, IVS_DIGEST_CPU, IVS_DIGEST_KIND_UINT),
    IVS_DF("cpu.fault_reason", cpu.fault_reason, 1u, 1u, IVS_DIGEST_CPU, IVS_DIGEST_KIND_UINT),
    IVS_DF("cpu.busak", cpu.busak, 1u, 1u, IVS_DIGEST_CPU, IVS_DIGEST_KIND_UINT),
    IVS_DF("cpu.reserved", cpu.reserved, 1u, 8u, IVS_DIGEST_CPU, IVS_DIGEST_KIND_PAD),
    IVS_DF("mem.profile", mem.profile, 4u, 1u, IVS_DIGEST_MEM, IVS_DIGEST_KIND_PROFILE),
    IVS_DF("mem.gate_open", mem.gate_open, 1u, 4u, IVS_DIGEST_MEM, IVS_DIGEST_KIND_UINT),
    IVS_DF("mem.exec_loaded", mem.exec_loaded, 1u, 1u, IVS_DIGEST_IMAGES, IVS_DIGEST_KIND_UINT),
    IVS_DF("mem.grom_loaded", mem.grom_loaded, 1u, 1u, IVS_DIGEST_IMAGES, IVS_DIGEST_KIND_UINT),
    IVS_DF("mem.cart_count", mem.cart_count, 2u, 1u, IVS_DIGEST_IMAGES, IVS_DIGEST_KIND_UINT),
    IVS_DF("mem.cart_seg", mem.cart_seg, 2u, 2u * IVS_MEMSYS_CART_SEGMENTS, IVS_DIGEST_IMAGES, IVS_DIGEST_KIND_UINT),
    IVS_DF("mem.sysram", mem.sysram, 2u, IVS_MEMSYS_SYSRAM_WORDS, IVS_DIGEST_MEM, IVS_DIGEST_KIND_UINT),
    IVS_DF("mem.scratchpad", mem.scratchpad, 2u, IVS_MEMSYS_SCRATCHPAD_WORDS, IVS_DIGEST_MEM, IVS_DIGEST_KIND_UINT),
    IVS_DF("mem.gram", mem.gram, 2u, IVS_MEMSYS_GRAM_WORDS, IVS_DIGEST_MEM, IVS_DIGEST_KIND_UINT),
    IVS_DF("mem.exec", mem.exec, 2u, IVS_MEMSYS_EXEC_WORDS, IVS_DIGEST_MEM, IVS_DIGEST_KIND_UINT),
    IVS_DF("mem.grom", mem.grom, 2u, IVS_MEMSYS_GROM_WORDS, IVS_DIGEST_MEM, IVS_DIGEST_KIND_UINT),
    IVS_DF("mem.cart", mem.cart, 2u, IVS_MEMSYS_CART_WORDS, IVS_DIGEST_MEM, IVS_DIGEST_KIND_UINT),
    IVS_DF_SLOT(0, "exec"),
    IVS_DF_SLOT(1, "grom"),
    IVS_DF_SLOT(2, "cartridge"),
    {"tail_padding", (uint32_t)IVS_MACHINE_SLOTS_END, 1u, (uint32_t)(sizeof(struct ivs_machine) - IVS_MACHINE_SLOTS_END),
     IVS_DIGEST_IMAGES, IVS_DIGEST_KIND_PAD, 0u},
};

#define IVS_DIGEST_FIELD_COUNT (sizeof ivs_digest_fields / sizeof ivs_digest_fields[0])

size_t ivs_machine_digest_field_count(void)
{
  return IVS_DIGEST_FIELD_COUNT;
}

const ivs_digest_field *ivs_machine_digest_field(size_t i)
{
  if (i >= IVS_DIGEST_FIELD_COUNT) {
    return NULL;
  }
  return &ivs_digest_fields[i];
}

/* The sink: a stage buffer flushed to the hash (hash != 0) and to a caller buffer (buf). */
#define IVS_DSINK_STAGE 256u

typedef struct ivs_dsink {
  ivs_sha256 sha;
  uint8_t *buf;
  size_t cap;
  size_t flushed; /* bytes flushed so far */
  uint32_t used;  /* bytes in stage */
  uint8_t hash;
  uint8_t stage[IVS_DSINK_STAGE];
} ivs_dsink;

static void ivs_dsink_flush(ivs_dsink *k)
{
  if (k->hash != 0u) {
    ivs_sha256_update(&k->sha, k->stage, k->used);
  }
  if (k->buf != NULL && k->flushed < k->cap) {
    size_t n = k->cap - k->flushed;
    if (n > k->used) {
      n = k->used;
    }
    memcpy(k->buf + k->flushed, k->stage, n);
  }
  k->flushed += k->used;
  k->used = 0u;
}

static void ivs_dsink_byte(ivs_dsink *k, uint8_t b)
{
  if (k->used == IVS_DSINK_STAGE) {
    ivs_dsink_flush(k);
  }
  k->stage[k->used] = b;
  k->used++;
}

/* v as width bytes, most significant first (D-35). */
static void ivs_dsink_uint(ivs_dsink *k, uint64_t v, uint32_t width)
{
  uint32_t i;
  uint32_t sh;
  for (i = 0u; i < width; i++) {
    sh = 8u * (width - 1u - i);
#ifdef IVS_FAULT_DIGEST_LITTLE_ENDIAN
    sh = 8u * i;
#endif
    ivs_dsink_byte(k, (uint8_t)(v >> sh));
  }
}

static void ivs_dsink_bytes(ivs_dsink *k, const uint8_t *p, size_t n)
{
  size_t i;
  for (i = 0; i < n; i++) {
    ivs_dsink_byte(k, p[i]);
  }
}

/* Element e of a UINT row, read natively (memcpy of one scalar, alias-safe). */
static uint64_t ivs_digest_elem(const uint8_t *p, uint32_t width)
{
  uint8_t u8v;
  uint16_t u16v;
  uint32_t u32v;
  uint64_t u64v;
  switch (width) {
  case 1u:
    memcpy(&u8v, p, 1u);
    return u8v;
  case 2u:
    memcpy(&u16v, p, 2u);
    return u16v;
  case 4u:
    memcpy(&u32v, p, 4u);
    return u32v;
  default:
    memcpy(&u64v, p, 8u);
    return u64v;
  }
}

static void ivs_digest_profile(ivs_dsink *k, uint32_t index)
{
  const ivs_profile_desc *d = ivs_profile_get(index);
  uint32_t len = 0u;
  if (d == NULL || ivs_machine_profile_id_length(d->id, &len) != IVS_OK) {
    ivs_dsink_uint(k, 0xFFFFFFFFu, 4u);
    ivs_dsink_uint(k, index, 4u);
    return;
  }
  ivs_dsink_uint(k, len, 4u);
  ivs_dsink_bytes(k, (const uint8_t *)d->id, len);
}

/* The preimage of one domain (1..4) into k. */
static void ivs_digest_domain(const struct ivs_machine *m, uint32_t domain, ivs_dsink *k)
{
  const uint8_t *base = (const uint8_t *)m;
  size_t r;
  static const uint8_t tag[4] = {0x49u, 0x56u, 0x53u, 0x44u}; /* "IVSD" */
  ivs_dsink_bytes(k, tag, sizeof tag);
  ivs_dsink_uint(k, domain, 1u);
  ivs_dsink_uint(k, IVS_DIGEST_DOMAIN_VERSION, 2u);
  for (r = 0; r < IVS_DIGEST_FIELD_COUNT; r++) {
    const ivs_digest_field *f = &ivs_digest_fields[r];
    uint32_t e;
    if (f->domain != domain) {
      continue;
    }
#ifdef IVS_FAULT_DIGEST_SKIPS_RAM
    if (f->offset == (uint32_t)offsetof(struct ivs_machine, mem.sysram)) continue;
#endif
#ifdef IVS_FAULT_DIGEST_SKIPS_IMAGE_TABLE
    if (f->offset >= (uint32_t)offsetof(struct ivs_machine, slot)) continue;
#endif
#ifdef IVS_FAULT_DIGEST_INCLUDES_PADDING
    if (f->kind == IVS_DIGEST_KIND_PAD) ivs_dsink_bytes(k, base + f->offset, (size_t)f->width * f->count);
#endif
    if (f->kind == IVS_DIGEST_KIND_PROFILE) {
      ivs_digest_profile(k, (uint32_t)ivs_digest_elem(base + f->offset, 4u));
      continue;
    }
    if (f->kind != IVS_DIGEST_KIND_UINT) {
      continue; /* MAGIC and PAD are never serialized */
    }
    if (f->count > 1u) {
      ivs_dsink_uint(k, f->count, 4u);
    }
    for (e = 0u; e < f->count; e++) {
      ivs_dsink_uint(k, ivs_digest_elem(base + f->offset + (size_t)e * f->width, f->width), f->width);
    }
  }
}

static void ivs_dsink_init(ivs_dsink *k, uint8_t hash, uint8_t *buf, size_t cap)
{
  memset(k, 0, sizeof *k);
  k->hash = hash;
  k->buf = buf;
  k->cap = cap;
  ivs_sha256_init(&k->sha);
}

size_t ivs_machine_digest_preimage(const struct ivs_machine *m, uint32_t domain, uint8_t *buf, size_t cap)
{
  ivs_dsink k;
  if (m == NULL || domain < IVS_DIGEST_CPU || domain > IVS_DIGEST_INSTANCE) {
    return 0u;
  }
  ivs_dsink_init(&k, 0u, buf, buf == NULL ? 0u : cap);
  ivs_digest_domain(m, domain, &k);
  ivs_dsink_flush(&k);
  return k.flushed;
}

static void ivs_digest_domain_sha(const struct ivs_machine *m, uint32_t domain, uint8_t out[32])
{
  ivs_dsink k;
  ivs_dsink_init(&k, 1u, NULL, 0u);
  ivs_digest_domain(m, domain, &k);
  ivs_dsink_flush(&k);
  ivs_sha256_final(&k.sha, out);
}

ivs_status ivs_machine_state_digest(const ivs_machine *m, uint32_t domain, uint8_t out_sha256[32])
{
  static const uint8_t tag[8] = {0x49u, 0x56u, 0x53u, 0x53u, 0x54u, 0x41u, 0x54u, 0x45u}; /* "IVSSTATE" */
  const struct ivs_machine *im;
  uint8_t d[4][32];
  ivs_dsink k;
  uint32_t i;
  uint8_t with_id = 1u;
  if (out_sha256 == NULL || domain > IVS_DIGEST_INSTANCE ||
      ivs_machine_span_overlaps(out_sha256, 32u, m, sizeof(struct ivs_machine)) != 0u) {
    return IVS_E_BAD_ARG;
  }
  im = ivs_machine_from_handle(m);
  if (im == NULL) {
    return IVS_E_BAD_HANDLE;
  }
  if (domain != IVS_DIGEST_COMPOSITE) {
    ivs_digest_domain_sha(im, domain, out_sha256);
    return IVS_OK;
  }
#ifdef IVS_FAULT_DOMAIN_ID_DROPPED
  with_id = 0u;
#endif
  for (i = 0u; i < 4u; i++) {
    ivs_digest_domain_sha(im, i + 1u, d[i]);
  }
  ivs_dsink_init(&k, 1u, NULL, 0u);
  ivs_dsink_bytes(&k, tag, sizeof tag);
  ivs_dsink_uint(&k, IVS_DIGEST_COMPOSITE_VERSION, 2u);
  for (i = 0u; i < 4u; i++) {
    if (with_id != 0u) {
      ivs_dsink_uint(&k, i + 1u, 1u);
    }
    ivs_dsink_bytes(&k, d[i], 32u);
  }
  ivs_dsink_flush(&k);
  ivs_sha256_final(&k.sha, out_sha256);
  return IVS_OK;
}
