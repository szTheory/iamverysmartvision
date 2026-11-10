/* SPDX-License-Identifier: MIT */
/* CP-1610 decode table: all 74 rows of data/expected/cp1610-rows.ids, transcribed from the
 * instruction formats and tables of the GI manual (Fig 3.3.1-3.3.5, pdf 50-52; instruction set
 * tables, pdf 53-58). The Phase 1 family table data/cp1610/opcode-families.json is an inventory
 * and the independent comparison of the decode sweep; it is never a value source here.
 * Decode looks only at bits 9..0 of a word (D-07, POL-ILL-01: bits 15..10 are "Future Use",
 * Fig 3.3.1-3.3.5, pdf 50-52, and take no part). Rows are listed most specific first and the
 * first match wins, so every one of the 1024 decles maps to exactly one row and every row has a
 * handler. The suite cpu-sweep compares this table with the Phase 1 family table (D-16).
 *
 * Columns: id, mask, value, eq (bits 5..3 == bits 2..0), words, base cycles, interruptible,
 * handler. Base cycles are the printed counts without modifiers (branches 7 of `7/9`, shifts 6
 * of `6/8`, MOVR, TSTR, MVI@ and the @ forms the unstarred value); the handler adds the rest.
 * Interruptible is 0 where the page prints "Not Interruptable" (EIS, DIS, TCI, CLRC, SETC, pdf
 * 56) or "Shifts are not interruptable" (pdf 53), and for the manual-silent HLT, SDBD and MVO
 * family by the default of C56 / POL-INT-02; every other row is interruptible. */
#include "common/ivs_prelude.h"

#include "cpu/cp1610.h"

#define IVS_ROW(id, mask, value, eq, words, cycles, intr, handler)                                \
  {(id), (mask), (value), (eq), (words), (cycles), (intr), (handler)}

