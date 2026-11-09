# CP-1610 reference notes

Project-owned notes on the instruction set, addressing, stack, reset, interrupt and bus behaviour of the CPU family used by the Intellivision.
Every fact carries a page cite to one primary source and a label: [OBS] means the page image was read in this phase, [INF] means the statement is an inference.
These notes are an inventory of what the manual says, not a statement about any implementation.

## Source and method

The only source for these notes is SRC-gi-cp1600-um-1975, the General Instrument CP-1600 Microprocessor Users Manual of May 1975.
Its sha256 and size are pinned once in `data/sources.json`; these notes do not repeat them.
The scan has no printed page numbers, so every cite uses the pdf page index, for example `§3.4.6, pdf 62`.
Encodings, cycle counts and status bits were read from rendered page images and are [OBS] only where the image was read.
The OCR text layer is damaged in tables and was used only to locate pages, never as a source for a value.
No other emulator, documentation or table was used as a source for any value here.
The manual describes the CP-1600, and applying it to the CP-1610 is an inference [INF].
The electrical specifications section is headed Tentative (`§2.4, pdf 38`) [OBS], and none of its values are used here.
Pages are cited as `pdf <n>`; section numbers are the ones printed on the page, and a section number printed as `C.5` on pdf 24 is cited as printed.
The rendered pages live only in the gitignored local cache and are never committed.

## Citation form

A cite is written `§<section>, pdf <n>`, for example `§3.4.6, pdf 62` for the stack operations page.
Figures are cited by their printed figure number plus the pdf page, for example `Fig. 3.2.1, pdf 46`.
A page without a printed section heading is cited by pdf page alone.
A label [OBS] follows a fact read from an image in this phase, and [INF] follows a fact derived from one or more such readings.
A cite never carries a printed page number because none exists in the scan.

## Stack operations (C17)

When R6 addresses storage indirectly, post-increment and pre-decrement are performed automatically (`§3.4.6, pdf 62`) [OBS].
When data moves out of the CPU via R6, it is placed in the cell addressed by R6 and then R6 is incremented by one (`§3.4.6, pdf 62`) [OBS].
When data moves into the CPU via R6, R6 is first decremented and then the data is fetched from the cell it addresses (`§3.4.6, pdf 62`) [OBS].
The page describes this as a dynamic last-in, first-out storage area commonly called a stack (`§3.4.6, pdf 62`) [OBS].
PSHR r is the assembler form of MVO@ r, R6 and PULR r is the assembler form of MVI@ R6, r (`§3.4.6, pdf 62`) [OBS].
Mode 6 is described as stack operations: indirect through R6, post-incremented for output operations (push) and pre-decremented for input operations (pull) (`§3.2.1, pdf 46`) [OBS].
The same page states that the stack is maintained by R6 and expands upward in external memory, from low address to high address (`§3.2.1, pdf 46`) [OBS].
The effective-address figure gives the MVO-only case as: EA takes (R6), then (R6) is incremented by one (`Fig. 3.2.4, pdf 48`) [OBS].
The same figure gives the non-MVO case as: (R6) is decremented by one, then EA takes (R6) (`Fig. 3.2.4, pdf 48`) [OBS].
The interrupt sequence stores the program counter at the address held in R6, the stack pointer, and increments the stack pointer after the store (`§C.6, pdf 24`) [OBS].
The three pages agree: the stack grows toward higher addresses, a push writes at R6 and then increments it, and a pull decrements R6 and then reads. [claim:CL-0001]
Because the interrupt entry uses the same post-increment rule, the first interrupt-saved word lands at the address held in R6 at acceptance [INF].
The image reading in this phase matches the behaviour recorded in decision D-21, so the status is resolved from a primary source.
C17 status: resolved-primary

## Instruction formats

Every instruction word is a 10-bit decle in bits 9..0; bits 15..10 are marked Future Use in every format figure (`pdf 46-52`) [OBS].
Bit 9 distinguishes the two groups: bit 9 = 1 is the External Reference group (memory access and conditional branch), bit 9 = 0 is the Internal Reference group (`§3.3, pdf 50`) [OBS].

### Register to register

