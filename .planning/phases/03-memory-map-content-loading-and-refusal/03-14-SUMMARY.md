---
phase: 03-memory-map-content-loading-and-refusal
plan: 14
subsystem: evidence
tags: [phase-close, evidence-table, record-reconciliation, D5, D-24, D-23, D-38, A-07, ci-budget, honest-caveats]

requires:
  - phase: 03-memory-map-content-loading-and-refusal
    provides: "plans 03-01..03-13: fault loop, SHA-256, public headers, memmap descriptors and literal, decode dump, IVSC v1, machine lifecycle, loads, refusal matrix, reset, isolation, API effect table"
provides:
  - "One fresh gate run mapped to ROADMAP SC1-SC5 and MEM-01..05, API-01, API-06 (12-row evidence table below)"
  - "Records say D5 is decided by D-24: reset.r7 resolving_evidence and note, C32 and C25 resolvers; C5 policy names memmap.undriven_read"
  - ".planning/CI-EFFICIENCY.md: dated D-38/A-07 re-baseline section (hosted budget 40 s, revisit trigger 50 s), phase-end measurement, hosted figures of the phase, remaining levers"
affects: [phase-04, verify-work]

actuals:
  tokens: 9000
  tasks: 3
  commits: 2
plan_head_before: e65ee54ef9701a0bedc3aa4924f93aff985ec9ca
plan_head_after: 5b11ef5747db0004e8cd6e2c3a130a3e6b7f5b7b

tech-stack:
  added: []
  patterns:
    - "A throwaway per-id record check in the gitignored build tree, shown to fail for exactly the stale records before the edit and clean after it"

key-files:
  created:
    - .planning/phases/03-memory-map-content-loading-and-refusal/03-14-SUMMARY.md
  modified:
    - data/discrepancies.json
    - data/profiles/intv-2609-ntsc-family-p1.json
    - .planning/CI-EFFICIENCY.md

key-decisions:
  - "D5 is recorded as decided by D-24 in the profile and discrepancy records; the hardware read-back stays a D10 measurement and the boot stub stays Phase 4 work"
  - "Hosted gate budget re-baselined 30 s -> 40 s with the revisit trigger at 50 s (D-38, A-07); no lever applied at phase end because the two latest hosted runs (30 s, 33 s) are inside the budget"
  - "MEM-01..05, API-01 and API-06 stay Complete in REQUIREMENTS.md: each has a green gate stat, a named test family and at least one fault or control"

patterns-established:
  - "Phase close-out maps every success criterion and requirement to stats, test families, faults and controls of one gate run, and lists what the evidence does not show"

requirements-completed: [MEM-01, MEM-02, MEM-03, MEM-04, MEM-05, API-01, API-06]

coverage:
  - id: D1
    description: "Every SC and requirement maps to green, controlled gate evidence from one fresh run"
    requirement: MEM-01
    verification:
      - kind: integration
        ref: "sh tools/gate.sh: gate: PASS steps=26/26 controls=224/224 faults=115/115 tests-expected=2094 tests-executed=2094 tests-skipped=0 tests-missing=0 claims=7"
        status: pass
    human_judgment: false
  - id: D2
    description: "Records reconciled with what the phase decided, checked per id by a script that failed first"
    requirement: MEM-01
    verification:
      - kind: integration
        ref: "cmake -DROOT=\"$PWD\" -P build/phase3-records.cmake: three FAIL lines before the edit, exit 0 after"
        status: pass
    human_judgment: false
  - id: D3
    description: "Hosted budget re-baselined with phase-end measurement"
    requirement: API-06
    verification:
      - kind: integration
        ref: "grep -c -F 'hosted gate budget 30 s -> 40 s' .planning/CI-EFFICIENCY.md prints 1; sh and dash gate PASS"
        status: pass
    human_judgment: false

duration: 12min
completed: 2026-11-11
status: complete
---

# Phase 3 Plan 14: Phase evidence, record reconciliation and budget re-baseline Summary

**Phase 3 closes with one fresh gate run mapped to SC1-SC5 and the seven requirements. The profile and discrepancy records now say D5 is decided by D-24. `CI-EFFICIENCY.md` re-baselines the hosted budget to 40 s with a 50 s trigger. The limits of the evidence are written down below.**

## Performance

