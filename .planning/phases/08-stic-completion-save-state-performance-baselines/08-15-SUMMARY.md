---
phase: 08-stic-completion-save-state-performance-baselines
plan: 15
subsystem: Phase 8 wave B, plan 7 - the corruption and fuzz family of the save state - a position classifier and one invariant checker that every case runs through, exhaustive truncations and byte flips with and without the trailer, header bit flips, length and table lies, one derived invariant violated at a time, saturated counters, a seeded random family against a model of the container layer, the instance-free describe sweep, handles, repeatability, a self-control of the checker and a libFuzzer target outside the gate
tags: [B, STA-01, corruption, fuzz, restore, describe, ivsn-v1, classifier, model, libfuzzer, D-16, D-20, D-37, D-40, D-41, D-44, T-08-36]
status: complete

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-14 end state (the hard-point suite, state_util.c builders, the poisoned-target comparison, the 18 cuts); counts at plan start steps=38/38 controls=378/378 faults=235/235 fault-runs=235 tests-expected=4888 tests-executed=4884 tests-skipped=4 tests-missing=0 api-calls=19 public-symbols=19 state-rows=88 state-cuts=18"
provides:
  - "tests/state/state_fuzz.c: the corruption family over restore and the instance-free describe: the fixture (one valid snapshot of 4,299 bytes of a Lamp Tone instance in the middle of its third frame, one target with its STATE and PAD rows poisoned and a baseline copy), the checker fz_check, the position classifier fz_classify (19 classes, written from the layout table and the check order), the model of the container layer fz_model_run, the sweeps and 12 test ids (4888 to 4912); no core file changed, no fault, no control"
  - "tests/fuzz/fuzz_snapshot.c: the libFuzzer target, behind the existing IVS_ENABLE_LIBFUZZER block of tests/CMakeLists.txt, EXCLUDE_FROM_ALL, in no gate step, no CI job and no inventory; built with Homebrew LLVM 21 and run 100,000 times from one valid snapshot without a violation"
  - "docs/formats/ivsn-v1.md: the section Fuzzing (as built, plan 08-15), appended below the frozen sections (0 lines removed)"
affects: [08-16, 08-17]

actuals:
  tokens: 32011
  tasks: 3
  commits: 3
plan_head_before: a811588ead9169e55498517e7340d9bdc8a11473
plan_head_after: 6d2b4b86db4775f408edd0bd42be23e2036dedf8

key-decisions:
  - "One checker for every case, and an expectation per case written from the specification: a flipped byte is judged by a position classifier (the layout table and the check order give, per byte, the reason, the offset, the section and the slot kind with the trailer left as it was and the allowed set with it recomputed), a multi-byte or whole-length mutation by a small model of the container layer (steps 2 to 6 of the check order) written from the same text; neither reads machine_snapshot_read.c or machine_snapshot_restore.c, and the classifier agreed with the code on every single-flip case at the first run, offsets included: 8,598 unsealed, 8,598 resealed and 9,792 header-bit cases, the sets the plan lists (the 3,190 and 3,968 now built are subsets of the last two)"
  - "A refusal is held against the WHOLE 114,416-byte target by memcmp with a baseline copy whose STATE and PAD rows are poisoned, an acceptance against the digest field table read as data (nothing outside a STATE or PAD row is written, every PAD row is zero, the state-only preimage of each changed section equals the input's) and, on a bounded subset (the first 16 accepted cases of an id and every 97th), against all seven sections, a full save with the eight informational digest fields and the trailer taken from the input (the one named exception of D-41), the report of describe and a 6,000-tick run of the restored machine"
  - "The family is made cheap by thinning what is redundant and keeping what is distinct, in both presets and the same: the second value of a free-row byte and the interior of the long free rows in the resealed byte sweep, one rotating bit of each digest byte in the header bit sweep; a run of the plan's literal sets (8,598 and 9,792 cases) passes too and costs 1.3 cpu-s more on the dev preset and 0.8 on release (deviation 1)"
  - "The checker is shown able to fail (state.judge.fuzz-invariant-detects), and the family catches the nine faults of the suite collaterally (each restore or reader fault fails the ids the property it breaks calls for; none of the new ids fails where it should not)"
  - "Two acceptances are known and pinned, not closed: the STIC register words are stored masked by the write path but nothing refuses a bit outside the mask on restore (256 of 256 flips accepted), and machine time is not tied to a unit boundary; closing either needs a new invariant proved on the reachable states and a change to the core, which this plan does not make"

key-files:
  created:
    - tests/state/state_fuzz.c
    - tests/fuzz/fuzz_snapshot.c
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-15-SUMMARY.md
  modified:
    - tests/CMakeLists.txt
    - tests/state/state_cases.h
    - tests/state/state_main.c
    - data/expected/tests/state.ids
    - docs/formats/ivsn-v1.md
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-14
---

# Phase 8 Plan 15: the corruption and fuzz family Summary

