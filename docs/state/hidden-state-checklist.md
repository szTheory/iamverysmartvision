# The hidden-state checklist of the machine instance

This note is the written checklist behind `data/expected/state-checklist.ids` (STA-01, STA-02; plan 08-09, D-11, D-16, D-17).
It answers one question before any serializer exists: could a field hide, that is, could a member of the instance change a later step or a delivered output and still be missing from a snapshot, or be restored to a wrong value without any test noticing?
Every row of the digest field table (`core/src/machine/machine_digest.c`, 88 rows) has a class, a witness cut where it matters and a test that must name it.
A row cannot exist without a line: the gate step `state` compares the table text with the checklist and with `data/state/schema.literal.txt`, and a control proves each failure.
The cuts are defined in `docs/state/hard-points.md`; the container is specified in `docs/formats/ivsn-v1.md`; the classes are defined there and restated below.
Every figure is derived from the struct headers and the table text, not from a run; a label `[candidate]` marks a rule that plan 08-13 must still prove on reachable states.

## Why the instance is the whole resumable state

The run call keeps no state between calls: no budget, deficit or pointer. The whole instance is the resumable state, and any slicing of one target into several calls gives the same machine (the slice suite proves it against a twin stepped through the internal seam, including whole-storage memcmp). [nc]
A run ends only at a unit boundary. One unit is exactly one instruction, interrupt entry, reset entry, bus grant, hold or release, halted idle step or fault, so the CPU keeps nothing inside a unit: there is no pending-instruction microstate, no cycle counter and no latched line (the lines are inputs of one step and nothing is latched).
The PSG and the STIC are caught up to the unit end at every unit end in the step seam, in the fixed order PSG then STIC, never to a call target, so no device work stays pending across a boundary.
The core holds no file-scope state: the only statics in the core sit inside fault-conditional blocks that are compiled out of every normal build, and the `c-purity` step and the core symbol check enforce the rest.
Outside the instance, by design, are the three caller-owned output buffers (video, audio, PSG log) and the input event array. A host that restores resubmits its input events from the restored applied-event count and keeps its own buffers; the cursors that tell it where to resume (`obs.video.count`, `obs.audio.emitted`, `obs.psglog.count`, `obs.input.count`) are STATE rows.

## The four classes

<!-- The classes are defined in docs/formats/ivsn-v1.md, section Classes; this is the same definition in one line each. -->
- STATE: serialized and restored byte for byte. 51 rows.
- IMAGE_BEARING: the loaded image arrays (`mem.exec`, `mem.grom`, `mem.cart`, 110,592 bytes, never serialized, rights). 3 rows.
- BOUND: identity checked against the live instance, never serialized, never written by restore: the handle magic, the two profile indices, the four image-structure rows of the memory system and the 21 slot rows. 28 rows.
- PAD: the six reserved words, never serialized, zeroed by restore. 6 rows.

The class rule is a decision of this plan that the research text did not state: the research listed `magic` with the pads, but a restore zeroes the PAD rows and must never touch the magic of the handle, so `magic` is BOUND. The counts 51, 3, 28 and 6 sum to 88.
How a witness is read: a fresh target holds zeros, so a row that is zero or default at a cut would look right whether it was restored or not. Every STATE row therefore names a cut at which its bytes differ from the power-on state of the program and at which a wrong value changes the continuation.

## The per-struct tables

In every table the column "Zero target" says why a restore into a fresh, all-zero target would not show a lost row at the witness cut.

### Instance rows

| Row | Class | Why it can hide | Witness | Zero target |
|---|---|---|---|---|
| `magic` | BOUND | makes the handle valid; a restore that wrote it could accept a stale handle or destroy a valid one | none; `state.restore.refuse-identity-profile` | a fresh target already has the magic |
| `profile` | BOUND | registry index of the profile; the snapshot names the profile by id string and parameter digest instead | none; same test | a fresh target already has the index |
| `phase` | STATE | the restore sets POWERED; a snapshot is refused while the machine is OFF, so the value is POWERED at every cut | HP-FRAME-MID | OFF is 0, POWERED is not |
| `reserved` | PAD | stale bytes would break the whole-instance memcmp | none; `state.restore.poisoned-target-equals-source` | a zero target has zero pads |
| `time_ticks` | STATE | machine time in master ticks; every device cursor is checked against it | HP-FRAME-MID (at least 59,736 ticks, one frame) | a zero target is at tick 0 |

