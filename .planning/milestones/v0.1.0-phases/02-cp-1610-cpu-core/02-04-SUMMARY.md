---
phase: 02-cp-1610-cpu-core
plan: 04
subsystem: cpu-core
tags: [cmake, cp1610, named-faults, vector-checker, gate, controls]

requires:
  - phase: 02-cp-1610-cpu-core
    plan: 03
    provides: cpu-vectors, cpu-api and harness suites; c-build, c-tests and tests-inventory steps; TAP harness and vector generator
provides:
  - tests/faults.cmake (targets ivs_fault_<ID>, aggregate ivs_faults) driven by data/expected/faults/cpu.ids
  - generator option MUTATE (cmake/IvsVectorTable.cmake)
  - faults RESET_ENTRY (site in cp1610.c reset entry) and GEN_EXPECT_MUTATION
  - gate steps c-faults, faults-verdict, cp1610-vectors; 26 controls (9 faults-*, 17 vectors-*)
  - data/expected/cp1610-policies.ids (19 ids), `# suite=` header in cp1610-vectors.ids, tools/check/vector-emulator-names.txt
affects: [02-05, 02-06, 02-07, 02-08, 02-13, 02-14, 02-16, 02-18]

actuals:
  tokens: 27400
  tasks: 2
  commits: 2

tech-stack:
  added: []
  patterns:
    - "A fault executable is the owning TU recompiled with IVS_FAULT_<ID> plus plain object copies of the other core TUs and the harness, linked with ivs_warnings only (never ivs_san)"
    - "c-faults runs binaries and writes results; faults-verdict is a pure checker over build/gate/faults and data/expected, so text overlays drive its controls"
    - "vectors.cmake caps findings at 20 per token and then prints one line with the exact total under the same token, so no uncontrolled cap token exists"

key-files:
  created:
    - tests/faults.cmake
    - tools/check/steps/c-faults.sh
    - tools/check/steps/faults-verdict.sh
    - tools/check/steps/cp1610-vectors.sh
    - tools/check/vectors.cmake
    - tools/check/vector-emulator-names.txt
    - data/expected/faults/cpu.ids
    - data/expected/cp1610-policies.ids
    - data/expected/controls/faults-verdict.ids
    - data/expected/controls/vectors.ids
    - tools/check/controls/faults-*/ (9 controls)
    - tools/check/controls/vectors-*/ (17 controls)
    - tools/check/controls/_good/tree/core/src/common/ivs_prelude.h
    - tools/check/controls/_good/tree/core/src/cpu/good.c
    - tools/check/controls/_good/tree/data/expected/faults/cpu.ids
    - tools/check/controls/_good/tree/build/gate/faults/GOOD_FAULT/toy.txt
    - tools/check/controls/_good/tree/data/expected/cp1610-vectors.ids
    - tools/check/controls/_good/tree/data/expected/cp1610-policies.ids
    - tools/check/controls/_good/tree/data/expected/cp1610-skips.ids
  modified:
    - core/src/cpu/cp1610.c
    - cmake/IvsVectorTable.cmake
    - tests/CMakeLists.txt
    - tools/check/steps/c-build.sh
    - data/expected/gate-steps/cpu.ids
    - data/expected/cp1610-vectors.ids
    - tools/check/controls/_good/tree/data/cp1610/vectors/toy.json
    - tools/check/controls/_good/tree/build/gate/{dev,release}/toy.txt
    - tools/check/controls/tests-inventory-*/overlay/build/gate/dev/toy.txt (6 files, digest only)

key-decisions:
  - "faults.cmake reads IVS_CORE_SOURCES from the core directory scope (get_directory_property DEFINITION) and adds the core sources to CMAKE_CONFIGURE_DEPENDS, so moving a fault site to another TU reconfigures; core/CMakeLists.txt is unchanged"
  - "vector-program-mismatch compares the binary part with the whole hex word (the generator's rule), not only its low 10 bits, so a 16-bit data word written with all 16 bits stays valid in both places"
  - "vector-row-mismatch accepts only a leaf row for the decle (a matching row no other matching row refines); with several leaves (the $0BF TSTR/JR coincidence) each leaf is accepted, MOVR is not"
  - "Malformed lines and bad ids in faults/cpu.ids map onto existing tokens (fault-unknown-vector, fault-missing-site) and a missing or duplicate vector inventory onto vector-id-*/vector-duplicate-id, so no token exists without a control"
  - "mame is added only to tools/check/vector-emulator-names.txt (vectors.cmake), not to the shared denylist-cite.txt: six in-scope lines carry the name today"

