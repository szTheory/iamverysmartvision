---
phase: 03-memory-map-content-loading-and-refusal
plan: 02
subsystem: core
tags: [sha256, D-28, A-04, A-07, D-38, suite-machine, named-faults, gate-cost]

requires:
  - phase: 03-memory-map-content-loading-and-refusal
    provides: "plan 03-01: fault loop over every data/expected/faults/*.ids with a per-file suite; ivs_fault_suite() registry in tests/CMakeLists.txt"
provides:
  - "core/src/common/ivs_sha256.{h,c}: streaming SHA-256 from FIPS 180-4; plain 112-byte context with no pointers and no implicit padding, so assignment is a snapshot (A-04)"
  - "data/sha256/vectors.txt + cmake/IvsShaVectors.cmake: literal digests re-derived with CMake string(SHA256) at every regeneration, build fails closed on a mismatch"
  - "suite machine (ivs_tests_machine, ctest name machine, both presets): machine_main.c concatenates case families declared in machine_cases.h; family sha256 with 24 ids"
  - "data/expected/faults/sha256.ids (# suite=machine): SHA_K_WRONG, SHA_ROTATE_WRONG, SHA_LENGTH_LE, SHA_PAD_BOUNDARY, SHA_FINAL_BLOCK_DROPPED, SHA_INIT_NO_RESET"
  - "tools/check/steps/c-faults.sh: fault executables run in at most 4 lanes, findings in listed order (A-07 lever)"
  - ".planning/CI-EFFICIENCY.md: dated Phase 3 measurement 1 with the phase-end fault projection"
affects: [03-07, 03-10, 03-11, 03-12, 03-14, ci-efficiency]

actuals:
  tokens: 9700
  tasks: 3
  commits: 3
plan_head_before: c70f9004c987d0b34f36a7a9b31806ecdf27961a
plan_head_after: 2782039e1dcf57baf4f7615d51298f38ff4eed97

tech-stack:
  added: []
  patterns:
    - "A table-driven case family fits the ivs_test_case interface: the generator emits one static wrapper function per row plus an IVS_<X>_CASES macro, and the family file puts that macro at the head of its exported case table"
    - "A family that reads committed data exports a source list pointer (ivs_machine_<family>_sources) so the suite's result file carries `# source <path> <sha256>` for tests-inventory"
    - "Independent oracle in the build: a cmake -P generator recomputes every literal with a second implementation (string(SHA256)) and stops the build on disagreement"

key-files:
  created:
    - core/src/common/ivs_sha256.h
    - core/src/common/ivs_sha256.c
    - cmake/IvsShaVectors.cmake
    - data/sha256/vectors.txt
    - tests/machine/machine_cases.h
    - tests/machine/machine_main.c
    - tests/machine/machine_sha256.c
    - data/expected/tests/machine.ids
    - data/expected/faults/sha256.ids
  modified:
    - core/CMakeLists.txt
    - tests/CMakeLists.txt
    - tools/check/steps/c-faults.sh
    - .planning/CI-EFFICIENCY.md

key-decisions:
  - "The SHA-256 context carries an explicit `reserved` word, so it is 112 bytes with no implicit padding: a memcmp of two contexts is meaningful and the size equals the documented field sum"
  - "The 1,000,000 x a vector is re-derived with string(SHA256) like every other line (about 15 ms as a CMake string), instead of being checked against the FIPS literal only"
  - "The 'abc' case is table-driven (vectors.txt id fips-abc) with the same test id machine.sha256.fips-abc that the tracer introduced"
  - "New case machine.sha256.init-resets: SHA_INIT_NO_RESET is only visible when a context is re-initialized after taking input"
  - "Lane lever applied on the projection trigger (phase-end c-faults 9.4 s serial > 8 s), not on the measured gate time"
  - "MEM-03 and API-06 are not marked complete: this plan adds the digest primitive and the machine suite only (same practice as 03-01)"

patterns-established:
  - "Later machine families add one extern table pair to machine_cases.h and one ivs_machine_add call in machine_main.c, plus their sources to the ivs_fault_suite(machine ...) registration"

requirements-completed: []

