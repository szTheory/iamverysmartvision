---
phase: 03-memory-map-content-loading-and-refusal
plan: 01
subsystem: testing
tags: [named-faults, D-37, H3, gate, controls, c-faults, faults-verdict, cmake]

requires:
  - phase: 02-cp-1610-cpu-core
    provides: "single-file named-fault path (data/expected/faults/cpu.ids, suite cpu-vectors, 67 faults incl. GEN_EXPECT_MUTATION), control runner with _good/tree, gate 24 steps / 192 controls"
provides:
  - "tests/faults.cmake: per-file, per-suite fault executables over every data/expected/faults/*.ids; configure errors for a missing suite header, an unregistered suite and an id listed twice; shared plain object library ivs_fault_suite_obj_<suite> per suite (W7); GEN_ dispatch through a registry"
  - "tests/CMakeLists.txt: registry functions ivs_fault_suite() and ivs_fault_gen(); cpu-vectors and GEN_EXPECT_MUTATION registered"
  - "tools/check/steps/c-faults.sh: loop over every faults file with that file's suite; tokens c-faults-run: no suite in <file>, c-faults-run: duplicate <ID>"
  - "tools/check/steps/faults-verdict.sh: per-file verdict, union site checks (H3), tokens fault-suite-missing and fault-duplicate-id"
  - "_good/tree second toy faults file (second.ids, GOOD_FAULT2, suite toy) and four controls (faults-duplicate-id, faults-suite-missing, faults-second-missing-site, faults-second-not-detected)"
affects: [03-02, 03-03, 03-04, 03-05, 03-06, 03-07, ci-efficiency]

actuals:
  tokens: 7840
  tasks: 2
  commits: 2
plan_head_before: c39c16badb06f211dc99d8cd54251259e714a2ac
plan_head_after: bd94dba5522795e2126969b7ffe5bd9cd9368967

tech-stack:
  added: []
  patterns:
    - "Fault suites are registered once in tests/CMakeLists.txt (ivs_fault_suite / ivs_fault_gen) and read by tests/faults.cmake; the faults .ids files stay pure inventory with a `# suite=` header"
    - "Site faults of one suite link one shared plain (no ivs_san) object library of that suite's test sources; a GEN_ fault recompiles the suite sources with its mutated directory first on the include path"
    - "Fault ids are unique across all faults files; site checks run against the union of non-GEN_ ids"

key-files:
  created:
    - tools/check/controls/_good/tree/data/expected/faults/second.ids
    - tools/check/controls/_good/tree/build/gate/faults/GOOD_FAULT2/toy.txt
    - tools/check/controls/faults-duplicate-id/
    - tools/check/controls/faults-suite-missing/
    - tools/check/controls/faults-second-missing-site/
    - tools/check/controls/faults-second-not-detected/
  modified:
    - tests/faults.cmake
    - tests/CMakeLists.txt
    - tools/check/steps/c-faults.sh
    - tools/check/steps/faults-verdict.sh
    - tools/check/controls/_good/tree/core/src/cpu/good.c
    - tools/check/controls/faults-orphan-site/overlay/core/src/cpu/good.c
    - data/expected/controls/faults-verdict.ids

key-decisions:
  - "Registry functions live in tests/CMakeLists.txt (faults.cmake is included after the registration and only on the dev preset); registration itself is unconditional and cheap"
  - "W7 gate-cost lever applied as planned: one ivs_fault_suite_obj_<suite> object library per suite, so 66 site faults of cpu-vectors share one compile of cpu_vectors.c"
  - "An id repeated inside one file is also fault-duplicate-id / c-faults-run: duplicate (previously hidden by sort -u in c-faults); the repeat is reported and not judged twice"
  - "A faults file with no usable suite still contributes its non-GEN_ ids to the duplicate and union site checks, so a missing header does not cascade into orphan-site findings"
  - "API-06 and MEM-04 are not marked complete: this plan only builds the fault infrastructure they will use (same practice as Phase 2 infrastructure plans)"