patterns-established:
  - "Every new fault lands as one inventory line plus one #ifdef site of at most 3 lines; faults-verdict then demands detection for the stated reason with no collateral"

requirements-completed: []
requirements-advanced: [RUN-03, CPU-01, CPU-04]

coverage:
  - id: D1
    description: "Fault RESET_ENTRY (reset entry keeps only the low byte of the IAB reply) fails exactly cpu.UNIT-RESET.provider-ffff; provider-0000 and every other vector pass"
    requirement: CPU-04
    verification:
      - kind: other
        ref: "build/gate/faults/RESET_ENTRY/cpu-vectors.txt: 1 not ok (cpu.UNIT-RESET.provider-ffff); faults-verdict faults=2/2"
        status: pass
    human_judgment: false
  - id: D2
    description: "GEN_EXPECT_MUTATION builds the table with cpu.CLRC.clears-c expect.cycles + 1 and fails exactly that vector, so expectations travel JSON -> generator -> harness"
    requirement: CPU-01
    verification:
      - kind: other
        ref: "build/gate/faults/GEN_EXPECT_MUTATION/cpu-vectors.txt: 1 not ok (cpu.CLRC.clears-c)"
        status: pass
    human_judgment: false
  - id: D3
    description: "faults-verdict shows each of its nine tokens through its own control; site set equality between core/src and the fault inventory"
    requirement: RUN-03
    verification:
      - kind: other
        ref: "sh tools/check/run-controls.sh: 9 lines 'control ok   faults-*', 'control ok   _good/faults-verdict'"
        status: pass
    human_judgment: false
  - id: D4
    description: "cp1610-vectors checks file set, keys, kinds, cites, sources, rows (most-specific label, A-13), programs, emulator names, ids, suite and skips; each of 17 tokens has one control"
    requirement: RUN-03
    verification:
      - kind: other
        ref: "gate: '-- cp1610-vectors pass vectors=7 source=4 policy=3'; 17 lines 'control ok   vectors-*'; per-token EXPECT grep prints 1 seventeen times"
        status: pass
    human_judgment: false

duration: 15min
completed: 2026-11-10
status: complete
plan_head_before: 54a7aa320c3a20eb3aa48f975cfc6d72c5f78010
plan_head_after: 7797d20a72f4b3c5a89b4c3f2ab7391eff19420b
commits: 2
---

# Phase 2 Plan 04: named-fault path and structural vector checker Summary

**A deliberately broken reset entry and a mutated JSON expectation are now built as plain (non-sanitized) fault executables, run by `c-faults` and judged by the pure `faults-verdict` step. The vector data is checked structurally by `cp1610-vectors`. Every one of the 26 new failure tokens is shown by its own control.**

## Performance

- **Duration:** about 15 min (from the plan-02-03 metadata commit at 00:59 to the Task 2 commit at 01:14 local time; plan reading came before that)
- **Completed:** 2026-11-10
- **Tasks:** 2 (1 tracer, 1 auto)
- **Files:** 114 changed in the two task commits (2660 insertions, 12 deletions; most of them are small control files)

## Accomplishments

- `tests/faults.cmake` (included when `IVS_FAULT_CONTROLS`, dev preset only):
  - It reads `data/expected/faults/cpu.ids` at configure time.
  - For each site fault it requires exactly one core TU holding `IVS_FAULT_<ID>`, else `FATAL_ERROR`.
  - It builds `ivs_fault_<ID>` from that TU compiled with the define, plain object copies of the other core TUs and of the harness, and the normal vector table.
  - Fault executables link `ivs_warnings` only. The built binary carries no ASan symbols (`nm | grep -ci asan` = 0).
  - Targets are EXCLUDE_FROM_ALL, collected in `ivs_faults`, and never added to ctest.
  - `GEN_EXPECT_MUTATION` runs the generator with `-DMUTATE=<must-fail id>` into `build/dev/tests/fault-GEN_EXPECT_MUTATION/ivs_vectors.inc`.