- **Duration:** about 12 min (2026-11-11T08:47:37Z to about 08:59Z).
- **Tasks:** 3 of 3. Task 1, the tracer, is the gate run and the evidence table in this file. It changed no tracked file, so it has no commit.
- **Files:** 3 tracked files in 2 commits (+93, -5), plus this SUMMARY.
- **Gate runs:** 9 in total, all `gate: PASS`. The timings are in the measurement section.

## Fresh gate run (Task 1 tracer)

`sh tools/gate.sh` at HEAD `e65ee54`, before any edit. It exited 0 with `gate-time total=25`:

```
gate: PASS steps=26/26 controls=224/224 api-calls=10 api-statuses=14 api-rows=27 cc=AppleClang c-presets=2 fault-runs=115 core-imports=4 purity-selftest=5/5 ctest-runs=2 ci-uses=1 ci-workflows=1 claims=7 provisional=6 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=184 waived=0 discrepancies=62 pending-resolvers=18 open=57 faults=115/115 tracked=1157 binaries-listed=0 terms=9 doc-files=12 profile-keys=51 mode=tree hygiene-files=1157 history-commits=1 files=1157 manifests=2 effective-yes=1 effective-no=1 literal-rows=28 views=2 dump-lines=56 records=51 fields=51 unknown-fields=16 cases=4 sources=9 refs-verified=1/4 tests-expected=2094 tests-executed=2094 tests-skipped=0 tests-missing=0 docs=12 claim-refs=6 tags=10 tiers=8
```

The final run after Task 3 has the same line except `pending-resolvers=17`. C32's resolver is now a measurement, not `pending:`, which is the intended effect of Task 2. The sh and dash lines are byte-identical (`cmp`).

**Inventory at the phase end**
- **Tests:** 1047 ids in two presets, so 2094 executed of 2094 expected:
  - `cpu-vectors` 412, `machine` 356, `memmap` 86, `cpu-sweep` 75, `ivsc` 75;
  - `cpu-api` 24, `cpu-fuzz` 8, `headers` 6, `harness` 4, `core-symbols` 1.
- **Faults:** 115 = cpu 67, machine 25, memmap 11, sha256 6, ivsc 6.
- **Machine test families:**
  - `refuse` 250, `prec` 25, `sha256` 24, `load` 16, `digest` 12;
  - `create` 8, `sweep` 7, `reset` 6, `iso` 5;
  - `api`, `destroy` and `status` 1 each.

## Evidence table: SC1-SC5 and the seven requirements

Every row's stats come from the run above. Every named test ran in both presets (`tests-missing=0`). Every named fault is detected (`faults=115/115`). Every named control printed `control ok` (`controls=224/224`).