### CPU (domain 1, `ivs_cp1610`, 32 bytes)

| Row | Class | Why it can hide | Witness | Zero target |
|---|---|---|---|---|
| `cpu.r` | STATE | R6 is the stack pointer, R7 the program counter; both set by the entry unit | HP-INT-ENTRY: R6 is pushed, R7 holds the interrupt vector | R6 and R7 are 0 at power-on |
| `cpu.status` | STATE | the four flags C, OV, Z, S (bits 4 to 7) matter only to a branch or a status read; they are set by one unit and read by a later one | HP-HLT: the last flag-setting unit of the counting loop precedes its branch, and the instruction that follows the HLT reads the Z flag (see the choice below) | flags are 0 at power-on |
| `cpu.intff` | STATE | the interrupt enable matters only when INTRM falls due at an open boundary | HP-FRAME-MID: the machine idles mid-frame with the enable set and the next frame's interrupt is accepted only if it is restored | 0 at power-on |
| `cpu.dbd` | STATE | the SDBD prefix is pending: the next instruction decodes with an extra decle and two more cycles | HP-SDBD, taken after the SDBD unit | 0 at power-on |
| `cpu.prev_interruptible` | STATE | decides whether the next boundary tests the lines or fetches | HP-OPEN: an open boundary at which the interrupt falls due | 0 at power-on |
| `cpu.run_state` | STATE | RUNNING, HALTED, FAULTED, RESET_PENDING decide what a unit is | HP-HALTED | UNRESET is 0 |
| `cpu.halt_pending` | STATE | HLT sets the stop latch; the stop happens at the next interruptible boundary, after one more instruction | HP-HLT | 0 at power-on |
| `cpu.fault_reason` | STATE | sticky; every later call stops with a guest fault at once | HP-FAULTED | 0 at power-on |
| `cpu.busak` | STATE | a held bus grant is a unit of its own; the release needs BUSRQ low | HP-STEAL-A, the first grant unit | 0 at power-on |
| `cpu.reserved` | PAD | stale bytes | none; `state.restore.poisoned-target-equals-source` | zero |

The CPU has no other member (`sizeof` is asserted at 32 bytes): the cycle counter, the instruction microstate and the latched lines are absent by design.

### Memory system (domain 2 and the image-structure rows of domain 3)

| Row | Class | Why it can hide | Witness | Zero target |
|---|---|---|---|---|
| `mem.profile` | BOUND | the profile index of the decode | none; `state.restore.refuse-identity-profile` | already equal |
| `mem.gate_open` | STATE | the register gate and the graphics gate follow the timeline cursor: FRAME_START opens both, REG_CLOSE and GMEM_CLOSE close them; index 0 is always 1 and index 3 always 0 | HP-RESETPEND: right after a reset every gate is open and the first register write of frame 0 must reach the register file | a zero target has every gate closed and blocks that write |
| `mem.exec_loaded`, `mem.grom_loaded`, `mem.cart_count`, `mem.cart_seg` | BOUND | the committed image structure; only a load changes it | none; `state.restore.refuse-identity-slots` | the live target keeps its own |
| `mem.sysram` | STATE | 512 words of backing for a 352-word extent: BACKTAB, the program's variables and the stack | HP-LATCH: BACKTAB words and the toggling word are written by the guest | zeros render empty cards |
| `mem.scratchpad` | STATE | 256 words of backing for a 240-word extent (8-bit RAM) | HP-SDBD: the program stores a byte before the cut and reads it after | zeros read back as 0 |
| `mem.gram` | STATE | 512 words; written by the guest, read by the renderer | HP-LATCH: card 0 holds a nonzero pattern that decides the pixels | zeros draw blank cards |
| `mem.exec`, `mem.grom`, `mem.cart` | IMAGE_BEARING | read-only after a load; 110,592 bytes | none; `state.save.no-image-bytes` | the target keeps its own images |
| `mem.psg_reg` | STATE | the 16 PSG register words, masked on store | HP-PSG-MID | zeros are silence |
| `mem.port_pulldown` | STATE | derived from the controller words (INV-4) and stored; a lost value changes what the guest reads at the ports | HP-INPUT: a key is held across the cut | a zero target reads idle ports |
| `mem.psg_reserved` | PAD | stale bytes | none | zero |
| `mem.stic_reg` | STATE | the 64 register words, including the collision registers (bits ORed in at FRAME_END) and the MOB, border and delay registers; read masks apply on the CPU view, not here | HP-MOB: MOB, border and delay registers are nonzero and a collision bit is set | zeros show no object |