**Every truncation and every single-byte flip of a real snapshot, with the trailer left as it was and with it recomputed, every bit of the header, the length and table lies, each derived invariant violated alone and a fixed-seed random set now end in a defined outcome: a refusal with the reason, offset and section the specification's layout dictates and the whole target instance byte-identical, or an acceptance that re-encodes canonically. A position classifier (19 classes) and a model of the container layer, both written from `docs/formats/ivsn-v1.md` and not from the code, give the expectation of each case; one checker enforces the invariants of 08-DISCUSS-state 6.1 on each (a documented (status, reason) pair, a poisoned 400-byte report written only as the contract says, the whole 114,416-byte target equal to its baseline after a refusal, nothing outside the STATE and PAD rows written and the sections re-encoded after an acceptance, two attempts identical), and a self-control shows it can fail. The instance-free describe call is swept over heap buffers of exactly the input size, handles and overlaps are refused before a byte is read, and `tests/fuzz/fuzz_snapshot.c` carries the same invariants into a libFuzzer target outside the gate (built with Homebrew LLVM 21, 100,000 runs from one valid snapshot, no violation). 12 test ids (4888 to 4912), no core change, no fault, no control; the corruption family found no defect and no validator gap that a proof about reachable states could close, and two acceptances are pinned and recorded. The gates pass under sh, dash and the Ubuntu container with the plan-end counts `steps=38/38 controls=378/378 faults=235/235 fault-runs=235 tests-expected=4912 tests-executed=4908 tests-skipped=4 tests-missing=0 api-calls=19 public-symbols=19 state-rows=88 state-cuts=18`. STA-01 and STA-02 stay Pending until plan 08-17 closes them on its terms.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `68c9d70` | 1 (tracer) | the fixture, the checker, the position classifier, `state.fuzz.truncations` and `state.fuzz.byte-flips` (ids 4888 to 4892) |
| `de91ad1` | 2 | `state.fuzz.byte-flips-resealed`, `state.fuzz.header-bit-flips`, `state.fuzz.length-and-table-lies`, `state.fuzz.semantic-lies`, the known acceptance of `mem.stic_reg`, the bounded subset also comparing the report with describe and running the restored machine (ids 4892 to 4900) |
| `6d2b4b8` | 3 | `state.fuzz.saturated-counters`, `.random-resealed`, `.describe-heap`, `.different-handle`, `.second-attempt-identical`, `state.judge.fuzz-invariant-detects`, `tests/fuzz/fuzz_snapshot.c` and its CMake block, the Fuzzing as-built section of the specification (ids 4900 to 4912) |
| the commit that adds this file | | this SUMMARY, STATE.md and ROADMAP.md |

## The family: what each id holds

The input is one valid snapshot of 4,299 bytes: a Lamp Tone instance (the `lamp-pass` scenario, so the first of its six input events is applied) stopped at the first unit boundary at or after machine time 156,400, frame 2 cycle 9,229. It is restored into a second instance of the same three images, powered, run to tick 20,000, with every STATE and PAD row poisoned 0xA5; its 114,416 bytes are copied once and put back after every accepted case.

| Id | Attempts | Refused | Accepted | Subset (full checks) | What it holds |
|----|----------|---------|----------|----------------------|---------------|
| `state.fuzz.truncations` | 4,299 | 4,299 | 0 | n/a | every length 0 to 4,298 on a heap buffer of exactly that length: 61 at the supplied length (offset = length) |
| `state.fuzz.byte-flips` | 8,598 | 8,598 | 0 | n/a | every offset, XOR 0xFF and XOR 0x01, trailer left: the header fields by their own reasons (62 to 68 at the field, 61 or 67 for the total by arithmetic, 69 for the table), everything else 70 at the start of the trailer |
| `state.fuzz.byte-flips-resealed` | 3,190 | 1,258 | 1,932 | 35 | trailer recomputed: header classes as above, the profile id 71 or 72, the parameter digest 74, the slot records 76 and 78, frames 78, count prefixes 77, values accepted, 78 or 79, informational digests and the repaired trailer accepted |
| `state.fuzz.header-bit-flips` | 3,968 | 3,712 | 256 | 18 | every bit of every header byte that is not a digest byte (196 bytes x 8 bits) and one rotating bit of each of the 416 digest bytes, each with and without the reseal |
| `state.fuzz.length-and-table-lies` | 126 | 126 | 0 | n/a | 117 rows and 9 whole-field cases: 12 totals, a buffer one byte short, 5 extra-byte lengths, 4 lengths above the bound, 3 format versions, each of the 16 flag bits and all 16, 4 schema versions, 7 section counts, 3 magics, 2 schema digests, 26 section-table lies (wraparound, overlap, gap, unknown, duplicate and swapped domains, reserved bytes, a section one byte long or short, with and without the reseal), 8 whole-field id forms and 6 one-byte ones, 3 parameter digests, 24 slot rows (eight for each of the three slots), the buffer of length 0 |
| `state.fuzz.semantic-lies` | 28 | 28 | 0 | n/a | INV-2 (gate flag, gates 0 and 3, cursor +1 and -1), INV-3 (index +1 and -1, emitted, a level of 16, a fourth byte of the levels, a register bit its mask drops), INV-4 (pull-down, reserved bit, an unconnected word, the floor above time), INV-6 (two), and 11 validator and scalar rows (event index 16, SDBD with the interruptible flag, FAULTED without a reason, run state 9, a status bit, phase 0 and 2, time 2^63 + 1, a tone flag of 2, an envelope level of 16, the color stack pointer 4): 79 or 78 at the first element of the struct the rule concerns |
| `state.fuzz.saturated-counters` | 2 | 0 | 2 | 2 | the four 32-bit STIC counters at 0xFFFFFFFF and the three free 64-bit counts at 2^64 - 1; machine time at its largest (an even number of frames, 154,402,237,124,256 of them) with the frame index, video count, PSG sample index and emitted count moved with it, accepted, re-encoded, and its machine runs to 2^63 |
| `state.fuzz.random-resealed` | 1,200 | 777 | 423 | 20 | seed 0x9E3779B97F4A7C15, one to eight bytes at random offsets, odd cases resealed with 15 of 16 bytes in the payload, judged by the model of the container layer (a bad container: exactly its reason, offset and section; a good one: accepted or 72 to 80) |
| `state.fuzz.describe-heap` | 4,484 | 3,435 | 1,049 | n/a | a third of the lengths, every header byte and every 13th payload byte XOR 0xFF with and without the reseal, every 5th header byte with all its bits, every row of the lies and semantic tables, 250 random cases, all through `ivs_snapshot_describe` on heap buffers of exactly the input size: status, reason, offset and section equal the model's, the report is written as the contract says (a refusal: reason, offset, section and the header facts only; a success: the 360-byte v1 report equal to what the bytes state) |
| `state.fuzz.different-handle` | 10 | 10 | 0 | n/a | a NULL handle, a destroyed one, one with a damaged magic, an out-of-range stored profile index and another profile index in the memory system (BAD_HANDLE, reason 29, the buffer is one byte and the length claimed is 4,299, so a read is an error), the arguments before the handle (58), a buffer across the start, inside and across the end of the instance (81) and a report inside the instance (nothing written) |
| `state.fuzz.second-attempt-identical` | 623 cases, 1,246 calls | 574 | 49 | 32 | the truncations (every 97th), the byte flips (every 59th and 43rd), the header bits (every 29th byte), all lies and semantic rows, every 8th of 400 random cases and the two saturated snapshots, each run twice with the other poison in the report on a target put back: status, reason, offset, section, kind and, after an acceptance, the instance are identical |
| `state.judge.fuzz-invariant-detects` | 49 control runs | n/a | n/a | n/a | the checker fails on: a classifier row with another reason (one of each of the 19 classes), a wrong offset, an acceptance expected where the case is refused and the other way round, a byte of the restored target changed, a byte of the re-encoded section changed, a byte of the target changed after a refusal, a byte of the report changed; the untampered control of each passes |