patterns-established:
  - "A new Phase 3 faults file needs only: the .ids file with `# suite=<name>`, an ivs_fault_suite(<name> ...) registration (if the suite is new) and one IVS_FAULT_<ID> site per non-GEN_ id"

requirements-completed: []

coverage:
  - id: D1
    description: "Fault build, run and verdict loop over every data/expected/faults/*.ids with a per-file suite (D-37, H3)"
    requirement: API-06
    verification:
      - kind: integration
        ref: "sh tools/gate.sh and dash tools/gate.sh: gate: PASS steps=24/24 faults=67/67 fault-runs=67"
        status: pass
      - kind: integration
        ref: "scratch two-file probe (ADD_OVERFLOW moved to probe.ids, suite cpu-vectors): gate: PASS, -- faults-verdict pass faults=67/67, no orphan or missing-site finding"
        status: pass
      - kind: other
        ref: "scratch configure probes: duplicate id across files, missing suite header and unregistered suite each stop cmake --preset dev with the planned message"
        status: pass
    human_judgment: false
  - id: D2
    description: "Four controls for the multi-file rules on the good tree"
    requirement: API-06
    verification:
      - kind: integration
        ref: "sh tools/check/run-controls.sh: controls=196/196; each new control prints exactly its own FAIL token"
        status: pass
    human_judgment: false

duration: 8min
completed: 2026-11-11
status: complete
---

# Phase 3 Plan 01: Multi-file named-fault loop Summary

**The named-fault path (build in `tests/faults.cmake`, run in `c-faults`, verdict in `faults-verdict`) now works over every `data/expected/faults/*.ids` file. Each file names its suite, suites are registered once in `tests/CMakeLists.txt`, site checks use the union of ids, and a duplicate id or missing suite header fails. The real tree is unchanged in behaviour: `gate: PASS steps=24/24 controls=196/196 ... faults=67/67`.**

## Performance

- **Duration:** about 8 min of execution, timed from the recorded start (reading the plan and context before it is not counted)
- **Started:** 2026-11-11T04:43:57Z
- **Completed:** 2026-11-11T04:51:44Z
- **Tasks:** 2 of 2
- **Files modified:** 22 in 2 task commits (diff +308 / -106)

## Accomplishments

- **Build (`tests/faults.cmake`).**
  - The file globs `data/expected/faults/*.ids` with `CONFIGURE_DEPENDS` and sorts the list.
  - For each file it reads the `# suite=` header. Configure fails with `faults: <file>: no suite header` or `faults: suite <name> is not registered`.
  - A global seen-id list fails configure with `fault <ID>: listed in <file A> and <file B>`.
  - Owner detection is unchanged: exactly one core `.c` per non-GEN_ id.
  - Site faults link the suite's shared object library `ivs_fault_suite_obj_<suite>` (plain flags, never `ivs_san`). GEN_ ids are looked up in the `ivs_fault_gen` registry. The registered suite must match the file's suite.
  - Target names (`ivs_fault_<ID>`), `EXCLUDE_FROM_ALL`, the `ivs_faults` aggregate and `ivs_warnings`-only linking are kept.
- **Registry (`tests/CMakeLists.txt`).** `ivs_fault_suite(<suite> SOURCES INCLUDE_DIRS GENERATED)` and `ivs_fault_gen(<GEN_ID> SUITE OUTPUT GENERATOR ARGS DEPENDS)` are defined here. `cpu-vectors` and `GEN_EXPECT_MUTATION` are registered with the arguments the old hard-coded block passed. `faults.cmake` adds `-DOUT` and `-DMUTATE`.
- **Run (`c-faults.sh`).**
  - The script checks every file's suite header before the build.
  - It runs each id once and writes `<results>/faults/<ID>/<suite>.txt`, where the suite comes from the file that lists the id.
  - A repeated id prints `c-faults-run: duplicate <ID>`. `fault-runs` is summed over files.
  - Each fault executable's stdin is now `/dev/null`, because the loop reads the file list from stdin.
