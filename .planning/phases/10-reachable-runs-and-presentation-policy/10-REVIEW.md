---
phase: 10-reachable-runs-and-presentation-policy
status: findings
blockers: 2
warnings: 6
infos: 2
---

# Phase 10: Code Review

**Reviewed:** 2026-11-25
**Branch:** `gsd/phase-10-reachable-runs-and-presentation-policy`, base `origin/main`
**Scope:** the source diff `origin/main..HEAD -- tools tests cmake runner core` (17 files: 5 changed scripts/CMake hooks, 3 new control trees, 4 `data/expected` inventories) plus the prose that publishes commands (`docs/running.md`, `docs/handoffs.md`, `docs/presentation.md`) and the ops record that publishes the release procedure (`.planning/ops/RELEASE-OPS.md`).
**Method:** every finding below was reproduced with a probe on this tree; the probes and their output are listed in "Probes run". Adversarial stance: the two guards and the rehearsal were assumed breakable until a probe showed otherwise.

## Summary

The phase lands three mechanisms and a new rehearsal mode. What works is substantial and was verified, not accepted: the claimed claim-word list catches `looks-like` and `sounds-like` while missing `overlooks`; the committed control `claims-handoff-artifact` does fail for its intended token only, with `claim-dangling-path` correctly excluded by an existing file in `_good/tree`; `claims-handoff-row` fires on exactly line 3 and on nothing else; every command published in `docs/handoffs.md` (H7-1's 21674-byte WAV and `refpcm.sha256`, H7-2's exit-14 refusal and 45806-byte PPM, H8-3's `report.end 81`, D11's SKIPPED/77) and in `docs/running.md` (the 131-line report, the first and last line, the nine-line tail byte for byte, `host.calls 1599` at slice 1000, `outcome missing-fixture` without the `images` component) was executed here and holds. `sh tools/release-rehearse.sh --no-gate --source-route` was run end to end on this tree and passes.

Two BLOCKERs remain, both in the *enforcement* of EV-05 rather than in its intent. First, the handoff ledger — this phase's central deliverable — can be deleted, emptied of one row, or made unreadable and the `claims` step still prints `-- claims pass`: the existence guard that lets the toy control tree have no ledger also lets the real tree have none. Second, the handoff-artifact predicate is lexical, so a claim may cite a `build/` path spelled `./build/...` or `Build/...` (which resolves on this project's primary, case-insensitive host) and be accepted. Neither is a logic slip in an edge case; both are the check being satisfiable while the evidence is missing or wrong.

The rehearsal's verbatim filter copy is correct today (byte-identical to `golden.sh:146-156` and `:159-161` after the rename) and its comparison limb fails closed on delegate list drift, but nothing compares the copy to its source and, for this scenario, all four live-domain inputs are `1`, so the copied `route_live` is inert — a copy that returned `1` unconditionally would produce the same bytes and pass.

## Findings

### BLOCKER

#### B-01: The handoff ledger can be missing, short a row, or unreadable and the gate still passes

**File:** `tools/check/steps/claims.sh:26-43`

**What:** the whole handoff-grammar check is inside `if [ -f "$ROOT/data/expected/handoffs.ids" ]`. A missing ledger skips the block entirely; an unreadable ledger makes `awk` fail, `$hb` stays empty, and `[ -s "$hb" ] && exit 1` is false, so the step prints its pass line and exits 0.

Probes (`Probes run` 9 and 10):

```
# ledger moved aside
-- claim pass records=28 claims=28 provisional=27
-- claims pass claims=28 provisional=27            EXIT=0
# ledger chmod 000
awk: can't open file <home>/data/expected/handoffs.ids
-- claims pass claims=28 provisional=27            EXIT=0
```

**Why:** `data/expected/handoffs.ids` is referenced by exactly three things on this tree — `tools/check/kinds/claim.cmake:114` (only fires when a claim cites it), `tools/check/steps/claims.sh:26` (the guarded reader), and `docs/handoffs.md:3` (prose). No step enumerates `data/expected/*.ids` and requires each to be registered or present (`grep -rn 'data/expected/[*]' tools/` returns one comment; no step globs the directory), so deleting the file is invisible. That file is the phase's deliverable: `10-CONTEXT.md` decides it in the "The handoff ledger (EV-05)" block, and GUARDRAILS §5.7 requires every `H<n>-<k>`/`D<n>` id to have a row in it. The gate can be green with the whole ledger gone, and a later milestone then re-asks the owner exactly what the ledger exists to stop being re-asked.

A second limb of the same defect: the grammar is checked, the *set of ids* is not. `H7-1`, `H7-2`, `H8-3`, `D10`, `D11` are pinned only by the phase record. Replacing the file with `X-1 handed-off evidence: none` fails nothing (`claim-handoff-row` is satisfied by any well-formed row), and deleting the `D11` row while keeping four well-formed rows fails nothing either. Compare `data/claims.json`, whose id set *is* pinned by `data/expected/claims.ids` and refused as `claim-id-extra` — the ledger has no equivalent.

