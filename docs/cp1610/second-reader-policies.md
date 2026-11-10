# CP-1610 second-reader policy statements

This document states, in words, the rule of each project policy that a CP-1610 vector may depend on.
It is an allowed input of the isolated second reader described in `docs/cp1610/second-reader-protocol.md`.
Page cites are "pdf N" of SRC-gi-cp1600-um-1975.

## Scope

Each section below is headed by the opaque policy token that the blinded copies carry, and states that policy's rule in words.
This document prints no value that a page prints, such as a cycle count, an access offset or an encoding, and no per-vector result; those come from the page images.
A parameter that a policy chooses and that no page prints appears only on a line tagged [param].
The rules hold for every vector wherever they apply; a vector that carries a token is one whose expectation depends mainly on the rule under that token.
One step executes exactly one unit: an instruction, an interrupt entry, a reset entry, a bus-grant unit, a halted unit, a fault unit or a not-reset report.
SDBD is an instruction unit of its own, and the instruction after it is the next unit.
The cycles of a unit are counted from its start, and the bus entries of a vector count cycles from the start of the vector.

## p01

An MVO@ whose source register field names the same register as its pointer register field, for the pointer registers R4, R5, R6 and R7, is refused with the sticky fault UNSUPPORTED_MVO_SSS_EQ_MMM (pdf 55 restricts these forms).
The refusal is found right after the opcode fetch: the step is a FAULT unit, R7 stays at the refused word, the fetch appears in the bus list, no write happens and nothing else changes.
[param] The refusing unit lasts as long as the instruction fetch alone.
A faulted CPU then behaves as stated under p02.

## p02

After SDBD, every target outside the supported forms stated under p18 is refused with the sticky fault UNSUPPORTED_SDBD_TARGET.
The refused targets include direct mode, every form through R6 (PULR and PSHR among them), MVO@, MVOI, branches, jumps, register-only and internal control instructions, and another SDBD.
The refusal is found right after the target's opcode fetch: the step is a FAULT unit, R7 stays at the refused word, DBD stays set, the fetch appears in the bus list and its cycles are counted, and nothing else changes.
[param] The refusing unit lasts as long as the instruction fetch alone.
A sticky fault sets run_state FAULTED and fault_reason, and the step that sets it reports the FAULT event.
[param] Every later step of a faulted CPU is a FAULT unit of 0 cycles that makes no access, reports no event and changes nothing; only reset leaves the faulted state.

## p03

In the halted state, INTR and INTRM are ignored.
In the halted state, BUSRQ is granted, held and released as stated under p05, and the CPU stays halted.
Otherwise each step in the halted state is a HALTED unit that makes no access and reports no event.
[param] A HALTED unit lasts 1 cycle.
Only a STPST edge or reset leaves the halted state; a STPST edge returns the CPU to running, and the same step then goes on as a running step.

## p04

Decode uses only the low ten bits of a fetched instruction word, and the upper six bits are ignored.
NOP and SIN each behave the same for both values of their N bit.
A jump whose interrupt-enable and interrupt-disable bits are both set is refused with the sticky fault UNSUPPORTED_JUMP_DE.
That refusal is found once the second word of the jump, which holds those two bits, has been read like an immediate operand (a READ entry): the step is a FAULT unit, R7 stays at the first word of the jump, the reads made so far appear in the bus list, and nothing else changes.
[param] That refusing unit lasts 8 cycles.
A faulted CPU then behaves as stated under p02.

## p05

A bus request is granted only at an interruptible boundary, never at the closed boundary right after an interrupt entry or the reset entry.
At an open boundary the order of the tests is a pending stop (p09), then BUSRQ, then INTR, then INTRM.
The grant step is a BUS_GRANT unit that makes no access and sets BUSAK.
While BUSAK is set and BUSRQ stays high, each step is a hold BUS_GRANT unit that makes no access.
[param] A grant unit and a hold unit each last 1 cycle.
When BUSAK is set and BUSRQ is low, the step is the release BUS_GRANT unit, which makes no access, clears BUSAK and reports BUS_RELEASED.
[param] The release unit lasts the configured release latency, 2 cycles by default (`config` key `release_latency_cpu_cycles`).
A held or releasing grant is served at the start of a step, before any other test.
A bus-grant unit never changes prev_interruptible, so after a release the next step goes to the interrupt test of the same open boundary and not straight to a fetch.

## p06

Exactly one wait rule holds, quoted from the project's decision record: "BDRDY waits apply only to accesses that begin with a BAR/ADAR address phase, which in the D-02 kind list are FETCH, READ, WRITE and ADAR; INTAK, IAB and DW take no waits."
A wait is inserted as idle cycles between the address phase and the data transfer of the access that takes it (pdf 37, pdf 40, pdf 43).
So a wait lengthens its unit and moves every later bus entry of that unit, while the entry that takes it keeps its own address-phase cycle.
Without a `waits` entry for an address, accesses there take no wait.
[param] The wait cap is 35 cycles by default (`config` key `wait_limit_cpu_cycles`).
[param] A wait above the cap abandons the unit: the step is a FAULT unit with the sticky fault BUS_WAIT_LIMIT that ends at the address-phase cycle of that access plus the cap plus 2 cycles, and the architectural state stays as it was before the unit.

## p07

When the destination register of a read through a pointer register is that pointer register itself, for example MVI@ through R4 into R4 or PULR into R6, the value read wins over the pointer's increment or decrement.

## p08