- **Verdict (`faults-verdict.sh`).**
  - Each file is judged against its own `data/expected/tests/<suite>.ids`.
  - A missing header, or a missing suite ids file, gives `fault-suite-missing`. An id listed twice gives `fault-duplicate-id`.
  - The orphan-site and missing-site comparisons use the union of non-GEN_ ids over all files (H3).
  - Existing tokens and the pass line are unchanged. The awk stays POSIX-only and was run under dash.
- **Good tree and controls.**
  - `second.ids` (suite `toy`, `GOOD_FAULT2`) is added with its result file and a second site in `good.c`. Because the suite stays `toy`, the twelve tests-inventory controls are untouched.
  - Four new controls, each failing only for its own token (checked by running the step on each built tree):
    - `faults-duplicate-id` -> `-- FAIL fault-duplicate-id: GOOD_FAULT (data/expected/faults/second.ids)`
    - `faults-suite-missing` -> `-- FAIL fault-suite-missing: data/expected/faults/second.ids (no suite header)`
    - `faults-second-missing-site` -> `-- FAIL fault-missing-site: GOOD_FAULT2` (FORBID `fault-orphan-site:` holds)
    - `faults-second-not-detected` -> `-- FAIL fault-not-detected: GOOD_FAULT2 cpu.ZERO.alpha`

## Task Commits

1. **Task 1 (tracer): the fault loop over every faults file.** `5fe9a84` (feat). Tracer verified end to end before expansion: `sh tools/gate.sh` PASS (`faults=67/67`, `controls=192/192`, `gate-time total=30`) and `dash tools/gate.sh` PASS (`gate-time total=26`); the two-file scratch probe is below.
2. **Task 2: second toy faults file and four controls.** `bd94dba` (test).

**Plan metadata:** recorded in the docs(03-01) completion commit.

## Tracer proof (scratch two-file probe, never committed)

The working-tree copy was built as the plan prescribes (git clone plus a tar overlay of `git ls-files -co --exclude-standard`) in the session scratchpad.

- **Probe 1, as planned (`RESET_ENTRY` moved to `probe.ids`, `# suite=cpu-vectors`).** The fault loop passed:
  - `-- c-faults pass fault-runs=67`
  - `-- faults-verdict pass faults=67/67`
  - no `fault-orphan-site` and no `fault-missing-site` line

  The gate still ended `gate: FAIL steps=23/24` because of a different step:
  - `-- FAIL vector-policy-no-fault: POL-RST-01 (no fault in data/expected/faults/cpu.ids has a must-fail vector naming it)`
  - Cause: `tools/check/vectors.cmake` (lines 1269-1310) reads `data/expected/faults/cpu.ids` by path, and RESET_ENTRY is the only fault witness for policy POL-RST-01. 03-PATTERNS.md records that check as CPU-only with no change needed. See Deviations 1 and Open items.
- **Probe 2 (`ADD_OVERFLOW` moved instead; it is not the only witness of any policy).** Full gate green:
  - `-- c-faults pass fault-runs=67`
  - `-- faults-verdict pass faults=67/67`
  - `gate: PASS steps=24/24 controls=192/192 ... faults=67/67 ...`
  - no orphan-site or missing-site finding
- **Configure-time refusals (same scratch copy, `cmake --preset dev`).**
  - `fault ADD_OVERFLOW: listed in data/expected/faults/cpu.ids and data/expected/faults/probe.ids`
  - `faults: data/expected/faults/probe.ids: no suite header`
  - `faults: suite nosuch is not registered (data/expected/faults/probe.ids; see ivs_fault_suite in tests/CMakeLists.txt)`
- **Producer token.** Running `c-faults.sh` directly on a suite-less file printed `-- FAIL c-faults-run: no suite in data/expected/faults/zz.ids` and exited 1.

## Acceptance counts

