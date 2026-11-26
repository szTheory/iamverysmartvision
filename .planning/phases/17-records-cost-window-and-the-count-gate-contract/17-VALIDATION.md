---
phase: "17"
slug: "records-cost-window-and-the-count-gate-contract"
status: validated
nyquist_compliant: true
wave_0_complete: true
created: "2026-11-26"
---

# Phase 17 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

**Audited 2026-11-26 against the tree at `f84d702`** (the code-review disposition commit), after the plans were
revised twice and the review fix pass landed. Every figure below was produced by a command in `## The commands,
verbatim` at that commit; nothing is carried over from the seeded draft or from a plan's prediction.

What the audit changed, and why:

1. **The task rows were re-derived, and their ids restated as `NN-TN`.** The seeded rows were numbered by
   behaviour rather than by task, and four of the seven were out of order against the plans as executed: the
   seeded `17-01-02` named the `ci-log` count (plan 01's *first* task), `17-02-02` named the join id (plan 02's
   *third* task), `17-02-03` named the source-archive letter (its *second*), and `17-03-01`/`17-03-02` both
   named plan 03's *second* task. The `NN-TN` shape is the one phase 16's validated record uses and it makes the
   plan/task mapping explicit. Seven tasks, seven rows.
2. **Both Wave 0 references are satisfied**, not MISSING: `runner-process.judge.flatwrap-shim` landed in
   `be69a5e` and `count-gate mutant extra-file EV-08` in `bfa0815`.
3. **Both declared manual-only rows were promoted to automated**, because a deterministic command settles each.
   They are recorded under `## Manual-Only Verifications` with the command that proves them and with the residual
   that no tool judges, so nothing is lost by promoting them.
4. **No `data/expected/handoffs.ids` row was added.** That file still holds its eight rows (`handoffs=8` on the
   gate line, unchanged by this phase). The project forbids adding a handoff row for something a tool can check,
   and after the promotions nothing in this phase needs one.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | The project's own gate: 38 POSIX `sh` steps, the CMake `-P` checkers they call, TAP suites under CTest in two presets (`dev`, `release`), and 404 negative-control trees. No external test framework. |
| **Config file** | `tests/CMakeLists.txt` (suite registration), `tools/gate.sh` (step order), `data/expected/tests/*.ids` (id inventories), `data/expected/gate-steps/*.ids` |
| **Quick run command** | `sh tools/gate.sh > /tmp/v17-sh.log 2>&1; tail -1 /tmp/v17-sh.log` and the same with `dash` |
| **Full suite command** | `sh tools/gate-linux.sh > /dev/null 2>&1; grep -q '^gate: PASS' build/gate-linux.log` |
| **Measured runtime** | `sh` **70.42 s** and `dash` **81.57 s** wall, one timed run each on a warm tree at `f84d702` (`/usr/bin/time -p`). The container gate is the long pole; its own `gate-time total` line is recorded in `build/gate-linux.log` and is not re-quoted here. |

---

## Sampling Rate

