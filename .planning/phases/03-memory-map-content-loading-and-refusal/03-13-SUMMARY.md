---
phase: 03-memory-map-content-loading-and-refusal
plan: 13
subsystem: api
tags: [api-effects, effect-table, status-pin, symbol-list, refusal-effects, gate-step, negative-controls, D-06, D-08, D-09, D-12, D-15, A-08, API-06]

requires:
  - phase: 03-memory-map-content-loading-and-refusal
    provides: "plan 03-09: public headers, ivs_status.h, api-statuses.ids; plan 03-10: load ladder; plan 03-11: refusal matrix, prec rows, out-parameter rule; plan 03-12: reset, powered prior, headers suite"
provides:
  - "data/api/api-effects.json: ten calls, 27 refusal rows (call x status x effect x outs x tests), precedence arrays with pinned_by test ids, notes for A-08 a/b/d/e and the out-parameter rule"
  - "Static gate step api-effects (tools/check/api-effects.cmake, cmake -P): status pin, reserved band, IVS_API list equality (D-06), cited test ids, registered effects, no dead status, table shape"
  - "Nine controls (eight planned plus api-effects-format) and good-tree toy inputs"
affects: [03-14, phase-04]

actuals:
  tokens: 16000
  tasks: 3
  commits: 2
plan_head_before: d667a5d6d2f00d868720159de5a8d0ab89446479
plan_head_after: 7498c92055b2d5608c0b1b439eb4c89a42e21019

tech-stack:
  added: []
  patterns:
    - "A cmake -P checker reading only ROOT, so the control runner's tree and the real tree use one code path; capped findings per token"
    - "The JSON calls list is the D-06 symbol list: IVS_API lines of core/include/ivs/*.h must equal it both ways"
    - "Precedence entries carry pinned_by test ids, so the documented order is checked against the inventories like the refusal rows"

key-files:
  created:
    - data/api/api-effects.json
    - tools/check/api-effects.cmake
    - tools/check/steps/api-effects.sh
    - data/expected/controls/api-effects.ids
    - tools/check/controls/_good/tree/core/include/ivs/ivs_status.h
    - tools/check/controls/_good/tree/data/api/api-effects.json
    - tools/check/controls/_good/tree/data/expected/api-statuses.ids
    - tools/check/controls/api-status-renumbered/
    - tools/check/controls/api-status-unpinned/
    - tools/check/controls/api-status-reserved/
    - tools/check/controls/api-call-undocumented/
    - tools/check/controls/api-call-undeclared/
    - tools/check/controls/api-refusal-no-test/
    - tools/check/controls/api-effect-unregistered/
    - tools/check/controls/api-dead-status/
    - tools/check/controls/api-effects-format/
  modified:
    - data/expected/gate-steps/machine.ids
    - tools/check/controls/sources-no-files/MAKE

key-decisions:
  - "Status names in the table are the ivs_status.h macro names (IVS_E_BAD_ARG ...), so rows compare directly with the pin"
  - "One row per (call, status); a status that two stages return (load BAD_ARG, OVERSIZED, MALFORMED, OUT_OF_RANGE, UNSUPPORTED, DIGEST_MISMATCH) has one row and several precedence entries"
  - "A row needs at least one cited test that is in an inventory and not skip-ok (the len-wrap rows are skip-ok on 32-bit hosts)"
  - "A ninth token and control, api-effects-format, covers table shape, unpinned row or precedence statuses and row/precedence consistency"
  - "API-06 marked complete: every status every public call can return has a row with effect none and cited tests; the OQ-5 residual is stated in the table note, the checker header and here"

patterns-established:
  - "A new public function or status cannot land without a table row: the step fails on an undocumented IVS_API declaration, an unpinned define and a status with no row"

requirements-completed: [API-06]

coverage:
  - id: D1
    description: "Every public function, status and refusal row is tied to the headers, the pin and the test inventories by a static step (D-06, D-09, D-15)"
    requirement: API-06
    verification:
      - kind: integration
        ref: "sh and dash tools/gate.sh: steps=26/26, api-calls=10 api-statuses=14 api-rows=27"
        status: pass
    human_judgment: false
  - id: D2
    description: "Each table rule fails for its own token, and the toy tree passes"
    requirement: API-06
    verification:
      - kind: integration
        ref: "sh tools/check/run-controls.sh: controls=224/224, nine `control ok   api-*` lines and `control ok   _good/api-effects`"
        status: pass
    human_judgment: false
  - id: D3
    description: "Every (public function, status) pair the code can return has a row whose cited tests assert that status"
    requirement: API-06
    verification:
      - kind: unit
        ref: "Task 3 audit table below; cited cases ran in both presets (tests-expected=2094 tests-executed=2094 tests-missing=0)"
        status: pass
    human_judgment: false