| Check | Baseline (HEAD c39c16b) | After |
|-------|-------------------------|-------|
| `grep -c -F 'faults/*.ids'` in c-faults.sh / faults-verdict.sh / faults.cmake | 0 / 0 / 0 | 3 / 3 / 3 |
| `grep -c 'ivs_fault_suite(cpu-vectors' tests/CMakeLists.txt` | 0 | 1 |
| `grep -c 'ivs_fault_suite_obj_' tests/faults.cmake` | 0 | 7 |
| `grep -c -F 'fault-duplicate-id' tools/check/steps/faults-verdict.sh` | 0 | 3 |
| `grep -c -F 'fault-suite-missing' tools/check/steps/faults-verdict.sh` | 0 | 4 |
| `grep -c -F 'GOOD_FAULT2' .../_good/tree/data/expected/faults/second.ids` | n/a | 1 |
| `run-controls.sh` new controls ok (`grep -c -E 'control ok +(faults-duplicate-id\|...)$'`) | n/a | 4 |
| `run-controls.sh` last line | `controls=192/192` | `controls=196/196` |
| `git status --porcelain .planning/config.json .planning/state.json` | both ` M` | both ` M`, in no commit of this plan |

## Gate timing (first Phase 3 data point)

The host was loaded throughout (load average 14.1 at the start of the final runs, 16.9 at the end).

| Run | Result | `gate-time total=` |
|-----|--------|--------------------|
| `sh tools/gate.sh` after Task 1 | PASS, controls=192/192 | 30 |
| `dash tools/gate.sh` after Task 1 | PASS, controls=192/192 | 26 |
| `sh tools/gate.sh` after Task 2 | PASS, controls=196/196 | 32 |
| `dash tools/gate.sh` after Task 2 | PASS, controls=196/196 | 27 |

Step times after Task 1: `c-faults` 2 s and `faults-verdict` 4 s, the same as the Phase 2 figures (3 s and 4 s). The planning baseline was `gate-time total=29`. Plan 03-02 does the budget measurement.

## Uncontrolled-token exceptions

- `c-faults-run: duplicate` is producer-side (it needs a build). It is shown by the configure-time error `fault <ID>: listed in <file A> and <file B>` (which stops the build before c-faults reaches its loop) and covered by the verdict-side control `faults-duplicate-id`.
- `c-faults-run: no suite` is producer-side (the step builds). It is shown by the configure-time error `faults: <file>: no suite header` and by the direct run quoted above, and covered by the verdict-side control `faults-suite-missing`.

## Files Created/Modified

- `tests/faults.cmake`: multi-file, per-suite fault executable generation, shared suite objects, GEN_ dispatch via the registry.
- `tests/CMakeLists.txt`: `ivs_fault_suite`, `ivs_fault_gen` and the cpu-vectors / GEN_EXPECT_MUTATION registrations.
- `tools/check/steps/c-faults.sh`, `tools/check/steps/faults-verdict.sh`: the multi-file loop, new tokens and header comments.
- `tools/check/controls/_good/tree/{data/expected/faults/second.ids, build/gate/faults/GOOD_FAULT2/toy.txt, core/src/cpu/good.c}`: the second toy faults file, its result and site.
- `tools/check/controls/{faults-duplicate-id, faults-suite-missing, faults-second-missing-site, faults-second-not-detected}/`: new controls.
- `tools/check/controls/faults-orphan-site/overlay/core/src/cpu/good.c`: keeps the GOOD_FAULT2 site.
- `data/expected/controls/faults-verdict.ids`: four new names under `# Multi-file fault loop controls (plan 03-01, D-37).`

## Decisions Made

See `key-decisions` in the frontmatter. In short:

- Registry in `tests/CMakeLists.txt`, shared suite objects (W7).
- Duplicates are refused within a file as well as across files.
- A suite-less file still feeds the union site check.
- API-06 and MEM-04 stay open until the plans that implement them.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Repo reality] Tracer probe moved ADD_OVERFLOW, not RESET_ENTRY, to get a green full gate**
- **Found during:** Task 1, step 5.
- **Issue:** Moving `RESET_ENTRY` out of `cpu.ids` fails `cp1610-vectors` (`vector-policy-no-fault: POL-RST-01`). `tools/check/vectors.cmake` reads only `data/expected/faults/cpu.ids` for the policy-has-a-fault rule, and RESET_ENTRY is POL-RST-01's only fault witness. The fault loop itself passed in that probe (`faults=67/67`, no orphan or missing-site finding).
- **Fix:** No code change. The probe was repeated with `ADD_OVERFLOW`, which is not the only witness of any policy: full `gate: PASS`, `faults=67/67`. Both probes are quoted above. `vectors.cmake` was not generalized, because 03-PATTERNS.md settles it as CPU-only and the coupling only matters if a CPU fault ever leaves `cpu.ids`. It is recorded as an open item.
- **Files modified:** none (scratch only).