- **After every task commit:** the quick run command, under both `sh` and `dash` — one summary line, `cmp`-ed.
- **After every plan wave:** the full suite command, one container gate at a time.
- **Before `/gsd:verify-work`:** full suite green, `build/gate-linux.log` copied aside before anything else runs.
- **Max feedback latency:** budget 200 s; measured 70–82 s for the whole gate on this host.

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 17-01-T1 | 01 | 1 | EV-08 (c1) | T-17-01, T-17-02, T-17-03 | the window is re-derivable from the named ledger by the printed recipe, and the seven release rows are counted rather than re-appended | record, recomputed | `V1` — six record lines recomputed from `.planning/ci-runs.tsv` and matched whole-line against `17-01-SUMMARY.md` | ✅ | ✅ green |
| 17-01-T2 | 01 | 1 | EV-08 (c1) | T-17-04, T-17-05 | one verdict line, written after the priced addition, and both shells produce one identical summary | integration (step + record) | `V3` — `sh`/`dash` summary lines `cmp`-identical, zero `-- FAIL ` lines, exactly one `checkpoint: reprice` | ✅ | ✅ green |
| 17-02-T1 | 02 | 2 | EV-08 (c2) | T-17-09 | the documented token set is mechanically the append-only inventory, not a glossary that drifts from it | unit (doc `diff`) | `V4` — eight line-initial tokens on each side, `diff` silent | ✅ | ✅ green |
| 17-02-T2 | 02 | 2 | EV-08 (c4) | T-17-08 | the two live documents agree on the release letter, and the archive is amended rather than rewritten | unit (doc count) | `V5` — the canonical sentence occurs exactly 2× in `docs/packaging.md` and 1× in `RELEASE-OPS.md` §(i) | ✅ | ✅ green |
| 17-02-T3 | 02 | 2 | EV-08 (c3) | T-17-06, T-17-07 | the join id is `ok` in both presets and never a skip, and its registry row resolves through the step | integration (CTest TAP) | `V6` — `ok <n> runner-process.judge.flatwrap-shim` in both presets' TAP files, the `cart-role` row present, `important-gates pass gates=120 lines=366` | ✅ | ✅ green |
| 17-03-T1 | 03 | 3 | EV-08 (c5) | T-17-12, T-17-15 | the lesson's three clauses are their own lines, the last numbered lesson is 32, and the four invariants recompute from the tree | unit (doc) + step | `V7` — three clause lines verbatim, last numbered lesson `32.`, the `invariants:` line equal to the recomputed quadruple, `INDEX.md` at `1–32`, `runner-selftest`'s own refusal proof | ✅ | ✅ green |
| 17-03-T2 | 03 | 3 | EV-08 (c5) | T-17-13, T-17-14 | the row points at a case the step actually runs, and that case's committed stand-in is refused on every gate run | unit (step) | `V8` — `count-gate mutant extra-file EV-08` present, `extra-file` in `pkg-selftests.ids`, and the gate line carrying `gates=120 lines=366` with `pkg-selftest=20/20` | ✅ | ✅ green |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

Every row was run in this audit at `f84d702` and exited 0. Sampling continuity holds: all seven tasks carry an
automated command, and no three consecutive tasks lack one.

---

## The commands, verbatim

Each block is self-contained and exits non-zero on a mismatch. `V8` reads the summary line `V3` wrote, so a
reader runs `V3` first.

### V1 — the recomputed window, against the record (17-01-T1)

```sh
set -e
S=.planning/phases/17-records-cost-window-and-the-count-gate-contract/17-01-SUMMARY.md
d=$(mktemp); p=$(mktemp); r=$(mktemp); e=$(mktemp); trap 'rm -f "$d" "$p" "$r" "$e"' EXIT
git diff --quiet 21d023c -- .planning/ci-runs.tsv      # the ledger is unchanged since the record's start-commit
awk -F'\t' 'NR>1' .planning/ci-runs.tsv > "$d"
med() { awk -F'\t' -v c="$1" 'NR>1 { print $c }' .planning/ci-runs.tsv | tail -5 | sort -n | sed -n 3p; }
f=$(awk -F'\t' 'NR>1 { n++; if ($2 == "45835631107") f = n } END { print f }' .planning/ci-runs.tsv)
sed -n "$((f - 5)),$((f - 1))p" "$d" > "$p"
pmed() { awk -F'\t' -v c="$1" '{ print $c }' "$p" | sort -n | sed -n 3p; }
printf 'window: cpu_s=%s build_s=%s gate_s=%s\n' "$(med 13)" "$(med 8)" "$(med 10)" > "$e"
printf 'previous-window: cpu_s=%s build_s=%s gate_s=%s\n' "$(pmed 13)" "$(pmed 8)" "$(pmed 10)" >> "$e"
printf 'rows: %s lines: %s\n' \
  "$(awk 'END { print NR + 0 }' "$d")" "$(awk 'END { print NR + 0 }' .planning/ci-runs.tsv)" >> "$e"
rel=
for id in 45835631107 45836207855 45839153455 45839646456 45841732895 45842795129 45843408874; do
  n=$(awk -F'\t' -v i="$id" 'NR>1 && $2 == i' .planning/ci-runs.tsv | awk 'END { print NR + 0 }')
  rel="$rel $id=$n"
  awk -F'\t' -v i="$id" 'NR>1 && $2 == i' .planning/ci-runs.tsv >> "$r"
done
printf 'release-rows:%s\n' "$rel" >> "$e"
printf 'release-build_s-range: %s to %s\n' \
  "$(awk -F'\t' '{ print $8 }' "$r" | sort -n | sed -n 1p)" \
  "$(awk -F'\t' '{ print $8 }' "$r" | sort -n | sed -n 7p)" >> "$e"
printf 'r-range: %s to %s\n' \
  "$(tail -5 "$d" | awk -F'\t' '{ printf "%.4f\n", $13 / $8 }' | sort -n | sed -n 1p)" \
  "$(tail -5 "$d" | awk -F'\t' '{ printf "%.4f\n", $13 / $8 }' | sort -n | sed -n 5p)" >> "$e"
cat "$e"
test "$(grep -c -x -F -f "$e" "$S")" -eq 6
```

