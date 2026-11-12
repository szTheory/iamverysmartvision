/* SPDX-License-Identifier: MIT */
/* Suite psg, family log (AUD-02, plan 05-08, D-13, D-14, D-18, A-01): every guest write to $01F0..$01FF becomes one
 * 16-byte record in a caller log buffer and a step of the domain-19 chain (digest domain 19, IVS_DIGEST_PSGLOG).
 * Record, written byte by byte big-endian: bytes 0..7 tick, 8..9 value (the raw 16-bit bus value), 10 addr (low
 * nibble of the bus address), 11 source (1 CPU write, 2 interrupt-entry write), 12..15 zero. Chain:
 *   chain' = SHA-256('I' 'V' 'S' 'R' || 19 || chain || be64(index) || be32(16) || record), index = records before.
 * Expected values here come from the cycle tables (the offsets of data/cp1610/vectors: a direct MVO writes at CPU
 * cycle 6 of its 11-cycle unit, an MVO@ at cycle 4 of its 9-cycle unit), the observed unit starts of a seam-stepped
 * twin, hand-written byte literals, the CPU's own access trace and a replay into a fresh PSG; never from a core
 * output read back as the expectation.
 *
 * Authoring recipe of the chain literals (POSIX sh or dash, in a mktemp -d directory; nothing is committed; the
 * values were cross-checked with shasum -a 256, a second, independent hash implementation):
 *   oct() { h=$1; while [ -n "$h" ]; do b=${h%"${h#??}"}; h=${h#??}; printf '\\%03o' "0x$b"; done; }
 *   sha() { cmake -E sha256sum "$1" | cut -c1-64; }
 *   Z=0000000000000000000000000000000000000000000000000000000000000000
 *   printf "$(oct "4956535213${Z}0000000000000000""00000010""000000000000004c03ff0401""00000000")" > c1.in; C1=$(sha c1.in)
 * The 65-byte preimage is "IVSR" (49 56 53 52), the domain byte 19 (0x13), the 32-byte chain (zeros first), be64 of
 * the index (0 for the first record), be32 16 and the 16-byte record. The domain digest literal hashes the domain
 * preimage "IVSD" || 0x13 || be16 1 || be32 32 || chain || be64 count:
 *   printf "$(oct "49565344""13""0001""00000020""${C1}""0000000000000001")" > d1.in; D1=$(sha d1.in)
 * with tick 0x4c = 76 (the unit start 52 = 13 CPU cycles, the reset entry then the 8-cycle MVII, plus the write at
 * cycle 6 = 24 ticks):
 *   C1 = 198b9bed934a9ff42b5ba79751b692b85559aa015622a9dea874e119e5eb9bba
 *   D1 = 8b1cff42d5105b7004a6dfa301d102efd4675c27c74b9a2ac2f1b5ae5c11f6eb
 * and the two-record chain of psg.log.chain-two-records (records {tick 0x10, value 0x0102, addr 3, source 1} then
 * {tick 0x20, value 0x0304, addr 10, source 2}, the second with index 1 and the chain A1 as input):
 *   A1 = e2c80a2dbceffcccf1f5873057341e388b7e4124da4bcc803a9d88bfa79ab242
 *   A2 = 4e28c2cdd119f2c1f8019631c3dd559320730f461df9b3bd5bfa6747b89e2f1d
 * These literals check framing and order, not hardware behaviour.
 *
 *   psg.log.framing                  one MVO of $03FF to $01F4 gives one record, 16 bytes, equal to the hand-written
 *                                    literal, report fields 16 and 0, and the chain row and the domain-19 digest
 *                                    equal the literals above
 *   psg.log.chain-two-records        the chain after two records equals the literals; the swapped order differs
 *   psg.log.byte-order               tick, value, addr and source bytes at their offsets, big-endian
 *   psg.log.reserved-zero            bytes 12..15 are zero whatever the other fields hold
 *   psg.log.source-values            a WRITE through the machine bus wrapper logs source 1, a DW source 2
 *   psg.log.value-raw-16bit          a write of $FFFF to $01F4 logs $FFFF while the register holds $000F
 *   psg.log.addr-low-nibble          the 16 registers log addr 0..15; $01EF and $0200 log nothing
 *   psg.log.no-reads-logged          reads of the block (READ, ADAR, FETCH accesses and a port-reading guest) log nothing
 *   psg.log.no-synthetic-power-on-records  a fresh powered instance has no record; a guest that never touches the PSG
 *                                    delivers none in several frames
 *   psg.log.strictly-increasing      the ticks of the worst-case writer's records and of a random program increase
 *   psg.log.window                   the stamp of a direct MVO is its unit start + 6 x 4 and of an MVO@ + 4 x 4,
 *                                    the unit starts observed from a seam-stepped twin
 *   psg.log.vs-cpu-trace             every WRITE or DW to $01F0..$01FF of the CPU's access trace appears in the log
 *                                    with tick = unit start + offset x 4, and no other record exists
 *   psg.log.replay-equals-run        replaying the records through ivs_psg_write into a fresh PSG reproduces the
 *                                    samples and the AUDIO row of the original run
 *   psg.log.effect-boundary          a write stamped t (a multiple of 16) is seen by the sample at t + 16 and not
 *                                    by the sample at t; one stamped t - 1 is seen by the sample at t */