The reasons the sweeps meet: the unsealed sweeps 61 to 70, the resealed ones 61 to 69, 71, 72, 74, 76, 77, 78, 79, the lies all of 60 to 72, 74, 76, 78, the semantic rows 78 and 79; 73 is the seam of plan 08-13 (one registered profile), 75 needs a target without an image and is the matrix of plan 08-13 (deviation 4), 80 is reached by the chained lengths of plan 08-13 and 81 and 58 by `state.fuzz.different-handle`. Each id fails when it has not met the reasons it exists to meet and when a sweep that expects acceptances has none.

### The position classifier

Written from the layout table, the check order and the "As built" notes of the reader (the offset a refusal reports). The class of an offset, with the outcome of one flipped byte:

| Class (bytes) | Trailer left | Trailer recomputed |
|---------------|--------------|--------------------|
| magic (0 to 7) | 62 at the byte | the same |
| format version (8, 9), flags (10, 11), schema version (16 to 19), section count (20 to 23), schema digest (24 to 55) | 63, 64, 65, 68, 66 at the field | the same |
| total (12 to 15) | 61 at the length when the flipped total is above it, else 67 at 12 | the same |
| profile id (56 to 119) | 70 | 71 at the first offending byte, or 72 (well formed, not registered) |
| parameter digest (120 to 151) | 70 | 74 at 120 |
| slot loaded flag, SHA-256, length (152 to 271) | 70 | 78 at the record (or at the length when 1 becomes 0), 76 at the SHA-256, 76 at the length, slot kind 1 to 3 |
| composite digest (272 to 303), section digests | 70 | accepted (informational) |
| section table domain, reserved bytes, offset, length | 69 | 69 |
| frame of a section, count prefix of an array row | 70 | 78 at the byte, 77 at the prefix |
| value of a row | 70 | accepted or 78 or 79; a free row only accepted |
| trailer | 70 | the reseal repairs the flip: the valid snapshot, accepted |