**Fix:** require the ledger, and distinguish "absent" from "present and empty":

```sh
hled="$ROOT/data/expected/handoffs.ids"
if [ ! -f "$hled" ]; then
  printf -- '-- FAIL claim-handoff-row: no ledger (%s)\n' "data/expected/handoffs.ids"
  exit 1
fi
if ! awk '...' "$hled" > "$hb"; then
  cat "$hb"
  printf -- '-- FAIL claim-handoff-row: unreadable (%s)\n' "data/expected/handoffs.ids"
  exit 1
fi
```

Then remove the `-f` guard's purpose by giving the positive control a ledger: add `tools/check/controls/_good/tree/data/expected/handoffs.ids` with one well-formed row (`H7-1 handed-off evidence: none`), so `good_one claims` still passes with the file required. Prove the new token with a control (`MAKE`: `delete data/expected/handoffs.ids`, `EXPECT`: `FAIL claim-handoff-row: no ledger`) — GUARDRAILS §2.3's stand-in showing the guard can fail. For the id set, pin it the way claims are pinned: add the five ids to `data/expected/handoffs.ids`'s header as a `# ids:` line or a companion single-column inventory, and have `claims.sh` refuse `claim-handoff-row: unknown id <id>` / `: missing id <id>` in both directions. Add the `D11`-deleted and one-wrong-id overlays to the control's two extra cases.

---

#### B-02: The handoff-artifact predicate is lexical, so `./build/...` and `Build/...` are accepted claim evidence

**File:** `tools/check/kinds/claim.cmake:112-116`

**What:** the predicate matches `^build/`, `[.]wav$`, `[.]ppm$`, `[.]tsv$` and one exact string against the raw `artifact` value. `claim-dangling-path` (`:99-102`) accepts any path in `^[A-Za-z0-9._/-]+$` without `..` that exists, so a `./`-prefixed or case-varied spelling reaches the handoff predicate unchanged and does not match it.

Probes (`Probes run` 7 and 8), a scratch copy of `_good/tree` with the file present so `claim-dangling-path` cannot fire and `data/expected/claims.ids` extended so `claim-id-extra` cannot:

```
artifact "./build/gate/dev/runner/toy.report"     -> -- claims pass claims=3 provisional=0  EXIT=0
artifact "out/x.WAV"                              -> (same run, accepted)
artifact "data/bench/workloads.TSV"               -> (same run, accepted)
artifact "Build/gate/dev/runner/toy.report"       -> -- claims pass claims=2 provisional=0  EXIT=0
artifact "data/evidence/gate/dev/runner/toy.report" (data/evidence -> ../build) -> accepted
```

**Why:** GUARDRAILS §2.8 is "no claim may cite a handoff artifact, a `build/` path, a `.wav`, a `.ppm` or a bench `.tsv`". This is the only mechanism that enforces it, and the spellings that defeat it are not exotic: `./build/release/runner/demo-lamp-tone-pass.report` is what a shell-minded author writes, and it *only* needs to exist — which it does during every gate run, because `build/gate/` is populated by the time `claims` runs. `Build/` resolves on macOS's default case-insensitive filesystem, the project's primary host. `[.][.]` (`:100`) already refuses `data/../build/x`, so the omission of `./` is an inconsistency in the same predicate set, not a strategic choice. The comment at `:105-110` claims each of the five classes "is refused by its own token rather than by `claim-dangling-path`, which only fires while the file is absent" — true, and precisely why a present file spelled differently walks through.

**Fix:** normalise before testing, once:

```cmake
  if(NOT "${artifact}" STREQUAL "" AND EXISTS "${ROOT}/${artifact}")
    file(REAL_PATH "${ROOT}/${artifact}" real)      # resolves ./ , // , and symlinks
    file(RELATIVE_PATH norm "${ROOT}" "${real}")
    string(TOLOWER "${norm}" low)                   # case-insensitive, as the host is
    if("${low}" MATCHES "^build/" OR "${low}" MATCHES "[.]wav$" OR "${low}" MATCHES "[.]ppm$"
       OR "${low}" MATCHES "[.]tsv$" OR "${low}" STREQUAL "data/expected/handoffs.ids")
      gate_fail(claim-handoff-artifact "${id}")
    endif()
  endif()
```

`file(REAL_PATH ...)` collapses the symlink case too, so all three bypasses close with one normalisation. Then extend the committed stand-in (see W-02) with `./build/...` and `Build/...` claims so the two spellings are proved refused on every gate run.

---

### WARNING

#### W-01: The rehearsal's copied filter is unguarded against drift, and its `route_live` half is inert for this scenario

**File:** `tools/release-rehearse.sh:47-68` (the copy), `:171-178` (its use)