#include "psg_util.h"

#include "psg_cases.h"

#define LG_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define LG_SENTINEL 0xA5u
#define LG_WRITER_N 600u

static const uint8_t lg_c1[32] = {
    0x19, 0x8b, 0x9b, 0xed, 0x93, 0x4a, 0x9f, 0xf4, 0x2b, 0x5b, 0xa7, 0x97, 0x51, 0xb6, 0x92, 0xb8,
    0x55, 0x59, 0xaa, 0x01, 0x56, 0x22, 0xa9, 0xde, 0xa8, 0x74, 0xe1, 0x19, 0xe5, 0xeb, 0x9b, 0xba,
};
static const uint8_t lg_d1[32] = {
    0x8b, 0x1c, 0xff, 0x42, 0xd5, 0x10, 0x5b, 0x70, 0x04, 0xa6, 0xdf, 0xa3, 0x01, 0xd1, 0x02, 0xef,
    0xd4, 0x67, 0x5c, 0x27, 0xc7, 0x4b, 0x9a, 0x2a, 0xc2, 0xf1, 0xb5, 0xae, 0x5c, 0x11, 0xf6, 0xeb,
};
static const uint8_t lg_a1[32] = {
    0xe2, 0xc8, 0x0a, 0x2d, 0xbc, 0xef, 0xfc, 0xcc, 0xf1, 0xf5, 0x87, 0x30, 0x57, 0x34, 0x1e, 0x38,
    0x8b, 0x7e, 0x41, 0x24, 0xda, 0x4b, 0xcc, 0x80, 0x3a, 0x9d, 0x88, 0xbf, 0xa7, 0x9a, 0xb2, 0x42,
};
static const uint8_t lg_a2[32] = {
    0x4e, 0x28, 0xc2, 0xcd, 0xd1, 0x19, 0xf2, 0xc1, 0xf8, 0x01, 0x96, 0x31, 0xc3, 0xdd, 0x55, 0x93,
    0x20, 0x73, 0x0f, 0x46, 0x1d, 0xf9, 0xb3, 0xbd, 0x5b, 0xfa, 0x67, 0x47, 0xb8, 0x9e, 0x2f, 0x1d,
};

/* Steps m through the seam (counting sink) until the PSGLOG row holds more than before records, returning the machine
 * time before that unit in *start (the observed unit start) and the machine time after it in *end. 1 on success. */
static int lg_find_write(ivs_machine *m, uint64_t *start, uint64_t *end)
{
  ivs_psg_sink k;
  uint32_t guard;
  uint64_t before_count = ((struct ivs_machine *)m)->obs.psglog.count;
  ivs_psg_sink_init(&k, NULL, 0u);
  for (guard = 0u; guard < 40u; guard++) {
    ivs_cp1610_result res;
    uint64_t before = pu_now(m);
    if (ivs_machine_step_unit_sink_internal(m, &res, &k) != IVS_OK) {
      return 0;
    }
    if (((struct ivs_machine *)m)->obs.psglog.count > before_count) {
      *start = before;
      *end = pu_now(m);
      return 1;
    }
  }
  return 0;
}

