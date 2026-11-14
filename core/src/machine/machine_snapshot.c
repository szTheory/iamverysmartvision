/* SPDX-License-Identifier: MIT */
/* The snapshot writer (plan 08-11, D-12, D-13, D-14, docs/formats/ivsn-v1.md): the IVSN v1 container is built here, big-endian, field
 * by field, from the digest field table of machine_digest.c, which is the single serializer (D-11). The bytes of a section are the
 * state-only walk of that table (ivs_machine_state_preimage: the STATE rows of one domain, encoded exactly as the digest encodes them),
 * so no row is described twice and no image byte can be reached by this file. This file holds no static or global state, allocates
 * nothing, reads no clock and opens no file; every length is a 64-bit value on the public side and a 32-bit field in the container.
 *
 * Container (the oracle of plan 08-09; every number below was derived on paper before this code existed and is held to it by the
 * suite state, never the other way round):
 *     0   8  magic 89 49 56 53 4E 0D 0A 1A        8   2  format version        10   2  flags (0)
 *    12   4  total_bytes                          16   4  schema version      20   4  section count (7)
 *    24  32  schema digest   56  64  profile id   120  32  profile parameter digest
 *   152 120  three slot records of 40: u32 loaded, 32 bytes SHA-256 of the loaded container, u32 length_bytes
 *   272  32  composite digest (informational)    304 308  seven section entries of 44: u8 domain, 3 zero bytes, u32 offset,
 *                                                         u32 length, 32 bytes full-domain digest (informational)
 *   612      the sections, in table order, adjacent; then a 32-byte SHA-256 trailer over every earlier byte.
 * The size of the first profile is 612 + 3,655 + 32 = 4,299 bytes (sections 35, 2,761, 615, 51, 63, 79 and 51). The size is not a
 * constant of this file: it is the header, the lengths of the seven state-only walks (a counting pass over the table, no buffer) and
 * the trailer, so a state row added to the table changes it and the schema digest together.
 * Refusals (ivs/ivs_state.h): size and save both refuse in the order BAD_ARG for the argument pointers, BAD_HANDLE, BAD_ARG for a buffer
 * or output overlapping the instance, BAD_STATE while the machine is OFF, and (save only) BUFFER_TOO_SMALL last, which writes the exact
 * size to the output and never a byte of the buffer. Everything that can fail is decided before the first byte of the buffer is written.
 * Named faults (plan 08-11, suite state): SNAPSHOT_LITTLE_ENDIAN (the integer writer of the header below). The other two snapshot
 * faults are in the state-only walk of machine_digest.c. */
#include "common/ivs_prelude.h"

#include "common/ivs_sha256.h"
#include "ivs/ivs.h"
#include "machine/machine.h"

/* ---- the pinned layout ----
 * The offsets, the section count and the header and trailer sizes are in machine.h since plan 08-12, shared with the reader
 * (machine_snapshot_read.c): one set of numbers for the writer and the reader. */

/* ivs_snapshot_report v1 (ivs/ivs_state.h): every offset and the size are pinned; all members are u32, u64, bytes or image reports. */
_Static_assert(sizeof(ivs_snapshot_report) == 360u, "ivs_snapshot_report v1 is 360 bytes");
_Static_assert(IVS_SNAPSHOT_REPORT_V1_SIZE == 360u, "IVS_SNAPSHOT_REPORT_V1_SIZE is 360");
_Static_assert(offsetof(ivs_snapshot_report, struct_size) == 0u, "snapshot report struct_size at 0");
_Static_assert(offsetof(ivs_snapshot_report, reason) == 4u, "snapshot report reason at 4");
_Static_assert(offsetof(ivs_snapshot_report, format_version) == 8u, "snapshot report format_version at 8");
_Static_assert(offsetof(ivs_snapshot_report, schema_version) == 12u, "snapshot report schema_version at 12");
_Static_assert(offsetof(ivs_snapshot_report, total_bytes) == 16u, "snapshot report total_bytes at 16");
_Static_assert(offsetof(ivs_snapshot_report, byte_offset) == 24u, "snapshot report byte_offset at 24");
_Static_assert(offsetof(ivs_snapshot_report, section_index) == 32u, "snapshot report section_index at 32");
_Static_assert(offsetof(ivs_snapshot_report, kind) == 36u, "snapshot report kind at 36");
_Static_assert(offsetof(ivs_snapshot_report, profile_id) == 40u, "snapshot report profile_id at 40");
_Static_assert(offsetof(ivs_snapshot_report, profile_params_sha256) == 104u, "snapshot report profile_params_sha256 at 104");
_Static_assert(offsetof(ivs_snapshot_report, composite_sha256) == 136u, "snapshot report composite_sha256 at 136");
_Static_assert(offsetof(ivs_snapshot_report, slots) == 168u, "snapshot report slots at 168");

