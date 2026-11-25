---
phase: 11-input-record-and-replay
status: passed
score: 5/5 success criteria met (1 on recorded terms, accepted as an override)
verified: 2026-11-25T15:52:00Z
verifier: gsd-verifier (Claude)
overrides_applied: 1
overrides:
  - must_have: "its grammar has no field able to hold a content digest, image name, length, frame byte, audio byte or path"
    reason: "No field is designated for content and the writer emits only the seven table fields, but the two slug fields (scenario 1..48, profile 1..64 over [a-z0-9-]) can literally spell a 32-hex run, which the oracle's sentence 'no 32-character hex sequence ... is expressible anywhere in the file' denies. Measured: a scenario whose id is a 32-hex string records a log carrying one 32-hex run. Accepted because a slug holds the operator's own scenario id (already present in their untracked scenario file), never a value the images produced, so D15's 'the log holds nothing the images produced' still holds; the rights check masks the two fields by rule and pins their shape (A-11-05, runner.record.no-content-digest assertion 4). Recorded as Open item O-11-V1 for an appended amendment."
    accepted_by: "gsd-verifier, under the owner's autonomous-operation delegation (.planning/AUTONOMY.md, 2026-11-09)"
    accepted_at: "2026-11-25T15:52:00Z"
behavior_unverified: 0
covered_files: [".planning/phases/11-input-record-and-replay/11-01-PLAN.md", ".planning/phases/11-input-record-and-replay/11-01-SUMMARY.md", ".planning/phases/11-input-record-and-replay/11-02-PLAN.md", ".planning/phases/11-input-record-and-replay/11-02-SUMMARY.md", ".planning/phases/11-input-record-and-replay/11-03-PLAN.md", ".planning/phases/11-input-record-and-replay/11-03-SUMMARY.md", ".planning/phases/11-input-record-and-replay/11-04-PLAN.md", ".planning/phases/11-input-record-and-replay/11-04-SUMMARY.md", ".planning/phases/11-input-record-and-replay/11-05-PLAN.md", ".planning/phases/11-input-record-and-replay/11-05-SUMMARY.md", "data/expected/faults/runner.ids", "data/expected/important-gates.ids", "data/expected/tests/runner-process.ids", "data/expected/tests/runner.ids", "docs/formats/ivsl-v1.md", "docs/runner.md", "runner/CMakeLists.txt", "runner/exec.c", "runner/ivs_runner.h", "runner/ivsl.c", "runner/ivsl.h", "runner/main.c", "runner/report.c", "tests/runner-process-tap.sh", "tests/runner/runner_exec.c", "tools/check/controls/_good/tree/.planning/REQUIREMENTS.md", "tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md", "tools/check/controls/_good/tree/data/expected/important-gates.ids", "tools/check/controls/important-gates-bad-req-phase5/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-bad-req-phase6/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md", "tools/check/controls/important-gates-req-uncovered-phase5/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-req-uncovered-phase6/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-req-uncovered-phase7/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-uncovered-gate/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-unknown-id-phase5/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-unknown-id-phase6/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-unknown-id/overlay/data/expected/important-gates.ids", "tools/check/steps/important-gates.sh"]
covered_digest: "v4:sha256:4aed21098af933424078f30c232a822310872b2efaa6a6a5a925a9b6f4672dc7"
---

# Phase 11: Input Record and Replay Verification Report

**Phase Goal:** A run is recorded as a rights-clean, input-only log, and a replay of
that log reproduces the recorded run's digest chain and applied-event chain — with
the vacuity guards that v0.1.0's round-trip hook lacked.
**Verified:** 2026-11-25T15:52:00Z, on branch `gsd/phase-11-input-record-and-replay`
**Status:** passed
**Tier ceiling:** `deterministic-repeatability` — the replay runs the same core, so it
is never an independent comparison. Nothing in this report raises that ceiling.
**Re-verification:** No — initial verification (no previous `11-VERIFICATION.md`).