Observed: `window: cpu_s=572 build_s=49 gate_s=164`, `previous-window: cpu_s=367 build_s=34 gate_s=115`,
`rows: 118 lines: 119`, the seven `release-rows: <id>=1` pairs, `release-build_s-range: 34 to 51`,
`r-range: 10.5143 to 11.6735` — all six present in the record, so `6`.

### V2 — the independent count from the same named file (17-01-T1)

```sh
git ls-files > /tmp/v17-fl
ROOT="$PWD" CHECK_DIR="$PWD/tools/check" FILELIST=/tmp/v17-fl sh tools/check/steps/ci-log.sh \
  | grep -qxF -e '-- ci-log pass ci-runs=118'
```

The `ci-log` step names the same file the record counts, so a record that counted a different file disagrees
with a committed check rather than only with itself.

### V3 — one verdict line and one summary line under both shells (17-01-T2)

```sh
set -e
S=.planning/phases/17-records-cost-window-and-the-count-gate-contract/17-01-SUMMARY.md
sh tools/gate.sh > /tmp/v17-sh.log 2>&1
dash tools/gate.sh > /tmp/v17-dash.log 2>&1
tail -1 /tmp/v17-sh.log > /tmp/v17-sh.sum
tail -1 /tmp/v17-dash.log > /tmp/v17-dash.sum
cmp /tmp/v17-sh.sum /tmp/v17-dash.sum
grep -qE '^gate: PASS ' /tmp/v17-sh.sum
test "$(grep -c -e '^-- FAIL ' /tmp/v17-sh.log)" -eq 0
test "$(grep -c -e '^-- FAIL ' /tmp/v17-dash.log)" -eq 0
test "$(grep -c -e '^checkpoint: ' "$S")" -eq 1
grep -qxF -e 'checkpoint: reprice' "$S"
```

### V4 — the documented tokens against the append-only inventory (17-02-T1)

```sh
set -e
grep '^flatwrap-' data/expected/flat-refusals.ids > /tmp/v17-a
grep '^flatwrap-' docs/formats/ivsc-v1.md > /tmp/v17-b
test "$(grep -c . /tmp/v17-a)" -eq 8
test "$(grep -c . /tmp/v17-b)" -eq 8
diff /tmp/v17-a /tmp/v17-b
```

Both counts are asserted before the `diff` is allowed to be silent, so an empty page grep cannot pass as
agreement.

### V5 — the canonical release letter in both live documents (17-02-T2)

```sh
set -e
C='the release ships the static package and the source archive, which carries the source of the build-only tools and of the build-only host shim; no shared object, no built host shim'
test "$(awk -v c="$C" 'index($0, c) { n++ } END { print n + 0 }' docs/packaging.md)" -eq 2
sed -n '/^## (i) /,/^### (i\.1)/p' .planning/ops/RELEASE-OPS.md > /tmp/v17-ro.i
test "$(awk -v c="$C" 'index($0, c) { n++ } END { print n + 0 }' /tmp/v17-ro.i)" -eq 1
```

The `RELEASE-OPS.md` count is scoped to section (i)'s own slice, because the byte-identical `(i.1)` append
already quotes the superseded sentence and an unscoped grep would be satisfied by text this phase may not touch.

### V6 — the join id in both presets, and its registry row (17-02-T3)

```sh
set -e
git ls-files > /tmp/v17-fl
grep -qE '^ok [0-9]+ runner-process\.judge\.flatwrap-shim$' build/gate/dev/runner-process.txt
grep -qE '^ok [0-9]+ runner-process\.judge\.flatwrap-shim$' build/gate/release/runner-process.txt
grep -qxF -e 'cart-role mutant runner-process.judge.flatwrap-shim EV-08' data/expected/important-gates.ids
ROOT="$PWD" CHECK_DIR="$PWD/tools/check" FILELIST=/tmp/v17-fl sh tools/check/steps/important-gates.sh > /tmp/v17-ig.out 2>&1
grep -qxF -e '-- important-gates pass gates=120 lines=366' /tmp/v17-ig.out
test "$(grep -c -e '^-- FAIL important-gates-' /tmp/v17-ig.out)" -eq 0
grep -qF -e 'tests-expected=5044 tests-executed=5040' /tmp/v17-sh.sum
```