**What:** the copy *is* byte-identical today — `diff` of the renamed functions against `tools/check/steps/golden.sh:146-156` and `:159-161` is empty (`Probes run` 2). But nothing compares them: no gate step, control, or rehearsal line reads `golden.sh`. And in this scenario all four live-domain inputs are `1`:

```
output.video.bytes -> 1   output.audio.bytes -> 1   input.events -> 1   output.psglog.bytes -> 1
```

With all four branches taken, `route_filter` prints every conditional key class regardless, so a `route_live` that returned `1` unconditionally — or one that dropped the `n == 1` or `v !~ /^0+$/` clause — produces byte-identical output and the `cmp` still succeeds. The comment at `:50-52` ("The copy is safe because it fails closed: a stale copy of the key list selects a different set of lines") is correct for *key-list* drift, and silent on this half.

**Why:** PLAN-LESSONS 20 is exactly this shape — "equivalence checks pass vacuously… give every equivalence check a guard that the compared thing happened". Here the compared thing (`want`) does happen (`[ -s "$work/route/want" ]` at `:176` guards it), but the *live-domain discriminator* inside it never varies, so half the copied program is dead code that a mistake can corrupt undetected. The gate does exercise `live_count`'s discriminating direction (14 goldens carry a `none` domain, which `golden.sh:341-350` cross-checks against the report), so the *original* is covered; only the copy is not.

**Fix:** assert the copy equals its source at the top of the mode — two lines, no new count, and it makes the "verbatim copy" a checked fact rather than a comment:

```sh
src=$(sed -n '/^filter_report() {/,/^}/p;/^live_count() {/,/^}/p' tools/check/steps/golden.sh \
        | sed 's/^filter_report() {/route_filter() {/; s/^live_count() {/route_live() {/')
cpy=$(sed -n '/^route_filter() {/,/^}/p;/^route_live() {/,/^}/p' "$0")
[ "$src" = "$cpy" ] || fail "source-route: the filter copy has drifted from tools/check/steps/golden.sh"
```

#### W-02: The committed stand-in covers 2 of the 5 refused artifact classes

**File:** `tools/check/controls/claims-handoff-artifact/overlay/data/claims.json:5-25`, `EXPECT:1-2`

**What:** the predicate refuses five classes — `^build/`, `.wav`, `.ppm`, `.tsv`, `data/expected/handoffs.ids`. The control's two claims exercise `build/gate/dev/runner/toy.report` and `data/bench/workloads.tsv`. Deleting `[.]wav$`, `[.]ppm$` or the `STREQUAL "data/expected/handoffs.ids"` limb from `claim.cmake` fails no control: `claims-handoff-artifact` would still pass on its two claims, and no other control cites an artifact of those three shapes.

**Why:** GUARDRAILS §2.3 requires "a must-fail mechanism… a liveness guard, and a committed scratch stand-in showing the guard fail" per new check, and `10-CONTEXT.md` decides the guard as "(i) The `claims` step refuses a claim citing a handoff artifact, a `build/` path, a `.wav`, a `.ppm` or a bench `.tsv`". The `10-VERIFICATION.md` scratch probe covered all five, but a scratch probe is not a committed stand-in — the point of committing it is that it runs on every gate.

**Fix:** add three claims to the overlay, with the files present in the overlay so `claim-dangling-path` cannot fire (a `.wav`/`.ppm` is refused by name in `binary-ext`, which does not apply to a control tree; the ledger needs one well-formed row so `claim-handoff-row` stays silent):

```json
{ "id": "CL-0003", "statement": "…", "profile": "intv-2609-ntsc-family-p1",
  "tier": "source-expectation", "artifact": "out/x.wav", "depends_on": ["stic.part"],
  "standing": "established", "note": "Example entry." }
```

with `out/x.wav`, `out/x.ppm` and a valid `data/expected/handoffs.ids` added under `overlay/`, `CL-0003..CL-0005` added to `overlay/data/expected/claims.ids`, and `EXPECT` extended to five lines. Add `./build/...` and `Build/...` as further claims once B-02's normalisation lands.

#### W-03: `claims-handoff-row` has no `FORBID`, so it cannot prove its finding is the only one

**File:** `tools/check/controls/claims-handoff-row/` (no `FORBID` file), `EXPECT:1`

**What:** `EXPECT` is one line, `FAIL claim-handoff-row: line 3`, matched as a substring. `run-controls.sh:222-253` requires every `EXPECT` token to occur and no non-empty `FORBID` line to occur; with no `FORBID`, any additional finding in the step's output is ignored. The control would still pass if the same run also emitted `FAIL claim-handoff-row: no rows` (a broken `rows` counter no longer skipping the blank line at ledger line 2), or a pre-existing claim token such as `claim-dangling-path` on the inherited good-tree ledger.