duration: 10min
completed: 2026-11-11
status: complete
---

# Phase 3 Plan 13: API effect table and the api-effects gate step Summary

**`data/api/api-effects.json` documents the success behaviour, check precedence and refusal effect of all ten public functions. A cheap static gate step ties it to the status header, the status pin, the `IVS_API` declarations and the test inventories, and nine controls prove each rule fails for its own token.**

## Performance

- **Duration:** about 10 min (2026-11-11T08:35:57Z to about 08:47Z).
- **Tasks:** 3 of 3. Task 3, the audit, changed nothing, so it has no commit.
- **Files:** 46 in 2 commits (+1055).
- **Step cost:** `step-time api-effects 0`. The standalone run takes 0.06 s: one `cmake -P` over three small files, the headers and about 1,100 inventory lines.
- **Gate time (local, warm):** `gate-time total` 25 (sh) and 26 (dash) after Task 2, and 29 after Task 1. These are local figures. The orchestrator reads the hosted figure (budget 40 s, revisit trigger 50 s).

## Accomplishments

### The effect table (`data/api/api-effects.json`)

- **Registry:** one effect, `none`. A refused call:
  - leaves the instance storage byte-identical (whole-storage memcmp);
  - leaves the state digest equal, where the handle is valid before and after;
  - writes only the listed out parameters;
  - allocates nothing.
- **Fields of each call:**
  - `header`;
  - `on_ok`;
  - `precedence`: ordered `{status, checks, pinned_by}`;
  - `notes`;
  - `refusals`: `{status, effect, outs, tests}`.
- **Rows:** 27 over the ten calls. `ivs_api_version` and `ivs_status_name` have no refusals and an `on_ok` text. Every Phase 3 refusal has effect `none`.
- **Rules written into the table:**
  - **A-08 a:** OVERSIZED precedes the input-overlap BAD_ARG. The overlap test runs only within the bound. Pinned by `machine.prec.oversized-over-overlap`.
  - **A-08 b:** UNSUPPORTED (options) comes after BAD_HANDLE. In a cartridge, the layer-1 parser runs in D-33 order, then layer 2 (span, then features), then `expect_sha256`. Each stage has its own precedence entry, and the cross-stage outcomes are pinned by the `machine.prec.cart-*` rows. D-33 governs the inside of the parser.
  - **A-08 d:** reset from POWERED is a power cycle. Nothing returns POWERED to OFF except `destroy`, so load and unload are BAD_STATE after the first reset. This note is on reset, load and unload, and is pinned by `.after-power-cycle.w` and `machine.reset.power-cycle-one-way`.
  - **A-08 e:** unknown enumerants are BAD_ARG, never UNSUPPORTED.
  - **Out-parameter rule:** as written in `core/include/ivs/ivs_machine.h`. An out parameter that overlaps the instance is BAD_ARG with nothing written. This covers the load report, the info out, the digest out and the create out pointer. Notes on load, create, info and state_digest record it, and the load precedence lists it first.
  - **Create order:** BAD_ARG (out inside the storage), BAD_ARG (arguments), UNSUPPORTED, UNKNOWN_PROFILE, BUFFER_TOO_SMALL, with `pinned_by` rows.

### The step (`tools/check/api-effects.cmake`, `tools/check/steps/api-effects.sh`)

The step reads only ROOT and writes nothing. It reports the stats `api-calls=10 api-statuses=14 api-rows=27`.

