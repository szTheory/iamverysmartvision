---
phase: 03-memory-map-content-loading-and-refusal
plan: 12
subsystem: api
tags: [power-on-reset, missing-system-image, step-seam, isolation, powered-prior, sweeps, public-headers, named-faults, D-05, D-06, D-07, D-12, D-13, D-14, D-20, D-34, D-36, D-37, A-08, MEM-04, API-01, API-06]

requires:
  - phase: 03-memory-map-content-loading-and-refusal
    provides: "plan 03-09: public headers, instance struct, state digest, suite machine; plan 03-10: load/unload ladder; plan 03-11: refusal matrix runner, priors f/x/s/c, out-parameter rule"
  - phase: 02
    provides: "ivs_cp1610_reset / ivs_cp1610_step (s, cfg, lines, port, log, log_cap, out) and the vector rows the stand-in encodings come from"
provides:
  - "ivs_machine_reset and IVS_RESET_POWER_ON: the tenth D-05 public function"
  - "Internal seam ivs_machine_step_unit_internal (core/src/machine/machine.h only) on ivs_memsys_bus"
  - "Registry field ticks_per_cpu_cycle, so machine time stays in master ticks (D2)"
  - "Families reset (6), iso (5), sweep (7); powered prior w and reset rows in the refusal matrix; 5 new prec rows"
  - "Suite headers (tests/headers-tap.sh): each public header alone as C17 and C++17"
  - "Five named faults (machine faults 25, total 115)"
affects: [03-13, 03-14, phase-04]

actuals:
  tokens: 24000
  tasks: 3
  commits: 3
plan_head_before: 8bbe9f3276f71d78886d8632668de3adaadca9bd
plan_head_after: d70943640bf461648d49e23c954a9074647e6a8b

tech-stack:
  added: []
  patterns:
    - "Shared stand-in module (tests/machine/machine_standin.c): two hand-assembled EXEC programs, generated GROM patterns, core-sealed one-segment cartridges, instances in exact-size heap storage"
    - "Checkpoint traces: composite and CPU digests after every 8th unit, a solo run of the same content as the reference"
    - "Oracle-free sweep with one reused instance proven equal to a fresh one before every load (memcmp after refusals, fresh digest and bytes after unload)"
    - "One shared out-parameter check (ivs_machine_out_overlaps) so one named fault covers create, info and state_digest"

key-files:
  created:
    - core/src/machine/machine_run.c
    - tests/machine/machine_standin.c
    - tests/machine/machine_reset.c
    - tests/machine/machine_iso.c
    - tests/machine/machine_sweep.c
    - tests/headers-tap.sh
    - data/expected/tests/headers.ids
  modified:
    - core/include/ivs/ivs_machine.h
    - core/src/machine/machine.h
    - core/src/machine/machine.c
    - core/src/machine/machine_digest.c
    - core/src/machine/machine_load.c
    - core/src/memmap/memsys.h
    - core/CMakeLists.txt
    - cmake/IvsMemmapTable.cmake
    - tests/CMakeLists.txt
    - tests/machine/machine_cases.h
    - tests/machine/machine_main.c
    - tests/machine/machine_refuse.c
    - data/expected/tests/machine.ids
    - data/expected/faults/machine.ids
    - data/expected/faults/sha256.ids

key-decisions:
  - "Machine time is master ticks (D2): the seam adds elapsed CPU cycles times the registry's ticks_per_cpu_cycle (4 for the D3 profile), not raw cycles as the plan text said; the registry field takes the former reserved u16, so the record layout is unchanged"
  - "Reset checks BAD_ARG (kind), BAD_HANDLE, then MISSING_SYSTEM_IMAGE (EXEC before GROM) on the slot records, and only then applies the power-on state; the slots, cartridge segments and profile index are untouched"
  - "Reset rows use prior f with builders that load one image (no-exec, no-grom), f (neither), x for the row kind EXEC (no-exec-with-cart), s for the kind rows and w for kind-2-powered"
  - "Argument BAD_ARG and option UNSUPPORTED rows also run on the powered prior w (17 more cases): those classes win over BAD_STATE"
  - "Sweep bases must load unmutated (else the sweep is vacuous); a byte that already holds 0x00 or 0xFF is no change and is skipped"
  - "Requirements: MEM-04 and API-01 complete; API-06 stays open for plan 03-13 (effect table)"

patterns-established:
  - "A test family that needs running instances uses machine_standin.c rather than its own image builders"

