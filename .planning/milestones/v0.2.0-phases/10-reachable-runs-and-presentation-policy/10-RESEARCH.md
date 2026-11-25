# Phase 10: Reachable Runs and Presentation Policy - Research

**Researched:** 2026-11-24
**Domain:** release rehearsal, documented operator route, presentation/palette policy, human-handoff ledger, gate-step extensions
**Confidence:** HIGH for everything measured in the tree; MEDIUM for the two prose decisions the frozen criteria leave ambiguous (see Q4 and Q11).

## Summary

Three of this phase's five deliverables are **smaller than the roadmap sentence
implies**, and one of them is **already delivered**. The colour-symbol refusal
exists (`tools/check/core-symbols.sh:163-173`, rule `palette|rgb|ppm|framepreview`)
with its committed stand-in (`tools/check/controls/core-symbols-palette-defined/`)
and it passes in today's gate (`control ok   core-symbols-palette-defined`,
`/tmp/g10-baseline-sh.out:220`). The release rehearsal already builds both
archives and already proves the source archive is a function of the commit; it
needs one extraction, one cold build and one golden comparison. And the operator
route **works today, measured end to end**: I built the real source archive
(`ivs-v0.1.0-source.tar.gz`, 5,673,163 bytes, 2571 members), extracted it into a
clean directory outside the repository, ran `cmake --preset release` +
`cmake --build --preset release` (244 targets, 2.5 s wall, 19.4 cpu-s), ran
`ivs-run` on `demo-lamp-tone-pass`, and the report filtered by the gate's own
golden key list is **byte-identical to `data/golden/demo-lamp-tone-pass.digests`**
(124 lines both).

The two real findings are adversarial. **First, the tree is red at phase entry**:
`steps=36/38`, because `hygiene` and `hygiene-history` both fail on one literal in
`.planning/phases/09-registered-evidence-and-gate-cost/09-SECURITY.md:122`
(a home-relative example path, quoted in the sentence). Criterion 5 ("the gate summary keeps
`steps=38/38`") cannot be met until that one line is fixed, and the fix is not
Phase 10's content — it is a one-line correction to a Phase 9 record. **Second,
criterion 4's premise is false**: the sentence "nothing downloadable is runnable"
appears in `docs/packaging.md:121` and **nowhere in `README.md`** (measured:
`grep -c -F` gives 1 and 0). It cannot be "kept identical" because it was never
identical; `README.md` has to gain the phrase.

**Primary recommendation:** land the entry fix first as its own commit (one line in
`09-SECURITY.md`, restoring `steps=38/38` and making every later count delta
measurable); then follow CONTEXT's order — frozen documents (`docs/running.md`,
`docs/presentation.md`, `data/expected/handoffs.ids`, the handoff packet) in
commits *before* the checks that enforce them, proved with
`git merge-base --is-ancestor`; then the two guards (one `claims` hook rule, one
`text` word-list extension) each with one control; then the rehearsal mode last,
because it is the only piece that is release-time and gate-free.

### Provenance labels used in this document

- `[OBS]` observed in the repository or in a command's output in this session,
  with the path and line or the command and its output.
- `[INF]` inference from an observation, stated as such.
- `[REC]` recommendation for the plan.
- `[UNK]` not determinable from the tree in this session.
- `[ASSUMED]` training knowledge, not verified here. This document contains
  none: every claim below is a file read or a command output. The two
  interpretation risks that would normally be assumptions are recorded as
  *open questions* with the measurement beside them, not as assumed facts.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

**The documented route (PKG-05)**

- **`docs/running.md` is the only route document, and it carries three commands
  and the exact expected tail.** `cmake --preset release`, `cmake --build
  --preset release`, then `ivs-run` on `demo-lamp-tone-pass`; the exact stdout
  tail is quoted byte for byte, together with where the `.ppm` and `.wav` land.
  **No helper script** — a helper would need either zero logic (a document with a
  shebang) or a new gate id, and both are worse than three lines a person can
  read. — **Reversibility:** reversible — one markdown file.
- **The proof is the release rehearsal, not a gate step.** `tools/release-rehearse.sh`
  gains a mode that unpacks the **source archive** into a clean directory, builds
  it, and runs `demo-lamp-tone-pass` to its golden; the transcript is recorded at
  release time. The gate already pays for a cold build and `release-assets`
  already checks archive membership, so a second cold build in CI would be
  duplicate cost. — **Reversibility:** reversible — one script plus a recorded
  transcript.
- **The route cannot become an install claim.** The "nothing downloadable is
  runnable" sentence is kept **identical** in `README.md` and `docs/packaging.md`
  (a grep proves exactly one matching line in each) so release text and README
  cannot disagree, and both gain one paragraph pointing at `docs/running.md`.

**The presentation and palette policy (TOOL-04 narrowed)**

- **`docs/presentation.md` is frozen before its check exists**, with the six
  clauses: the core publishes palette **indices** only; exactly one operator
  preview palette exists, labelled "not STIC colour data", never installed, never
  in a golden, digest, assert or claim; the project makes **no colour claim** for
  any profile; a host chooses palette, scaling and aspect; "index frame" is the
  published plane and "picture/render/looks like" are never evidence; a real
  colour table would be a profile field with a source tier. — **Reversibility:**
  reversible — a document plus a check.
- **One deny-list check, inside an existing step, no new step.** A colour symbol
  defined under `core/src` is refused by the `core-symbols` deny list, which
  already has the machinery and a control (`core-symbols-palette-defined`). The
  check reuses that step and that control rather than adding a second mechanism
  for one property. — **Reversibility:** reversible — a control file.
- **The stand-in is committed and re-runnable**, per GUARDRAILS §2.3, and it must
  fail for its intended reason only.

**The handoff ledger (EV-05)**

- **`data/expected/handoffs.ids` is created here** with H7-1, H7-2, H8-3, D10 and
  D11; **every row is `evidence: none`** and each is dispositioned `handed-off`
  or `not-performed`. A handoff is **never** a pass, never blank, and never a
  phase exit criterion a tool cannot meet. — **Reversibility:** costly — the file
  becomes the project's record of what a human owes, and a later milestone cites
  the row id instead of re-asking; changing a row's meaning after the fact would
  re-open a closed question.
- **Two guards, both shown able to fail.** (i) The `claims` step refuses a claim
  citing a handoff artifact, a `build/` path, a `.wav`, a `.ppm` or a bench
  `.tsv`. (ii) The `text` step's claim-word list gains the look/listen words, so
  "looks like" or "sounds right" in prose needs a claim tag. —
  **Reversibility:** reversible — an ids file plus a control each.
- **H7-1 and H7-2 get a handoff packet** — the exact commands, the deterministic
  identities, and the report digest lines a human would compare — so the owner
  can perform the check without reconstructing it. The packet is a document, not
  a gate step.

**Ordering and evidence**

- **The policy documents are committed before the checks that enforce them**
  (oracle-first, GUARDRAILS §2.9), and the ancestry is proved with
  `git merge-base --is-ancestor`.
- **No new gate step.** Every new mechanism lands as an ids line, a control or a
  selftest case inside a suite that already exists. The predicted deltas are
  stated per commit before it lands.
- **`claims=` does not move.** No claim is added, none is raised, and the
  documented route's ceiling is `installed-host-behavior` — which is a
  *ceiling*, not a claim to be minted here.

**Decided autonomously**

- [Route] Q: "Document or helper script for the three commands?" → Selected: "Document only; a helper needs zero logic or a new id." — one markdown file, reversible.
- [Route] Q: "Where is the reachability proof taken?" → Selected: "In the release rehearsal, not in CI." — the gate already pays for a cold build; a second one in CI is duplicate cost.
- [Policy] Q: "A new deny-list step or the existing `core-symbols` one?" → Selected: "The existing step, which already has the machinery and the control." — a control file, reversible.
- [Ledger] Q: "Ids file grammar: one row per handoff with `evidence:`?" → Selected: "`<id> <disposition> evidence: none`, one line each, five rows." — an ids file, reversible.
- [Guards] Q: "One combined guard or two?" → Selected: "Two: the `claims` step for artifacts, the `text` step for words. They fail for different reasons and a single guard would blur which one fired." — two controls, reversible.

### Claude's Discretion

The plan chooses whether the rehearsal mode is a new flag on
`tools/release-rehearse.sh` or a mode of an existing one; how the handoff packet
is laid out; and whether the `text` claim-word addition needs one control or two.

### Deferred Ideas (OUT OF SCOPE)

- **Installing the runner or any image** — D14, structurally blocked.
- **A windowed frontend, a palette table, a second cartridge** — REQUIREMENTS
  "Out of Scope".
- **Any new gate step** — forbidden this milestone.
- **Running H7-1/H7-2** — the owner's, not a tool's; the ledger row is the
  deliverable.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| PKG-05 | A person can build and run the emulator from the released source archive using documented commands only, and the release rehearsal proves that route end to end against the demo cartridge golden from a moved prefix | Q2 (the rehearsal mode, exact edit points, archive name), Q3 (the exact route and the measured tail, proven in a clean directory). Measured today: the route works; the golden matches byte for byte. |
| TOOL-04 *(narrowed)* | A frozen presentation and palette policy keeps colour, scaling and aspect out of the core and out of every claim, and a check refuses a colour symbol in the core | Q5: the refusal **already exists** (`core-symbols.sh:163-173`) with its committed stand-in and a passing control; the new work is `docs/presentation.md` plus citing the check by path and line. The one operator preview palette is `runner/frame_export.c:12`. |
| EV-05 | A handoff ledger (`data/expected/handoffs.ids`) records every human-only item (H7-1, H7-2, H8-3, D10, D11) with `evidence: none`, a claim may not cite a handoff artifact, and a later milestone cites the row instead of re-asking | Q8 (the ids grammar, measured from four existing files, and how the `claims` hook would read it), Q9 (each of the five items exactly as the project records it), Q6 (the exact extension point and the control shape). |
</phase_requirements>

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|--------------|----------------|-----------|
| The documented build-and-run route | **Public prose** (`docs/running.md`) | — | A route a person follows is text; CONTEXT rules out a helper script because zero-logic scripts and new gate ids are both worse. |
| The reachability proof | **Release tooling** (`tools/release-rehearse.sh`, release-time) | — | Not a gate step (criterion 1, GUARDRAILS §2.1). The gate already pays for one cold build. |
| The presentation/palette policy | **Public prose** (`docs/presentation.md`) | **Gate step** (`c-purity` -> `core-symbols.sh`) | The document is the oracle; the enforcement is a defined-symbol rule inside an existing step. |
| The colour-symbol refusal | **Gate step** (`c-purity`, via `tools/check/core-symbols.sh`) | **Ctest suite** (`core-symbols`, on the shipped archive) | Two callers already exist; no new step. |
| The handoff ledger | **Data** (`data/expected/handoffs.ids`) | **Gate step** (`claims`) | The ledger is data read by a checker; it is never prose (AUTONOMY §3). |
| The claim/handoff guard | **Gate step** (`claims`, via `tools/check/kinds/claim.cmake`) | — | The `artifact` field of `data/claims.json` is the one place a claim names a file. |
| The look/listen word guard | **Gate step** (`text`) | — | The claim-word list already lives there with one control per word. |
| The operator preview palette | **Runner host code** (`runner/frame_export.c`, not installed) | — | Outside the core and outside the install list (R7, 11 files). |

## Q1 — The entry state: the tree is red, and the exact reason

`[OBS]` `sh tools/gate.sh` on this tree (HEAD `f3a6c67`, clean worktree) ends:

```
gate-time total=82
gate: FAIL steps=36/38 controls=386/386 ... files=2571 ... docs=60 ... claims=28 provisional=27 ...
```

with exactly two step failures and **one root cause**:

```
-- FAIL hygiene-home-path: .planning/phases/09-registered-evidence-and-gate-cost/09-SECURITY.md
-- FAIL gate-step-failed: hygiene
-- FAIL hygiene-history-home-path: 08985f30b7
-- FAIL gate-step-failed: hygiene-history
```

`[OBS]` The match is the home-relative form `HOME_NEW` describes, taken from a
*quotation of the check itself* at
`.planning/phases/09-registered-evidence-and-gate-cost/09-SECURITY.md:122`:

> `SC2088` on the deliberate literal a control writes as a home-relative path
> (the sentence quotes that literal, and the quotation is what the rule matches)

`[OBS]` `tools/hygiene.sh:42` defines `HOME_NEW`, which includes an alternative
of the form slash-tilde followed by three or more name characters, and
`tools/hygiene.sh:120` applies it to every tracked file;
`tools/check/steps/hygiene-history.sh` commits the whole FILELIST into a
throwaway repository, so the same line is also the `hygiene-history-home-path`
finding (the reported short sha `08985f30b7` is that throwaway commit, which is a
function of the file list, not a real repository commit).

`[INF]` One edit to that one sentence clears both steps: `steps` returns to
`38/38` and no other token moves (`hygiene-files`, `files` and `tracked` count
files, not content).

`[REC]` Make it **commit 1 of the phase, alone, before any other change**, with
the predicted delta `steps 36/38 -> 38/38` and every other token unchanged. Two
reasons: criterion 5 cannot be verified otherwise, and every later count delta
must be measured against a green baseline (GUARDRAILS §3, "An unexpected token is
a stop, not a re-bless"). If the orchestrator prefers to fix it outside the phase,
the plan's first task must still *verify* `steps=38/38` before touching anything.

`[REC]` The correction is a live-record edit, not an archive rewrite:
`.planning/phases/09-.../09-SECURITY.md` is in the live phase directory, not in
`.planning/milestones/`, so GUARDRAILS §2.10 (append-only archives) does not
apply. Keep the sentence's meaning and replace only the literal, e.g. name the
form without a tilde prefix.

## Q2 — `tools/release-rehearse.sh`: the mode, the exact edit points, the archive name

### What exists

`[OBS]` `tools/release-rehearse.sh` is 164 lines, POSIX sh, and runs six steps in
order (`:9-28`):

| Step | Lines | What it does |
|---|---|---|
| 1 | `64-77` | `sh tools/release.sh [--release] [--no-gate] --out "$work/a"` into a first temp dir |
| 2 | `79-87` | the same dry run again into `"$work/b"`, always `--no-gate` |
| 3 | `91-99` | the two source archives must be byte-equal (`cmp -s`), and their names must match |
| 4 | `101-135` | the two install archives: `equal`, or `equal-except-ranlib-stamp` on Darwin, or `DIFFERENT` (exit 1) |
| 5 | `137-153` | the single tag glob of `.github/workflows/release.yml` tried on seven sample tags |
| 6 | `155-163` | `sh tools/release-readiness.sh --allow-dirty`, sections A to I printed plus the `P4:` line |

`[OBS]` There are exactly two modes today: `--no-gate` (`:42`) and `--release`
(`:43`). Flag parsing is the loop at `:38-48`; the help text is the comment block
at `:3-29`, and `usage()` prints exactly `sed -n '3,29p'` (`:33`) — **the comment
block and the `sed` range must move together**.

### What it builds, and where it writes

`[OBS]` `tools/release.sh:170-171` writes the source archive as
`git archive --format=tar --prefix="ivs-$tag/" HEAD | gzip -n`, and
`tools/release.sh:166` names it `ivs-$tag-source.tar.gz`. `[OBS]` The tag
defaults to `v$hdrver` from `core/include/ivs/ivs.h:23-25`
(`IVS_API_VERSION_MAJOR 0u`, `MINOR 1u`, `PATCH 0u`), so today the archive is
**`ivs-v0.1.0-source.tar.gz` with prefix `ivs-v0.1.0/`**. Measured:

```
release:   ivs-v0.1.0-source.tar.gz 5673163 bytes
release-assets source ok members=2571
```

`[OBS]` Every temporary path is under one `mktemp -d "${TMPDIR:-/tmp}/release-rehearse.XXXXXX"`
(`:56`) removed by `trap 'rm -rf "$work"' EXIT` (`:57`), so the new mode needs no
cleanup code of its own.

`[OBS]` The tool preflight at `:53` already requires `git tar gzip awk cmake cmp
mktemp find sort`; the new mode needs nothing beyond that (`tar -tzf` and
`tar -xzf` are covered by `tar`, the runner is built by `cmake`).

### What the new mode has to do — measured, not guessed

`[OBS]` I ran the whole route by hand against the real archive:

```
sh tools/release.sh --no-gate --out /tmp/rel10          # 1.1 s
mkdir /tmp/clean10 && cd /tmp/clean10
tar -xzf /tmp/rel10/ivs-v0.1.0-source.tar.gz            # -> ./ivs-v0.1.0/
cd ivs-v0.1.0
cmake --preset release                                  # 0.2 s configure
cmake --build --preset release                          # 244 targets, real 2.533 s, user 13.237 s, sys 6.122 s
./build/release/runner/ivs-run --content-dir build/release/images \
  data/scenarios/demo-lamp-tone-pass.ivss > report.txt  # exit 0
```

The clean directory has **no `.git`** and the configure still succeeded, and the
build produced `build/release/runner/ivs-run` plus the six images
(`boot-stub.exec`, `boot-stub-2.exec`, `diag-cpu-1.ivsc`, `diag-ctrl-psg-1.ivsc`,
`diag-stic-1.ivsc`, `diag-stic-2.ivsc`, `lamp-tone.ivsc`). The report is
**byte-identical** to the repository's own dev-tree report (`diff` empty, 131
lines, `host.calls 11`, `report.end 130`).

### How the golden comparison is already done, and how the rehearsal reuses it

`[OBS]` The gate's `golden` step compares a `.digests` golden in exactly four
lines (`tools/check/steps/golden.sh`):

- `live_count()` (`:159-161`): a domain is observed iff its count key is a
  decimal other than 0.
- `filter_report()` (`:146-156`): the golden key list, as one awk program over
  the report.
- `:353-355`: `filter_report ... < "$rep" > want`; `awk '/^#/ {next} {print}'
  "$ROOT/$g" > have`; `cmp -s want have` else `golden-mismatch`.

`[OBS]` Reproducing those three lines against the clean tree's report gives:

```
live: video=1 audio=1 input=1 psglog=1
GOLDEN MATCH: the source-archive build reproduces data/golden/demo-lamp-tone-pass.digests byte for byte
     124 /tmp/clean10/want
     124 /tmp/clean10/have
```

**So the route is proven today; the mode only has to script it.**

`[REC]` Reuse the *committed golden* as the oracle rather than inventing a second
comparison: the rehearsal's assertion is `filter(report) == body(golden)`, the
same equality the gate performs, against the same committed file. The awk program
must be copied into `release-rehearse.sh` (there is no `.awk` file anywhere in the
tree — `[OBS]` `git ls-files | grep -c '\.awk$'` = 0 — and `filter_report` is a
shell function inside a step that needs `ROOT`, `FILELIST` and a populated
`build/gate/<preset>/runner/`, so it cannot be sourced from the rehearsal).

`[REC]` The duplication is **safe because it fails closed**: if the golden key
list ever changes and the golden is re-blessed, a stale copy in the rehearsal
produces a different set and the `cmp` fails loudly. A drifted duplicate cannot
pass. Record that argument in the script header and in the plan, and cite
`tools/check/steps/golden.sh:146-161` as the source of truth.

`[REC]` Read the prefix from the archive instead of re-deriving the tag:
`prefix=$(tar -tzf "$arc" | head -n 1)` and strip the trailing slash. Re-deriving
`v$hdrver` in a second place would be a second source for one fact.

### The exact edit points

`[REC]` Region by region, with line numbers from the current file:

| Region | Lines | Change |
|---|---|---|
| Header comment block | `3-29` | add the new mode to the ordered list and to the `--` option paragraph; **keep the block ending at line 29 or move the `sed` range at `:33` in the same edit** |
| Flag parse loop | `38-48` | add the flag (e.g. `--source-route`) and its `1` |
| Preflight | `53` | none — every tool needed is already listed |
| Step 3 (source archives) | `91-99` | unchanged; the new mode consumes `$work/a/$src_a`, which step 3 has already proved equal to run 2's |
| **New step, inserted after `:99`** | — | `mkdir "$work/route"`; `tar -xzf "$work/a/$src_a" -C "$work/route"`; `cd` into the prefix; `cmake --preset release`; `cmake --build --preset release`; run `ivs-run` into `"$work/route/report"`; filter; `cmp` against `"$ROOT/data/golden/demo-lamp-tone-pass.digests"`; print one `rehearse: source-route: ...` line |
| Step 6 tail | `163` | unchanged; `rehearse: ok` stays the last line |

`[REC]` Print one greppable result line, in the style of the existing ones, so the
transcript CONTEXT asks for is one line a reader can cite, e.g.
`rehearse: source-route: ivs-v0.1.0-source.tar.gz built and ran demo-lamp-tone-pass to its golden (124 lines, prefix ivs-v0.1.0)`.

`[REC]` Fail closed on each sub-step with the existing `fail()` helper (`:36`), so
a broken route is `exit 1` and never a silent skip (PLAN-LESSONS 24).

`[INF]` Cost of the mode when it runs: one extraction plus one cold build, about
2.5 s wall and 19.4 cpu-s on this host — release-time only, never in CI, so it
moves no hosted figure.

## Q3 — `docs/runner.md` and the exact route

### What it already documents

`[OBS]` `docs/runner.md` is 286 lines. Relevant regions:

- `:9` "The runner is not installed (D-24): it is built in both presets
  (`build/<preset>/runner/ivs-run`) for the gate".
- `:94-95` the one documented invocation shape in a fenced block:
  `build/dev/runner/ivs-run --content-dir build/dev/images data/scenarios/diag-boot-pass.ivss`.
- `:247-259` the nine `--update-golden` writing commands, all in the
  `build/dev/runner/ivs-run --content-dir build/dev/images ...` form.
- `:263` "The tracked goldens are now written by the release runner
  (`build/release/runner/ivs-run --content-dir build/release/images`; the dev and
  release reports are byte-equal, which the step checks)".
- `:38-40` the operator output options: `--video-raw`, `--frame-ppm`
  ("the LAST delivered frame as a binary PPM (P6, 255) ... in the operator
  preview palette ... `operator preview palette, not STIC colour data`,
  `runner/frame_export.h` ... created exclusively with mode 0600 ... An exported
  frame is never evidence").
- `:35`, `:195-212` the WAV: `--audio-wav` writes "the dense audio as a mono
  16-bit WAV made by the reference converter", and `tools/listen-demo.sh` renders
  one scenario "into `build/listen/<id>.wav` (gitignored; a `.wav` is never
  committed)".

`[OBS]` **`docs/runner.md` never names `cmake --preset release` or
`cmake --build --preset release`, and never gives a release-preset invocation
line as a command a person runs.** It documents the *artefact locations* and the
*golden-writing commands*, not the route. `[INF]` So `docs/running.md` adds a
route rather than contradicting one, and the only overlap to police is the
`--content-dir build/release/images` spelling, which must be identical in both.

### The exact tail, measured

`[OBS]` `./build/release/runner/ivs-run --content-dir build/release/images data/scenarios/demo-lamp-tone-pass.ivss`
exits 0 and prints 131 lines; the last nine are, byte for byte:

```
audio.sample_ticks 16
audio.chain.check pass
output.video.frames 13
video.geometry 159x96
video.row_lines 2
video.frame.last_tick 776568
video.chain.check pass
host.calls 11
report.end 130
```