/* The seven sections, in table order: the domains that hold STATE rows. Domain 3 (IMAGES) holds none, so it has no section. Declared in
 * machine.h (the reader compares the section table of a container with it). */
const uint8_t ivs_snap_domain[IVS_SNAP_SECTIONS] = {IVS_DIGEST_CPU,   IVS_DIGEST_MEM,   IVS_DIGEST_INSTANCE, IVS_DIGEST_VIDEO,
                                                    IVS_DIGEST_AUDIO, IVS_DIGEST_INPUT, IVS_DIGEST_PSGLOG};

/* The derived section lengths (plan 08-12): from the descriptors of the digest field table alone, with no instance. A section is the
 * 7-byte frame (IVSD, the domain, the domain version) and, for each STATE row of the domain, width times count, plus 4 bytes of count
 * when the count is above 1: the rule of the state-only walk, restated from the specification. */
#define IVS_SNAP_FRAME_BYTES 7u

uint32_t ivs_snapshot_section_derived_length(uint32_t index)
{
  uint64_t n = IVS_SNAP_FRAME_BYTES;
  size_t rows = ivs_machine_digest_field_count();
  size_t r;
  if (index >= IVS_SNAP_SECTIONS) {
    return 0u;
  }
  for (r = 0u; r < rows; r++) {
    const ivs_digest_field *f = ivs_machine_digest_field(r);
    if (f == NULL || f->domain != ivs_snap_domain[index] || f->snap != IVS_SNAP_STATE || f->kind != IVS_DIGEST_KIND_UINT) {
      continue;
    }
    n += (uint64_t)f->width * f->count + (f->count > 1u ? 4u : 0u);
  }
  return (uint32_t)n;
}

uint64_t ivs_snapshot_derived_total(void)
{
  uint64_t total = (uint64_t)IVS_SNAP_HEADER_BYTES + (uint64_t)IVS_SNAP_TRAILER_BYTES;
  uint32_t i;
  for (i = 0u; i < IVS_SNAP_SECTIONS; i++) {
    total += ivs_snapshot_section_derived_length(i);
  }
  return total;
}

/* ---- the profile parameter digest (Task 1) ---- */

/* The version of the parameter digest preimage (docs/formats/ivsn-v1.md, "Profile identity"). */
#define IVS_PARAMS_DIGEST_VERSION 1u

ivs_status ivs_snapshot_params_digest(uint32_t profile_index, uint8_t out[32])
{
  /* IVSP (4) + be16 version (2) + four 32-byte body digests: the 134-byte preimage is built in one small local. */
  uint8_t pre[4u + 2u + 4u * 32u];
  const uint8_t *d[4];
  uint32_t i;
  if (out == NULL) {
    return IVS_E_BAD_ARG;
  }
  d[0] = ivs_memmap_table_digest(profile_index);
  d[1] = ivs_psg_table_digest(profile_index);
  d[2] = ivs_stic_table_digest(profile_index);
  d[3] = ivs_ctrl_table_digest(profile_index);
  for (i = 0u; i < 4u; i++) {
    if (d[i] == NULL) {
      return IVS_E_BAD_ARG;
    }
  }
  pre[0] = 0x49u; /* I */
  pre[1] = 0x56u; /* V */
  pre[2] = 0x53u; /* S */
  pre[3] = 0x50u; /* P */
  pre[4] = (uint8_t)(IVS_PARAMS_DIGEST_VERSION >> 8);
  pre[5] = (uint8_t)(IVS_PARAMS_DIGEST_VERSION & 0xFFu);
  for (i = 0u; i < 4u; i++) {
    memcpy(pre + 6u + 32u * i, d[i], 32u);
  }
  ivs_sha256_digest(pre, sizeof pre, out);
  return IVS_OK;
}