| # | Criterion | Gate stats | Test families | Faults | Controls |
|---|-----------|-----------|---------------|--------|----------|
| SC1 | Decode dump matches the sourced literal; 10/8/16-bit masking and truncation at each boundary | `literal-rows=28 views=2 dump-lines=56`, `profile-keys=51`, `tests-missing=0`, `claims=7` (CL-0006 provisional) | `memmap.dump.matches-literal`, `memmap.row.*` (28), `memmap.row-gate.*`, `memmap.width.*` (6), `memmap.anchor.*` (28), `memmap.mirror.gram-3a00`, `memmap.gate.*`, `memmap.unloaded.*` | all 11 memmap: `GEN_MAP_MUTATION`, `MEM_ROM_NO_MASK`, `MEM_RAM8_NO_TRUNC`, `MEM_RAM16_TRUNC`, `MEM_ROM_WRITABLE`, `MEM_MIRROR_MASK`, `MEM_GATE_IGNORED`, `MEM_UNDRIVEN_ZERO`, `MEM_CART_END_INCLUSIVE`, `MEM_WO_READS_DEVICE`, `MEM_IAB_VECTOR_SWAPPED` | `memmap-dump-{diff,stale,missing}`, the ten `memmap-literal-*` |
| SC2 | EXEC, GROM and cartridge load as separate typed inputs with length and digest; stand-in bytes only; no real image committed | `tests-executed=2094`, `binaries-listed=0`, `tracked=1157`, `manifests=2 effective-yes=1 effective-no=1`, `scanned=184 waived=0` | `machine.load.*` (16: `exec-ok`, `grom-ok`, `cart-ok`, `info-all-slots`, `order-free`, `cart-whole-digest-not-trailer`, `expect-*`), `machine.sha256.*` (24) | `CART_REPORT_TRAILER_DIGEST`, `UNLOAD_KEEPS_IMAGE`, `LOAD_TABLE_BEFORE_VALIDATE`, the six `SHA_*` | `files-unlisted-binary`, `files-unlisted-binary-{content,archive,nonul}`, `files-fixture-without-manifest`, `manifests-digest-mismatch` |
| SC3 | Every refusal-matrix row returns its own typed error; storage and state digest unchanged | `api-rows=27`, `api-statuses=14`, `tests-missing=0` | `machine.refuse.*` (250, priors f/s/c/w, whole-storage memcmp plus digest), `machine.prec.*` (25). Unknown profile: `machine.refuse.create.unknown-profile`. Missing system image: `machine.refuse.reset.no-exec`, `.no-grom`, `.neither` | `CHECK_ORDER_MALFORMED_FIRST`, `OVERLAP_BEFORE_LENGTH`, `LOAD_LEN_BEFORE_CHECK`, `CART_COPY_BEFORE_VALIDATE`, `DUPLICATE_REPLACES`, `LEN_TRUNCATED_32`, `ROM_DECLE_MASKED_NOT_REFUSED`, `STATE_CHECK_OMITTED`, `RESET_CLEARS_BEFORE_IMAGE_CHECK`, `CREATE_MAGIC_BEFORE_PROFILE_CHECK` | `api-dead-status`, `api-refusal-no-test`, `api-effect-unregistered` |
| SC4 | Cartridge format in the project's own words; positive, boundary and malformed fixtures pass and sit in the manifest | `docs=12 claim-refs=6`, `terms=9 doc-files=12`, `manifests=2 effective-yes=1`, `claims=7` (CL-0007) | `ivsc.P01..P06`, `ivsc.B01..B12`, `ivsc.N01..N34`, `ivsc.L01..L10`, `ivsc.X01..X03`, `ivsc.fuzz.*` (9), `machine.sweep.*` (7) | all 6 ivsc: `IVSC_SKIP_DIGEST`, `IVSC_END_OFF_BY_ONE`, `IVSC_ACCEPT_WORD_400`, `IVSC_COUNT_17`, `IVSC_TRUST_TOTAL_BYTES`, `IVSC_ADJACENT_AS_OVERLAP` | `manifests-{missing-key,digest-mismatch,id-missing,uncommitted-output-tracked}`, `files-fixture-without-manifest` |
| SC5 | Two instances with different content step without affecting each other; every error has a documented, tested effect | `api-calls=10 api-statuses=14 api-rows=27`, `core-imports=4 purity-selftest=5/5`, `steps=26/26` | `machine.iso.*` (5: interleaved-equals-sequential, poisoned-storage-identical, memcpy-relocation, destroy-mid-run, cross-load), `machine.reset.*`, `headers.*` (6) | `STATIC_CARRY`, `INIT_SKIPS_MEMSET`, `DESTROY_NO_SCRUB`, `RESET_KEEPS_RAM` | `core-purity-static-mutable`, all nine `api-*` |
| MEM-01 | Map generated from descriptors; dump equals the literal | `literal-rows=28 views=2 dump-lines=56`, `fields=51 unknown-fields=16` | `memmap.dump.matches-literal`, `memmap.anchor.*`, `memmap.registry.*`, `memmap.describe.*` | `GEN_MAP_MUTATION`, `MEM_MIRROR_MASK`, `MEM_WO_READS_DEVICE`, `MEM_CART_END_INCLUSIVE` | `memmap-dump-*`, `memmap-literal-*`, `core-purity-generated-{unlisted,scan,missing}` |
| MEM-02 | Width masking and store truncation tested at each boundary | `literal-rows=28`, `tests-missing=0` | `memmap.row.*` (lo-1, lo, hi, hi+1, mirror wrap per literal row), `memmap.width.*`, `memmap.gate.*` | `MEM_ROM_NO_MASK`, `MEM_RAM8_NO_TRUNC`, `MEM_RAM16_TRUNC`, `MEM_ROM_WRITABLE`, `MEM_GATE_IGNORED` | `memmap-dump-diff` |
| MEM-03 | Typed loads of EXEC, GROM and cartridge; length and digest reported (D5 decided as D-24) | `tests-executed=2094`, `fault-runs=115` | `machine.load.*`, `machine.sha256.*` (FIPS and `cmake -E sha256sum` literals), `machine.refuse.load.{exec,grom}.*` shape rows | the six `SHA_*`, `CART_REPORT_TRAILER_DIGEST`, `LOAD_TABLE_BEFORE_VALIDATE`, `UNLOAD_KEEPS_IMAGE` | `faults-not-detected`, `faults-missing-site`, `tests-inventory-missing` |
| MEM-04 | Malformed, oversized, overlapping, out-of-range, missing-image and unknown-profile inputs refused with effect none | `api-rows=27`, `tests-skipped=0` | `machine.refuse.*`, `machine.prec.*`, `ivsc.N*`, `ivsc.L*` | the ten SC3 faults, `CART_SPAN_END_INCLUSIVE`, `REPORT_INSIDE_UNCHECKED`, `OUT_OVERLAP_UNCHECKED` | `api-refusal-no-test`, `api-dead-status` |
| MEM-05 | One documented cartridge format with positive, boundary and malformed fixtures | `manifests=2 effective-yes=1`, `claims=7`, `docs=12` | `ivsc.*` (75), `machine.sweep.*` | the 6 `IVSC_*` | `manifests-*`, `files-unlisted-binary-*` |
| API-01 | Create, reset, unload and destroy independent instances; no shared mutable state | `core-imports=4 purity-selftest=5/5`, `api-calls=10` | `machine.create.*`, `machine.destroy.*`, `machine.reset.*`, `machine.iso.*`, `headers.*` | `STATIC_CARRY`, `INIT_SKIPS_MEMSET`, `DESTROY_NO_SCRUB`, `RESET_KEEPS_RAM` | `core-purity-static-mutable`, `core-purity-thread`, `core-purity-clock`, `api-call-undeclared` |
| API-06 | Every error has a documented state effect and a test | `api-calls=10 api-statuses=14 api-rows=27` | `machine.refuse.*`, `machine.prec.*`, `machine.digest.*` (12) | `DIGEST_SKIPS_RAM`, `DIGEST_SKIPS_IMAGE_TABLE`, `DIGEST_LITTLE_ENDIAN`, `DIGEST_INCLUDES_PADDING`, `DOMAIN_ID_DROPPED` | all nine `api-*`: `api-status-{renumbered,unpinned,reserved}`, `api-call-{undocumented,undeclared}`, `api-refusal-no-test`, `api-effect-unregistered`, `api-dead-status`, `api-effects-format` |