`build/gate/<preset>/runner-process.txt` is the TAP result file the `c-tests` step writes, so the two `ok` lines
are the gate's own record of both presets rather than a separate invocation.

### V7 — lesson 32, its invariants and the tail-clause proof (17-03-T1)

```sh
set -e
git ls-files > /tmp/v17-fl
L=.planning/ops/PLAN-LESSONS.md
grep -qxF -e '  - **1.** Every count gate names the file it counts.' "$L"
grep -qxF -e "  - **2.** Every baseline names the plan's own start commit sha and is measured there." "$L"
grep -qxF -e '  - **3.** No verify block decides an outcome by a pipeline tail.' "$L"
test "$(awk '/^[0-9]+\. / { last = $0 } END { print substr(last, 1, 3) }' "$L")" = '32.'
faults=$(cat data/expected/faults/*.ids | grep -v '^#' | awk 'NF' | wc -l | tr -d ' ')
goldens=$(ls data/golden/ | wc -l | tr -d ' ')
set -- $(python3 -c "import json;c=json.load(open('data/claims.json'))['claims'];print(len(c),sum(1 for x in c if x['standing']=='provisional'))")
printf 'invariants: faults=%s goldens=%s claims=%s provisional=%s\n' "$faults" "$goldens" "$1" "$2" > /tmp/v17-inv
cat /tmp/v17-inv
grep -qxF -f /tmp/v17-inv "$L"
grep -qF -e 'Numbered planning lessons 1–32' .planning/ops/INDEX.md
test "$(grep -c -F -e 'Numbered planning lessons 1–25' .planning/ops/INDEX.md)" -eq 0
ROOT="$PWD" CHECK_DIR="$PWD/tools/check" FILELIST=/tmp/v17-fl sh tools/check/steps/runner-selftest.sh > /tmp/v17-rs.out 2>&1
grep -qF -e '-- runner-selftest proof control-pass-line-not-last: profile (observed:' /tmp/v17-rs.out
test "$(grep -c -e '^-- FAIL runner-selftest' /tmp/v17-rs.out)" -eq 0
```

The four invariants are recomputed from the tree and then compared as one whole line against the lesson, so a
lesson that quoted a stale figure fails. `invariants: faults=238 goldens=29 claims=29 provisional=28`.

### V8 — the count-gate row, and the invariant tokens on the gate line (17-03-T2)

```sh
set -e
sed -n 's/^gate: PASS //p' /tmp/v17-sh.sum | tr ' ' '\n' > /tmp/v17.tokens
: > /tmp/v17.miss
for t in steps=38/38 faults=238/238 goldens=29 claims=29 provisional=28 prefix-files=11 \
         tests-expected=5044 tests-executed=5040 gates=120 lines=366 pkg-selftest=20/20; do
  grep -qxF -e "$t" /tmp/v17.tokens || printf 'moved: %s\n' "$t" >> /tmp/v17.miss
done
test ! -s /tmp/v17.miss
grep -qxF -e 'count-gate mutant extra-file EV-08' data/expected/important-gates.ids
grep -qxF -e 'extra-file' data/expected/pkg-selftests.ids
grep -qF -e 'count-gate' tools/check/steps/important-gates.sh
```

`pkg-selftest=20/20` is the `pkg-install` step's count of in-step cases that failed as intended, `extra-file`
among them — the committed stand-in the `count-gate` row points at, refused on every gate run rather than once.

---

## Wave 0 Requirements

Both seeded Wave 0 references are **satisfied** at `f84d702`; neither is MISSING.

- [x] `runner-process.judge.flatwrap-shim` — the id itself was the phase's deliverable. **Satisfied by**
  `tests/runner-process-tap.sh`'s five-limb case with its committed must-fail stand-in and the appended line in
  `data/expected/tests/runner-process.ids` (plan 17-02, `be69a5e`), `ok` in both presets (`V6`).