`[OBS]` `host.calls` is the one host-dependent key (`docs/runner.md:114`, "it
counts host service calls, so it changes with the slice size"). Measured:
`host.calls 11` at the default `--slice-ticks 4194304`, `host.calls 1599` at
`--slice-ticks 1000`. It is stable across presets and across the clean tree
(both 11).

`[OBS]` The `.ppm` and `.wav` landing sites, measured:

| Output | Command | Path | Size | Mode |
|---|---|---|---|---|
| WAV | `sh tools/listen-demo.sh demo-lamp-tone-pass` | `build/listen/demo-lamp-tone-pass.wav` | 21674 bytes | 0644 |
| PPM | `ivs-run --frame-ppm <file> ...` | wherever the operator points it; no default | 45806 bytes (`P6\n159 96\n255\n` + 159x96x3) | 0600 |

`[OBS]` `listen-demo.sh` prints `refpcm.sha256 e3b6638887947d64f7fa7d2be5687aff0eca85981b0ea79cfcfd3c74d057f1ad`
for the pass scenario. `[OBS]` `--frame-ppm` adds **no report key at all**
(`docs/runner.md:40`, "exit code and report unchanged"), so the picture's only
deterministic identity is the scenario's own frame hash:
`assert frame 12 hash == a7b9889b136cf47b2586def7f992c7caaf19c451e043a9a676b2f838bd91c2d7`
(`data/golden/demo-lamp-tone-pass.expect:81`), and frame 12 is the last of the 13
delivered frames.

`[REC]` `docs/running.md` must carry: the three commands; the nine-line tail
above with the sentence that `host.calls` is the one host-dependent line and 11 is
its value at the default slice; the `.wav` path and its gitignored status; the
`.ppm` as an operator-chosen path with mode 0600 and "never evidence"; and the
pointer that the golden is `data/golden/demo-lamp-tone-pass.digests`.

`[REC]` If the plan prefers a tail that is *unconditionally* byte-exact, quote the
eight lines up to `video.chain.check pass` and give `host.calls 11` /
`report.end 130` as two separately explained lines. Either reading is honest;
record which one the plan chose and why.

## Q4 — `README.md` and `docs/packaging.md`: the sentence is not there twice

`[OBS]` Measured with the canonical string `nothing downloadable is runnable`:

```
README.md            lines matching: 0
docs/packaging.md    lines matching: 1
```

`[OBS]` `docs/packaging.md:121` is the only match, and it is a bullet whose
sentence continues past the phrase:

> `- Neither the runner (`ivs-run`), nor `ivs_pack`, nor `ivs_asm` is installed or shipped, so nothing downloadable is runnable yet: a reader builds the runner and the image tools from the source tarball.`

`[OBS]` `README.md:17` says the same thing in different words and contains none of
the phrase:

> `Releases, when they exist, are listed on the repository's releases page. ... carries the source archive, the installed package archive and the checksums; none of it can run a program, because neither the runner nor `ivs_pack` nor `ivs_asm` is installed. ...`

`[OBS]` `CHANGELOG.md:12` carries a third phrasing (`nothing downloadable executes
a program`) chosen in plan 08-01 precisely because `runs` is a claim word
(`.planning/milestones/v0.1.0-phases/08-.../08-01-SUMMARY.md:130`).

`[INF]` CONTEXT's "is kept **identical**" is therefore a **premise that does not
hold**: the two files were never identical, so the phase cannot keep what does not
exist. It must *make* them identical, which means editing `README.md:17`.

`[REC]` Define the checked object as the 34-byte substring, not the whole line, and
check it line-wise as the roadmap asks:

```sh
s='nothing downloadable is runnable'
[ "$(grep -c -F -- "$s" README.md)" -eq 1 ] && [ "$(grep -c -F -- "$s" docs/packaging.md)" -eq 1 ]
```

That satisfies "a grep proves a single matching line in each" while tolerating
packaging.md's `- ` bullet prefix. A `grep -x` on the whole line would be
**unsatisfiable** in packaging.md without destroying its bullet list.

`[REC]` Edit `README.md:17` minimally so it contains the phrase and keeps its
meaning, e.g. `... none of it can run a program: nothing downloadable is runnable,
because neither the runner nor `ivs_pack` nor `ivs_asm` is installed.` Leave
`docs/packaging.md:121` alone — it already matches, and "do not edit a sentence
that is still true" is the Phase 8 rule
(`.planning/milestones/v0.1.0-phases/08-.../08-20-PLAN.md:158`).

`[REC]` Both files then gain one paragraph pointing at `docs/running.md`. Neither
paragraph may contain a claim word (`supports|compatible|passes|accurate|verified|runs|plays|playable`,
`tools/check/steps/text.sh:45`) unless the line carries `[claim:CL-NNNN]` or
`[nc]`. `runnable` is **not** a claim word; `runs` is, and `-w` does not match it
inside `runnable`.

## Q5 — The colour symbol: the refusal already exists

### How a colour symbol is refused today

`[OBS]` `tools/check/core-symbols.sh` carries a **second defined-symbol rule**
always on (`:38-41`, `:163-173`):

```awk
if (tolower(n) ~ /(palette|rgb|ppm|framepreview)/) print n
```

and each hit becomes `-- FAIL core-symbols-palette-defined: <symbol> in <file>`
(`:173`). The rule runs over `nm -P` of each archive, excluding undefined and
weak-undefined types and Darwin assembler-local labels, so it catches a
file-local `static` too — i.e. a colour table defined anywhere under `core/src`,
whatever its linkage. `[OBS]` `tools/check/core-symbols-deny.txt:56-59` labels the
same four words for *imports*, and `:52-55` labels the PCM family for the
sibling rule.

`[OBS]` Two callers exist: `tools/check/steps/c-purity.sh` (fresh `-O0` and `-O2`
archives, `:39-47`) and the ctest suite `core-symbols`
(`tests/core-symbols-tap.sh`, the shipped archive, imports only).

### What `core-symbols-palette-defined` is

`[OBS]` `tools/check/controls/core-symbols-palette-defined/` holds three files:

- `CHECK` -> `c-purity`
- `EXPECT` -> `FAIL core-symbols-palette-defined:`
- `overlay/core/src/cpu/good.c` -> the good tree's stand-in source with one extra
  definition, `int ivs_good_palette_lookup(void) { return 3; }`, and its header
  comment names plan 06-09, D-12 and D-13.

`[OBS]` It is green in today's gate: `control ok   core-symbols-palette-defined`
(`/tmp/g10-baseline-sh.out:220`). `[OBS]` There is no `FORBID` file; the guard is
the EXPECT token itself — `tools/check/run-controls.sh:225-240` requires every
EXPECT line to occur as a fixed string and fails `control-token-missing`
otherwise, so a control that produced a *different* failure cannot pass.

`[INF]` **Success criterion 2's mechanism limb is already satisfied**: "one
deny-list check refuses a colour symbol defined under `core/src`, shown able to
fail by a committed scratch stand-in; the check adds no gate step." All three
clauses are true today, measured.

`[REC]` Do **not** add a second mechanism. `docs/presentation.md` cites
`tools/check/core-symbols.sh:38-41` and `:163-173` by line, and the plan records
that the enforcement pre-dates the policy (which inverts the usual oracle-first
order — see Q11). If the plan wants a *policy-specific* stand-in in addition, the
cheapest is a second EXPECT-carrying control with a different colour word (e.g.
`ivs_good_rgb_lookup`), which costs one control directory and one inventory line;
that is optional, not required by any frozen sentence.

### Where the operator preview palette lives

`[OBS]` Exactly one exists:

- `runner/frame_export.h:4` "The palette is the operator preview palette, not
  STIC colour data"; `:20` "Entries of the preview palette: indices 0..15, then
  the blank value"; `:28` `int ivs_frameppm_palette(uint32_t i, uint8_t rgb[3]);`
- `runner/frame_export.c:12` `static const uint8_t fp_palette[IVS_FRAMEPPM_PALETTE_ENTRIES][3]`
- `runner/main.c:113` the `--help` text names it "not STIC colour data".

`[OBS]` It is not installed: `data/expected/pkg-install-files.ids` holds 11 paths
and no runner file (`prefix-files=11` in the summary line). `[OBS]` No golden,
digest, assert or claim names it: `grep -n palette data/golden/*` is empty and no
`artifact` in `data/claims.json` is a `.ppm` (Q6 table).

## Q6 — `tools/check/steps/claims.sh`: how it decides, and the exact extension

### The mechanism

`[OBS]` `tools/check/steps/claims.sh` is 13 lines and owns nothing: it runs
`cmake -DROOT="$ROOT" -DKIND=claim -P "$CHECK_DIR/records.cmake"` (`:8`), requires
a `-- claim pass records=<n> ...` line (`:11-12`), and reprints it as
`-- claims pass records=<n> ...` (`:13`). Everything is in:

- `tools/check/schema/claim.txt` — `file data/claims.json`;
  `inventory data/expected/claims.ids`; `keys id statement profile tier artifact
  depends_on standing note`; `enumfrom tier data/tiers.json evidence_tiers id`;
  `match standing ^(established|provisional)$`; `hook claim`.
- `tools/check/kinds/claim.cmake` — the hook.

`[OBS]` "A claim cites an artifact" is the `artifact` string field, and it is
already validated at `tools/check/kinds/claim.cmake:96-102`:

```cmake
# The artifact must be a repo-relative file under ROOT.
if(NOT "${artifact}" STREQUAL "")
  if(NOT "${artifact}" MATCHES "^[A-Za-z0-9._/-]+$" OR "${artifact}" MATCHES "^/"
     OR "${artifact}" MATCHES "[.][.]"
     OR NOT EXISTS "${ROOT}/${artifact}" OR IS_DIRECTORY "${ROOT}/${artifact}")
    gate_fail(claim-dangling-path "${id}")
  endif()
endif()
```

`[OBS]` The ids file the step reads is **`data/expected/claims.ids`** (via the
schema's `inventory` line, read by `read_ids` at `tools/check/lib.cmake:75-101`,
which appends whole lines, so an inventory file must be single-column). The
record file is `data/claims.json`.

### Why the new rule is not redundant

`[OBS]` Because `claim-dangling-path` requires the artifact to **exist**, four of
the five handoff-shaped classes are already refused today — but for the wrong
reason and only while the file is absent. The hole is real:

| Handoff-shaped artifact | Exists in the tree? | Today's verdict |
|---|---|---|
| `build/gate/dev/runner/x.report` | no (build/ is ignored) | refused, `claim-dangling-path` |
| `build/listen/demo-lamp-tone-pass.wav` | no (`*.wav` in `.gitignore:18`) | refused, `claim-dangling-path` |
| any `.ppm` | no (`*.ppm` in `.gitignore:20`) | refused, `claim-dangling-path` |
| `data/bench/baseline-darwin-arm64.tsv` | **yes** (`git ls-files` matches) | **accepted today** |
| `data/bench/workloads.tsv` | **yes** | **accepted today** |
| `data/expected/handoffs.ids` (once created) | **yes** | **accepted today** |

`[OBS]` So the phase's rule closes two live holes (the two `data/bench/*.tsv`
files) and pre-empts a third (the ledger itself).

`[REC]` Add **one** refusal in `hook_record`, immediately after the existing
artifact block at `kinds/claim.cmake:96-102`, with one token:

```cmake
# A claim never cites a handoff-shaped artifact (EV-05, GUARDRAILS 2.8): a build
# product, an operator render, a timing record or the handoff ledger itself is
# never evidence for a claim.
if(NOT "${artifact}" STREQUAL "")
  if("${artifact}" MATCHES "^build/" OR "${artifact}" MATCHES "[.]wav$"
     OR "${artifact}" MATCHES "[.]ppm$" OR "${artifact}" MATCHES "[.]tsv$"
     OR "${artifact}" STREQUAL "data/expected/handoffs.ids")
    gate_fail(claim-handoff-artifact "${id}")
  endif()
endif()
```

`[REC]` Use `[.]tsv$` (any `.tsv`), not `^data/bench/.*[.]tsv$`: it is one
predicate instead of a path prefix to get wrong, it covers
`.planning/ci-runs.tsv` (a hosted timing record) as well, and no live claim
artifact ends in `.tsv` — measured, all 28:

```
CL-0001 docs/cp1610/reference-notes.md          CL-0015 tests/installed-consumer/main.c
CL-0002 data/profiles/intv-2609-ntsc-family-p1.json   CL-0016 data/stic/...regmap.literal.txt
CL-0003 (same profile file)                     CL-0017 docs/stic/timing-notes.md
CL-0004 docs/cp1610/reference-notes.md          CL-0018 data/video/geometry.literal.txt
CL-0005 data/cp1610/opcode-families.json        CL-0019 docs/video/frame-derivations.md
CL-0006 data/memmap/...literal.txt              CL-0020 fixtures/FX-diag-stic-1/...listing.txt
CL-0007 docs/formats/ivsc-v1.md                 CL-0021 docs/stic/reference-notes.md
CL-0008 fixtures/FX-diag-cpu-1/...listing.txt   CL-0022 tests/installed-consumer/main.c
CL-0009 data/input/...literal.txt               CL-0023 data/golden/demo-lamp-tone-pass.expect
CL-0010 data/input/...literal.txt               CL-0024 tests/installed-consumer/main.c
CL-0011 docs/input/controller-notes.md          CL-0025 tests/video/video_mob.c
CL-0012 data/psg/rules.json                     CL-0026 tests/video/video_collision.c
CL-0013 docs/psg/reference-notes.md             CL-0027 fixtures/FX-diag-stic-2/...listing.txt
CL-0014 fixtures/FX-diag-ctrl-psg-1/...listing.txt  CL-0028 tests/state/state_cut.c
```

