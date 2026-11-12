---
phase: 04-headless-runner-and-installed-consumer
plan: 16
subsystem: fixtures, tests/run, tools/pack, data
tags: [diagnostic, original-diagnostic, hand-derivation, slicing, RUN-06, RUN-05, D-27, D-31]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-14: boot stub contract 1 (J $5000, R6 = 0340), tools/pack (--ivsc, --c-include), ivs_images; 04-15: listing step, listings.ids, suite pack; 04-09: the slicing oracle of family slice; 04-06: ivs_machine_cpu_state and ivs_machine_peek"
provides:
  - "fixtures/FX-diag-cpu-1: the 56-word hand-assembled listing of ivs-diag-cpu-1 (5000-5037) and its manifest"
  - "data/golden/diag-cpu-1.expect: tier original-diagnostic, oracle independent-derivation, the hand derivation with page cites, 18 assert lines in the runner grammar (halted, R0-R6, ten words)"
  - "build/<preset>/images/diag-cpu-1.ivsc (176 bytes, never tracked) and its bytes in ivs_images.inc"
  - "tests/expect-inc.cmake: assert lines of a .expect file to an X-macro C include (fails the build on any line it cannot turn into a check)"
  - "run.diag.predicate, run.diag.slice (D-07 on the real program), run.diag.repeat; ivs_run_slice_halting exported from run_slice.c"
  - "suite pack: pack.words-diag-cpu-1 and pack.trailer-diag-cpu-1"
  - "claim CL-0008 (provisional), discrepancy C68"
affects: [04-17, 04-20, 04-21, 04-23, 04-25]

actuals:
  tokens: 10800
  tasks: 3
  commits: 4
plan_head_before: 5fb2884712d95dab7ef342a8e9356bbaa7d248a2
plan_head_after: e8b6330

tech-stack:
  added: []
  patterns:
    - "Derivation before execution: the predicate file is committed (90f9609) before the program first runs on the emulator, and the test reads its values from that file through a generated include, so the oracle cannot be retyped from an observed run"

key-files:
  created:
    - fixtures/FX-diag-cpu-1/diag-cpu-1.listing.txt
    - fixtures/FX-diag-cpu-1/manifest.json
    - data/golden/diag-cpu-1.expect
    - tests/expect-inc.cmake
    - tests/run/run_diag.c
  modified:
    - tools/pack/CMakeLists.txt
    - tests/CMakeLists.txt
    - tests/pack-tap.sh
    - tests/run/run_slice.c
    - tests/run/run_cases.h
    - tests/run/run_main.c
    - data/expected/faults/run.ids
    - docs/diagnostics.md
    - data/claims.json
    - data/discrepancies.json

key-decisions:
  - "The derivation commit came first: listing, manifest and .expect landed in 90f9609 before any emulator run of the program; the plan's Task 1/Task 2 split (values typed in the test until Task 2) was skipped because the file already existed"
  - "expect-inc.cmake accepts only `assert halted == yes|no`, `assert reg R<n> == HHHH` and `assert mem AAAA == HHHH`; any other body line stops the build, and run.diag.predicate also checks the clause shape (1 halted, 7 reg, 10 mem)"
  - "run.diag.slice reuses family slice's oracle through ivs_run_slice_halting(powered) instead of a copy; the five patterns run in one case"
  - "run.diag.repeat uses two different storage poison bytes (3C, C3), so equal digests also show no poison byte reaches a digest"
  - "RUN-06 and RUN-05 stay Pending: RUN-06 now has its API-level evidence, and plans 04-20 and 04-25 also list it (runner path, closure); RUN-05's goldens are plan 04-21"

requirements-completed: []

duration: about 15 min of edits and gates (first commit 17:57, last 18:05), plus the container gate
completed: 2026-11-11
---

# Phase 4 Plan 16: ivs-diag-cpu-1, the first original diagnostic, reaches its hand-derived predicate Summary

**A 56-word project-authored diagnostic, entered from a power-on reset through the project boot stub and driven only through the public API, halts with R0-R6 and ten memory words equal to values derived by hand from 16-bit arithmetic and the GI manual's tables; the derivation was committed before the program first ran, and the result holds under the five slicing patterns of D-07.**