R4 and R5 post-increment on every access made through them.
R6 post-increments on a write through it (MVO@ through R6 and PSHR) and pre-decrements on a read through it (PULR and every other read through R6).
All register arithmetic, including these increments and decrements, wraps modulo 2^16.
The program counter R7 advances by one for every instruction word fetched or read as an operand, and a write to R7 by the instruction replaces that value.

## p09

HLT sets a stop latch and reports HALT_LATCHED.
HLT is itself non-interruptible (p13), so the instruction after HLT is always executed before the stop.
The latch is evaluated at the next interruptible boundary, where it wins over BUSRQ and INTR.
That step is a HALTED unit that makes no access: it clears the latch, enters the halted state and reports STOPPED.
[param] That HALTED unit lasts 1 cycle.
A STPST edge in a running step is processed first and toggles the same latch, so a second edge before the boundary cancels a pending stop.
The halted state is left only as stated under p03.

## p10

SDBD sets DBD, and the next instruction consumes and clears it.
No stop, bus-request or interrupt test happens between SDBD and its target, so DBD never reaches an interrupt handler.
A refused target leaves DBD set (p02).

## p11

The CPU holds no vector address: every IAB access, for reset and for interrupts, is answered by the bus, and all 16 bits of the answer become the new R7.
In these vectors the test bus answers an IAB access with `init.stimulus.iab`.
An IAB access changes no register other than R7.

## p12

EIS, JE and JSRE set INTFF; DIS, JD and JSRD clear it; reset clears it; GSWD and RSWD leave it unchanged.
A change of INTFF by JE takes effect before the boundary test that follows it.
[param] `interrupt_accept_clears_intff` defaults to 0, so accepting an interrupt leaves INTFF unchanged; when a vector's `config` sets it to 1, accepting an interrupt clears INTFF.

## p13

After each instruction unit, prev_interruptible becomes 1 when the instruction is interruptible and 0 when it is not, and the next step tests lines only when it is 1.
The instructions that the manual marks non-interruptible are non-interruptible, and SIN counts as interruptible (pdf 13).
By policy HLT, SDBD and every MVO form (MVO, MVO@, MVOI and PSHR) are non-interruptible too.
The reset entry and an interrupt entry leave prev_interruptible 0, so the first instruction after either is always executed before any test.
A bus-grant unit leaves prev_interruptible unchanged (p05).

## p14

The CPU receives the line levels once per step and never latches them.
In these vectors the levels of a step are its entry of `init.stimulus.step_lines`, or `init.lines` when there is none, and they are the levels seen at the start of that step.
In the machine the levels are sampled at the final TS1 of the preceding unit, which these vectors do not model.
STPST is an edge event of the step that carries it, not a level.

## p15

Reset models the release of MSYNC: it zero-fills every register and status flag, clears INTFF, DBD, BUSAK, the stop latch and any fault, and sets run_state RESET_PENDING with prev_interruptible 0.
The first step after reset is a RESET_ENTRY unit that follows the initialization timing figure (pdf 27): one IAB access whose answer loads all 16 bits of R7, no other register change, and no line test.
The step after the reset entry fetches the first instruction without any line test.
A step taken before any reset, in run_state UNRESET, is a NOT_RESET unit that makes no access, reports no event and changes nothing.
[param] A NOT_RESET unit lasts 0 cycles.

## p16

An accepted interrupt is an INT_ENTRY unit that follows the interrupt timing figure (pdf 35).
Its order is the acknowledge access INTAK at the address in R6, the write of R7 at the address in R6, the increment of R6 modulo 2^16, and the IAB read whose answer loads all 16 bits of R7.
The INTAK entry carries data 0000.
The write of R7 is reported as kind DW, never WRITE, with the address in R6 and the old R7 as data.
The status flags are not pushed.
The first instruction of the handler is always executed before any further test, because the entry leaves prev_interruptible 0.
INTAK, DW and IAB take no waits (p06).

## p17

After an interruptible unit, the next step tests the lines once, at its start.
The order of that test is a pending stop (p09), then BUSRQ, then INTR, then INTRM.
INTR is always honoured, and INTRM is honoured only when INTFF is set.
Lines are levels and are never latched, so a line that is low at the test is not honoured later.
After a non-interruptible unit, the reset entry or an interrupt entry, the boundary is closed: no line is tested and the step fetches.

## p18

After SDBD, the six immediate instructions (MVII, ADDI, SUBI, CMPI, ANDI, XORI) and the six indirect read instructions (MVI@, ADD@, SUB@, CMP@, AND@, XOR@) through R1 to R5 read a double-byte operand; every other target is refused (p02).
Through R1, R2 or R3 the target reads the same address twice and leaves the pointer register unchanged.
Through R4 or R5 the target reads two consecutive words and increments the pointer register twice.
In immediate form the target reads the two words that follow it, and R7 advances past both.
Each data byte is the low eight bits of its word, and which byte forms the low half of the operand follows the page (pdf 47, pdf 57).
The target's cycle count is the double-byte count printed in the summary tables, which does not include the SDBD instruction; the SDBD instruction is its own unit with its own printed count.

## p19

A taken branch costs the larger and a not-taken branch the smaller of the two values printed in the branch table (pdf 54).
NOPP is never taken and always costs the smaller value.
BEXT asks the external-condition provider with the low four bits of the decle, and an absent provider answers false.
In these vectors the provider answers true exactly for the codes listed in `init.stimulus.ebci`; without that key the provider is absent.
The target of a taken branch follows the branch format figure (pdf 47).
