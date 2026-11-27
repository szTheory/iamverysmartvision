---
phase: 19-discriminating-the-display-path
fixed_at: 2026-11-27T07:20:00Z
review_path: .planning/phases/19-discriminating-the-display-path/19-REVIEW.md
iteration: 1
findings_in_scope: 5
fixed: 5
skipped: 0
status: all_fixed
---

# Phase 19: Code Review Fix Report

**Fixed at:** 2026-11-27
**Source review:** `.planning/phases/19-discriminating-the-display-path/19-REVIEW.md`
**Iteration:** 1

**Summary:**
- Findings in scope: 5 (WR-01, WR-02, WR-03, IN-06, IN-07) — the dispatch's scope: the three warnings and the two free record fixes
- Fixed: 5, plus IN-04 closed as a side effect of WR-03's rewrite
- Skipped: 0
- Left open by scope, with reasons: IN-01, IN-02, IN-03, IN-05 (all cosmetic or record-ordering; none is a code defect that can bite the phase's evidence)
- `stic` suite: `executed=111 skipped=0 failed=0`, id set byte-identical to `data/expected/tests/stic.ids`
- `runner-process` suite: `executed=164 skipped=3 failed=0`, 167 ids, identical to its inventory
- Matrix masks unchanged: `0xAAAA`, `0xCCCC`, `0xF0F0`, `0xFF00`; the diagonal still rejects the entangled guest at cell 2 (bits `0x2`)
- Gate: `sh tools/gate.sh` rc 0 in 69 s and `dash tools/gate.sh` rc 0 in 67 s, sequentially, zero `-- FAIL` lines, summary lines `cmp`-identical
- `tools/gate-linux.sh`: **not run** — the docker daemon on this host is unresponsive; recorded as a visible not-run line, never a pass (lesson 24)

Verification ran in the **main checkout** (no worktree: `workflow.use_worktrees` is `false` in
`.planning/config.json`, the documented opt-out). The gate numbers below are therefore reproducible from the tree as
committed, not from an isolated tree that was torn down.

## Fixed Issues

### WR-01: the public declaration comment for `ivs_stic_effect` stated a guarantee the code no longer provides

**Files modified:** `core/src/stic/stic.h`
**Commit:** `48c7ba4`
**Applied fix:** the comment now describes the two-limb READ match the code makes (a mode-select address keeps the
register-index match so `$4021` still selects Color Stack mode; every other READ matches by exact address, so the
`$0020` READ row is reachable and `$0040` still has no effect), and the WRITE/DW register-index match. Comment only: no
code, no test, no count moved.

### WR-02: the bus-replay loop was copied five times, and three copies hardcoded the access kind and address

**Files modified:** `tests/stic/stic_disc.c`
**Commit:** `812b838`
**Applied fix:** `sd_replay_one` is now the one place the stamp is composed
(`unit_start_tick = h->stamp[i] - q->access_strobe_ticks`) and the one place the access kind is decided — from the
hit's own `write` flag, never a literal. `sd_replay` is the kind filter over the recorded hits (`SD_READS`,
`SD_WRITES`, `SD_ANY`), and `sd_observe_hits` collapses into `sd_observe_x` with `SD_ANY`. `sd_g4_vector` keeps its
own ascending catch-up policy (a re-strobing guest must not move the cursor past an earlier frame close before it is
read) and calls `sd_replay_one` inside that pass, so plan 19-01's recorded interleaving is unchanged.
**No behaviour change, measured:** the suite's own result file is `executed=111 skipped=0 failed=0` with an id set
identical to the inventory, and the binary's stdout still prints masks `0xAAAA`, `0xCCCC`, `0xF0F0`, `0xFF00` and
`the diagonal rejected the entangled guest at cell 2 (bits 0x2), observed 1`.

### WR-03: `--max-runs` could not change the sweep; every accepted value ran the same six offsets

**Files modified:** `tools/disc-sweep.sh`, `tests/runner-process-tap.sh`
**Commit:** `f24ad2c`
**Applied fix:** the option is a cap on the walk (`cap` = `--max-runs` when given and below the list length, else the
list length), the table's own header states how much of the list was covered
(`# disc-sweep: 2 of 6 offsets (run-count cap 2, bound 16), ...`), and a value above the fixed bound still refuses
rather than silently running the whole list. The always-true post-block comparison is gone and the list-length
invariant is stated once, next to the list definition (IN-04). `runner-process.disc-sweep.bound` gained the positive
limb: `--max-runs 2` must write exactly two offset rows, the **first two** offsets in list order (`14000`, `14934`), and
a header naming the cap against the list length. No id added or removed (167 ids, `executed=164 skipped=3 failed=0`).
**Measured before the fix:** every accepted value 6..16 produced the same six-row table; after it, `--max-runs 2`
produces two rows. The case fails if the option goes back to being inert, which the old case could not.