**Why:** the phase's own other control does this correctly (`claims-handoff-artifact/FORBID` holds `claim-dangling-path`), and the review focus for this control is exactly "`EXPECT` cannot be matched by output produced for a different reason, and `FORBID` excludes the pre-existing token". Asymmetric control design is how a control rots: the next editor tightens the wrong thing and nothing objects.

**Fix:** add `tools/check/controls/claims-handoff-row/FORBID`:

```
claim-handoff-row: no rows
claim-handoff-row: line 1
claim-handoff-row: line 2
claim-dangling-path
claim-order
claim-id-extra
```

Line 1 is well-formed and line 2 is blank in the overlay, so any finding on them is a defect in the reader; the claim tokens are the pre-existing ones a broken inherited ledger would raise.

#### W-04: `docs/running.md`'s nine-line expected tail is a hand-quoted figure that nothing pins

**File:** `docs/running.md:63-75`

**What:** three of the nine quoted lines are not in `data/golden/demo-lamp-tone-pass.digests`, so no data file carries their values:

```
audio.chain.check  NOT in golden
host.calls         NOT in golden
report.end         NOT in golden
```

(verified key by key). The other six are in the golden and would move with a reviewed golden diff.

**Why:** `10-CONTEXT.md` makes the exact tail part of PKG-05's deliverable ("the exact stdout tail is quoted byte for byte"), and the rehearsal verifies the *golden*, not the document — so a stale `audio.chain.check` or a `report.end` that moved because a key was added can never fail anything. The project's own rule is "Recompute, never quote… a quoted figure that disagrees with the file is a defect" (AGENTS.md, INDEX.md rule 3), and every other frozen figure on this tree has a mechanism; this one has prose only.

**Fix:** pin the nine lines in `data/expected/` (e.g. `data/expected/running-tail.ids`, one literal line per row) and check it in the `golden` step, which already opens the same report and the same golden; or derive the block from the golden plus `report.end` so the document cannot carry a number no file holds. Cheap alternative if a new `data/expected` file is not wanted this phase: extend the source-route mode to `grep -c -x -F` each of the nine lines against `$rep` and fail on a miss, which turns the rehearsal into the tail's liveness check without a new gate id.

#### W-05: `docs/running.md` states the golden's excluded keys exhaustively and is wrong about two of them

**File:** `docs/running.md:89-94`

**What:** the text says "The keys left out are the ones that are not a function of the scenario and the images - `ivsr`, `api.version`, `host.calls`, `report.end` and the operator keys". Measured against the report and the golden, the excluded set is larger:

```
keys in the report but not in the golden: api.version  audio.chain.check  exec:  grom:  host.calls  ivsr  report.end
```

`audio.chain.check` and the `content` identity lines `exec:` / `grom:` are functions of the scenario and the images, which is the opposite of the stated reason; and `audio.chain.check pass` is quoted two lines earlier in the very same document (`:67`), so a reader who follows the paragraph to the golden finds the key absent with no explanation.

**Why:** this is the sentence a reader uses to check the golden against a report, and `10-CONTEXT.md` makes the reachability route the phase's product. An enumeration presented as complete that a single `comm` disproves is the defect class ("a quoted figure that disagrees with the file"), and it will mislead the next person who wonders why an assert they can see in the report is not pinned.

**Fix:** name the real set, and fix the rationale rather than the list alone — the filter's reason is "keys that are not a *fixed function of the machine model at this profile*", which excludes `host.calls` (host-dependent), `exec:`/`grom:` (content identity, already pinned elsewhere), `audio.chain.check` (a per-domain check the golden key list omits) and the framing keys `ivsr`/`api.version`/`report.end`. Cite `tools/check/steps/golden.sh:146-156` for the list so the prose and the mechanism cannot disagree again.

#### W-06: The documented release procedure never invokes `--source-route`

**File:** `.planning/ops/RELEASE-OPS.md:8` and `:52`

**What:** the ops record publishes the rehearsal as `sh tools/release-rehearse.sh [--no-gate] [--release]` (`:8`) and as "Before tagging, the orchestrator runs `sh tools/release-rehearse.sh --release` and expects it to finish with `rehearse: ok`" (`:52`). Neither names `--source-route`, and `git diff --stat origin/main..HEAD -- .planning/ops/RELEASE-OPS.md` is empty — the phase did not update it.

**Why:** `10-CONTEXT.md` decides "The proof is the release rehearsal, not a gate step… the transcript is recorded at release time", and criterion 1 is satisfied only if the mode is run before a tag. Following the procedure as recorded, the release engineer runs the mode that does *not* unpack the archive, builds nothing cold, and never touches the demo golden — the phase's only automated proof of PKG-05 would simply not happen, and nothing would notice because the rehearsal is not a gate step by design. `docs/running.md` publishes the reader's route but not the maintainer's obligation.