static const ivs_cp1610_row ivs_cp1610_rows[] = {
    /* Internal control, Fig 3.3.4 (pdf 51) and control table (pdf 56): 0000 000 ooo, 4 cycles.
     * SDBD 0000 000 001 is printed on pdf 57. J is the jump group's first word 0000 000 100
     * (Fig 3.3.5, pdf 52; jump table, pdf 56): three words, 12 cycles. */
    IVS_ROW("HLT", 0x3FFu, 0x000u, 0u, 1u, 4u, 0u, IVS_CP1610_OP_HLT),
    IVS_ROW("SDBD", 0x3FFu, 0x001u, 0u, 1u, 4u, 0u, IVS_CP1610_OP_SDBD),
    IVS_ROW("EIS", 0x3FFu, 0x002u, 0u, 1u, 4u, 0u, IVS_CP1610_OP_EIS),
    IVS_ROW("DIS", 0x3FFu, 0x003u, 0u, 1u, 4u, 0u, IVS_CP1610_OP_DIS),
    IVS_ROW("J", 0x3FFu, 0x004u, 0u, 3u, 12u, 1u, IVS_CP1610_OP_JUMP),
    IVS_ROW("TCI", 0x3FFu, 0x005u, 0u, 1u, 4u, 0u, IVS_CP1610_OP_TCI),
    IVS_ROW("CLRC", 0x3FFu, 0x006u, 0u, 1u, 4u, 0u, IVS_CP1610_OP_CLRC),
    IVS_ROW("SETC", 0x3FFu, 0x007u, 0u, 1u, 4u, 0u, IVS_CP1610_OP_SETC),
    /* Single register, Fig 3.3.3 (pdf 51) and register table (pdf 53): 0000 ooo DDD, 6 cycles.
     * GSWD 0000 110 0DD, NOP 0000 110 10N, SIN 0000 110 11N (control table, pdf 56; footnote,
     * pdf 51), RSWD 0000 111 SSS. */
    IVS_ROW("INCR", 0x3F8u, 0x008u, 0u, 1u, 6u, 1u, IVS_CP1610_OP_INCR),
    IVS_ROW("DECR", 0x3F8u, 0x010u, 0u, 1u, 6u, 1u, IVS_CP1610_OP_DECR),
    IVS_ROW("COMR", 0x3F8u, 0x018u, 0u, 1u, 6u, 1u, IVS_CP1610_OP_COMR),
    IVS_ROW("NEGR", 0x3F8u, 0x020u, 0u, 1u, 6u, 1u, IVS_CP1610_OP_NEGR),
    IVS_ROW("ADCR", 0x3F8u, 0x028u, 0u, 1u, 6u, 1u, IVS_CP1610_OP_ADCR),
    IVS_ROW("GSWD", 0x3FCu, 0x030u, 0u, 1u, 6u, 1u, IVS_CP1610_OP_GSWD),
    IVS_ROW("NOP", 0x3FEu, 0x034u, 0u, 1u, 6u, 1u, IVS_CP1610_OP_NOP),
    IVS_ROW("SIN", 0x3FEu, 0x036u, 0u, 1u, 6u, 1u, IVS_CP1610_OP_SIN),
    IVS_ROW("RSWD", 0x3F8u, 0x038u, 0u, 1u, 6u, 1u, IVS_CP1610_OP_RSWD),
    /* Register shift, Fig 3.3.2 (pdf 50) and shift table (pdf 53): 0001 ooo NRR, 6 cycles
     * (8 when N = 1), not interruptible. */
    IVS_ROW("SWAP", 0x3F8u, 0x040u, 0u, 1u, 6u, 0u, IVS_CP1610_OP_SWAP),
    IVS_ROW("SLL", 0x3F8u, 0x048u, 0u, 1u, 6u, 0u, IVS_CP1610_OP_SLL),
    IVS_ROW("RLC", 0x3F8u, 0x050u, 0u, 1u, 6u, 0u, IVS_CP1610_OP_RLC),
    IVS_ROW("SLLC", 0x3F8u, 0x058u, 0u, 1u, 6u, 0u, IVS_CP1610_OP_SLLC),
    IVS_ROW("SLR", 0x3F8u, 0x060u, 0u, 1u, 6u, 0u, IVS_CP1610_OP_SLR),
    IVS_ROW("SAR", 0x3F8u, 0x068u, 0u, 1u, 6u, 0u, IVS_CP1610_OP_SAR),
    IVS_ROW("RRC", 0x3F8u, 0x070u, 0u, 1u, 6u, 0u, IVS_CP1610_OP_RRC),
    IVS_ROW("SARC", 0x3F8u, 0x078u, 0u, 1u, 6u, 0u, IVS_CP1610_OP_SARC),
    /* Register to register, Fig 3.3.1 (pdf 50) and register table (pdf 53): 0 ooo SSS DDD.
     * TSTR is 0010 SSS SSS and JR 0010 SSS 111, both refining MOVR 0010 SSS DDD (6, +1 if DDD
     * is 6 or 7; TSTR +1 if SSS is 6 or 7; JR 7). CLRR 0111 DDD DDD refines XORR. Decle
     * 0010 111 111 is TSTR R7, JR R7 and MOVR R7,R7 at once; the first match (TSTR) owns it
     * and all three readings give the same result. */
    IVS_ROW("TSTR", 0x3C0u, 0x080u, 1u, 1u, 6u, 1u, IVS_CP1610_OP_TSTR),
    IVS_ROW("JR", 0x3C7u, 0x087u, 0u, 1u, 7u, 1u, IVS_CP1610_OP_JR),
    IVS_ROW("MOVR", 0x3C0u, 0x080u, 0u, 1u, 6u, 1u, IVS_CP1610_OP_MOVR),
    IVS_ROW("ADDR", 0x3C0u, 0x0C0u, 0u, 1u, 6u, 1u, IVS_CP1610_OP_ADDR),
    IVS_ROW("SUBR", 0x3C0u, 0x100u, 0u, 1u, 6u, 1u, IVS_CP1610_OP_SUBR),
    IVS_ROW("CMPR", 0x3C0u, 0x140u, 0u, 1u, 6u, 1u, IVS_CP1610_OP_CMPR),
    IVS_ROW("ANDR", 0x3C0u, 0x180u, 0u, 1u, 6u, 1u, IVS_CP1610_OP_ANDR),
    IVS_ROW("CLRR", 0x3C0u, 0x1C0u, 1u, 1u, 6u, 1u, IVS_CP1610_OP_CLRR),
    IVS_ROW("XORR", 0x3C0u, 0x1C0u, 0u, 1u, 6u, 1u, IVS_CP1610_OP_XORR),
    /* Branches (pdf 54): 1000 S0 cccc plus a displacement word, 7/9 cycles (base 7); S is the
     * displacement sign and is not part of the row. NOPP prints 7. BEXT is 1000 S1 EEEE. */
    IVS_ROW("B", 0x3DFu, 0x200u, 0u, 2u, 7u, 1u, IVS_CP1610_OP_BRANCH),
    IVS_ROW("BC", 0x3DFu, 0x201u, 0u, 2u, 7u, 1u, IVS_CP1610_OP_BRANCH),
    IVS_ROW("BOV", 0x3DFu, 0x202u, 0u, 2u, 7u, 1u, IVS_CP1610_OP_BRANCH),
    IVS_ROW("BPL", 0x3DFu, 0x203u, 0u, 2u, 7u, 1u, IVS_CP1610_OP_BRANCH),
    IVS_ROW("BZE", 0x3DFu, 0x204u, 0u, 2u, 7u, 1u, IVS_CP1610_OP_BRANCH),
    IVS_ROW("BLT", 0x3DFu, 0x205u, 0u, 2u, 7u, 1u, IVS_CP1610_OP_BRANCH),
    IVS_ROW("BLE", 0x3DFu, 0x206u, 0u, 2u, 7u, 1u, IVS_CP1610_OP_BRANCH),
    IVS_ROW("BUSC", 0x3DFu, 0x207u, 0u, 2u, 7u, 1u, IVS_CP1610_OP_BRANCH),
    IVS_ROW("NOPP", 0x3DFu, 0x208u, 0u, 2u, 7u, 1u, IVS_CP1610_OP_BRANCH),
    IVS_ROW("BNC", 0x3DFu, 0x209u, 0u, 2u, 7u, 1u, IVS_CP1610_OP_BRANCH),
    IVS_ROW("BNOV", 0x3DFu, 0x20Au, 0u, 2u, 7u, 1u, IVS_CP1610_OP_BRANCH),
    IVS_ROW("BMI", 0x3DFu, 0x20Bu, 0u, 2u, 7u, 1u, IVS_CP1610_OP_BRANCH),
    IVS_ROW("BNZE", 0x3DFu, 0x20Cu, 0u, 2u, 7u, 1u, IVS_CP1610_OP_BRANCH),
    IVS_ROW("BGE", 0x3DFu, 0x20Du, 0u, 2u, 7u, 1u, IVS_CP1610_OP_BRANCH),
    IVS_ROW("BGT", 0x3DFu, 0x20Eu, 0u, 2u, 7u, 1u, IVS_CP1610_OP_BRANCH),
    IVS_ROW("BESC", 0x3DFu, 0x20Fu, 0u, 2u, 7u, 1u, IVS_CP1610_OP_BRANCH),
    IVS_ROW("BEXT", 0x3D0u, 0x210u, 0u, 2u, 7u, 1u, IVS_CP1610_OP_BRANCH),
    /* Output group 1001 MMM SSS: MVO direct (MMM 000, two words, 11, pdf 55), PSHR (MMM 110,
     * 9, pdf 55), MVOI (MMM 111, two words, 9, pdf 57), MVO@ (otherwise, 9, pdf 55). */
    IVS_ROW("MVO", 0x3F8u, 0x240u, 0u, 2u, 11u, 0u, IVS_CP1610_OP_MVO),
    IVS_ROW("PSHR", 0x3F8u, 0x270u, 0u, 1u, 9u, 0u, IVS_CP1610_OP_PSHR),
    IVS_ROW("MVOI", 0x3F8u, 0x278u, 0u, 2u, 9u, 0u, IVS_CP1610_OP_MVOI),
    IVS_ROW("MVO@", 0x3C0u, 0x240u, 0u, 1u, 9u, 0u, IVS_CP1610_OP_MVO_AT),
    /* Input group 1010 MMM DDD: MVI direct (two words, 10), PULR (MMM 110, 11), MVII (MMM 111,
     * two words, 8, pdf 57), MVI@ (otherwise, 8, +3 if MMM = 6), pdf 55. */
    IVS_ROW("MVI", 0x3F8u, 0x280u, 0u, 2u, 10u, 1u, IVS_CP1610_OP_MVI),
    IVS_ROW("PULR", 0x3F8u, 0x2B0u, 0u, 1u, 11u, 1u, IVS_CP1610_OP_PULR),
    IVS_ROW("MVII", 0x3F8u, 0x2B8u, 0u, 2u, 8u, 1u, IVS_CP1610_OP_MVII),
    IVS_ROW("MVI@", 0x3C0u, 0x280u, 0u, 1u, 8u, 1u, IVS_CP1610_OP_MVI_AT),
    /* Arithmetic and logic groups ooo MMM DDD for ADD 1011, SUB 1100, CMP 1101, AND 1110,
     * XOR 1111: direct (MMM 000, two words, 10, pdf 55), immediate (MMM 111, two words, 8,
     * pdf 57), indirect (otherwise, 8, +3 if MMM = 6, pdf 55). */
    IVS_ROW("ADD", 0x3F8u, 0x2C0u, 0u, 2u, 10u, 1u, IVS_CP1610_OP_ADD),
    IVS_ROW("ADDI", 0x3F8u, 0x2F8u, 0u, 2u, 8u, 1u, IVS_CP1610_OP_ADDI),
    IVS_ROW("ADD@", 0x3C0u, 0x2C0u, 0u, 1u, 8u, 1u, IVS_CP1610_OP_ADD_AT),
    IVS_ROW("SUB", 0x3F8u, 0x300u, 0u, 2u, 10u, 1u, IVS_CP1610_OP_SUB),
    IVS_ROW("SUBI", 0x3F8u, 0x338u, 0u, 2u, 8u, 1u, IVS_CP1610_OP_SUBI),
    IVS_ROW("SUB@", 0x3C0u, 0x300u, 0u, 1u, 8u, 1u, IVS_CP1610_OP_SUB_AT),
    IVS_ROW("CMP", 0x3F8u, 0x340u, 0u, 2u, 10u, 1u, IVS_CP1610_OP_CMP),
    IVS_ROW("CMPI", 0x3F8u, 0x378u, 0u, 2u, 8u, 1u, IVS_CP1610_OP_CMPI),
    IVS_ROW("CMP@", 0x3C0u, 0x340u, 0u, 1u, 8u, 1u, IVS_CP1610_OP_CMP_AT),
    IVS_ROW("AND", 0x3F8u, 0x380u, 0u, 2u, 10u, 1u, IVS_CP1610_OP_AND),
    IVS_ROW("ANDI", 0x3F8u, 0x3B8u, 0u, 2u, 8u, 1u, IVS_CP1610_OP_ANDI),
    IVS_ROW("AND@", 0x3C0u, 0x380u, 0u, 1u, 8u, 1u, IVS_CP1610_OP_AND_AT),
    IVS_ROW("XOR", 0x3F8u, 0x3C0u, 0u, 2u, 10u, 1u, IVS_CP1610_OP_XOR),
    IVS_ROW("XORI", 0x3F8u, 0x3F8u, 0u, 2u, 8u, 1u, IVS_CP1610_OP_XORI),
    IVS_ROW("XOR@", 0x3C0u, 0x3C0u, 0u, 1u, 8u, 1u, IVS_CP1610_OP_XOR_AT),
};

_Static_assert(sizeof ivs_cp1610_rows / sizeof ivs_cp1610_rows[0] == 74u, "74 decode rows");

const ivs_cp1610_row *ivs_cp1610_decode(uint16_t decle)
{
  uint32_t d = (uint32_t)decle & 0x3FFu;
  size_t i;
  for (i = 0u; i < sizeof ivs_cp1610_rows / sizeof ivs_cp1610_rows[0]; i++) {
    const ivs_cp1610_row *row = &ivs_cp1610_rows[i];
    if ((d & row->mask) != row->value) {
      continue;
    }
    if (row->eq != 0u && ((d >> 3) & 7u) != (d & 7u)) {
      continue;
    }
    return row;
  }
  return NULL;
}