### IN-06: `PROJECT.md`'s status line said "after Phase 18" while its body described Phase 19

**Files modified:** `.planning/PROJECT.md`
**Commit:** `12a9cdd`
**Applied fix:** the prefix now reads `after Phase 19 — Discriminating the Display Path` and states the outcome in one
sentence (Exit B, the diagonal matrix, the display-path field still `unknown-default-chosen`). The appended paragraphs
are untouched (append-only).

### IN-07: `19-VALIDATION.md` still described an unstarted phase

**Files modified:** `.planning/phases/19-discriminating-the-display-path/19-VALIDATION.md`
**Commit:** `fb94092`
**Applied fix:** `status: complete`, `nyquist_compliant: true`, `wave_0_complete: true`; the seven tasks that shipped
(not eight — 19-02's two tasks landed in one commit) each `✅ green` against a command actually run in this pass, with
the commands and their measured output in a `## The commands this pass ran` section (V1–V5); the six Wave 0 boxes
checked with where each landed; the token line; and the container gate recorded as a visible `not run` line with its
reason rather than an empty sign-off block.

### IN-04: the second run-count check was always true — closed as a side effect of WR-03

**Files modified:** `tools/disc-sweep.sh` (same commit as WR-03)
**Commit:** `f24ad2c`
**Applied fix:** the constant comparison is gone. The list-length invariant is asserted once, immediately after the
offset list is counted and before the output is created, so the bound is stated in one place instead of two.

## Left open, with reasons

### IN-01: two file headers describe the changed events without the parameter that now decides them

**File:** `core/src/stic/stic_events.c:15-16`, `core/src/machine/machine_bus.c:20-31`
**Reason:** out of this pass's scope, and cosmetic in the sense the dispatch allowed: both branches carry an accurate
inline comment where the decision is made, `stic_params.h` is the place a reader learns the parameters exist, and
`stic_regs.c`'s header was amended rather than left alone. The remedy is an append-only `Plan 19-01 (D-54, C82/C94): …`
paragraph per header (§2.12) and belongs with the next commit that touches those files rather than in a comment-only
commit made to close an Info finding.

### IN-02: `sd_hits.cycle[]` is written and never read

**File:** `tests/stic/stic_disc.c`
**Reason:** out of scope, and deliberately left alone by WR-02's refactor: the field is written in `sd_measure` and has
no reader, but the refactor's contract was "no behaviour change", and deleting a recorded field in the same commit
would have mixed a cleanup with a correctness fix. The natural fix (use the cycle in the `SD_G1_BROKEN` diagnostic,
which currently reports a tick where a CPU cycle is the readable number) changes a diagnostic's text, so it wants its
own commit and its own before/after evidence.

### IN-03: the `else()` branch of the new gating select is unreachable

**File:** `cmake/IvsSticTable.cmake:378-383`
**Reason:** out of scope and cosmetic: the branch cannot run, so it can neither pass nor fail anything. Its second half
is the real item — a stale `gating_apply` could leak across `foreach(st_profiles)` iterations because `set()` is
directory-scoped — and that wants an `unset(gating_apply)` at the top of the loop plus a configure-time negative
control, which is a change to a build-time policy selector and not a review tidy-up.

### IN-05: Exit A's trigger is interpreted only in the post-measurement section

**File:** `.planning/phases/19-discriminating-the-display-path/19-DECISION.md:241-245`
**Reason:** the record is append-only (§2.12), so the finding's own remedy is a `PLAN-LESSONS` entry rather than an edit
to the record. This pass added lesson 34 (the predicate a correct record cannot satisfy, the review's adjudication of
deviation 2); IN-05's lesson — *a phase-exit trigger must be interpreted in the plan or the pre-measurement record,
never only in the appended result section* — is the adjacent plan-satisfiability shape and is owed to the next plan
that carries an exit choice, where it can be written with that plan's own instance rather than this one's.

---

_Recorded: 2026-11-27T07:20:00Z_
_Fixer: Claude (gsd-code-fixer)_
_Iteration: 1_