static int lg_framing(void)
{
  /* The tracer: stand-in program C writes R0 = $03FF to $01F4 (the coarse tone period of channel A: four bits, so
   * the register holds $000F) once, then reads it back and stores it to $0200, then loops. The unit that holds the
   * direct MVO starts at S; the write is its access at CPU cycle 6 (a direct MVO is 11 cycles, its write follows the
   * ADAR address phase), stamped S + 6 x 4 master ticks. */
  ivs_ms_inst twin;
  ivs_ms_inst x;
  ivs_run_report rep;
  uint8_t *log = NULL;
  uint8_t want[16];
  uint8_t digest[32];
  uint64_t start = 0u;
  uint64_t end = 0u;
  uint64_t cap;
  uint32_t i;
  int rc = IVS_TEST_FAIL;
  twin.storage = x.storage = NULL;
  twin.m = x.m = NULL;
  LG_CHECK(ivs_ms_powered_prog(&twin, IVS_MS_PROG_C, 0x5Au) && ivs_ms_powered_prog(&x, IVS_MS_PROG_C, 0x5Au));
  LG_CHECK(lg_find_write(twin.m, &start, &end));
  cap = pu_log_need(x.m, 0u, end);
  log = malloc((size_t)cap);
  LG_CHECK(log != NULL && cap >= 2u * 16u);
  memset(log, LG_SENTINEL, (size_t)cap);
  LG_CHECK(pu_run_to_log(x.m, end, NULL, log, &rep));
  LG_CHECK(rep.stop_reason == IVS_STOP_TARGET && rep.now_ticks == end);
  LG_CHECK(start == 52u); /* the reset entry (5 cycles) and the 8-cycle MVII: 13 CPU cycles of 4 ticks */
  LG_CHECK(rep.psg_log_bytes_written == 16u && rep.psg_log_first_index == 0u);
  /* the hand-written record */
  pu_be64(want, start + 6u * 4u); /* 76 */
  LG_CHECK(want[7] == 0x4Cu);
  want[8] = 0x03u;
  want[9] = 0xFFu;
  want[10] = 0x04u;
  want[11] = 0x01u;
  want[12] = want[13] = want[14] = want[15] = 0u;
  LG_CHECK(memcmp(log, want, 16u) == 0);
  for (i = 16u; i < cap; i++) {
    LG_CHECK(log[i] == LG_SENTINEL); /* nothing past the delivered record */
  }
  LG_CHECK(((struct ivs_machine *)x.m)->obs.psglog.count == 1u);
  LG_CHECK(memcmp(((struct ivs_machine *)x.m)->obs.psglog.chain, lg_c1, 32u) == 0);
  LG_CHECK(ivs_machine_state_digest(x.m, IVS_DIGEST_PSGLOG, digest) == IVS_OK);
  LG_CHECK(memcmp(digest, lg_d1, 32u) == 0);
  rc = IVS_TEST_PASS;
done:
  free(log);
  if (twin.storage != NULL) {
    ivs_ms_free(&twin);
  }
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return rc;
}

static int lg_chain_two_records(void)
{
  ivs_psg_sink k;
  ivs_psg_logrow row;
  ivs_psg_logrow swapped;
  uint8_t buf[32];
  static const uint8_t rec1[16] = {0, 0, 0, 0, 0, 0, 0, 0x10, 0x01, 0x02, 0x03, 0x01, 0, 0, 0, 0};
  static const uint8_t rec2[16] = {0, 0, 0, 0, 0, 0, 0, 0x20, 0x03, 0x04, 0x0A, 0x02, 0, 0, 0, 0};
  memset(&row, 0, sizeof row);
  memset(&swapped, 0, sizeof swapped);
  memset(buf, LG_SENTINEL, sizeof buf);
  ivs_psg_sink_init(&k, NULL, 0u);
  ivs_psg_sink_log(&k, buf, sizeof buf);
  ivs_psg_log_append(&k, &row, 0x10u, 0x0102u, 0x03u, 1u);
  if (memcmp(row.chain, lg_a1, 32u) != 0 || row.count != 1u || memcmp(buf, rec1, 16u) != 0) {
    return IVS_TEST_FAIL;
  }
  ivs_psg_log_append(&k, &row, 0x20u, 0x0304u, 0x0Au, 2u);
  if (memcmp(row.chain, lg_a2, 32u) != 0 || row.count != 2u || memcmp(buf + 16, rec2, 16u) != 0 ||
      k.log_written != 32u || k.log_records != 2u || k.log_overflow != 0u) {
    return IVS_TEST_FAIL;
  }
  /* the order matters: the same two records the other way round give another chain */
  ivs_psg_sink_init(&k, NULL, 0u);
  ivs_psg_log_append(&k, &swapped, 0x20u, 0x0304u, 0x0Au, 2u);
  ivs_psg_log_append(&k, &swapped, 0x10u, 0x0102u, 0x03u, 1u);
  if (memcmp(swapped.chain, row.chain, 32u) == 0 || k.log_written != 0u || k.log_records != 2u) {
    return IVS_TEST_FAIL; /* the counting sink delivers nothing and still counts */
  }
  return IVS_TEST_PASS;
}