The collision bits, the MOB registers, the border-extension registers and the delay registers are ordinary words of `mem.stic_reg`: plan 08-08 added no hidden state (the STIC state stays 528 bytes), so the checklist gained no STIC row for them. The HP-MOB cut is a cut with those registers non-default, not a new row.

### PSG (domain 4, `ivs_psg_state`, 72 bytes)

| Row | Class | Why it can hide | Witness | Zero target |
|---|---|---|---|---|
| `psg.next_sample` | STATE | the index of the next chip step; a value off by one shifts every later sample stamp. It equals `time_ticks / sample_period_ticks + 1` after a unit end (INV-3) and is stored, not recomputed | HP-PSG-MID | a zero target has 0, the power-on value is 1 |
| `psg.tone_counter`, `psg.tone_out` | STATE | mid-period phase is not derivable once the guest has rewritten the periods | HP-PSG-MID: periods rewritten twice | zero counters restart every tone |
| `psg.noise_counter`, `psg.noise_lfsr` | STATE | the shift register carries history | HP-PSG-MID: noise advanced for thousands of steps | a zero register differs from the seed |
| `psg.env_counter`, `psg.env_level`, `psg.env_dir`, `psg.env_held`, `psg.env_cycles` | STATE | an envelope cycle in progress; `env_held` is 1 after a reset until the first shape write | HP-ENV-MID: a repeating decay shape, not held, a level between 1 and 14 | zeros are a held envelope at level 0 |
| `psg.reserved` | PAD | stale bytes | none | zero |

### STIC (domain 4, `ivs_stic_state`, 528 bytes)

| Row | Class | Why it can hide | Witness | Zero target |
|---|---|---|---|---|
| `stic.frame_index`, `stic.event_index` | STATE | the timeline cursor is state, never derived from time by modulo | HP-FRAME-MID (frame at least 1, event cursor between 9 and 15) | a zero target restarts the frame |
| `stic.display_latch` | STATE | set by the strobe inside the register window, it decides at GMEM_CLOSE whether the frame is displayed | HP-CLOSED: the cut follows the first strobe of the interrupt routine, before the decision | a zero latch hides the frame, its windows and its grants |
| `stic.windows_active` | STATE | decided at GMEM_CLOSE; the row fetches and the bus windows follow it | HP-FRAME-MID | zero means no windows |
| `stic.mode`, `stic.cs_ptr` | STATE | the mode is selected by a register read; the color-stack pointer is observable through the STIC report and the digest because each render starts at 0 | HP-LATCH: a Color Stack frame with an advance word, so the pointer is nonzero | zero is Foreground/Background and pointer 0 |
| `stic.blocked_reads`, `stic.blocked_writes` | STATE | saturating counters of register accesses that met a closed window | HP-LATCH: the loop touches the registers outside the window | zero counts |
| `stic.late_grants` | STATE | bus windows that ended without a grant, added at FRAME_END | HP-STEAL-B; see the open point below | zero |
| `stic.grants_in_frame` | STATE | grants of the frame in progress; FRAME_END adds `bus_count - grants_in_frame` to the late count | HP-STEAL-C: the unit after a release, at least one grant counted | zero grants |
| `stic.reserved0` | PAD | stale bytes | none | zero |
| `stic.row_latch` | STATE | the latched BACKTAB copy (12 rows of 20 words): a guest store after a row's fetch must not change that frame's row, so recomputing it from BACKTAB on restore is wrong | HP-LATCH: the guest rewrites a word of an already fetched row | a zero latch renders a blank row |