coverage:
  - id: D1
    description: "Core SHA-256 from FIPS 180-4 against independent literal digests (D-28)"
    requirement: MEM-03
    verification:
      - kind: unit
        ref: "build/dev/tests/ivs_tests_machine --only machine.sha256. (24/24 ok); ctest --preset release -R '^machine$' passed"
        status: pass
      - kind: other
        ref: "scratch working-tree copy with pat-0057 literal a70c... -> b70c...: cmake --build --preset dev exit 1, `IvsShaVectors: pat-0057: literal b70c... != string(SHA256) a70c...`"
        status: pass
    human_judgment: false
  - id: D2
    description: "Six named SHA-256 faults through the generalized fault loop (D-28, D-37)"
    requirement: API-06
    verification:
      - kind: integration
        ref: "sh tools/gate.sh and dash tools/gate.sh: gate: PASS steps=24/24 controls=196/196 faults=73/73 fault-runs=73"
        status: pass
    human_judgment: false
  - id: D3
    description: "Gate measurement 1 of Phase 3 recorded and lane lever applied on the projection trigger (D-38, A-07)"
    verification:
      - kind: other
        ref: ".planning/CI-EFFICIENCY.md section 2026-11-11 local measurement 1 of Phase 3"
        status: pass
    human_judgment: false

duration: 15min
completed: 2026-11-11
status: complete
---

# Phase 3 Plan 02: Core SHA-256 and suite machine Summary

**The core now has its own streaming SHA-256 (`core/src/common/ivs_sha256.{h,c}`, written from FIPS 180-4) with a copyable 112-byte context. Suite `machine` checks it against 20 literal digests that `cmake/IvsShaVectors.cmake` re-derives with CMake's `string(SHA256)` whenever the table is generated, plus split, snapshot, reset and context cases. Six named faults in `data/expected/faults/sha256.ids` each turn their own cases red. The gate's fault runs now use 4 lanes, because the phase-end projection crossed the 8 s trigger. Final gate: `gate: PASS steps=24/24 controls=196/196 ... faults=73/73 ... tests-missing=0` under sh and dash.**

## Performance

- **Duration:** about 15 min (2026-11-11T04:56:51Z to about 05:12Z), not counting the reading before the recorded start
- **Tasks:** 3 of 3
- **Files:** 13 in 3 task commits (diff +952 / -14)

## Accomplishments

- **SHA-256 (D-28, A-04).**
  - Written from FIPS 180-4 sections 4.1.2, 4.2.2, 5.1.1, 5.3.3 and 6.2.2 only. No other implementation was read.
  - K and H0 were cross-checked in the scratchpad: each was derived from the first 64 (or 8) primes by integer cube and square roots and compared with the table (`K ok 64`, `H ok 8`).
  - Integer-only. Words are assembled by shifts after widening, rotates are on `uint32_t`, and the length is a 64-bit big-endian bit count.
  - Padding goes through a local 128-byte buffer, compressed as one or two blocks.
  - The context `{h[8], total_bytes, block[64], used, reserved}` is 112 bytes, fixed by a `_Static_assert`.
- **Suite `machine` (D-15, H9).**
  - `machine_main.c` copies every family's table into one bounded static array (512 cases, 8 sources) and runs it as suite `machine`.
  - It is registered in both presets with `SKIP_RETURN_CODE 77` and in `ivs_fault_suite(machine ...)`, which includes the generated include.
- **Vectors and oracle.**
  - `vectors.txt` holds 20 lines: `fips-empty`, `fips-abc`, `fips-448`, `fips-896`, `fips-million-a`, and `pat-0000` through `pat-8192` (lengths 0, 1, 55, 56, 57, 63, 64, 65, 119, 120, 127, 128, 129, 2048, 8192).
  - Every digest was typed in from `cmake -E sha256sum` over scratch files. The generator recomputes each one with `string(SHA256)`.
  - The grammar is strict and fails closed: unique ids, alphanumeric messages, no `;` `\` `[` `]` TAB or CR anywhere in the file.
  - The `file(SHA256)` of `vectors.txt` is embedded, so the TAP file carries `# source data/sha256/vectors.txt <sha256>` and tests-inventory re-hashes it.