static int lg_byte_order(void)
{
  ivs_psg_sink k;
  ivs_psg_logrow row;
  uint8_t buf[16];
  static const uint8_t want[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0xA1, 0xB2, 0x0D, 0x02, 0, 0, 0, 0};
  memset(&row, 0, sizeof row);
  memset(buf, LG_SENTINEL, sizeof buf);
  ivs_psg_sink_init(&k, NULL, 0u);
  ivs_psg_sink_log(&k, buf, sizeof buf);
  ivs_psg_log_append(&k, &row, 0x0102030405060708ull, 0xA1B2u, 0xFDu, 2u);
  return memcmp(buf, want, 16u) == 0 ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int lg_reserved_zero(void)
{
  ivs_psg_sink k;
  ivs_psg_logrow row;
  uint8_t buf[32];
  uint32_t i;
  memset(&row, 0, sizeof row);
  memset(buf, 0xFFu, sizeof buf);
  ivs_psg_sink_init(&k, NULL, 0u);
  ivs_psg_sink_log(&k, buf, sizeof buf);
  ivs_psg_log_append(&k, &row, ~0ull, 0xFFFFu, 0xFFu, 0xFFu);
  ivs_psg_log_append(&k, &row, 0u, 0u, 0u, 0u);
  for (i = 12u; i < 16u; i++) {
    if (buf[i] != 0u || buf[16u + i] != 0u) {
      return IVS_TEST_FAIL;
    }
  }
  return buf[10] == 0x0Fu && buf[8] == 0xFFu ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* A powered instance, a bus context over a log sink of cap bytes (one frame slot), for the synthetic-access tests. */
typedef struct lg_rig {
  ivs_ms_inst x;
  ivs_machine_bus_ctx ctx;
  ivs_psg_sink sink;
  uint8_t *log;
  uint64_t cap;
} lg_rig;

static int lg_rig_init(lg_rig *r)
{
  uint32_t profile;
  uint32_t p;
  memset(r, 0, sizeof *r);
  if (!ivs_ms_powered(&r->x, IVS_MS_CONTENT_A, 0x5Au)) {
    return 0;
  }
  profile = ivs_profile_count();
  for (p = 0u; p < ivs_profile_count(); p++) {
    if (strcmp(ivs_profile_get(p)->id, IVS_MS_PROFILE) == 0) {
      profile = p;
    }
  }
  r->cap = 34432u;
  r->log = malloc((size_t)r->cap);
  if (r->log == NULL || profile == ivs_profile_count()) {
    ivs_ms_free(&r->x);
    free(r->log);
    r->log = NULL;
    return 0;
  }
  memset(r->log, LG_SENTINEL, (size_t)r->cap);
  ivs_psg_sink_init(&r->sink, NULL, 0u);
  ivs_psg_sink_log(&r->sink, r->log, r->cap);
  r->ctx.m = ivs_machine_from_handle_mut(r->x.m);
  r->ctx.psg = ivs_psg_params_get(profile);
  r->ctx.sink = &r->sink;
  r->ctx.ticks_per_cpu_cycle = ivs_profile_get(profile)->ticks_per_cpu_cycle;
  return 1;
}

static void lg_rig_free(lg_rig *r)
{
  free(r->log);
  if (r->x.storage != NULL) {
    ivs_ms_free(&r->x);
  }
}

/* One synthetic access of kind kind to addr with data data at CPU cycle offset offset of a unit starting at start. */
static ivs_bus_reply lg_access(lg_rig *r, uint32_t kind, uint32_t addr, uint32_t data, uint32_t offset, uint64_t start)
{
  ivs_cp1610_access a;
  memset(&a, 0, sizeof a);
  a.kind = (uint8_t)kind;
  a.addr = (uint16_t)addr;
  a.data = (uint16_t)data;
  a.offset_cpu_cycles = offset;
  r->ctx.unit_start_tick = start;
  return ivs_machine_bus(&r->ctx, &a);
}

static int lg_source_values(void)
{
  lg_rig r;
  uint8_t want1[16];
  uint8_t want2[16];
  int rc = IVS_TEST_FAIL;
  if (!lg_rig_init(&r)) {
    return IVS_TEST_FAIL;
  }
  (void)lg_access(&r, IVS_ACC_WRITE, 0x01F2u, 0x0005u, 2u, 100u);
  (void)lg_access(&r, IVS_ACC_DW, 0x01F2u, 0x0006u, 2u, 200u);
  LG_CHECK(r.sink.log_written == 32u);
  pu_be64(want1, 100u + 2u * 4u);
  want1[8] = 0x00u;
  want1[9] = 0x05u;
  want1[10] = 0x02u;
  want1[11] = 0x01u; /* WRITE: source 1 */
  memset(want1 + 12, 0, 4u);
  pu_be64(want2, 200u + 2u * 4u);
  want2[8] = 0x00u;
  want2[9] = 0x06u;
  want2[10] = 0x02u;
  want2[11] = 0x02u; /* DW: source 2 */
  memset(want2 + 12, 0, 4u);
  LG_CHECK(memcmp(r.log, want1, 16u) == 0);
  LG_CHECK(memcmp(r.log + 16, want2, 16u) == 0);
  rc = IVS_TEST_PASS;
done:
  lg_rig_free(&r);
  return rc;
}

static int lg_value_raw_16bit(void)
{
  lg_rig r;
  int rc = IVS_TEST_FAIL;
  if (!lg_rig_init(&r)) {
    return IVS_TEST_FAIL;
  }
  (void)lg_access(&r, IVS_ACC_WRITE, 0x01F4u, 0xFFFFu, 0u, 64u);
  LG_CHECK(r.sink.log_written == 16u);
  LG_CHECK(r.log[8] == 0xFFu && r.log[9] == 0xFFu && r.log[10] == 0x04u);
  /* the register keeps only its four bits: the log keeps what the bus carried */
  LG_CHECK(ivs_memsys_psg_reg_internal(&r.ctx.m->mem, 4u) == 0x000Fu);
  rc = IVS_TEST_PASS;
done:
  lg_rig_free(&r);
  return rc;
}

static int lg_addr_low_nibble(void)
{
  lg_rig r;
  uint32_t a;
  int rc = IVS_TEST_FAIL;
  if (!lg_rig_init(&r)) {
    return IVS_TEST_FAIL;
  }
  for (a = 0u; a < 16u; a++) {
    (void)lg_access(&r, IVS_ACC_WRITE, 0x01F0u + a, 0x0001u + a, 0u, 64u + 28u * a);
  }
  LG_CHECK(r.sink.log_written == 16u * 16u);
  for (a = 0u; a < 16u; a++) {
    LG_CHECK(r.log[16u * a + 10u] == a);
    LG_CHECK(r.log[16u * a + 9u] == 1u + a);
  }
  /* outside the block: nothing is logged */
  (void)lg_access(&r, IVS_ACC_WRITE, 0x01EFu, 1u, 0u, 2000u);
  (void)lg_access(&r, IVS_ACC_WRITE, 0x0200u, 1u, 0u, 2100u);
  LG_CHECK(r.sink.log_written == 16u * 16u && r.ctx.m->obs.psglog.count == 16u);
  rc = IVS_TEST_PASS;
done:
  lg_rig_free(&r);
  return rc;
}

static int lg_no_reads_logged(void)
{
  lg_rig r;
  ivs_ms_inst x;
  ivs_run_report rep;
  uint8_t *log = NULL;
  uint32_t k;
  uint32_t a;
  static const uint32_t kinds[4] = {IVS_ACC_READ, IVS_ACC_ADAR, IVS_ACC_FETCH, IVS_ACC_INTAK};
  int rc = IVS_TEST_FAIL;
  x.storage = NULL;
  x.m = NULL;
  if (!lg_rig_init(&r)) {
    return IVS_TEST_FAIL;
  }
  for (k = 0u; k < 4u; k++) {
    for (a = 0u; a < 16u; a++) {
      (void)lg_access(&r, kinds[k], 0x01F0u + a, 0u, 0u, 64u);
    }
  }
  LG_CHECK(r.sink.log_written == 0u && r.sink.log_records == 0u && r.ctx.m->obs.psglog.count == 0u);
  /* a guest that reads both ports (program PORTS) leaves no record either */
  LG_CHECK(ivs_ms_powered_prog(&x, IVS_MS_PROG_PORTS, LG_SENTINEL));
  log = malloc((size_t)pu_log_need(x.m, 0u, 3000u));
  LG_CHECK(log != NULL);
  LG_CHECK(pu_run_to_log(x.m, 3000u, NULL, log, &rep));
  LG_CHECK(rep.psg_log_bytes_written == 0u && ((struct ivs_machine *)x.m)->obs.psglog.count == 0u);
  rc = IVS_TEST_PASS;
done:
  free(log);
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  lg_rig_free(&r);
  return rc;
}

static int lg_no_synthetic_power_on(void)
{
  static const uint8_t zero[40] = {0};
  ivs_ms_inst x;
  ivs_run_report rep;
  uint8_t *log = NULL;
  uint8_t digest[32];
  uint8_t fresh[32];
  int rc = IVS_TEST_FAIL;
  x.storage = NULL;
  x.m = NULL;
  LG_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, LG_SENTINEL)); /* program A writes $0200 only */
  LG_CHECK(memcmp(&((struct ivs_machine *)x.m)->obs.psglog, zero, sizeof zero) == 0);
  log = malloc((size_t)pu_log_need(x.m, 0u, 200000u));
  LG_CHECK(log != NULL);
  LG_CHECK(pu_run_to_log(x.m, 200000u, NULL, log, &rep)); /* more than three frames */
  LG_CHECK(rep.psg_log_bytes_written == 0u && rep.psg_log_first_index == 0u);
  LG_CHECK(memcmp(&((struct ivs_machine *)x.m)->obs.psglog, zero, sizeof zero) == 0);
  /* and the domain digest of the empty log is the one of a fresh instance */
  LG_CHECK(ivs_machine_state_digest(x.m, IVS_DIGEST_PSGLOG, digest) == IVS_OK);
  {
    ivs_ms_inst y;
    y.storage = NULL;
    y.m = NULL;
    LG_CHECK(ivs_ms_powered(&y, IVS_MS_CONTENT_B, LG_SENTINEL));
    LG_CHECK(ivs_machine_state_digest(y.m, IVS_DIGEST_PSGLOG, fresh) == IVS_OK);
    ivs_ms_free(&y);
  }
  LG_CHECK(memcmp(digest, fresh, 32u) == 0);
  rc = IVS_TEST_PASS;
done:
  free(log);
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return rc;
}