- [x] `count-gate mutant extra-file EV-08` — the registry row that carries lesson 32's mechanical clause.
  **Satisfied by** the row over the existing `extra-file` case of `pkg-install`, whose count comes from the named
  file `data/expected/pkg-install-files.ids` (plan 17-03, `bfa0815`); no new selftest case, no new test id and
  no new step, so `pkg-selftest=20/20` is unchanged and the phase's priced line stays 0.11 (`V8`).
- [x] Framework install: none — the harness is the project's own. **Satisfied by** the absence of any dependency
  in `tools/gate.sh`; `cmake`, `sh`, `awk`, `grep`, `git` and `python3` are the host's.

*Everything else this phase needed already existed: the suite, the two documents, the inventory and the step.*

---

## Manual-Only Verifications

**No manual-only row remains.** Two were declared when this file was seeded; both were promoted, because a
deterministic command settles each. They are recorded here with the command that proves them and with the one
residual that no tool judges, so the promotion is visible rather than silent.

| Behavior | Requirement | Settled by | Residual a tool cannot judge |
|----------|-------------|------------|------------------------------|
| The window is recomputed rather than quoted | EV-08 (c1) | `V1` — every figure the record prints is recomputed from `.planning/ci-runs.tsv` by the record's own recipe and matched whole-line, with the ledger proved unchanged since the record's `start-commit` (`21d023c`) by `git diff --quiet`; `V2`'s `ci-log` step counts the same file independently | whether the author computed the figures or typed numbers that happen to agree. That is intent, not a behaviour the requirement demands: EV-08 asks that the record's figures be re-derivable from the named file, and `V1` fails on any figure that is not. The intent is recorded in the phase record's own recipe block, which is the artifact under review |
| The packaging letter and `RELEASE-OPS.md` (i) agree sentence for sentence | EV-08 (c4) | `V5` — the canonical sentence is one fixed string, counted as exactly 2 occurrences in `docs/packaging.md` and 1 in the `RELEASE-OPS.md` §(i) slice | whether the sentence is the *right* sentence. That was decided by the owner-facing decision in the phase's discuss record and by the review's CR-01 disposition, both prose artefacts a person reads; the mechanical agreement is what a step can hold, and it does |

Neither behaviour needs a `data/expected/handoffs.ids` row, and none was added: `handoffs=8` on the gate line is
this phase's own count, unchanged, because a handoff row is for something no tool on this host can supply and
both of these are supplied by a command above.

*The container gate (`tools/gate-linux.sh`) records a visible `not run` line when docker is unavailable — never
a pass (lesson 24). It is not part of any row above, because no row in this phase depends on the platform.*

---

## Validation Sign-Off

- [x] All tasks have `<automated>` verify or Wave 0 dependencies — seven tasks, seven automated commands
- [x] Sampling continuity: no 3 consecutive tasks without automated verify
- [x] Wave 0 covers all MISSING references — both references are satisfied, neither is MISSING
- [x] No watch-mode flags
- [x] Feedback latency within the 200 s budget (measured 70.42 s under `sh`, 81.57 s under `dash`)
- [x] `nyquist_compliant: true` set in frontmatter

**Approval:** approved 2026-11-26 by the validation audit at `f84d702`. The measured closing line, quoted
verbatim, is identical under `sh` and `dash` (`cmp` on the extracted final lines):

```
gate: PASS steps=38/38 controls=404/404 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=118 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=29 provisional=28 handoffs=8 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=695 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2810 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=63 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2810 history-commits=1 gates=120 lines=366 input-rows=31 input-policies=11 input-profile-fields=31 files=2810 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=20/20 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=12/12 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=5044 tests-executed=5040 tests-skipped=4 tests-missing=0 docs=64 claim-refs=27 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

The invariants this phase was briefed to hold are all present and unmoved: `steps=38/38`, `faults=238/238`,
`goldens=29`, `claims=29`, `provisional=28`, `prefix-files=11`; the figures this phase was measured to move read
`tests-expected=5044`, `tests-executed=5040`, `gates=120`, `lines=366`. The three `git ls-files` counts on that
line — `files=2810 tracked=2810 hygiene-files=2810` — are the tracked list at this commit, read rather than
predicted.

## Validation Audit 2026-11-26

| Metric | Count |
|---|---|
| Gaps found | 7 |
| Resolved | 7 |
| Escalated | 0 |