Every figure below was measured by this verifier on this tree. No SUMMARY claim was
taken as evidence; each is cited only where it agreed with a measurement, and the two
places where the record and the tree differ are in "Deviations accepted".

## Gate, re-run on the final tree

Three shells, all on this tree after the probe/restore cycle (`git status --porcelain`
= ` M .planning/state.json` only, the orchestrator's file; `git diff` empty):

```
sh    tools/gate.sh  -> exit 0, gate-time total=76, 0 '-- FAIL' lines
      gate: PASS steps=38/38 controls=395/395 faults=237/237 fault-runs=237 gates=110
      lines=337 tests-expected=4988 tests-executed=4984 tests-skipped=4 tests-missing=0
      runner-files=18 docs=64 doc-files=63 cases=13 goldens=29 claims=28 provisional=27
      api-calls=19 public-symbols=19 pinned-defines=120 files=2647 tracked=2647
      hygiene-files=2647 mode=tree
dash  tools/gate.sh  -> exit 0, 0 '-- FAIL' lines, summary line byte-identical to sh's
linux sh tools/gate-linux.sh -> exit 0, gate-time total=346, 0 '-- FAIL' lines
      gate: PASS ... (Ubuntu 24.04, gcc 13.3.0, cmake 3.28.3; host-dependent tokens as designed)
```

The orchestrator's measured token string is reproduced token for token.

## Success Criteria

| # | Criterion (ROADMAP, frozen) | Verdict | Evidence |
|---|---|---|---|
| 1 | `docs/formats/ivsl-v1.md` committed before any code, ancestry proved; grammar has no field able to hold a content digest, image name, length, frame byte, audio byte or path; an unknown directive is refused with a typed token | **met on recorded terms** (override, see Open item O-11-V1) | Ordering: `git log --diff-filter=A -1 --format=%H -- docs/formats/ivsl-v1.md` = `dcfcd65b6861…` (08:53:42); `… -- runner/ivsl.c` = `50148e7166d8…` (09:16:24); `git merge-base --is-ancestor dcfcd65 50148e7` → exit 0, different commits. Fields: the table has exactly seven (version, scenario, profile, content, at_tick, c0, c1); the parser enforces them — `ivs_ivsl_slug` accepts only `[a-z0-9-]` (so `/`, `.`, `:`, `=` and `_` are inexpressible), `ivs_ivsl_dec` 1..19 digits with no sign/`0x`/leading zero, `ivs_ivsl_hex8` exactly eight upper-case hex digits; `ivs_ivsl_format` refuses any other name/content and writes only those seven fields. Typed token: `runner.record.refuse.unknown-directive` = `ok 204`, three trigger rows, each requiring `ivs_ivsl_reason_name(err.reason)` == the document's spelling `unknown-directive` (`IVS_IVSL_R_UNKNOWN_DIRECTIVE`). **Measured limit:** a `scenario` id that is a 32-hex string is a legal slug and the runner records it verbatim — `scenario 832906ce775d32a98ae607386ce4bbcd` in a produced log, 1 run of 32+ hex digits; see Open item O-11-V1 |
| 2 | `runner.record.replay-equals-straight` and `runner.record.format-roundtrip` pass in process: a recorded log replays to the straight run's nine digest lines and applied-event chain, and write, parse, write again is byte-identical | **met** | `./build/dev/tests/ivs_tests_runner`: `ok 202 runner.record.replay-equals-straight`, `ok 203 runner.record.format-roundtrip`, `# suite=runner executed=261 skipped=0 failed=0`. Round trip = five assertions (byte-identical second write with equal length; format/parse fixpoint over events, ticks, both words, names, content class; byte figures computed from the strings; `IVS_IVSL_MAX_EVENTS == 64u == IVS_SCN_MAX_INPUTS`; a refusal leaves the caller's buffer untouched). **Non-vacuity, probed:** the replay path was made to parse the log and ignore it (`runner/exec.c`: the `log_in` branch never taken) → `not ok 202 … # the recorded and the replayed reports: record-no-events`. So the case fails if the log is ignored, which is A-11-03's requirement |
| 3 | The liveness guards do the work: the equality case asserts ≥1 event, `input.consumed == input.events`, an `at_tick` list equal to the recorded one and all nine domains observed, while `replay-differs` and `replay-inert` are rejected **by the guard**, each shown able to fail | **met** | Guards are `rx_record_compare` (`tests/runner/runner_exec.c:3084`), in a fixed order: `record-no-events` (log empty or either report's `input.events` 0), `record-count-mismatch` (≠ the log's count), `record-not-consumed` (`input.consumed` ≠ its own `input.events`), then `record-differs` (whole-report `strcmp`, `host.calls` included — the log is a caller buffer, never a host call). `at_tick` list asserted at lines 3184-3196: `100`/`KEY_1`/`0` and `200`/`0`/`DISC_E`, count 2. "All nine domains observed" is in the **process** case (A-11-01) — `tests/runner-process-tap.sh:1254-1266` requires all nine `digest.*` lines present, non-`-` and equal, plus `output.video.bytes`, `output.audio.bytes`, `output.psglog.records`, `input.events` nonzero on both runs; the in-process case asserts the nine lines only, and says so in its comment (the fake host observes 6 of 9). **Probes on a scratch copy (all restored, `git diff` empty afterwards):** P1 equality disabled → `not ok 214 runner.judge.record-replay-differs` `# the one-nibble pair gave '(accepted)', want record-differs`, `ok 215` inert; P2 no-events clause disabled → `not ok 215 runner.judge.record-replay-inert` `# the identical zero-event pair gave '(accepted)', want record-no-events`, `ok 214` differs, and `not ok 202` with `record-count-mismatch`; P4 `input.consumed` printed as `consumed+1` → `not ok 202 … record-not-consumed`. Each mechanism is live and each is separable from the other |
| 4 | Every malformed log in the refusal family is refused with a typed token and no state change, and the two named faults fail exactly their ids | **met** | Nine ids, all `ok`: `runner.record.refuse.{unknown-directive,version,arity,hex,trailing,tick-order,tick-bound,event-cap,controller-bit}` (`ok 204`–`ok 212`). Each row is judged five ways by `rx_refuse`: parse refused; `err.reason` is the row's code; `ivs_ivsl_reason_name` renders the document's token (document against code, not code against a copy of itself); the 1-based line; and the `0xA5`-poisoned `ivs_ivsl_log` the caller passed is byte-identical afterwards (first differing byte printed on failure) — that is the "no state change" assertion, plus the process case's "a refusal creates no file". Four acceptance rows bound the four classes that have a legal neighbour: equal ticks accepted (2 events), `IVS_SCN_TIME_MAX-1` accepted / `IVS_SCN_TIME_MAX` and `+1` refused, 64 events accepted / 65 refused, bit 30 accepted / bit 31 refused (`rx_accept` at 3637, 3669, 3702, 3744). **Both faults probed:** `ivs_fault_RECORD_DROPS_LAST_EVENT` → exit 1, `not ok 202` (`# the log took 116 bytes, want 152`) and `not ok 203` (`# the recorded log does not parse: reason ok line 0`), 259 ok, 2 not-ok; `ivs_fault_REPLAY_APPLIES_LATE` → exit 1, `not ok 202` only (`# the recorded and the replayed reports: record-differs`), 260 ok, 1 not-ok. Both match their registry rows exactly (`runner.ids:81,84`), and neither site is outside `IVS_RUNNER_SOURCES` (`build/dev/ivs-runner-sources.txt:1,3` lists `runner/exec.c` and `runner/ivsl.c`) |
| 5 | `runner.record.no-content-digest` finds zero 32-hex runs in a produced log; the log is text, absent from `binary-ext.txt` and not gitignored; the gate ends `steps=38/38` with the predicted count delta and the tier ceiling `deterministic-repeatability`, provisional | **met** | `ok 213 runner.record.no-content-digest`, diagnostic `# no-content-digest: the produced log 152 bytes, 0 masked 32-hex runs; the stand-in 1; the 32-digit slug 1 unmasked and 0 masked`. **Independently reproduced on a real file** the runner wrote: `ivs-run --content-dir … --record-input pass.ivsl demo-lamp-tone-pass.ivss` → exit 0, log **319 bytes**, `grep -oE '[0-9A-Fa-f]{32,}' pass.ivsl \| wc -l` = **0**; the same file with the golden's `digest.input` line appended = **1** (A-11-05: without the stand-in the check cannot fail; the stand-in is a string literal in committed source, never a tracked `.ivsl`). Text/hygiene: `grep -n ivsl tools/check/binary-ext.txt` and `grep -n ivsl .gitignore` both empty; `git ls-files \| grep -Ei '\.(ivsl\|bin\|rom\|ivsc)$'` empty; `find . -name '*.ivsl'` empty after the gate, the suites and the process case. `steps=38/38` at the phase base (`git ls-tree c98db20 tools/check/steps/ \| wc -l` = 38; `git diff c98db20..HEAD -- data/expected/gate-steps/` empty) and at HEAD (gate above). Predicted delta measured exactly: `faults`/`fault-runs` 235→237, `gates` 109→110, `lines` 333→337, `steps` unchanged, `claims=28 provisional=27` unchanged. TOOL-01's close: see the Requirements table |

