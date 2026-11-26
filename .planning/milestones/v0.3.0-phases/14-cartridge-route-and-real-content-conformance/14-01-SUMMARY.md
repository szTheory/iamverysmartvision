---
phase: 14-cartridge-route-and-real-content-conformance
plan: 01
subsystem: content route (the project's second IVSC v1 writer) and the pack suite
tags: [tool, container, refusals, gate-wiring, derived-material]
status: complete
dependency-graph:
  requires:
    - core/src/content/ivsc.h (the container's constants, internal header)
    - core/src/common/ivs_sha256.h (the digest, via ivs_core)
    - ivs_core_internal (the include path tools/pack already links)
    - tools/check/steps/c-build.sh, tools/check/integrity.cmake, tools/check/controls/
  provides:
    - tools/flatwrap/flatwrap.c (ivs_flatwrap, build-only)
    - data/expected/flat-refusals.ids (seven tokens, append-only)
    - nine pack ids (seven refusal-plus-positive cases, two judges)
  affects:
    - plan 14-03 (the runner-process round trip reuses the wrapped container this tool writes)
    - plan 14-04 (the five GUARDRAILS 2.4 rows this plan's ids need land there)
tech-stack:
  added: []
  patterns:
    - a build-only tool shares the core's container layout through ivs_core_internal instead of copying it
    - a refusal token is pinned append-only in data/expected/<surface>-*.ids and minted into one case per token
    - an output that is derived rights-bearing material is created exclusively (fopen "wbx"), never truncating
key-files:
  created:
    - tools/flatwrap/flatwrap.c
    - tools/flatwrap/CMakeLists.txt
    - data/expected/flat-refusals.ids
    - tools/check/controls/c-build-unlisted-flatwrap-source/CHECK
    - tools/check/controls/c-build-unlisted-flatwrap-source/EXPECT
    - tools/check/controls/c-build-unlisted-flatwrap-source/overlay/tools/flatwrap/extra.c
  modified:
    - CMakeLists.txt
    - tools/check/steps/c-build.sh
    - tools/check/controls/_good/tree/CMakeLists.txt
    - data/expected/controls/runner-hygiene.ids
    - tools/check/integrity.cmake
    - docs/packaging.md
    - tests/pack-tap.sh
    - tests/CMakeLists.txt
    - data/expected/tests/pack.ids
decisions:
  - the executable is ivs_flatwrap and the source list is the hyphenated ivs-flatwrap-sources.txt
  - the eight magic bytes are restated in the tool, not exported from the core
  - the SPDX alternation gains flatwrap with no fourth control directory
  - the 65,536-word pair is a token pair, not a refuse-to-accept pair
  - the input size is decided from the exact byte count, not from a cap-plus-one read
metrics:
  duration: ~26 min
  completed: 2026-11-25
  tasks: 2
  files: 15
actuals:
  tokens: 12246
  tasks: 2
  commits: 2
plan_head_before: 0825c0e
plan_head_after: d17cc3b
requirements: [TOOL-07]
---

# Phase 14 Plan 01: the wrap tool and the first real content route

One flat file, one stated base and one stated word order become the project's own `IVSC v1` container
through a build-only tool that states bytes and detects nothing, and that container loads through the
public API as `content cart user` with `slot.cart.sha256` equal to the digest the tool printed on stdout;
its seven typed refusals are pinned append-only and each has one case that asserts the refusal beside its
boundary positive.

## What was built

**Task 1 (`a25db11`, 12 files).** `tools/flatwrap/flatwrap.c` — one C17 file, ISO C apart from the POSIX
`realpath` call the work-tree rule needs — implements `ivs_flatwrap <flat> <base> <out>
[--word-order hi-first|lo-first]`. It reads the container's sizes, version, width, word maximum and
address space from `content/ivsc.h` and the digest from `common/ivs_sha256.h`, both over the
`ivs_core_internal` include path `tools/pack` already links, so the layout has one definition. Check
order is the published contract and is stated in the file's header comment: length → words → base →
crossing → output directory → output exists → word range. Seven tokens, one stderr line
`ivs_flatwrap: <token>: <detail>`, exit 1, no output file; every other failure is a file or usage error
with no token and exit 2. The output is created exclusively (`fopen(path, "wbx")`, the portable short form
of `runner/hostio.c:137`'s `O_CREAT|O_EXCL`), and a failed write removes the file.

The same commit carries the whole same-commit set a new `tools/<x>/*.c` owes (GUARDRAILS V5 plus the limb
V5 does not name): the root `add_subdirectory`, the target and the repo-relative source-list writer
(`ivs-flatwrap-sources.txt`), the `c-build` module list and its header enumeration, the toy tree's seventh
list, the `c-build-unlisted-flatwrap-source` control and its inventory line, the SPDX alternation in
`tools/check/integrity.cmake`, and the two "not installed" sentences in `docs/packaging.md`.
`data/expected/flat-refusals.ids` pins the seven tokens.

**Task 2 (`d17cc3b`, 3 files).** `tests/pack-tap.sh` takes the tool as a third argument and mints
`pack.flatwrap.neg.<token>` by looping over the pinned file, so a token added to the pin without a case is
a finding rather than a silent gap. Each case asserts the refusal **and** its boundary positive in one
case. `pack.judge.flatwrap-tokens` diffs the pin against the literals the source can print and against the
tokens the cases observed; `pack.judge.flatwrap-digest-live` checks the printed digest and the trailer
with `cmake -E sha256sum` (an implementation other than the core's) and shows the equality live.

## Measured round trip (task 1 `<automated>`, exit 0)

```
$ build/dev/tools/flatwrap/ivs_flatwrap $t/flat.bin 5000 $t/wrapped.ivsc
23db1e4931fade388ae28ee4a1d1d80e9a575b5be4d4609add1fd1884fd65857
$ build/dev/runner/ivs-run --content-dir $t --exec $t/exec.bin --grom $t/grom.bin \
      --cart $t/wrapped.ivsc $t/scn.ivss          # content exec/grom/cart user, run-to 1000 ticks
outcome pass
exit.code 0
slot.cart.loaded 1
slot.cart.length_bytes 8256
slot.cart.sha256 23db1e4931fade388ae28ee4a1d1d80e9a575b5be4d4609add1fd1884fd65857
slot.cart.status ok
exec: -
$ cmake -E sha256sum $t/wrapped.ivsc
23db1e4931fade388ae28ee4a1d1d80e9a575b5be4d4609add1fd1884fd65857  …/wrapped.ivsc
```

The container is 8,256 bytes = 24 prefix + 8 entry + 2 × 4,096 words + 32 trailer. The three digests — the
one the tool printed, the one the loader published as `slot.cart.sha256`, and `cmake -E sha256sum` of the
file — are the same value, and that value reproduces 14-RESEARCH's own hand-built 8,192-zero-byte
container at `$5000` byte for byte (`23db1e49…`), which is an independent confirmation that the writer
produces exactly the layout the spec describes rather than merely one the parser accepts.

The two report lines are asserted **separately**, each as a whole line with its own count, so an
alternation satisfied by either half alone cannot pass while the digest differs.

## The 65,536-word deviation from criterion 2's literal wording

Criterion 2 asks for a "65,537↔65,536 words" pair beside the other six, and every other pair is
refuse↔accept. For this one an acceptance is impossible twice over: `IVS_IVSC_MAX_SEGMENT_WORDS` is
`65,535u` and `len_words` is a 2-byte field, so a one-segment writer cannot encode 65,536 words at all;
and `$4000 + 65536 = $14000` crosses `$10000` first, so the crossing rule refuses it anyway. The pair is
therefore a **token** pair, and `pack.flatwrap.neg.flatwrap-words` asserts both halves: 65,537 words
(131,074 bytes) is refused with `flatwrap-words`, and 65,536 words (131,072 bytes) is refused with
`flatwrap-cross`. That pins the count rule's threshold at exactly 65,536 and proves the writer never emits
a wrapped or zero `len_words`. The real acceptance boundary is `flatwrap-cross`'s own pair: 49,153 words
at `4000` refused, 49,152 words at `4000` accepted (`$10000 - $4000 = $C000 = 49,152`), with the entry read
back as `4000c000` so its last word sits at `$FFFF`.

## Decisions

1. **`tools/flatwrap` is a path and `ivs_flatwrap` is the executable** (the underscore convention of
   `ivs_pack`, `ivs_asm`, `ivs_bench`); the source list the build writes is the hyphenated
   `ivs-flatwrap-sources.txt`. The two names are recorded side by side because they will otherwise read
   as a typo: `ivs_flatwrap` is the target and the binary, `ivs-flatwrap-sources.txt` is the inventory the
   `c-build` step compares against.
2. **The eight magic bytes are restated in `flatwrap.c`** rather than declared in `content/ivsc.h`. An
   `extern` definition would export a new core symbol and move the gate's `defined=`/`public-symbols=`
   figures — a core surface change for a build-only tool. `tools/pack/pack.c:298` already restates the
   same eight bytes, so this is the tree's existing practice, and the drift detector is the load itself:
   the core's parser refuses a container whose magic differs, and task 1's verify loads one.
3. **The SPDX alternation gains `flatwrap` with no fourth control directory.** `tools/bench` was added to
   the same alternation in plan 08-18 with no `integrity-missing-spdx-tools-bench` control, so "one
   control per directory" already described the first four and was not a rule. The rule stays proved
   three times (`integrity-missing-spdx-tools-c`, `-asm`, `-pcm`) and the 0.31 cpu-s a fourth control
   costs is not spent on it; the control this plan does add is the `c-build` unlisted-source guard for the
   new directory. This is why the phase's priced addition is **1.535** and not 1.845 (1.535 + 0.31), the
   figure plan 14-04 computes and prints.
4. **The input's size is decided from the exact byte count**, not from a cap-plus-one read. `fseek`/
   `ftell` give the length before any byte is read, so an oversized file never reaches the 131,074-byte
   buffer, the word count is checked before any multiplication by two, and an odd truncated read can never
   mislabel an oversized even file as `flatwrap-length`. The observable contract — "an oversized file
   reaches the word-count rule rather than the heap" — is exactly what the plan asked for; only the
   mechanism differs, and it is ISO C.

## Deviations from Plan

### GUARDRAILS §2.4: five must-fail rows land in plan 14-04, not here (recorded, not silent)

§2.4 requires each new check's must-fail mechanism row in `data/expected/important-gates.ids` to ship in
the check's own commit. **That row cannot ship in this one**: `important-gates.sh` refuses a row naming a
`Pending` requirement, and TOOL-07, SCN-03 and CPU-06 all stay `Pending` until plan 14-04's flip — so a
row here is `important-gates-unknown-id` and a flip here is `important-gates-complete-uncovered`. Exactly
**five** rows land in 14-04's task 1, and they are these five and no larger set:

| Row | Owning plan |
|---|---|
| `flatwrap control c-build-unlisted-flatwrap-source TOOL-07` | this plan |
| `flatwrap mutant pack.judge.flatwrap-digest-live TOOL-07` | this plan |
| `cart-role mutant runner-process.judge.flatwrap-roundtrip SCN-03` | 14-03 |
| `cart-role mutant runner-process.judge.owner-run-cart-pass SCN-03` | 14-03 |
| `cpu06 fault ACCEPT_CLEARS_INTFF_IGNORED CPU-06` | pre-existing fault, registered for the flip |

Two of this phase's ids owe no §2.4 row and the record says so rather than leaving a reader to count:
`pack.judge.flatwrap-tokens` is a coverage diff (its failure is a mismatch between the pin and the
literals the source can print, which needs no scratch mutation, so it has no stand-in limb to register),
and `runner-process.cart-user.refuse-derived` (14-03) is a refusal asserted by exit status plus one typed
token, not an equivalence with a stand-in.

What does ship here, per §2.4's other two limbs, is the liveness guard and its committed stand-in:
`pack.judge.flatwrap-digest-live`'s flipped-byte limb is the guard, and the same-input-twice equality is
the stand-in it can fail. The commit that closes this deviation is 14-04's task 1.

### Plan defects found by measurement (recorded, not faked)

1. **The `c-build` acceptance criterion's guard mutation does not fail as written.** The criterion claims
   that deleting the `'^tools/flatwrap/.*[.]c$ ivs-flatwrap-sources.txt'` item from the `for mod in` list
   makes the step report `c-build-unlisted-source: tools/flatwrap/flatwrap.c`. Measured: the step still
   passes, exit 0 — deleting the item does not tighten the rule, it removes the check, so nothing reports
   anything. The mutation that does prove the mechanism is the one the control uses: a tracked
   `tools/flatwrap/probe-unlisted.c` that the build does not list produces
   `-- FAIL c-build-unlisted-source: tools/flatwrap/probe-unlisted.c` and exit 1 (measured), and the
   committed control `c-build-unlisted-flatwrap-source` reproduces it in the gate
   (`control ok   c-build-unlisted-flatwrap-source`, `controls=402/402`).
2. **`grep -c 'install' tools/flatwrap/CMakeLists.txt` reads 1, not 0.** The criterion's baseline assumes
   the word does not occur; the file's header comment states the policy in the tree's own idiom ("no
   install rule, never shipped"), exactly as `tools/pack/CMakeLists.txt` does (`grep -c install` there
   reads 2). Removing the word to make a crude grep read 0 would delete the sentence that documents the
   policy, so the comment stays and the substantive checks are recorded instead: `grep -cE
   '^[[:space:]]*install[[:space:]]*\(' tools/flatwrap/CMakeLists.txt` → 0, and
   `data/expected/pkg-install-files.ids` still lists 11 paths after the gate's `pkg-install` step.
3. **The word-count case cannot carry a refuse↔accept pair** — the recorded deviation above, with its
   arithmetic; the case pins a token pair instead and the acceptance boundary moved to `flatwrap-cross`.
4. **The generic state step's `requirements.mark-complete TOOL-07` had to be reverted.** The executor
   contract's `<state_updates>` says to mark the plan's `requirements:` complete, and doing so made the
   gate refuse: with TOOL-07 `Complete` and no line for it in `data/expected/important-gates.ids`, the
   `important-gates` step emits `important-gates-complete-uncovered: TOOL-07` (measured by reading the
   step's own rule: a Complete requirement with neither a registry line nor an exclusion ledger row
   fires it, and `data/expected/important-gates-excluded.ids` has no TOOL-07 row). TOOL-07, SCN-03 and
   CPU-06 are flipped by plan 14-04's task 1 together with the five §2.4 rows, which is exactly why the
   §2.4 deviation above exists; so `.planning/REQUIREMENTS.md` is left byte-identical (checkbox and
   traceability row both still `Pending`) and the flip is 14-04's.

### Auto-fixed issues

None. The plan's check order, token set, control shape and test ids were executed as written; the three
items above are contract defects in the plan, recorded with their measurements.

## §2.4 mechanisms and guards observed in this plan

- `c-build-unlisted-flatwrap-source` (control) — `control ok`, and the same mechanism measured by hand as
  `-- FAIL c-build-unlisted-source: tools/flatwrap/probe-unlisted.c`, exit 1.
- SPDX coverage — measured by deleting line 1 of `flatwrap.c`: `-- FAIL integrity-missing-spdx:
  tools/flatwrap/flatwrap.c`, exit 1 (header restored, `git diff` clean).
- `pack.flatwrap.neg.flatwrap-output-exists` — measured by a scratch mutant that truncated the existing
  file before refusing: `not ok … # the refused run changed the existing file`. The exit code alone would
  have passed, which is the point of the byte-unchanged half.
- `pack.judge.flatwrap-digest-live` — measured by a scratch mutant that dropped the last written word, so
  a flipped input byte could not move the digest: `not ok … # judge-vacuous: flipping one input byte left
  the digest at ea599cab…`. The judge is therefore able to fail, not a second equality check.

The frontmatter's `commits: 2` is the measured task-commit count from `plan_head_before` to
`plan_head_after` (`git rev-list --count 0825c0e..d17cc3b` = 2, one per task). The metadata commit that
carries this file is the third, by design: it is separate from the per-task commits, so a verifier
reading `git rev-list --count 0825c0e..HEAD` at the final commit reads 3 and the two figures do not
contradict each other.

## Token delta actually measured

| Figure | Before this plan | After | Arithmetic |
|---|---|---|---|
| `tests-expected` | 4,990 | **5,008** | +9 ids × 2 presets = 18 |
| `tests-executed` | 4,986 | 5,004 | the same 18, `tests-skipped=4` unchanged, `tests-missing=0` |
| `controls` | 401 | **402** | one control directory and its one inventory line |
| `tracked` / `files` / `hygiene-files` | 2,736 (2,732 before this phase's four PLAN.md) | **2,742** | +6: `flatwrap.c`, `tools/flatwrap/CMakeLists.txt`, `flat-refusals.ids`, the control's `CHECK`, `EXPECT` and `overlay/tools/flatwrap/extra.c` |
| `steps` / `faults` / `goldens` | 38 / 238 / 29 | 38 / 238 / 29 | unchanged invariants |
| `prefix-files` | 11 | 11 | no install rule |
| `pkg-selftest` | 19/19 | 19/19 | unchanged |

Gate at the task-2 commit: `sh tools/gate.sh` exit 0, `steps=38/38 controls=402/402 faults=238/238
goldens=29 tests-expected=5008 tests-executed=5004 tests-skipped=4 tests-missing=0 prefix-files=11
tracked=2742 files=2742`, `gate-time total=83`.

Gate at the final commit, `sh tools/gate.sh` and `dash tools/gate.sh`, both exit 0 with byte-identical
last lines: `steps=38/38 controls=402/402 faults=238/238 goldens=29 tests-expected=5008
tests-executed=5004 tests-skipped=4 tests-missing=0 prefix-files=11 tracked=2743 files=2743`. The one
file between 2,742 and 2,743 is this SUMMARY.md itself, so both readings are right for their commit.

## Derived-material checks

- `git status --porcelain --ignored=matching --untracked-files=all` (status asserted 0) then
  `grep -c -E '[.]ivsc$'` → 0. A bare `git ls-files` or `git status --porcelain` cannot see an ignored
  untracked container, so neither was used as the check.
- `.gitignore` and `tools/check/binary-ext.txt` are byte-identical to the starting commit: this phase
  added no extension and removed no refusal surface.
- No container, no fixture and no golden was created by this plan; the only digest printed is the one
  line on stdout, and the suite's own containers live in its `mktemp -d` and are removed by its trap.

## Known Stubs

None. Every value the tool writes comes from an operator-stated fact or from the core's own constants;
there is no placeholder, no unwired data source and no skipped case.

## Deferred Issues

- The five `important-gates.ids` rows above (blocked on 14-04's requirement flip, by design), and with
  them the TOOL-07 / SCN-03 / CPU-06 completion flip itself.
- Nothing else. The `c-build` and SPDX guard mutations that the plan's acceptance criteria mis-state are
  recorded here with the mutations that do work; the mechanisms themselves are green and proved.
- `state.update-progress` declined (`progress percent withheld by buildStateFrontmatter`), so STATE.md's
  frontmatter progress block is unchanged. That refusal is a pre-existing property of this STATE.md (its
  `progress:` block reads `total_phases: 4 completed_phases: 8`), not an effect of this plan; the plan
  counter and the ROADMAP row were updated directly instead (`1/4 plans executed`, plan 14-01 checked).

## Self-Check

- `tools/flatwrap/flatwrap.c` — FOUND
- `tools/flatwrap/CMakeLists.txt` — FOUND
- `data/expected/flat-refusals.ids` — FOUND (7 tokens)
- `tools/check/controls/c-build-unlisted-flatwrap-source/{CHECK,EXPECT,overlay/tools/flatwrap/extra.c}` — FOUND
- commit `a25db11` — FOUND (ancestor of HEAD)
- commit `d17cc3b` — FOUND (ancestor of HEAD)
- `data/expected/tests/pack.ids` — 23 ids (14 + 9)
- `data/expected/pkg-install-files.ids` — 11 paths

**Self-Check: PASSED**