Test id notes:
- The plan names `memmap.dump.*`. The inventory has one dump case, `memmap.dump.matches-literal`. The gate step `memmap` compares the nonce-bound dev and release dumps on top of it (`dump-lines=56`).
- The faults listed per row are the ones whose intended witness is that family, per the fault files and the 03-xx SUMMARY re-judging tables. `faults-verdict` checks each fault against its listed witness ids.

## Gaps

None. Every criterion has a green stat, a named test family that ran in both presets, and at least one named fault or control. The tier limits of that evidence are in the next two sections; none of them is a red stat.

## Record reconciliation (Task 2)

**Script.** `build/phase3-records.cmake` is throwaway and untracked: `build/` is gitignored and the file is not committed. It checks per id:
- C21's resolver starts with `measurement:`.
- C51 is `partial` with non-blank evidence.
- C60, C61 and C62 exist, and each resolver starts with `measurement: D10`.
- In `reset.r7` resolving_evidence and note, and in the C32 and C25 resolvers, every `D5` is followed by ` decided`, and at least one occurs.
- `interrupt.iab_value` resolving_evidence is exactly `measurement: D10 interrupt vector read-back on owner hardware`.
- The profile has `memmap.cart_span`, `cartridge.features_supported`, `system_images.exec` and `system_images.grom`, and lacks `memmap.cart_window` and `memmap.cart_align_words`.

**Before the edits** (`cmake -DROOT="$PWD" -P build/phase3-records.cmake`, exit 1):

```
FAIL reset.r7: D5 not recorded as decided in resolving_evidence or note
FAIL C32: D5 not recorded as decided in resolver
FAIL C25: D5 not recorded as decided in resolver
CMake Error at build/phase3-records.cmake:141 (message):
  phase3-records: 3 failing rule(s)
```

Exactly three FAIL lines, for the stale records, and none for `interrupt.iab_value` or the other rules.