| Token | Fails when |
|-------|------------|
| `api-status-renumbered` | a header define and its pin line disagree, or one number is used by two names |
| `api-status-unpinned` | a name exists on one side only |
| `api-status-reserved` | a defined or pinned value lies in -24..-31 |
| `api-call-undocumented` | an `IVS_API` line in `core/include/ivs/*.h` has no JSON call, or no parsable name |
| `api-call-undeclared` | a JSON call has no `IVS_API` line |
| `api-refusal-no-test` | a cited row test or `pinned_by` id is not a first token of `data/expected/tests/*.ids`, or a row has no live (non-skip-ok) test |
| `api-effect-unregistered` | a row effect is not in the registry |
| `api-dead-status` | a pinned non-OK status has no row |
| `api-effects-format` | the table shape is wrong: unreadable input, schema, keys or types; duplicate names; a header field that differs from the declaring header; a row or precedence status that is not a pinned non-OK name; a duplicate (call, status) row; or a precedence entry without its row, or a row missing from precedence |

The `IVS_API` list equality is the D-06 symbol check that plan 03-09 deferred. Header self-containment is suite `headers` (plan 03-12), which compiles each public header alone as C17 and as C++17.

### Controls

- **Good tree:**
  - a toy `ivs_status.h` (`IVS_OK`, `IVS_E_BAD_ARG`, `IVS_API int ivs_toy(int x);`);
  - its pin;
  - a one-call table citing `cpu.ZERO.alpha`, with `pinned_by` `cpu.ZERO.beta`.
- **Nine controls:** the eight planned plus `api-effects-format`, whose table is not JSON. Each expects its exact `FAIL <token>:` text, and most forbid the neighbouring status tokens.

## Task 3 audit: (function, status, test id)

Method:
1. List every `return IVS_E_*` site with `grep -n 'IVS_E_' core/src/*/*.c`.
2. Follow the helper results: `ivs_load_refuse`, the parser via `ivs_load_cart`, and `ivs_machine_profile_lookup`.
3. Check each cited id against its case source: the `RF_*` table rows of `tests/machine/machine_refuse.c`, the create and digest case bodies, the `ivsc-load.json` expectations and the `ivsc_main.c` code-built cases.

A one-off scratch script, not committed and not in the gate, matched each cited id's asserted status to its row. It found:
- 0 mismatches;
- 2 expected exceptions, checked by hand:
  - `machine.prec.overlap-over-digest` is a struct literal, not an `RF_*` macro; line 1140 asserts `IVS_E_OVERLAP`;
  - `machine.refuse.create.out-after-instance-ok` is the OK boundary case and is cited only in `pinned_by`.