`[INF]` So `claims=28 provisional=27` cannot move from this rule; it is purely
additive.

### The control shape

`[OBS]` `tools/check/run-controls.sh` builds each control as: `CHECK` (the step
name), `EXPECT` (one fixed string per line, all must occur), optional `FORBID`
(no line may contain it), `overlay/` (copied over a fresh copy of
`_good/tree`), optional `MAKE`, optional `DEVIATION`. The step must **exit
non-zero** (`:216`) and the tokens are matched inside output lines (`:225-240`).

`[OBS]` The existing `claims-dangling-artifact` control is the template: it
overlays `data/claims.json` with `"artifact": "docs/nope.md"` and expects
`FAIL claim-dangling-path: CL-0001`.

`[REC]` The new control must **fail for its intended reason only**, so its
overlay must make the artifact **exist** and must not trip `claim-dangling-path`:

```
tools/check/controls/claims-handoff-artifact/CHECK            -> claims
tools/check/controls/claims-handoff-artifact/EXPECT           -> FAIL claim-handoff-artifact: CL-0001
tools/check/controls/claims-handoff-artifact/FORBID           -> claim-dangling-path
tools/check/controls/claims-handoff-artifact/overlay/data/claims.json
tools/check/controls/claims-handoff-artifact/overlay/data/bench/workloads.tsv
```

with `"artifact": "data/bench/workloads.tsv"` in the overlaid `claims.json`. The
`FORBID` on `claim-dangling-path` is what proves the rule fired for its own
reason (PLAN-LESSONS 20: "a check that cannot fail proves nothing"). `[OBS]`
`_good/tree/data/claims.json` cites `data/tiers.json`, which matches none of the
new predicates, so the positive `_good/claims` run stays green.

`[REC]` Add the control name to `data/expected/controls/claims.ids` — that file
already holds both the `claims-*` and the `text-*` controls (`:1-16`), and
`run-controls.sh:283-293` refuses a control directory that is not in an inventory
(`control-not-in-inventory`) and an inventory line with no directory
(`control-inventory-orphan`).

## Q7 — `tools/check/steps/text.sh`: the claim-word list and the blast radius

### Where the list is and what shape it has

`[OBS]` The list is **one `grep -E` alternation on line 45**, used with `-w -i`:

```sh
grep -a -n -i -w -E 'supports|compatible|passes|accurate|verified|runs|plays|playable' \
  | grep -a -v -E '\[claim:CL-[0-9]{4}\]|\[nc\]' \
  | sed -n 's/^\([0-9][0-9]*\):.*/\1/p'
```

`[OBS]` The same words are restated in the header comment at `:6-8`, and the one
exemption is the tier identifier `playable-guest-progress`, removed from a line
before the match (`:43`, `:52`) and only in the files that contain it.

`[OBS]` The step scans `README.md`, `CHANGELOG.md` and `docs/**/*.md` from
FILELIST only (`:25-28`), reports `text-claim-untagged: <file>:<line>` sorted, and
prints `-- text pass docs=<n> claim-refs=<n>`.

### The measured blast radius of every candidate word

`[OBS]` Lines matching each candidate in the scanned set, excluding lines that
already carry `[claim:CL-NNNN]` or `[nc]`:

| Candidate | Lines | Where |
|---|---|---|
| `looks` | **1** | `docs/provenance.md:82` ("the things the scan looks for") |
| `sounds` | **1** | `docs/video/demo-lamp-tone-1-derivation.md:268` ("channel A sounds from the amplitude write of frame 3 to that of frame 7") |
| `listen` | 3 | `docs/runner.md:211`, `:226`, `:274` — all the helper's name and path (`tools/listen-demo.sh`, `build/listen/`) |
| `listening` | 4 | `docs/diagnostics.md:166`, `docs/input/diag-ctrl-psg-1-derivation.md:120`, `docs/runner.md:212`, `:226` |
| `listenable` | 2 | `docs/runner.md:212`, `:274` |
| `audible` | 3 | — |
| `heard` | 2 | — |
| `view` | 6 | — |
| `sound` | 21 | — |
| `render` | 17 | — |
| `picture` | 39 | — |
| `pictures` | 10 | — |

`[INF]` Adding `sound`, `render` or `picture` would force tags on 40 to 60 lines
of already-committed prose, several of them frozen derivations, and would make
every future sentence containing "picture" need a waiver. That is unpriceable and
would move `claim-refs` (which must not move).

`[REC]` Add **exactly two words, `looks` and `sounds`**, matching CONTEXT's own
phrasing ("'looks like' or 'sounds right'"). Cost measured: two tagged lines.

- `docs/provenance.md:82` is a reference document, not a frozen oracle: append
  `[nc]`.
- `docs/video/demo-lamp-tone-1-derivation.md:268` **is** a frozen derivation
  (GUARDRAILS §2.10, PLAN-LESSONS 21). `[OBS]` the recorded allowance already
  permits it: `07-VERIFICATION.md` says the derivation is "unchanged since the
  record, **apart from bare claim tags** and an appended section (k)". So a bare
  `[nc]` on that line is inside the allowance; record it as a bare claim tag, not
  as an edit of the frozen content, and do not touch a literal.