Open point for `stic.late_grants`: it becomes nonzero only after a frame in which a bus window ended without a grant. Lamp Tone grants every window unless a non-interruptible stretch spans a whole hold; a stretch of at least 1,026 cycles (the window pitch 912 plus the hold 114) always covers one whole hold, and 94 consecutive stores of 11 cycles reach it. If plan 08-14 finds the row zero at HP-STEAL-B, it adds a cut with such a stretch or exempts the row with that argument; the number of exemptions at this plan is zero.

### Observation rows (domains 16 to 19)

| Row | Class | Why it can hide | Witness | Zero target |
|---|---|---|---|---|
| `obs.video.chain`, `obs.video.count` | STATE | the chain is the digest of every delivered slot; the count equals the frames completed (INV-6, candidate) | HP-FRAME-MID: the first slot after the cut continues the chain | a zero chain is not the chain start value |
| `obs.audio.chain`, `obs.audio.count`, `obs.audio.emitted`, `obs.audio.prev_levels` | STATE | the chain continues across the cut; `emitted` equals `next_sample - 1` (INV-3); the previous levels decide the next level-change record | HP-PSG-MID: the levels change after the cut | zeros |
| `obs.audio.reserved` | PAD | stale bytes | none | zero |
| `obs.input.words`, `obs.input.count`, `obs.input.floor_tick`, `obs.input.chain` | STATE | the controller words in force, the applied-event count that tells the host where its schedule resumes, the floor below which events are refused, the chain | HP-INPUT: a key held, an event applied before the cut and one pending after it | the chain start value is not zero |
| `obs.psglog.chain`, `obs.psglog.count` | STATE | the log chain and record count; the next record index comes from the count | HP-LOG: a write whose stamp lies in the last unit | zero |

### Slot rows (domain 3)

The 21 rows `slot.exec.*`, `slot.grom.*` and `slot.cartridge.*` (kind, loaded, length_bytes, length_units, unit_bits, base_addr, sha256) are BOUND. Only a load changes them. The snapshot carries their identities (the loaded flag, the SHA-256 and the length) in the header, and a restore compares them with the live slots and never writes them: `state.restore.refuse-identity-slots`.

## The choice of witness

The 18 cuts are the inventory of `data/expected/state-cuts.ids`; each is the witness of at least one STATE row and a row names exactly one cut. A row with no good witness among the 18 would get a new cut line here, never a weak witness; no row needed one.

- `cpu.r`: HP-INT-ENTRY is the unit boundary after the interrupt entry unit. R6 (the stack pointer, pushed by one) and R7 (the interrupt vector) are both nonzero and both decide what executes next; a restore that replayed or skipped the entry shows in R7 at once.
- `cpu.status`: the flags are read only by a branch or a status read, so the cut must stand between a flag-setting unit and its reader. In P-HLT the counting loop ends with DECR (sets Z) and BNZE (reads it, not taken); the HLT unit sets only the stop latch and leaves the flags; the instruction that follows the HLT always executes (HLT is not interruptible) and is a BZE that reads Z. At HP-HLT the flags are Z set and the rest clear, which is not the power-on value, and a restore that lost the flags makes the BZE fall through and execute a store that the true run skips.
- `mem.gate_open`: a gate flag matters when a guest access meets it. At a reset every gate is open and the first strobe of frame 0 reaches the register file. A zero target blocks it. Mid-frame values (windows closed) are covered by the derived invariant INV-2 and by HP-LATCH, whose loop accesses the registers in the closed window.
- `phase`, `time_ticks`: both are nonzero at HP-FRAME-MID and both are read by every later step; a wrong value is the largest divergence there is.
- `mem.sysram`, `mem.scratchpad`, `mem.gram`: each is written by the guest before its cut and read by the guest or the renderer after it (HP-LATCH for the first and the third, HP-SDBD for the scratchpad, whose program stores a byte and reads it back). The rows are swept whole: the unreachable tails (words 352 to 511 of the system RAM, 240 to 255 of the scratchpad) are part of the row, and the reachable part makes the row diverge.
- `obs.video.*`, `obs.audio.*`, `obs.psglog.*`, `obs.input.*`: each is a chain or a count that only ever grows from its own value. A restore that started a chain from its start value would continue a different chain; the digest of that domain shows it. HP-FRAME-MID, HP-PSG-MID, HP-LOG and HP-INPUT stand where each chain already has history. Plan 08-14 may find that a chain row is never read by anything but its own digest; the exemption it then writes needs the argument in its own words.