The format is bit 9 = 0, bits 8..6 an operation code, bits 5..3 SSS (source) and bits 2..0 DDD (destination) (`§3.3.1, Fig. 3.3.1, pdf 50`) [OBS].
The operation codes are 010 MOVR, 011 ADDR, 100 SUBR, 101 CMPR, 110 ANDR and 111 XORR (`§3.3.1, pdf 50`) [OBS].

### Register shift

The format is bits 9..6 = 0001, then the mode bits B (bit 5), L (bit 4), A (bit 3), N (bit 2) and a two-bit register field DD in bits 1..0 (`§3.3.2, Fig. 3.3.2, pdf 50`) [OBS].
B = 1 is a right shift and B = 0 a left shift (`§3.3.2, pdf 50`) [OBS].
L = 1 is with link bits and L = 0 without (`§3.3.2, pdf 50`) [OBS].
A = 1 is arithmetic and A = 0 is logical or rotate (`§3.3.2, pdf 50`) [OBS].
N = 1 is a two-position shift and N = 0 a one-position shift (`§3.3.2, pdf 50`) [OBS].
The page notes that shifts can only be performed on R0 to R3, which matches the two-bit register field (`§3.3.2, pdf 50`) [OBS].
The mapping of the B, L and A combinations to mnemonics is a table on a later page and is not restated here.

### Single register

The format is bit 9 = 0, bits 8..6 = 000, bits 5..3 an operation code and bits 2..0 DDD (`§3.3.3, Fig. 3.3.3, pdf 51`) [OBS].
The operation codes are 001 INCR, 010 DECR, 011 COMR, 100 NEGR, 101 ADCR, 110 GSWD and 111 RSWD (`§3.3.3, pdf 51`) [OBS].
Operation code 000 is not listed in this figure; the pattern with bits 5..3 = 000 belongs to the internal control group below (`§3.3.4, pdf 51`) [INF].
A footnote on the same page gives true Get Status Word operation only for R0 to R3 as the register specification (`§3.3.3, pdf 51`) [OBS].
The footnote says R4 or R5 results in a NOP instruction (`§3.3.3, pdf 51`) [OBS].
The footnote says R6 or R7 results in a NOP internal to the CPU and causes a pulse on the PCIT pin, usable to form a software interrupt or trap instruction (`§3.3.3, pdf 51`) [OBS].

### Internal control

The format is bits 9..3 = 0000000 and bits 2..0 an operation code (`§3.3.4, Fig. 3.3.4, pdf 51`) [OBS].
The operation codes are 000 HLT, 001 SDBD, 010 EIS, 011 DIS, 101 TCI, 110 CLRC and 111 SETC (`§3.3.4, pdf 51`) [OBS].
Operation code 100 is not listed in this figure; the pattern 0000 000 100 is the first decle of the jump group (`§3.3.5, pdf 52`) [INF].

### Jump and jump-and-save-return

The jump group occupies three consecutive memory locations (`§3.3.5, pdf 52`) [OBS].
The first decle is 0000 000 100 (`Fig. 3.3.5, pdf 52`) [OBS].
The second decle holds the field BB in bits 9..8, six address bits in bits 7..2 and the field DE in bits 1..0 (`Fig. 3.3.5, pdf 52`) [OBS].
The third decle holds ten address bits in bits 9..0 (`Fig. 3.3.5, pdf 52`) [OBS].
The destination address takes bits 7..2 of the second decle as bits 15..10 and bits 9..0 of the third decle as bits 9..0 (`§3.3.5, pdf 52`) [OBS].
The page says the interrupt system can be enabled or disabled by bits 1..0 of the second decle, the DE field (`§3.3.5, pdf 52`) [OBS].
On direct jumps the return address is not saved (`§3.3.5, pdf 52`) [OBS].
On subroutine jumps the return address is saved in register 4, 5 or 6, designated by BB plus 4 (`§3.3.5, pdf 52`) [OBS].
The saved address is printed as the program counter plus an offset whose last digit is smudged in the scan; 3 is consistent with the three-location instruction length [INF].
If BB = 3 the designated register becomes R7, the program counter, which the page describes as the jump instruction (`§3.3.5, pdf 52`) [OBS].
The summary table gives DE = 00 for J and JSR, DE = 01 for JE and JSRE, and DE = 10 for JD and JSRD; DE = 11 does not appear in the table (`pdf 56`) [OBS].
The same table prints BB = 11 for J, JE and JD and marks the JSR family as BB not equal to 11; all six forms take 12 cycles (`pdf 56`) [OBS].
JE and JSRE enable the interrupt system, JD and JSRD disable it, and J and JSR leave it unchanged (`pdf 56`) [OBS].