| Function | Status | Code site | Cited tests (examples; the full list is in the table) |
|----------|--------|-----------|------------------------------------------------------|
| ivs_machine_storage_requirements | BAD_ARG | machine.c req/id checks | machine.refuse.requirements.{null-out,size-0,short,null-id,id-unterminated} |
| ivs_machine_storage_requirements | UNKNOWN_PROFILE | profile_lookup | machine.refuse.requirements.unknown-profile |
| ivs_machine_create | BAD_ARG | out overlap, NULLs, short opts, id, alignment | machine.refuse.create.{null-opts,...,out-inside-storage} (11) |
| ivs_machine_create | UNSUPPORTED | flags, tail | machine.refuse.create.{flags-1,tail-nonzero,flags-over-unknown-profile} |
| ivs_machine_create | UNKNOWN_PROFILE | profile_lookup | machine.refuse.create.{unknown-profile,near-miss,trailing-space,id-63-unknown,unknown-over-short-storage} |
| ivs_machine_create | BUFFER_TOO_SMALL | storage size | machine.refuse.create.short-storage |
| ivs_machine_destroy | BAD_HANDLE | from_handle | machine.refuse.destroy.{null,twice,wrong-magic} |
| ivs_machine_reset | BAD_ARG | kind | machine.refuse.reset.{kind-0,kind-2,kind-2-powered}, machine.prec.reset-arg-over-{handle,missing} |
| ivs_machine_reset | BAD_HANDLE | from_handle, profile NULL (defensive) | machine.refuse.reset.{null,destroyed}, machine.prec.reset-handle-over-missing |
| ivs_machine_reset | MISSING_SYSTEM_IMAGE | EXEC then GROM | machine.refuse.reset.{no-exec,no-grom,neither,no-exec-with-cart} |
| ivs_machine_load | BAD_ARG | report check, kind, NULL bytes, opts size, input overlap | 18 ids incl. arg.kind-*, report-inside, input-inside, prec.arg-over-handle, prec.overlap-over-malformed |
| ivs_machine_load | BAD_HANDLE | stage 2 | load.{exec,cart}.null-machine, load.exec.destroyed, prec.handle-over-unsupported |
| ivs_machine_load | UNSUPPORTED | option flags and tail; layer-2 features | flags-bit-*, opts-tail-nonzero, feature-bit-*, ivsc.L05/L06, three prec rows |
| ivs_machine_load | BAD_STATE | stage 4 | load.{exec,grom,cart}.powered.w, exec.after-power-cycle.w, prec.bad-state-over-duplicate |
| ivs_machine_load | DUPLICATE_LOAD | stage 5 | {exec,grom}.duplicate-load.s, cart.duplicate-load.c, ivsc.L07, four prec rows |
| ivs_machine_load | OVERSIZED | stage 6; parser total_bytes | oversized-* (8), len-wrap-* (skip-ok), cart.total-over-max.f, two prec rows |
| ivs_machine_load | MALFORMED | system image short and wide word; parser structure and word range | 26 matrix ids, four prec rows |
| ivs_machine_load | OUT_OF_RANGE | parser addr wrap; layer-2 span | cart.{addr-wrap,span-*,exec-region}, ivsc.B04/B05/B07/L01-L04, two prec rows |
| ivs_machine_load | OVERLAP | parser | cart.overlap-{exact,partial}, prec.overlap-over-digest, prec.cart-overlap-before-span |
| ivs_machine_load | DIGEST_MISMATCH | parser trailer; expect_sha256 | cart.trailer-bit-flip, {exec,grom,cart}.expect-mismatch, prec.cart-trailer-before-span |
| ivs_machine_unload | BAD_ARG | kind | unload.arg.kind-{0,4}.f, kind-0.w |
| ivs_machine_unload | BAD_HANDLE | from_handle | unload.exec.{null-machine,destroyed} |
| ivs_machine_unload | BAD_STATE | phase | unload.{exec,grom,cart}.powered.w, grom.after-power-cycle.w |
| ivs_machine_info | BAD_ARG | out NULL, short, overlap | info.{null-out,null-both,size-0,short,out-inside} |
| ivs_machine_info | BAD_HANDLE | from_handle | info.{null-handle,destroyed} |
| ivs_machine_state_digest | BAD_ARG | out NULL, overlap, domain | digest.{null-out,domain-5,domain-16,domain-99,domain-over-handle,out-inside} |
| ivs_machine_state_digest | BAD_HANDLE | from_handle | digest.{null-handle,destroyed} |

**Not reachable through the public API, so no row:**
- the parser's own BAD_ARG (`ivsc.c`: NULL view or report, or NULL bytes with a length; the load refuses those first);
- the parser's length-bound OVERSIZED (the load's bound equals the parser's maximum and comes first);
- the CPU and memsys statuses (internal);
- the internal step seam `ivs_machine_step_unit_internal` (no public declaration).

**Result:** every pair the code returns is documented and has at least one cited test that asserts that exact status. No row was added and no test was needed.

## Task Commits

1. **Task 1, tracer: the effect table and the api-effects step** (`e2b029c`, feat). Tracer gate: the step's verify and the full gate re-ran green (`steps=26/26`) before Task 2.
2. **Task 2: good-tree toy inputs and nine controls** (`7498c92`, test).
3. **Task 3: coverage audit.** No change, so no commit.

## Decisions Made

See `key-decisions` in the frontmatter.

## Deviations from Plan

**1. [Rule 2 - Missing control] Ninth token and control `api-effects-format`**
- **Found during:** Task 1.
- **Issue:** the plan names eight tokens. It leaves the shape rules without a token: an unparsable table, a row status that is not a pinned non-OK name, duplicate rows, and precedence and row consistency. A checker rule without a control is unproven.
- **Fix:** one more token and its control (the table is replaced by `x`).
- **Result:** counts are `controls=224/224`, not 221 (215 real + 8 = 223, + 1). `grep -c -E 'control ok +api-'` prints 9, not 8.
- **Commits:** `e2b029c` (token) and `7498c92` (control).

**2. [Rule 3 - Blocking] `sources-no-files` control updated**
- **Found during:** Task 2.
- **Issue:** the control deletes every `data/**/*.json` of the good tree to prove the step fails with no files. The new good-tree `data/api/api-effects.json` broke that premise (`control-token-missing`).
- **Fix:** the control's MAKE also deletes `data/api/api-effects.json`. The token and its meaning are unchanged.
- **Commit:** `7498c92`.

