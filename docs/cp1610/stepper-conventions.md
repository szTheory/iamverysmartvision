# CP-1610 stepper conventions

This note is for integrators of the CP-1610 unit stepper (`core/src/cpu/cp1610.h`).
It states the conventions a reader needs that no page of SRC-gi-cp1600-um-1975 prints.
Every page cite is "pdf N" of that manual, and every unresolved choice names its discrepancy id in `data/discrepancies.json`.
A label [OBS] marks a fact read from a page image, and [INF] marks a convention or an inference.
This note holds no vector values; the vectors under `data/cp1610/vectors/` carry their own derivations.
This note is not an input of the blind second reading: A-12 allows those readers only their blinded copy, the page images, the GI pdf, the reader protocol and the policy-statement document.

## Units and their cycle counts

One call to `ivs_cp1610_step` executes exactly one unit, and time crosses the API only as CPU cycles relative to the start of that unit (D-01, D-04) [INF].
An instruction unit lasts the printed cycle count of its row plus the printed modifiers plus every wait it took (pdf 43: each memory access adds its own wait) [OBS].
A reset entry lasts the cycles of the initialization sequence figure (pdf 27), and an interrupt entry lasts the cycles of the interrupt sequence figure (pdf 35, C30) [OBS].
A step that starts in UNRESET reports a NOT_RESET unit, and a step that starts in FAULTED reports a FAULT unit; both take 0 cycles, make no access and change nothing (D-12) [INF].
The step that refuses an encoding or exceeds the wait cap is itself a FAULT unit, and it counts the cycles spent up to the refusal, as chosen in plan 02-02 [INF].
A BUS_GRANT unit that grants or holds the bus lasts 1 cycle, because the CPU samples BUSRQ every TS1 (pdf 36) [INF, C52].
The BUS_GRANT unit that releases the bus lasts the configured release latency (C52) [INF].
A HALTED unit lasts 1 cycle and makes no access (C45) [INF].
So every unit after the reset entry that is not a no-progress report advances time, and a run loop over steps cannot spin without time passing unless the state is UNRESET or FAULTED (D-12) [INF].

## Access records

Each access is one 12-byte record with the cycle of its address phase, the address, the data, the wait the device answered, the kind and the flags (D-02) [INF].
Idle cycles are not recorded; they are the gaps up to the unit length (D-02) [INF].
An IAB record carries address 0 and a reason flag, reset or interrupt, and the bus layer answers it with the vector, which loads all 16 bits of R7 (POL-IAB-01, C32, C42) [INF].
The write of R7 onto the stack during an interrupt entry is its own kind, DW, never WRITE (A-11), and it is stamped at the DW cycle, one cycle before DWS (pdf 10) [OBS].
A direct-mode operand access is an ADAR record, and it carries the effective address, the word read through the address word (pdf 11, pdf 30) [OBS].
There is exactly one wait rule (A-04 as amended by A-11, C53): BDRDY waits apply only to accesses that begin with a BAR or ADAR address phase, which are FETCH, READ, WRITE and ADAR; INTAK, IAB and DW take no waits (pdf 40) [OBS].
A wait is inserted as idle cycles between the address phase and the data transfer (pdf 37), so it shifts every later record of the unit and the unit length; the record that took it keeps its own address-phase cycle [OBS].
A wait up to the configured cap is accepted; a longer one ends the unit one cycle past the cap with the sticky fault BUS_WAIT_LIMIT and the architectural state as before the unit (pdf 12 bounds the wait but defines nothing past it; C53) [INF].
The TCI and PCIT strobe records may sit up to one cycle past the end of their unit (pdf 33, pdf 34, C39) [INF].

## Step-start order

A STPST edge is an event of one step, not a level (pdf 14, A-01, C54), and it is processed first [INF].
In RUNNING the edge toggles the stop latch, so a second edge before the boundary cancels a pending stop (C54) [INF].
In HALTED the edge resumes RUNNING, and the same step then continues as a RUNNING step (pdf 14) [INF].
A bus grant still held is served next: a hold unit while BUSRQ stays high, else the release unit (pdf 36, POL-BUS-01) [INF].
A grant never changes the previous-unit interruptibility, so the step after a release goes to the interrupt test and not straight to a fetch (Fig 2-6, pdf 22, A-02, C52) [OBS].
At an interruptible boundary the priority is stop, then BUSRQ, then INTR, then INTRM with INTFF set (pdf 25 orders bus requests before interrupt requests; POL-INT-01, POL-HLT-01, C40) [INF].
HLT sets the same stop latch as STPST, and HLT is non-interruptible by default, so the instruction after HLT always executes before the stop (pdf 56, C40, C56) [INF].
A closed boundary, after a non-interruptible unit, the reset entry or an interrupt entry, tests no line and fetches (Fig 2-6, pdf 22) [OBS].
In HALTED, INTR and INTRM are ignored and BUSRQ is granted and released as usual while the CPU stays HALTED (POL-HLT-02, C45, A-10) [INF].

## Vector fields

A vector's `init` is the full starting state, with `prev_interruptible` 1 and run state RUNNING unless it says otherwise [INF].
`steps` is the number of units the harness steps, one call each [INF].
`step_lines` gives the lines of each step in order, and without it every step uses `init.lines` [INF].
`config` overrides fields of the profile default config for the whole vector [INF].
`waits` lists addresses and the wait the flat test bus answers for every access at that address, of any kind; the core alone decides which kinds take it [INF].
`iab` is the value the test bus answers to an IAB read, and a vector without it fails if any IAB happens [INF].
`ebci` lists the external branch condition codes that the provider answers true, and without the key the provider is absent and answers false (C57) [INF].
`reset` set to true calls the reset function before the first step [INF].
`expect` lists only the fields that differ from `init`, plus the total cycles, the unit kinds per step and the union of the events [INF].
A `bus` list is compared in order by kind, address and data, and its cycles are compared only when `bus_basis` is `figure` [INF].

## Config defaults and profile fields

`interrupt_accept_clears_intff` defaults to 0, from the profile field `cpu.interrupt_accept_clears_intff` (C41) [INF].
`release_latency_cpu_cycles` defaults to 2, from the profile field `bus.release_latency_cpu_cycles` (C52), and 0 is refused [INF].
`wait_limit_cpu_cycles` defaults to 35, from the profile field `bus.wait_limit_cpu_cycles` (C53) [INF].
The IAB vector and the EBCI default belong to the bus layer, through the profile fields `interrupt.iab_value` and `cpu.ebci_default` (C32, C57) [INF].