## Addressing modes

The MMM mode field of an External Reference instruction is bits 5..3, the destination or source register DDD is bits 2..0, and the operation code is bits 8..6 (`§3.2.1, Fig. 3.2.1, pdf 46`) [OBS].
The External Reference operation codes are 001 MVO, 010 MVI, 011 ADD, 100 SUB, 101 CMP, 110 AND and 111 XOR; the eighth External Reference instruction is the conditional branch (`§3.2.1, pdf 46`; `§3.2.2, pdf 47`) [OBS].
The register named by DDD is the source of data for output operations and the destination for input operations (`§3.2.1, pdf 46`) [OBS].

| MMM | Mode | Cite |
|-----|------|------|
| 000 | Direct: the address is in the next memory location | `Fig. 3.2.1, pdf 46`; `Fig. 3.2.3, pdf 48` |
| 001 | Indirect through R1 | `Fig. 3.2.1, pdf 46` |
| 010 | Indirect through R2 | `Fig. 3.2.1, pdf 46` |
| 011 | Indirect through R3 | `Fig. 3.2.1, pdf 46` |
| 100 | Indirect through R4, then R4 is incremented | `Fig. 3.2.1, pdf 46`; `Fig. 3.2.4, pdf 48` |
| 101 | Indirect through R5, then R5 is incremented | `Fig. 3.2.1, pdf 46`; `Fig. 3.2.4, pdf 48` |
| 110 | Stack through R6: post-increment on output, pre-decrement on input | `Fig. 3.2.1, pdf 46`; `Fig. 3.2.4, pdf 48` |
| 111 | Immediate: the data is in the next memory location | `Fig. 3.2.1, pdf 46`; `Fig. 3.2.5, pdf 49` |

The table is [OBS] for every row.
In direct mode the instruction occupies two words, PC becomes PC + 2, and EA is formed from the low six bits of the second word, whose width depends on the memory word size, followed by ten address bits (`Fig. 3.2.3, pdf 48`) [OBS].
In indirect modes 001 to 110 the program counter advances by one (`Fig. 3.2.4, pdf 48`) [OBS].
In immediate mode the instruction occupies two words, PC becomes PC + 2 and EA is PC + 1 (`Fig. 3.2.5, pdf 49`) [OBS].
The program counter is incremented before the operation code is performed, so a destination of R7 modifies the incremented counter and acts as a branch (`§3.2.3, pdf 48`) [OBS].
The effective address addresses the first operand, and the second operand is always an internal register that receives the result (`§3.2.3, pdf 48`) [OBS].

### Conditional branch format

The format is bit 9 = 1, bits 8..6 = 000, bit 5 the direction bit, bit 4 the I/E bit and bits 3..0 the condition code, followed by an address displacement word (`§3.2.2, Fig. 3.2.2, pdf 47`) [OBS].
Bit 4 = 1 selects an external condition code, giving 32 conditions in two groups of 16 (`§3.2.2, pdf 47`) [OBS].
If the direction bit is 0, the displacement is added to PC + 2; if it is 1, the displacement is one's complemented and then added to PC + 2 (`§3.2.2, pdf 47`) [OBS].
A false condition continues with the next sequential instruction (`§3.2.2, pdf 47`) [OBS].

## SDBD (double-byte data)