## Order of work (derivation first)

1. `90f9609`: listing, manifest and `data/golden/diag-cpu-1.expect` with the full derivation, committed after a green gate whose only use of the listing was the two text parsers of suite `pack`; the emulator had not run the program.
2. `dcdb57e`: first emulator run. `run.diag.predicate` passed on the first run: every one of the 18 clauses matched the hand derivation; no mismatch to investigate. Observed (not asserted): the stop came at 2084 ticks after 70 units, which is the hand-summed 520 CPU cycles to the entry into the halted state plus the 1-cycle HALTED unit, as the DISCUSS-diag note predicted.

## Derivation notes

- Every word was re-derived by hand from pdf 53-57 (page images in the local `refs/` cache). No word differs from the DISCUSS-diag section 5 draft; the 56 words, the BNZE displacement 0003 (5020 + ~0003 = 501C) and the predicate all agree with it.
- Flag rules: `§3.1.2, pdf 45` (C carry out, OV two's complement overflow, Z, S); status-change columns of pdf 53 and 57 (INCR, DECR, CLRR: S Z only; ADDR, ADDI: all four; MVO, MVII, MVI@, PSHR, PULR, GSWD: none); GSWD layout pdf 56; stack direction C17 (pdf 62, 46, 48); MVI@ post-increment of R4 (pdf 55).
- Cycle hand-sum 5 + 33 + 134 + 14 + 208 + 126 = 520, recorded as regression information only.
- Cartridge image identity (build tree, both presets): `cmake -E sha256sum build/dev/images/diag-cpu-1.ivsc` = `243626a75f7592fcb904a6c2cba2de307f19f08eb484ae9f94fd0383526ee5c0`, 176 bytes (24 header + 8 segment + 112 words + 32 trailer). Identity only, not rights or accuracy.
- libasm cross-check: **not run, tool absent** (`which libasm` finds nothing; nothing installed).

## Performance

- Plain-run times (wall, 3 runs each, load average 11-15): `ivs_tests_run --only run.diag.` dev 0.08-0.10 s, release 0.01-0.02 s (dev per case: predicate 0.08, slice 0.11, repeat 0.08, mostly process start); whole suite `run` dev 0.48-0.51 s, release 0.27-0.28 s; suite `pack` (`sh tests/pack-tap.sh`) dev 0.40-0.41 s, release 0.19-0.20 s (was 0.25 / 0.14 with one listing; the second listing adds one more cmake parse, one dump and the trailer case).
- Gate totals: derivation commit 23 s wall, Task 1 `gate-time total=25`, Task 2 `24`, final `31` (sh) and `29` (dash).

## Counts

Start: `steps=30/30 controls=263/263 faults=140/140 tests-expected=2462 claims=7 discrepancies=67 listings=1 listing-words=22`.
End: `steps=30/30 controls=263/263 fault-runs=140 faults=140/140 tests-expected=2472 tests-executed=2472 tests-skipped=0 tests-missing=0 claims=8 provisional=7 discrepancies=68 open=63 listings=2 listing-words=78 manifests=4 claim-refs=7` (+10 tests: pack.words-diag-cpu-1, pack.trailer-diag-cpu-1 and run.diag.{predicate,slice,repeat}, each in both presets). Next discrepancy id **C69**, next claim id **CL-0009**.

Acceptance checks: listing word lines 56; `grep -c '^assert '` = 18; `grep -c 'pdf 56'` = 3; `grep -c '"id": "CL-0008"' data/claims.json` = 1; `ctest -R '^pack$'` passes; `-- listing pass listings=2 listing-words=78`.

## Deviations from Plan

**1. [Order] Derivation committed before the tracer** (user instruction): the `.expect` file landed in Task 1's first commit, so the tracer read values from the generated include from its first run and never carried typed literals. Commits `90f9609`, `dcdb57e`.

**2. [Rule 3 - Blocking] `data/expected/faults/run.ids` (not in the plan's file list):** `run.diag.slice` also detects RUN_STOP_BEFORE_OVERSHOOT, RUN_BUDGET_CARRY and INPUT_STATIC_CARRY, which faults-verdict reported as `fault-wrong-reason`. `run.diag.` / `run.diag.slice` joined their allowed prefixes with a comment. INPUT_STATIC_CARRY is seen only because the input cases run before family diag, so the carried words differ between the replay and its reference; `run.slice.halt.fixed-7` (before the input family) still passes under it, as its must-pass column requires. Commit `b2305e9`.

**3. [Refactor] `tests/run/run_slice.c` (not in the plan's file list)** exports `ivs_run_slice_halting(powered)`: `rs_reference` and `rs_case` take a program descriptor (powered function, fault flag) instead of a program number; the eleven existing cases are unchanged and still pass. Commit `b2305e9`.

**4. `run.diag.slice` is one case running the five patterns** (the plan names one id); a failing pattern is named in the diagnostic. The edges pattern is not repeated on the diagnostic (it tests the run bound, not the program).

## Open Items

- Plan 04-17: the IVSS assert grammar must accept exactly the 18 lines of `data/golden/diag-cpu-1.expect`; the scenario `diag-boot-pass` is named in its header but not yet written.
- Plan 04-20: mutants through `ivs_pack --patch` (listed in docs/diagnostics.md); 04-21: the `.digests` golden that pins the container identity hash (the manifest note refers to it); 04-23: the consumer re-runs the program from `build/<preset>/images/diag-cpu-1.ivsc`.
- RUN-06 and RUN-05 stay Pending (see key decisions). Carried: C14 and C63-C68 revisited in Phase 5/6/7; `API-02`, `PKG-03`, `PKG-04`, RUN-01, RUN-04 Pending; `pending-resolvers=17`, `unknown-fields=16`; `sh tools/hygiene.sh --history` still fails on earlier planning commits (this plan's commits add no home paths).
- If a later diagnostic needs SDBD, the listing step's operand rule must learn the prefix first (04-15).

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=30/30 controls=263/263 ... cc=AppleClang c-presets=2 fault-runs=140 ... claims=8 provisional=7 ... discrepancies=68 pending-resolvers=17 open=63 faults=140/140 ... listings=2 listing-words=78 manifests=4 ... tests-expected=2472 tests-executed=2472 tests-skipped=0 tests-missing=0 docs=13 claim-refs=7 tags=10 tiers=8`
- `dash tools/gate.sh`: the same line, `gate: PASS`.
- `sh tools/gate-linux.sh` (not required by this plan; run for the new dd/od shell and the cmake script on cmake 3.28, gcc 13 and dash): Ubuntu 24.04 container, x86_64, gcc 13.3.0, cmake 3.28.3: `gate: PASS steps=30/30 controls=263/263 ... cc=GNU ... claims=8 provisional=7 ... discrepancies=68 ... faults=140/140 ... listings=2 listing-words=78 ... tests-expected=2472 tests-executed=2472 tests-skipped=0 tests-missing=0 ... claim-refs=7`, `gate-time total=159`, `gate-linux: exit 0`.

## Threat Flags

None beyond the plan's threat model:
- T-04-46 mitigated: derivation with page cites committed before the first run; R7, flags and time excluded; shared-misreading risk in CL-0008, the `.expect` header and docs.
- T-04-47 mitigated: the container is written only under `build/`; the manifest lists only the listing; the files and manifests steps pass with `binaries-listed=0`.
- T-04-48 mitigated: CL-0008 provisional, tier `original-diagnostic`, statement scoped to the project stub and diagnostic.
- T-04-SC: no package installs; libasm not installed.

## Self-Check: PASSED

- Files present: fixtures/FX-diag-cpu-1/diag-cpu-1.listing.txt, fixtures/FX-diag-cpu-1/manifest.json, data/golden/diag-cpu-1.expect, tests/expect-inc.cmake, tests/run/run_diag.c.
- Commits reachable from HEAD: 90f9609, dcdb57e, b2305e9, e8b6330 (`git rev-list --count 5fb2884..HEAD` = 4 before the docs commit).
- No `.ivsc`, `.bin`, `.rom` or `.exec` file tracked (`git ls-files` check).