## The derived invariants

A restore accepts only a snapshot whose decoded values satisfy these rules; each is `[candidate]` until plan 08-13 proves that every reachable state satisfies it (a wrong rule would refuse real snapshots). The same text is in `docs/formats/ivsn-v1.md`.

1. INV-1: the CPU members pass the CPU state validator, the STIC members pass the STIC state validator and `event_index` is below the event count of the profile.
2. INV-2: the next STIC event tick is above `time_ticks` and the event before it is at or below it; the gate flags equal the replay of the event table up to `event_index`.
3. INV-3: `psg.next_sample` equals `time_ticks / sample_period_ticks + 1`; `obs.audio.emitted` equals `psg.next_sample - 1`; the flag members of the PSG are 0 or 1; `env_level` and `env_cycles` are at most 15; the previous levels are at most 15; the noise register fits its width; every stored PSG register equals itself under its write mask.
4. INV-4: a controller word at or above the profile's controller count is 0 with bit 31 clear; `mem.port_pulldown` equals the value derived from the words; `obs.input.floor_tick` is at most `time_ticks`.
5. INV-5: `phase` is POWERED and `time_ticks` is at most 2^63.
6. INV-6: `obs.video.count` equals `stic.frame_index`; the PSG log count and the input count are free.

Counters, chains, RAM words and the latched card rows are free values; for them the only check is the trailer of the container.
A stored derived field is serialized and checked, never recomputed on restore, so that a field that drifts from its derivation is a visible failure and a recomputed-instead-of-restored field cannot hide.

## The comparison oracle

For each cut c of program P, with E the end tick of the uninterrupted reference:
1. REF: a fresh instance steps from 0 to E through the public run call with video, audio and log buffers sized from the info report; the streams, the final instance bytes and the nine digests (the composite, the four state domains and the four observation domains) are kept.
2. SUB: a second instance steps to c at the pinned tick and is saved. A poisoned target is built: an instance of the same profile with the same images loaded, every non-image, non-slot byte filled with `0xA5` through the offset, width and count of the field table, the magic and the profile index set. The snapshot is restored into it. At once, `memcmp` over the whole instance equals the source instance (the pads equal too, because restore zeroes them) and the nine digests equal.
3. The target continues from c to E with fresh stream buffers, resubmitting the input events from the restored applied-event count. The video, audio and log streams of the two legs concatenated equal the reference byte for byte, the first-index fields of the second leg equal the counts of the first, the final instance bytes equal the reference and the digests and public reports equal.
4. Saving the target right after the restore gives the snapshot bytes again (the canonical form), and snapshot, restore, snapshot is a fixed point at every cut; the snapshot bytes of the development and the release presets are identical.

The poisoned target is essential: a fresh target is zero, and every row that is zero at the cut would look restored.
The oracle does not depend on the shared table. Digests and snapshot are written from one table, so a row missing from the table is missing from both; the independent checks are the table-free `memcmp` of the whole instance, the streams, the existing test that every byte of the instance lies in exactly one row, the size assertion of the instance and the pin of the schema digest. The checklist closes the remaining gap from the side of the data: the gate step compares the table text with the checklist and the schema literal, and the blind second read of this plan (see `docs/state/checklist-second-read-request.md`) compares the member list of the struct headers with the 88 rows without seeing the table.