/* Instance running the worst-case writer with n writes (heap words), or the random program. 1 on success. */
static int lg_writer(ivs_ms_inst *x, uint32_t n)
{
  uint16_t *w = malloc((n + 8u) * sizeof w[0]);
  size_t cnt;
  int ok;
  if (w == NULL) {
    return 0;
  }
  cnt = pu_prog_writer(w, n);
  ok = ivs_ms_powered_words(x, w, cnt, LG_SENTINEL);
  free(w);
  return ok;
}

static int lg_random(ivs_ms_inst *x, uint32_t seed)
{
  uint16_t w[5u * 160u + 4u];
  size_t cnt = pu_prog_random(w, seed, 160u);
  return ivs_ms_powered_words(x, w, cnt, LG_SENTINEL);
}

static int lg_ticks_increase(const uint8_t *log, uint64_t n)
{
  uint64_t i;
  for (i = 1u; i < n; i++) {
    if (pu_get_be64(log + 16u * i) <= pu_get_be64(log + 16u * (i - 1u))) {
      return 0;
    }
  }
  return 1;
}

static int lg_strictly_increasing(void)
{
  ivs_ms_inst x;
  ivs_ms_inst y;
  ivs_run_report rep;
  uint8_t *log = NULL;
  uint64_t n;
  int rc = IVS_TEST_FAIL;
  x.storage = y.storage = NULL;
  x.m = y.m = NULL;
  LG_CHECK(lg_writer(&x, LG_WRITER_N) && lg_random(&y, 0x2545F491u));
  log = malloc((size_t)pu_log_need(x.m, 0u, 80000u));
  LG_CHECK(log != NULL);
  LG_CHECK(pu_run_to_log(x.m, 20000u, NULL, log, &rep));
  n = rep.psg_log_bytes_written / 16u;
  LG_CHECK(n > 400u && lg_ticks_increase(log, n));
  free(log);
  log = malloc((size_t)pu_log_need(y.m, 0u, 80000u));
  LG_CHECK(log != NULL);
  LG_CHECK(pu_run_to_log(y.m, 80000u, NULL, log, &rep));
  n = rep.psg_log_bytes_written / 16u;
  LG_CHECK(n > 400u && lg_ticks_increase(log, n));
  rc = IVS_TEST_PASS;
done:
  free(log);
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  if (y.storage != NULL) {
    ivs_ms_free(&y);
  }
  return rc;
}