/* ---- the schema text and its digest ---- */

/* A bounded text builder: appends never write past cap - 1 bytes and the text stays NUL terminated. */
typedef struct ivs_snap_text {
  char *p;
  size_t cap;
  size_t n;
} ivs_snap_text;

static void ivs_snap_text_str(ivs_snap_text *t, const char *s)
{
  size_t i;
  for (i = 0u; s[i] != '\0' && t->n + 1u < t->cap; i++) {
    t->p[t->n] = s[i];
    t->n++;
  }
  t->p[t->n] = '\0';
}

static void ivs_snap_text_dec(ivs_snap_text *t, uint32_t v)
{
  char tmp[10];
  size_t k = 0u;
  char one[2];
  do {
    tmp[k] = (char)('0' + (char)(v % 10u));
    k++;
    v /= 10u;
  } while (v != 0u);
  one[1] = '\0';
  while (k > 0u) {
    k--;
    one[0] = tmp[k];
    ivs_snap_text_str(t, one);
  }
}

static const char *ivs_snap_kind_word(uint32_t kind)
{
  switch (kind) {
  case IVS_DIGEST_KIND_UINT:
    return "UINT";
  case IVS_DIGEST_KIND_PROFILE:
    return "PROFILE";
  case IVS_DIGEST_KIND_MAGIC:
    return "MAGIC";
  case IVS_DIGEST_KIND_PAD:
    return "PAD";
  default:
    return "UNKNOWN";
  }
}

static const char *ivs_snap_class_word(uint32_t snap)
{
  switch (snap) {
  case IVS_SNAP_STATE:
    return "STATE";
  case IVS_SNAP_IMAGE:
    return "IMAGE_BEARING";
  case IVS_SNAP_BOUND:
    return "BOUND";
  case IVS_SNAP_PAD:
    return "PAD";
  default:
    return "UNKNOWN";
  }
}

size_t ivs_snapshot_schema_line_count(void)
{
  return ivs_machine_digest_field_count() + 1u;
}

size_t ivs_snapshot_schema_line(size_t i, char *buf, size_t cap)
{
  ivs_snap_text t;
  const ivs_digest_field *f;
  if (buf == NULL || cap == 0u) {
    return 0u;
  }
  t.p = buf;
  t.cap = cap;
  t.n = 0u;
  buf[0] = '\0';
  if (i == 0u) {
    ivs_snap_text_str(&t, "IVSSCHEMA ");
    ivs_snap_text_dec(&t, IVS_SNAPSHOT_SCHEMA_VERSION);
    return t.n;
  }
  f = ivs_machine_digest_field(i - 1u);
  if (f == NULL) {
    return 0u;
  }
  ivs_snap_text_str(&t, f->name);
  ivs_snap_text_str(&t, " ");
  ivs_snap_text_dec(&t, f->width);
  ivs_snap_text_str(&t, " ");
  ivs_snap_text_dec(&t, f->count);
  ivs_snap_text_str(&t, " ");
  ivs_snap_text_dec(&t, f->domain);
  ivs_snap_text_str(&t, " ");
  ivs_snap_text_str(&t, ivs_snap_kind_word(f->kind));
  ivs_snap_text_str(&t, " ");
  ivs_snap_text_str(&t, ivs_snap_class_word(f->snap));
  return t.n;
}

/* The schema digest: the SHA-256 of the exact bytes of data/state/schema.literal.txt, which is every line above followed by a newline,
 * fed through one small local line buffer (no large frame, no allocation). */
void ivs_snapshot_schema_digest(uint8_t out[32])
{
  ivs_sha256 c;
  char line[128];
  size_t i;
  size_t n = ivs_snapshot_schema_line_count();
  ivs_sha256_init(&c);
  for (i = 0u; i < n; i++) {
    size_t len = ivs_snapshot_schema_line(i, line, sizeof line - 1u);
    line[len] = '\n';
    ivs_sha256_update(&c, (const uint8_t *)line, len + 1u);
  }
  ivs_sha256_final(&c, out);
}