## The negative controls

- The runtime row-omission sweep (plan 08-14), built from the field accessor, over the 51 STATE rows at their witness cuts: restore correctly, then replace row r with a sweep value and continue to E; the result must differ from the reference in a digest, a report or a stream. The sweep value is never the constant fill alone: it is the byte-wise complement of the row's own bytes at the cut, and for a row that the validators restrict to 0 or 1 the other member of the set, so that a row whose reference value happens to equal the fill cannot hide. A row at which the sweep finds no divergence is a checklist defect: its witness is weak or the row is dead, and a dead row goes to an exemption with a reason of at least three words, checked by the gate.
- The compile-time faults named for plans 08-11 to 08-14: SNAPSHOT_LITTLE_ENDIAN, SNAPSHOT_PAD_SERIALIZED, SNAPSHOT_IMAGE_BYTES_EMBEDDED, SNAPSHOT_LENGTH_TRUSTED, SNAPSHOT_TRAILER_UNCHECKED, RESTORE_MUTATES_BEFORE_VALIDATE, RESTORE_PROFILE_UNCHECKED, RESTORE_IMAGE_DIGEST_UNCHECKED and SNAPSHOT_STIC_ROWS_OMITTED.
- The six pure controls of this plan, one per failure of the gate step: `state-checklist-row-missing`, `state-checklist-line-extra`, `state-checklist-witness-unknown`, `state-checklist-test-unknown`, `state-checklist-exempt-no-reason` and `state-schema-literal-stale`.

## The adversarial list

Each item of the design review (08-DISCUSS-state 7.3) with the mechanism that answers it.
1. Zero coincidence: the poisoned target and the whole-instance memcmp; the sweep requires a nonzero, behaviour-relevant value at the witness cut.
2. Recomputed instead of restored: the derived fields are serialized and checked (INV-2 to INV-4) at restore and by a save-side test over every cut; HP-LATCH differs by design and `state.cut.latch-not-recomputed` (plan 08-14) builds the recomputing variant and requires that it diverge.
3. Shared-table blind spot: the table-free memcmp, the every-byte-in-one-row test, the instance size assertion, the schema pin, the gate step over the table text and the blind second read of the member list.
4. Equal digests with unequal pads: the memcmp and the canonical re-encode, with the fault SNAPSHOT_PAD_SERIALIZED.
5. Restore equals the saved digest but not the future: HP-STEAL-A and the sweep of `cpu.busak`; `prev_interruptible` after a grant (a grant never changes it) at HP-OPEN.
6. Host-side hidden state: the input events and the output buffers are the host's; the host resubmits from the restored applied-event count, which the runner hook of plan 08-16 does and the installed-consumer pass proves.
7. Same-build bias: a round trip on one build shows consistency, not accuracy or stability across builds; the claim written for STA-01 and STA-02 is a deterministic-repeatability claim for one profile and one build, with no statement about hardware or about snapshots across versions [nc]; the development and release snapshot bytes are compared, and the byte order is held by construction, by the literal header and by the fault SNAPSHOT_LITTLE_ENDIAN, not by a big-endian host.
8. Validators too strict: every invariant has a test that all reachable states satisfy it, over every cut and the diagnostic programs.
9. Saturation and wrap: the blocked and late counters saturate at the largest 32-bit value; a hand-built snapshot with saturated counters must round-trip, and `time_ticks` wraps nowhere reachable (INV-5).
10. Restore changes the profile index or the slots: it must not; the whole-instance memcmp outside the restored region and the rights test (no image byte in the output) hold it.
11. Time and file leakage: save and restore read no host time source and no file; the core symbol deny list and the purity step apply to the new translation units like any other.

## Editing the checklist

After the blind second read, the checklist, the schema literal and the container classes change only by the `correction:` lines of `docs/state/checklist-second-read-record.md`, applied by plan 08-10 in its first commit. A later plan that finds a defect (a weak witness, a dead row) records it in an appended note and changes the line with the reason, never silently.