static int lg_window(void)
{
  /* LOGMIX: MVII #$0F, R0 (8 cycles); MVO R0, $01FB (11 cycles, write at cycle 6); MVII #$01FC, R3 (8 cycles);
   * MVO@ R0, R3 (9 cycles, write at cycle 4); then a branch to itself. The twin is stepped through the seam to
   * observe the start of each writing unit; the run call's records must be start + offset x 4. */
  ivs_ms_inst twin;
  ivs_ms_inst x;
  ivs_run_report rep;
  uint16_t w[16];
  size_t n = pu_prog_logmix(w);
  uint8_t *log = NULL;
  uint64_t s1 = 0u;
  uint64_t e1 = 0u;
  uint64_t s2 = 0u;
  uint64_t e2 = 0u;
  int rc = IVS_TEST_FAIL;
  twin.storage = x.storage = NULL;
  twin.m = x.m = NULL;
  LG_CHECK(ivs_ms_powered_words(&twin, w, n, LG_SENTINEL) && ivs_ms_powered_words(&x, w, n, LG_SENTINEL));
  LG_CHECK(lg_find_write(twin.m, &s1, &e1) && lg_find_write(twin.m, &s2, &e2));
  LG_CHECK(e1 - s1 == 11u * 4u && e2 - s2 == 9u * 4u); /* the unit lengths of the cycle tables */
  log = malloc((size_t)pu_log_need(x.m, 0u, e2));
  LG_CHECK(log != NULL);
  LG_CHECK(pu_run_to_log(x.m, e2, NULL, log, &rep));
  LG_CHECK(rep.psg_log_bytes_written == 32u);
  LG_CHECK(pu_get_be64(log) == s1 + 6u * 4u);
  LG_CHECK(pu_get_be64(log + 16) == s2 + 4u * 4u);
  LG_CHECK(log[10] == 0x0Bu && log[26] == 0x0Cu);
  rc = IVS_TEST_PASS;
done:
  free(log);
  if (twin.storage != NULL) {
    ivs_ms_free(&twin);
  }
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return rc;
}

