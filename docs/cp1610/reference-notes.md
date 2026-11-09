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
Electrical specifications in the manual are marked Tentative and are not used here.
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
The three pages agree: the stack grows toward higher addresses, a push writes at R6 and then increments it, and a pull decrements R6 and then reads.
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
The meaning of each DE value is given in a table on a later page and is not restated here.

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
