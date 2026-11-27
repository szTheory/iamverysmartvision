---
phase: "17"
slug: "records-cost-window-and-the-count-gate-contract"
status: secured
# threats_open = count of OPEN threats at or above workflow.security_block_on severity (high).
threats_open: 0
asvs_level: 1
created: "2026-11-26"
audited: "2026-11-26T21:51Z"
auditor: "gsd-security-auditor (pi, subagent)"
---

# Phase 17 -- Security

> Per-phase security contract: threat register, accepted risks, and audit trail.

**Verdict: SECURED -- 18 register rows across three plans, 16 distinct threat ids, 16 closed,
0 open.** `block_on: high`, `asvs_level: 1`. No threat at or above `high` is open; `T-17-SC` (the
only `high` row) is closed by measurement, not by assumption.

## Basis of this audit

This phase's realistic threat surface is **evidence integrity, not attack surface**. It adds no
network service, no parser, no file format, no user input path, no dependency, no installed file and
no rights-bearing artifact: one shell suite, two registries, two live documents, one lessons file,
one index and thirteen one-line control appends. The security property is therefore **inverted** --
the risk is a check that can pass vacuously (a digest compared to itself, a `grep` whose pattern
cannot match, a `diff` silent because both sides are empty, a count taken from a file other than the
one named), a test id that silently skips instead of failing, or a record that contradicts a frozen
archive. Every mitigation below was checked by **making the control fail**, or by showing the
refusal firing for its own reason, not by reading the plan.

Evidence classes, used consistently in the register:

- **code** -- `file:line` where the refusal or the equality is emitted.
- **live** -- a command run in this session against the committed tree, a `mktemp` copy of one input,
  or the committed `shim_join` function extracted verbatim (lines 1882-1902 of
  `tests/runner-process-tap.sh`) and driven from a scratch script. **No tracked file was modified.**
- **control** -- the committed stand-in that makes the refusal fire for its own reasons, confirmed by
  the gate's own `controls=404/404` on a run taken in this session.
- **review** -- a measurement `17-REVIEW.md` took that this audit did not repeat, cited only where
  repeating it would need a mutation this audit may not make. The two review fixes that mattered
  (CR-01, IN-01) were re-verified here rather than cited.

Final state measurement, taken before this file existed:

```
sh tools/gate.sh    -> exit 0, zero `^-- FAIL ` lines, gate: PASS (see the token line below)
dash tools/gate.sh  -> exit 0, zero `^-- FAIL ` lines, summary line `cmp`-identical to the sh run
```

The invariants the brief froze are unmoved on that run, read off the line rather than predicted:
`steps=38/38`, `faults=238/238`, `goldens=29`, `claims=29`, `provisional=28`, `prefix-files=11`,
`handoffs=8`; and `gates=120`, `lines=366`, `pkg-selftest=20/20`, `controls=404/404`,
`tests-expected=5044` (2522 inventory ids x 2 presets, recomputed: `cat data/expected/tests/*.ids |
grep -v '^#' | awk NF | wc -l` -> 2522), `tests-executed=5040`, `tests-skipped=4`, `tests-missing=0`.

The container gate (`tools/gate-linux.sh`) was **not** re-run by this audit; no threat in this
register depends on the platform, and the phase's own record of that run is not re-quoted here as
evidence (R-17-E).

After this record was staged, the gate was run again on the tree that carries it: green, zero
`^-- FAIL ` lines, with `files=2811 tracked=2811 hygiene-files=2811`. Those three are the only moved
tokens, they count `git ls-files`, and the one file they gained is this record -- a records-only
move, which is why every frozen invariant above still reads its frozen value on that run too.

## Trust Boundaries

Union of the three plans' boundaries. Nothing crosses a process, a host or a privilege line in this
phase; every boundary is between one record and the next reader of it.