`[REC]` Do **not** add `listen`, `listening` or `listenable`: three of the four
occurrences are the *helper's own name* (`tools/listen-demo.sh`), which every
future document will mention, and the others are negations ("not a listening
test", "no listenable-tone statement") that would each need a waiver to say
nothing changed. `-w` means `listen` does not match inside `listenable`, so
omitting the family costs nothing that the two added words do not already catch.

### The control(s) that would move

`[OBS]` Two existing controls cover this step:
`text-untagged-claim-word` (overlay `docs/x.md` = one line "This emulator
supports every title.", EXPECT `FAIL text-claim-untagged: docs/x.md:1`) and
`text-untagged-playable` (overlay lines 1, 2, 3, 4; EXPECT lines 1 and 3). Both
live in `data/expected/controls/claims.ids:10-11`.

`[REC]` Neither existing control can prove the *new* words fire, so add one
control (Claude's discretion says one or two; one is enough because both words are
in one alternation and one overlay can carry both):

```
tools/check/controls/text-untagged-look-listen/CHECK    -> text
tools/check/controls/text-untagged-look-listen/EXPECT   -> FAIL text-claim-untagged: docs/x.md:1
                                                           FAIL text-claim-untagged: docs/x.md:2
tools/check/controls/text-untagged-look-listen/overlay/docs/x.md
    line 1: This looks like a console.
    line 2: It sounds right.
```

`[REC]` The overlay must contain no other claim word (measured against the eight
in the list) and no `[claim:...]` token, so the only two findings are the two
intended ones.

## Q8 — `data/expected/` ids-file grammar, and what `handoffs.ids` must look like

### Measured grammar of the existing files

`[OBS]` Four examples, read in full:

| File | Grammar | Header |
|---|---|---|
| `data/expected/important-gates-excluded.ids` | `<REQ> <reason>` — reason is words joined by underscores, "at least three", `none` illegal; 10 rows; **no `#allow-empty`** | 24 comment lines, then one `#` comment per group |
| `data/expected/state-cuts.ids` | `<cut-id> <program> <pinned-tick> <family>`; 18 rows | 12 comment lines defining each column |
| `data/expected/state-sweep-notes.ids` | `<row> <witness cut> <sweep>` where sweep is `ROW` or `EXEMPT:<reason>` | 15 comment lines |
| `data/expected/d10-measurements.ids` | `<id> | <discrepancy id> | <what the owner records> | <anchor>` — `|`-separated because fields contain spaces; 47 rows | 6 comment lines |

`[OBS]` Conventions that hold across all four: `#` starts a comment; blank lines
are skipped; the first field is the id; every file states its own grammar and its
own reader in its header; a file whose emptiness is legal carries
`#allow-empty: <reason>` (read by `tools/check/lib.cmake:92-95`), and
`important-gates-excluded.ids` explicitly refuses one.

`[OBS]` **`data/expected/gate-hardening.ids` does not exist**; the control
inventory of that name is `data/expected/controls/gate-hardening.ids` (four
control names, plan 02-01). Do not confuse the two directories.

### The grammar for `handoffs.ids`

`[REC]` Follow CONTEXT literally: one line per handoff,
`<id> <disposition> evidence: none` — four space-separated tokens, in the
`state-cuts.ids` style (fixed positional tokens, header comment naming each):

```
# Human-only items: what a person owes, never evidence, never citable by a claim
# (EV-05, GUARDRAILS 2.8, AUTONOMY.md section 3). One line per item:
#   <id> <disposition> evidence: none
# The id is the one the project records use (an H<n>-<k> of a phase record, or a
# D<n> owner gate). The disposition is handed-off or not-performed, and nothing
# else: a handoff is never a pass and never blank. The third and fourth tokens are
# the literal words evidence: none, so a row cannot claim an evidence tier.
# Read by tools/check/kinds/claim.cmake (the artifact refusal) and cited by later
# milestones instead of re-asking. Emptiness is not legal: there is no #allow-empty.
H7-1 handed-off evidence: none
H7-2 handed-off evidence: none
H8-3 handed-off evidence: none
D10 not-performed evidence: none
D11 not-performed evidence: none
```

`[REC]` Do **not** use `read_ids` for it. `[OBS]` `read_ids`
(`tools/check/lib.cmake:75-101`) appends the **whole line**, so it is only usable
for single-column inventories; a four-token file needs its own `awk '$1'` or a
`file(READ)` plus a token split.

`[REC]` **Absence must mean empty.** The toy tree has no
`data/expected/handoffs.ids` (`[OBS]` `ls tools/check/controls/_good/tree/data/expected/`
lists 20-odd files, none of them this one) and every control that runs `claims`
or `text` would fail with an inventory-missing token if the reader used
`read_ids`. Guard the read with `if(EXISTS "${ROOT}/data/expected/handoffs.ids")`,
exactly as `claim_load_profile` guards its profile file
(`kinds/claim.cmake:26-28`) and as `important-gates.sh` treats a missing exclusion
ledger as an empty set.

`[REC]` If the plan wants the toy tree to be faithful, add a one-row
`_good/tree/data/expected/handoffs.ids`; that is optional and costs one file.

### How the `claims` step reads it

Two independent uses, and the plan should pick one:

1. **Path shape only** (recommended, Q6): the hook never opens the ledger. The
   four predicates plus `data/expected/handoffs.ids` cover "a claim may not cite a
   handoff artifact" as a class.
2. **Ledger membership**: the hook parses the first token of each row and refuses
   an artifact equal to a row id. That is weaker (a row id is not a path, so the
   dangling rule already refuses it) and couples the claims step to a file whose
   grammar may grow.

`[REC]` Take (1), and say so in `docs/presentation.md`'s sibling record: the
ledger is read by *humans and later milestones*, and the gate enforces the
*class* of artifact, not membership of a list that a future row could silently
widen.

## Q9 — H7-1, H7-2, H8-3, D10, D11: exactly what each asks a human to do

`[OBS]` `.planning/milestones/v0.1.0-phases/07-.../07-VERIFICATION.md:93-94`
(section 2, "Handed off to the owner"):

| Id | As recorded | Exact owner action |
|---|---|---|
| **H7-1** | "Listen to the exported WAV of the pass scenario" — a person's judgement of sound; **not evidence**; "none; no claim depends on it" | `sh tools/listen-demo.sh demo-lamp-tone-pass`, then listen to `build/listen/demo-lamp-tone-pass.wav` |
| **H7-2** | "View the exported frames (`--frame-ppm`) of the pass scenario" — a person's judgement of a picture, with an arbitrary preview palette; **not evidence**; "none; no claim depends on it" | run `ivs-run` on the pass scenario with `--frame-ppm <file>` and open the file in any viewer |

`[OBS]` `.planning/milestones/v0.1.0-phases/08-.../08-MILESTONE-CLOSE.md:399`
(section K, "Hand-offs for the owner"):

| Id | As recorded | Exact owner action | Effect on claims |
|---|---|---|---|
| **H8-3** | "Listening and viewing" — a person's judgement of sound and picture | "H7-1 and H7-2 of the Phase 7 record, and a look at the Phase 8 pictures (`ivs-run` with `--frame-ppm` on a `diag-stic2-` scenario)" | none |

`[OBS]` `.planning/STATE.md:562` and `:604`, and
`data/expected/d10-measurements.ids` (47 rows):

| Id | As recorded | Exact owner action |
|---|---|---|
| **D10** | "physical measurement (44 hardware items)" (`STATE.md:562`) — `[OBS]` the file holds **44** `D10-*` rows and **3** `D11-*` rows, so 47 in total | the 44 `D10-*` items, by `docs/input/d10-procedure.md` and `docs/stic/d10-procedure.md` (the anchors in column 4 of the ids file) |
| **D11** | "owner-local real EXEC and GROM runs" — `D11-EXEC-01..03` (owner system image reading the controllers; commercial game input and audio; a real system image drawing its title screen) | follow `docs/owner-images.md` with `tools/owner-run.sh`; never commit an image, never run in CI |

`[INF]` **H8-3 is not a fifth act.** It is the Phase 8 close record's name for
the same two acts (H7-1, H7-2) plus a look at the STIC pictures. The ledger must
still carry it — criterion 3 names all five ids — but its row must say so, or a
later milestone will treat it as a third listening task. `[REC]` disposition it
`handed-off` and put the relationship in the file header or in the handoff packet
("H8-3 is the Phase 8 close record's id for H7-1 and H7-2 plus the
`diag-stic2-` pictures; one act discharges both rows").

`[OBS]` **A stale figure to correct in passing:** `STATE.md:604` says "D10 (47
hardware items)" while `STATE.md:562` says "44 hardware items". Measured: 44
`D10-*` + 3 `D11-*` = 47 rows. The ledger's D10 and D11 rows must not repeat the
conflation (AGENTS.md: "Recompute, never quote").

`[OBS]` **A second stale figure:** `07-VERIFICATION.md:95` (H7-3) says "42 items
in `data/expected/d10-measurements.ids`". Measured today: 44 `D10-*` rows (47
total). The archive is append-only, so this is an appended note, never an edit
(GUARDRAILS §2.10); the ledger row cites the file, not the number.

### The handoff packet: what a human actually compares

`[OBS]` Measured on this tree:

| Item | Command | Deterministic identity |
|---|---|---|
| H7-1 | `sh tools/listen-demo.sh demo-lamp-tone-pass` | `build/listen/demo-lamp-tone-pass.wav`, 21674 bytes, `refpcm.sha256 e3b6638887947d64f7fa7d2be5687aff0eca85981b0ea79cfcfd3c74d057f1ad` |
| H7-2 | `ivs-run --content-dir build/release/images --frame-ppm <file> data/scenarios/demo-lamp-tone-idle.ivss` and the same for `demo-lamp-tone-pass` | two files of 45806 bytes, mode 0600, header `P6`/`159 96`/`255`; the pass file is the last delivered frame, frame 12, whose canonical text form hashes to `a7b9889b136cf47b2586def7f992c7caaf19c451e043a9a676b2f838bd91c2d7` (`data/golden/demo-lamp-tone-pass.expect:81`) |
| H8-3 picture limb | `ivs-run --content-dir build/release/images --frame-ppm <file> data/scenarios/diag-stic2-mob-pass.ivss` | 45806 bytes, mode 0600, exit 0, `report.end 81` |
| D10 | `docs/input/d10-procedure.md`, `docs/stic/d10-procedure.md` | the 44 anchors in column 4 of `d10-measurements.ids` |
| D11 | `docs/owner-images.md`, `tools/owner-run.sh` | `SKIPPED owner-run: no operator images (D11)`, exit 77, when no image exists |

`[REC]` The packet is one document (Claude's discretion on layout). It must say,
for each of H7-1 and H7-2: the command, the artefact path, its size and mode, the
one digest line to compare, and the sentence that a `yes`/`no` answer is a
handoff result and never evidence, never a claim and never a tier. It must not
include the word `looks` or `sounds` on an untagged line (Q7) — the packet is
itself scanned.

## Q10 — The count deltas: what may move, what may not, and by how much

### The baseline, measured today (red for the Q1 reason)

`[OBS]` `sh tools/gate.sh` (82 s wall on this host):

```
gate: FAIL steps=36/38 controls=386/386 ... claims=28 provisional=27 ... gates=107 lines=330
  ... d10-items=47 faults=235/235 tracked=2571 ... doc-files=59 ... cases=13 ...
  ... tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 docs=60
  ... claim-refs=26 ... files=2571 ... hygiene-files=2571
```

`[OBS]` Note three tokens have moved since the Phase 9 record
(`09-09-SUMMARY.md`: `controls=384/384`, `files=tracked=hygiene-files=2555`,
`cases=12`). The current values are the ones a Phase 10 plan must predict against;
quoting the Phase 9 record would be a stale-figure defect.

### Fields frozen by this phase's criteria

| Token | Required | Reason |
|---|---|---|
| `steps` | `38/38` | criterion 5; GUARDRAILS §2.1; `[OBS]` `ls tools/check/steps/*.sh \| wc -l` = 38 |
| `claims` | `28` | criterion 5 ("`claims=` at its Phase 9 value") |
| `provisional` | `27` | follows from `claims`; no claim is added or raised |

### Fields that must not move, and why

| Token | Value | Why it cannot move |
|---|---|---|
| `gates`, `lines` | `107`, `330` | only move if a registry row is added — see the decision below |
| `faults` | `235/235` | no fault site is added (GUARDRAILS R10 would require it in the same commit) |
| `tests-expected/executed/skipped/missing` | `4958/4954/4/0` | no test id, no scenario, no suite case is added |
| `cases` | `13` | no new scenario |
| `goldens` | `29` | no new golden (R11 would require four coordinated edits) |
| `claim-refs` | `26` | no new `[claim:CL-NNNN]` reference |
| `prefix-files` | `11` | R7: nothing new is installed |

### Fields that will move (predicted, with the file that causes each)

`[REC]` With the recommended file set:

| Token | From | To | Caused by |
|---|---|---|---|
| `steps` | `36/38` | `38/38` | the Q1 one-line fix (commit 1) |
| `controls` | `386/386` | `388/388` | `claims-handoff-artifact` and `text-untagged-look-listen` |
| `files`, `tracked`, `hygiene-files` | `2571` | `2571 + N` | the new tracked files: `docs/running.md`, `docs/presentation.md`, `docs/handoffs.md` (the packet), `data/expected/handoffs.ids`, 3 files per control x 2 controls = 6. N = 10 (11 if the toy tree gains a `handoffs.ids`) |
| `doc-files` | `59` | `62` | `docs/running.md`, `docs/presentation.md`, `docs/handoffs.md` (README.md is already counted; CHANGELOG.md is not in this token) |
| `docs` | `60` | `63` | the same three (`docs` = `doc-files` + `CHANGELOG.md`) |
| `gates` | `107` | `109` **if** registry rows are added | `claims` and `text` are new gate names in `data/expected/important-gates.ids` (`[OBS]` both currently have 0 rows) |
| `lines` | `330` | `332` (or `333`) **if** registry rows are added | 2 rows for EV-05 (`claims`, `text`); a third for TOOL-04 (`c-purity`, which already has 3 rows so `gates` would not move again) |

`[REC]` If the packet lives under `data/` instead of `docs/`, `doc-files` and
`docs` each move by 2 and only `files`/`tracked`/`hygiene-files` absorb the
packet. State the chosen location in the plan's predicted-delta table; the three
file-count tokens are the ones a stale prediction most easily gets wrong.

### The decision the plan must make: registry rows or exclusion rows

`[OBS]` The important-gates accepted set is **derived**, not hand-written
(`tools/check/steps/important-gates.sh:145-181`): it parses `| REQ | Phase |
Status |` rows with `^Complete` from `.planning/REQUIREMENTS.md` and every
`.planning/milestones/*-REQUIREMENTS.md`, subtracts
`data/expected/important-gates-excluded.ids`, and refuses a Complete requirement
in neither the registry nor the ledger (`important-gates-complete-uncovered`) and
a ledger row whose requirement is not Complete (`important-gates-excluded-stale`).

`[INF]` Therefore: **the moment PKG-05, TOOL-04 and EV-05 are marked `Complete`,
the tree goes red unless each gains a registry row or an exclusion row, in the
same commit.** This is exactly the D-09-05/D-09-06 pattern Phase 9 recorded, and
it is the single most likely way a Phase 10 plan turns the gate red at the close.

`[REC]` Preferred: **register the two mechanisms that are real and new** —

```
claims control claims-handoff-artifact EV-05
text control text-untagged-look-listen EV-05
```

— because both are new controls that run in every gate, have a liveness guard
(the EXPECT token) and a committed stand-in; that is precisely what the registry
exists for, and it costs two lines. Add `c-purity control
core-symbols-palette-defined TOOL-04` **only** if the plan accepts that the
control pre-dates the requirement; otherwise exclusion rows are honest.

`[REC]` PKG-05 has no gate mechanism at all (the rehearsal is release-time by
design), so it needs an exclusion row in
`data/expected/important-gates-excluded.ids` with a reason in the existing
`close_record_only` category, e.g.
`PKG-05 release_rehearsal_is_release_time_not_a_gate_step`.

`[REC]` `lines=330` and `gates=107` are **not** frozen by Phase 10's criteria, so
moving them is legal — but state the new values in the plan's prediction before
the commit (GUARDRAILS §3) and do not attempt to also re-derive the Phase 9
`lines=330` baseline for the ten deferred mechanisms (STATE.md's deferred row
makes that a separate milestone's job with its own re-derivation).

`[OBS]` **GUARDRAILS R2 is partly stale.** It says a registry acceptance must move
"`tools/check/steps/important-gates.sh` (the `nreq` split string **and** its
header comment **and** the token prose)". Measured: `nreq` at `:156`, `:160`,
`:177`, `:180` is now an awk **counter** over the derived set, not a split string
of a hand-written list (plan 09-05 deleted that). So only the header comment and
the token prose can need updating. Record this as a GUARDRAILS correction, not as
a plan deviation.

## Q11 — Conflicts with the frozen criteria, with the measurement

`[OBS]` Five places where a frozen sentence is not satisfiable as written.

**C1 — the tail is not unconditionally byte-exact.** Criterion 1 asks for "the
exact expected stdout tail" byte for byte. `host.calls` is in that tail and is
documented as host-dependent (`docs/runner.md:114`); measured 11 at the default
slice and 1599 at `--slice-ticks 1000`. **Amendment:** quote the tail at the
default slice and state the dependency in one sentence, or end the quoted tail at
`video.chain.check pass`. Either is honest; a plan that asserts "byte for byte"
without the qualification writes a false sentence.

**C2 — "kept identical" has no antecedent.** Criterion 4 and CONTEXT both say the
sentence is *kept* identical. Measured: `grep -c -F 'nothing downloadable is runnable'`
gives **1** in `docs/packaging.md` and **0** in `README.md`. **Amendment:** the
sentence is *made* identical, which requires an edit to `README.md:17`; and the
checked object must be the substring, because `docs/packaging.md:121` is a `- `
bullet and a whole-line `grep -x` comparison is unsatisfiable without destroying
the list.

**C3 — criterion 5 is unsatisfiable at phase entry.** `steps=36/38` today, from
the Q1 finding. **Amendment:** one line in
`.planning/phases/09-.../09-SECURITY.md:122`, as commit 1.

**C4 — the policy document cannot precede its check.** CONTEXT and GUARDRAILS §2.9
require the oracle (the policy document) before the code that enforces it, and
criterion 2 says the check "adds no gate step". Measured: the enforcement already
exists (`core-symbols.sh:163-173`, since plan 06-09) and the control already
passes. **Amendment:** record the inversion explicitly — `docs/presentation.md`
is a *new* oracle for an *existing* mechanism, and the ancestry proof is
`git merge-base --is-ancestor <the 06-09 commit> <the policy commit>` in the
direction the history actually runs. Do not claim the phase froze a rule before
building it; it froze a rule that was already built.

**C5 — H8-3 is not a fifth act.** Criterion 3 lists five ids as if they were five
human tasks; H8-3 is the Phase 8 close record's umbrella for H7-1 + H7-2 + the
STIC pictures (Q9). **Amendment:** the ledger carries five rows (satisfying the
criterion) and the file header or packet states the relationship, so no later
milestone books a third listening task.

`[OBS]` Two smaller record defects to fold into the plan rather than re-discover:
`STATE.md:604` ("D10 (47 hardware items)") and `07-VERIFICATION.md:95` ("42
items") against the measured 44 `D10-*` + 3 `D11-*` rows.

## Standard Stack

No package is installed, added or upgraded by this phase. The stack is the
project's own:

### Core

| Tool | Version | Purpose | Why this one |
|---|---|---|---|
| POSIX `sh` + `dash` | system | every script and the gate | `[OBS]` GUARDRAILS §3 requires both `sh tools/gate.sh` and `dash tools/gate.sh` |
| CMake | 3.25 floor; 3.28.3 hosted, 4.4.3 local | presets, `-P` script mode, `-E tar`/`sha256sum` | `[OBS]` `CMakeLists.txt`, `CMakePresets.json` (`base`, `dev`, `release`); D-35 records the floor and the two exercised versions |
| Ninja | system | the preset generator | `[OBS]` `tools/gate.sh:61` fails closed without it |
| `awk`, `grep`, `sed`, `cmp`, `tar`, `gzip` | POSIX/system | every check and the rehearsal | `[OBS]` `tools/release-rehearse.sh:53` already preflights them |
| C17 compiler (`cc`) | gcc 13 hosted, AppleClang local | `c-purity`'s fresh archives | `[OBS]` `cc=AppleClang` in the summary line |

### Supporting

| Tool | Purpose | When to use |
|---|---|---|
| `nm -P`, `ar` | the defined-symbol rules | only inside `core-symbols.sh`; never call `nm` from a plan task |
| `cmake -E sha256sum` | `SHA256SUMS` | already used by `release.sh:213` |
| `tools/check/run-controls.sh` | negative controls | the only way to prove a new refusal fires |

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|---|---|---|
| a new gate step for the colour rule | the existing `c-purity` step | a step is the expensive unit and is **forbidden** this milestone (GUARDRAILS §2.1); the mechanism already exists |
| a helper script for the three commands | `docs/running.md` only | CONTEXT decided the document; a helper needs zero logic or a new id |
| a second golden comparison in the rehearsal | the committed `.digests` golden + a copied `filter_report` | a second *comparison* would be a second oracle; a copied *filter* fails closed (Q2) |
| a CI job for the source route | the release rehearsal | the gate already pays for a cold build (CONTEXT, criterion 1) |

**Installation:** none. `[OBS]` `data/expected/pkg-install-files.ids` stays at 11
files; nothing in this phase is installed.

## Package Legitimacy Audit

**Not applicable.** This phase installs no external package, adds no dependency
and changes no build input. `[OBS]` The phase's only executables are the ones the
tree already builds (`build/<preset>/runner/ivs-run`, the gate steps) and the
POSIX tools listed above. No `npm`/`pip`/`cargo`/`vcpkg` manifest is touched, so
the Package Legitimacy Gate has nothing to check and no `[ASSUMED]` package name
appears anywhere in this research.

## Architecture Patterns

### System Architecture Diagram

```
  person
    |  1. reads docs/running.md (three commands + the exact tail)
    v
  source archive  ivs-<tag>-source.tar.gz   (tools/release.sh: git archive HEAD | gzip -n)
    |  built from HEAD, prefix ivs-<tag>/, members == git ls-files
    v
  clean directory (no .git)
    |  cmake --preset release ; cmake --build --preset release
    v
  build/release/runner/ivs-run  +  build/release/images/*.ivsc|*.exec
    |  ivs-run --content-dir build/release/images data/scenarios/demo-lamp-tone-pass.ivss
    v
  report (131 lines, deterministic except host.calls)
    |  filter_report (the golden key list)
    v
  cmp  ==  data/golden/demo-lamp-tone-pass.digests        <-- the golden, the one oracle
    |
    +--> operator files, never evidence:
           build/listen/<id>.wav   (H7-1)
           <any path>.ppm          (H7-2)
                |
                v
           a person says yes or no  ->  data/expected/handoffs.ids (evidence: none)

  gate (never runs the route):
    c-purity -> core-symbols.sh  -- refuses a colour symbol defined in core/src
    claims   -> kinds/claim.cmake -- refuses a claim citing a handoff-shaped artifact
    text     -> claim-word list   -- refuses "looks"/"sounds" without a claim tag
```

### Recommended Project Structure (files this phase touches or adds)

```
docs/
├── running.md            # NEW  the three commands, the tail, the .ppm/.wav landings
├── presentation.md       # NEW  the six-clause policy, cites core-symbols.sh by line
├── handoffs.md           # NEW  the H7-1/H7-2 packet (Claude's discretion: name/layout)
├── packaging.md          # EDIT one paragraph pointing at docs/running.md (line 121 unchanged)
└── runner.md             # EDIT only if a spelling must agree with docs/running.md
README.md                 # EDIT line 17 gains the phrase; one paragraph -> docs/running.md
data/expected/
├── handoffs.ids          # NEW  five rows, evidence: none
└── controls/claims.ids   # EDIT two control names appended
tools/check/
├── steps/text.sh         # EDIT line 45 alternation + lines 6-8 comment
├── kinds/claim.cmake     # EDIT one refusal after :96-102
├── controls/claims-handoff-artifact/{CHECK,EXPECT,FORBID,overlay/...}
├── controls/text-untagged-look-listen/{CHECK,EXPECT,overlay/docs/x.md}
└── ../release-rehearse.sh # EDIT header 3-29, flags 38-48, new step after :99
.planning/phases/09-.../09-SECURITY.md  # EDIT one line (the Q1 entry fix)
```

### Pattern 1: oracle first, with the ancestry proved in the direction history runs

**What:** commit the frozen document (or the ids file) before the check that
enforces it, then prove the order.
**When to use:** every new check this phase.
**Example:**

```sh
# after the policy commit and the check commit:
git merge-base --is-ancestor <policy-commit> <check-commit>   # must exit 0
```

For `docs/presentation.md` the mechanism is older than the policy, so the proof
runs the other way (Q11 C4) and must be stated as an inversion.

### Pattern 2: one control, one token, one FORBID when a second token could fire

**What:** a control directory whose EXPECT names the intended token and whose
FORBID names any token that would make it pass for the wrong reason.
**When to use:** whenever the new rule sits beside an older rule on the same field
(`claim-handoff-artifact` beside `claim-dangling-path`).
**Example:** the `claims-handoff-artifact` control above.

### Pattern 3: absence is empty, not missing

**What:** a reader of a new `data/expected/*.ids` file treats a missing file as an
empty set, because the toy control tree does not carry it.
**When to use:** `handoffs.ids`, and any future ledger read by a hook.
**Example:** `if(EXISTS "${ROOT}/data/expected/handoffs.ids") ... endif()`.

### Anti-Patterns to Avoid

- **A second colour mechanism.** The rule exists; adding `colour|color` to a new
  deny list would be a second mechanism for one property, and `color` as a
  substring would fire on `colour_space`-style prose elsewhere.
- **A `.ppm` or `.wav` in the tree.** `[OBS]` `.gitignore:18,20` already ban them
  and `tools/check/binary-ext.txt` refuses them as release members; a committed
  render would be a rights and a claim problem at once.
- **Adding `picture`, `render` or `sound` to the claim-word list.** Measured 39,
  17 and 21 untagged lines (Q7).
- **Asserting `host.calls` byte for byte** without naming the slice dependency.
- **Registering a requirement without its exclusion row or its registry row** in
  the same commit (Q10) — that is the close-time red tree.
- **Editing a frozen derivation's literals** to satisfy a claim-word rule; only a
  bare claim tag is inside the recorded allowance.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---|---|---|---|
| Comparing a report with a golden | a second comparator, or a `--golden` flag on `ivs-run` | `filter_report` + `cmp` against `data/golden/<id>.digests` | the golden and the key list are already the oracle; `docs/runner.md:243-245` keeps the list in three places on purpose and a fourth would drift |
| Deciding whether a claim's artifact is real | a new path linter | `claim-dangling-path` (exists) + one new predicate set | the shape rule and the existence rule are complementary; the plan adds four predicates, not a linter |
| Refusing a colour symbol | a source grep for `palette` | `core-symbols.sh`'s defined-symbol rule | a grep would fire on comments and identifiers that are not symbols, and would miss a `static` table reached through an alias |
| Extracting a tarball portably | a `tar` flag zoo | `tar -xzf` (already used at `release-rehearse.sh:109-110`) and `tar -tzf` | the script already runs on both hosts with the same flags |
| Proving a control can fail | a new selftest harness | `run-controls.sh` `EXPECT`/`FORBID` | the harness already runs 386 controls and prints one line each |

**Key insight:** this phase adds **two predicates and three documents**, not
machinery. Every mechanism it needs already exists and already runs in the gate;
the phase's difficulty is that each new sentence must be *measured* before it is
written, because four of the frozen sentences are wrong about the tree (Q11).

## Common Pitfalls

### Pitfall 1: predicting a delta against the Phase 9 record

**What goes wrong:** the plan predicts `controls=384/384`, `files=2555`,
`cases=12` because `09-09-SUMMARY.md` says so.
**Why it happens:** the Phase 9 record is the most recent prose, and it is stale.
**How to avoid:** recompute from today's gate run (Q10's baseline:
`386/386`, `2571`, `13`) and state the baseline commit.
**Warning signs:** the first gate after the first commit shows a token that moved
without a cause.

### Pitfall 2: the control passes for the wrong reason

**What goes wrong:** `claims-handoff-artifact`'s overlay names `build/x` which
does not exist, so `claim-dangling-path` fires, the step exits non-zero, the
EXPECT token is missing, and the control fails — or worse, the EXPECT is written
loosely enough to match and the control proves nothing.
**Why it happens:** two rules on one field.
**How to avoid:** make the artifact exist in the overlay and add `FORBID
claim-dangling-path` (PLAN-LESSONS 20).
**Warning signs:** the control's output shows a token you did not expect.

### Pitfall 3: the toy tree has no `handoffs.ids`

**What goes wrong:** every `claims` and `text` control fails with an
inventory-missing token.
**Why it happens:** the hook used `read_ids`, which fails closed on a missing file.
**How to avoid:** `if(EXISTS ...)`; absence is empty (Q8).
**Warning signs:** `controls` drops sharply and the failures are all
`claims-*`/`text-*`.

### Pitfall 4: the rehearsal mode is a third source for the tag or the prefix

**What goes wrong:** the mode re-derives `v$hdrver` and hard-codes `ivs-v0.1.0/`,
and the first tag after a version bump breaks it — at release time, when nobody is
watching.
**Why it happens:** the tag is visible in the source and looks stable.
**How to avoid:** read the prefix from the archive (`tar -tzf | head -n 1`); take
the archive name from step 3's `$src_a`.
**Warning signs:** a version bump needs an edit to two scripts.

### Pitfall 5: `docs/running.md` contradicts `docs/runner.md`

**What goes wrong:** the new document writes `build/release/ivs-run` or
`--content-dir build/release` (missing `/images`), and a person follows it into a
`missing-fixture` run.
**Why it happens:** the two documents were written at different times by different
phases.
**How to avoid:** copy the invocation shape from `docs/runner.md:94` and `:263`
verbatim; run the document's exact command lines before committing them.
**Warning signs:** any path in the new document that does not appear in
`docs/runner.md`.

### Pitfall 6: a new document trips the scanners it was written for

**What goes wrong:** `docs/presentation.md` says "looks like" as the forbidden
phrase and the `text` step refuses it for having no claim tag; or it says "clock"
bare and `glossary` refuses it; or a line contains a home-relative path.
**Why it happens:** the policy document quotes the words the policy bans.
**How to avoid:** put `[nc]` on every line of the new documents that contains
`looks` or `sounds`, avoid the bare word the glossary bans, avoid tilde paths, and
run `sh tools/check/steps/text.sh` plus `glossary.sh` against the new files before
committing.
**Warning signs:** `docs=` moved by more than the number of files added, or a
`text-claim-untagged` line naming a file this phase created.

### Pitfall 7: marking a requirement Complete in its own commit

**What goes wrong:** `PKG-05`, `TOOL-04` or `EV-05` flips to `Complete` and the
important-gates derivation immediately demands a registry or exclusion row
(Q10) — the exact `-- FAIL important-gates-complete-uncovered: EV-04` that plan
09-05 measured and reverted.
**Why it happens:** the status flip and the ledger row are two edits that feel
like one decision.
**How to avoid:** one commit holds the flip **and** its registry/exclusion rows;
run the step before and after.
**Warning signs:** `important-gates-complete-uncovered` naming a requirement this
phase delivered.

## Code Examples

### The golden comparison the rehearsal must reproduce

```sh
# Source: tools/check/steps/golden.sh:146-161 (filter_report, live_count) and
# :353-355 (the three-line comparison). Copied here because the rehearsal is
# release-time and cannot source a gate step.
live() { awk -v k="$1" '$1 == k { v = $2; n++ } END { print (n == 1 && v ~ /^[0-9]+$/ && v !~ /^0+$/) ? 1 : 0 }' "$2"; }
lv=$(live output.video.bytes "$rep"); la=$(live output.audio.bytes "$rep")
li=$(live input.events "$rep");       lp=$(live output.psglog.bytes "$rep")
awk -v v="$lv" -v a="$la" -v i="$li" -v p="$lp" '
    $1 ~ /^(slot|step|cpu|input)\./ { print; next }
    $1 ~ /^(scenario\.id|scenario\.profile|outcome|outcome\.detail|exit\.code)$/ { print; next }
    $1 ~ /^digest\.(cpu|mem|images|instance|composite)$/ { print; next }
    $1 ~ /^output\.(video|audio)\.bytes$/ { print; next }
    ($1 ~ /^(digest\.video|output\.video\.frames|video\.geometry|video\.row_lines|video\.frame\.last_tick|video\.chain\.check)$/ && v == 1) { print; next }
    ($1 ~ /^(digest\.audio|output\.audio\.samples|output\.audio\.changes|audio\.sample_ticks)$/ && a == 1) { print; next }
    ($1 == "digest.input" && i == 1) { print; next }
    ($1 ~ /^(digest\.psglog|output\.psglog\.records|output\.psglog\.bytes)$/ && p == 1) { print }' \
  "$rep" > "$want"
awk '/^#/ { next } { print }' "$ROOT/data/golden/demo-lamp-tone-pass.digests" > "$have"
cmp -s "$want" "$have" || fail "the source-archive run differs from data/golden/demo-lamp-tone-pass.digests"
```

### The measured route, exactly

```sh
# Source: measured in this session against the real archive.
sh tools/release.sh --no-gate --out /tmp/rel10
mkdir -p /tmp/clean && cd /tmp/clean
tar -xzf /tmp/rel10/ivs-v0.1.0-source.tar.gz
cd ivs-v0.1.0
cmake --preset release
cmake --build --preset release
./build/release/runner/ivs-run --content-dir build/release/images \
  data/scenarios/demo-lamp-tone-pass.ivss > report.txt
tail -9 report.txt
```

### The claim-side refusal, in place

```cmake
# Source: tools/check/kinds/claim.cmake:96-102, with the new block appended.
# The artifact must be a repo-relative file under ROOT.
if(NOT "${artifact}" STREQUAL "")
  if(NOT "${artifact}" MATCHES "^[A-Za-z0-9._/-]+$" OR "${artifact}" MATCHES "^/"
     OR "${artifact}" MATCHES "[.][.]"
     OR NOT EXISTS "${ROOT}/${artifact}" OR IS_DIRECTORY "${ROOT}/${artifact}")
    gate_fail(claim-dangling-path "${id}")
  endif()
  # A claim never cites a handoff-shaped artifact (EV-05, GUARDRAILS 2.8).
  if("${artifact}" MATCHES "^build/" OR "${artifact}" MATCHES "[.]wav$"
     OR "${artifact}" MATCHES "[.]ppm$" OR "${artifact}" MATCHES "[.]tsv$"
     OR "${artifact}" STREQUAL "data/expected/handoffs.ids")
    gate_fail(claim-handoff-artifact "${id}")
  endif()
endif()
```

### The `text` word list, edited

```sh
# Source: tools/check/steps/text.sh:45. Two words added; the header comment at
# :6-8 must be updated in the same edit.
grep -a -n -i -w -E 'supports|compatible|passes|accurate|verified|runs|plays|playable|looks|sounds' \
  | grep -a -v -E '\[claim:CL-[0-9]{4}\]|\[nc\]' \
  | sed -n 's/^\([0-9][0-9]*\):.*/\1/p'
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|---|---|---|---|
| hand-written accepted-requirement set in `important-gates.sh` | derived from the requirement records minus an exclusion ledger | plan 09-05 | a new Complete requirement must arrive with a registry or exclusion row in the same commit (Q10) |
| one `--out`-only rehearsal (`--no-gate`, `--release`) | a source-archive route mode | this phase | the reachability proof is a release-time transcript, not a CI job |
| the runner built only for the gate | built from the released archive by a person | this phase | PKG-05's claim ceiling is `installed-host-behavior`, not "installed" |
| `README.md` and `docs/packaging.md` describing the same fact in different words | one checked phrase in both | this phase | a grep can prove agreement |

**Deprecated/outdated in the records, to correct in passing (append-only):**

- GUARDRAILS R2's "`nreq` split string" — the split was deleted by plan 09-05;
  `nreq` is now an awk counter over the derived set.
- `STATE.md:604` "D10 (47 hardware items)" against `STATE.md:562` "44 hardware
  items"; measured 44 `D10-*` + 3 `D11-*`.
- `07-VERIFICATION.md:95` "42 items in `data/expected/d10-measurements.ids`";
  measured 47 rows, 44 of them `D10-*`.
- `09-09-SUMMARY.md`'s `controls=384/384`, `files=2555`, `cases=12` — today's
  values are `386/386`, `2571`, `13`.

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| — | *(none)* | — | Every claim in this document is a file read or a command output recorded above. The two places where the tree leaves a sentence ambiguous — whether the tail includes `host.calls` and whether "identical sentence" means the substring or the whole line — are recorded as **open questions with the measurement beside them**, not as assumed facts. |

**If this table is empty:** all claims in this research were verified or cited —
no user confirmation is needed for a factual claim. The two items below are
*decisions the plan must make*, not facts it must confirm.

## Open Questions

1. **Does the quoted tail include `host.calls`?**
   - What we know: the tail is 9 lines; `host.calls 11` is stable at the default
     slice, across presets and across the clean tree, and becomes 1599 at
     `--slice-ticks 1000`; `docs/runner.md:114` documents it as the one
     host-dependent key.
   - What's unclear: whether criterion 1's "byte for byte" tolerates a qualified
     line.
   - Recommendation: quote all nine lines, and state the slice dependency in the
     same paragraph. Record it as an amendment note in the phase record.

2. **Is "the identical sentence" the whole line or the phrase?**
   - What we know: `docs/packaging.md:121` is a `- ` bullet whose sentence
     continues past the phrase; `README.md` has 0 matches.
   - What's unclear: whether the roadmap's "a single matching line in each" means
     `grep -x` (whole line) or `grep -c` (line containing the phrase).
   - Recommendation: `grep -c -F`, with the 34-byte phrase as the checked object,
     and the README edit as the change that makes the count 1 in each.

3. **Registry rows or exclusion rows for PKG-05, TOOL-04, EV-05?**
   - What we know: the derivation demands one or the other in the same commit as
     the status flip; `lines`/`gates` are not frozen by this phase's criteria.
   - What's unclear: whether the milestone wants to start re-deriving the Phase 9
     `lines=330` baseline here.
   - Recommendation: two registry rows (EV-05's two new controls) plus one
     exclusion row (PKG-05, release-time); defer the ten Phase 9 mechanisms.

4. **Where does the handoff packet live?**
   - What we know: it is a document, not a gate step; a `docs/` location moves
     `doc-files` and `docs` by 3 rather than 2.
   - What's unclear: nothing material — Claude's discretion.
   - Recommendation: `docs/handoffs.md`, named in the predicted-delta table.

5. **Does the plan want a TOOL-04 registry row for the pre-existing control?**
   - What we know: `core-symbols-palette-defined` exists and passes; it was
     created under D-12/D-13 (plan 06-09), not under TOOL-04.
   - What's unclear: whether registering a control under a requirement that did
     not cause it is honest bookkeeping or a false attribution.
   - Recommendation: an exclusion row with reason
     `enforcement_predates_the_requirement_core_symbols_palette_defined`, and say
     so in `docs/presentation.md`.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|---|---|---|---|---|
| POSIX `sh` | every script | yes | `/bin/sh` (bash 3.2 in POSIX mode on this host) | — |
| `dash` | GUARDRAILS §3 second gate shape | yes | system dash | `sh` only, with a visible `not run` line |
| `cmake` | presets, `-P`, `-E` | yes | 4.4.3 local (`[OBS]` D-35 records 3.28.3 hosted) | — |
| `ninja` | the preset generator | yes | system | — |
| `cc` | `c-purity`'s fresh archives | yes | AppleClang (summary line `cc=AppleClang`) | — |
| `git` | `git archive`, the ancestry proofs | yes | system | — |
| `tar`, `gzip`, `awk`, `cmp`, `sort`, `mktemp`, `find` | the rehearsal | yes | system (`release-rehearse.sh:53` preflights them) | — |
| `docker` | `tools/gate-linux.sh` (the third gate shape) | `[UNK]` in this session | — | write a visible `not run` line, never a pass (PLAN-LESSONS 24) |
| `gh` | nothing in this phase (agents never call it) | not needed | — | — |

**Missing dependencies with no fallback:** none identified.

**Missing dependencies with fallback:** `docker`, if absent on the executing
host: the container gate line becomes a visible `not run` with the reason.

## Validation Architecture

### Test Framework

| Property | Value |
|----------|-------|
| Framework | the project's own gate: 38 POSIX-sh steps + 386 negative controls + 2 ctest suites, driven by `tools/gate.sh` |
| Config file | `data/expected/gate-steps/*.ids` (24 files, 38 step names), `data/expected/controls/*.ids` (37 files, 386 control names) |
| Quick run command | `sh tools/check/steps/<name>.sh` with `ROOT`, `CHECK_DIR` and `FILELIST` set; for one control, `CONTROLS_DIR=... sh tools/check/run-controls.sh` (runs the whole set) |
| Full suite command | `sh tools/gate.sh` then `dash tools/gate.sh`; `sh tools/gate-linux.sh` for the Ubuntu substance |

`[OBS]` There is no pytest/jest/vitest in this repository. The framework is the
gate itself, and its definition of done is GUARDRAILS §3 verbatim.

### Phase Requirements -> Test Map

| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|---|---|---|---|---|
| PKG-05 | the source archive builds in a clean directory and runs the demo to its golden | integration (release-time, **not** in the gate) | `sh tools/release-rehearse.sh --source-route` (the new mode) | Wave 1 (the mode is new) |
| PKG-05 | the documented commands are the ones that work | manual-once, recorded as a transcript | run the three commands from `docs/running.md` in a clean extraction | Wave 1 (the transcript) |
| PKG-05 | the two prose files agree on the sentence | unit (grep) | `[ "$(grep -c -F 'nothing downloadable is runnable' README.md)" -eq 1 ] && [ "$(grep -c -F 'nothing downloadable is runnable' docs/packaging.md)" -eq 1 ]` | Wave 1 (a plan verify clause; no new step) |
| TOOL-04 | a colour symbol defined under `core/src` is refused | unit (control) | `sh tools/gate.sh` -> `control ok   core-symbols-palette-defined` | yes, since plan 06-09 |
| TOOL-04 | the core still imports nothing host-shaped | unit (step) | `sh tools/gate.sh` -> `core-imports=7 purity-selftest=7/7` | yes |
| TOOL-04 | the policy document exists and cites the rule | unit (grep) | `grep -q 'core-symbols.sh' docs/presentation.md` (a plan verify clause) | Wave 0 |
| EV-05 | the ledger holds five rows, all `evidence: none`, all dispositioned | unit (shape) | `awk '!/^#/ && NF' data/expected/handoffs.ids \| wc -l` = 5 and every row matches `^H7-1 \|^H7-2 \|^H8-3 \|^D10 \|^D11 ` with `evidence: none` | Wave 0 |
| EV-05 | a claim citing a handoff-shaped artifact is refused | unit (control) | `sh tools/gate.sh` -> `control ok   claims-handoff-artifact` | Wave 1 |
| EV-05 | a claim citing a `build/`, `.wav`, `.ppm` or `.tsv` path is refused | unit (control, same one) | the same control's EXPECT token `FAIL claim-handoff-artifact: CL-0001` | Wave 1 |
| EV-05 | the look/listen words need a claim tag | unit (control) | `sh tools/gate.sh` -> `control ok   text-untagged-look-listen` | Wave 1 |
| EV-05 | the ledger does not become a claim's evidence | unit (step) | `sh tools/gate.sh` -> `claims=28 provisional=27` unchanged | yes |

### Sampling Rate

- **Per task commit:** the step(s) the task touched, run directly
  (`sh tools/check/steps/claims.sh`, `text.sh`, `c-purity.sh` with `ROOT`,
  `CHECK_DIR`, `FILELIST` exported), plus the one control by running
  `run-controls.sh` if the task added one.
- **Per wave merge:** `sh tools/gate.sh` (about 82 s on this host) with the full
  summary line compared token by token with the plan's prediction.
- **Phase gate:** `sh tools/gate.sh` and `dash tools/gate.sh` byte-identical,
  `sh tools/gate-linux.sh` green, `git status --porcelain` empty,
  `steps=38/38` and `claims=28` as criterion 5 requires.

### Wave 0 Gaps

- [ ] `docs/running.md` — the route document (criterion 1, PKG-05).
- [ ] `docs/presentation.md` — the six-clause policy (criterion 2, TOOL-04).
- [ ] `docs/handoffs.md` — the H7-1/H7-2 packet (CONTEXT; Claude's discretion on
      the name).
- [ ] `data/expected/handoffs.ids` — the five-row ledger (criterion 3, EV-05).
- [ ] The one-line fix in `.planning/phases/09-.../09-SECURITY.md:122`, without
      which `steps=38/38` is unattainable (Q1, Q11 C3).
- [ ] The decision on registry rows versus exclusion rows (Q10), because it
      determines whether `lines`/`gates` move at the close.

*(No framework install is needed: the gate already exists and runs.)*

## Security Domain

`security_enforcement` is `true` and `security_asvs_level` is `1` in
`.planning/config.json`, so this section is required.

### Applicable ASVS Categories

| ASVS Category | Applies | Standard Control |
|---------------|---------|-----------------|
| V2 Authentication | no | the project has no authentication surface |
| V3 Session Management | no | no session, no cookie, no token |
| V4 Access Control | no | no server, no role, no permission model |
| V5 Input Validation | **yes** | the scenario parser (`runner/scenario.c`) is the one untrusted-input boundary: fixed caps, a pure function, no allocation (`docs/runner.md:279-285`). This phase adds **no** input path; the new checks read tracked files only. |
| V6 Cryptography | no new use | the existing SHA-256 use (`runner_sha256.c`, `machine_digest.c`) is integrity, not authentication (`CL-0028`'s note says so). This phase adds no hash. |
| V7 Error handling and logging | **yes, lightly** | every new refusal is one line, names the id or the file, and never prints matched secret-shaped text (`tools/hygiene.sh:14-15` is the existing rule; the new `claim-handoff-artifact` line prints the claim id only). |
| V12 Files and resources | **yes** | the rehearsal extracts an archive into a `mktemp` directory and builds there; it must not write into the repository, and it must clean up via the existing `trap` (`release-rehearse.sh:57`). |

### Known Threat Patterns for this stack

| Pattern | STRIDE | Standard Mitigation |
|---|---|---|
| A committed render (`.ppm`, `.wav`) that is really derived art of rights-bearing content | Information disclosure / rights | `*.wav` and `*.ppm` in `.gitignore:18,20`; `binary-ext.txt` refuses them as release members; `--audio-wav`/`--frame-ppm` refuse a scenario with `content ... user`; the new `claim-handoff-artifact` rule refuses a claim that cites one |
| A build product cited as evidence | Repudiation | the new `claims` predicate set (`^build/`) |
| A path of the releasing host inside a shipped asset | Information disclosure | `release.sh:224-228` scans the staged install tree for the checkout path in both the logical and physical form |
| A tracked file carrying a home or scratch path (the Q1 defect) | Information disclosure | `tools/hygiene.sh --tree` in every gate, `--history` at release; the Q1 fix restores it |
| A policy document that becomes a de-facto colour claim | Repudiation / claims integrity | `docs/presentation.md` clause 3 (no colour claim) plus the `core-symbols` palette rule plus `text`'s claim-word tripwire |
| A handoff row silently upgraded to a pass | Repudiation | the row grammar has exactly two dispositions and the literal `evidence: none`; the `claims` rule refuses a claim citing a handoff-shaped artifact; `claims=28` must not move |
| Extracting an untrusted archive | Tampering | the archive is built locally from `HEAD` by `release.sh`; the rehearsal extracts into a `mktemp` directory, never into the repository, and `tar -xzf` writes only relative members |
| A gate step that passes vacuously | Repudiation | GUARDRAILS §2.4 (no case asserts only `exit 0`) and §2.3 (a control that can fail); every new control carries an EXPECT token and, where a second token could fire, a FORBID |

`[OBS]` No new attack surface is introduced: the phase adds two documents, one
data file, two control directories and two predicates. It adds no network call, no
service, no credential, no parser and no installed file.

## Sources

### Primary (HIGH confidence, read in this session)

- `tools/release-rehearse.sh` (164 lines, read in full) — the six steps, the flags,
  the `usage()` range `3,29`, the `trap` cleanup.
- `tools/release.sh` (274 lines, read in full) — `git archive HEAD` with prefix
  `ivs-$tag/` (`:170`), the archive name (`:166`), the member checks (`:200-205`),
  the host-path scan (`:224-228`).
- `docs/runner.md` (286 lines, read in full) — the invocation shapes (`:94`,
  `:247-259`, `:263`), the operator output options (`:35-40`), the report keys
  (`:114`, `:116-146`), the golden key list (`:243-245`).
- `tools/check/steps/golden.sh` (450 lines; `:146-161` and `:300-356` read in
  detail) — `live_count`, `filter_report`, the three-line comparison.
- `tools/check/steps/claims.sh` (13 lines, read in full),
  `tools/check/schema/claim.txt` (28 lines, read in full),
  `tools/check/kinds/claim.cmake` (152 lines, read in full),
  `tools/check/records.cmake` (333 lines, read in full),
  `tools/check/lib.cmake:75-101` (`read_ids`).
- `tools/check/steps/text.sh` (96 lines, read in full) — the word list at `:45`,
  the exemption at `:43`/`:52`.
- `tools/check/core-symbols.sh` (208 lines, read in full) — the two
  defined-symbol rules at `:152-173`; `tools/check/core-symbols-deny.txt` and
  `core-symbols-allow.txt` (read in full).
- `tools/check/steps/c-purity.sh:1-80` — the two callers and the archives.
- `tools/check/run-controls.sh` (read in the regions `:1-30`, `:150-275`,
  `:274-360`) — the control grammar, `EXPECT`/`FORBID`/`overlay`/`MAKE`, the
  inventory checks, `_good` derivation.
- `tools/gate.sh` (read in full) — the summary-line construction, the pass-line
  rule, the token accumulation.
- `tools/hygiene.sh:1-190` — `HOME_PATH`/`HOME_NEW`, `--tree`, `--history`, the
  baseline.
- `tools/check/steps/important-gates.sh:120-181` — the derived accepted set and
  the two-directional ledger.
- `data/expected/important-gates-excluded.ids`,
  `data/expected/state-cuts.ids`, `data/expected/state-sweep-notes.ids`,
  `data/expected/d10-measurements.ids`, `data/expected/controls/claims.ids`,
  `data/expected/controls/gate-hardening.ids`,
  `data/expected/important-gates.ids` (counted), `data/claims.json` (all 28
  artifacts enumerated) — read in full.
- `tools/check/controls/core-symbols-palette-defined/{CHECK,EXPECT,overlay/core/src/cpu/good.c}`,
  `tools/check/controls/claims-dangling-artifact/`,
  `tools/check/controls/text-untagged-claim-word/`,
  `tools/check/controls/text-untagged-playable/` — read in full.
- `README.md` (72 lines, read in full), `docs/packaging.md:100-149`,
  `docs/video/demo-lamp-tone-1-derivation.md:268`,
  `docs/provenance.md:82`.
- `.planning/phases/10-.../10-CONTEXT.md`, `.planning/MILESTONE-CONTEXT.md` §2,
  `.planning/ROADMAP.md` Phase 10, `.planning/REQUIREMENTS.md`,
  `.planning/ops/GUARDRAILS-v0.2.md`, `.planning/ops/PLAN-LESSONS.md`,
  `.planning/ops/INDEX.md`, `.planning/ops/EXEC-RULES.md`,
  `.planning/STATE.md` (Recorded Constraints and Deferred Items),
  `.planning/phases/09-.../09-05-SUMMARY.md`, `09-09-SUMMARY.md`,
  `.planning/milestones/v0.1.0-phases/07-.../07-VERIFICATION.md:85-125`,
  `.planning/milestones/v0.1.0-phases/08-.../08-MILESTONE-CLOSE.md:368-430`.
- `AGENTS.md`.

### Commands run in this session (all outputs quoted above)

- `sh tools/gate.sh` — the baseline summary line (82 s, `steps=36/38`).
- `sh tools/release.sh --no-gate --out /tmp/rel10` — the archive name and size.
- `tar -xzf` into a clean directory, `cmake --preset release`,
  `cmake --build --preset release` — the cold build (2.5 s, 244 targets).
- `build/release/runner/ivs-run --content-dir build/release/images data/scenarios/demo-lamp-tone-pass.ivss`
  — the 131-line report, the nine-line tail, `host.calls 11`.
- the same with `--slice-ticks 1000` — `host.calls 1599`.
- the `filter_report` reproduction — the 124-line golden match.
- `sh tools/listen-demo.sh demo-lamp-tone-pass` — 21674 bytes, `refpcm.sha256`.
- `ivs-run --frame-ppm` on `demo-lamp-tone-idle`, `demo-lamp-tone-pass` and
  `diag-stic2-mob-pass` — 45806 bytes, mode 0600, header `P6 159 96 255`.
- `grep -c -F 'nothing downloadable is runnable'` on both prose files — 0 and 1.
- the candidate-word scan over `README.md`, `CHANGELOG.md` and `docs/**/*.md`.

### Secondary (MEDIUM confidence)

- None. No web source, no Context7 lookup and no MCP provider was used: every
  question in this phase is answered by the repository itself, and
  `.planning/config.json` has every search provider disabled
  (`brave_search`, `exa_search`, `firecrawl`, `tavily_search`, `ref_search`,
  `perplexity`, `jina` all `false`).

### Tertiary (LOW confidence)

- None.

## Metadata

**Confidence breakdown:**

- Standard stack: **HIGH** — no package is added; every tool is preflighted by an
  existing script and its version is read from a gate token.
- Architecture (where each capability lives, and the exact edit regions): **HIGH**
  — every line number and every mechanism was read in this session, and the
  end-to-end route was executed.
- Pitfalls: **HIGH** for the six measured ones (the red tree, the missing
  sentence, the already-existing colour rule, the 0.19 cpu-s control unit, the
  toy-tree absence, the derived accepted set); **MEDIUM** for the claim-word blast
  radius, which is a count of lines today and will grow as prose is added.

**Research date:** 2026-11-24
**Baseline commit:** `f3a6c67` (`docs(10): smart discuss context`), clean worktree
**Valid until:** the next commit that touches `tools/check/steps/*.sh`,
`tools/gate.sh`, `README.md`, `docs/packaging.md` or `data/claims.json` — the line
numbers and counts above are pinned to this tree, so a plan that lands after such
a commit must re-measure rather than quote (AGENTS.md: "Recompute, never quote").
