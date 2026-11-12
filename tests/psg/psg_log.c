/* SPDX-License-Identifier: MIT */
/* Suite psg, family log (AUD-02, plan 05-08, D-13, D-14, D-18, A-01): every guest write to $01F0..$01FF becomes one
 * 16-byte record in a caller log buffer and a step of the domain-19 chain (digest domain 19, IVS_DIGEST_PSGLOG).
 * Record, written byte by byte big-endian: bytes 0..7 tick, 8..9 value (the raw 16-bit bus value), 10 addr (low
 * nibble of the bus address), 11 source (1 CPU write, 2 interrupt-entry write), 12..15 zero. Chain:
 *   chain' = SHA-256('I' 'V' 'S' 'R' || 19 || chain || be64(index) || be32(16) || record), index = records before.
 * Expected values here come from the cycle tables of docs/cp1610 (offsets), the observed unit starts of a seam-
 * stepped twin, hand-written byte literals, the CPU's own access trace and a replay into a fresh PSG; never from
 * a core output read back as the expectation.
 *
 * Authoring recipe of the chain literal of psg.log.framing (POSIX sh or dash, in a mktemp -d directory; nothing is
 * committed; the values were cross-checked with a second, independent hash implementation):
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
 *   C1 = 198b9bed934a9ff42b5ba79751b692b85559aa015622a9dea874e119e5eb9bba   (shasum -a 256 agrees)
 *   D1 = 8b1cff42d5105b7004a6dfa301d102efd4675c27c74b9a2ac2f1b5ae5c11f6eb
 * These literals check framing and order, not hardware behaviour.
 *
 *   psg.log.framing  one MVO of $03FF to $01F4 gives one record, 16 bytes, equal to the hand-written literal, report
 *                    fields 16 and 0, and the chain row and domain-19 digest equal the literals above */
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

/* Steps m through the seam (counting sink) until the PSGLOG row holds more than before records, returning the machine
 * time before that unit in *start (the observed unit start) and the machine time after it in *end. 1 on success. */
static int lg_find_write(ivs_machine *m, uint64_t *start, uint64_t *end)
{
  uint64_t before_count = ((struct ivs_machine *)m)->obs.psglog.count;
  ivs_psg_sink k;
  uint32_t guard;
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

static const uint8_t lg_c1[32] = {
    0x19, 0x8b, 0x9b, 0xed, 0x93, 0x4a, 0x9f, 0xf4,
    0x2b, 0x5b, 0xa7, 0x97, 0x51, 0xb6, 0x92, 0xb8,
    0x55, 0x59, 0xaa, 0x01, 0x56, 0x22, 0xa9, 0xde,
    0xa8, 0x74, 0xe1, 0x19, 0xe5, 0xeb, 0x9b, 0xba,
};
static const uint8_t lg_d1[32] = {
    0x8b, 0x1c, 0xff, 0x42, 0xd5, 0x10, 0x5b, 0x70,
    0x04, 0xa6, 0xdf, 0xa3, 0x01, 0xd1, 0x02, 0xef,
    0xd4, 0x67, 0x5c, 0x27, 0xc7, 0x4b, 0x9a, 0x2a,
    0xc2, 0xf1, 0xb5, 0xae, 0x5c, 0x11, 0xf6, 0xeb,
};

static int lg_framing(void)
{
  /* The tracer: stand-in program C writes R0 = $03FF to $01F4 (the coarse tone period of channel A: four bits, so
   * the register holds $000F) once, then reads it back and stores it to $0200, then loops. The unit that holds the
   * direct MVO starts at S; the write is its access at CPU cycle 6 (docs/cp1610 cycle tables: a direct MVO is
   * 11 cycles: fetch 4, address word 3 .. the write begins at cycle 6), stamped S + 6 x 4 master ticks. */
  ivs_ms_inst twin;
  ivs_ms_inst x;
  ivs_run_report rep;
  uint8_t *log = NULL;
  uint8_t want[16];
  uint8_t chain_row[32];
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
  LG_CHECK(ivs_machine_state_digest(x.m, IVS_DIGEST_PSGLOG, chain_row) == IVS_OK);
  LG_CHECK(memcmp(chain_row, lg_d1, 32u) == 0);
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

const ivs_test_case ivs_psg_log_cases[] = {
    {"psg.log.framing", lg_framing, NULL},
};
const size_t ivs_psg_log_count = sizeof ivs_psg_log_cases / sizeof ivs_psg_log_cases[0];