## The derived invariants as decided by plan 08-13 (appended note)

The section "The derived invariants" above is the oracle of plan 08-09 and is not edited.
Plan 08-13 held each rule against the live instances of real programs at every unit boundary (`state.restore.invariants-hold-reachable`: 23 programs, 411,075 boundaries; the programs and the counts are in "As built (plan 08-13)" of `docs/formats/ivsn-v1.md`), and the status of each `[candidate]` mark above is:

| Rule | Status | Note |
|---|---|---|
| INV-1 | kept | judged as a range rule (reason 78) with INV-5, because it is the three device validators and the event cursor |
| INV-2 | corrected | the clause "the next STIC event tick is above `time_ticks`" has one exception, the reset state (machine time 0, frame 0, event 0) where the first event has not been applied yet; the other clauses and the gate flags hold as written |
| INV-3 | kept, strengthened | the previous audio levels are three four-bit levels and a fourth byte of 0 (the stricter form implies "at most 15" and is derived from the step code; no reachable state forced the change) |
| INV-4 | kept | |
| INV-5 | kept | judged with INV-1 as a range rule (reason 78) |
| INV-6 | kept | the candidate `obs.video.count == stic.frame_index` holds at every boundary of every program |

No rule was dropped. The status of the hidden-state rows (the witness cuts, the sweep) is the work of plan 08-14.

## As built (plan 08-14, task 3): the row-omission sweep

This section is an appended note; the sections above and `data/expected/state-checklist.ids` are untouched.
The runtime row-omission sweep (`state.sweep.row-omission`, `tests/state/state_sweep.c`) takes each of the 51 STATE rows at its witness cut, restores the snapshot of the cut correctly into a poisoned target, replaces the bytes of that row with a sweep value, continues to the end tick of the cut and requires a difference from the uninterrupted run outside the row's own bytes: the final instance with the row's byte range masked, the video, audio and PSG log streams, the first-index fields and the other members of the run report, the status of the run call, and the three public reports.
A row that matters keeps `ROW`; a row that no continuation reads is `EXEMPT`. [nc]
The checklist was not edited: the amendments below are the appended notes `data/expected/state-sweep-notes.ids`, which the sweep reads after the checklist (a note replaces the witness cut and the sweep column of the row it names) and which the sweep test judges (a note must name a STATE row and a cut, an exemption needs a reason of at least three words, and every cut must stay the witness of at least one row).

How the sweep value is chosen. The first candidate of every row is the byte-wise complement of its bytes at the cut, as written above; for a row the validators restrict to a few values the other members follow (the lowest bit flipped, the constant fill `0xA5`, zero, the first or last element alone, every small value of a scalar). Each candidate is first judged by the validators and invariants of the restore (`ivs_snapshot_invariants` on a probe of the modified instance), because a value no run reaches may lie outside what the core can step safely. Candidates that pass all six rules are tried first (at most six continuations), then candidates that break only a derived rule (INV-2 to INV-4, INV-6, or the range rule only: a level, cycle, direction or hold flag of the envelope inside its range, whose relation to its neighbours is a derived rule). A row for which no candidate differs is inert; a row with no safe candidate at all would be a finding (there was none). The sweep value is therefore the complement where that is valid and a different member of the set where it is not, never the constant fill alone.

The zero-coincidence guard (1): the bytes of the row at the cut differ from the bytes of the row in the power-on state of the same program. Two cases cannot, by nature, and are held to the zero row of a fresh target instead: `phase` (POWERED at every cut) and every row at the cut of tick 0, which is the power-on state (HP-RESETPEND, the witness of `mem.gate_open`). `mem.gate_open` and `phase` equal their power-on rows and are non-zero; `psg.env_held` is zero at its cut and differs from its power-on value 1.

Results (51 rows: 39 diverge in behaviour, 3 in a public report only, 9 are exempt; 0 disagreements with the checklist after the amendments below):