SDBD takes 4 cycles, has the pattern 0000 000 001, and sets Double Byte Data for the next instruction, which must be an External Reference instruction (`pdf 57`) [OBS].
The manual says the assembler normally supplies SDBD when the data is wider than the memory word (`pdf 57`) [OBS].
With SDBD in force, two words are accessed: the least significant byte comes from the low eight bits of the first word and the most significant byte from the low eight bits of the second word (`§3.2.1, pdf 47`) [OBS].
Double-byte data may be accessed through indirect modes 4, 5 and 7 (`§3.2.1, pdf 47`) [OBS].
If modes 1, 2 or 3 are used, only one word is accessed and both bytes come from the low eight bits of that same word (`§3.2.1, pdf 47`; `pdf 57`) [OBS].
Modes 0 and 6 are not supported in the double-byte operation by this version of the CP-1600 (`§3.2.1, pdf 47`; `Fig. 3.2.6, pdf 49`; `pdf 57`) [OBS].
For indirect SDBD with modes 4 and 5, the low byte comes from the word at the address in the register, the register is incremented, the high byte comes from the next word, and the register is incremented again (`Fig. 3.2.6, pdf 49`) [OBS].
For indirect SDBD with modes 1 to 3 the same register address supplies both bytes, and the register is not incremented (`Fig. 3.2.6, pdf 49`) [OBS].
The SDBD word precedes the instruction, so the External Reference instruction is at PC and SDBD at PC - 1 in the figure (`Fig. 3.2.6, pdf 49`) [OBS].
For immediate SDBD the program counter advances by 3 after the instruction word, with the low byte at PC + 1 and the high byte at PC + 2 (`Fig. 3.2.7, pdf 49`) [OBS].
The immediate SDBD figure marks the upper bits of each data word as Not Used (`Fig. 3.2.7, pdf 49`) [OBS].
If the user writes SDBD explicitly, the data word is always generated to fill two bytes (`Fig. 3.2.7, pdf 49`) [OBS].
The indirect double-byte table prints one 4-cycle row for SDBD followed by one row per instruction with 10 cycles: MVI@, ADD@, SUB@, CMP@, AND@ and XOR@ (`pdf 57`) [OBS].
The page does not say whether the printed 10 includes the 4 cycles of the SDBD row; the one-row-per-pair layout suggests it does not [INF].
The immediate double-byte table prints one count of 14 cycles for each of MVII, ADDI, SUBI, CMPI, ANDI and XORI, spanning the SDBD decle, the instruction decle and the two data words (`pdf 58`) [OBS].
The single-byte immediate forms take 8 cycles for those six instructions and 9 for MVOI (`pdf 57`) [OBS].
In the double-byte immediate rows the low byte is marked L and the upper byte U, with XX marked as don't care (`pdf 58`) [OBS].
MVOI is not listed in the immediate double-byte table (`pdf 58`) [OBS].
The XOR@ row of the indirect double-byte table prints the opcode field as the three digits `111` instead of four; the other five rows print four digits, and the single-byte XOR@ row on `pdf 55` prints `1111`, so the reading is 1111 [INF].

## Unsupported and NOP-like encodings

MVO@ with SSS = MMM equal to 4, 5, 6 or 7 is marked not supported (`pdf 55`) [OBS].
The same MVO@ row gives the pattern 1001 MMM SSS and 9 cycles (`pdf 55`) [OBS].
PSHR is the MVO@ form with MMM = 110 and takes 9 cycles; PULR is the MVI@ form with MMM = 110 and takes 11 cycles, which is the 8-cycle MVI@ count plus the 3 added when MMM = 6 (`pdf 55`) [OBS].
GSWD has the pattern 0000 110 0DD and takes 6 cycles (`pdf 56`) [OBS].
GSWD with R4 or R5 is a NOP instruction, and with R6 or R7 it is a NOP internal to the CPU plus a pulse on the PCIT pin (`§3.3.3, pdf 51`) [OBS].
The GSWD description on the summary table places C in bits 4 and 12, OV in bits 5 and 13, Z in bits 6 and 14 and S in bits 7 and 15, with bits 0 to 3 and 8 to 11 set to 0 (`pdf 56`) [OBS].
RSWD has the pattern 0000 111 SSS, takes 6 cycles, and restores bit 4 to C, bit 5 to OV, bit 6 to Z and bit 7 to S (`pdf 56`) [OBS].
NOP has the pattern 0000 110 10N and takes 6 cycles (`pdf 56`) [OBS].
SIN has the pattern 0000 110 11N and takes 6 cycles, and it pulses the PCIT pin (`pdf 56`) [OBS].
The N bit of NOP and SIN is shown as an operand written in angle brackets, `<n>`, and the page does not say what N selects (`pdf 56`) [OBS].
The jump group has no listed DE value 11 in the second decle; this is an operand-level gap, not a gap in the decle patterns (`pdf 56`) [OBS].
HLT has the pattern 0000 000 000, takes 4 cycles, and is described as halting after the next instruction is executed, resuming on control start (`pdf 56`) [OBS].