**Fix:** one line in RELEASE-OPS.md §"the rehearsal" — `sh tools/release-rehearse.sh --release --source-route` as the pre-tag invocation, with the expected `rehearse: source-route: … golden match (report 131 lines, 124 of 124 compared)` line and the note that a `not run` line must be written if it is skipped (PLAN-LESSONS 24: never a pass).

### INFO

#### I-01: The look/listen word list misses the singular and the past tense, which are the most common spellings of the claim

**File:** `tools/check/steps/text.sh:47`

**What:** `-w -E '…|looks|sounds'` catches `looks-like` and `sounds-like` correctly (`-` is a non-word constituent, verified) and produces no false positive on `overlooks` or `sounding` — the boundary is sound. It does not catch `The sound is right.`, `The picture looked right.`, `It sounded like…`, `Nothing looks…` is caught but `look` in `the look of the plane` is not:

```
1:A looks-like console.        <- caught
2:It sounds-like the arcade.   <- caught
3:The sound is right.          <- missed
4:The picture looked right.    <- missed
6:Sounding board.              <- correctly not caught
```

**Why:** `10-CONTEXT.md` freezes only the two words ("plan 10-05 added them and no other word of that family"), so this is a recorded scope decision rather than an error — but "the sound is right" is the same judgement as "it sounds right", and a document that phrases the handoff as a noun slips past the tripwire with no `[nc]` and no claim tag.

**Fix:** if the family is widened later, add `look|looked|sound|sounded` and accept the false-positive cost on ordinary prose (`look at`, `sound reasoning`), or restrict to the phrasings that matter with a two-word pattern (`(looks?|looked|sounds?|sounded)\s+(like|right|correct|the same)`). Do not widen without re-running the gate, since `docs/provenance.md` and `docs/video/demo-lamp-tone-1-derivation.md` already needed one `[nc]` each for the two-word form.

#### I-02: The verification record's D11 command line is not the command that exits 77

**File:** `.planning/phases/10-reachable-runs-and-presentation-policy/10-VERIFICATION.md:105`

**What:** the row reads "owner-local real EXEC/GROM images (`sh tools/owner-run.sh` exits 77 without them)". The bare invocation exits **2** with a usage line:

```
$ sh tools/owner-run.sh
owner-run: usage: owner-run.sh [--preset dev|release] [--record] <exec-image> <grom-image>   EXIT=2
$ sh tools/owner-run.sh refs/FX-owner-exec.bin refs/FX-owner-grom.bin
SKIPPED owner-run: no operator images (D11)                                                 EXIT=77
```

**Why:** `docs/handoffs.md`'s D11 section is correct — it publishes the two-argument form and states the SKIPPED/77 behaviour for it, and that is what I executed. The record is the only place the bare form appears, so the documentation a reader is sent to is right; the record is wrong. Recorded because a later milestone citing this row to re-check D11 would run the bare command, get exit 2, and read it as a refusal.

**Fix:** replace with `sh tools/owner-run.sh refs/FX-owner-exec.bin refs/FX-owner-grom.bin` exits 77 on a host with no operator images, as an appended correction (GUARDRAILS 2.10 — append, never edit the archived line).

## What was checked and found sound