/* ---- the container ---- */

/* v as width bytes at p, most significant first: the one writer of every header integer. */
static void ivs_snap_put(uint8_t *p, uint64_t v, uint32_t width)
{
  uint32_t i;
  for (i = 0u; i < width; i++) {
    uint32_t sh = 8u * (width - 1u - i);
#ifdef IVS_FAULT_SNAPSHOT_LITTLE_ENDIAN
    sh = 8u * i;
#endif
    p[i] = (uint8_t)(v >> sh);
  }
}

/* The exact lengths of the seven state-only sections by a counting pass (no buffer), and the total of the container. */
static uint64_t ivs_snap_measure(const struct ivs_machine *im, uint32_t len[IVS_SNAP_SECTIONS])
{
  uint64_t total = (uint64_t)IVS_SNAP_HEADER_BYTES + (uint64_t)IVS_SNAP_TRAILER_BYTES;
  uint32_t i;
  for (i = 0u; i < IVS_SNAP_SECTIONS; i++) {
    len[i] = (uint32_t)ivs_machine_state_preimage(im, ivs_snap_domain[i], NULL, 0u);
    total += len[i];
  }
  return total;
}

/* 1 when the n bytes at p (a 64-bit length) overlap the m bytes at q, or run past the top of the address space (such a span names no
 * real object, so it is refused like an overlap, as the run call refuses its buffers). */
static uint32_t ivs_snap_span_hits(const void *p, uint64_t n, const void *q, size_t m)
{
  if (n == 0u) {
    return 0u;
  }
  if (n > (uint64_t)SIZE_MAX) {
    return 1u;
  }
  return ivs_machine_span_overlaps(p, (size_t)n, q, m);
}

ivs_status ivs_machine_snapshot_size(const ivs_machine *m, uint64_t *out_bytes)
{
  const struct ivs_machine *im;
  uint32_t len[IVS_SNAP_SECTIONS];
  if (out_bytes == NULL) {
    return IVS_E_BAD_ARG;
  }
  im = ivs_machine_from_handle(m);
  if (im == NULL) {
    return IVS_E_BAD_HANDLE;
  }
  if (ivs_machine_out_overlaps(out_bytes, sizeof *out_bytes, im, sizeof *im) != 0u) {
    return IVS_E_BAD_ARG;
  }
  if (im->phase != IVS_PHASE_POWERED) {
    return IVS_E_BAD_STATE;
  }
  *out_bytes = ivs_snap_measure(im, len);
  return IVS_OK;
}