- **Cases (24 ids).** One case per vector, plus:
  - `split-every-offset`: two `update` calls at every offset 0..130 equal one call.
  - `ctx-copy-snapshot`: at every k in 0..130, a copy finishes to the digest of the first k bytes and the original then finishes to the full digest.
  - `ctx-plain`: the size equals the field sum and assignment copies every field.
  - `init-resets`: `init` after input, with or without `final`, gives the FIPS "abc" digest.
  - The 1,000,000-byte case takes about 0.07 s in the dev preset (ASan+UBSan) as a whole process. It is fed in 4096-byte chunks from a static buffer.

## Fault behaviour (each fault executable run by hand)

| Fault | Site (`ivs_sha256.c`) | Failing ids | must-fail | must-pass (green) |
|-------|-----------------------|-------------|-----------|-------------------|
| SHA_K_WRONG | bit 8 of K[5] flipped at use | all 20 vectors + init-resets | fips-abc | split-every-offset |
| SHA_ROTATE_WRONG | Sigma1 rotate 11 -> 12 | all 20 vectors + init-resets | fips-448 | ctx-copy-snapshot |
| SHA_LENGTH_LE | length bytes little-endian | every vector except the two empty ones, + init-resets | pat-0001 | fips-empty |
| SHA_PAD_BOUNDARY | spill test `> 56` | fips-448, pat-0056, pat-0120 | pat-0056 | pat-0057 |
| SHA_FINAL_BLOCK_DROPPED | second padding block not compressed | fips-448, pat-0056, pat-0057, pat-0063, pat-0120, pat-0127 | pat-0063 | pat-0055 |
| SHA_INIT_NO_RESET | init keeps `total_bytes` | init-resets | init-resets | fips-abc |

Every failing id is under the allowed prefix `machine.sha256.`. The invariance cases stay green under the digest-wide faults because both sides of each comparison use the same code. That is why they serve as must-pass witnesses.

## Task Commits

1. **Task 1 (tracer): SHA-256 "abc" through suite `machine` and SHA_K_WRONG.** `43bbcb3` (feat). Tracer verified end to end before expansion: `ivs_fault_SHA_K_WRONG` failed `fips-abc` with `ctx-plain` green, and `sh tools/gate.sh` gave `gate: PASS steps=24/24 controls=196/196 ... faults=68/68 ... tests-missing=0` (`gate-time total=29`).
2. **Task 2: vector table, oracle, invariance cases, five faults.** `cd74d2f` (test). `sh tools/gate.sh`: `gate: PASS ... faults=73/73 ... tests-expected=1096 tests-executed=1096 tests-skipped=0 tests-missing=0`.
3. **Task 3: gate measurement 1 and the lane lever.** `2782039` (perf).

**Plan metadata:** recorded in the docs(03-02) completion commit.

## Acceptance evidence

| Check | Result |
|-------|--------|
| `grep -c -F '# suite=machine' data/expected/faults/sha256.ids` | 1 |
| `grep -v '^#' data/expected/faults/sha256.ids \| grep -c .` | 6 |
| `ivs_tests_machine --list \| grep -c '^machine\.sha256\.'` vs `grep -c '^machine\.sha256\.' data/expected/tests/machine.ids` | 24 = 24 |
| `nm -P -u build/release/core/libivs_core.a` | `___stack_chk_fail ___stack_chk_guard _memcpy` (plus the in-archive `_ivs_cp1610_decode`), all allowlisted; `ivs_sha256.c.o` imports nothing |
| Scratch working-tree copy (git clone + tar overlay of `git ls-files -co --exclude-standard`), `pat-0057` literal `a70c...` changed to `b70c...` | `cmake --build --preset dev` exit 1: `IvsShaVectors: pat-0057: literal b70cdc426e03c55f6bab861695327e7c7b032d7da1ed0f1aca0c252125d4e77a != string(SHA256) a70cdc426e03c55f6bab861695327e7c7b032d7da1ed0f1aca0c252125d4e77a` |
| Same copy, `fips-million-a` literal `cdc7...` changed to `ddc7...` | exit 1: `IvsShaVectors: fips-million-a: literal ddc76e5c... != string(SHA256) ...` |
| `grep -c -F 'A-07' .planning/CI-EFFICIENCY.md` | 3 (baseline 0) |