- **The two handoff controls fail for their intended reason only.** `claims-handoff-artifact` produces exactly `FAIL claim-handoff-artifact: CL-0001` and `: CL-0002`, exit 1, with `claim-dangling-path` absent — because `_good/tree` already carries `build/gate/dev/runner/toy.report` and the overlay carries `data/bench/workloads.tsv`, so both targets exist and only the new predicate can fire. `FORBID claim-dangling-path` therefore does its job.
- **`claims-handoff-row` fires on exactly line 3**, not line 1 (well-formed) and not line 2 (blank, skipped by `NF == 0`), and the `rows == 0` branch does not fire. `EXPECT` matched once.
- **`text-untagged-look-listen` fires on exactly `docs/x.md:1` and `:2`** and nothing else, and the same two lines with `[nc]` pass.
- **The rehearsal's comparison limb is semantically the gate's.** `route_filter`/`route_live` are byte-identical to `filter_report`/`live_count` after the rename; the `have` side uses the same `awk '/^#/ { next } { print }'` as `golden.sh:354`; the golden is the same file. Reproduced standalone against the real report: `want` 124 lines, `have` 124 lines, `cmp` clean. The dev-vs-release difference is covered because `golden.sh`'s own "Dev versus release" section pins dev against release, so comparing a *release* report to a dev-derived golden is not a second oracle.
- **`route_filter` cannot succeed vacuously on empty or non-report input.** `:174` requires `[ -s "$rep" ]`; `:176` requires `[ -s "$work/route/want" ]` with the explicit message "the filter matched nothing"; `have` is `data/golden/…digests`, a committed file. If the runner wrote an error to stdout, `want` would be empty and the mode fails.
- **The copy fails closed on key-list drift.** A copy missing a key class emits fewer lines, a copy with an extra class emits more; both change `cmp`. Removal of a class from `golden.sh` has the same effect in the other direction. The gap W-01 names is the `route_live` half and the absence of a drift assertion, not a vacuous `cmp`.
- **The rehearsal's extraction is bounded and safe.** `tools/release.sh:170` builds the archive with `git archive HEAD` under `--prefix="ivs-$tag/"`, so members are tracked files only: no absolute path, no `..`, and no symlink (`git ls-files -s | awk '$1=="120000"'` is empty). Extraction is `tar -xzf … -C "$work/route"` into a `mktemp -d` workspace, and both GNU tar and bsdtar strip a leading `/` or `../`, so even a hostile archive cannot write outside `$work`. `cmake --preset release` runs in a subshell `cd`'d into the extracted prefix, and `CMakePresets.json`'s `binaryDir` is `${sourceDir}/build/release`, so the cold build writes inside the extracted tree and nothing into the repository. The header's "it writes nothing into the repository" is true. Neither `tools/gate.sh` nor `.github/` nor `tools/check/` references `release-rehearse` (only `.planning/ops/RELEASE-OPS.md` prose does), so it cannot be run by accident from the gate or CI.
- **The archive name and prefix really are read at run time.** `first=$(tar -tzf … | sed -n '1p')`, `case $first in */)`, `prefix=${first%/}`, `[ -n "$prefix" ]`, `[ -d "$work/route/$prefix" ]` — four guards before the `cd`, and a version bump needs no edit.
- **POSIX shell.** `claims.sh` behaves identically under `/bin/dash` and `sh` for both the control tree (exit 1, the same single FAIL line) and the real tree (exit 0, the same pass line); `mktemp`/`trap … EXIT`/`trap 'exit 1' HUP INT TERM` are POSIX and clean up on the failure path; the `printf --` forms avoid the leading-`-` problem; `awk` is invoked with `NR` over a single file so `NR`/`FNR` cannot diverge; no `-v` assignment in the new code carries a backslash for awk to decode. `shellcheck -s sh` on the three changed scripts reports only pre-existing `SC2010` (`ls | grep`, lines 126/127/184/185, untouched by this phase) and an `SC2094` false positive on the `$rep` read in the step-4 pipeline.
- **The full `--source-route` mode runs green on this tree**: `rehearse: source-route: ivs-v0.1.0-source.tar.gz, golden match (report 131 lines, 124 of 124 compared), prefix ivs-v0.1.0`, last line `rehearse: ok`, exit 0.
- **Every command published in `docs/handoffs.md` and `docs/running.md` runs as published.** H7-1: `sh tools/listen-demo.sh demo-lamp-tone-pass` → `21674 bytes`, mode `0644`, `refpcm.sha256 e3b6638887947d64f7fa7d2be5687aff0eca85981b0ea79cfcfd3c74d057f1ad`, byte-identical to the doc. H7-2: exit 0, `report.end 130`, 45806 bytes, mode `0600`, `P6\n159 96\n255\n`; and re-running it on an existing path gives exit 14 with zero stdout lines, as documented. H8-3: `diag-stic2-mob-pass` → `report.end 81`, 45806 bytes, same header. D11: the documented two-argument form prints exactly `SKIPPED owner-run: no operator images (D11)` and exits 77. `docs/running.md`: 131 lines, `ivsr 3` first, `report.end 130` last, the nine-line tail byte-identical, seven images under `build/release/images`, `host.calls 1599` at `--slice-ticks 1000`, `outcome missing-fixture` with `--content-dir build/release`, `--frame-ppm` adds no report key (`diff` of the two reports empty), mode `0600`. All 124 golden body lines appear exactly once in the report, none missing, none duplicated.
- **`docs/presentation.md`'s measured references are current**: `runner/frame_export.c:12` is the `static const uint8_t fp_palette` line; `runner/frame_export.h:28` is the `ivs_frameppm_palette` declaration; `runner/frame_export.h:4` carries the literal `not STIC colour data`; `runner/main.c:113` carries it in the `--frame-ppm` help; `data/expected/pkg-install-files.ids` holds 11 rows; no file under `data/golden/` matches `palette`. `tools/check/core-symbols.sh:38-41` and `:163-173` name the rule and the refusal the document cites.
- **The `[nc]` waivers added to `docs/provenance.md:82` and `docs/video/demo-lamp-tone-1-derivation.md:268` are the retarget GUARDRAILS R5 *requires*, not an edit of a frozen oracle for convenience.** R5 says a public claim word on a line must carry `[claim:CL-NNNN]` or `[nc]`; both lines are descriptive prose (`the scan looks for`, `channel A sounds from`), neither asserts hardware behaviour, and the append-only rule (GUARDRAILS 2.10 / lesson 21) governs *oracle content*, which `[nc]` does not change.
- **`data/expected/important-gates-excluded.ids` holds 12 rows** (`REL-01 PKG-05 API-01 API-05 API-06 RUN-03 PKG-02 EV-02 EV-01 EV-03 EV-04 TOOL-04`), so the comment change "the six requirements it lists" → "the twelve" in `tools/check/steps/important-gates.sh:28` is correct and recomputable, not a stale quote. `tools/check/controls/*/` = 389 directories and `data/expected/controls/*.ids` = 389 rows, so `controls=389/389` recomputes. The three new registry rows resolve (`claims-handoff-artifact`, `claims-handoff-row`, `text-untagged-look-listen` all present in `data/expected/controls/claims.ids`).
- **The new predicate sits after the dangling-path check and the two are independent**, so a real (not toy) violation citing a `build/` path in a clean checkout emits both tokens. That is benign — the fix is the same — and it means the toy control's "existing file" construction is the only way to isolate the new token, which is what it does.