ivs_status ivs_machine_snapshot_save(const ivs_machine *m, uint8_t *buf, uint64_t cap, uint64_t *out_written)
{
  static const uint8_t magic[8] = {0x89u, 0x49u, 0x56u, 0x53u, 0x4Eu, 0x0Du, 0x0Au, 0x1Au};
  const struct ivs_machine *im;
  const ivs_profile_desc *pd;
  uint32_t len[IVS_SNAP_SECTIONS];
  uint8_t dig[IVS_SNAP_SECTIONS][32];
  uint8_t composite[32];
  uint8_t schema[32];
  uint8_t params[32];
  uint32_t id_len = 0u;
  uint64_t total;
  uint32_t off;
  uint32_t i;
  if (out_written == NULL || (buf == NULL && cap > 0u) || ivs_snap_span_hits(buf, cap, out_written, sizeof *out_written) != 0u) {
    return IVS_E_BAD_ARG;
  }
  im = ivs_machine_from_handle(m);
  if (im == NULL) {
    return IVS_E_BAD_HANDLE;
  }
  if (ivs_machine_out_overlaps(out_written, sizeof *out_written, im, sizeof *im) != 0u ||
      ivs_snap_span_hits(buf, cap, im, sizeof *im) != 0u) {
    return IVS_E_BAD_ARG;
  }
  if (im->phase != IVS_PHASE_POWERED) {
    return IVS_E_BAD_STATE;
  }
  total = ivs_snap_measure(im, len);
  if (cap < total) {
    *out_written = total;
    return IVS_E_BUFFER_TOO_SMALL;
  }

  /* Everything that can fail is decided here, into locals, before the first byte of the buffer is written. */
  pd = ivs_profile_get(im->profile);
  if (pd == NULL || ivs_machine_profile_id_length(pd->id, &id_len) != IVS_OK ||
      ivs_snapshot_params_digest(im->profile, params) != IVS_OK ||
      ivs_machine_state_digest(m, IVS_DIGEST_COMPOSITE, composite) != IVS_OK) {
    return IVS_E_BAD_HANDLE; /* a damaged profile: unreachable for a handle that passed ivs_machine_from_handle */
  }
  for (i = 0u; i < IVS_SNAP_SECTIONS; i++) {
    if (ivs_machine_state_digest(m, ivs_snap_domain[i], dig[i]) != IVS_OK) {
      return IVS_E_BAD_HANDLE;
    }
  }
  ivs_snapshot_schema_digest(schema);

  /* The sections: the state-only walk of each domain written straight into the buffer at its offset. cap bounds the walk, so a
   * disagreement between the counting pass and this one (a test proves there is none) could never write past the container. */
  off = IVS_SNAP_HEADER_BYTES;
  for (i = 0u; i < IVS_SNAP_SECTIONS; i++) {
    (void)ivs_machine_state_preimage(im, ivs_snap_domain[i], buf + off, (size_t)(total - off));
    off += len[i];
  }

  /* The header, field by field. */
  memcpy(buf, magic, sizeof magic);
  ivs_snap_put(buf + IVS_SNAP_OFF_FORMAT, IVS_SNAPSHOT_FORMAT_VERSION, 2u);
  ivs_snap_put(buf + IVS_SNAP_OFF_FLAGS, 0u, 2u);
  ivs_snap_put(buf + IVS_SNAP_OFF_TOTAL, total, 4u);
  ivs_snap_put(buf + IVS_SNAP_OFF_SCHEMA, IVS_SNAPSHOT_SCHEMA_VERSION, 4u);
  ivs_snap_put(buf + IVS_SNAP_OFF_COUNT, IVS_SNAP_SECTIONS, 4u);
  memcpy(buf + IVS_SNAP_OFF_SCHEMA_DIGEST, schema, 32u);
  memset(buf + IVS_SNAP_OFF_PROFILE_ID, 0, IVS_PROFILE_ID_MAX_BYTES);
  memcpy(buf + IVS_SNAP_OFF_PROFILE_ID, pd->id, id_len);
  memcpy(buf + IVS_SNAP_OFF_PARAMS, params, 32u);
  for (i = 0u; i < IVS_SLOT_COUNT; i++) {
    uint8_t *r = buf + IVS_SNAP_OFF_SLOTS + IVS_SNAP_SLOT_BYTES * i;
    ivs_snap_put(r, im->slot[i].loaded, 4u);
    memcpy(r + 4u, im->slot[i].sha256, 32u);
    ivs_snap_put(r + 36u, im->slot[i].length_bytes, 4u);
  }
  memcpy(buf + IVS_SNAP_OFF_COMPOSITE, composite, 32u);
  off = IVS_SNAP_HEADER_BYTES;
  for (i = 0u; i < IVS_SNAP_SECTIONS; i++) {
    uint8_t *e = buf + IVS_SNAP_OFF_TABLE + IVS_SNAP_ENTRY_BYTES * i;
    ivs_snap_put(e, ivs_snap_domain[i], 1u);
    memset(e + 1u, 0, 3u);
    ivs_snap_put(e + 4u, off, 4u);
    ivs_snap_put(e + 8u, len[i], 4u);
    memcpy(e + 12u, dig[i], 32u);
    off += len[i];
  }

  /* The trailer: the SHA-256 of every earlier byte. */
  ivs_sha256_digest(buf, (size_t)(total - IVS_SNAP_TRAILER_BYTES), buf + (size_t)(total - IVS_SNAP_TRAILER_BYTES));
  *out_written = total;
  return IVS_OK;
}