## Requirements

| Requirement | Source plan | Description | Status | Evidence |
|---|---|---|---|---|
| TOOL-01 | 11-01…11-05 | A run can be recorded as an input-only log and replayed at machine-time boundaries, and the replay reproduces the recorded run's digest chain and applied-event chain, guarded against a vacuous match | **Complete, and the flip is one commit** | `git show --stat 16afb31` = **16 files**, carrying the checkbox flip (`- [ ] **TOOL-01**` → `- [x]`), the Traceability row (`\| TOOL-01 \| Phase 11 \| Pending \|` → `Complete`), four `runner-record` registry rows, the R2 header retarget, ten toy registry lines (`grep -rl '^toy-tool01 fault GOOD_FAULT TOOL-01$' tools/check/controls/ \| wc -l` = 10) and three toy requirement rows (= 3). Identity recomputed with the gate's own awk: `complete=58 excluded=12 accepted=46 registered=46`, `58 - 12 = 46`, `diff accepted registered` **empty**, `TOOL-01` in accepted (1) and in the exclusion ledger (0), `-- important-gates pass gates=110 lines=337` |

No orphaned requirement: `grep -E "Phase 11" .planning/REQUIREMENTS.md` names only TOOL-01, and every plan's `requirements` field names TOOL-01.

## What was automated / handed off / could not be judged by a tool