requirements-completed: [MEM-04, API-01]

coverage:
  - id: D1
    description: "Power-on reset with MISSING_SYSTEM_IMAGE and BAD_ARG/BAD_HANDLE refusals, effect none (D-14, A-08 e, SC3 reset rows)"
    requirement: MEM-04
    verification:
      - kind: unit
        ref: "build/dev/tests/ivs_tests_machine --only machine.refuse.reset. (9) and --only machine.prec.reset- (3), whole-storage memcmp and digest"
        status: pass
    human_judgment: false
  - id: D2
    description: "The powered prior blocks every load and unload (D-13, A-08 d) and sits in the ladder"
    requirement: MEM-04
    verification:
      - kind: unit
        ref: "25 machine.refuse.{load,unload}.*.w cases, machine.prec.unsupported-over-bad-state, machine.prec.bad-state-over-duplicate, machine.reset.power-cycle-one-way"
        status: pass
    human_judgment: false
  - id: D3
    description: "Two instances with different content step without affecting each other (D-36, SC5)"
    requirement: API-01
    verification:
      - kind: unit
        ref: "machine.iso.{interleaved-equals-sequential,poisoned-storage-identical,memcpy-relocation,destroy-mid-run,cross-load}, dev (ASan+UBSan) and release"
        status: pass
    human_judgment: false
  - id: D4
    description: "Public headers stand alone in C17 and C++17"
    requirement: API-01
    verification:
      - kind: unit
        ref: "ctest -R '^headers$' in both presets; dash tests/headers-tap.sh: 6 executed, 0 skipped on this host"
        status: pass
    human_judgment: false
  - id: D5
    description: "Lifecycle and isolation faults each fail a named row"
    requirement: API-06
    verification:
      - kind: integration
        ref: "sh and dash tools/gate.sh: faults=115/115 fault-runs=115"
        status: pass
    human_judgment: false

duration: 15min
completed: 2026-11-11
status: complete
---

# Phase 3 Plan 12: Power-on reset, the internal step seam, isolation, sweeps and the headers suite Summary

**`ivs_machine_reset` completes the ten-function D-05 surface. It refuses a missing EXEC or GROM with `MISSING_SYSTEM_IMAGE` and leaves the instance unchanged; on success it powers the instance. From then on every load and unload is `BAD_STATE` until destroy. The internal seam `ivs_machine_step_unit_internal` steps one CPU unit on the profile-driven bus, and a hand-assembled stand-in program runs through the profile's reset vector and writes RAM. Five isolation cases show two instances with different content stepping independently. Oracle-free sweeps show that mutated cartridges never leave partial state. A `headers` suite compiles each public header alone as C17 and as C++17. Five named faults each fail a named row. Final gate under sh and dash (byte-identical summary lines): `gate: PASS steps=25/25 controls=215/215 ... fault-runs=115 ... faults=115/115 ... tests-expected=2094 tests-executed=2094 tests-skipped=0 tests-missing=0`.**

## Performance

- **Duration:** about 15 min (2026-11-11T08:15:47Z to 08:30Z).
- **Tasks:** 3 of 3.
- **Files:** 22 in 3 commits (diff +1630 / -52).
- **Suite machine:** 356 cases.
  - Release: 0.21 s (was 0.159 s at 03-11).
  - Dev (ASan+UBSan): 0.43 s (was 0.33 s).
  - Family timings, dev: reset 0.06 s, iso 0.09 s, refuse 0.27 s (now 250 cases), prec 0.09 s.
- **Sweep family:** 0.07 s for 1,630 loads in dev. Loads per base: B01 555, P01 180, P02 221, P03 196, P04 213, P06 265.
- **Suite headers:** 6 compiler runs per preset. On this host all six cases ran, C++ included: Apple clang `c++`, 0 skipped.
- **Gate time (local, warm):** `gate-time total` 24 to 30 s (sh 30, 24, 25; dash 26, 25). These are local figures; the hosted figure is the orchestrator's to read (budget 40 s, revisit trigger 50 s).

## Accomplishments

### Reset and the step seam (`core/src/machine/machine_run.c`)