| Family | Rows | Divergence in behaviour | In a public report only | Exempt |
|---|---|---|---|---|
| instance (`phase`, `time_ticks`) | 2 | 2 | 0 | 0 |
| CPU | 9 | 8 | 1 (`cpu.fault_reason`) | 0 |
| memory system | 7 | 7 | 0 | 0 |
| PSG | 10 | 9 | 0 | 1 (`psg.env_level`) |
| STIC | 11 | 7 | 2 (`stic.blocked_reads`, `stic.blocked_writes`) | 2 (`stic.late_grants`, `stic.cs_ptr`) |
| observation rows | 12 | 6 | 0 | 6 (the four chains, `obs.input.count`, `obs.input.words`) |

The three rows that differ in a public report only keep `ROW`: nothing but the report reads a faulted CPU's reason (the CPU stops at the fault) and the two saturating counters of blocked register accesses, and the report echoes them; the plan counts the public reports as divergence, and their witness cuts hold non-zero values.

Witnesses amended (the sweep found the listed witness inert; the cut definitions are in the corrections of `docs/state/hard-points.md`):
- `cpu.intff`: HP-FRAME-MID to HP-OPEN. At HP-OPEN a lost interrupt enable means the interrupt of the frame is not accepted, so the display is not enabled for the frame and the frame differs.
- `cpu.prev_interruptible`: HP-OPEN to HP-STEAL-A (now the boundary before the first bus grant). At HP-OPEN a boundary closed by mistake accepts the interrupt one idle unit late; every idle unit is 36 ticks and the late entry leaves the same machine at the same ticks, so the loop cannot show it (the sweep found it inert). A closed boundary before a bus grant delays the grant, and the run reports another number of units.
- `cpu.busak`: HP-STEAL-A to HP-STEAL-B (now the boundary before a release). After the grant a lost BUSAK is harmless, because BUSRQ is still asserted and the grant is taken again.

Exempt rows, with the argument and the cut at which the sweep found each inert (every candidate, 3 to 6 continuations, no change in behaviour or in any report):

| Row | Cut | What reads it, and why that cannot change a continuation |
|---|---|---|
| `obs.video.chain` | HP-FRAME-MID | `h(n+1)` is the SHA-256 of the previous chain value and the new frame record; nothing reads the chain but the VIDEO digest and its own next value |
| `obs.audio.chain` | HP-PSG-MID | the same over the level-change records |
| `obs.input.chain` | HP-INPUT | the same over the applied events |
| `obs.psglog.chain` | HP-LOG | the same over the PSG register writes |
| `obs.input.count` | HP-INPUT | incremented by every applied event and read by nothing; the host resubmits from the applied count of the report of its own earlier call |
| `obs.input.words` | HP-INPUT | every event carries the whole controller state and overwrites the four words; the pull-downs the guest reads are a separate stored row (`mem.port_pulldown`, a ROW), and only the run report echoes the words, so a replaced value is overwritten by the next event before anything reads it |
| `psg.env_level` | HP-ENV-MID | a function of the cycle index and the direction (the validator enforces it) and recomputed at every envelope step before it is used for a sample |
| `stic.cs_ptr` | HP-LATCH | assigned by every render before anything reads it (each render starts at 0); between two renders it is an output of the last render |
| `stic.late_grants` | HP-STEAL-C | a saturating count that the end of a frame adds to and nothing reads; the STIC report echoes it, it is zero at every one of the 18 cuts (every bus window of the programs of the cuts is granted: no non-interruptible stretch of 1,026 cycles occurs), so no cut witnesses it non-default; a designed program with such a stretch would |

Nine exemptions, at most the eleven the plan expects. Each exempt row is covered by the poisoned-target equivalence (`state.restore.poisoned-target-equals-source`, which fails a row a restore leaves at the poison byte), by the whole-instance and digest equality of every cut comparison, and, for the four chains, by the digest of their domain at the end of each cut comparison.
The sweep neither assumes nor fakes the result: `state.judge.row-sweep-detects-omission` applies it to `cpu.r` at HP-SDBD (read by the program, reported as divergent) and on an exempt row of this list (reported as inert), so it can fail both ways.