Everything the table says was true of the code at the first run: all 8,598 unsealed, 8,598 resealed and 9,792 header-bit cases (the plan's literal sets; reason, offset, section and kind where the table fixes them) and all 4,484 describe cases, whose expectation is the model. The one thing the classifier does not name is which entry or offset the reader reports for a defect of a section table domain (the reader may name the repeated or the out of order one), so those rows check the reason only.

## Accepted by design, the known acceptances and the gap analysis

The per-row counts of the resealed byte sweep (refused/accepted; XOR 0xFF at every offset of the constrained rows, XOR 0x01 as well, and for a free row the first 8, the last 8 and every 4th element of a long one):

| Rows | Refused/accepted | Why an acceptance is by design |
|------|------------------|--------------------------------|
| free by the specification: `cpu.r` 0/16, `mem.sysram` 0/280, `mem.scratchpad` 0/152, `mem.gram` 0/280, `stic.row_latch` 0/144, `stic.blocked_reads`, `blocked_writes`, `late_grants`, `grants_in_frame` 0/4 each, `obs.video.chain`, `audio.chain`, `input.chain`, `psglog.chain` 0/32 each, `obs.audio.count`, `input.count`, `psglog.count` 0/8 each | none refused | counters, chains, RAM words and the latched card rows are free values ("Derived invariants"): the trailer is the only check, and every flip is accepted and re-encoded |
| a Boolean or small member: `cpu.intff` 1/1, `cpu.prev_interruptible` 1/1, `cpu.halt_pending` 1/1, `cpu.busak` 1/1, `cpu.run_state` 1/1, `stic.display_latch`, `windows_active`, `mode`, `cs_ptr` 7/1 each, `psg.tone_out` 21/3, `psg.env_dir` 7/1, `psg.env_cycles` 7/1, `psg.env_level` 7/1, `psg.noise_counter` 7/1, `psg.noise_lfsr` 3/5, `psg.tone_counter` 15/9 | the flips that leave the range or break a cross-field rule | XOR 0x01 lands on another value of the range (0 for 1, 2 for 3, a HALTED run state for RUNNING): a state some step can make or that no rule connects to the rest of the state |
| `cpu.status` 2/0, `cpu.dbd` 2/0, `cpu.fault_reason` 2/0, `mem.gate_open` 8/0, `mem.port_pulldown` 4/0, `phase` 8/0, `psg.next_sample` 16/0, `psg.env_counter` 8/0, `psg.env_held` 8/0, `stic.frame_index` 16/0, `stic.event_index` 8/0, `obs.video.count` 16/0, `obs.audio.emitted` 16/0, `obs.input.words` 32/0 | none accepted | the validators and invariants refuse every flip |
| `mem.psg_reg` 40/24 | the bits its write mask drops are refused | a bit inside the write mask is another register value (INV-3) |
| `obs.audio.prev_levels` 5/3, `obs.input.floor_tick` 14/2, `time_ticks` 15/1 | | a level of at most 15, a floor at most machine time, and a machine time one tick off that the sample index, the frame cursor and the floor still agree with |
| informational digests (composite 0/64 and section digests 0/448 byte flips; 256 header bit flips) | | D-41: the eight fields are not authenticated, and the full save of the subset differs from the input only there and in the trailer |
| the trailer, resealed | accepted 64 | the reseal repairs the flip: the input is the valid snapshot |
| `mem.stic_reg` 0/256 | **known acceptance** | see below |

**Known acceptances (pinned, not closed).**
- `mem.stic_reg`: the 64 STIC register words are stored masked by the write path of the memory system (`reg_wmask`), and the collision words are ORed with 0x03FF, so a word with a bit outside its mask is a state no step makes, and a guest read returns the stored word. No validator or invariant refuses it; all 256 flips of the sweep are accepted, canonically re-encoded and, on the subset, run for 6,000 ticks without a sanitizer report. The test pins the acceptance (a row list `fz_known_rows`), so a later validator is a conscious change. A candidate INV-7 (every stored STIC register inside its write mask, or inside 0x03FF for a collision word) is derivable and would be proved on the 411,075 boundaries of plan 08-13, but it edits the core and the invariant function and is not made here (open item).
- `time_ticks`: not tied to a unit boundary (all units in the programs of plan 08-13 start on a master tick that is a multiple of the CPU cycle, but no invariant says so), so a snapshot that is one tick off the saved time is accepted when the other invariants still agree. Recorded in the specification; no invariant is added.

**Validator gaps closed in the core: none.** The plan allows closing a gap in the core only when it is provably true of reachable states; the two above are candidates, neither was proved here, and the only core change of the plan is none (`git show --stat` of the three commits lists no `core/` file).

## What each new test can fail on (mutation probes in the working tree, reverted)

| Probe | Failing ids (new ids in bold) |
|-------|-------------------------------|
| `machine_snapshot_read.c`: the flags check removed | read.describe-refuse-header, read.reasons-covered, judge.matrix-expectation-corrupt, restore.refuse-structure, restore.reasons-covered, restore.atomic-every-refusal, **fuzz.byte-flips, byte-flips-resealed, header-bit-flips, length-and-table-lies, describe-heap, second-attempt-identical, judge.fuzz-invariant-detects** |
| `machine_snapshot_restore.c`: the frame check made unreachable | restore.refuse-content-validators, restore.reasons-covered, restore.atomic-every-refusal, **fuzz.byte-flips-resealed, random-resealed, second-attempt-identical** |
| the commit does not zero the PAD rows | restore.poisoned-target-equals-source, same-profile-any-instance, cut.cpu-points, cut.stic-points, cut.device-points, cut.latch-not-recomputed, judge.cut-compare-detects-divergence, sweep.row-omission, judge.row-sweep-detects-omission, **fuzz.byte-flips-resealed, header-bit-flips, saturated-counters, random-resealed, second-attempt-identical, judge.fuzz-invariant-detects** |
| INV-6 made always true | restore.refuse-content-invariants, restore.reasons-covered, restore.atomic-every-refusal, judge.refusal-expectation-corrupt, **fuzz.semantic-lies, second-attempt-identical** (the resealed byte sweep still accepts the changed video count, because its allowed set for a constrained row includes an acceptance: the semantic rows are what hold each invariant) |

The nine faults of the suite, each run as its own executable (reduced point set), fail these new ids collaterally and `state.fuzz.different-handle` and the other must-pass ids stay green where they should: SNAPSHOT_LITTLE_ENDIAN and SNAPSHOT_PAD_SERIALIZED, SNAPSHOT_IMAGE_BYTES_EMBEDDED and SNAPSHOT_STIC_ROWS_OMITTED (the writer: the fixture itself is wrong, every id but at most one fails), SNAPSHOT_TRAILER_UNCHECKED (byte-flips, header-bit-flips, length-and-table-lies, random-resealed, describe-heap, second-attempt-identical, the judge), SNAPSHOT_LENGTH_TRUSTED (truncations, length-and-table-lies, describe-heap, second-attempt-identical, the judge), RESTORE_PROFILE_UNCHECKED and RESTORE_IMAGE_DIGEST_UNCHECKED (byte-flips-resealed, header-bit-flips, length-and-table-lies, second-attempt-identical), RESTORE_MUTATES_BEFORE_VALIDATE (byte-flips-resealed, semantic-lies, second-attempt-identical: a refusal that only a STIC validator makes leaves the CPU rows committed, and the whole-instance comparison after every refusal sees it). No fault is duplicated by a sweep (no new fault, no new must-fail id), as the plan says.

## The libFuzzer target

`tests/fuzz/fuzz_snapshot.c`: the first input byte is a control byte (bit 0: recompute the SHA-256 trailer over the rest), the rest is copied to a heap buffer of exactly its size; the target restores into a live instance of the stand-in content put back from a baseline copy, holds the invariants above (a documented pair; describe and restore agree on the container; a refusal leaves the instance byte-identical; an acceptance writes only STATE and PAD rows, re-encodes every section, a full save differs from the input only in the eight digest fields and the trailer, the restored machine runs 6,000 ticks; a second attempt is identical) and aborts on a violation. The environment variable `IVS_SNAPSHOT_FUZZ_SEED` makes the first start write the corpus seed (the control byte 1 and one valid snapshot).
- Compile check: `cc -fsyntax-only -std=c17` with the project include paths and the warning list of the build (`-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wstrict-prototypes -Wmissing-prototypes -Wcast-qual`) prints nothing (host compiler: Apple clang).
- Link and run (Homebrew LLVM 21.1.8 has the runtime; Apple clang has none): a scratch build directory outside the repository, `-DCMAKE_C_COMPILER=<llvm@21 clang> -DCMAKE_C_FLAGS=-fsanitize=fuzzer-no-link -DIVS_ENABLE_LIBFUZZER=ON -DIVS_LIBFUZZER_SANITIZERS=fuzzer,undefined` (the AddressSanitizer runtime is not used: it hung at start-up on this macOS 26 host with Homebrew LLVM 18 and 21 before, see the header of `fuzz_ivsc.c`), the seed written, then 100,000 runs (`-runs=100000 -seed=7`, 173 s): no crash, no violation, no artifact; the corpus grew from 1 input to 133 (461 KB) and 866 coverage points (1,261 features) of the instrumented core. The corpus and the build stay in the scratch directory; nothing was committed.
- The target is referenced by no gate step, no CI job and no inventory: `grep -rn fuzz_snapshot tools .github` prints nothing, and the only mention under `tests/` is its own source and the CMake block.

## Deviations from Plan

**1. [Cost stop rule - fewer attempts than the plan's literal sets, in both presets] `state.fuzz.byte-flips-resealed` takes 3,190 attempts and `state.fuzz.header-bit-flips` 3,968, not 8,598 and 9,792.** The plan lists every offset with XOR 0xFF and XOR 0x01 resealed and every bit of the header with and without the reseal. At the first run the literal sets measured 1.16 s and 0.81 s of cpu on the dev preset (0.71 and 0.52 on release): the whole family would have added about 2.8 cpu-s to the dev preset and 1.9 to release (about 4.6 hosted cpu-s more than built), on a projection that the plan's cost stop rule limits to 8.3. The cut removes only redundancy: the second value (XOR 0x01) of a byte of a free row (XOR 0xFF flips every bit of a byte and the valid snapshot's own round trip shows each bit restored as it was, so each bit of the byte is shown restored as 0 and as 1) and the interior of the four long free rows (the 512 system RAM words, 256 scratchpad words, 512 graphics RAM words, 240 latched card words: taken at the first 8, the last 8 and every 4th element, both bytes of each, because a free row is one array decoded by one loop and the boundary elements are where a loop that drops or doubles one shows); and, in the header bit sweep, 7 of the 8 bits of each of the 416 digest bytes (a digest is compared or ignored as a whole; one bit per byte, rotating with the offset, covers all eight positions in every digest, and every byte of every digest takes XOR 0xFF and XOR 0x01 in the byte sweeps). Every class of the classifier, every constrained row, every frame, count prefix and header byte keeps both values. **The literal sets were run once with the thinning removed in a scratch edit (reverted): 8,598 resealed attempts (1,258 refused, 7,340 accepted, 91 subset cases) and 9,792 header bit cases (7,744 refused, 2,048 accepted, 37 subset cases) all pass in the dev and the release preset.** The unsealed byte sweep keeps all 8,598 attempts as the plan says; `state.fuzz.describe-heap` (not sized by the plan), the random family (N = 1,200 for about 0.13 s of the dev preset, under the plan's bound of 1.5 s) and `state.fuzz.second-attempt-identical` are sized by measurement.

**2. [Rule 2 - correctness, an addition] The bounded subset also runs the restored machine and compares the report with describe.** The plan's subset is the full save; a state that restore accepts and the core cannot run is the failure a hostile snapshot would cause next, so each of the first 16 accepted cases of an id and every 97th runs 6,000 ticks through the public call (any sanitizer report or a refusal fails the case) and compares the whole 400-byte report with what `ivs_snapshot_describe` writes for the same bytes. The 91 accepted cases of the literal sweep, the 20 of the random family and the largest-time snapshot (run to 2^63) all ran clean.

**3. [Interpretation] The classifier and the model name the exact reason, offset, section and slot kind, not the allowed sets the plan lists for several classes.** The plan names the allowed set for a resealed flip (the profile id 71 or 72, a slot record 75 or 76 or 78, payload 77 to 80); the specification and the "As built" notes give each class one outcome, so the classifier says it exactly (the id form is computed from the changed field, the loaded flag by arithmetic, the total by comparison with the length) and the family is stronger for it. The sets stay for the value bytes (accepted, 78 or 79) and for the profile id when the model of a random case cannot know the registry. A section table defect names the reason only where the reader may name the repeated or the out of order entry.

**4. [Interpretation] Reason 75 (a slot loaded in the snapshot and missing in the target) is not produced by this family.** The fixture has one target, which holds all three images, so a snapshot's slots always match and a flipped slot record ends in 76 or 78; 75 needs a second target without a cartridge and is the matrix of plan 08-13 (`state.restore.refuse-identity-slots`, 37 rows). The plan's list does not name 75; 08-DISCUSS-state 6.1 does ("slot empty in the target").

**5. [Information] The judge is not isolated from the core.** `state.judge.fuzz-invariant-detects` runs real calls (an untampered control of each tamper), so it fails together with the ids it judges when the code breaks the class of its control (the flags probe above fails it); it shows the checker can fail, not that the core works, which the other ids do. Same shape as `state.judge.refusal-expectation-corrupt` of plan 08-13.

**6. [Information - the fault-run time target] The nine fault runs sum to 7.7 to 8.1 local cpu-s (user plus sys, unsanitized `-O1`, the whole suite once each, the container gate running at the same time), 1.0 to 1.4 above the 6.7 of plan 08-14 and above the plan's target of about 6.** Per fault run: SNAPSHOT_LITTLE_ENDIAN 0.68 to 0.75, SNAPSHOT_PAD_SERIALIZED 0.50 to 0.57, SNAPSHOT_IMAGE_BYTES_EMBEDDED 0.23 to 0.25, SNAPSHOT_TRAILER_UNCHECKED 1.12 to 1.28, SNAPSHOT_LENGTH_TRUSTED 1.05 to 1.20, RESTORE_PROFILE_UNCHECKED 1.12 to 1.20, RESTORE_IMAGE_DIGEST_UNCHECKED 1.17 to 1.19, RESTORE_MUTATES_BEFORE_VALIDATE 1.13 to 1.20, SNAPSHOT_STIC_ROWS_OMITTED 0.55 to 0.58. The family under `IVS_STATE_FAULT_RUN` takes every 16th offset or length, bit and random case (about 2,400 attempts and the fixture, 0.12 to 0.17 s per fault run); the lies and semantic tables (154 rows) run whole in a fault run, which is about 0.04 s of it, and thinning them did not move the measurement beyond its noise, so it was not kept. The overshoot of the target is recorded here and in the cost section; no check was dropped.

**7. [Information] The unsealed sweeps were not thinned; the plan's own rule for Task 1 stands.** Both values at all 4,299 offsets: 8,598 attempts, about 0.46 s on the dev preset and 0.30 on release (without the fixture), which is the part of the family a literal reading of the plan still asks for and the first candidate if a hosted row reads 8.3 or more (the second value of a byte that only the trailer judges adds nothing, SHA-256 cannot tell the flips apart: it would remove 3,655 attempts, about 0.2 cpu-s per preset).

No auth gates, no package installs (T-08-SC), no `gh` call, no push, no tag, no `tools/ci-record.sh`; `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` were never staged. No core file changed. D10 and D11 are not performed and no claim above is more than provisional: every statement is about the repeatability of one build and one profile, and nothing is measured on hardware.

## Decisions Made

- The expectation of a hostile-input case is written from the specification by a classifier (one flipped byte) or a model (several bytes, or a lie about the length), never from the reader or restore; the classifier had no disagreement with the code, which is evidence about the text and the code together, and every disagreement would have been a finding about one of them.
- The refusal oracle is the whole instance, not the rows: 114,416 bytes compared with a baseline copy after every refusal, so a byte outside every row of the table, a pad byte or a BOUND row changed by a refusal is seen.
- The acceptance oracle is the digest field table read as data (what restore may write, what it must zero, what the sections must re-encode to) plus a bounded set of heavier checks, so the canonical re-encode rule of the specification holds for the accepted cases at a cost that stays a few microseconds each.
- What is accepted and why is a result of the family, listed and pinned (the free rows by the specification, the in-range alternates, the digests, and the one known acceptance), not an excuse: a later validator turns a pinned acceptance into a refusal by changing one list.

## Cost (laptop)

| Item | Value |
|------|-------|
| estimate of the plan | 130,000 tokens (raw, confidence low), 3 tasks |
| actuals | 32,011 tokens (chars/4 over the added lines of the 3 commits outside `.planning/`), 3 tasks, 3 commits (measured from the ledger of the plan head) |
| gate total, sh and dash, Task 1 tree | 99 s and 103 s, identical summary line (md5 819c1591376e05dc191c7b382a293f99) |
| gate total, sh and dash, Task 2 tree | 105 s and 106 s, identical summary line (md5 74c6aca1c0e0151701a3da75853dcd1a) |
| gate total, sh and dash, Task 3 tree | 103 s and 98 s (`gate-cpu` user 182.8 sys 155.8 and user 170.4 sys 142.5), identical summary line (md5 f57a0054a29d22b79536f47a24b490b4) |
| container gate (gcc 13.3.0, cmake 3.28.3, x86_64) | Task 1 tree 389 s, Task 2 tree 372 s, Task 3 tree 375 s, closing tree 353 s (the first three ran while other work shared the laptop); all exit 0, 0 FAIL lines |
| suite `state` (61 ids), whole | dev preset (sanitized, `-O1`) 3.4 to 3.9 s user plus 0.15 s sys (49 ids before the plan: 2.5 and 0.15), release 2.2 to 2.3 s user (49 ids before: about 1.3) |
| the 12 new ids, the whole family in one process | dev 1.55 s user plus 0.11 s sys, release 1.00 s |
| per id, one id per process, so each carries the fixture (about 0.08 s dev, 0.04 s release), dev and release cpu | truncations 0.15 and 0.08, byte-flips 0.54 and 0.34, byte-flips-resealed 0.40 and 0.25, header-bit-flips 0.29 and 0.18, random-resealed 0.18 and 0.11, describe-heap 0.29 and 0.18, second-attempt-identical 0.15 and 0.09, the other five 0.01 each |
| the literal sets (deviation 1, a scratch run, not committed) | byte-flips-resealed 8,598 attempts 1.16 dev and 0.71 release, header-bit-flips 9,792 cases 0.81 and 0.52 |
| 9 fault runs, sum | 7.7 to 8.1 local cpu-s (6.7 at the end of plan 08-14) |
| attempts per second | a refusal that reaches the trailer costs about 35 microseconds on release and 55 on the dev preset (the reader's schema digest and the SHA-256 of the trailer, about 37 microseconds per call as plan 08-13 measured), an accepted resealed case about 95 (a reseal, a restore, the instance compared with the baseline, one section re-encoded, the instance put back); the sanitizer costs little here because the work is hashing |

**The stop rule.** The additions of this plan, at the measured costs: the suite grows by 1.55 local cpu-s on the dev preset and 1.00 on release, the nine fault runs by 1.0 to 1.4 (no id is a must-fail id of a fault, so no fault is added and no fault build is added), no step, control or process case: about 3.7 local cpu-s, 6.2 hosted cpu-s at the factor 1.675 of the record. Over the `build_s` of the hosted rows (40 for the fastest of the last five, 46 for the newest recorded row 43658364307, 63 for the slowest) that is +0.15, +0.13 and +0.10 in `r`, and the projection after plan 08-14 (8.03 to 8.06) reads **about 8.13 to 8.21** after this plan, under the line 8.3 by 0.09 to 0.17 and over the reading 8.0. The stop rule did not fire and no test case was withheld; the margin left for plans 08-16 and 08-17 is therefore about 0.1 in `r`, which is the figure the checkpoint C entry of plan 08-17 starts from (the hosted row after the push settles the reading; `.planning/ci-runs.tsv` was not touched). If a hosted row reads 8.3 or more the first trims are the second value of the unsealed byte flips in the payload (0.2 cpu-s per preset) and then the point sets of `state.fuzz.describe-heap` and `state.fuzz.second-attempt-identical`; the literal sets of the plan would add 1.3 and 0.8 and are not the way back.

## Open Items

- **A candidate INV-7 (STIC registers inside their write masks) and the unit-boundary alignment of machine time** are the two known acceptances (`fz_known_rows` pins the first; `time_ticks` accepts a one-tick difference). Closing either is a change of `ivs_snapshot_invariants` with a proof on the 411,075 boundaries of plan 08-13 and a row in `state.fuzz.semantic-lies`; the owner or plan 08-17 decides whether it is wanted before STA-01 is closed.
- **Plan 08-17 (registry and close-out):** the close-out note of STA-01 and STA-02 can cite, besides the exemption list of plan 08-14, the family of this plan: attempts and accepted counts per id above, the one known acceptance and the pinned list of free rows. The cost figures above are the inputs of the `checkpointC-08` entry (the family adds 1.55 dev, 1.00 release and 1.0 to 1.4 of fault-run cpu-s; the nine fault runs sum to 7.7 to 8.1, over the target of about 6, which the entry should state).
- **Plan 08-16 (file options of `ivs-run`):** a restore from a file that a user supplies is the hostile-input case this family covers at the API; the runner's own file reader (length, a read of the whole file into a buffer that the call then sees) is not part of this plan.
- **`state.judge.fuzz-invariant-detects` depends on the core** (deviation 5); an isolated judge would need a checker that runs against a fake restore, which the suite has no seam for.
- **Information.** D10 and D11 are not performed; no claim above is more than provisional; no hosted-only check was run (the hosted-style container gate ran green); a snapshot is exact-build state; STA-01 and STA-02 stay Pending and REL-01 stays Pending (D-28).

## Interfaces for the next plans

- Tests: `state_fuzz.c` keeps its helpers static; the ids and counts are in `data/expected/tests/state.ids` (11 `state.fuzz.`, 6 `state.judge.`). The fixture, the classifier and the model are the place to add a row (an invariant that a later plan adds needs a row in `fz_build_semantic` and, if its rows are free or known, an entry of `fz_free_rows` or `fz_known_rows`).
- The target: `tests/fuzz/fuzz_snapshot.c`; the recipe is in its header and in the as-built section of `docs/formats/ivsn-v1.md`.
- Gate counts after this plan `steps=38/38 controls=378/378 faults=235/235 fault-runs=235 tests-expected=4912 tests-executed=4908 tests-skipped=4 tests-missing=0 api-calls=19 public-symbols=19 state-rows=88 state-cuts=18` (the next plan adds no id of this suite; plan 08-17 expects `tests-expected=4932`).
- Threat id T-08-36 is used here (T-08-SC throughout); the next state plans continue from T-08-37 or the numbering their plans assign.

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=38/38 controls=378/378 ... faults=235/235 ... fault-runs=235 ... tests-expected=4912 tests-executed=4908 tests-skipped=4 tests-missing=0 ... api-calls=19 ... public-symbols=19 ... state-rows=88 state-cuts=18 (0 FAIL lines)
dash tools/gate.sh:    the identical summary line (0 FAIL lines)
GATE_LINUX_PLATFORM= sh tools/gate-linux.sh: exit 0, build/gate-linux.log ends gate: PASS (gcc 13.3.0, cmake 3.28.3, x86_64), the plan-end counts, 0 FAIL lines
```

After the Task 1 tree: sh and dash `tests-expected=4892 tests-executed=4888` with the identical summary line (md5 819c1591376e05dc191c7b382a293f99), container exit 0 (389 s). After the Task 2 tree: `tests-expected=4900 tests-executed=4896`, identical summary line (md5 74c6aca1c0e0151701a3da75853dcd1a), container exit 0 (372 s). After the Task 3 tree: the plan-end counts under sh (103 s) and dash (98 s) with the identical summary line (md5 f57a0054a29d22b79536f47a24b490b4), and the container gate on the same tree: exit 0, `gate: PASS`, the plan-end counts, 0 `FAIL` lines, `gate-time total=375`. On the closing tree (this SUMMARY, STATE.md and ROADMAP.md added or edited; `tracked=2462`): sh (`gate-time total=91`, `gate-cpu` user 161.9 sys 130.8) and dash (90 s, user 161.6 sys 129.7) `gate: PASS` with the identical summary line (md5 1bf81a69e67a1ef7de32df35dc12495e), 0 `FAIL` lines, the plan-end counts above, and `GATE_LINUX_PLATFORM= sh tools/gate-linux.sh`: exit 0, `gate: PASS`, the plan-end counts, `core-imports=5`, 0 `FAIL` lines, `gate-time total=353` (`gate-cpu` user 1,340 sys 84 emulated); `build/gate-linux.log` holds that run. The dev-preset `ctest` suite `state` (61 ids) and the release-preset `state` suite pass; the nine fault executables fail the ids their lines of `data/expected/faults/state.ids` name, with only `state.` collateral (the new ids among it, listed above).

## Known Stubs

None.

## Threat Flags

None new. T-08-36 (a crash or an unbounded cost on hostile bytes): every one of the 8,598 + 3,190 + 3,968 single-byte and bit flips built (and the 8,598 + 9,792 of the plan's literal sets, run once), the 4,299 truncations, the lies and the 1,200 random mutations is judged on a heap buffer of exactly the input size under the dev preset's sanitizer, a refusal leaves the whole instance unchanged and each accepted case of the bounded subset is run; the cost is bounded by arithmetic (35 to 55 microseconds a refusal that reaches the trailer, a few thousand attempts an id) and measured above; the libFuzzer target is `EXCLUDE_FROM_ALL`, behind a cache option that is OFF, and in no gate step or CI job, and `state.judge.fuzz-invariant-detects` shows the checker can fail. T-08-SC: no installs.

## Self-Check: PASSED

Verified before the final commit: the SUMMARY and the created files of the plan exist (FOUND for `tests/state/state_fuzz.c`, `tests/fuzz/fuzz_snapshot.c` and the SUMMARY); commits `68c9d70`, `de91ad1` and `6d2b4b8` are ancestors of HEAD and `git rev-list --count a811588ead9169e55498517e7340d9bdc8a11473..HEAD` printed 3 before this file's commit; nothing is staged but the files of this commit and `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` are not staged; `git ls-files` lists no `.ivsc`, `.bin`, `.rom`, `.exec`, `.ivsn`, `.wav`, `.ppm` or `.vraw` file; the diff of the three commits outside `.planning/` holds no home, scratch or temp path and no "Intellivision"; no commit deletes a file and none touches a `core/` file; `grep -c '^state[.]fuzz[.]' data/expected/tests/state.ids` prints 11 and `grep -c '^state[.]judge[.]' ...` prints 6; `grep -c 'srand\|time(\|clock(' tests/state/state_fuzz.c` prints 0; `grep -rn fuzz_snapshot tools .github` prints nothing; `git diff a811588 HEAD -- docs/formats/ivsn-v1.md` removes 0 lines; the claim words of the text step do not occur untagged in the new section.