GUARDRAILS §5.6.

**Automated (this verifier ran it, on this tree).** The three gate shapes (`sh`, `dash`,
`gate-linux` in an `ivs-` container, all `gate: PASS`, 0 `-- FAIL`); the in-process
suite (261 ids) and the process suite (133 ids, 2 skipped by design); the two fault
executables and the exact set of ids each one fails; four scratch probes that each
disabled one mechanism (equality, no-events guard, `consumed == events`, the log
substitution) and each produced the finding it names, with the tree restored and
`git diff` empty afterwards; a real record/replay from the command line; an
independent 32-hex scan on the produced file; the ancestry proof; the requirement
identity arithmetic; the hygiene, `binary-ext`, `.gitignore` and `git ls-files`
checks; the registry/allowlist diffs (`api-effects.json`, `core/`, `data/golden/`,
`claims.json`, `pkg-install-files.ids`, `data/expected/gate-steps/` all unchanged).

**Handed off: nothing.** No item was deferred to a human or to a later run. The
container gate, which plan 11-05 recorded as a visible `not run`, was run here by the
orchestrator's boundary and passed (`exit 0`).

**Could not be judged by a tool: nothing in the criteria.** The two judgement calls
this report makes are stated rather than hidden: whether the drop fault's failure of
`format-roundtrip` is honest (yes — see "Deviations accepted") and whether the two slug
fields defeat criterion 1's field-absence clause (they narrow it — Open item O-11-V1).
Both are decided on measurement, not on taste.