**3. [Counts] Plan text against reality**
- **Controls:** the plan expects `controls=221/221` and `faults=113/113`. Reality is `controls=224/224` (see 1) and `faults=115/115`, unchanged by this plan; plan 03-12 added two.
- **Steps:** `steps=26/26` as planned.

**4. [Shape] Precedence entries are objects**
- **What:** each entry is `{status, checks, pinned_by}`, not a bare status string, and each call has a `notes` array.
- **Why:** the A-08 notes and the pinning rows then sit next to the order they explain, and the step checks the `pinned_by` ids against the inventories.

**5. [Scope] The step does not use FILELIST**
- **What:** it globs `core/include/ivs/*.h` and `data/expected/tests/*.ids` under ROOT, so the plan's verify command (`FILELIST=/dev/null`) works.
- **Effect:** the control runner's tree and the real tree use the same path. An untracked header in `core/include/ivs` would be checked too, which fails closed.

---

**Total deviations:** 5. One added control (1), one control fix (2) and three notes (3 to 5). No existing token, test id or step changed meaning.

## Issues Encountered

None beyond deviation 2.

## Open items

- **OQ-5 residual (kept as the default).** The step proves that each cited test id exists in an inventory, and `tests-inventory` proves it ran in both presets. It does not prove that the case asserts the row's status. The table's `note` and the checker header say so.
  - The status match was checked once in this plan by a scratch audit, not in the gate.
  - A future drift, such as a refusal row's test changed to assert another status, would not fail `api-effects`. It would fail only if the case itself breaks.
  - A gate-level check would need the step to parse test sources. Revisit if a drift happens.
- **Effect assertions differ per cited case.** Every row has at least one cited case that checks effect none (whole-storage memcmp, digest where the handle is valid, or out bytes intact). Not every cited case checks it:
  - `machine.refuse.destroy.null` and `.twice` check only the status; `.wrong-magic` checks the storage;
  - `machine.prec.*` rows run through the matrix oracle, which checks it.
- **No digest-fields golden.** This is still the 03-09 open item. The effect table does not need it.
- **Hosted timing:** the orchestrator reads it from the next hosted run. The new step costs 0 s locally.
- **Machine-suite cost (from 03-12):** unchanged by this plan. Watch it at 03-14.

## Known Stubs

None.

## Threat Flags

No new surface. The plan's register is covered:
- **T-03-46:** `api-status-renumbered`, `-unpinned` and `-reserved`, each with a control.
- **T-03-47:** `api-dead-status`, `api-refusal-no-test` and `api-effect-unregistered`, plus the Task 3 audit.
- **T-03-48:** `api-call-undocumented` and `api-call-undeclared`.

## User Setup Required

None.

## Next Phase Readiness

- **Plan 03-14:** the effect table and its step are live. A new status or public function in Phase 4 (`ivs_machine_run`, input and A/V buffers) fails the gate until it gets a pin line and a table row with tests.
- **Final gate:**
  - `sh tools/gate.sh`: `gate: PASS steps=26/26 controls=224/224 api-calls=10 api-statuses=14 api-rows=27 cc=AppleClang c-presets=2 fault-runs=115 ... faults=115/115 ... tests-expected=2094 tests-executed=2094 tests-skipped=0 tests-missing=0 ...` (`gate-time total=25`)
  - `dash tools/gate.sh`: the identical summary line (`gate-time total=26`).

## Self-Check: PASSED

- **Files found:**
  - `data/api/api-effects.json`
  - `tools/check/api-effects.cmake`
  - `tools/check/steps/api-effects.sh`
  - `data/expected/controls/api-effects.ids`
  - `tools/check/controls/_good/tree/core/include/ivs/ivs_status.h`
- **Commits:** `e2b029c` and `7498c92` are ancestors of HEAD. The ledger count from `d667a5d` is 2, and each message ends with the Co-Authored-By line.
- **Gate re-run with this SUMMARY in the tree:** sh and dash both end `gate: PASS steps=26/26 controls=224/224 ... faults=115/115 ... tests-missing=0` (`gate-time total=25` each).
- **Hygiene:**
  - No absolute home path and no "Intellivision" in any added line.
  - No rights-bearing bytes.
  - No code, tables or comments were copied from another emulator or assembler.
  - `.planning/config.json` and `.planning/state.json` are still ` M` and in no commit of this plan.
