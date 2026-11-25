---
phase: 09-registered-evidence-and-gate-cost
reviewed: 2026-11-26T12:00:00Z
reviewer: gsd-code-reviewer
base: e172643
head: 86c7878
depth: deep
status: findings
blockers: 0
warnings: 3
infos: 6
files_reviewed: 13
files_reviewed_list:
  - tools/gate.sh
  - tools/check/run-controls.sh
  - tools/check/manifests.cmake
  - tools/check/steps/golden.sh
  - tools/check/steps/important-gates.sh
  - tools/check/steps/memmap.sh
  - tools/check/steps/runner-selftest.sh
  - tools/check/steps/sources.sh
  - tests/runner-process-tap.sh
  - tests/state/state_util.h
  - tests/state/state_util.c
  - tests/state/state_restore.c
  - tests/state/state_fuzz.c
data_files_reviewed:
  - data/expected/important-gates.ids
  - data/expected/important-gates-excluded.ids
  - data/expected/controls/{important-gates,memmap,provenance,rights}.ids
  - data/expected/runner-scenarios.ids
  - data/expected/tests/{runner-process,state}.ids
  - tools/check/controls/{_good/tree,important-gates-*,memmap-generator-refused,manifests-output-anchor,sources-src-token-whole}/**
---

# Phase 9: Code Review Report

**Reviewed:** 2026-11-26
**Depth:** deep (every changed shell, CMake and C file, plus the fixtures and inventories they read)
**Files reviewed:** 13 source files, 9 inventory/fixture groups
**Status:** findings (0 blockers, 3 warnings, 6 infos)

## Summary

The phase is in good shape on the two things it is about. The four cost trims each carry
their preserving condition **in code, not only in a comment**, and I verified each one by
reading the guard site rather than the prose: the whole-instance `memcmp` and the `produced`
accumulation sit outside the digest branch (`tests/state/state_util.c:479`, `:496` vs the
branch at `:487`); the fuzz offset loop bound, `fz_pick` and `masks[]` are untouched and only
`j == 1` of the unsealed path is skipped (`tests/state/state_fuzz.c:931-933`); `release-only`
prints an ordinary `ok` line so `tests-skipped` stays 4; the `zero` round-trip launch is still
the one live launch. The lever (3) derivation judgement is real, not decorative: I ran
`derivation_equal` and its in-case negative half standalone and both behave as claimed, and the
negative half fires `variant-derivation-differs` for the one-value stand-in.

The preflight partition preserves evidence. I ran the gate under both `sh` and `dash` on this
tree and diffed the normalised streams against the two committed pre-change captures: the
**only** difference is the two new partition lines. `keep_fails` runs during the replay, the
buffers are per-step filenames, and nothing else is emitted outside the replay. `grep -c
'^-- FAIL '` is 0 in both logs, so the phase's own post-hoc fix (`d3ff4bd`) for the
"advertised failure in a passing run" class is effective, and the new proof lines use the
`-- runner-selftest proof ` prefix precisely so `keep_fails` and `ci-summary.sh` (both anchored
`^-- FAIL `) cannot collect them.

The defects I found are not in the mechanisms the phase advertises but at their edges, and all
three warnings are the same shape the phase exists to remove: **a documented invariant with no
machine check**. The most consequential is the exclusion ledger, which is the single escape
hatch of the EV-04 check and whose own header declares a row format that no code enforces — a
bare `API-02` line silences a Complete requirement and the step prints `pass` (probe B below).
The second is the per-step pass-line guard, which is enforced only for the 33 steps that have a
control and therefore not for the five producers. The third is the `release-only` branch of the
golden step, which narrows the evidence for those two ids to one report and then refuses nothing
if that report is missing.

No blocker: I could not construct a state in which the gate passes while a *mechanism the phase
adds* is silently absent. Every "silent no-op" path I found is either closed by another check
that turns the gate red (the good-tree registry is guarded by the `_good` positive control; a
no-op memmap generator breaks the C build because `tests/memmap/memmap_rows.c:71` includes the
generated `.inc`) or requires an edit that is itself the evidence of the defect (a malformed
ledger row).

## Findings

### WR-01: the exclusion ledger's declared row format is enforced by nothing, and it is the only way to silence the EV-04 check

**File:** `tools/check/steps/important-gates.sh:90-93`, `data/expected/important-gates-excluded.ids:1-25`

**What is wrong.** The derived must-be-covered set is `Complete - excluded`, and the exclusion
set is built by

```sh
awk '!/^#/ && NF { print $1 }' "$exp/important-gates-excluded.ids" > "$tmp/excluded"
```

`$1` is taken whatever the rest of the line holds. The ledger's own header states three
invariants — "One line per exclusion: '<REQ> <reason>'", "the reason is words joined by
underscores (at least three)", "A reason of 'none' is not legal" — and none of them is checked
anywhere in the tree.

**Why it matters.** This file is the escape hatch of the phase's central mechanism. The property
EV-04 claims is "deleting a registry line cannot silently delete the requirement from the
must-be-covered set", and that property holds. What is not guarded is the inverse: *adding a
malformed row* removes a requirement from the set with no refusal and no reason recorded, which
is the same evidence deletion performed through the one file the check trusts. The check that is
supposed to make the gap unrepeatable can therefore be defeated by a one-word edit that the
file's own header says is illegal.

**Evidence (probes B and C).** On a copy of the good tree with the `API-02` registry line
removed:

- no ledger row → `-- FAIL important-gates-complete-uncovered: API-02` and
  `-- FAIL important-gates-uncovered-gate: API-02` (correct);
- a bare `API-02` row → `-- important-gates pass gates=42 lines=43` (requirement silenced);
- a `API-02 none` row → `-- important-gates pass gates=42 lines=43` (silenced with the reason
  the header explicitly forbids).

**Fix.** Validate the row where the set is built and refuse a malformed one with its own token.
For example:

```sh
if [ -f "$exp/important-gates-excluded.ids" ]; then
  awk '!/^#/ && NF {
         if (NF < 2 || $2 == "none" || $2 !~ /_[^_]+_[^_]+/) print "MALFORMED " $1
         else print $1
       }' "$exp/important-gates-excluded.ids" > "$tmp/excluded.raw"
  grep '^MALFORMED ' "$tmp/excluded.raw" | while IFS= read -r l; do
    printf 'important-gates-excluded-malformed %s\n' "${l#MALFORMED }" >> "$find"
  done
  grep -v '^MALFORMED ' "$tmp/excluded.raw" > "$tmp/excluded"
fi
```

Then pin the new token in `data/expected/controls/important-gates.ids` and add one control that
overlays a bare row (the committed `important-gates-complete-uncovered` control already overlays
this file, so the shape is established). A cheaper alternative that needs no new token: treat a
malformed row as `important-gates-excluded-stale` and extend that token's prose to cover
"malformed", but the diagnostic then no longer says what is wrong.

### WR-02: the per-step pass-line guard covers 33 of the 38 steps, and the five it does not cover are the producers

**File:** `tools/check/run-controls.sh:172-186` (with `tools/gate.sh:168` as the weaker
enforcement that remains for the uncovered steps)

**What is wrong.** A-02/O2 lands the pass-line rule inside `good_one`, which runs once per
*distinct `CHECK` value in `tools/check/controls/*/CHECK`*. That set is 33, not 38:

```
$ cat tools/check/controls/*/CHECK | sort -u | wc -l   -> 33
$ for f in tools/check/steps/*.sh; ... -> NO-CONTROL c-faults c-tests ci-summary pkg-install runner-selftest
```

For those five steps the only remaining rule is `tools/gate.sh:168`, `grep -q -F -e "-- $name
pass"` — "a pass line exists anywhere", which is exactly the weaker check the hardening item
replaces.

**Why it matters.** The item is advertised as "each step's pass line must be the LAST non-empty
line" (`run-controls.sh:18-19`, `09-06-SUMMARY.md`). It is enforced for the 33 steps that have a
control, and the five excluded ones are the largest producers in the gate (`c-tests`,
`c-faults`, `pkg-install` are the three longest steps). A step that prints its pass line and then
continues reporting passes on the uncovered five and fails on the covered 33 — the same defect
class, decided by whether a control directory happens to exist. The risk is bounded (a
`-- FAIL` printed after a pass line is still collected by `keep_fails` and turns the gate red), so
this is a narrowing, not a false pass.

**Fix.** Enforce the rule in the replay in `tools/gate.sh`, where every step's buffer is already
read and its rc already known — one `awk` per step, no control needed, and it covers all 38:

```sh
  if [ "$(cat "$GATE_TMP/rc.$name")" -eq 0 ]; then
    last=$(awk 'NF { l = $0 } END { print l }' "$GATE_TMP/step.out.$name")
    case $last in
      "-- $name pass"*) ;;
      *) bad gate-step-pass-line-not-last "$name" ;;
    esac
    if grep -q -F -e "-- $name pass" "$GATE_TMP/step.out.$name"; then
```

Keep the `good_one` copy: it is what proves the rule can fire (the `st-pass-line` case).

### WR-03: a `release-only` id's only report is refused by nothing, and the report `release-only=` counts is read by nobody

**File:** `tools/check/steps/golden.sh:405-416`

**What is wrong.** `$tmp/ids` is built from the report filenames that *exist*
(`golden.sh:383-393`), and the new branch then does:

```sh
  if [ "$(skip_of "$id")" = release-only ]; then
    relonly=$((relonly + 1))
    continue
  fi
  dv="$gate/dev/runner/$id.report"
  rl="$gate/release/runner/$id.report"
  if [ ! -f "$dv" ] || [ ! -f "$rl" ]; then
    bad golden-dev-release-diff "$id (a report of one preset is missing)"
```

Two consequences. (a) The `continue` sits *before* the existence refusal, so for a
`release-only` id the refusal `golden-dev-release-diff … a report of one preset is missing` is
unreachable; and because the id only enters the loop if a report exists somewhere, a
`release-only` id whose release report has gone is not iterated at all — `release-only` simply
prints `1` instead of `2` and the gate passes. (b) Nothing reads
`build/gate/release/runner/<id>.report` for a `release-only` id (`grep -rn 'release/runner'
tools/` finds only `golden.sh:416`, which the branch never reaches for these ids), so the report
the stat counts is written by `tests/runner-process-tap.sh` and consumed by nothing.

**Why it matters.** The trim's own justification is that the two folded ids keep a judgement
(the derivation equality) and that the mode of every run is made visible by `release-only=<n>`.
The stat is the only thing that would reveal the two ids no longer running, and it asserts
nothing: `release-only=1` is a passing gate. The evidence for "the folded scenario still ran on
Release" is one file that no check opens.

**Fix.** Iterate the inventory's `release-only` rows rather than relying on a report existing,
and keep the "the producer ran" evidence the trim did not intend to remove:

```sh
while read -r rid rskip; do
  [ "$rskip" = release-only ] || continue
  [ -f "$gate/release/runner/$rid.report" ] \
    || bad golden-dev-release-diff "$rid (a release-only scenario has no release report)"
done < "$tmp/invskip"
```

That makes `release-only=` a count that cannot silently shrink, and turns a lost report into the
same refusal every other id gets.

### IN-01: the partition header says nothing is emitted during execution, and the preflight verdict is

**File:** `tools/gate.sh:17` vs `tools/gate.sh:144-155`

**What is wrong.** "Nothing is emitted during execution" is the written form of the A-05
preserving condition. The `gate-preflight: PASS|FAIL` line is printed between the two groups, in
the execution loop, before the replay. My probe (below) shows the emitted set outside the replay
is exactly two lines — the partition line before execution and this verdict between the groups —
so the sentence is false by one line.

**Why it matters.** A reader using this comment to reason about output order (as the phase's own
verifier did) will conclude the stream is the replay alone.

**Fix.** `# The only output outside the replay is the partition line before execution and the gate-preflight verdict between the two groups.`

### IN-02: two stale counts in the files that define the exclusion mechanism

**File:** `data/expected/important-gates-excluded.ids:24`, `tools/check/steps/important-gates.sh:28`

**What is wrong.** The ledger header says "the file holds seven rows"; it holds ten
(`REL-01 API-01 API-05 API-06 RUN-03 PKG-02 EV-01 EV-02 EV-03 EV-04`). The step header says "the
six requirements it lists then fail important-gates-complete-uncovered"; it lists ten.

**Why it matters.** Both sentences are the reader's guide to a mechanism whose whole job is
counting requirements, and a reader who trusts "six" will not predict the right failure set for
a deleted ledger.

**Fix.** Make both sentences count-free ("the rows it lists then fail …"), which is also what
survives the next requirement.

### IN-03: the `c.accepted` line citation is stale in both places that carry it

**File:** `tests/state/state_fuzz.c:919`, `data/expected/tests/state.ids:107`

**What is wrong.** Both cite `tests/state/state_fuzz.c:945-948` for the acceptance refusal in
`fz_byte_flips`. At HEAD the function opens at `:946` and the refusal is at `:954`:

```
$ grep -n 'static int fz_byte_flips' tests/state/state_fuzz.c   -> 946:static int fz_byte_flips(void)
$ grep -n 'c.accepted != 0u'        tests/state/state_fuzz.c   -> 954:  if (c.accepted != 0u) {
```

The six comment lines this plan inserted above `fz_run_flips` shifted the site the citation was
written against.

**Why it matters.** Trim (d) has no purpose-built "moves the digest" guard, and the recorded
substitute is precisely "read the refusal at this line". A stale citation sends the reader to the
`ST_CHECK`/`fz_ctx_init` lines instead.

**Fix.** Cite `:954` in both, or cite the function (`in fz_byte_flips`) and no line.

### IN-04: `control-pass-line-not-last` now covers two different causes

**File:** `tools/check/run-controls.sh:172-186`

**What is wrong.** The previous check was `run_step && grep -q -F -e "-- $1 pass"`; the new one
is "the last non-empty line starts with `-- <step> pass`". A step that exits 0 and prints **no**
pass line at all now reports `control-pass-line-not-last` (with an empty detail), where it used
to report `control-good-failed`.

**Why it matters.** The token is the diagnostic a reader uses to tell "the step printed too much"
from "the step printed no verdict"; it now says the former for both. `gate.sh` still separates the
two (`gate-step-no-pass-line` vs `gate-step-failed`), so the two halves of the gate disagree on
the vocabulary.

**Fix.** Split the case:

```sh
      "-- $1 pass"*) ... ;;
      "") fail control-no-pass-line "$1"; cat "$2/out" ;;
      *)  fail control-pass-line-not-last "$1"; cat "$2/out" ;;
```

### IN-05: the golden body comparison reads only the dev report, so a `release-only` scenario can never carry a golden

**File:** `tools/check/steps/golden.sh:317`

**What is wrong.** `rep="$gate/dev/runner/$scn.report"` is unconditional, so a
`data/golden/<release-only-id>.{expect,digests}` would fail with `golden-mismatch (no report
build/gate/dev/runner/…)` before any comparison. Not reachable today — there is no golden for
`demo-lamp-tone-shift` or `demo-lamp-tone-swap` — so this is a trap rather than a defect.

**Why it matters.** The fold is the model the next phase will follow for any other heavy repeat
variant, and the natural next step (give a folded variant a golden so the fold keeps a pinned
expectation) fails with a message that names the wrong cause.

**Fix.** Select the report by mode, one line:
`case $(skip_of "$scn") in release-only) rep="$gate/release/runner/$scn.report" ;; *) rep="$gate/dev/runner/$scn.report" ;; esac`

### IN-06: the toy-registry drift check has no liveness guard that it compared anything

**File:** `tools/check/run-controls.sh:299-300`

**What is wrong.** The whole drift check is inside `if [ -f "$_good_reg" ]; then`. If the
good-tree registry is absent or renamed, the check silently disappears; nothing reports "I
compared N overlays" and N is not derived anywhere. The two tokens are proven able to fire by
`runner-selftest.sh:91-105`, so the mechanism is live — but only in the synthetic tree.

**Why it matters.** GUARDRAILS 2.3 asks for a liveness guard beside every must-fail mechanism.
Here the guard is indirect: deleting the good-tree registry makes the `_good` positive control of
`important-gates` fail (`important-gates-uncovered-gate: data/expected/important-gates.ids
missing`), so the gate does go red. The check itself still has no statement of the form "this
check ran and examined 10 overlays", which is what would catch a *path* change rather than a
deletion.

**Fix.** Derive the count and refuse zero:
```sh
  ov_n=0
  ... ov_n=$((ov_n + 1)) ...
  [ "$ov_n" -gt 0 ] || fail control-toy-none "no overlay carries $reg"
```
with `control-toy-none` pinned in the same ids file as the other two.

### IN-07: the `SRC-` comment claims more than the class delivers

**File:** `tools/check/steps/sources.sh:39-48`

**What is wrong.** "The quantifier is `+`, so the match ends only where the token ends" is not
quite what `SRC-[A-Za-z0-9_-]+` does: a dot also ends the token, so `SRC-bsr-2609.pdf` extracts
the *registered* `SRC-bsr-2609` and passes. That is the measured trade-off the same comment
records (the dot-inclusive class would turn the tree red for three real `.pdf` tokens), so the
behaviour is right and deliberate — only the justification overstates it.

**Why it matters.** A future reader who trusts "ends only where the token ends" will not expect a
typo after a dot (`SRC-bsr-2609.1`) to be accepted.

**Fix.** "The match ends at a character outside `[A-Za-z0-9_-]` — a dot included, which is the
measured trade-off below."

## What was checked and found sound

- **Trim (c), the sampled digests.** `memcmp(x->storage, u->ref[w->tsel], IVS_MACHINE_BYTES)` at
  `tests/state/state_util.c:479` and `u->produced |= …` at `:496` are both outside the branch at
  `:487`; the reference digests are still taken once per target in `su_target` for either mode
  (`:389-391`); `digest_sampled` is a second flag, `su_run_init` `memset`s the struct and leaves
  it 0 (`:338-345`), and no other reader keys on `u->digests` (`grep -n 'digests' tests/state/*.c`
  gives only `su_target`, the branch, and the two setters in `state_restore.c:1722,1747`). The
  liveness guard `rs_reasons_covered` recomputes `produced` in mode 0 and requires
  `produced == RS_ALL_REASONS`.
- **Necessity note on the same trim (not a finding).** Because `su_nine_digests` is a pure
  function of the instance bytes that the `memcmp` immediately above already compares, the digest
  comparison cannot fail independently of it; sampling it removes work from a check that was
  already redundant, which is why no stand-in can show the *sampled* comparison failing. That is
  an argument that the trim is safe, not that it lost evidence.
- **Trim (d), the unsealed flip.** `for (j = 0u; j < 2u; j++)` with `if (!sealed && j == 1u)
  continue;` at `tests/state/state_fuzz.c:931-933`; the offset loop bound (`k < (uint32_t)fz.n`),
  `fz_pick` and `masks[]` are untouched, the resealed path keeps both values, and the id's
  assertion set is `c.accepted != 0` plus `fz_finish`'s `c->attempts > 0u` (`:628`) — reason
  coverage (`fz_seen`) is asserted only on the resealed id, which keeps both values, so the trim
  cannot lose an asserted reason.
- **Trim (a), the round-trip refusals.** `rt_refuse zero …` is still called exactly once
  (`tests/runner-process-tap.sh:1116`) and the id keeps its single `result` call; the three static
  judgements name real guard sites and I verified every citation: `runner/main.c:589-592` (the
  arm binds `seen`/`hi`, no `slot`), `:609-610`, `:613-621`, `:623-624`, and `:76` for the usage
  block. `hex_plus_one 0x8000000000000000` returns `9223372036854775809`, and `rt_arm` extracts
  exactly the five-line arm.
- **Lever (3), the derivation judgement.** I extracted `derivation_block`, `scenario_body`,
  `input_ticks`, `derivation_equal`, `variant_negative` and ran them standalone: `EQ PASS` for all
  four scenarios, `NEG PASS` for both folded variants (the one-value stand-in is rejected for
  `variant-derivation-differs`), block sizes 65/30/65/58 with 55/29/55/49 asserts — matching the
  `Counts:` line at `docs/video/demo-lamp-tone-1-derivation.md:316`. The stand-in that changes
  nothing is refused (`judge-vacuous`), so the negative half cannot pass vacuously. `release-only`
  rows print an ordinary `ok` line and `tests-skipped` is 4 in both gate runs.
- **The preflight partition.** `sh` and `dash` both: `gate-partition preflight=29 producers=9`,
  `gate-preflight: PASS steps=29/29`, `gate: PASS steps=38/38 controls=384/384 … release-only=2`,
  0 `^-- FAIL ` lines, and the token list byte-identical between the two shells. Normalised
  against the committed pre-change captures the only difference is the two partition lines. The
  class is genuinely derived: every step whose source names `build/` or `IVS_GATE_DIR` is a
  producer and nothing else is (`c-build c-faults c-tests cp1610-vectors faults-verdict golden
  memmap pkg-install tests-inventory`), and every build/-reading step I could find is in that
  set. `keep_fails` runs in the replay; buffers are `step.out.$name`; `bad` is not called in the
  execution loop.
- **The advertised-failure class.** Both observed-refusal proofs in `runner-selftest.sh` print with
  the `-- runner-selftest proof …` prefix (`:161-174`, `:200-210`), so `gate.sh`'s `keep_fails`
  and `ci-summary.sh:86` (both `grep '^-- FAIL '`) cannot collect them; the nested runner's output
  is captured, never printed; `variant_negative` prefixes its observed refusal with `# `, and
  `result` prefixes tap failures with `# `.
- **The memmap generator controls.** The liveness guard (`memmap-generator-missing`) is reached
  by the copied `CHECK_DIR` in `runner-selftest.sh`, and the refusal is additive: I reproduced
  the raw `cmake` output for the `memmap-generator-refused` overlay and the step's re-join turns
  cmake's wrapped `… mirror\n  3 is not a power of two …` into one line, so the control's second
  EXPECT line genuinely pins the re-join. A no-op generator cannot pass unnoticed either: the
  generated `.inc` is a build input (`tests/memmap/memmap_rows.c:71`).
- **The manifest output anchor.** `check_entries(lbl, key, obj)` gets `dirname` as `ARGV3` and the
  anchor is raised only for `outputs`, after `path_ok`, against `^fixtures/<dir>/` and `^data/`;
  the real tree's only `data/` outputs are `FX-ivsc-vectors`'s two derived files, as the comment
  states, and `notices`/`recipe.sources` are left unanchored.
- **The derived must-be-covered set.** Recomputed with the step's own awk: 54 Complete (10 live +
  50 archived, 6 overlapping), 10 excluded, 44 accepted, 44 distinct requirement fields in the
  registry, `comm` empty in both directions, `54 - 10 = 44`. The parse is fail-loud in both
  directions: an unreadable record shrinks `accepted`, and every registry row then reports
  `important-gates-unknown-id`, so a missing record cannot produce a quiet empty set. Rows in
  other tables of the two records do not match (`grep -n '^| *[A-Z]+-[0-9]+ *|'` finds only the
  traceability rows, 10 and 50).
- **Portability and encoding.** `sh -n` and `dash -n` clean on all eight changed scripts; no
  non-ASCII byte in any changed file under `tests/`, `tools/`, `cmake/`, `data/`; `LC_ALL=C` is
  set before every new `sort`/`comm`/`grep` in the new code; the empty-first-file trap in the
  shift-tick `awk` is guarded by `[ ! -s … ]`; the `awk -v` values passed to `-v` are scenario ids
  and step names, none of which contains a backslash.
- **Inventories.** The `+6` and `+8` lines in `data/expected/tests/*.ids` and the `+12` in
  `runner-scenarios.ids` are comment lines or two column-5 changes; `tests-expected=4958`,
  `tests-executed=4954`, `tests-skipped=4` are unchanged from the pre-change capture, so wave A1
  added no test id. `data/expected/gate-steps/` is untouched and `steps=38/38`.

## Probes run

```
$ for f in tools/gate.sh tools/check/run-controls.sh tools/check/steps/{important-gates,golden,memmap,sources,runner-selftest}.sh tests/runner-process-tap.sh; do sh -n "$f"; dash -n "$f"; done
(no output: all clean)

$ LC_ALL=C grep -q -P '[^\x00-\x7F]' on every changed file under tests/ tools/ cmake/ data/
(no output: no non-ASCII byte)

$ sh tools/gate.sh > /tmp/p9rev/gate-sh.log 2>&1          # 68 s, exit 0
gate-partition preflight=29 producers=9: api-effects c-purity ci-log ci-parity ci-summary claims cp1610 cp1610-reader denylist discrepancies files glossary hygiene hygiene-history important-gates input integrity listing manifests profile psg-vectors release-assets runner-scan runner-selftest sources state text video vocab
gate-preflight: PASS steps=29/29
gate: PASS steps=38/38 controls=384/384 … reports=30 host-dependent=3 release-only=2 … gates=107 lines=330 … cases=12
$ grep -c '^-- FAIL ' gate-sh.log -> 0

$ dash tools/gate.sh > /tmp/p9rev/gate-dash.log 2>&1      # 83 s, exit 0
gate: PASS (same token list)   ; grep -c '^-- FAIL ' -> 0
$ diff <(tail -2 gate-sh.log|head -1|tr ' ' '\n') <(grep '^gate: PASS' gate-dash.log|tr ' ' '\n')
TOKENS IDENTICAL

$ norm() { sed -E -e 's/^step-time .*$/step-time .../' -e 's/^gate-cpu .*$/gate-cpu .../' \
    -e 's/^gate-time .*$/gate-time .../' -e 's/^gate-env .*$/gate-env .../' \
    -e 's/(^| )(ci-runs|files|tracked|hygiene-files|history-commits)=[0-9]+/\1\2=N/g' "$1"; }
$ diff <(norm 09-PREFLIGHT-BEFORE-SH.log)   <(norm gate-sh.log|grep -v '^exit=')
1a2,3
> gate-partition preflight=29 producers=9: …
> gate-preflight: PASS steps=29/29
$ diff <(norm 09-PREFLIGHT-BEFORE-DASH.log) <(norm gate-dash.log|grep -v '^exit=')
1a2,3
> gate-partition preflight=29 producers=9: …
> gate-preflight: PASS steps=29/29

$ awk over .planning/REQUIREMENTS.md + .planning/milestones/*-REQUIREMENTS.md (the step's own expression)
file=.planning/REQUIREMENTS.md rows=10
file=.planning/milestones/v0.1.0-REQUIREMENTS.md rows=50
complete=54  excluded=10  accepted=44  registry reqs=44
comm -13 accepted registry -> (empty)      comm -23 accepted registry -> (empty)

$ # ledger format probe on a copy of the good tree with the API-02 registry line removed
A) no ledger row:
   -- FAIL important-gates-complete-uncovered: API-02
   -- FAIL important-gates-uncovered-gate: API-02
B) bare 'API-02' ledger row:
   -- important-gates pass gates=42 lines=43
C) 'API-02 none' ledger row:
   -- important-gates pass gates=42 lines=43

$ cat tools/check/controls/*/CHECK | sort -u | wc -l   -> 33
$ for f in tools/check/steps/*.sh; do grep -qx -F "$(basename $f .sh)" <(cat tools/check/controls/*/CHECK) || echo "NO-CONTROL $n"; done
NO-CONTROL c-faults  NO-CONTROL c-tests  NO-CONTROL ci-summary  NO-CONTROL pkg-install  NO-CONTROL runner-selftest