/* The CPU's own account of the writes of units unit run through the seam with an access trace: the expected records
 * of every WRITE or DW to $01F0..$01FF, tick = unit start + offset x ticks per cycle. Appends them to want (16 bytes
 * each) and returns the new record count. */
static uint64_t lg_trace_units(ivs_ms_inst *x, uint64_t units, uint8_t *want, uint64_t n_want, ivs_psg_sink *sink)
{
  uint64_t u;
  for (u = 0u; u < units; u++) {
    ivs_cp1610_access trace[IVS_CP1610_MAX_ACCESSES];
    ivs_cp1610_result res;
    uint64_t start = pu_now(x->m);
    uint32_t i;
    memset(trace, 0, sizeof trace);
    if (ivs_machine_step_unit_trace_internal(x->m, &res, sink, trace, IVS_CP1610_MAX_ACCESSES) != IVS_OK) {
      return UINT64_MAX;
    }
    for (i = 0u; i < res.n_accesses && i < IVS_CP1610_MAX_ACCESSES; i++) {
      const ivs_cp1610_access *a = &trace[i];
      if ((a->kind == IVS_ACC_WRITE || a->kind == IVS_ACC_DW) && a->addr >= 0x01F0u && a->addr <= 0x01FFu) {
        uint8_t *r = want + 16u * n_want;
        pu_be64(r, start + (uint64_t)a->offset_cpu_cycles * 4u);
        r[8] = (uint8_t)(a->data >> 8);
        r[9] = (uint8_t)(a->data & 0xFFu);
        r[10] = (uint8_t)(a->addr & 0x0Fu);
        r[11] = a->kind == IVS_ACC_DW ? 2u : 1u;
        memset(r + 12, 0, 4u);
        n_want++;
      }
    }
  }
  return n_want;
}