## Probes run

All run on this tree at `HEAD` = `5731f4a` (+ the untracked `10-VERIFICATION.md`), commits `d3952ea..HEAD`.

| # | Probe | Result |
|---|---|---|
| 1 | `diff` of the renamed rehearsal functions vs `golden.sh:146-156` and `:159-161` | empty on both — `LIVE: IDENTICAL`, `FILTER: IDENTICAL` |
| 2 | `grep -rn 'filter_report\|route_filter' tools/ .github/` | only `golden.sh:146,353` and the copy — no drift check exists |
| 3 | Extract `route_live`/`route_filter` from the rehearsal with `sed`, run on the real report, `cmp` with the golden body | `124 / 124`, `GOLDEN MATCH` |
| 4 | `route_live` on the four domain keys of the real report | `1 1 1 1` — the copy's discriminator is inert for this scenario |
| 5 | `claims-handoff-artifact` control tree run in isolation | `-- FAIL claim-handoff-artifact: CL-0001`, `: CL-0002`, exit 1, no `claim-dangling-path` |
| 6 | `claims-handoff-row` control tree in isolation | `-- FAIL claim-handoff-row: line 3` only, exit 1 |
| 7 | Scratch `_good/tree` + 3 claims (`./build/...`, `out/x.WAV`, `data/bench/workloads.TSV`), all targets present, `claims.ids` extended | `-- claims pass claims=3 provisional=0`, **exit 0** — accepted |
| 8 | Scratch `_good/tree` + claims `Build/...` and a `data/evidence -> ../build` symlink | `-- claims pass claims=2 provisional=0`, **exit 0** — accepted |
| 9 | Real tree, `data/expected/handoffs.ids` moved aside, `claims` step | `-- claims pass`, **exit 0** — silent pass |
| 10 | Real tree, ledger `chmod 000`, `claims` step | awk error on stderr, `-- claims pass`, **exit 0** — silent pass |
| 11 | `sh` and `dash` on the handoff-row control tree and on the real tree | identical output, same exit codes |
| 12 | Step `text.sh` regex with a word-boundary probe file | `looks-like`, `sounds-like` caught; `The sound is right.`, `The picture looked right.`, `Overlooking`, `Sounding` not caught |
| 13 | `sh tools/release-rehearse.sh --no-gate --source-route` on this tree | exit 0, `rehearse: source-route: ivs-v0.1.0-source.tar.gz, golden match (report 131 lines, 124 of 124 compared), prefix ivs-v0.1.0`, last line `rehearse: ok` |
| 14 | `docs/running.md`'s three commands | exit 0, 131 lines, `ivsr 3` … `report.end 130`, nine-line tail byte-identical |
| 15 | `host.calls` at `--slice-ticks 1000`; `--content-dir build/release` | `host.calls 1599`; `outcome missing-fixture` + `outcome.detail missing role=exec path=boot-stub-2.exec` |
| 16 | Golden body vs report, `grep -c -x -F` per line | 124 lines, 0 missing, 0 duplicated |
| 17 | `comm` of report keys against golden keys | excluded: `api.version audio.chain.check exec: grom: host.calls ivsr report.end` |
| 18 | `sh tools/listen-demo.sh demo-lamp-tone-pass` | `21674 bytes`, `refpcm.sha256 e3b66…f1ad`, mode `0644` — matches H7-1 |
| 19 | H7-2 / H8-3 `--frame-ppm` runs; re-run on an existing path | 45806 bytes, mode `0600`, `P6 / 159 96 / 255`; `report.end 130` and `81`; exit 14, 0 stdout lines |
| 20 | `sh tools/owner-run.sh` (bare) and with the two documented arguments | exit 2 + usage; `SKIPPED owner-run: no operator images (D11)` + exit 77 |
| 21 | `shellcheck -s sh` on the three changed scripts | only pre-existing `SC2010` ×4 and one `SC2094` false positive |
| 22 | Tracked symlinks; `git archive` prefix; `binaryDir` of the release preset | none; `git archive HEAD --prefix="ivs-$tag/"`; `${sourceDir}/build/release` |
| 23 | Recompute: `data/expected/important-gates-excluded.ids` rows; control dirs vs inventory rows | 12; 389 = 389 |
| 24 | `git status --porcelain` after every probe | only `.planning/state.json` (orchestrator) and the untracked `10-VERIFICATION.md`; no source file modified |

