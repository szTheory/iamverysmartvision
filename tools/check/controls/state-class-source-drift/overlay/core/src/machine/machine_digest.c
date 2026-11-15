/* SPDX-License-Identifier: MIT */
/* Known-good control tree stand-in for the digest field table (plan 08-09): a handful of rows that carry one row of each snapshot class and
 * exercise both forms the gate step state reads, a literal row and a row group made by a slot macro. It is valid C (the c-purity step compiles it)
 * and has no writable data. The table is read as text by the step; the names below are local stand-ins of the machine's constants. */
#include "common/ivs_prelude.h"

#define IVS_DIGEST_CPU 1u
#define IVS_DIGEST_MEM 2u
#define IVS_DIGEST_IMAGES 3u
#define IVS_DIGEST_INSTANCE 4u
#define IVS_DIGEST_KIND_UINT 1u
#define IVS_DIGEST_KIND_MAGIC 3u
#define IVS_DIGEST_KIND_PAD 4u
#define IVS_SNAP_STATE 1u
#define IVS_SNAP_IMAGE 2u
#define IVS_SNAP_BOUND 3u
#define IVS_SNAP_PAD 4u
#define TOY_WORDS 8u

typedef struct ivs_toy_field {
  const char *name;
  uint32_t offset;
  uint32_t width;
  uint32_t count;
  uint8_t domain;
  uint8_t kind;
  uint16_t snap;
} ivs_toy_field;

struct ivs_toy_machine {
  uint32_t magic;
  uint32_t counter;
  uint16_t ram[2u * TOY_WORDS];
  uint16_t image[64];
  uint32_t pad;
  uint32_t loaded[1];
  uint8_t sha256[32];
};

#define IVS_DF(name, member, width, count, domain, kind, snap) \
  {name, (uint32_t)offsetof(struct ivs_toy_machine, member), width, count, domain, kind, snap}

#define IVS_DF_SLOT(i, label)                                                                                       \
  IVS_DF("toy.slot." label ".loaded", loaded[i], 4u, 1u, IVS_DIGEST_IMAGES, IVS_DIGEST_KIND_UINT, IVS_SNAP_BOUND), \
      IVS_DF("toy.slot." label ".sha256", sha256, 1u, 32u, IVS_DIGEST_IMAGES, IVS_DIGEST_KIND_UINT, IVS_SNAP_BOUND)

static const ivs_toy_field ivs_digest_fields[] = {
    IVS_DF("toy.magic", magic, 4u, 1u, IVS_DIGEST_INSTANCE, IVS_DIGEST_KIND_MAGIC, IVS_SNAP_BOUND),
    IVS_DF("toy.counter", counter, 4u, 1u, IVS_DIGEST_CPU, IVS_DIGEST_KIND_UINT, IVS_SNAP_BOUND),
    IVS_DF("toy.ram", ram, 2u, 2u * TOY_WORDS, IVS_DIGEST_MEM, IVS_DIGEST_KIND_UINT, IVS_SNAP_STATE),
    IVS_DF("toy.image", image, 2u, 64u, IVS_DIGEST_MEM, IVS_DIGEST_KIND_UINT, IVS_SNAP_IMAGE),
    IVS_DF("toy.pad", pad, 4u, 1u, IVS_DIGEST_CPU, IVS_DIGEST_KIND_PAD, IVS_SNAP_PAD),
    IVS_DF_SLOT(0, "exec"),
};

_Static_assert(sizeof ivs_digest_fields / sizeof ivs_digest_fields[0] == 7u, "the toy table has seven rows (the gate step state counts them)");