**2. [Rule 2 - Control sharpness] `faults-orphan-site` overlay gains the GOOD_FAULT2 site**
- **Found during:** Task 2.
- **Issue:** The overlay replaces `good.c`. Without the new site the control would also print `fault-missing-site: GOOD_FAULT2`. That does not collide with a FORBID line (the control has none), so the plan did not require a change, but the control would fail for two reasons instead of one.
- **Fix:** The GOOD_FAULT2 site (three lines) was added to the overlay copy, so the control prints only `fault-orphan-site: IVS_FAULT_ORPHAN core/src/cpu/good.c`. The `core-purity-*` overlays of `good.c` run `c-purity`, which does not check fault sites, so they need no change.
- **Files modified:** `tools/check/controls/faults-orphan-site/overlay/core/src/cpu/good.c`.
- **Committed in:** `bd94dba`.

**3. [Rule 1 - Robustness] stdin of fault executables is `/dev/null`**
- **Found during:** Task 1.
- **Issue:** The new `while read` loop over the file list would hand its stdin to each fault executable.
- **Fix:** Each fault executable now runs with `< /dev/null`.
- **Committed in:** `5fe9a84`.

---

**Total deviations:** 3 (1 probe adjustment forced by repo reality, 1 control sharpening, 1 robustness fix).
**Impact on plan:** No scope creep. No core source, vector, inventory id or existing token changed. The planned truths hold: the RESET_ENTRY probe still showed `faults=67/67` with no site findings, and the full-gate PASS was shown with ADD_OVERFLOW.

## Issues Encountered

- None beyond Deviation 1.

## Open items

- **`vectors.cmake` reads only `faults/cpu.ids` (policy-has-a-fault rule, lines 1269-1310).** A CPU fault moved to another faults file would fail `vector-policy-no-fault` if it was a policy's only witness. Phase 3 adds new faults in new files and moves none, so nothing breaks now. If a later plan reorganizes CPU faults, generalize the rule to the union of the `# suite=cpu-vectors` files. That needs a matching change in the good tree, whose `cpu.ids` uses suite `toy`.
- **Core-consumed GEN_ path.** Only the test-consumed GEN_ path exists. Plan 03-04 adds the core-consumed form when the first core TU includes a generated table, as planned.
- **Gate budget.** Measured in plan 03-02 (A-07). The local figures above were taken on a loaded host.

## User Setup Required

None.

## Next Phase Readiness

- Plan 03-02 can add `data/expected/faults/sha256.ids` with `# suite=machine` and an `ivs_fault_suite(machine ...)` registration; no gate script needs to change.
- Final gate after Task 2: `sh tools/gate.sh` and `dash tools/gate.sh` both `gate: PASS steps=24/24 controls=196/196 ... faults=67/67 ... tests-expected=1048 tests-executed=1048 tests-skipped=0 tests-missing=0`.

## Self-Check: PASSED

- Files: `tests/faults.cmake`, `tests/CMakeLists.txt`, both step scripts, `second.ids`, `GOOD_FAULT2/toy.txt` and the four control directories were found.
- Commits: `5fe9a84` and `bd94dba` are ancestors of HEAD, and the ledger count from `c39c16b` is 2.
- Task 1 acceptance: see the counts table; the scratch probe's `-- faults-verdict pass faults=67/67` line is quoted, and no orphan or missing-site finding appeared.
- Task 2 acceptance: 4 new controls ok, `controls=196/196`, and `.planning/config.json` and `.planning/state.json` are still ` M` and in no commit.