**H12 outcome:** no new import. The release `-O2` archive imports nothing new: the core has no `memcmp`, so there is no `bcmp`, and the zero fills were inlined. `tools/check/core-symbols-allow.txt` is unchanged. Gate `core-imports=3` is the same as before.

## Gate measurement 1 (A-07)

The host was heavily loaded throughout: 1-minute load average 8.9 to 13.1. The trees were warm, so `c-build` is 0. These are local figures, not hosted ones. The full tables are in `.planning/CI-EFFICIENCY.md`.

| Point | `gate-time total=` runs | Median | c-faults | faults-verdict |
|-------|-------------------------|--------|----------|----------------|
| before lever (sh x3) | 20, 22, 26 | **22 s** | 2, 2, 2 | 2, 3, 4 |
| after lever (sh x3) | 20, 19, 21 | **20 s** | 1, 1, 1 | 3, 3, 3 |
| after lever (dash x1) | 22 | n/a | 1 | 3 |

The `c-faults` step on its own (warm, median of 5 under dash) took 1.63 s serial and 0.69 s with 4 lanes.

**Projection (step 1b):**

- Measured: the plain release `ivs_tests_machine` takes 0.0056 s, a machine fault executable 0.0057 s, and a cpu-vectors fault executable 0.019 s.
- Suite `machine` budget is 0.25 s per run (refusal 0.06, sweeps 0.08, isolation 0.05, digest 0.03, everything else 0.03).
- Phase-end fault groups:

| Group | Faults | Per run | Serial |
|-------|--------|---------|--------|
| machine | 29 | 0.25 s | 7.25 s |
| memmap | 11 | 0.06 s | 0.66 s |
| ivsc | 6 | 0.03 s | 0.18 s |
| cpu-vectors | 67 | 0.019 s | 1.29 s |
| **Total** | **113** | | **9.4 s** |

**Lever decision:**

- Trigger 3 (projected phase-end `c-faults` above 8 s) fired, so the lane lever was applied.
- Trigger 1 could not be evaluated, because the load was above 4. It was not met either: the median was 22 s against 29 + 5.
- Trigger 2 was not met: `c-faults` took 2 s against a 5 s limit.

## Decisions Made

See `key-decisions` in the frontmatter.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Fault witness] New case `machine.sha256.init-resets`**
- **Found during:** Task 2, step 4.
- **Issue:** `SHA_INIT_NO_RESET` (init leaves `total_bytes` unchanged) is invisible to any case that starts from a fresh context. The plan's id list (vectors plus `split-every-offset`, `ctx-copy-snapshot`, `ctx-plain`, `fips-abc`) had no case that re-initializes a used context, so the fault could not be detected.
- **Fix:** Added `init-resets`: `init` after input, both without and after `final`, then "abc" must give the FIPS digest. Tests zero every context before the first `init`, and `ivs_sha256_digest` zero-initializes its local, so the fault build never reads an indeterminate field.
- **Committed in:** `cd74d2f`.

**2. [Rule 2 - Stronger oracle] `rep:` line re-derived with string(SHA256) too**
- **Found during:** Task 2, step 2.
- **Issue:** The plan exempted the 1,000,000 x "a" line from the `string(SHA256)` check to avoid a 1 MB CMake string.
- **Fix:** Measured `string(REPEAT "a" 1000000)` plus `string(SHA256)` at about 15 ms (`cmake -P`, 0.014 s total). Every line is now re-derived, and the generator header explains why. The scratch probe on the `fips-million-a` literal fails the build as expected.
- **Committed in:** `cd74d2f`.

**3. [Rule 1 - Plan wording] `SHA_PAD_BOUNDARY` breaks only lengths that are 56 mod 64**
- **Issue:** The plan says the `> 56` test makes "only lengths 56..63 mod 64 break". With `> 56`, lengths 57..63 still spill correctly. Only a remainder of exactly 56 loses its 0x80 byte under the length field. The observed failures were fips-448 (56 bytes), pat-0056 and pat-0120.
- **Fix:** None to the code. The faults row uses must-fail `pat-0056` and must-pass `pat-0057`, which is the distinction the fault exists to show. `SHA_FINAL_BLOCK_DROPPED` covers the whole 56..63 range.