- `cmake/IvsVectorTable.cmake`: `MUTATE` adds 1 to the merged `expect.cycles` of one vector. The build fails if the id is unknown or the vector asserts no cycles.
- `core/src/cpu/cp1610.c`: one 3-line site after the IAB load in the reset entry. `#ifdef IVS_FAULT_RESET_ENTRY` keeps only the low byte of R7.
- `c-build.sh` also builds `ivs_faults` in the dev tree. `c-faults.sh` builds `ivs_faults`, then runs each fault with `IVS_TEST_OUT_DIR=build/gate/faults/<ID>` and the gate nonce. A non-zero exit is expected. A missing binary or result gives `c-faults-run`.
- `faults-verdict.sh` (pure) emits the nine tokens and the site check, prints every finding sorted, and passes with `faults=<ok>/<total>`.
- `tools/check/vectors.cmake` and `steps/cp1610-vectors.sh`: the 17 structural tokens, a cap of 20 findings per token followed by an exact-total line, and stats `vectors= source= policy=`. The emulator-name lists are read from `CHECK_DIR` only.
- Data:
  - `cp1610-policies.ids` holds the 19 policy ids.
  - `cp1610-vectors.ids` gains `# suite=cpu-vectors`.
  - `faults/cpu.ids` holds `GEN_EXPECT_MUTATION` and `RESET_ENTRY`.
  - `gate-steps/cpu.ids` gains `c-faults`, `faults-verdict` and `cp1610-vectors`.

## Task Commits

1. Task 1 (tracer): 1f4a0f4 - `feat(02-04): named-fault path with RESET_ENTRY and the generator mutation control`
2. Task 2: 7797d20 - `feat(02-04): cp1610-vectors structural checker with 17 controls`

Tracer feedback gate: after Task 1 the tracer `<verify>` ran end to end before Task 2 started. It gave `gate: PASS steps=22/22 controls=137/137 ... fault-runs=2 ... faults=2/2`, and `run-controls.sh` ended with `controls=137/137`.

## A-08 split

A-08 puts the fault path with one fault into the single walking-skeleton plan. As plan 02-02 recorded (INFO 14), that skeleton is spread over plans 02-02, 02-03 and 02-04. The fault path with its first real fault (`RESET_ENTRY`) and the generator-path control (`GEN_EXPECT_MUTATION`) land here, in plan 02-04, not in plan 02-02, together with the structural vector data checker. Each of the three plans ends with a green gate. From here on, every plan that adds behaviour lands its own faults through this machinery.

## Uncontrolled-token exceptions

- `c-faults-run`: environment failure (a fault binary that cannot be built or run); proven by the real gate's `c-faults` step, and an overlay control would need a broken real build.

## Verification results

- `sh tools/gate.sh` (unpiped, final run): `gate: PASS steps=23/23 controls=154/154 cc=AppleClang c-presets=2 fault-runs=2 ctest-runs=2 ... vectors=7 source=4 policy=3 ... faults=2/2 ... tests-expected=54 tests-executed=54 tests-skipped=0 tests-missing=0 ...`. Wall time 31.0 s (`9.71s user 19.00s system 92% cpu 31.044 total`). The step line was `-- cp1610-vectors pass vectors=7 source=4 policy=3`.
- `dash tools/gate.sh`: `gate: PASS steps=23/23 controls=154/154 ...`. `c-faults`, `faults-verdict` and `cp1610-vectors` were each also run directly under `/bin/dash`, and each printed its pass line.
- `run-controls.sh`: `controls=154/154`, with 9 `control ok   faults-*`, 17 `control ok   vectors-*`, `control ok   _good/faults-verdict` and `control ok   _good/cp1610-vectors`. The faults and vectors controls were also run one by one: each prints its own token, and `faults-collateral` and `faults-missing-site` print only theirs (FORBID guards the neighbour token).
- Task 1 acceptance:
  - `grep -c '^not ok'` is 1 for `RESET_ENTRY` (`cpu.UNIT-RESET.provider-ffff`) and 1 for `GEN_EXPECT_MUTATION` (`cpu.CLRC.clears-c`).
  - `grep -c IVS_FAULT_ core/src/cpu/cp1610.c` = 1, and the site body is 3 lines.
  - `grep -c IVS_FAULT_ build/release/build.ninja` = 0.
- Task 2 acceptance:
  - 19 `POL-` lines.
  - `mame` and `jzintv` appear once each in `vector-emulator-names.txt`, and `denylist-cite.txt` has 0 `mame` lines.
  - The overlay `toy.json` mentions MAME, and EXPECT carries `cpu.ZERO.alpha derivation`.
  - The per-token loop prints `1` seventeen times.
- Throwaway probe of the A-13 rule, on a scratch copy of the real data (nothing committed):
  - `$2B0` labelled MVI@ fails (`labelled by: PULR`). PULR passes.
  - `$0BF` labelled JR or TSTR passes. Labelled MOVR, it fails.
  - `$1C0` labelled SETC fails (`labelled by: CLRR`).
  - An SDBD-prefixed `$0001, $1C0` labelled CLRR passes.