## Deviations accepted

1. **`IVS_FAULT_RECORD_DROPS_LAST_EVENT` also fails `runner.record.format-roundtrip`,
   and the registry says so.** Judged honest and correctly recorded. The fault sits in
   `ivs_ivsl_format` itself, so a formatted-then-reformatted log drops a second time and
   format is not a fixpoint under it; no edit to the round-trip id can change that
   without making the id accept a zero-event log. The allowed column therefore names
   `runner.record.replay-equals-straight|runner.record.format-roundtrip` — two **exact**
   ids, never a `runner.record.` prefix, so nothing is hidden from the id that must
   catch the fault. Measured: 2 not-ok ids, both named. `11-05-SUMMARY.md` records the
   two routes it considered and why the second was taken; the registry comment above
   the row states it in `faults-verdict`'s own terms. The round-trip id remains green
   in every normal build and remains able to fail, so nothing was weakened.
2. **The no-log witness of the stripped copy ends `guest-fail 10`, not `pass`.**
   Recorded as plan 11-04's deviation 1 and judged correct: the stripped
   `demo-lamp-tone-pass` supplies no key, so the lamp-tone program's own assert fails
   (`assert=4 line=27 mem 02F2 == 007E got 00FF`). The witness exists to show the report
   *differs*, and it does; the vacuity argument is unaffected.
3. **`arity` and `trailing` are classes, not single triggers, and the family has nine
   tokens, not seven (A-11-02).** Judged correct: both tokens enumerate their members
   in the document and in the cases (`arity` 8 line shapes, `trailing` 3 file shapes),
   and neither is a bucket — no value another token owns is fed to them. The count of
   seven was an estimate in the frozen prose; the property the criterion is for (a
   closed family, each with a distinct typed token) is what was measured.
4. **Criterion 1's field-absence clause, on the recorded terms stated in the override
   above.** The clause is met for `image name` and `path` (inexpressible: the slug
   charset has no `/`, `.`, `:`), and for every content-bearing field in the sense that
   no field is designated for content and the writer emits only the seven table fields.
   It is **not** met literally for `content digest` (a 32-hex run fits the slug fields —
   measured) and cannot be met literally for `length`/`frame byte`/`audio byte` in any
   numeric format (a 19-digit tick and an 8-hex-digit word can spell any such number).
   The rights property the clause serves still holds: the runner writes the operator's
   own scenario id, never a value the images produced, so the log is not derived
   material under D-11.

## Open items

- **O-11-V1 (documentation, not a code defect).** `docs/formats/ivsl-v1.md` line 82-83
  states: *"A name is at most 48 or 64 characters and a tick at most 19 digits, so no
  32-character hex sequence and no path is expressible anywhere in the file (D-15)."*
  Measured false for the two name fields: `scenario 832906ce775d32a98ae607386ce4bbcd`
  is a legal slug and the runner records it, giving a produced log with one 32-hex run.
  `runner.record.no-content-digest` masks exactly those two fields (by rule, shape-pinned
  — assertion 4 of that id), so it reports 0 on such a log. Recommended remedy is an
  **appended** amendment (GUARDRAILS 2.10 — never a rewrite) narrowing the sentence to
  the fields where it is true and naming the mask as the reason the two names are
  excluded; no format, code or test change follows from it, and no version bump is due
  because no field changes. Recorded here so it is not re-litigated from scratch.
- **Carried from the phase records, not re-verified as work:** the five deferred items
  in `.planning/STATE.md` (the `scenario`/`profile` mismatch refusals O-1/O-2, a
  content-identity field under D-15, promoting the process case to a real-executable
  gate case, the snapshot file options, and a new gate step for record/replay
  evidence). None is a criterion, none is due in Phase 11, and the frozen reversal
  trigger for the third is "a defect found only at the process boundary".

## VERIFICATION: passed