## Interruptibility (C19)

The register shift table states that shifts are not interruptable (`pdf 53`) [OBS].
The same note says shift-right instructions set the S flip-flop from bit 7 of the result and that two-position shifts add 2 cycles (`pdf 53`) [OBS].
EIS, DIS, TCI, CLRC and SETC each carry the words Not Interruptible in their description (`pdf 56`) [OBS].
The HLT description on `pdf 56` says nothing about interruptibility, so HLT is `manual-silent`.
The SDBD description on `pdf 57` says nothing about interruptibility, so SDBD is `manual-silent`.
The MVO, MVO@, MVOI and PSHR descriptions on `pdf 55` and `pdf 57` say nothing about interruptibility, so MVO is `manual-silent`.
C19: partial.
The manual carries a not-interruptible marking for shifts and for EIS, DIS, TCI, CLRC and SETC only; the community list of non-interruptible instructions is not corroborated for HLT, SDBD and MVO. [claim:CL-0004]
The register-to-register, memory and jump summary tables on the pages read for this section carry no other interruptibility marking [OBS].

## Reset, MSYNC and IAB

MSYNC* is a negative-logic input that synchronizes the internal timing logic to the two main phases; it must stay active (low) for at least ten milliseconds after power is applied and becomes inactive (high) on a rising edge of Ø1, which corresponds to an internal TS3 (`pdf 12`) [OBS].
The pin description also says MSYNC* disables the interrupt system (`pdf 12`) [OBS].
On power-up with MSYNC* low, the CPU is forced to an inactive state: D0 to D15 are high impedance, the bus control signals issue NACT, all other signals are inactive, and the interrupt system is disabled (`§C.1, pdf 21`) [OBS].
The page states that all eight internal registers and the status bits contain arbitrary, undefined data at that point (`§C.1, pdf 21`) [OBS].
When MSYNC* goes inactive (high), the bus control signals issue IAB and the CPU inputs the starting address of the main program from the bus into the PC (`§C.1, pdf 21`) [OBS].
The start PC is therefore supplied by a device over the bus by IAB; the manual describes no fixed reset vector inside the CPU (`§C.1, pdf 21`; `Fig. 2-6, pdf 22`) [OBS].
The page adds that the initialization address can be defined by the user at any desired bus address or can be the default address resulting from the logical state of the non-driven bus (`§C.1, pdf 21`) [OBS].
The flow diagram shows MSYNC* leading to Input Starting PC (IAB), then into the fetch loop that begins with Output Current PC (BAR) (`Fig. 2-6, pdf 22`) [OBS].
The initialization timing diagram shows NACT while MSYNC* is low, then NACT, IAB, three NACT microcycles and BAR, with the first read marked at the BAR (`pdf 27`) [OBS].
Counting the four-slot TS1 to TS4 groups printed on that diagram, IAB falls in the second microcycle after MSYNC* rises and the first BAR in the sixth [INF].
Because registers and status bits are undefined after initialization, no value for R7 other than what the bus supplies, and no value for any other register or flag, is a manual fact [INF].
The profile field `reset.r7` is a community-sourced default and not a manual fact, and the profile fields `reset.registers_other` and `reset.flags` record zero as a chosen default on top of the undefined state stated above.
A hardware read-back is the evidence that would replace those defaults.

## Interrupt sequence

