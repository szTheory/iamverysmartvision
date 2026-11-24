state: closed
oracle-commit: 02899e1b1814ea678cc4cd78568534779a012873
corrections: 0

# Hidden-state checklist blind second-read record

Evidence class: `source-expectation, second-read`. This is not an `independent-implementation-comparison`: the same model family read the same headers, so a shared misreading remains possible.

- Date: 2026-11-14.
- Reader: one fresh isolated subagent, label `blind-reader-1`, with no access to the checklist, the digest table, the schema literal, `docs/state/`, the plans or the git history by instruction. It made 7 tool calls, all reads of the six input files.
- Inputs: the six blinded files built with the commands of `checklist-second-read-request.md` (machine-structs.h 73 lines, cp1610.h 251, psg.h 211, stic.h 257, memsys.h 265, classes.md 6), and the output form of the request.

## Comparison

- The reader listed 88 members: 67 scalar or array members of the instance proper, plus the 7 fields of each of the three image slots (21). The checklist has 88 rows (67 plus 21 slot rows). Every reader member path matches exactly one row by name (slot[0..2] match `slot.exec`, `slot.grom`, `slot.cartridge` by position and field). No reader member is uncovered by a row and no row is uncovered by a reader member. No `row-missing` finding, no member the digest table misses.
- Classes: 87 of 88 agree. The reader folded nothing and split nothing: `mem.stic_reg` (with its 24 reserved slots) and `mem.gate_open` (two fixed entries, two dynamic) were each read as one STATE array, as the checklist has them.
- One disagreement, `phase`: the reader said `unclear`, reason: the `IVS_PHASE_*` values and their readers are not in the six input files. The reader added that it is probably STATE. Resolution: the reader was limited by the blinded inputs, not the checklist wrong. The class definitions and the instance struct make `phase` a lifecycle value that a later step reads, which is the STATE definition; the checklist keeps STATE. A note, not a correction.
- Situations (the non-default case the reader gave for each STATE member) were not graded against the checklist witnesses. They are the reader's own cases and agree in kind with the witnesses (for example cpu.dbd after SDBD, psg.env_* mid-envelope, obs.input.floor_tick after an event). The reader inferred `stic.cs_ptr`, `blocked_reads`, `blocked_writes`, `late_grants` and `grants_in_frame` from their comments because the headers do not show a consumer; the checklist assigns them witnesses and tests of their own, and plan 08-14 carries those out.
- The reader called `mem.exec_loaded`, `mem.grom_loaded`, `mem.cart_count` and `mem.cart_seg` BOUND because they record the structure of the loaded images; this equals the checklist.

## Result

Corrections: 0. The oracle data (the checklist, the row descriptors, the pinned schema digest) is unchanged, so the frozen baseline is the oracle commit named in line 2. Plan 08-10 may start.