## REVIEW COMPLETE

**2 BLOCKERs, 6 WARNINGs, 2 INFOs.** The two blockers are the same defect in two places: EV-05's ledger and EV-05's artifact refusal are both satisfiable while the evidence is missing or wrong. Neither needs new machinery — B-01 is a required-file guard plus an id pin, B-02 is one path normalisation — and both are provable with committed stand-ins the phase already knows how to write.

## Fixes applied

Applied by the code fixer on the same branch, one commit per finding. Counts are from the full gate run after the last commit; `controls` is the only token that moves, and it was predicted before the commit that moves it. W-04, W-05 and both INFOs are left as findings (prose-only, out of the requested scope).

### B-01 — the ledger is required and its row set is pinned (`b693b12`)

**Token:** `claim-handoff-row` with a typed detail, the convention this file already uses for the grammar (`line <n>`, `no rows`): `no ledger`, `unreadable`, `unknown id <id>`, `missing id <id>`. One token with a detail keeps each control's `EXPECT` a single readable line while the four cases stay distinguishable.
**Controls:** `claims-handoff-missing` (`MAKE delete data/expected/handoffs.ids`, `FORBID claims pass`), `claims-handoff-unreadable` (the path is a directory, assembled at run time, so the case does not depend on `chmod` surviving a root gate), `claims-handoff-id-missing` (grammar clean, `D11` row dropped) and `claims-handoff-id-extra` (grammar clean, one `X-1` row). The good tree carries a five-row ledger, so the positive control still passes with the file required.
**Delta:** predicted `controls=389` to `393`, every other token flat. Measured `controls=393/393`; `steps=38/38`, `claims=28`, `gates=109`, `lines=333`, `doc-files=62`, `docs=63`, `claim-refs=26`, `tests-*` unchanged.

### B-02 — the predicate is normalised before it matches (`7bc5a66`)

**Token:** unchanged (`claim-handoff-artifact <id>`); the refusal now fires on the resolved, relative, lower-cased path. `ROOT` is resolved too, because a tree reached through a symlink (a worktree under `/tmp`, a link on Darwin) otherwise made every relative path a `../` path and disarmed the two prefix clauses.
**Stand-in:** `claims-handoff-artifact` now carries eight claims — the five refused classes (`build/` path, `.wav`, `.ppm`, bench `.tsv`, the ledger itself) plus `./build/...`, `Build/...` and a run-time `symlink x build` case. `FORBID` adds `claim-handoff-row` and `claim-id-extra`, so the control proves isolation.
**Residual, recorded not papered over:** a symlink whose target lies outside the repository normalises to a `../`-relative path that no clause matches; it is refused by `claim-dangling-path` only when broken. Written into the token prose at `tools/check/kinds/claim.cmake`.
**Delta:** predicted 0 (same control, extended). Measured 0.

### W-01 — the copy is asserted equal to its source (`29083f0`)

Two extraction compares inside `tools/release-rehearse.sh` (a line in the rehearsal itself, not a gate step: the copy exists only in that script, so a gate step would run on every gate to protect a release-time path, and `runner-selftest` never reads the file). Run on every rehearsal, not only with `--source-route`, so a stale copy is caught even by a run that does not use it. Verified: both pairs identical today, and a one-character mutation of the copy is detected. **Delta:** 0, as predicted.

### W-02 — `claims-handoff-row` has a `FORBID` (`aff2f92`)

Names the siblings that must not fire: `no ledger`, `unreadable`, `no rows`, `line 1`, `line 2`, `missing id`, `unknown id`, `claim-dangling-path`, `claim-order`, `claim-id-extra`. **Delta:** 0, as predicted.

### W-03 — the release procedure invokes `--source-route` (`99a9827`)

`.planning/ops/RELEASE-OPS.md` (g) now gives the pre-tag invocation `sh tools/release-rehearse.sh --release --source-route`, the line it must print, and the rule that a skipped mode is recorded as `not run`; the flag list in the bullet at `:8` gains `[--source-route]`. **Delta:** 0, as predicted.

### Not fixed, deliberately

W-04 (the nine-line tail is a hand-quoted figure), W-05 (the excluded-key enumeration is wrong), I-01 (the look/listen word list) and I-02 (the D11 command line in the verification record) are left in place: the first two want a new pinned data file or a prose rewrite the task did not ask for, and neither INFO is one line.
