---
phase: 02-cp-1610-cpu-core
verified: 2026-11-10T21:55:00Z
status: passed
score: 13/13 must-haves verified (5 roadmap SCs + 8 requirements)
covered_files: [".planning/phases/02-cp-1610-cpu-core/02-01-PLAN.md", ".planning/phases/02-cp-1610-cpu-core/02-01-SUMMARY.md", ".planning/phases/02-cp-1610-cpu-core/02-02-PLAN.md", ".planning/phases/02-cp-1610-cpu-core/02-02-SUMMARY.md", ".planning/phases/02-cp-1610-cpu-core/02-03-PLAN.md", ".planning/phases/02-cp-1610-cpu-core/02-03-SUMMARY.md", ".planning/phases/02-cp-1610-cpu-core/02-04-PLAN.md", ".planning/phases/02-cp-1610-cpu-core/02-04-SUMMARY.md", ".planning/phases/02-cp-1610-cpu-core/02-05-PLAN.md", ".planning/phases/02-cp-1610-cpu-core/02-05-SUMMARY.md", ".planning/phases/02-cp-1610-cpu-core/02-06-PLAN.md", ".planning/phases/02-cp-1610-cpu-core/02-06-SUMMARY.md", ".planning/phases/02-cp-1610-cpu-core/02-07-PLAN.md", ".planning/phases/02-cp-1610-cpu-core/02-07-SUMMARY.md", ".planning/phases/02-cp-1610-cpu-core/02-08-PLAN.md", ".planning/phases/02-cp-1610-cpu-core/02-08-SUMMARY.md", ".planning/phases/02-cp-1610-cpu-core/02-09-PLAN.md", ".planning/phases/02-cp-1610-cpu-core/02-09-SUMMARY.md", ".planning/phases/02-cp-1610-cpu-core/02-10-PLAN.md", ".planning/phases/02-cp-1610-cpu-core/02-10-SUMMARY.md", ".planning/phases/02-cp-1610-cpu-core/02-11-PLAN.md", ".planning/phases/02-cp-1610-cpu-core/02-11-SUMMARY.md", ".planning/phases/02-cp-1610-cpu-core/02-12-PLAN.md", ".planning/phases/02-cp-1610-cpu-core/02-12-SUMMARY.md", ".planning/phases/02-cp-1610-cpu-core/02-13-PLAN.md", ".planning/phases/02-cp-1610-cpu-core/02-13-SUMMARY.md", ".planning/phases/02-cp-1610-cpu-core/02-14-PLAN.md", ".planning/phases/02-cp-1610-cpu-core/02-14-SUMMARY.md", ".planning/phases/02-cp-1610-cpu-core/02-15-PLAN.md", ".planning/phases/02-cp-1610-cpu-core/02-15-SUMMARY.md", ".planning/phases/02-cp-1610-cpu-core/02-16-PLAN.md", ".planning/phases/02-cp-1610-cpu-core/02-16-SUMMARY.md", ".planning/phases/02-cp-1610-cpu-core/02-17-PLAN.md", ".planning/phases/02-cp-1610-cpu-core/02-17-SUMMARY.md", ".planning/phases/02-cp-1610-cpu-core/02-18-PLAN.md", ".planning/phases/02-cp-1610-cpu-core/02-18-SUMMARY.md", "CMakeLists.txt", "CMakePresets.json", "cmake/IvsSweepTable.cmake", "cmake/IvsVectorTable.cmake", "cmake/IvsWarnings.cmake", "core/CMakeLists.txt", "core/src/common/ivs_prelude.h", "core/src/common/ivs_status.h", "core/src/cpu/cp1610.c", "core/src/cpu/cp1610.h", "core/src/cpu/cp1610_decode.c", "data/cp1610/vector-review.json", "data/cp1610/vectors/branch.json", "data/cp1610/vectors/busrq-halt.json", "data/cp1610/vectors/control.json", "data/cp1610/vectors/direct.json", "data/cp1610/vectors/encoding.json", "data/cp1610/vectors/immediate.json", "data/cp1610/vectors/indirect.json", "data/cp1610/vectors/interrupt.json", "data/cp1610/vectors/jump.json", "data/cp1610/vectors/register.json", "data/cp1610/vectors/sdbd.json", "data/cp1610/vectors/shift.json", "data/expected/cp1610-vectors.ids", "data/expected/faults/cpu.ids", "data/expected/tests/core-symbols.ids", "data/expected/tests/cpu-api.ids", "data/expected/tests/cpu-fuzz.ids", "data/expected/tests/cpu-sweep.ids", "data/expected/tests/cpu-vectors.ids", "data/expected/tests/harness.ids", "tests/CMakeLists.txt", "tests/core-symbols-tap.sh", "tests/cpu/cpu_api.c", "tests/cpu/cpu_fuzz.c", "tests/cpu/cpu_sweep.c", "tests/cpu/cpu_vectors.c", "tests/faults.cmake", "tests/harness/harness_selftest.c", "tests/harness/ivs_test.c", "tests/harness/ivs_test.h", "tests/harness/ivs_testbus.c", "tests/harness/ivs_testbus.h", "tests/harness/ivs_vec.h", "tools/check/core-symbols-allow.txt", "tools/check/core-symbols-deny.txt", "tools/check/core-symbols.sh", "tools/check/run-controls.sh", "tools/check/steps/c-build.sh", "tools/check/steps/c-faults.sh", "tools/check/steps/c-purity.sh", "tools/check/steps/c-tests.sh", "tools/check/steps/cp1610-vectors.sh", "tools/check/steps/faults-verdict.sh", "tools/check/steps/tests-inventory.sh", "tools/check/vectors.cmake", "tools/gate.sh"]
covered_digest: "v4:sha256:66f6df9424af42ce479bd12592b275259c8946d831b2a49261c2559d6bf5303e"
behavior_unverified: 0
overrides_applied: 0
decision_coverage:
  honored: 25
  total: 25
  not_honored: []
