---
phase: 14-cartridge-route-and-real-content-conformance
plan: 03
subsystem: the owner-local route's cartridge role (an option, its six refusals, its round trip and two operator pages)
tags: [cartridge-role, derived-output-refusals, snapshot-rule, digest-liveness, option-not-positional, operator-pages]
status: complete
dependency-graph:
  requires:
    - tools/flatwrap/flatwrap.c (ivs_flatwrap, plan 14-01: the container this plan's cases load)
    - runner/main.c (`--cart <file>` is already the operator option for `content cart user`, and the three refusal loops are already role-generic)
    - data/scenarios/owner-exec-title.ivss (the marker line the helper inserts after)
    - data/expected/rights-pinned.ids (the four `<token> <path>` pins that must keep matching)
  provides:
    - three `runner-process` ids (one refusal-plus-snapshot case, two judge-shaped pass cases)
    - the suite's optional sixth argument, ivs_flatwrap
    - tools/owner-run.sh `--cart <cart-image>` and the conditional `content cart user` insertion
    - docs/owner-images.md's cartridge role and docs/RIGHTS.md's disposition sentence
  affects:
    - plan 14-04 (its two `cart-role mutant` rows over `runner-process.judge.flatwrap-roundtrip` and `runner-process.judge.owner-run-cart-pass`)
tech-stack:
  added: []
  patterns:
    - an operator role that is a path becomes an option, never a third positional, when an existing id asserts the positional arity and the lock grammar
    - a generated behaviour switch asserts its own count against the arguments, so the conditional cannot silently invert
    - an equality between a tool's printed value and a report key is proved live by moving one input byte, and the second half fails if both values stop moving
    - a credential-shaped pass path is run on project-made zero-filled files, and one mutated byte after the lock is written shows the pin can fail
key-files:
  created: []
  modified:
    - tests/runner-process-tap.sh
    - tests/CMakeLists.txt
    - data/expected/tests/runner-process.ids
    - tools/owner-run.sh
    - docs/owner-images.md
    - docs/RIGHTS.md
decisions:
  - the cartridge is `--cart <cart-image>`, an option parsed in the existing loop, because nine existing ids pass exactly two positionals and the lock's grammar is asserted against a `cart` role
  - the helper adds no size rule for the container, because the core refuses a short one (reason 1) and a long one (reason 2) at load; a route rule would be a second registration of one refusal
  - the pass path's outcome is pinned to the value this plan measured (guest-fail, 10, `stop step=1 line=24`), not to the research's predicted `line=23`, because the inserted `content cart user` shifts the halted step's scenario line
  - the snapshot rule is executed as the ABSENCE of a seventh refusal, with the tick value derived as half the scenario's own `run-to` and a cut count required so the limb cannot pass vacuously
  - the page's ignore-pattern sentence now names `*.ivsc`, which the file and the binary extension list already carried but the page's list did not
metrics:
  duration: ~80 min
  completed: 2026-11-26
  tasks: 3
  files: 6
actuals:
  tokens: 24240
  tasks: 3
  commits: 3
plan_head_before: 2fcfd7d07de46db552270cff2420066581cb8c14
plan_head_after: 6837c8f
requirements: [SCN-03]
coverage:
  - id: D1
    description: "a scratch scenario naming all three user roles is refused for each of the six derived-output options (exit 14, one exact stderr line, no report, no file), the three audio options together name the first of their fixed order, and the snapshot rule is the absence of a seventh refusal (--roundtrip-at-ticks accepted, nothing written, report equal apart from host.calls, at least one cut)"
    requirement: SCN-03
    verification:
      - kind: unit
        ref: "ctest --preset dev|release -R '^runner-process$': runner-process.cart-user.refuse-derived"
        status: pass
    human_judgment: false
  - id: D2
    description: "a container the wrap tool wrote loads as `content cart user` with slot.cart.length_bytes 8256 and slot.cart.sha256 equal to the printed digest, and one flipped input byte moves both the printed digest and the report's value"
    requirement: SCN-03
    verification:
      - kind: unit
        ref: "runner-process.judge.flatwrap-roundtrip (both presets); scratch mutant that wraps the same input twice fails it as judge-vacuous"
        status: pass
    human_judgment: false
  - id: D3
    description: "the route takes the cartridge as an option, its pass path runs end to end on project-made files with `exec: -`, and a one-byte EXEC mutation after --record is refused at load (IVS_E_DIGEST_MISMATCH, exit 14)"
    requirement: SCN-03
    verification:
      - kind: unit
        ref: "runner-process.judge.owner-run-cart-pass (both presets); nine owner-run ids and bad-lock-line's `cart` example unchanged"
        status: pass
      - kind: other
        ref: "sh tools/check/steps/files.sh: rights-pinned=4/4 binaries-listed=0; tools/check/steps/text.sh: docs=64 claim-refs=26, no finding"
        status: pass
    human_judgment: false
---

# Phase 14 Plan 03: the cartridge role in the owner-local route, proved

A scenario that names all three user roles is refused for every one of the six derived-output options, the
snapshot rule is proved to be the *absence* of a seventh refusal, a container the project's own wrap tool
wrote round-trips through the runner with `slot.cart.sha256` equal to the digest the tool printed (proved
live by flipping one input byte), and the owner-local route takes the cartridge as `--cart` — an option, not
a third positional — running its pass path on zero-filled project-made files with a pinned outcome that a
one-byte EXEC mutation shows can fail.

## What was built

**Task 1 (`b3beccb`, 3 files).** The suite takes an optional sixth argument, `ivs_flatwrap`, wired in
`tests/CMakeLists.txt` and bound through the existing `abs_file` pattern; when it is absent the cartridge
cases report `not ok` with that reason, never a skip, so a configuration without the tool cannot pass. The
new case `runner-process.cart-user.refuse-derived` builds a scratch content directory under `$tmp` (so no
tracked scenario exists and no `skip-ok` count moves) with a scenario naming `content exec user`,
`content grom user` and `content cart user`, zero-filled `exec.bin` (8192) and `grom.bin` (2048), and a
container the tool wrote from a zero-filled flat file, and asserts six refusals plus the snapshot limb.

**Task 2 (`50024a5`, 2 files).** `runner-process.judge.flatwrap-roundtrip` wraps a zero-filled 8192-byte flat
file at `$4000` into `$tmp`, runs the scratch scenario with `--cart`, and pins five `slot.cart.*` facts, the
`exec: -` label and the measured outcome/exit pair; then it flips one byte of the tool's *input* and requires
the second digest to differ and the second run's report to carry it.

**Task 3 (`6837c8f`, 5 files).** `tools/owner-run.sh` gains `--cart <cart-image>` in its existing option loop,
the cartridge joins the location loop (so an in-tree container outside `refs/` is refused like the other two)
and gets a regular-file check but no size rule; the `awk` step that inserts the two `expect-sha256` lines now
conditionally inserts one `content cart user` line and asserts its count against the arguments. The case
`runner-process.judge.owner-run-cart-pass` runs the pass path and its mutation limb. Both operator pages are in
the same commit, per V10.

## Measured values (all from this plan's runs)

### The six refusals, one line each, exit 14, no report, no file

```
ivs-run: --frame-ppm refused: content user
ivs-run: --video-raw refused: content user
ivs-run: --audio-raw refused: content user
ivs-run: --psg-log-out refused: content user
ivs-run: --audio-wav refused: content user
ivs-run: golden refused: content user        (--update-golden)
```

The all-three-at-once run (`--audio-wav --psg-log-out --audio-raw`) names `--audio-raw`, the first of the
fixed order, and leaves none of the three files.

### The snapshot limb

Plain run against the hook run, both `run-to 1000 ticks`, container at `$4000`:

```
plain:      host.calls 6
--roundtrip-at-ticks 500: host.calls 8      (two host calls per cut, so one cut)
bodies (grep -v '^host.calls ') : IDENTICAL
files in the scratch content dir before/after: IDENTICAL (5 entries each)
```

The tick value is arithmetic over the scenario's own `run-to` (`cu_to / 2`), not a guess, and the cut count
is required, so the limb fails rather than passing vacuously when the hook makes no cut (measured: dropping
`--roundtrip-at-ticks` from the run gives `not ok … host.calls 6 against the plain run's 6`).

### The round trip and its liveness limb

```
$ ivs_flatwrap flat.bin 4000 a.ivsc        8d73b98c58bb75a6515c6a791ebd2925265d05874e324c31a5836e61055b74f5
  wc -c a.ivsc                             8256            (24 + 8 + 2 x 4096 + 32)
  slot.cart.loaded                         1
  slot.cart.status                         ok
  slot.cart.length_bytes                   8256
  slot.cart.sha256                         8d73b98c58bb75a6515c6a791ebd2925265d05874e324c31a5836e61055b74f5
  outcome / exit.code                      pass / 0
  exec:                                    -

one input byte flipped (first word's low half, 00 -> 01, still <= $03FF):
$ ivs_flatwrap flat.bin 4000 b.ivsc        b6bc201272c1a65f882b7896433d8766b6e2c51ee01064d05c736076cae51ddf
  second run's slot.cart.sha256            b6bc201272c1a65f882b7896433d8766b6e2c51ee01064d05c736076cae51ddf
```

Measured stand-in: replacing the flipped-byte wrap by a second wrap of the same input makes the case fail
with `flatwrap-roundtrip: the flipped input byte left the printed digest at '8d73b98c…' (second '8d73b98c…'):
the equality is vacuous`, so the equality compares two movable things.

### The route's pass path and its mutation limb

Scratch root outside the work tree, `refs/e.bin` 8192 zero bytes, `refs/g.bin` 2048, `refs/c.ivsc` written by
the tool from a zero-filled flat file, `--record` writes the two-role lock and goes on:

```
run with --cart refs/c.ivsc
  first stdout line    OWNER-LOCAL RUN (D11): inputs are the owner's, not redistributable, not run in CI, not evidence, no claim
  stderr               (empty)
  slot.exec.loaded     1
  slot.cart.loaded     1
  slot.cart.sha256     8d73b98c58bb75a6515c6a791ebd2925265d05874e324c31a5836e61055b74f5
  exec:                -
  outcome              guest-fail
  exit.code            10
  outcome.detail       stop step=1 line=24 expected=target got=halted
  exit status          10

one EXEC byte flipped after --record, same run
  exit status          14
  outcome.detail       load-refused role=exec status=IVS_E_DIGEST_MISMATCH
  cpu.run_state        unreset
```

Measured stand-ins both ways: with the mutation made a no-op the case reports `the mutated EXEC run: exit 2,
expected 14`, `… does not name the digest mismatch at load`, `… executed the machine`; and a helper whose
`awk` always inserts `content cart user` is refused by its own count assertion,
`owner-run: the temporary scenario copy does not carry the cartridge role the arguments state`.

### The no-cartridge path is byte-identical to the pre-plan helper

Both helpers (the pre-plan one from `2fcfd7d:tools/owner-run.sh` and this one) were run in separate scratch
roots with the same two zero-filled images, `--record` and no `--cart`, and a runner wrapper that captured
the temporary scenario:

```
diff <pre-plan temp scenario> <this commit's temp scenario>   -> identical
diff <pre-plan report>        <this commit's report>          -> identical
```

### Token delta actually measured

| Figure | Before this plan | After | Arithmetic |
|---|---|---|---|
| `tests-expected` | 5,008 | **5,014** | +3 ids x 2 presets = 6 |
| `tests-executed` | 5,004 | 5,010 | the same 6, `tests-skipped=4` unchanged, `tests-missing=0` |
| `runner-process` ids | 135 | **138** | 3 new ids, all `ok` in both presets |
| `controls` | 402/402 | 402/402 | unchanged (no control added: the three ids are refusals, equalities and a pin, and 14-01's deviation table already assigns the `cart-role mutant` rows to 14-04) |
| `steps` / `faults` / `goldens` | 38 / 238 / 29 | 38 / 238 / 29 | unchanged invariants |
| `rights-pinned` | 4/4 | 4/4 | the four `<token> <path>` pins still match |
| `binaries-listed` | 0 | 0 | no new binary extension; `binary-ext.txt` and `.gitignore` byte-identical to `2fcfd7d` |
| `tracked` | 2,744 | 2,744 | no file created; the SUMMARY is the only addition (2,745 at the metadata commit) |
| `prefix-files` | 11 | 11 | the route adds no installed file (D-49) |

Gate at the task-3 commit (`6837c8f`, before this file): `sh tools/gate.sh` exit 0,
`steps=38/38 controls=402/402 faults=238/238 goldens=29 tests-expected=5014 tests-executed=5010
tests-skipped=4 tests-missing=0 rights-pinned=4/4 prefix-files=11 binaries-listed=0 tracked=2744`.

Gate at the final commit, `sh tools/gate.sh` and `dash tools/gate.sh`, both exit 0 with byte-identical last
lines: `gate: PASS steps=38/38 controls=402/402 … faults=238/238 … goldens=29 … tests-expected=5014
tests-executed=5010 tests-skipped=4 tests-missing=0 … rights-pinned=4/4 … prefix-files=11
tracked=2745 binaries-listed=0`, the one file between 2,744 and 2,745 being this SUMMARY.

## Decisions

1. **`--cart` is an option, and the lock keeps exactly two roles.** Nine existing ids pass exactly two
   positionals; `runner-process.owner-run.bad-lock-line`'s `cart` example is a refusal that must stay one; the
   template insertion asserts two `expect-sha256` lines; and the container's identity is already
   `slot.cart.sha256` in the report. All four are checkable, and all four are in the helper's header comment
   so a later reader does not re-litigate the choice.
2. **No cart size rule in the route.** The core refuses a short container (reason 1) and a long one (reason 2)
   of `docs/formats/ivsc-v1.md` at load with a precise report line, so a route rule would register one refusal
   twice. The exec/grom exact sizes stay because those shapes are exact.
3. **The pass path is pinned to measurement, not to the research.** 14-RESEARCH's Code Example 4 predicted
   `stop step=1 line=23 expected=target got=halted` for the route without a cartridge; with the inserted
   `content cart user` line the halt names line 24. Both the research figure and this one are right for their
   own scenario, and the case pins the number this run measured. Recorded as a deviation below.
4. **The snapshot rule is executed as an absence.** Criterion 4's wording ("all six derived-output refusals
   and the snapshot rule fire") reads as seven refusals; the snapshot rule is the absence of a seventh, so the
   case proves both halves — six exact refusals, then an accepted hook run that writes nothing and leaves the
   report unchanged apart from `host.calls`.
5. **The cartridge lives in `refs/`, and the page now says so.** The location loop covers it, so an in-tree
   container outside `refs/` is refused with the same message as the images (measured in the new case). The
   page's ignore-pattern sentence named `/refs/`, `*.bin`, `*.grom` and `*.img`; `*.ivsc` was already in
   `.gitignore` and `ivsc` in `binary-ext.txt`, so the sentence was corrected to name it rather than left
   saying something narrower than the tree.

## Deviations from Plan

### 1. The research's measured `outcome.detail` line number moves from 23 to 24 (a plan figure, corrected by measurement)

- **Found during:** Task 3 (`runner-process.judge.owner-run-cart-pass`).
- **Issue:** the plan says to pin "what the run measured" and warns that the research's `guest-fail`/exit 10
  figure is a prediction. The prediction was right about outcome and exit code; its `outcome.detail` names
  scenario line 23, which is the `run-to 600 frames` line of the unmodified template. The helper's conditional
  insertion puts `content cart user` before `run-to`, so the tail is line 24.
- **Fix:** the case pins `stop step=1 line=24 expected=target got=halted`, the value this run measured, and the
  arm without `--cart` keeps line 23 (verified in the same scratch root). No code changed for this.
- **Files modified:** `tests/runner-process-tap.sh`.

### 2. GUARDRAILS §2.4's two `cart-role mutant` rows do not ship here (recorded, not silent)

14-01's deviation table already assigns five `important-gates.ids` rows to plan 14-04's task 1, because
`important-gates.sh` refuses a row naming a `Pending` requirement and SCN-03 stays `Pending` until that
flip. Two of those five are this plan's ids (`runner-process.judge.flatwrap-roundtrip` and
`runner-process.judge.owner-run-cart-pass`), so adding the rows here would be
`important-gates-unknown-id` and flipping SCN-03 here would be `important-gates-complete-uncovered`.
`runner-process.cart-user.refuse-derived` owes no §2.4 row: it is a refusal asserted by exit status plus one
typed line, not an equivalence with a stand-in. The liveness guards this plan does ship are committed and
measured — the flipped-byte limb (2) and the one-byte EXEC mutation (3).

### 3. `.planning/REQUIREMENTS.md` is left byte-identical (SCN-03 still `Pending`)

Same reason as 14-01's deviation 4: the generic `state requirements.mark-complete` step would make the
`important-gates` step fail with `important-gates-complete-uncovered: SCN-03`. The flip is 14-04's task 1,
together with the five §2.4 rows.

### 4. The suite's `usage` id gains one more run (a plan silence, filled)

The plan says the nine `owner-run` ids are "unchanged and passing". The usage assertion string had to move
with the usage line (it pins the whole string), and one run was added for `--cart` with no value, so the new
option's arity refusal is exercised rather than assumed. The id, its name and its count are unchanged; only
its body gained a run.

### Auto-fixed issues

**1. [Rule 1 - Bug] Em dashes in three added comment blocks failed `integrity-non-ascii`**
- **Found during:** the gate run before the task-3 commit.
- **Issue:** `integrity` refused `data/expected/tests/runner-process.ids` and `tests/runner-process-tap.sh` with
  `integrity-non-ascii`: seven U+2014 characters in comment prose, which the tree's ASCII rule forbids.
- **Fix:** replaced with `-`; no code or assertion changed. Re-ran the gate: `PASS steps=38/38 …`.
- **Files modified:** `tests/runner-process-tap.sh`, `data/expected/tests/runner-process.ids`.

**2. [Rule 1 - Bug] The task-1 commit carried all three ids while the suite emitted one**
- **Found during:** preparing the task-2 commit.
- **Issue:** the ids file was edited with all three new ids in one pass, so the task-1 commit's inventory held
  138 ids against a suite emitting 136 — a state `tests-inventory` refuses (`tests-missing`). That would have
  made the task-1 commit's gate red.
- **Fix:** the task-1 commit was amended (unpushed, local, on `main`) to hold exactly its own id, and each later
  task's commit adds its own id with its own comment block; each commit's inventory equals the suite's id set.
- **Files modified:** `data/expected/tests/runner-process.ids`.

## §2.4 mechanisms and guards observed in this plan

- `runner-process.cart-user.refuse-derived` — measured failing limb: dropping `--roundtrip-at-ticks` from the
  hook run gives `not ok … host.calls 6 against the plain run's 6: the hook made no cut, so the snapshot rule
  was not exercised`.
- `runner-process.judge.flatwrap-roundtrip` — measured stand-in: a second wrap of the same input gives
  `not ok … the equality is vacuous`.
- `runner-process.judge.owner-run-cart-pass` — measured stand-in: a no-op mutation gives
  `not ok … the mutated EXEC run: exit 2, expected 14` (and both follow-on findings), because the pinned pass
  report is then not met.
- The helper's conditional insertion (T-14-13) — measured: an `awk` that always inserts is refused by the
  helper's own count assertion, one line, exit 2.

## Threat surface scan

No new network endpoint, auth path, file access pattern or schema at a trust boundary: the plan forwards one
existing runner option (`--cart`) and adds one optional argument to an operator helper. The two new written
surfaces are the container (already named in `binary-ext.txt` and `.gitignore`, unchanged by this plan) and
the temporary scenario copy (already outside the work tree, and now counted).

## Threat register dispositions

| Threat ID | Disposition | Where it is mitigated and measured |
|---|---|---|
| T-14-12 | mitigate | six options asserted individually with their exact stderr line, exit 14, no report, no file; the audio trio given together |
| T-14-13 | mitigate | the helper's count assertion, measured by the always-insert mutant |
| T-14-14 | mitigate | the flipped-byte limb, measured by the same-input-twice mutant |
| T-14-15 | mitigate | `--cart` is an option; the lock has two roles; `bad-lock-line`'s `cart` example is unchanged and `ok`; nine `owner-run` ids unchanged |
| T-14-16 | mitigate | container and images live in `$tmp`/the scratch root; the scratch file listing is compared; `git ls-files` carries no `.ivsc`/`.cfg`; `exec: -` asserted |

## Known Stubs

None. Every value the three cases assert comes from a measured run, a tool's printed line or a file's own size;
there is no placeholder, no unwired value and no skipped case. The three new ids are `ok` in both presets.

## Deferred Issues

- The two `cart-role mutant` rows and the SCN-03 flip belong to plan 14-04's task 1, by design (deviation 2).
- Nothing else. The helper's `--cart` has no size rule by decision 2, and the core's two container refusals are
  cited rather than duplicated.

## Self-Check

- `tests/runner-process-tap.sh` — FOUND (three cases, the sixth argument, the conditional-insertion assertions)
- `tests/CMakeLists.txt` — FOUND (`$<TARGET_FILE:ivs_flatwrap>` as the sixth argument)
- `data/expected/tests/runner-process.ids` — FOUND (138 ids)
- `tools/owner-run.sh` — FOUND (`--cart`, `sh -n` and `dash -n` both clean)
- `docs/owner-images.md` — FOUND (`grep -c 'The route has no cartridge role'` is 0)
- `docs/RIGHTS.md` — FOUND (the flat-cartridge row names the derived container)
- `tools/check/binary-ext.txt`, `.gitignore` — unchanged since `2fcfd7d` (`git diff --name-only` empty)
- commit `b3beccb` — FOUND (ancestor of HEAD)
- commit `50024a5` — FOUND (ancestor of HEAD)
- commit `6837c8f` — FOUND (ancestor of HEAD)

**Self-Check: PASSED**