| Boundary | Description | Data Crossing |
|----------|-------------|---------------|
| `.planning/ci-runs.tsv` -> the phase record | the ledger is append-only data; the record is prose derived from it, so a record that reads a different file or a different statistic states an unmeasured number as measured | three medians, two ranges, one row count |
| the phase record -> every later pricing sentence | Phases 18, 19 and 20 price against the allowance this window recomputes | one median triple, one allowance |
| the operator -> the ledger | a missing release row is an operator obligation; an executor that repairs the ledger destroys the only evidence the run happened | seven run ids |
| the wrap tool's output -> the shim and the runner | two processes read the same container; only a digest equality across them shows the second consumed what the first wrote | two 64-hex digests, one frame count |
| the token inventory -> the published format page | the page must not drift from the append-only inventory, and a reader must not confuse the tool's tokens with the parser's numbered reasons | eight tokens |
| a release -> its consumer | the packaging page and the release-decision line are the project's statement of what a release carries | one sentence, three places |
| the registry -> the requirement record | a row naming a `Pending` requirement is refused, and a `Complete` requirement with no row is refused | requirement ids, gate names |
| the test suite -> the gate's evidence | an id that skips instead of failing would leave the join unguarded while every count stayed green | one TAP result per preset |
| a plan's verify block -> the evidence it claims | a verdict decided by a pipeline tail, or a count taken from an unnamed file, can pass while measuring nothing | a verdict token, a count |
| a lesson -> a future planner | a rule with no mechanism is quoted rather than obeyed | three clauses |

---

## Threat Register

*18 register rows across the three plans resolve to 16 distinct ids: `T-17-SC` is carried by all
three plans (one shared threat, verified once), and `T-17-01`..`T-17-15` are each carried by exactly
one plan (17-01: 01-05; 17-02: 06-11; 17-03: 12-15).*