The CPU enters the interrupt sequence after all bus requests have been resolved and either INTR* is active, or INTRM* is active and the internal interrupt flip-flop INTFF is set (`§C.6, pdf 24`) [OBS].
INTR* is always honored and is the highest-priority request line; INTRM* is honored only while INTFF is set, and INTFF is controlled by EIS, DIS and several other instructions (`pdf 12`) [OBS].
A request is honored at the completion of an interruptible instruction (`pdf 12`; `pdf 14`) [OBS].
The CPU samples INTR* and INTRM* every TS1 time (`§C.6, pdf 25`) [OBS].
The first micro state issues INTAK, and the CPU places the contents of R6, the stack pointer, in the address register (`§C.6, pdf 24`) [OBS].
The CPU then outputs the PC as data with DW and DWS, storing it at the location the stack pointer addresses, and the stack pointer is incremented after the store (`§C.6, pdf 24`) [OBS].
Finally the CPU issues IAB, which brings the starting address of the interrupt service routine into the PC from the interrupting device, and returns to the fetch sequence (`§C.6, pdf 25`) [OBS].
Only the PC is stored by the sequence as described; the status word is not stored by the CPU, and the software context-save example saves it with GSWD and PSHR (`§C.6, pdf 24`; `§3.4.7, pdf 62`) [OBS].
The text on the pages read does not state whether INTFF is cleared on acceptance (`pdf 12`; `§C.6, pdf 24`; `pdf 25`) [OBS].
The interrupt timing diagram shows the bus control sequence INTAK, NACT, DW, DWS, NACT, IAB, three NACT microcycles and then BAR for the first fetch of the service routine (`pdf 35`) [OBS].
Counting the four-slot groups printed on that diagram, nine microcycles precede the BAR of the first service-routine fetch, which would be the entry count if each cell is one microcycle [INF].
The instruction summary tables do not tabulate an interrupt-entry cycle count, so the nine is derived and not printed (`pdf 53-58`) [OBS].
The TCI pulse is generated by the TCI instruction to terminate the highest-priority interrupt in service (`pdf 13`) [OBS].
The manual describes a fixed interrupt address vector generated by IAB in the simplest case, and daisy-chained acknowledgement through INTAK in the nested case (`§4.5, pdf 75`) [OBS].
The SIN instruction pulses PCIT as an output, which interfacing hardware can return as an interrupt request, and the interrupt is acknowledged at the end of the SIN instruction (`pdf 13`) [OBS].
PCIT as an input prevents the program counter from incrementing during the fetch phase of all instructions (`pdf 13`) [OBS].

## Bus requests

A bus request is honored after the completion of an interruptible instruction (`pdf 14`; `§C.5, pdf 24`) [OBS].
If BUSRQ* is pulled low before the completion of the current interruptible instruction, the CPU honors it by driving BUSAK* low (`§C.5, pdf 24`) [OBS].
The CPU then enters a WAIT state that can be of indefinite duration, remains in it until BUSRQ* returns to logic 1, and does not use the bus; the bus control signals issue NACT and all other signals are inactive (`§C.5, pdf 24`) [OBS].
BUSRQ* can be activated asynchronously with respect to the CPU input clock, and the CPU synchronizes it digitally during every TS1 time (`§C.5, pdf 24`) [OBS].
If there are neither bus requests nor interrupt requests, the CPU returns to the fetch sequence (`§C.5, pdf 24`) [OBS].
The CPU honors bus requests before it honors interrupt requests (`pdf 25`) [OBS].
The state flow diagram shows the same order: after an interruptible instruction the bus request test comes first and the interrupt request test follows (`Fig. 2-6, pdf 22`) [OBS].
The flow diagram also shows that an uninterruptible instruction returns to the fetch loop without passing either test (`Fig. 2-6, pdf 22`) [OBS].

## Start and stop (HLT)

The CPU has an internal start/stop flip-flop toggled by the negative edge of the STPST pin, so alternate pulses switch between the RUN and STOP modes (`§4.4, pdf 74`) [OBS].
The HALT output indicates the run/stop condition, and the CPU stops only after the completion of an instruction (`§4.4, pdf 74`) [OBS].
The pin description says that the negative transition of STPST stops a running CPU only after the completion of an interruptible instruction (`pdf 14`) [OBS].
The HALT output is a high-active signal that indicates the stopped mode, which can result from STPST or from the execution of a HALT instruction (`pdf 14`) [OBS].
The HLT instruction takes 4 cycles and is described as halting after the next instruction is executed, resuming on control start (`pdf 56`) [OBS].
The figure on `pdf 74` shows MSYNC* and the HLT instruction acting on the same flip-flop that STPST toggles, with the HALT output formed from that flip-flop and the microcontrol sequencing logic [OBS].
The wording of `pdf 14` and `pdf 74` differs on whether the stop waits for any instruction or only an interruptible one, and the HLT row on `pdf 56` makes no statement about it; this is carried as an open item.