A scratch-root mutation also showed the other rules can fire. It used a copy of the two files with C21 `pending:`, C51 `open`, `memmap.cart_span` renamed to `memmap.cart_window`, and the `iab_value` text shortened. That run printed `FAIL C21`, `FAIL C51`, `FAIL interrupt.iab_value`, `FAIL memmap.cart_span` and `FAIL memmap.cart_window`, plus the three above.

**After the edits** (exit 0):

```
phase3-records: pass
```

**Text edits** (commit `9e5131b`). No tag, status, value or conflict changed.

| Record | Old text | New text |
|--------|----------|----------|
| `reset.r7` resolving_evidence | `pending: Phase 2 boot-stub decision (D5)` | `measurement: D10 reset read-back on owner hardware (D5 decided as D-24: user-supplied system images only; project boot stub in Phase 4)` |
| `reset.r7` note | "... unknown-default-chosen for Phase 2 and D5." | "... unknown-default-chosen; D5 decided as D-24 (user-supplied images only, project boot stub in Phase 4), so the EXEC or the boot stub supplies it." |
| C32 resolver | `pending: Phase 2 boot-stub decision (D5); measurement: D10 reset read-back on owner hardware` | `measurement: D10 reset read-back on owner hardware (D5 decided as D-24: user-supplied system images only, project boot stub in Phase 4)` |
| C25 resolver | `pending: D5 owner decision (open question 3), conditions in the Phase 4 plan` | `pending: Phase 4 project boot stub plan (D5 decided as D-24: user-supplied system images only)` |
| C5 default_policy | (appended, the plan's optional line) | "Blocked and undriven reads are the profile field memmap.undriven_read." |

- `interrupt.iab_value` is unchanged.
- `profile-fields.ids` equality stays enforced by the profile step: `-- profile pass records=51 fields=51 unknown-fields=16`.
- `-- discrepancies pass discrepancies=62 pending-resolvers=17 open=57`. The count was 18 before; C32 moved from `pending:` to `measurement:`.

## Budget re-baseline and phase-end measurement (Task 3)

New dated section in `.planning/CI-EFFICIENCY.md` (commit `5b11ef5`): "D-38/A-07 re-baseline: hosted gate budget 30 s -> 40 s; revisit trigger 40 s -> 50 s".

- **Local, three measured `sh tools/gate.sh` runs:** 30, 36 and 38 s, so the median is 36 s. The load at start was 9.9, 17.7 and 17.4.
- **Lower-load runs in the same session:** 25, 26 and 27 s at load 6.8 to 8.8. The median over all six runs is 28.5 s.
- **Earlier points:** 29 s planning baseline, 20 s at 03-02, 23 s at 03-11.
- **Largest steps** (run 1):
  - `controls` 13 s (overlapped, `controls-wait` 0);
  - `hygiene-history` 6 s;
  - `faults-verdict` 4 s;
  - `cp1610-vectors` 4 s;
  - `c-tests` and `c-faults` 3 s each.
- **Hosted figures of the phase** (read by the orchestrator): 36 s after 03-02, 44 s after 03-04 (cold gcc `c-build` 26 s), 30 s after 03-06 and 33 s after 03-10. One of the four runs was above 40 s, and none was above 50 s.
- **Levers:**
  - Applied: fault lanes (03-02).
  - Remaining, in order: owning-TU rebuild per fault; a background C build that overlaps a cold `c-build` with the steps that do not read the build tree.
  - Watch item: `hygiene-history` takes 4 to 6 s locally.
  - Not used: ccache and `actions/cache`.
- **Open item for the orchestrator:** the hosted figure of this plan's push.

## Honest caveats

- **MEM-01 is `source-expectation` tier.** Most literal rows are tier 4 from one community lineage (the wiki memory map, oldid 14847), next to the GI 1980 catalog pages for widths and the GROM/GRAM windows. The dump equality shows transcription consistency between the literal and the descriptors, not hardware truth. CL-0006 stays provisional.
- **The dump's device, impl and gate columns are declarative labels.** The widths and mirrors are measured behaviourally per row class, but these three columns report what the descriptors say.
- **D10 measurements are missing.** C20, C21, C60, C61 and C62 need them. C32 (reset read-back) and C42 (interrupt vector) do too.
- **System-image byte order is unverified.** The big-endian order of D-26 is [BG] background knowledge until a real owner-held EXEC/GROM is tried under D11. A wrong guess would surface as a MALFORMED refusal, not a silent miscompute.
- **Cartridge span is tier 4 (C62).** The span `$4000-$FFFF` and the window addresses come from community documents. The 256-word alignment is a tier-4 note only (A-01).
- **The `api-effects` step has a residual (OQ-5).** It proves each cited test id exists in an inventory, and `tests-inventory` proves the case ran. It cannot prove that a cited test returns the documented status. Each test asserts its own code, and plan 03-13's one-off audit found 0 mismatches. A later drift would not fail `api-effects`.
- **The writable-data rule's Linux behaviour rests on hosted runs only (RESEARCH A1).** The hosted gates of this phase passed. That run includes `c-purity` with `-fno-pie` and the `core-purity-static-mutable` control on the ubuntu gcc runner. This plan did not read the hosted logs itself; the orchestrator recorded the runs.
- **Isolation is behavioural.** There is no threaded test, by design (D-36). API-01's "run concurrently" rests on:
  - the five `machine.iso.*` cases;
  - the `c-purity` writable-data rule;
  - the purity bans on threads, clocks and globals.
- **Fault executables cannot detect over-reads.** ASan and UBSan in the dev preset cover them for the cases that run there.
- **Calling rules live in `data/api/api-effects.json`.** It documents:
  - the load precedence, with OVERSIZED before the input-overlap BAD_ARG (A-08a) and the staged cartridge order (A-08b);
  - the one-way power cycle (A-08d);
  - BAD_ARG for unknown enumerants (A-08e);
  - the create order.
- **Stand-ins are project-authored.** The stand-in programs, EXEC/GROM images and IVSC fixtures say nothing about commercial content. Their oracles are independent of the code under test only in the stated places:
  - the SHA-256 literals came from `cmake -E sha256sum` at authoring time;
  - the literal map was authored from sources before the descriptors were read;
  - the stand-in programs were hand-assembled.
- **The libFuzzer target was not run with ASan on the dev host.** The ASan runtime hung at start-up (03-08). The seeded fuzz family `ivsc.fuzz.*` runs in the gate. The libFuzzer target is optional and outside the gate.
- **Nothing in this phase is handed to the owner as UAT.** D10 and D11 are hard stops in `.planning/AUTONOMY.md`. Their evidence is recorded as missing, not handed off.

## Accepted risks / caveats

Open items carried from plans 03-01..03-13 and accepted at phase close. None blocks an SC or a requirement; each is a known hole in a control or a test.

- **`memmap-gen-*` generator controls are missing** (03-04, 03-05, 03-06). `cmake/IvsMemmapTable.cmake` is covered by its fail-closed build refusals and `GEN_MAP_MUTATION`. There is no gate-level negative control per refusal, which needs a toy memmap profile fixture convention.
- **No digest-fields golden** (03-09, 03-13). The `IVS_MACHINE_BYTES` static assert, `machine.digest.coverage` and the domain version stand in.
- **Unused generated-include inventory lines are not flagged** (03-03).
- **`vectors.cmake` reads only `faults/cpu.ids`** for the policy-has-a-fault rule (03-01). Moving a CPU fault to another file would trip `vector-policy-no-fault`.
- **`STATIC_CARRY` is not caught by `machine.iso.memcpy-relocation`** (03-12). The carried count cancels there. The must-fail case and three other cases catch it.
- **The stepped stand-ins never read cartridge words** (03-12). Cartridge content enters the isolation cases through the images digest only.
- **`len-wrap` rows exist only where `size_t` is 64 bits** (`skip-ok` elsewhere). No current CI host exercises the 32-bit path.
- **No named fault for the IVSC `total_bytes`-below-length check** (03-08). N09 and fuzz growth mutants reach it.
- **Case ids carry no profile id** (`memmap.row.<view>-<LO>-<HI>`, 03-06). A second profile would collide. The positive cartridge fixtures also assume a single span (03-07).
- **The mirror probe checks only the wrap at lo+mirror** (03-06). A halved mirror is caught by the dump and by `memmap.mirror.gram-3a00`.
- **The machine-suite dev run is over its family budget** (0.43 s dev against the 0.25 s phase-end figure, 03-12). The release run is inside it.
- **The phase PR's final hosted gate time is the orchestrator's to record.** If it is above 40 s, apply the remaining levers in `CI-EFFICIENCY.md` order.

## Task Commits

1. **Task 1, tracer: one fresh gate run mapped to SC1-SC5 and the seven requirements.** No tracked change and no commit. Tracer gate: the fresh run above, before any expansion task.
2. **Task 2: discrepancy and profile record reconciliation.** `9e5131b` (docs).
3. **Task 3: D-38/A-07 re-baseline, phase-end measurement and caveats.** `5b11ef5` (docs).

## Decisions Made

See `key-decisions` in the frontmatter.

## Deviations from Plan

**1. [Counts] Plan text against reality**
- **Plan text:** `controls=221/221`, `faults=113/113`, tests not stated.
- **Reality:** `controls=224/224`, `faults=115/115`, `tests-expected=2094`, `steps=26/26`.
- **Controls +3:** 03-05 added `memmap-literal-unknown-tag` and `memmap-literal-orphan`, and 03-13 added `api-effects-format`.
- **Faults +2:** 03-11 added `REPORT_INSIDE_UNCHECKED` and 03-12 added `OUT_OVERLAP_UNCHECKED`.
- **Where:** the CI-EFFICIENCY section states the real counts next to the plan's.

**2. [Rule 2 - Stricter check] The record script also requires at least one `D5` occurrence**
- **What:** the plan's "every occurrence of `D5` is followed by ` decided`" passes vacuously if the text drops D5. The script also requires one occurrence.
- **Effect:** none on the before or after runs.

**3. [Rule 2 - Record hygiene] ROADMAP and STATE D5 status lines**
- **What:** `.planning/ROADMAP.md`'s decision-gate row and `.planning/STATE.md`'s "Still open" list still showed D5 as open. PROJECT.md's Decision Register records it as decided (D-24, 2026-11-11).
- **Fix:** both lines were updated in the plan-metadata commit.
- **Not changed:** the Phase 3 and Phase 4 prose in ROADMAP ("D5 (owner, open)"). That is planning-time text, and it is left for the phase-completion step.

**4. [Measurement] Three measured runs under a load spike**
- **What:** the three designated runs had load 9.9 to 17.7.
- **Response:** the three lower-load runs of the same session are recorded next to them (median of six: 28.5 s). No lever was applied on a load-contaminated local figure.

**Total deviations:** 4, all record or measurement notes. No test, control, token or check changed.

## Issues Encountered

None.

## Known Stubs

None. The STIC and PSG stub devices are the planned D-19 policy (Phase 5+), not stubs of this plan.

## Threat Flags

No new surface. T-03-49 is covered by the evidence table, `## Gaps` and the caveat sections. T-03-50 is covered by the measured re-baseline, with no check dropped and the hosted figure left to the orchestrator.

## User Setup Required

None.

## Next Phase Readiness

- Phase 3 is ready for `/gsd-verify-work`. All seven requirements read Complete in REQUIREMENTS.md, backed by the table above.
- **Phase 4** builds on:
  - `ivs_machine_step_unit_internal`, for `ivs_machine_run`;
  - the effect table, which forces a pin line and a row per new status or function;
  - the C25 resolver, which now points at the Phase 4 project boot stub plan.
- **Final gate:**
  - `sh tools/gate.sh`: `gate: PASS steps=26/26 controls=224/224 ... discrepancies=62 pending-resolvers=17 open=57 faults=115/115 ... tests-expected=2094 tests-executed=2094 tests-skipped=0 tests-missing=0 ...` (`gate-time total=28`).
  - `dash tools/gate.sh`: the identical summary line (`gate-time total=28`).

## Self-Check: PASSED

- **Files found:**
  - `.planning/phases/03-memory-map-content-loading-and-refusal/03-14-SUMMARY.md`
  - `.planning/CI-EFFICIENCY.md`
  - `data/discrepancies.json`
  - `data/profiles/intv-2609-ntsc-family-p1.json`
- **Commits:** `9e5131b` and `5b11ef5` are ancestors of HEAD. The ledger count from `e65ee54` is 2, and each message ends with the Co-Authored-By line.
- **Record script:** `build/phase3-records.cmake` is gitignored and in no commit. It exits 0 at HEAD.
- **Budget section:** `grep -c -F 'hosted gate budget 30 s -> 40 s' .planning/CI-EFFICIENCY.md` prints 1.
- **Hygiene:**
  - No absolute home path and no "Intellivision" in any added line.
  - No rights-bearing bytes.
  - Nothing was copied from another emulator or assembler.
  - `.planning/config.json` and `.planning/state.json` are still ` M` and in no commit of this plan.