- **`ivs_machine_reset(m, kind)`**, public in `ivs/ivs_machine.h` with `IVS_RESET_POWER_ON 1u`.
  - **Check order:** `BAD_ARG` (any kind but 1, A-08 e), then `BAD_HANDLE`, then `MISSING_SYSTEM_IMAGE` (the profile's `required` slots, EXEC before GROM).
  - **Commit:** system RAM, scratchpad and GRAM are zeroed (the generator refuses any other power-on fill). The `stic-regs` and `gmem` gates open. `ivs_cp1610_reset` sets RESET_PENDING. Time becomes 0 and the phase becomes POWERED.
  - **Left alone:** slots, cartridge segments and the profile index.
  - **From POWERED:** reset is a power cycle with the same result. Nothing returns the instance to OFF (A-08 d).
- **`ivs_machine_step_unit_internal(m, out)`**, declared only in `core/src/machine/machine.h`. `cat core/include/ivs/*.h | grep -c step_unit_internal` prints 0.
  - **Refusals:** `BAD_ARG` (out NULL), `BAD_HANDLE`, `BAD_STATE` while OFF.
  - **CPU configuration:** `cpu.interrupt_accept_clears_intff`, `bus.release_latency_cpu_cycles` and `bus.wait_limit_cpu_cycles`, all from the registry.
  - **Lines:** all inactive.
  - **Port:** `ivs_memsys_bus` with `&m->mem` as its context, so the reset entry's IAB answers `reset.r7` (`$1000`).
  - **Call:** `ivs_cp1610_step(&cpu, &cfg, &lines, &port, NULL, 0u, &r)`. A NULL log with capacity 0 is accepted by `cp1610.c`, which only rejects a non-NULL log below 8.
  - **Time:** advances by `elapsed_cpu_cycles × ticks_per_cpu_cycle`.
  - **State:** none static.
- **Surface:**
  - `grep -c '^IVS_API ' core/include/ivs/ivs_machine.h` prints 8.
  - `grep -h '^IVS_API ' core/include/ivs/*.h | wc -l` prints 10.

### Stand-in programs (`tests/machine/machine_standin.c`)

The decles are hand-assembled from the project's own Phase 2 vector rows: `cpu.MVII.r1-load`, `MVO R1, $0200`, `INCR R1`, `DECR R3` and `B $1001`. The backward branch target is PC+2 plus the one's complement of the displacement. No assembler or other implementation was used.

| Program | Decles | Behaviour |
|---|---|---|
| A | `02B9 0155 0241 0200 0009 0220 0004` | R1 = $155; loop: store R1 to `$0200`, increment R1 |
| B | `02BA 002A 0242 0201 0012 000B 0220 0005` | R2 = $02A; loop: store R2 to `$0201`, decrement R2, increment R3 |

- **Every decle is at most 0x3FF**, so the 10-bit EXEC holds it unmasked.
- **Content A and B** pair a program with a GROM pattern and one core-sealed cartridge segment: A has 16 words at `$5000`, B has 24 words at `$6000`.

### Family reset (6 cases)

| Case | What it checks |
|---|---|
| `step-program` (tracer) | Program A runs until the store at `$1002` has run twice. Then `[$0200] = $0156`, R1 = $0156, PC lies in `$1002..$1006`, and time = cycles × 4 |
| `iab-reset-vector` | The first unit is RESET_ENTRY and leaves R7 = `$1000`. The registry's `reset_r7` and `ticks_per_cpu_cycle` match the profile literals. The next unit fetches `02B9` at `$1000` |
| `no-cart-ok` | Reset with EXEC and GROM only gives POWERED at time 0, with the images digest unchanged |
| `equivalence` | After 40 units and a power cycle, the composite digest and the instance bytes equal those of a fresh instance with the same content after one reset |
| `power-cycle-one-way` | A second reset is OK and keeps POWERED. Load and unload of every kind are `BAD_STATE`, and the instance is unchanged |
| `step-refusals` | Stepping while OFF (fresh, and loaded), out NULL, a NULL handle and a destroyed handle are each refused, with the bytes unchanged |

### Refusal matrix additions (`tests/machine/machine_refuse.c`)

- **Prior w:** EXEC and GROM loaded, then a power-on reset. Rows decided before the run phase now run on f, s, c and w: 15 load rows and 2 unload rows.
- **`BAD_STATE` on w:**
  - `machine.refuse.load.{exec,grom,cart}.powered.w`;
  - `machine.refuse.load.exec.after-power-cycle.w`;
  - `machine.refuse.unload.{exec,grom,cart}.powered.w`;
  - `machine.refuse.unload.grom.after-power-cycle.w`.

  In total, `grep -c -E '^machine\.refuse\.(load|unload)\..*\.w$'` prints 25.
- **Reset rows** (one fixed prior each, whole-storage `memcmp` and digest):

  | Row | Prior | Expected |
  |---|---|---|
  | `no-exec` | f, then GROM loaded | `MISSING_SYSTEM_IMAGE` |
  | `no-grom` | f, then EXEC loaded | `MISSING_SYSTEM_IMAGE` |
  | `neither` | f | `MISSING_SYSTEM_IMAGE` |
  | `no-exec-with-cart` | x | `MISSING_SYSTEM_IMAGE` |
  | `kind-0` | s | `BAD_ARG` |
  | `kind-2` | s | `BAD_ARG` |
  | `kind-2-powered` | w | `BAD_ARG` |
  | `null` | s | `BAD_HANDLE` |
  | `destroyed` | s | `BAD_HANDLE` |

- **Precedence rows:**
  - `unsupported-over-bad-state`;
  - `bad-state-over-duplicate`;
  - `reset-arg-over-handle`;
  - `reset-arg-over-missing`;
  - `reset-handle-over-missing`.

### Family iso (5 cases, D-36, no threads)

- **`interleaved-equals-sequential`:** A and B are first run alone for 64 units each. Then a fresh pair, with B created first, is stepped in chunks: B takes 7, 6, …, 1 and A takes 1, 2, …, 7, with B first. The composite and CPU digests at every 8th unit of each instance equal its solo run. The case also checks that A and B reach different CPU states, so it cannot pass vacuously.
- **`poisoned-storage-identical`:** storage poisoned with 0xA5 and with 0x5A, same content, 32 units each; the instances end byte-identical.
- **`memcpy-relocation`:** after 20 units the storage is copied to a new heap buffer. Both copies then step 30 units and end with equal digests and equal bytes.
- **`destroy-mid-run`:** A is destroyed halfway; B follows its solo trace to unit 64.
- **`cross-load`:** B gets one successful load, then `DUPLICATE_LOAD`, `OVERSIZED` and `MALFORMED` loads. A's digest and bytes stay unchanged, and A steps on.

### Family sweep (7 cases)

- **Bases:** `B01-segs-16`, `P01-min`, `P02-two-segments`, `P03-word-patterns`, `P04-adjacent` and `P06-partial`. These are every positive in-span fixture of at most 256 bytes. `machine.sweep.bases-complete` pins the list to the generated table.
- **Mutations per base:** every truncation length 0..len (len itself must load OK), and every change of a byte to 0x00 and to 0xFF.
- **Oracle:**
  - an accepted load must unload back to the fresh digest and the fresh bytes;
  - a refusal must have an IVSC status and leave the instance byte-identical.

  Each base accepted exactly one load, the unmutated one. This is robustness evidence, not accuracy evidence.

### Suite headers (`tests/headers-tap.sh`, 6 ids)

- **Compiles:** `#include "ivs/<h>"` alone, with `${CC:-cc} -std=c17 -pedantic -Wall -Wextra -Werror -fsyntax-only` and, when `${CXX:-c++}` exists, `-std=c++17` with the same warnings.
- **Output:** harness TAP with a nonce header and trailer, copied to `$IVS_TEST_OUT_DIR/headers.txt`. The script exits 0, 1 or 77.
- **ids:** `headers.cxx-*` are `skip-ok`.
- **Checks run:**
  - under dash and through ctest in both presets;
  - with `CXX` pointing at a missing compiler, which gave three visible skips and exit 0;
  - on a scratch copy of the headers with an undeclared `uint32_t`, which failed both cases.

### Named faults (`data/expected/faults/machine.ids`, now 25 rows; 115 in total)

| Fault | Site | Must-fail | Rows failed |
|---|---|---|---|
| `STATE_CHECK_OMITTED` | `machine_load.c` stage 4 | `machine.refuse.load.cart.powered.w` | 6 |
| `RESET_CLEARS_BEFORE_IMAGE_CHECK` | `machine_run.c` | `machine.refuse.reset.no-exec` | 4 |
| `STATIC_CARRY` | `machine_run.c` (file-scope static) | `machine.iso.interleaved-equals-sequential` | 4 |
| `RESET_KEEPS_RAM` | `machine_run.c` | `machine.reset.equivalence` | 1 |
| `OUT_OVERLAP_UNCHECKED` | `machine.c` (`ivs_machine_out_overlaps`) | `machine.refuse.info.out-inside` | 3 |

**Existing faults re-judged** (each widening is for the fault's own rule):

| Fault | Now also allowed | Why |
|---|---|---|
| `INIT_SKIPS_MEMSET` | `machine.reset.`, `machine.iso.`, `machine.refuse.reset.`, `machine.sweep.` | poisoned slot records |
| `DESTROY_NO_SCRUB` | the destroyed-handle reset and step rows | the magic survives destroy |
| `LOAD_LEN_BEFORE_CHECK` | `machine.reset.power-cycle-one-way`, `machine.sweep.` | the slot length is written before the refusal |
| `UNLOAD_KEEPS_IMAGE` | `machine.sweep.` | unload does not restore the fresh state |
| `CHECK_ORDER_MALFORMED_FIRST`, `DUPLICATE_REPLACES` | `machine.iso.cross-load` | that case checks exact statuses |
| `SHA_K_WRONG`, `SHA_ROTATE_WRONG`, `SHA_LENGTH_LE` | `machine.refuse.reset.`, `machine.sweep.` | fixtures with authored trailers are refused |
| `SHA_PAD_BOUNDARY`, `SHA_FINAL_BLOCK_DROPPED` | `machine.sweep.B01-segs-16` | its 184-byte trailer preimage ends 56 bytes into a block |

Every new fault was run, and its failures were read before its allowed prefixes were set.

## Task Commits

1. **Task 1 (tracer): reset, the seam, the stand-in program.** `a2d369e` (test).
   - `--only machine.reset.` passed 2 cases.
   - The first gate run failed only on `INIT_SKIPS_MEMSET` (expected re-judging).
   - After that, sh and dash PASS. Tracer feedback gate: the verify was re-run end to end before expanding.
2. **Task 2: reset refusals, powered prior, isolation.** `e782a5d` (test).
   - `--only machine.iso.` passed 5 and `--only machine.refuse.reset.` passed 9.
   - Release `ctest -R '^machine$'` passed.
   - sh and dash gate PASS after re-judging.
3. **Task 3: sweeps, headers suite, faults.** `d709436` (test).
   - `ctest --preset dev -R '^(machine|headers)$'` passed, and `dash tests/headers-tap.sh "$PWD"` exited 0.
   - sh and dash gate PASS with `faults=115/115 controls=215/215 tests-missing=0`. The summary lines are identical (`cmp`).

## Decisions Made

See `key-decisions` in the frontmatter.

## Deviations from Plan

### Auto-fixed Issues and additions

**1. [Rule 1 - Correctness] Machine time in master ticks, not CPU cycles**
- **Plan text vs D2:** the plan said "add the unit's cycles to machine time" and asked for a check that "machine time equal[s] the sum of unit cycles". `time_ticks` is in master ticks (D2, `docs/time-base.md`, the `ivs_info` comment).
- **Fix:** the registry gains `ticks_per_cpu_cycle`, which `cmake/IvsMemmapTable.cmake` reads from the profile and refuses when zero. It takes the former `reserved` u16 of `ivs_profile_desc`, so the record layout is unchanged.
- **Tests:** the seam adds cycles × ratio, and the tracer checks time = cycles × 4.
- **Commit:** `a2d369e`.

**2. [Count] Faults 115, machine fault rows 25, controls 215**
- **Plan figures:** `faults=113/113`, 23 machine rows and `controls=213/213`. That text predates 03-11's out-parameter fault and the controls of 03-05 and 03-06.
- **Actual:** 110 + 5 = 115 faults; 20 + 5 = 25 machine rows. Controls are unchanged at 215.

**3. [Addition, the 03-11 open item] `OUT_OVERLAP_UNCHECKED`**
- create, info and state_digest now call one helper, `ivs_machine_out_overlaps`, which holds the fault site. The load report keeps its own check and fault.
- **Commit:** `d709436`.

**4. [Scope] More rows than listed**
- **Rows added beyond the plan:**
  - the w prior on the 17 argument and option rows;
  - `machine.refuse.reset.no-exec-with-cart` and `.kind-2-powered`;
  - `machine.refuse.unload.grom.after-power-cycle.w`;
  - three reset precedence rows;
  - `machine.reset.no-cart-ok`, `.power-cycle-one-way` and `.step-refusals` as named cases;
  - `machine.sweep.bases-complete`.
- No planned id was dropped or renamed.

**5. [Structure] Shared stand-in module**
- `tests/machine/machine_standin.c` is not in the plan's file list. The families reset, iso and sweep share it, and it is registered in both the test executable and the machine fault suite.

**6. [Process] TDD order in Task 2**
- **What happened:** reset and the seam landed in the Task 1 tracer, so the Task 2 refusal and isolation cases passed when first run; no RED run was observed.
- **Negative evidence:** each behaviour is guarded by a named fault that turns its row red:
  - `RESET_CLEARS_BEFORE_IMAGE_CHECK` for the reset refusals;
  - `STATE_CHECK_OMITTED` for the powered rows;
  - `STATIC_CARRY` for isolation;
  - `RESET_KEEPS_RAM` for equivalence.

---

**Total deviations:** 6.
- One correctness fix (1).
- One count update (2).
- One addition closing an open item (3).
- Three scope or structure notes (4, 5, 6).

No existing test id, control, step or token changed meaning. Existing fault prefix strings were widened only for their own rules.

## Issues Encountered

- **Comment warning:** `-Wcomment` rejected a `/*` inside a block comment of `machine_standin.c` on the first build; the path was reworded.
- **Fault re-judging:** each new family made existing faults fail as expected; the widenings are in the re-judging table above.

## Open items

- **API-06 stays open (plan 03-13).** The effect table must carry:
  - reset: `BAD_ARG`, `BAD_HANDLE` and `MISSING_SYSTEM_IMAGE`, each with effect none;
  - the A-08 d rule: after the first reset, load and unload are `BAD_STATE` until a new instance;
  - the out-parameter `BAD_ARG` rule for load, info, state_digest and create.
- **Plan 03-13 counts:** its plan text says `faults=113/113` and `controls=213/213`. Expect 115 and 215, plus whatever 03-13 adds.
- **No threaded isolation test, by design (D-36).** API-01's "run concurrently" rests on the five behavioural cases plus the c-purity writable-data rule.
- **`STATIC_CARRY` does not fail `machine.iso.memcpy-relocation`.** The carried cycle count happens to cancel there. The must-fail case and three others do catch it.
- **Stepping cannot read the cartridge.** The stand-in programs never read cartridge words: a 10-bit EXEC cannot hold a `$5000` address without SDBD. Cartridge content enters the isolation cases through the images digest only.
- **`len-wrap` rows** still exist only where `size_t` is 64 bits (unchanged from 03-11).
- **Machine-suite cost:** 0.43 s dev and 0.21 s release, against 03-11's 0.25 s phase-end budget for the suite. Release is inside it; the dev run is not. The refusal matrix at 250 cases is the largest part. Watch it at 03-14.
- **Hosted timing:** the orchestrator reads it from the next hosted run.

## Known Stubs

None. The reset leaves STIC and PSG as the existing stub devices (D-19), which is planned Phase 5+ work, not a stub of this plan.

## Threat Flags

No new surface beyond the register.
- **T-03-41:** five `machine.iso.*` cases, `STATIC_CARRY`, and the c-purity writable rule, which `machine_run.c` passes with no static.
- **T-03-42:** validate before fill, full-oracle reset rows, `RESET_CLEARS_BEFORE_IMAGE_CHECK`.
- **T-03-43:** 25 `.w` rows and `STATE_CHECK_OMITTED`.
- **T-03-44:** sweeps with storage `memcmp`.
- **T-03-45:** the `headers` suite.

The step seam is internal only and has no public declaration.

## User Setup Required

None.

## Next Phase Readiness

- **Plan 03-13:** it can cite `machine.refuse.reset.*`, `machine.prec.reset-*`, the `.w` rows, `machine.reset.power-cycle-one-way` and the out-parameter rows per status.
- **Phase 4:** `ivs_machine_run` can be built on `ivs_machine_step_unit_internal`. Time is already in master ticks.

## Self-Check: PASSED

- **Files found:**
  - `core/src/machine/machine_run.c`
  - `tests/machine/machine_standin.c`
  - `tests/machine/machine_reset.c`
  - `tests/machine/machine_iso.c`
  - `tests/machine/machine_sweep.c`
  - `tests/headers-tap.sh`
  - `data/expected/tests/headers.ids`
- **Commits:** `a2d369e`, `e782a5d` and `d709436` are ancestors of HEAD. The ledger count from `8bbe9f3` is 3, and each message ends with the Co-Authored-By line.
- **Hygiene:**
  - No absolute home path and no "Intellivision" in any added line.
  - No rights-bearing bytes; every image is a generated stand-in or a hand-assembled project program.
  - No code, tables or comments were copied from another emulator or assembler.
  - `.planning/config.json` and `.planning/state.json` are still ` M` and in no commit of this plan.