## Branch timing

The branch table prints the cycle count as `7/9` for B, for each condition row that carries a count, and for BEXT, with no label saying which figure applies when the branch is taken (`pdf 54`) [OBS].
NOPP, the two-word no-operation form, prints a single count of 7 (`pdf 54`) [OBS].
The unconditional B also prints `7/9`, so the two figures cannot be read as a plain not-taken and taken pair for every row [INF].
A weak reading, that 7 is the count when no branch occurs because NOPP never branches and prints 7, is not stated by the manual [INF].
The only branch timing figure located among pdf 26 to 37 by page title is the Branch on External Instruction diagram on `pdf 32`.
That figure shows the bus control sequence BAR, NACT, DTB, NACT and then BAR with the label output PC + 1 to fetch the displacement (`pdf 32`) [OBS].
The figure ends during that displacement fetch and shows no execute tail, so it does not resolve which count applies to a taken branch (`pdf 32`) [OBS].
The branch format page says a false result continues with the next sequential instruction and a true result transfers control (`§3.2.2, pdf 47`) [OBS].
The same table prints the displacement range of the unconditional branch as +1025 to -1024 (`pdf 54`) [OBS].

## Discrepancy candidates

Plan 01-04 turns these into numbered log entries; no C numbers are assigned here.

- Topic: branch cycle count printed `7/9` with no taken or not-taken label; what the manual says: `7/9` for B, the condition rows and BEXT, and 7 for NOPP (`pdf 54`); `pdf 32` shows only the fetch of a BEXT and does not resolve it; resolving evidence: a timing figure or table that labels the two counts, or a bus trace of taken and not-taken branches on hardware.
- Topic: interrupt-entry cycle count is not tabulated; what the manual says: the sequence is INTAK, NACT, DW, DWS, NACT, IAB, three NACT and then the first fetch BAR (`pdf 35`); resolving evidence: a printed entry count in the manual, or a bus trace of an interrupt acceptance.
- Topic: the manual describes the CP-1600, not the CP-1610; what the manual says: the pages read describe the CP-1600 throughout and the electrical specifications section is headed Tentative (`pdf 12`; `pdf 14`; `§2.4, pdf 38`); resolving evidence: a CP-1610 datasheet page that states the same encodings, cycle counts and sequences.
- Topic: the reset PC is supplied by a device over the bus through IAB and not by a fixed vector; what the manual says: the CPU inputs the starting address from the bus at MSYNC* release, with registers and status undefined (`§C.1, pdf 21`; `pdf 27`); resolving evidence: the console memory map and bus behaviour on hardware, and a read-back of the registers after reset.
- Topic: the Fig. 3.3.2 mode-bit names do not match the shift mnemonic names for left shifts; what the manual says: A = 0 is logical or rotate, and the SLL row is the pattern with B L A = 0 0 1, which is A = 1, while the SWAP row has A = 0 (`§3.3.2, pdf 50`; `pdf 53`); resolving evidence: a Phase 2 reading of the shift table together with the figure, with the table taken as the operative statement.
- Topic: the cycle counts of SDBD indirect forms may or may not include the 4-cycle SDBD row; what the manual says: a 4-cycle SDBD row followed by a 10-cycle instruction row (`pdf 57`), against a single 14 for the immediate pairs (`pdf 58`); resolving evidence: a worked cycle example in the manual or a bus trace.
- Topic: stop behaviour wording; what the manual says: STPST stops the CPU after an interruptible instruction (`pdf 14`) while the start/stop section says after the completion of an instruction (`pdf 74`); resolving evidence: the HLT row of the instruction table and a bus trace.

## Open items for Phase 2