human_verification: []
---

# Phase 2: CP-1610 CPU Core — Verification Report

**Phase goal:** Every opcode in the inventory produces results that match hand-derived, page-cited expectations. The CP-1610 runs inside a host-independent core library, checked by an exact test inventory and one documented local gate command.
**Verified:** 2026-11-10T21:55:00Z, branch `gsd/phase-2-cp-1610-cpu-core` at `220b59e` (working tree: only `.planning/config.json` and `.planning/state.json` modified, both outside the gate's evidence)
**Status:** passed
**Re-verification:** No. This is the first verification.

## Method

These checks were run in this verification, not taken from SUMMARY.md:

1. **Gate run.** `sh tools/gate.sh`, exit 0, 31 s at load ~11-19. Final line:
   `gate: PASS steps=24/24 controls=192/192 cc=AppleClang c-presets=2 fault-runs=67 core-imports=2 purity-selftest=5/5 ctest-runs=2 ... vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 ... discrepancies=59 pending-resolvers=18 open=56 faults=67/67 ... tests-expected=1048 tests-executed=1048 tests-skipped=0 tests-missing=0`.
   Every one of the 192 control lines reads `control ok`.
2. **Built suites run directly** from `build/dev/tests/`, with a nonce supplied by the verifier: cpu-vectors `executed=412 failed=0`, cpu-api `24/0`, cpu-sweep `75/0`, cpu-fuzz `8/0`, harness `4/0`. 524 tests per preset × 2 presets = 1048, which matches the gate's figure.
3. **Three named-fault binaries run by hand.** Each failed only vectors inside its registered prefix and included its must-fail vector:
   - `ivs_fault_PUSH_DIRECTION` failed 4 vectors, all `cpu.PSHR.*`, including `cpu.PSHR.write-then-increment`.
   - `ivs_fault_PREV_INTERRUPTIBLE_IGNORED` failed `cpu.CLRC.defers-intr`, `cpu.DIS.defers-intr` and other vectors in its prefix.
   - `ivs_fault_SDBD_INTERRUPTIBLE` failed exactly `cpu.MVII.sdbd-then-intr-deferred`.
4. **Purity checker run on independent inputs.** A scratch object that calls `clock_gettime` was rejected: `-- FAIL core-symbols-forbidden: clock_gettime (clock)`, exit 1. A fresh `-O2` archive of the real core passed: `external=2 defined=6`, exit 0.
5. **Vector JSON tallied with a script.** 412 vectors: 291 `kind=source` (all `tier=source-expectation`) and 121 `kind=policy` (no tier). Every vector has a cite and a derivation. All 74 table rows plus 5 `UNIT-*` pseudo-rows have vectors. Every row has at least one vector that asserts cycles, at least one bus vector and at least one source vector. 274 bus vectors are figure-based (their cycle stamps are asserted) and 47 are inference-based (order, kind, address and data are asserted, but not the stamps).
6. **Fault sites read in the code.** `core/src/cpu/cp1610.c` holds 66 `#ifdef IVS_FAULT_*` sites, which matches the 66 non-`GEN_` ids in `data/expected/faults/cpu.ids`. The sites read are real mutations, for example `IVS_FAULT_RESET_ENTRY` masks R7 and `IVS_FAULT_BUSRQ_PRIORITY` flips the order between grant and accept.
7. **GSD tools.** `verify.artifacts` passed for all 18 plans. `verify.key-links` passed for all of them except one literal-pattern miss, covered below. `check.decision-coverage-verify` returned 25/25 decisions honoured.

## MVP-mode note

ROADMAP.md marks Phase 2 `**Mode:** mvp`, but its goal is not written as a user story ("As a ..., I want ..., so that ..."). Under MVP mode the verifier guard would normally refuse to verify. Phase 1 had the same mismatch and set the precedent followed here: the caller supplied explicit success criteria, and the phase is an infrastructure / core-library phase with no user flow, so verification ran against SC1-SC5 and the 8 requirement IDs. No User Flow Coverage table is produced. To make this consistent, run `/gsd-mvp-phase 2` to restate the goal, or drop the `Mode` line.

## Goal Achievement

### Observable Truths (Roadmap Success Criteria)

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| SC1 | The documented local command runs the full opcode-level inventory and reports executed, skipped and missing counts. A deliberate omission control shows that a missing, duplicated, skipped or stale required test makes it fail. | ✓ VERIFIED | The README documents `sh tools/gate.sh`, and `.github/workflows/gate.yml` runs the same command (`ci-parity pass ci-uses=1`). Gate stat: `tests-inventory pass tests-expected=1048 tests-executed=1048 tests-skipped=0 tests-missing=0`. These controls passed in the gate run: `tests-inventory-missing` (FAIL tests-missing), `-duplicate`, `-unexpected-skip`, `-stale-nonce` (FAIL tests-stale), `-stale-digest`, `-unlisted`, `-truncated`, `-plan-mismatch`, `-failed`, `-suite-missing`, `-empty` and `-ctest-drift`, 12 in all. `vectors-review-stale` also passed; it covers a stale second-read marker. |
| SC2 | Each opcode vector checks register, memory, flag and cycle results against hand-derived values that cite a manual page (tier: source expectation). A deliberately broken opcode (named-fault control) fails its vectors. | ✓ VERIFIED | `tests/cpu/cpu_vectors.c` compares the full 32-byte state (all eight registers, status, intff, dbd and the rest), the whole 64K memory image including which words are initialised, total cycles, unit kinds, events and the bus sequence. Any field missing from `expect` must stay unchanged, and a read of poison memory fails the vector. All 412 vectors carry a `cite` (`pdf N`) and a non-empty `derivation`. Each of the 74 rows has a vector that asserts cycles. Gate stats: `faults-verdict pass faults=67/67` and `c-faults pass fault-runs=67`. Controls `faults-not-detected`, `faults-wrong-reason`, `faults-collateral` and `GEN_EXPECT_MUTATION` passed; the last one checks the path from the JSON through the generator to the harness. The verifier also ran 3 fault binaries itself (Method 3). Caveat: by design, the source-kind vectors for the 7/9 branch rows assert no cycles (A-05). Branch cycles are carried by POL-BR-01 policy vectors, not source vectors, and the `vectors-cycles-basis` check enforces this. |
| SC3 | Interrupt and BUSRQ acceptance cases (previous-instruction interruptibility, I flag, D/SDBD, BUSRQ), SDBD modes (with a documented policy for unsupported modes), stack direction, auto-increment registers and reset entry each pass their vectors, and each has a named-fault control that fails for the intended reason. | ✓ VERIFIED | Vector files `interrupt.json` (39 vectors), `busrq-halt.json` (22), `sdbd.json` (27), `indirect.json` (35) and `control.json` (27) all pass. Named faults, per case:<br>- Interrupts: `PREV_INTERRUPTIBLE_IGNORED`, `INTFF_IGNORED`, `SDBD_INTERRUPTIBLE`, `BUSRQ_PRIORITY`, `GRANT_LOSES_INTERRUPTIBLE`, `GRANT_AFTER_INT_ENTRY`, `INT_ENTRY_ORDER` and `LINES_LATCHED`.<br>- SDBD: `SDBD_IMMEDIATE_PC`, `SDBD_INDIRECT_INCREMENT`, `SDBD_MODE13_ONE_READ`, `SDBD_DBD_NOT_CLEARED` and `SDBD_TARGET_ACCEPTED` (refusal policy POL-SDBD-02).<br>- Stack: `PUSH_DIRECTION` and `PULL_DIRECTION`.<br>- Auto-increment: `AUTOINC_R4R5_MISSING` and `AUTOINC_R1R3`.<br>- Reset: `RESET_ENTRY`.<br>"Intended reason" is enforced by the allowed-prefix and must-pass columns in `faults/cpu.ids`, and the `faults-wrong-reason` and `faults-collateral` controls show that check is live. All 19 POL-* ids have a vector (`policies-pending=0`), and `vectors-policy-no-fault` enforces a fault per policy. C17 is resolved: `c17-consistency pass c17=resolved-primary`. |
| SC4 | For any instruction, the stepper reports an ordered, cycle-stamped list of bus accesses that a test compares against the manual's bus sequence. | ✓ VERIFIED | `ivs_cp1610_step(..., ivs_cp1610_access *log, ...)` writes 12-byte records `{offset_cpu_cycles, addr, data, wait, kind, flags}`. 321 vectors carry an expected `bus` list, which is compared in order on kind, address and data. For the 274 `bus_basis=figure` vectors the cycle stamps are compared too (`bus-figure-pages=35`). Every row has a bus vector (`vector-bus-uncovered` is enforced). Caveat: for the MVO direct row only inference-based bus vectors exist (C56): stamps are not asserted, but order, kind, address and data are. cpu-api adds `bus-order-matches-log` and `wait-shifts-offsets`. |
| SC5 | A build/link or symbol check fails when the core library references host clocks, filesystem, environment, threads or I/O. A deliberate-violation control demonstrates this. | ✓ VERIFIED | `tools/check/core-symbols.sh` checks symbols against an allowlist: only mem*, the `_chk` variants, the stack protector and bzero are allowed. `c-purity` compiles the core fresh at -O0 and -O2, and a source layer checks includes, asm and floating point (`c-purity pass core-imports=2 purity-selftest=5/5`). The CTest suite `core-symbols` checks the artifact the build ships. The 15 `core-purity-*` controls all passed, among them clock (`FAIL core-symbols-forbidden: clock_gettime`), env (`getenv`), fs, thread (`pthread_create`), io, rand, alloc, header, asm, float and hollow. The verifier also reproduced the check independently (Method 4). |

**Score:** 5/5 success criteria verified (0 present-but-behavior-unverified)

### Requirements Coverage

| Requirement | Source Plan(s) | Description | Status | Evidence |
|-------------|----------------|-------------|--------|----------|
| CPU-01 | 02-05, 02-06, 02-08..02-13 | Every opcode in an opcode-level inventory executes with hand-derived expected register, memory, flag and cycle results, each vector citing its source page | ✓ SATISFIED | 74/74 rows covered (`cp1610-rows=74 rows-pending=0`). 412 vectors, every one with a cite. A 1024-decle sweep runs against the Phase 1 family table (`cpu-sweep` 75 cases; `patterns=1024 unclaimed=0`). Flag set/clear/preserve coverage is enforced; there are 5 documented impossible-case exemptions in `cp1610-flag-exempt.ids`, each with a page reason. |
| CPU-02 | 02-14, 02-15 | Interrupt and bus-request acceptance follows an explicit state model, with vectors and named-fault controls for each acceptance case | ✓ SATISFIED | State fields `prev_interruptible`, `intff`, `dbd` and `busak` (`cp1610.h`). Functions `ivs_boundary_open`, `ivs_int_accept` and `ivs_unit_grant` in `cp1610.c`. Interrupt and BUSRQ faults as listed under SC3. |
| CPU-03 | 02-13 | SDBD per source for supported modes; unsupported modes follow a named, documented, tested policy | ✓ SATISFIED | POL-SDBD-01/02/03. `UNSUPPORTED_SDBD_TARGET` is a sticky fault. 12 refusal targets in `sdbd.json`, 5 SDBD faults, and the sweep checks the DBD behaviour of every decle. |
| CPU-04 | 02-10, 02-06 (reset) | Stack direction, auto-increment and reset entry are verified by vectors from the primary manual (resolving C17) | ✓ SATISFIED | POL-STK-01/02 and POL-RST-01 have vectors in `indirect.json` and `UNIT-RESET.*`. Faults `PUSH_DIRECTION`, `PULL_DIRECTION`, `AUTOINC_*`, `STK_RESULT_LOSES` and `RESET_ENTRY`. Gate: `c17=resolved-primary`. |
| CPU-05 | 02-02, 02-15 | The stepper is pure and reports cycle-stamped bus accesses | ✓ SATISFIED | The step reads (state, cfg, lines, port replies), works on a copy and writes it back. The core has no mutable statics; the only static is the `const` decode table. cpu-api tests `deterministic-repeat`, `two-instances-interleaved`, `null-log-equivalent`, `bus-order-matches-log` and `wait-shifts-offsets` pass. The fuzz suite (8 seeds, ASan+UBSan in dev) checks that every record falls inside its unit. |
| API-05 | 02-07 | No dependency on host clocks, filesystems, environment, threads, controller discovery or presentation, enforced by a symbol check | ✓ SATISFIED | See SC5. Allowlist, not denylist (D-21). `core-imports=2`. |
| PKG-02 | 02-01, 02-02, 02-18 | One documented local command runs the same checks as the fast PR gate | ✓ SATISFIED | README line 27 `sh tools/gate.sh`. The workflow runs exactly `sh tools/gate.sh`, and `ci-parity` enforces this every run. |
| RUN-03 | 02-02, 02-04 | Exact test-inventory gate fails on missing, duplicated, unexpectedly skipped or stale tests and reports executed/skipped/missing counts | ✓ SATISFIED | See SC1. The `tests-inventory` step plus the 12 `tests-inventory-*` controls. ctest `-N` drift is checked as well. |

No orphaned requirements: REQUIREMENTS.md maps exactly these 8 IDs to Phase 2.

### Required Artifacts

`verify.artifacts` reports every plan's artifacts as passed (02-01: 4/4, 02-02: 5/5, 02-16: 5/5, and all others pass). The key artifacts are substantive and wired:

| Artifact | Status | Details |
|----------|--------|---------|
| `core/src/cpu/cp1610.c` (1416 lines) | ✓ VERIFIED | Complete stepper with ALU, addressing modes, branch, jump, shift, SDBD, refusal, interrupt, grant and halt units. No TBD, FIXME, TODO or placeholder. Linked into `ivs_core` and every test executable. |
| `core/src/cpu/cp1610_decode.c` | ✓ VERIFIED | 74-row const table. The sweep checks it against `opcode-families.json`. |
| `tests/cpu/cpu_vectors.c` + `cmake/IvsVectorTable.cmake` | ✓ VERIFIED | The generator turns JSON into C. It never captures output from the core, so expected values are not circular. |
| `data/cp1610/vectors/*.json` (12 files) | ✓ VERIFIED | 412 vectors, each file pinned by a second-read sha256 (`second-read=12/12`). |
| `tools/check/steps/{c-build,c-tests,c-faults,c-purity,tests-inventory,faults-verdict,cp1610-vectors}.sh` | ✓ VERIFIED | All are listed in `data/expected/gate-steps/*.ids` and all ran in this gate (`steps=24/24`). |

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| `tools/gate.sh` | `build/gate/NONCE` | NONCE contract | ✓ WIRED | `verify.key-links` reported a literal-pattern miss. The code writes `$IVS_GATE_DIR/NONCE` with `IVS_GATE_DIR="$ROOT/build/gate"` (gate.sh, results-directory block). The harness echoes the nonce, and the `tests-inventory-stale-nonce` control proves the check is live. |
| All other plan key links (21) | | | ✓ WIRED | `verify.key-links` exit 0. |
| JSON vectors → generator → harness | | `GEN_EXPECT_MUTATION` fault | ✓ WIRED | The fault mutates one `expect.cycles` and the vector fails. |
| core archive → purity check | | `c-purity` + ctest `core-symbols` | ✓ WIRED | `core-imports=2`. |

### Data-Flow Trace (Level 4)

Not applicable: this phase renders no UI data. The analogous flow, from JSON expectations through the C table to the comparison, is shown live by `GEN_EXPECT_MUTATION` and the `vectors-*` controls.

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|----------|---------|--------|--------|
| The gate is green end to end | `sh tools/gate.sh` | `gate: PASS steps=24/24 controls=192/192`, exit 0 | ✓ PASS |
| Clean vector suite | `build/dev/tests/ivs_tests_cpu_vectors` | `executed=412 skipped=0 failed=0` | ✓ PASS |
| Stack-direction fault is detected | `build/dev/tests/ivs_fault_PUSH_DIRECTION` | 4 failures, all `cpu.PSHR.*` | ✓ PASS |
| Interruptibility fault is detected | `build/dev/tests/ivs_fault_PREV_INTERRUPTIBLE_IGNORED` | the must-fail vector `cpu.CLRC.defers-intr` fails | ✓ PASS |
| SDBD deferral fault is detected | `build/dev/tests/ivs_fault_SDBD_INTERRUPTIBLE` | exactly 1 failure, `cpu.MVII.sdbd-then-intr-deferred` | ✓ PASS |
| Purity check rejects a clock call | `core-symbols.sh bad.a` (calls `clock_gettime`) | `FAIL core-symbols-forbidden: clock_gettime (clock)`, exit 1 | ✓ PASS |
| Purity check accepts the real core | `core-symbols.sh core.a` (-O2) | `pass external=2 defined=6` | ✓ PASS |

### Probe Execution

No probe scripts are declared or present (`scripts/*/tests/probe-*.sh`: none). Step 7c is skipped.

### Test Quality Audit

| Test file | Linked req | Active | Skipped | Circular | Assertion level | Verdict |
|-----------|-----------|--------|---------|----------|-----------------|---------|
| `tests/cpu/cpu_vectors.c` | CPU-01..04 | 412 | 0 | No (hand-derived JSON, blind second read) | Behavioral: full state, memory, cycles and bus | OK |
| `tests/cpu/cpu_api.c` | CPU-05, CPU-02 | 24 | 0 | No | Value / behavioral | OK |
| `tests/cpu/cpu_sweep.c` | CPU-01, CPU-03 | 75 | 0 | No (checked against the Phase 1 table) | Value | OK |
| `tests/cpu/cpu_fuzz.c` | CPU-05 | 8 | 0 | No (checks invariants) | Invariant | OK |
| ctest `core-symbols` | API-05 | 1 | 0 | No | Status, with an allowlist | OK |

Disabled tests on requirements: 0. Circular patterns: 0. The expected-value provenance is PARTIAL by nature: the vectors are hand-derived from the same pages by the same model family, and that is labelled `source-expectation, second-read`, never `independent-implementation-comparison` (`docs/cp1610/second-reader-protocol.md:11`; C43 open).

### Claim-Wording Check

`independent-implementation-comparison` appears only in the tier definitions (`data/tiers.json`, `docs/evidence-tiers.md`) and in sentences that disclaim it (`second-reader-protocol.md:11`, CONTEXT D-15, 02-16-PLAN). All 291 source vectors carry `tier: source-expectation`. The 121 policy vectors carry no tier, which the `vectors-policy-with-tier` control enforces. `vector-review.json` records the reviewer as `isolated-subagent-blind-v1` and its method as a "mechanical diff; page re-read for every disagreement". The wording complies with D-15.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| (several step scripts) | — | `XXXXXX` in `mktemp` templates | ℹ️ Info | Not a debt marker |
| — | — | No TBD/FIXME/TODO/placeholder in `core/`, `tests/` or the C steps | — | — |

### Decision Coverage

All 25 trackable CONTEXT.md decisions are honoured by the shipped artifacts (`check.decision-coverage-verify`: honored 25/25, not_honored []).

### Human Verification Required

N/A. This is an infrastructure / core-library phase with no user-facing elements. Every acceptance criterion is verifiable programmatically, and no truth is ⚠️ PRESENT_BEHAVIOR_UNVERIFIED (each behavior-dependent truth has passing vectors and named faults).

### Known and Accepted Caveats (do not fail the phase)

1. **412 vectors against D-16's estimate of ~300-360.** It was an estimate, not a cap. The overrun comes from the A-13 exact branch coverage (taken and not-taken, forward and backward) and the review additions.
2. **All 30 reader disagreements were adjudicated by the same executor** (`review-disagreements=30`). C43 stays open, and an independent re-adjudication is recommended (02-18-SUMMARY).
3. **WR-03 is only partly done.** The data-access wait-fault vectors and the named fault `IVS_FAULT_WAIT_FAULT_CONTINUES` are deferred, and so is the WR-05 citation of C59 from the `cpu.SWAP.n1` derivation in `shift.json`. Both wait for the next blind re-read round, because the vector files are pinned by the second-read sha256. The CR-01 / WR-03 behaviour itself is covered by cpu-api cases `wait-fault-no-write`, `-no-sdbd-high-read` and `-no-ebci`, plus fuzz invariants.
4. **The `JE_AFTER_BOUNDARY` fault is an approximation.** It treats DE=01 as 00 because the 32-byte state has no field for a pending enable (02-14 deviation 3).
5. **59 discrepancies remain open or partial (56 open, 18 pending resolvers).** Among them are C43 (shared misreading), C29 / POL-BR-01 (branch cycles carried by policy), C56 (MVO direct bus stamps are inference) and C59 (SWAP N=1). Each one is recorded as `unknown-default-chosen` with a resolver.
6. **Stale totals in prose.** STATE, plan and 02-SECURITY.md prose quotes earlier totals: controls=190 (now 192), tests 1040 (now 1048), discrepancies 59/55 open (now 59/56). The live gate figures above are authoritative.

### Informational Findings (non-blocking, not in the caller's caveat list)

- **The security audit predates the review fixes.** `02-SECURITY.md` audited commit `1b325b9`, and the 11 review-fix commits (`831d94a`..`02dd9ef`, including core changes for CR-01 and WR-01) came after it. Those fixes are hardening, and the gate passes on them. If audit-commit precision matters, re-run `/gsd-secure-phase 2` or add a note.
- **Gate budget.** The last recorded hosted gate time was `gate-time total=31` s against D-22's 30 s budget; CI-EFFICIENCY.md records this as within noise, with the item still open. The local 25 s budget is still unjudged because no idle host has been available; this run took 31 s at load ~11-19.
- **The MVP-mode goal is not a user story** (see the MVP-mode note above).
- **The ROADMAP checkbox for Phase 2 is still `[ ]`.** That is the orchestrator's update to make after verification.

### Gaps Summary

None. Every success criterion and requirement is backed by executed tests, vectors and live negative controls in a gate run made during this verification (`gate: PASS steps=24/24 controls=192/192 faults=67/67 tests-executed=1048 tests-missing=0`). Nothing is stubbed. The claim wording stays at "source-expectation, second-read". The caveats above are recorded, open items, not goal failures.

---

_Verified: 2026-11-10T21:55:00Z_
_Verifier: Claude (gsd-verifier)_