$ # derivation judgement, extracted and run standalone
EQ PASS demo-lamp-tone-pass / -idle / -shift / -swap
NEG PASS demo-lamp-tone-shift / demo-lamp-tone-swap
pass der=65 scn=65 asserts=55 | idle 30/30/29 | shift 65/65/55 | swap 58/58/49

$ sh /tmp/p9rev/rt.sh        # hex_plus_one and rt_arm
max_dec=0x8000000000000000   plus_one=9223372036854775809
--- arm ---
    } else if (strcmp(a, "--roundtrip-at-ticks") == 0) {
      seen = &have_roundtrip;
      lo = 1u;
      hi = IVS_RUN_TARGET_MAX;

$ cmake -DROOT=<toy tree> -DOUT=<tmp> -P cmake/IvsMemmapProbes.cmake   # the refused overlay
CMake Error at cmake/IvsMemmapProbes.cmake:32 (message):
  IvsMemmapProbes: data/memmap/intv-2609-ntsc-family-p1.literal.txt:2: mirror
  3 is not a power of two up to 65536
$ ROOT=<toy tree> … sh tools/check/steps/memmap.sh | grep memmap-generator
-- FAIL memmap-generator-refused: IvsMemmapProbes: data/memmap/intv-2609-ntsc-family-p1.literal.txt:2: mirror 3 is not a power of two up to 65536

$ grep -n 'memcmp(x->storage' tests/state/state_util.c -> 479   ; grep -n 'produced |=' -> 496
$ grep -n 'if (u->digests'   tests/state/state_util.c -> 487   ; grep -n 'c.accepted != 0u' tests/state/state_fuzz.c -> 858, 954, 1421
$ grep -n 'static int fz_byte_flips' tests/state/state_fuzz.c -> 946

$ grep -rn 'release/runner' tools/ -> tools/check/steps/golden.sh:72,416 only
$ grep -rn 'IvsMemmapProbes' --include=*.txt --include=*.cmake . -> tests/CMakeLists.txt:85-96 + tests/memmap/memmap_rows.c:71
$ git status --short -> only the pre-existing dirty .planning/state.json and the untracked 09-VERIFICATION.md
```

---

_Reviewed: 2026-11-26_
_Reviewer: gsd-code-reviewer (deep, read-only on source)_

---

## Fixes applied

The three warnings are fixed; the six INFO findings are left as recorded. One token was added
(`important-gates-excluded-malformed`); the other two mechanisms reuse a token that already exists
and already has a pinned control, and the reasons are given per finding below.

### WR-01: the exclusion ledger's declared row format is enforced

**Commit:** `0d84484`
**New token:** `important-gates-excluded-malformed: line <n>: <row>` - a ledger row that is not
exactly `<REQ> <reason>` with `<REQ>` shaped like a requirement id and `<reason>` at least three
underscore-joined words (no leading, trailing or doubled underscore), or that gives `none`.
**Where:** `tools/check/steps/important-gates.sh` - the same `awk` pass that builds the exclusion
set now validates the row and keeps a malformed one out of the set, so it can neither silence a
requirement nor pass as a recorded exclusion. Pinned in `data/expected/controls/important-gates.ids`.
**Control that proves it can fail:** `important-gates-excluded-malformed` overlays the good-tree
ledger with a bare `API-02` row and an `API-03 none` row. Observed, and nothing else:
`-- FAIL important-gates-excluded-malformed: line 7: API-02` /
`-- FAIL important-gates-excluded-malformed: line 8: API-03 none`.
**Good-tree row set:** the `_good` ledger now carries one well-formed row
(`FND-98 close_record_only_toy_positive_row_for_the_ledger_format`) for a toy requirement that is
Complete and unregistered (`| FND-98 | Phase 1 | Complete |` in both toy records, and in the
`important-gates-complete-uncovered` overlay records). Without it the positive control would not
exercise the accepting half of the validation, and a validator that refused every row would pass on
the pristine tree. The `_good` stats are unchanged (`gates=43 lines=44`), as are the real tree's
(`gates=107 lines=330`) and the `important-gates-complete-uncovered` control's three lines.
**Count delta:** `controls` 384 -> 386 (this control and WR-03's). `gates`/`lines` unchanged.

### WR-02: the pass-line rule now covers all 38 steps

**Commit:** `ba34aba`
**Token:** the existing `gate-step-no-pass-line`, reused, with the detail naming the shape:
`<name (the pass line is not the last non-empty line)>`. It is reused rather than split because the
invariant is one invariant - a step that exits 0 must *end* with its verdict - and because
`tools/gate.sh` has no ids inventory of its own tokens, so a second name here would be a token
nothing pins; the reviewer's IN-04 split is about `run-controls.sh`, whose tokens are pinned in
`data/expected/controls/*.ids`.
**Where:** the rule moved into `tools/check/pass-line.sh` (exit 0 accepted, 1 no verdict, 2 verdict
not last) and `tools/gate.sh`'s replay calls it once per step, so `c-faults c-tests ci-summary
pkg-install runner-selftest` are covered. `run-controls.sh`'s copy inside `good_one` is untouched:
it is the control-reachable proof. Output order and the summary line are unchanged.
**Stand-in that proves it can fail:** the `st-gate-pass-line` case of
`tools/check/steps/runner-selftest.sh` feeds the rule file three outputs and requires exit 0, exit 2
and exit 1. Observed: `-- runner-selftest proof pass-line-not-last: profile (exit 2 observed)`.
**Count delta:** `cases` 12 -> 13. `steps=38/38` unchanged.

### WR-03: a lost release report is a refusal, not a smaller count

**Commit:** `9f5e08c`
**Token:** the existing `golden-dev-release-diff` - it already means "a report of one preset is
missing", which is exactly this case, and it already has a pinned control; the detail names the id:
`<id (a release-only scenario has no release report)>`.
**Where:** `tools/check/steps/golden.sh` - `relonly` is no longer incremented in the report loop
(that loop only `continue`s for a `release-only` id). It is counted from `$tmp/invskip`, the
inventory rows whose column 5 is `release-only`, and each such row must have
`build/gate/release/runner/<id>.report`. The stat is therefore derived from the inventory and
cannot silently shrink.
**Control that proves it can fail:** `golden-release-only-no-report` marks the toy row `release-only`
and deletes its release report. Observed, and nothing else:
`-- FAIL golden-dev-release-diff: toy (a release-only scenario has no release report)`. Before the
fix that tree printed `-- golden pass ... release-only=1`, which is why the control also forbids
`-- golden pass`.
**Count delta:** `controls` 384 -> 386 (with WR-01's). `release-only=2`, `reports=30`,
`goldens=29`, `host-dependent=3` unchanged.

### D-09-06: the canonical list is now ten

WR-01's `important-gates-excluded-malformed` is a new must-fail mechanism with no registry line, so
it extends deviation D-09-06 (in `09-05-SUMMARY.md`, not rewritten here) from **nine** mechanisms to
**ten**: `important-gates-complete-uncovered`, `important-gates-excluded-stale`,
`control-toy-drift`, `control-deviation-unused`, `control-pass-line-not-last`,
`sources-src-token-whole`, `manifests-output-anchor`, `memmap-generator-refused`,
`memmap-generator-missing`, and now `important-gates-excluded-malformed`. The frozen `lines=330`
(ROADMAP criterion 3, amendment A-08) still holds after this run, and the reversal condition is
unchanged: a later milestone re-derives the baseline and then adds the rows. WR-02 and WR-03 add no
mechanism - both extend a mechanism already on the list's subject matter with a token that already
exists - so the list grows by one, not three.

### Predicted versus measured token delta

Predicted before the first commit: `controls` 384 -> 386, `cases` 12 -> 13, everything else byte
identical. Measured, `sh` and `dash`, after the last commit:

```
gate: PASS steps=38/38 controls=386/386 ... gates=107 lines=330 goldens=29 reports=30
release-only=2 host-dependent=3 cases=13 tests-expected=4958 tests-executed=4954
tests-skipped=4 tests-missing=0 claims=28 provisional=27
```

`diff` of the two token lists: identical. `grep -c '^-- FAIL '` is 0 in both logs. `files=` and
`tracked=` moved with the new committed files (derived at run time, not part of the frozen set).