| Threat ID | Plan(s) | Category | Component | Severity | Disposition | Mitigation | Evidence | Status |
|-----------|---------|----------|-----------|----------|-------------|------------|----------|--------|
| T-17-01 | 17-01 | Repudiation | the window quoted instead of recomputed | medium | mitigate | the recipe is printed in the record beside the numbers and the verify recomputes the same triple from the same named file at the same commit, comparing whole lines; the `ci-log` step independently counts the same file | code `17-01-SUMMARY.md:113-132` (the recipe, `column 13 is cpu_s, column 8 is build_s, column 10 is gate_s` over the one named file); **live** re-ran the record's own recipe at HEAD: `window: cpu_s=572 build_s=49 gate_s=164`, `previous-window: cpu_s=367 build_s=34 gate_s=115`, `rows: 118 lines: 119`, seven `release-rows: <id>=1`, `release-build_s-range: 34 to 51`, `r-range: 10.5143 to 11.6735` -- all six lines present in the record as whole lines (`grep -c -x -F -f` -> 6); `git diff --quiet 21d023c -- .planning/ci-runs.tsv` -> exit 0, so the file at HEAD is the file at the record's `start-commit`; the independent count `sh tools/check/steps/ci-log.sh` -> `-- ci-log pass ci-runs=118`, the same 118 the record prints | closed |
| T-17-02 | 17-01 | Tampering | a count read from the wrong file or the wrong column | medium | mitigate | every figure names its file and its column (13, 8, 10), the verify rebuilds each figure from `.planning/ci-runs.tsv` and compares it as one whole line, and no verify step decides an outcome by a pipeline tail | code `17-01-SUMMARY.md:119` (the three columns named), `:123-125` (one `awk` per column, the statistic read into a file by `sed -n 3p`), `:64` (the coverage row naming column 8 and column 10 in words); the verdict shape is a whole-line count (`test "$(grep -c -x -F -f "$e" "$S")" -eq 6`), never a pipeline tail -- `V1`/`V3` in `17-VALIDATION.md`; **live** the six recomputed lines match as whole lines, and the counts come from `awk -F'\t' 'NR>1'` over the named file, not from a tail of a pipeline; `ci-log.sh:24` names the same file for its own count | closed |
| T-17-03 | 17-01 | Tampering | the executor re-appending a missing ledger row | medium | mitigate | a row count of 0 or 2 stops the task; the verify's `cmp` against seven lines of `1` fails rather than repairing, and no task in the plan writes to the ledger | code `tools/check/steps/ci-log.sh:52` (`ci-log-duplicate: line <n> run <id>` -- a doubled run id is a finding, so a re-append cannot pass the gate); committed control `tools/check/controls/ci-log-duplicate/` (`EXPECT` = `FAIL ci-log-duplicate:`, overlay = the real header plus two rows sharing run id 1001); **live** ran the step against that overlay in a `mktemp` root -> `-- FAIL ci-log-duplicate: line 3 run 1001`, exit 1; the real ledger: seven per-id counts each 1, `git log --oneline 21d023c..HEAD -- .planning/ci-runs.tsv` -> empty (no phase commit touched it) and `git diff --quiet 21d023c -- .planning/ci-runs.tsv` -> exit 0 | closed |
| T-17-04 | 17-01 | Repudiation | two checkpoint verdicts, or a verdict with no measurement behind it | low | mitigate | the verify counts `checkpoint:` lines exactly (must be 1) and requires the window line before the pricing sentence and the verdict after it; the token must be one of the three section 2.4 words | code `GUARDRAILS-v0.4.md:47-53` (the three-token vocabulary and the rule that the medians are recomputed and charged before the verdict); **live** `grep -c '^checkpoint: '` on `17-01-SUMMARY.md` -> 1, the line is `checkpoint: reprice` (line 221), it is one of the three words, and it sits after the window block (line 140) and after the `## Cost` pricing block (line 191); the recorded arithmetic (`cpu_s` 367 -> 572 against a priced addition of 0.11) is what selects `reprice` under that rule | closed |
| T-17-05 | 17-01 | Information disclosure | the record quoting a pattern it certifies absent | low | mitigate | `sh tools/hygiene.sh --tree` refuses a home-path literal, an absolute scratch prefix or an at-sign followed by a dotted word in any tracked file, this record included | code `tools/hygiene.sh:18` (`HOME_PATH`), `:37-41` (`HOME_NEW`: `/private/tmp`, `/private/var`, `/var/folders`, `/tmp/claude-`, `/root/`, tilde paths), `:44` (`EMAIL`), `:132` (`found email`), `:95-133` (the `--tree` limb over the file list); **live** three probes in a `mktemp` root, each refused with its own token: a home path -> `-- FAIL hygiene-home-path: docs/a.md`, a scratch prefix -> `-- FAIL hygiene-home-path: docs/b.md`, a dotted-word at-sign -> `-- FAIL hygiene-email: docs/c.md` (exit 1 each); and the real tree -> `-- hygiene pass mode=tree hygiene-files=2810`, exit 0, before this record was tracked, then `-- hygiene pass mode=tree hygiene-files=2811` with the record staged and a full gate re-run green at `files=2811 tracked=2811 hygiene-files=2811` -- the one-file move is this record and nothing else | closed |
| T-17-06 | 17-02 | Repudiation | a test id that silently skips instead of failing | medium | mitigate | each missing argument is `not ok` with its own reason, the case never returns a skip, and the verify requires `tests-skipped=4` unchanged, `tests-missing=0`, and an `ok` line for the id in **both** presets' TAP files | code `tests/runner-process-tap.sh:1904`, `:1906`, `:1908` (three separate `why flatwrap-shim '<nth> argument ... is missing: the join cannot be run'` branches, each setting `ok=0`, so an absent argument is `not ok` with its own reason and never a skip); `tools/check/steps/tests-inventory.sh:161` (`tests-missing` per missing id per preset), `:163` (`tests-unexpected-skip`), `:197` (the `tests-missing=0` pass line); **live** on this session's gate run: `ok 113 runner-process.judge.flatwrap-shim` in `build/gate/dev/runner-process.txt` and in `build/gate/release/runner-process.txt` (both presets), zero lines mentioning the id with `SKIP`, and the line reads `tests-skipped=4 tests-missing=0`; the id is line 242 of `data/expected/tests/runner-process.ids`, so its absence would move `tests-expected` and its skip would be `tests-unexpected-skip` | closed |
| T-17-07 | 17-02 | Repudiation | a join that passes vacuously | medium | mitigate | five limbs, not one: two equalities across processes, the tool's own printed digest, a one-byte flip that must move **both** values, and a committed stand-in that replays the first container and is refused by the same `shim_join` on every gate run | code `tests/runner-process-tap.sh:1863-1866` (the five limbs declared), `:1882-1902` (`shim_join`: `join-frames`, `join-images`, `join-cart-digest`, each its own finding), `:1931`/`:1947` (limbs 1-3, container A then B), `:1952-1958` (limb 4, `join-immobile`), `:1966-1969` (the committed stand-in built from the first report's lines, with the IN-01-reachable `! grep` guard), `:1971-1979` (limb 5: acceptance -> `judge-vacuous`, refusal without `join-images` -> `judge-wrong-reason`); **live** extracted `shim_join` verbatim (lines 1882-1902) into a scratch script and drove it: the replayed-container stand-in against the second container's report -> exit 1 with exactly `join-images: the harness digested 'aaaa1111' and the runner 'bbbb2222': the two processes did not digest the same container`, and a matched pair with the same tool digest -> exit 0, so the refusal is not unconditional; the case itself is `ok` in both presets on this session's gate run, which is limb 5 being re-demonstrated | closed |
| T-17-08 | 17-02 | Tampering | a document edit that contradicts a frozen archive | medium | mitigate | section (i) is amended in place with the superseded sentence quoted and dated, `### (i.1)` is proved byte-identical to HEAD by `cmp`, and no `.planning/milestones/` record is touched at all | code `.planning/ops/RELEASE-OPS.md` section (i): the canonical sentence, then "The line above **replaces** the v0.3.0 wording, which was superseded on 2026-11-26 by plan 17-02 (EV-08)", then the superseded sentence quoted beneath it with its date; **live** `git show 21d023c:.planning/ops/RELEASE-OPS.md | sed -n '/^### (i\.1)/,$p'` vs the same slice at HEAD -> `cmp` silent (111 lines each, byte-identical); `git diff --name-only 21d023c..HEAD -- .planning/milestones/` -> empty, so no frozen archive was touched; CR-01's fix (`aee972a`) is what makes the pointer name the superseded line rather than the replacement | closed |
| T-17-09 | 17-02 | Repudiation | a doc check satisfied by an empty grep | medium | mitigate | both greps must yield exactly eight lines before the `diff` is allowed to be silent, and a stand-in that renames one existing line-initial token in a copy must make the same `diff` non-silent | code `17-VALIDATION.md` `V4` (the two `grep -c .` assertions before `diff`) and `17-02-PLAN.md`'s verify block; the inventory->case link is gate-enforced: `tests/pack-tap.sh:19-21` mints `pack.flatwrap.neg.<token>` by a loop over the eight pinned tokens, so a token added without a case is a finding, and `:369` `pack.judge.flatwrap-tokens` judges the pin against the literals the source can print; **live** `grep '^flatwrap-' data/expected/flat-refusals.ids` -> 8 lines, the same on `docs/formats/ivsc-v1.md` -> 8 lines, `diff` silent; stand-in: renaming `flatwrap-base` to `flatwrap-baseX` in a copy of the doc slice -> `4c4` with exit 1, so the same `diff` is non-silent. Liveness of my own stand-in: the `sed` target matched exactly 1 line before substitution (my first attempt used a pattern that matched 0 lines and was inert -- the failure mode this threat names, caught by the count) | closed |
| T-17-10 | 17-02 | Tampering | the registry row and the acceptance flip drifting apart | medium | mitigate | one commit carries the case, the inventory line, the row, the phase-table entry, the flip and the thirteen toy lines; the step's two findings and the control runner's toy-drift check are the enforcement, and both pinned `EXPECT` files are asserted unchanged | code `tools/check/steps/important-gates.sh:227` (`important-gates-unknown-id` for a row that does not resolve), `:230` (the known-set membership test), and the `complete-uncovered` / `excluded-stale` findings at `:236-239`; **live** `git show --name-only be69a5e` -> the suite, `data/expected/tests/runner-process.ids`, `data/expected/important-gates.ids`, `.planning/REQUIREMENTS.md` (the phase-table entry and the EV-08 flip) and 13 `tools/check/controls/` files; `git show --numstat be69a5e` over those 13 -> every one `1 0` (a pure last-line append, zero deletions, no `DEVIATION`); the three `EXPECT` files (`important-gates-bad-req-phase5`, `-phase6`, `important-gates-unknown-id`) are byte-identical to `21d023c` (`git diff --quiet` exit 0), and the two pinned lines still land on the same rows (`phase5:19` = `toy-input fault GOOD_FAULT INP-04`, `phase6:24` = `toy-video fault GOOD_FAULT VID-99`, both `cmp`-equal to `21d023c`); this session's gate run reports `controls=404/404` (toy-drift and deviation-unused among them) and `-- important-gates pass gates=120 lines=366` | closed |
| T-17-11 | 17-02 | Tampering | a case writing derived material into the work tree | low | mitigate | both content directories, the flat input, the scenario and every report live under the suite's `mktemp` directory; the second wrap writes a new path because an existing one is refused by token; `.ivsc` is refused in the tree by the binary-extension list | code `tests/runner-process-tap.sh:1890-1892` (`$tmp/fwshim/{a,b,fakes}`), `:1893` (`$tmp/fwshim-flat.bin`), `:1895` (`$tmp/fwshim/flatwrap-shim.ivss`), `:1921`/`:1938` (the two wraps write `$fwa`/`$fwb`, never the tree), `:1923`/`:1928`/`:1940`/`:1944` (the four runs, each reporting under `$tmp/runs/`); `tools/check/binary-ext.txt:20` (`ivsc`), `.gitignore:16` (`*.ivsc`); the token `flatwrap-output-exists` is in `data/expected/flat-refusals.ids:23` and has a committed case at `tests/pack-tap.sh:344-349` (a pre-existing output path is refused, so a second wrap cannot overwrite); **live** `git ls-files | grep -c '\.ivsc$'` -> 0; after two full gate runs in this session `git status --short` reports only the pre-existing `.planning/state.json` modification, so the new case wrote nothing into the tree; `files-unlisted-binary` refuses a tracked binary-by-name file (committed control `files-unlisted-binary-so`, `EXPECT` = `FAIL files-unlisted-binary: docs/save.so`, `ok` inside this session's `controls=404/404`) | closed |
| T-17-12 | 17-03 | Repudiation | a lesson with no mechanism, quoted as if obeyed | medium | mitigate | one `mutant` row over an existing case carries clause 1; clause 3 cites the already-registered `st-pass-line` row and the step prints its own observed-refusal line; clause 2's lack of a checker is stated in the lesson rather than implied away | code `.planning/ops/PLAN-LESSONS.md:85-91` (lesson 32 and the three clause lines, each its own line at `:93`, `:94`, `:95`), `:97-112` (the clause-to-mechanism map: clause 1 -> the `count-gate` row, clause 3 -> the `d09-06-registration mutant st-pass-line EV-07` row, clause 2 -> "**no mechanism**, and the gap is stated rather than implied away"); `data/expected/important-gates.ids:646` (`count-gate mutant extra-file EV-08`), `:529` (`d09-06-registration mutant st-pass-line EV-07`, pre-existing, cited not duplicated); **live** `sh tools/check/steps/runner-selftest.sh` -> exit 0, and it prints its own observed refusal: `-- runner-selftest proof control-pass-line-not-last: profile (observed: -- FAIL control-pass-line-not-last: profile)`, `-- runner-selftest pass cases=14`; `tools/check/pass-line.sh` is the clause-3 mechanism, cited and not re-registered | closed |
| T-17-13 | 17-03 | Repudiation | a count-gate row that can pass vacuously | medium | mitigate | the row points at a case whose committed stand-in is refused by the same `check_inventory` the real prefix is judged by, so the demonstration is re-run on every gate and reported as `pkg-selftest=20/20` | code `data/expected/important-gates.ids:646` (the row) with its comment at `:629-645`; `tools/check/steps/pkg-install.sh:101-120` (`check_inventory`: reads the named file, `pkg-inventory-list` when it is missing or empty, `comm -23`/`comm -13` for the two directions), `:241` (the real prefix judged by it), `:663-668` (the committed `extra-file` stand-in: copy the prefix, plant `share/ivs/stray.txt`, require `-- FAIL pkg-inventory-extra: share/ivs/stray.txt` from the same function); **live** `sh tools/check/steps/pkg-install.sh` -> exit 0 with `-- pkg-install pass prefix-files=11 ... pkg-selftest=20/20`, and this session's gate line carries `pkg-selftest=20/20`, so the stand-in is re-demonstrated on every run rather than once | closed |
| T-17-14 | 17-03 | Tampering | a row naming a case the step does not run | low | mitigate | the verify asserts `extra-file` is in `pkg-selftests.ids` before the registry is run; the registry's own `important-gates-unknown-id` and the step's `pkg-selftest-missing` are the second and third checks | code `data/expected/pkg-selftests.ids:18` (`extra-file`); `tools/check/steps/important-gates.sh:192-196` (the known-set is built from `pkg-selftests.ids`, `ci-summary-selftests.ids` and `runner-selftest.ids`), `:230` (`important-gates-unknown-id` when the row's `mutant <id>` pair is not in it); **live** `grep -x 'extra-file' data/expected/pkg-selftests.ids` -> line 18; this session's gate run -> `-- important-gates pass gates=120 lines=366` with zero `^-- FAIL important-gates-` lines, so both new rows resolved; the unknown-id refusal is itself proved by the committed control `important-gates-unknown-id` (`EXPECT` = `FAIL important-gates-unknown-id: fault NO_SUCH_FAULT`, `ok` inside `controls=404/404`) | closed |
| T-17-15 | 17-03 | Tampering | a stale quoted figure left beside the corrected one | low | mitigate | `INDEX.md`'s lesson range is corrected in the same commit and the verify refuses a surviving `1-25` | code `.planning/ops/INDEX.md:20` (`Numbered planning lessons 1-32; cite them by number in plans`); **live** `grep -n 'Numbered planning lessons' .planning/ops/INDEX.md` -> `1-32`; `grep -c -F 'Numbered planning lessons 1-25' .planning/ops/INDEX.md` -> 0; `awk '/^[0-9]+\. / { last = $0 } END { print substr(last, 1, 3) }' .planning/ops/PLAN-LESSONS.md` -> `32.`, so the range and the file agree; the four invariants the lesson prints recompute from the tree and match the lesson's own line (`faults=238 goldens=29 claims=29 provisional=28`, this session's gate line agrees) | closed |
| T-17-SC | 17-01, 17-02, 17-03 | Tampering | npm/pip/cargo installs | high | mitigate | no plan installs anything: one ledger is read, one Markdown record is written, two documents, one shell suite, one inventory and one registry are edited, and the project's own gate runs. No package manager runs, so no `[ASSUMED]` or `[SUS]` package exists to gate | **live** `git diff --name-only 21d023c..HEAD` -> 30 paths, none of them a manifest or a lockfile (`package.json`, `package-lock.json`, `npm-shrinkwrap`, `yarn.lock`, `pnpm-lock`, `requirements*.txt`, `pyproject.toml`, `Pipfile`, `Cargo.toml`/`.lock`, `go.mod`/`.sum`, `Gemfile` all absent); a scan of every changed file for `npm|npx|pip|pip3|cargo|yarn|pnpm|gem|go get|apt-get|brew` followed by `install|add|i|ci|update|get` -> no match; `grep -rn '\[ASSUMED\]\|\[SUS\]'` over the changed Markdown -> no match; no `CMakeLists.txt`, no `third_party/` and no `find_package`/`FetchContent` path is in the diff, and the gate ran green with `steps=38/38 controls=404/404`, so no dependency entered the build | closed |

*Status: closed / open / open -- below `block_on` threshold (non-blocking). Severity order:
critical > high > medium > low; only open threats at or above `high` count toward `threats_open`.
No row is `open`.*

---

## The two code-review defects, re-verified as fixed

Both were fail-open shapes at exactly the level this phase exists to close, so each was re-run here
rather than taken from the review's disposition line.

| Defect | Fix commit | Re-measured this session |
|--------|-----------|--------------------------|
| **CR-01** -- `RELEASE-OPS.md` section (i) pointed at the line it had just installed as the *superseded* one, so the record read as if the dead v0.3.0 wording were current | `aee972a` | the section now reads "The line above **replaces** the v0.3.0 wording, which was superseded on 2026-11-26 by plan 17-02 (EV-08)" above the quoted superseded sentence; the direction of the pointer is what makes the amendment readable rather than self-contradicting, and `### (i.1)` is still `cmp`-identical to `21d023c` |
| **IN-01** -- the stand-in's "no matching line" guard was written `grep ... \|\| exit 1`, which aborted the whole suite before the `judge-vacuous` branch could run: a dead guard inside the new id | `7e1c1a2` | code `tests/runner-process-tap.sh:1966-1969`: the branch is now `if ! grep -E '^(frames\|digest[.])' ... ; then ok=0; why judge-vacuous '...'`, so the branch is the only handler and can fire; the case is `ok` in both presets on this session's gate run |

---

## Accepted Risks Log

No threat in the register is `accept` or `transfer`; every row is `mitigate` and closed. The rows
below are **non-register residuals** -- they carry no threat id, are all below the `high` block
threshold, and are recorded here so they are not re-litigated as fresh findings.

| Risk ID | Threat Ref | Severity | Rationale | Accepted By | Date |
|---------|------------|----------|-----------|-------------|------|
| R-17-A | `T-17-01`, `T-17-02` (enforcement cadence) | medium | The medians are re-derived by a **committed recipe** (`17-01-SUMMARY.md:113-132`) and by `V1` in `17-VALIDATION.md`, but no gate step re-derives them: `ci-log` re-counts the ledger every run (`ci-runs=118`, and it refuses a doubled run id), which catches row drift, not median drift. Adding a step is forbidden by `GUARDRAILS-v0.4` section 2.1 (`steps` stays 38), and the checkpoint rule at `:47-53` makes the recipe a phase-boundary obligation, so the enforcement is a cadence rather than a gate. The mitigation as declared is present and works; what is absent is automatic re-derivation between checkpoints. | owner-delegated autonomous operation (AGENTS.md); owner of the residual is the next phase-boundary checkpoint (18), which must re-run the recipe | 2026-11-26 |
| R-17-B | `T-17-09` (enforcement cadence) | medium | The doc-to-inventory `diff` is a **committed verify command** (`V4`), not a gate step, so a future edit to `docs/formats/ivsc-v1.md`'s token lines with the inventory left alone would not fail the gate. The inventory side is gate-enforced in both directions (`pack.flatwrap.neg.<token>` is minted from the pin, and `pack.judge.flatwrap-tokens` judges the pin against the literals the source can print), so the drift this leaves open is doc-only. Adding a step is forbidden by section 2.1; the mitigation the threat names (assert 8 lines on each side before the `diff` may be silent, and a stand-in that makes it non-silent) is present and was run here. | owner-delegated autonomous operation; owner is Phase 20's stale-text sweep and any future edit to the format page | 2026-11-26 |
| R-17-C | -- (`17-REVIEW.md` IN-02) | low | Lesson 32's clause-1 exemplar cites `pkg-install`'s `prefix-files=`, which is counted from the **observed** installed listing (`pkg-install.sh:242`, `grep -c . "$T/inv.have"`) and pinned to the named file by reconciliation (`:113-114`), a weaker shape than clause 1 as written. Nothing is mis-enforced today: the named file is read and its absence is `pkg-inventory-list`, and the project's direct-count shape does exist and was measured here (`ci-log.sh:24`, `:59`, `ci-runs=118` read inside the `awk` that validates the named record). The correction is prose in a live document plus its registry comment, which section 2.12 scopes as a pointer **and** an appended amendment, and it would make two completed phase records stale. | owner-delegated autonomous operation; owner is Phase 20's stale-text sweep, where the count-gate contract and its exemplar are recomputed | 2026-11-26 |
| R-17-D | -- (`17-REVIEW.md` IN-03) | low | EV-08 read `Complete` for exactly one commit (`be69a5e`) before its count-gate clause had an artifact (`bfa0815`). No mechanism can close a one-commit traceability gap retroactively, and the final state is correct: both plan records declare `requirements-completed: [EV-08]`, the `count-gate` row exists, and `-- important-gates pass gates=120 lines=366` with zero findings is the current measurement. The registry refuses a `Complete` requirement with no row and a row naming a `Pending` requirement, so a **persistent** drift is caught; the pre-existing `cart-role` row is why the one-commit window was green. | owner-delegated autonomous operation; no owner needed -- historical, recorded in `17-REVIEW.md` and here | 2026-11-26 |
| R-17-E | -- (auditor observation) | low | The Ubuntu container gate (`tools/gate-linux.sh`) was **not** re-run by this audit; the phase records `gate-linux: exit 0` and this file does not re-quote it as evidence. No threat in this register depends on the platform: every closed row is proved by a host-independent command (POSIX `sh`/`dash`, the committed steps, the committed `shim_join`), and the two shell runs taken here are `cmp`-identical. Re-running it costs ~4 minutes of hosted-equivalent time for no new evidence on any row. | owner-delegated autonomous operation; the next full-suite run before `/gsd:verify-work` covers it | 2026-11-26 |

*Accepted risks do not resurface in future audit runs.*

---

## Unregistered Flags

None. No `## Threat Flags` section exists in `17-01-SUMMARY.md`, `17-02-SUMMARY.md` or
`17-03-SUMMARY.md`, and this audit found no new attack surface that the register does not already
cover: the phase's whole diff is 30 paths, all of them records, one test suite, two inventories, two
live documents, one registry and thirteen one-line control appends. The two open review findings are
recorded above as R-17-C and R-17-D rather than as new threats, because neither is a mitigation gap
against a declared threat.

---

## Security Audit Trail

| Audit Date | Threats Total | Closed | Open | Run By |
|------------|---------------|--------|------|--------|
| 2026-11-26 | 16 (18 register rows) | 16 | 0 | gsd-security-auditor (pi), ASVS L1, `block_on: high` |

**What was executed, not inferred.** `sh tools/gate.sh` (exit 0, zero `^-- FAIL ` lines) and
`dash tools/gate.sh` (exit 0, summary line `cmp`-identical); the `ci-log`, `pkg-install`,
`runner-selftest`, `important-gates` and `tests-inventory` steps run directly against a
`git ls-files` FILELIST; the record's own window recipe re-run at HEAD and matched as six whole
lines; the `ci-log-duplicate` control overlay run in a `mktemp` root to make the duplicate refusal
fire; the committed `shim_join` extracted verbatim and driven twice (stand-in refused with
`join-images`, matched pair accepted); three hygiene probes (home path, scratch prefix, dotted-word
at-sign) each refused, then the real tree passing; the eight-token doc/inventory `diff` plus a
renaming stand-in that made it non-silent; the ledger proved unchanged since `21d023c` by
`git diff --quiet`; the `(i.1)` slice `cmp`-ed against `21d023c`; `git show --numstat be69a5e` over
the thirteen toy files (every one `1 0`); the three pinned `EXPECT` files `git diff --quiet`-ed
against `21d023c`; and the arithmetic over `.planning/ci-runs.tsv`, `data/expected/tests/*.ids` and
`data/expected/pkg-selftests.ids`.

**Nothing was written outside this file.** The tree was inspected read-only; every probe lived in a
`mktemp` directory and was removed with it, the extracted `shim_join` scratch script was deleted, and
`git status --short` at close reports only the pre-existing `.planning/state.json` modification,
which this commit does not stage (`.planning/config.json` is likewise untouched).

---

## Sign-Off

- [x] All threats have a disposition (`mitigate` x 16; no `accept` or `transfer` rows)
- [x] Accepted risks documented in the Accepted Risks Log (five non-register residuals, all below
      the `high` block threshold)
- [x] `threats_open: 0` confirmed -- no open threat at or above `high`, and no open threat at all
- [x] Each mitigation is shown working by a command or a `file:line`, not by the plan's claim
- [x] The phase's frozen invariants recompute unmoved on a run taken in this session:
      `steps=38`, `faults=238`, `goldens=29`, `claims=29`, `provisional=28`, `prefix-files=11`,
      `handoffs=8`