**4. [Rule 2 - Witness strength] SHA_K_WRONG must-pass changed from `ctx-plain` to `split-every-offset` in Task 2**
- **Issue:** Task 1 used `ctx-plain` because no other case could stay green at that point. `ctx-plain` runs no hash code.
- **Fix:** Once `split-every-offset` existed, the row was switched to it. That case runs the full hash path and still stays green under the fault.
- **Committed in:** `cd74d2f`.

**5. [Rule 3 - Interface] Source list exported as a pointer, not an array**
- **Issue:** The generated `ivs_sha_sources[]` is `static` in the `.inc`. A second non-static array cannot be initialized from its elements in C.
- **Fix:** `machine_cases.h` declares `extern const ivs_test_source *const ivs_machine_sha256_sources;`, set to `ivs_sha_sources`.

**6. [Rule 3 - Repo reality] The "abc" case is table-driven in Task 2**
- **Issue:** Task 2 asks for one case per `vectors.txt` id "plus fips-abc". Keeping a hand-written `fips-abc` next to a `vectors.txt` line `fips-abc` would duplicate the id, which the harness rejects.
- **Fix:** "abc" is the `vectors.txt` line `fips-abc`, so the test id stays the same and is now also checked by `string(SHA256)`. The published "abc" literal is still in C, inside `init-resets`.

---

**Total deviations:** 6, all small. No scope creep: no gate token, control or existing inventory id changed. The lane lever was conditional in the plan, and its trigger fired.

## Issues Encountered

- None blocking. The laptop was loaded the whole time, so every timing is load-contaminated.

## Open items

- **Other machine families and the SHA faults (plans 03-07, 03-10).** Every `sha256.ids` row has the allowed prefix `machine.sha256.` only. Once the loader reports digests, a SHA fault will also fail `machine.load.*`, `machine.refuse.*` and `machine.digest.*` cases. The plan that wires SHA-256 into the loader must widen those prefixes (|-separated) in the same commit, or `faults-verdict` will report the extra failures.
- **`faults-verdict` cost.** 2 to 4 s at 73 faults, now the larger of the two fault steps. It grows with the fault count (about 113 by phase end). Re-check it at measurement 2 (plan 03-11).
- **Lane interrupt handling.** On HUP, INT or TERM, `c-faults.sh` kills its lane subshells. A fault executable already running finishes on its own (each takes under 0.03 s today). This is not the full process-tree stop of `run-controls.sh`. Add that only if the heavy families make single fault runs long.
- **`vectors.cmake` reads only `faults/cpu.ids`** (carried over from 03-01). Not touched here.
- **MEM-03 and API-06** stay open. The digest primitive and the machine suite are infrastructure for them.
- **Hosted figures** for this plan's gate are recorded by the orchestrator after the phase PR's CI.

## Known Stubs

None.

## User Setup Required

None.

## Next Phase Readiness

- Plans 03-03 onward can add families to suite `machine`. Each new family needs:
  - its table pair in `machine_cases.h`
  - an `ivs_machine_add` call in `machine_main.c`
  - its sources in both `add_executable(ivs_tests_machine ...)` and `ivs_fault_suite(machine ...)`
  - its ids in `data/expected/tests/machine.ids`
- Plan 03-10 can snapshot a partial hash by assigning the context.
- **Final gate:**
  - `sh tools/gate.sh`: `gate: PASS steps=24/24 controls=196/196 cc=AppleClang c-presets=2 fault-runs=73 core-imports=3 ... faults=73/73 ... tests-expected=1096 tests-executed=1096 tests-skipped=0 tests-missing=0`
  - `dash tools/gate.sh`: the same summary line.

## Self-Check: PASSED

- Files: the nine created files listed in `key-files` were found.
- Commits: `43bbcb3`, `cd74d2f` and `2782039` are ancestors of HEAD. The ledger count from `c70f900` is 3.
- Hygiene: no absolute home path and no "Intellivision" in any new file. `.planning/config.json` and `.planning/state.json` are still ` M` and in no commit of this plan.