static int lg_vs_cpu_trace(void)
{
  uint32_t seeds[2] = {0x9E3779B9u, 0x7F4A7C15u};
  uint32_t si;
  for (si = 0u; si < 3u; si++) {
    ivs_ms_inst x;
    ivs_psg_sink sink;
    uint8_t *want = malloc(16u * 4000u);
    uint8_t *log = malloc(16u * 4000u);
    uint64_t n;
    int ok;
    x.storage = NULL;
    x.m = NULL;
    if (want == NULL || log == NULL) {
      free(want);
      free(log);
      return IVS_TEST_FAIL;
    }
    ok = si == 2u ? lg_writer(&x, 300u) : lg_random(&x, seeds[si]);
    if (!ok) {
      free(want);
      free(log);
      return IVS_TEST_FAIL;
    }
    memset(log, LG_SENTINEL, 16u * 4000u);
    ivs_psg_sink_init(&sink, NULL, 0u);
    ivs_psg_sink_log(&sink, log, 16u * 4000u);
    n = lg_trace_units(&x, 1500u, want, 0u, &sink);
    ok = n != UINT64_MAX && n > 50u && sink.log_written == 16u * n && sink.log_records == n &&
         memcmp(log, want, (size_t)(16u * n)) == 0 && ((struct ivs_machine *)x.m)->obs.psglog.count == n;
    if (!ok) {
      ivs_test_diag("program %u: the log differs from the CPU access trace (%llu expected records, %llu logged)",
                    (unsigned)si, (unsigned long long)n, (unsigned long long)sink.log_records);
    }
    free(want);
    free(log);
    ivs_ms_free(&x);
    if (!ok) {
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

static int lg_replay_equals_run(void)
{
  ivs_ms_inst x;
  ivs_run_report rep;
  uint8_t *audio = NULL;
  uint8_t *log = NULL;
  uint8_t *replay = NULL;
  pu_rig r;
  ivs_psg_sink k;
  uint64_t n;
  uint64_t i;
  int rc = IVS_TEST_FAIL;
  x.storage = NULL;
  x.m = NULL;
  LG_CHECK(lg_random(&x, 0xD1B54A32u) && pu_init(&r));
  audio = malloc((size_t)pu_need(x.m, 0u, 60000u));
  log = malloc((size_t)pu_log_need(x.m, 0u, 60000u));
  LG_CHECK(audio != NULL && log != NULL);
  LG_CHECK(pu_run_to_log(x.m, 60000u, audio, log, &rep));
  LG_CHECK(rep.audio_bytes_written > 4000u && rep.psg_log_bytes_written >= 16u * 100u);
  replay = malloc((size_t)rep.audio_bytes_written + 4096u);
  LG_CHECK(replay != NULL);
  memset(replay, LG_SENTINEL, (size_t)rep.audio_bytes_written + 4096u);
  ivs_psg_sink_init(&k, replay, rep.audio_bytes_written + 4096u);
  n = rep.psg_log_bytes_written / 16u;
  for (i = 0u; i < n; i++) {
    const uint8_t *rec = log + 16u * i;
    pu_write(&r, &k, pu_get_be64(rec), (uint32_t)rec[10], ((uint32_t)rec[8] << 8) | rec[9]);
  }
  pu_run(&r, &k, rep.now_ticks);
  LG_CHECK(k.written == rep.audio_bytes_written);
  LG_CHECK(memcmp(replay, audio, (size_t)rep.audio_bytes_written) == 0);
  LG_CHECK(memcmp(&r.obs, &((struct ivs_machine *)x.m)->obs.audio, sizeof r.obs) == 0); /* the AUDIO row and chain */
  LG_CHECK(memcmp(&r.s, &((struct ivs_machine *)x.m)->psg, sizeof r.s) == 0);
  LG_CHECK(memcmp(r.regs, ((struct ivs_machine *)x.m)->mem.psg_reg, sizeof r.regs) == 0);
  rc = IVS_TEST_PASS;
done:
  free(audio);
  free(log);
  free(replay);
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  return rc;
}

static int lg_effect_boundary(void)
{
  /* R7 = $3F (every channel a constant 1), amplitude A written at stamp 80 (5 x 16) to 5 and at stamp 127 (8 x 16 - 1)
   * to 9. Sample k is at tick 16 k and carries level A at byte 0. */
  pu_rig r;
  ivs_psg_sink k;
  uint8_t buf[4u * 10u];
  int rc = IVS_TEST_FAIL;
  if (!pu_init(&r)) {
    return IVS_TEST_FAIL;
  }
  memset(buf, LG_SENTINEL, sizeof buf);
  ivs_psg_sink_init(&k, buf, sizeof buf);
  pu_write(&r, &k, 0u, 8u, 0x3Fu);
  pu_write(&r, &k, 80u, 11u, 5u);
  pu_write(&r, &k, 127u, 11u, 9u);
  pu_run(&r, &k, 160u);
  if (k.written != 40u) {
    return IVS_TEST_FAIL;
  }
  /* samples 1..5 (ticks 16..80): the write stamped 80 is not seen by the sample at 80 */
  LG_CHECK(buf[0] == 0u && buf[4] == 0u && buf[8] == 0u && buf[12] == 0u && buf[16] == 0u);
  /* samples 6 and 7 (ticks 96, 112): level 5; the write stamped 127 is not seen before the sample at 128 */
  LG_CHECK(buf[20] == 5u && buf[24] == 5u);
  /* samples 8 (tick 128) to 10: level 9 */
  LG_CHECK(buf[28] == 9u && buf[32] == 9u && buf[36] == 9u);
  rc = IVS_TEST_PASS;
done:
  return rc;
}

const ivs_test_case ivs_psg_log_cases[] = {
    {"psg.log.framing", lg_framing, NULL},
    {"psg.log.chain-two-records", lg_chain_two_records, NULL},
    {"psg.log.byte-order", lg_byte_order, NULL},
    {"psg.log.reserved-zero", lg_reserved_zero, NULL},
    {"psg.log.source-values", lg_source_values, NULL},
    {"psg.log.value-raw-16bit", lg_value_raw_16bit, NULL},
    {"psg.log.addr-low-nibble", lg_addr_low_nibble, NULL},
    {"psg.log.no-reads-logged", lg_no_reads_logged, NULL},
    {"psg.log.no-synthetic-power-on-records", lg_no_synthetic_power_on, NULL},
    {"psg.log.strictly-increasing", lg_strictly_increasing, NULL},
    {"psg.log.window", lg_window, NULL},
    {"psg.log.vs-cpu-trace", lg_vs_cpu_trace, NULL},
    {"psg.log.replay-equals-run", lg_replay_equals_run, NULL},
    {"psg.log.effect-boundary", lg_effect_boundary, NULL},
};
const size_t ivs_psg_log_count = sizeof ivs_psg_log_cases / sizeof ivs_psg_log_cases[0];