## Decisions Made

See `key-decisions` in the frontmatter. One point matters for plan 02-05: a MOVR vector cannot use `$0BF` (SSS = DDD = 7). That decle belongs to the TSTR/JR coincidence, so the R7-destination cycle bonus of MOVR must be shown with SSS != 7.

## Open item: repo-wide provenance list (owner: plan 02-18 notes)

The measurement was re-run on 2026-11-10. The command `git ls-files | grep -v -E '^(\.planning|\.claude|docs/provenance|tools/check)/|^docs/provenance\.md$|^data/sources\.json$' | xargs grep -n -i -a -F mame | grep -c -v -F '[cite-only]'` prints **6**. The six lines are in `data/discrepancies.json`, `data/expected/sources.ids` and `data/profiles/intv-2609-ntsc-family-p1.json`. The `jzintv` name has 0 in-scope matches.

Adding `mame` to the shared `denylist-cite.txt` would turn the repo-wide `denylist` step red. The `[cite-only]` marker cannot waive these lines, and `sources.ids` cannot carry the marker on its id line. So the name is only in `tools/check/vector-emulator-names.txt`, which `vectors.cmake` alone reads. Whether and how to widen the shared list (marker form for JSON and ids files, or a waiver scope) stays open for plan 02-18's notes.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug, rule conflict] Program binary compared with the whole hex word**
- **Found during:** Task 2
- **Issue:** The plan says the binary part must equal the low 10 bits of the hex. The generator, which already gates the build, requires the binary to equal the whole hex word. A 16-bit data word written with 16 binary digits would pass the generator and fail the low-10-bit rule.
- **Fix:** `vector-program-mismatch` uses the generator's rule (binary value = hex value). For every decle (hex <= $3FF) this is the same as the low-10-bit rule. The row label still uses the low 10 bits of the first decle.
- **Files modified:** tools/check/vectors.cmake
- **Commit:** 7797d20

**2. [Rule 3 - Blocking] Digest refresh in existing tests-inventory and faults overlays**
- **Found during:** Task 2
- **Issue:** The plan's toy.json changes (cite `pdf 1`, an inference-basis `bus` record on alpha) change its sha256. Six `tests-inventory-*` overlays and the faults overlays carry that digest. Without a refresh they would also fire `tests-vectors-digest` and stop being single-defect controls.
- **Fix:** Replaced the old digest with the new one in every fixture that carried it (16 files). `tests-inventory-stale-digest` keeps its deliberately wrong digest.
- **Commit:** 7797d20

**3. [Scope note] faults-wrong-reason also shows fault-collateral**
- As the plan specifies, the control makes `cpu.ZERO.beta` (the must-pass vector) fail, so `fault-collateral` fires as well. The control asserts only `fault-wrong-reason`. The reverse case is pinned: `faults-collateral` FORBIDs `fault-wrong-reason`.

**Total deviations:** 2 auto-fixed (Rule 1, Rule 3) and 1 scope note. **Impact:** none on scope. Every token has exactly one control.

## Authentication Gates

None.

## Known Stubs

None added. The good-tree `core/src/cpu/good.c` is a control fixture by design. Plan 02-02's scheduled stubs (HALTED unit, temporary UNIMPLEMENTED fault, lines not acted on) are unchanged.

## Threat Flags

None. All new surface is in the plan's threat model:
- T-02-09: plain fault executables, NONCE, list, prefix, must-pass and unknown-vector checks, with 9 controls.
- T-02-10: compile-time sites, 0 matches in the release build.ninja, site set equality.
- T-02-12: emulator-name check reading its lists from `CHECK_DIR`.
- T-02-46: id, file and suite set equality plus the skip check, with 17 controls.

## Issues Encountered

- **Gate time:** the local wall time is 31.0 s, above the D-22 local budget of 25 s. The extra cost is about 2.5 s from the 26 new controls plus the fault builds and runs. The plan expects this: plan 02-16 measures the gate against D-22 and applies the levers (background C build, parallel control runner).

## Next Phase Readiness

Plan 02-05 can add coverage derivation to `vectors.cmake`. The row leaves (`decle_leaves`) and the per-token cap are in place, and its tokens join the same `VTOKENS` total line. New faults need one `cpu.ids` line and one site. RUN-03, CPU-01 and CPU-04 are advanced but stay Pending, because later plans of this phase complete them.

## Self-Check: PASSED