- Branch taken and not-taken cycle column: `7/9` is unlabelled on `pdf 54`, and the one branch timing figure on `pdf 32` does not settle it.
- SDBD modes 0 and 6 policy: the manual marks them not supported (`pdf 57`) and says nothing about the resulting behaviour, so the policy for those encodings is a Phase 2 decision.
- Interrupt-acknowledge mechanism supplying the vector: the CPU reads the new PC from the bus at IAB (`pdf 25`), so the machine model needs a device or bus rule that supplies it.
- Reset PC source: the same bus rule supplies the starting address at MSYNC* release (`§C.1, pdf 21`).
- Per-opcode interruptibility for HLT, SDBD and MVO: the manual is silent (`pdf 55`; `pdf 56`; `pdf 57`), recorded under C19 as partial.
- Whether INTFF is cleared on interrupt acceptance: not stated on `pdf 12`, `pdf 24` or `pdf 25`.
- Whether the printed 10 for SDBD indirect forms includes the SDBD row (`pdf 57`).

## Decode coverage

The family table `data/cp1610/opcode-families.json` is checked by the `cp1610` gate step on every run.
The checker result line is `patterns=1024 unclaimed=0 ambiguous=0 coincidences=1`: every 10-bit decle pattern maps to exactly one most-specific row. [claim:CL-0005]
The one coincidence is 0010 111 111, which is TSTR R7, JR R7 and MOVR R7,R7; the table declares it with `also_encodes` and explains it in the TSTR and JR row notes.
The table has 74 rows, below the earlier estimate of about 90, because SDBD forms are attributes of the memory rows, branch aliases share one row and the jump family is one first-decle row.
Declared `refines` links let a specific row (for example TSTR) sit inside a general one (MOVR) without counting as an overlap.
The check catches encoding transcription errors: a wrong pattern, a missing row or an undeclared overlap fails the gate.
It does not check cycle counts, status bits or interruptibility, which are read from the page images and are only checked for presence.
The same step checks that the C17 status in the discrepancy log equals the `C17 status:` line above.
Phase 2 test vectors are hand-derived from the cited manual pages and are never generated from this table.

## Second-reader pass

A separate agent re-read the family table pages from the page images before opening the first reader's values (decision D-20).
The pass covered `§3.3, pdf 50` to `§3.3.5, pdf 52` and the instruction set pages `pdf 53` to `pdf 58`, plus the three stack pages `pdf 24`, `pdf 46` and `pdf 62`.
For each page the agent rendered it with `pdftoppm`, wrote every printed field to a scratch file, and only then compared that file with the table by a throwaway script.
The fields were the printed encoding, word count, cycles as printed, cycle modifiers, status bits, interruptibility and, on `pdf 57` and `pdf 58`, the SDBD form.
Every disagreement was re-read from a 200 dpi crop of the page, and the winning reading and page were recorded.
The verdicts are in `data/cp1610/second-reader.json`, one entry per row id, and the `cp1610-reader` gate step enforces id parity with the family table.
Result: 74 rows read, 71 agreed and 3 resolved, with no change to any table row.
The three resolved entries are page-internal differences, not table errors.
GSWD and RSWD print the generic field DDD in the format figure on `pdf 51` and 0DD and SSS in the instruction table on `pdf 56`; the table follows the instruction table.
XOR@ prints 1111 in the indirect section on `pdf 55` and a one-digit-short 111 in the double byte section on `pdf 57`; the table follows `pdf 55`.
The ADD@ SDBD prefix is also printed with a three-digit first group on `pdf 57`; the table does not carry the prefix text, so no entry was needed.
The stack pages `pdf 24`, `pdf 46` and `pdf 62` were read blind for push order, pull order and growth direction, and the reading agreed with the Stack operations section and the `C17 status:` line.
Limits of this pass:
- The second reader is the same model family as the first, so a misreading that both readers share is still possible.
- The decode check covers encodings only; cycles, status bits and interruptibility rest on the two readings alone.
- Agreement of two readings is not proof that a cite matches the page; the residual risk is recorded here for agent review.
- The manual describes the CP-1600, and its electrical data are marked Tentative, so the CP-1610 reading in this table is an inference from it.
