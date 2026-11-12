/* SPDX-License-Identifier: MIT */
/* PSG register-write log (plan 05-08, D-13, D-14, D-16, D-18, A-01): one 16-byte record per guest write to
 * $01F0..$01FF and the domain-19 chain over those records.
 *
 * The record is written byte by byte, big-endian, never as a struct and never in host byte order:
 *   bytes 0..7   tick     u64, the master tick of the write (the machine bus wrapper's stamp: unit start +
 *                         offset in CPU cycles x ticks per CPU cycle + psg.write_strobe_ticks)
 *   bytes 8..9   value    u16, the raw 16-bit bus value before the register's write mask (C79)
 *   byte 10      addr     the low nibble of the bus address (Intellivision order, $01F0 is 0)
 *   byte 11      source   IVS_PSGLOG_SRC_CPU_WRITE (1) or IVS_PSGLOG_SRC_CPU_DW (2)
 *   bytes 12..15 reserved u32, 0
 * The chain (domain 19) is
 *   chain' = SHA-256('I' 'V' 'S' 'R' || 19 || chain || be64(index) || be32(16) || record)
 * with index the number of records chained before this one and an initial chain of 32 zero bytes. It is
 * updated by every call, whether or not the sink has a log buffer (the counting log sink runs the same
 * code), and depends on the records only, never on how a run is sliced.
 * No static or global state outside the named fault block below (D-07).
 * Named faults (plan 05-08, suite psg): PSG_REG_WRITE_NOT_LOGGED (a write is neither chained nor delivered),
 * PSG_STATIC_CARRY (the record index of the chain is read from a file-scope static that an empty row
 * resynchronises, so two instances that log in turn mix their indices). */
#include "common/ivs_prelude.h"

#include "common/ivs_sha256.h"
#include "psg/psg.h"

#ifdef IVS_FAULT_PSG_STATIC_CARRY
/* Named fault PSG_STATIC_CARRY: the index of the next chain step lives in the file, not in the row. */
static uint64_t ivs_psg_log_carry_index;
#endif

static void ivs_psglog_put_be64(uint8_t *d, uint64_t v)
{
  uint32_t i;
  for (i = 0u; i < 8u; i++) {
    d[i] = (uint8_t)(v >> (56u - 8u * i));
  }
}

void ivs_psg_log_append(ivs_psg_sink *sink, ivs_psg_logrow *row, uint64_t tick, uint32_t value, uint32_t addr,
                        uint32_t source)
{
  uint8_t rec[IVS_PSG_LOG_RECORD_BYTES];
  uint8_t pre[4u + 1u + 32u + 8u + 4u + IVS_PSG_LOG_RECORD_BYTES];
  uint64_t index = row->count;
  uint32_t n = 0u;
#ifdef IVS_FAULT_PSG_REG_WRITE_NOT_LOGGED
  (void)sink;
  (void)row;
  (void)tick;
  (void)value;
  (void)addr;
  (void)source;
  return; /* named fault: a guest write to the PSG leaves no record and no chain step */
#endif
#ifdef IVS_FAULT_PSG_STATIC_CARRY
  if (row->count == 0u) {
    ivs_psg_log_carry_index = 0u;
  }
  index = ivs_psg_log_carry_index;
  ivs_psg_log_carry_index = index + 1u;
#endif
  ivs_psglog_put_be64(rec, tick);
  rec[8] = (uint8_t)((value >> 8) & 0xFFu);
  rec[9] = (uint8_t)(value & 0xFFu);
  rec[10] = (uint8_t)(addr & 0x0Fu);
  rec[11] = (uint8_t)(source & 0xFFu);
  rec[12] = 0u;
  rec[13] = 0u;
  rec[14] = 0u;
  rec[15] = 0u;
  pre[n++] = (uint8_t)'I';
  pre[n++] = (uint8_t)'V';
  pre[n++] = (uint8_t)'S';
  pre[n++] = (uint8_t)'R';
  pre[n++] = 19u;
  memcpy(pre + n, row->chain, 32u);
  n += 32u;
  ivs_psglog_put_be64(pre + n, index);
  n += 8u;
  pre[n++] = 0u;
  pre[n++] = 0u;
  pre[n++] = 0u;
  pre[n++] = (uint8_t)IVS_PSG_LOG_RECORD_BYTES;
  memcpy(pre + n, rec, IVS_PSG_LOG_RECORD_BYTES);
  n += IVS_PSG_LOG_RECORD_BYTES;
  ivs_sha256_digest(pre, n, row->chain);
  row->count += 1u;
  sink->log_records += 1u;
  if (sink->log != NULL) {
    if (sink->log_cap - sink->log_written >= IVS_PSG_LOG_RECORD_BYTES) {
      memcpy(sink->log + sink->log_written, rec, IVS_PSG_LOG_RECORD_BYTES);
      sink->log_written += IVS_PSG_LOG_RECORD_BYTES;
    } else {
      sink->log_overflow = 1u; /* never expected: the capacity rule and the run loop's safety net prove the room */
    }
  }
}
