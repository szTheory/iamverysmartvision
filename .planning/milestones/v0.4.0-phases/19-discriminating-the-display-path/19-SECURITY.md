---
phase: "19"
slug: "discriminating-the-display-path"
status: secured
# threats_open = count of OPEN threats at or above workflow.security_block_on severity (high).
threats_open: 0
asvs_level: 1
created: "2026-11-27"
audited: "2026-11-27T10:12Z"
auditor: "gsd-security-auditor (pi, subagent)"
---

# Phase 19 -- Security

> Per-phase security contract: threat register, accepted risks, and audit trail.

**Verdict: SECURED -- 13 register rows across three plans, 11 distinct threat ids, 11 closed, 0 open.**
`block_on: high`, `asvs_level: 1`. Both `critical` rows (`T-19-04`, `T-19-07`) and every `high` row
(`T-19-01`, `T-19-02`, `T-19-03`, `T-19-05`, `T-19-06`, `T-19-08`, `T-19-SC`) are closed by a command
run in this session against the committed tree, not by reading a SUMMARY. The two `medium` rows
(`T-19-09`, `T-19-10`) are closed the same way.

## Basis of this audit

This phase's threat surface is **evidence integrity, not attack surface**. It changes four engine
files (three parameter-driven branches whose default reproduces today's bytes, plus the latch field),
adds four authored guests, one 16-cell sweep id, one bounded `refs/`-only probe, six registry rows and
one exit record. It adds no core sink, no network, no install rule, no release membership, no
dependency, and no parser for input the project does not already own. The risks it carries are
therefore the inverted ones the three plans named: a guest whose observable cannot move (a dead guest
passing vacuously), a diagonal check that accepts a guest moving under two fields, a sweep cell that
mutates a global instead of a stack copy, a `W1` chain that drifts from the commit's file list, an exit
record improvised after the measurement, a probe that escapes `refs/` or becomes an exit criterion, and
a regression disguised as a parameterisation.

Evidence classes, used consistently below:

- **code** -- `file:line` where the guard, the refusal, the stack copy or the equality is emitted.
- **live** -- a command run in this session against the committed tree, the built suites, or a `mktemp`
  directory outside the repository. No tracked file was modified; every probe path was created under
  `refs/` inside a `mktemp` root and removed by its own `trap`.
- **suite** -- the committed id or committed stand-in that re-demonstrates the mechanism on every gate
  run, whose result file I re-read from a run taken now.
- **commit** -- a `git show`/`git diff` measurement of the commit that carries the chain, against its own
  parent or against the plan's start commit.

Final state measurement, taken on the tree this record is committed to, before this file existed:

```
cmake --build --preset dev            -> rc 0
ctest --preset dev -R '^stic$'        -> rc 0   # suite=stic executed=111 skipped=0 failed=0
ctest --preset dev -R '^state$'       -> rc 0   # suite=state executed=61 skipped=0 failed=0
ctest --preset dev -R '^machine$'     -> rc 0   # suite=machine executed=415 skipped=0 failed=0
ctest --preset dev -R '^runner-process$' -> rc 0  # executed=164 skipped=3 failed=0
sh tools/gate.sh                      -> rc 0, `grep -c '^-- FAIL '` = 0
```

The gate's own line, with the frozen invariants recomputed from the files that produce them rather than
quoted:

```
steps=38/38 controls=405/405 claims=29 provisional=28 handoffs=8 discrepancies=110 faults=238/238
goldens=29 gates=123 lines=378 prefix-files=11 records=155 fields=155 unknown-fields=48
tests-expected=5114 tests-executed=5108 tests-skipped=6 tests-missing=0 stic-rules=67
```

Cross-checked directly, each read from the file that produces it:

```
ls data/golden | wc -l                                            -> 29
grep -v '^#' data/expected/pkg-install-files.ids | awk NF | wc -l  -> 11
python3 -c 'json claims'                                          -> 29 claims, 28 provisional
grep -v '^#' data/expected/handoffs.ids | awk NF | wc -l           -> 8
grep -v '^#' data/expected/important-gates.ids | awk NF | wc -l    -> 378 data lines
grep -v '^#' data/expected/important-gates.ids | awk NF{print $1} | sort -u | wc -l -> 123 gate names
```

Every one matches the line, so no figure below is a quotation.

**The container gate (`tools/gate-linux.sh`) was not run and is not claimed.** Measured now:
`timeout 20 docker info` -> rc 124 (no answer), and `build/gate-linux.log` is 0 bytes, so no `gate: PASS`
line exists to read. No `ivs-*` image, container or volume was created. Recorded as **outstanding
evidence, never a pass** (lesson 24). No threat below depends on it: every mechanism is a POSIX shell
test, a C comparison, a CMake emission or a TAP case, all of which behave identically on the Linux image,
and every id cited below ran under the local gate on this host.

After this record was staged, the gate was run again on the tree that carries it: `sh tools/gate.sh`
-> rc 0, zero `-- FAIL` lines, with `files=2854 tracked=2854 hygiene-files=2854` against 2853
`git ls-files` entries plus the phase's untracked `19-VERIFICATION.md`, which the gate's file list counts
and this commit deliberately does not. `scanned=705` did not move, because `.planning/` is outside the
source scan. Those three are the only moved tokens, they count the file list, and the file they gained is
this record -- so every invariant above still reads its frozen value on that run too.

## Trust Boundaries

Union of the three plans' boundaries; no boundary is added by the fix pass.

| Boundary | Description | Data crossing |
|---|---|---|
| profile document -> generated parameter table | a profile row is untrusted input to the generator and to every run; a wrong default silently changes every frame | policy values, tags, cites |
| test-authored parameter row -> the engine | a stack copy of `ivs_stic_params` is test data the engine trusts completely; a cell that edits a global makes every later comparison meaningless | four 0/1 or tick-valued fields |
| guest program -> the observable | the observable is the only evidence the phase has; a guest whose observable cannot move asserts nothing | machine-state words (`blocked_writes`, `windows_active`, latch) |
| operator content -> the work tree | the probe's input is the owner's own images; anything derived from them landing in a tracked file is a disclosure defect | EXEC/GROM images (never read here) |
| probe output -> the work tree | the output path is attacker-adjacent: a `..` or symlink spelling can put it anywhere | offset/outcome/displayed table |
| probe -> the phase's exit criteria | a probe that becomes an exit criterion converts an unavailable human act into a blocked phase | -- |
| the measurement -> the decision | the result is known only after the apparatus runs, so an enumeration written afterwards cannot be told from a fit | exit enumerations |
| the record -> the reader | a claim, tier or golden that moves with an observation converts an observation into evidence it is not | claims, goldens, tags |

## Threat Register

| Threat ID | Category | Component | Severity | Disposition | Mitigation (as built) | Status |
|-----------|----------|-----------|----------|-------------|-----------------------|--------|
| T-19-01 | Repudiation | `tests/stic/stic_disc.c`, the four guest cases, the equivalence case | high | mitigate | two-limb guards + five committed stand-ins + six judges that pass only when the guard fails | **closed** |
| T-19-02 | Tampering | `data/expected/ivsn-literals.ids`, `data/profiles/*.json`, `data/discrepancies.json`, the W1 chain | high | mitigate | field + inventory + digest + all five literals + C82 evidence in one commit, with the six size literals and `discrepancies.ids` asserted unmoved | **closed** |
| T-19-03 | Information disclosure | `tools/disc-sweep.sh` output path and content | high | mitigate | physical-parent `refs/`-only refusal (flatwrap rule) + table of offsets/outcome/displayed only + three gate cases | **closed** |
| T-19-04 | Repudiation | `stic.disc.sweep-diagonal`, `stic.judge.disc-diagonal-two-fields` | critical | mitigate | one comparison over 16 stack-copied cells, `alternate iff own bit`, per-cell diagnostic; committed entangled stand-in rejected as `FOREIGN` | **closed** |
| T-19-05 | Repudiation | `data/claims.json`, `data/golden/`, the profile tag, `pkg-install-files.ids` | high | mitigate | every invariant recomputed from its producing file; tag asserted; golden count exact; zero claims cite the observation | **closed** |
| T-19-06 | Tampering | the cell builder in `tests/stic/stic_disc.c` | high | mitigate | `*out = *p` stack copy, local `ivs_stic_params q`, no write through the profile index or a global | **closed** |
| T-19-07 | Tampering | `stic_params.h`, `IvsSticTable.cmake`, the three branches | critical | mitigate | each member emitted from the profile row, value 1 = today's bytes, range-checked; no golden, rule statement or claim moved; C84 alias controls green | **closed** |
| T-19-08 | Repudiation | `19-DECISION.md` and the exit commit | high | mitigate | both enumerations in an earlier commit than the measurement; exit commit's file list equal to Task 1's blob in both directions; zero removed lines | **closed** |
| T-19-09 | Repudiation | `runner-scenarios.ids`, `tests/runner-process.ids` | medium | mitigate | skip declared in both inventories; `tests-inventory` refuses `tests-missing`/`tests-unexpected-skip` by name; gate prints exactly two probe skip lines | **closed** |
| T-19-10 | Denial of service | the probe's run count and wall clock | medium | mitigate | fixed six-offset list, bound 16 refused before the first run, `--wall-timeout-ms 20000` per run, elapsed re-check between runs | **closed** |
| T-19-SC | Tampering | npm/pip/cargo installs (all three plans) | high | mitigate | not applicable and asserted as such: no dependency, no package-manager name in any touched file, no manifest changed | **closed** |

*Status: open · closed · open -- below `block_on` threshold (non-blocking).*
*Severity: critical > high > medium > low -- only open threats at or above `block_on` count toward `threats_open`.*
*`T-19-SC` appears as a row in all three plans' registers; it is one threat id and is counted once.*

## Per-threat verification, with the command that closes it

### T-19-01 (high) -- a guest whose observable cannot move

*Mitigation as built.* Each guest case reads its registered observable on the shipped row and again on a
one-field reversal, failing unless both values occur; each of the five stand-ins is committed code
referenced by its own judge (`grep -c` = 2 for every one of `sd_prog_g1_dead`, `sd_prog_g2_dead`,
`sd_prog_g3_dead`, `sd_prog_g4_dead`, `sd_equiv_dead`); each judge returns `PASS` **only** when the guard
fails on the stand-in and reports `judge-vacuous` / `judge-wrong-reason` otherwise
(`tests/stic/stic_disc.c:401-421` for G1; the same shape at `:690`, `:832`, `:972`, `:565`, `:1236`).

*Command and result.*

```
ctest --preset dev -R '^stic$'   ->  # suite=stic executed=111 skipped=0 failed=0
grep 'stic\.\(disc\|judge\.disc\)' $IVS_TEST_OUT_DIR/stic.txt
  ok 100 stic.disc.g1-window          ok 101 stic.judge.disc-g1-dead
  ok 102 stic.disc.displayed-equivalence  ok 103 stic.judge.disc-equivalence-dead
  ok 104 stic.disc.g2-gating          ok 105 stic.disc.g3-read
  ok 106 stic.disc.g4-latch           ok 107 stic.judge.disc-g2-dead
  ok 108 stic.judge.disc-g3-dead      ok 109 stic.judge.disc-g4-dead
```

A green judge is itself the liveness proof: the judge's only `PASS` path is the guard returning a
failure on the stand-in, so a guard that could not move would be red here. The equivalence stand-in
feeds the guard two words from **two different instants** (`:481-515`, close tick then next frame start),
so the instant-equality -- not the value equality -- is what is exercised.

### T-19-02 (high) -- the W1 chain drifting from the commit

*Mitigation as built.* The profile row, its inventory line, the parameter digest, all five
digest-carrying snapshot literals and C82's `evidence` sentence land in one commit (`6cffdb4`).

*Command and result.* `git show --name-only 6cffdb4` lists the profile, `profile-fields.ids`,
`ivsn-literals.ids`, `discrepancies.json`, the three branch files, the generator, the range checks, the
reference-notes note and the `PROJECT.md` row. Against `6cffdb4~1`:

```
params-digest-sha256             moved=True     header-bytes              moved=False
stepped-snapshot-sha256          moved=True     trailer-bytes             moved=False
cut-snapshot-sha256.hp-frame-mid moved=True     sections-total-bytes      moved=False
cut-snapshot-sha256.hp-latch     moved=True     total-bytes               moved=False
cut-snapshot-sha256.hp-mob       moved=True     schema-digest-sha256      moved=False
                                               header-prefix-hex         moved=False
git show --name-only 6cffdb4 | grep -c discrepancies.ids  -> 0
git show --name-only 9b8c720 | grep -c discrepancies.ids  -> 0
```

The old value the retarget replaced is `efba85009a57e91ad21060742dadc34949b976829525bb5e18d6b0897af1425c`
(read from `git show 57fe055:data/expected/ivsn-literals.ids`), which is exactly the value the SUMMARY
records its reproduce-the-old-value control producing. The new value is held independently by a second
reader that rebuilds the preimage from the specification: `ctest -R '^state$'` -> `ok 7
state.literal.params-digest`. **Limitation stated honestly:** I did not re-run the generator at the start
commit (that needs an old-tree configure); the control is verified as *consistent with* the pinned old
value, and the current value is verified *live* by the second reader.

### T-19-03 (high) -- the probe escaping `refs/` or disclosing operator identity

*Mitigation as built.* The output's physical parent is resolved with a physical `cd -P`/`pwd -P` and
allowed only under `refs/`, with an unresolvable parent a refusal rather than a blind write
(`tools/disc-sweep.sh:76-84`); the table carries only `offset_cpu_cycles outcome displayed`.

*Command and result (live, in a `mktemp` root; nothing written into the work tree).*

```
CI=1 sh tools/disc-sweep.sh --out T/refs/x.tsv ...   -> rc 2, stderr "disc-sweep: refusing to run in CI ...", stdout 0 bytes
--out $PWD/build/aud-probe.tsv                     -> rc 2, "...is inside the work tree outside refs/"
--out $PWD/tools/../build/aud-probe.tsv            -> rc 2, "...is inside the work tree outside refs/"
--out T/link/aud-probe.tsv  (link -> $PWD/build)   -> rc 2, "...is inside the work tree outside refs/"
file-left-behind: none
--out T/refs/ok.tsv                                -> rc 0, header + 2 offset rows
grep -c -E '[0-9a-f]{64}' T/refs/ok.tsv            -> 0
```

The committed case re-demonstrates the same three spellings plus the positive limb and the no-digest
assertion on **every** gate run, on both presets: `ok 129 runner-process.disc-sweep.output-tree`.

### T-19-04 (critical) -- a diagonal that accepts a two-field guest

*Mitigation as built.* `sd_cell` builds each of `SD_CELLS 16u` cells as a stack copy
(`tests/stic/stic_disc.c:1022-1071`); `sd_diagonal` asserts one sentence -- alternate value if and only
if the cell sets the guest's own bit -- with a per-cell diagnostic naming reason/cell/bits/guest/observed
(`:1162-1202`); the committed stand-in re-reads G1 the entangled way and passes only on `SD_DIAG_FOREIGN`.

*Command and result.*

```
./build/dev/tests/ivs_tests_stic | grep -E 'sweep-diagonal|judge:'
  # sweep-diagonal: G1 field 0 mask 0xAAAA default 1 alternate 0
  # sweep-diagonal: G2 field 1 mask 0xCCCC default 1 alternate 0
  # sweep-diagonal: G3 field 2 mask 0xF0F0 default 0 alternate 1
  # sweep-diagonal: G4 field 3 mask 0xFF00 default 4 alternate 7
  # judge: the diagonal rejected the entangled guest at cell 2 (bits 0x2), observed 1
ok 110 stic.disc.sweep-diagonal
ok 111 stic.judge.disc-diagonal-two-fields
```

Each mask is exactly the cells with that guest's own bit set, so the matrix is diagonal in the strong
sense, and the stand-in's rejection at cell 2 (gating alone) is the committed proof that a guest moving
under a foreign field is refused rather than noted. No table of sixteen expected vectors exists, so the
comparison is one rule, not sixteen fits.

### T-19-05 (high) -- an observation promoted into evidence

*Mitigation as built / measured.* Every invariant recomputed from the file that produces it:

```
data/claims.json                          -> 29 claims, 28 provisional
grep -cE 'sweep-diagonal|disc-sweep|disc\.g[1-4]' data/claims.json -> 0
ls data/golden | wc -l                    -> 29
grep -v '^#' data/expected/pkg-install-files.ids | awk NF | wc -l -> 11
profile row stic.display_latch_cleared_each_frame: value 1, tag unknown-default-chosen, decision null
git log 57fe055..HEAD -- data/golden data/stic/rules.json data/claims.json data/expected/pkg-install-files.ids
  -> (empty)
```

`goldens` is asserted as an exact count by the gate (`goldens=29`), so a retarget that added or dropped a
file could not pass a lower bound. No claim cites the sweep or the probe.

### T-19-06 (high) -- a cell mutating a global instead of a stack copy

*Mitigation as built.* `sd_cell` does `*out = *p` and writes only through `out`
(`tests/stic/stic_disc.c:1034`); the sweep's only row is the local `ivs_stic_params q` (`:1163`), and
`c.stic` is `const ivs_stic_params *`.

*Command and result.*

```
grep -nE 'ivs_stic_params_get\([^)]*\)[[:space:]]*=' tests/stic/stic_disc.c   -> none
grep -n 'ivs_stic_params q' tests/stic/stic_disc.c                          -> 1163:      ivs_stic_params q;
grep -n 'sd_cell' tests/stic/stic_disc.c | head -3                          -> 1022 definition, 1167 call
```

`sd_cell` returns 0 when a field it must perturb is unreachable (`:1037`, `:1044`, `:1055`, `:1059`), so a
field that stops producing a parameter is a red case rather than a silent no-op. The four masks above are
order-independent evidence: each cell is rebuilt from the shipped row on every call.

### T-19-07 (critical) -- a regression disguised as a parameterisation

*Mitigation as built.* Both members are emitted from the profile row the generator already validates
(`cmake/IvsSticTable.cmake:375`, `:680`, `:767`) and range-checked in the machine suite
(`tests/machine/machine_stic_params.c:226-227`). Value 1 is today's path in all three branches:
`machine_bus.c:90` requires `access_gating_when_off == 0u` before opening the window;
`stic_events.c:59` guards the frame-start clear with `!= 0u`; `stic_regs.c:46-57` keeps the
register-index match for `ivs_stic_is_mode_select_addr` addresses and uses an exact-address match only
for the rest, so `$4021` still selects Color Stack mode and `$0040` stays inert. The old unconditional
READ early return is gone (`grep -F 'access == IVS_STIC_ACCESS_READ && ivs_stic_is_mode_select_addr'
core/src/stic/stic_regs.c` -> absent), which is what makes the `$0020` row reachable at all.

*Command and result -- the regression controls, each of which a wrong branch would turn red.*

```
ctest --preset dev -R '^machine$' -> # suite=machine executed=415 skipped=0 failed=0   (range checks run)
stic suite result file:
  stic.alias.read-4021-switches-mode            ok
  stic.alias.read-4021-returns-read-view-value  ok
  stic.alias.read-0040                          ok
  stic.enable.seq.strobe-outside-window-dropped ok
  stic.window.applies-when-display-off          ok
  stic.events.latch-cleared-at-frame-start      ok
gate line: goldens=29 stic-rules=67 claims=29 provisional=28   (none moved)
```

The three branches therefore reproduce the previous bytes at the shipped row: no golden moved, no rule
statement changed, no claim moved, and the six ids above - the five the plan names plus the latch id -
are green, which is what an inverted gate or an unconditional exact-address match would break.

### T-19-08 (high) -- the exit record improvised after the measurement

*Mitigation as built.* Both enumerations live in `19-DECISION.md`, added by `2118d71` **before** the
sweep ran, and the exit commit is compared against that blob -- never the working tree.

*Command and result.*

```
git log --diff-filter=A --format=%H -1 -- 19-DECISION.md      -> 2118d711...  (the enumeration commit)
git show 2118d71:19-DECISION.md | grep -c '^## Measured'      -> 0
git show 2118d71:19-DECISION.md | grep -cE 'Exit (A|B) taken' -> 0
# Exit B enumeration extracted from that blob vs 9b8c720's file list, both directions:
extra in commit:      (none)
missing from commit:  (none)
git diff 57fe055..HEAD -- 19-DECISION.md | grep -c '^-[^-]'   -> 0
sed -n 's/^Exit \([AB]\) taken.*/Exit \1 taken/p' 19-DECISION.md -> Exit B taken
```

The enumeration and the measurement are in different commits, in that order, and the earlier lines of the
record carry no removed line between the phase start and `HEAD` -- so a list completed after the result is
a stop rather than an append.

### T-19-09 (medium) -- the skip marker dropped from one inventory

*Mitigation as built.* The skip is declared in both inventories
(`data/expected/runner-scenarios.ids:57` = `disc-sweep-probe 13 missing-fixture deterministic skip-ok -`;
`data/expected/tests/runner-process.ids:154` = `runner-process.scenario.disc-sweep-probe skip-ok`), and
`tools/check/steps/tests-inventory.sh:161,163` refuses a missing id (`tests-missing`) and an undeclared
skip (`tests-unexpected-skip`) **by name**.

*Command and result.*

```
sh tools/gate.sh -> grep -cE '^-- SKIPPED (dev|release)/runner-process/runner-process\.scenario\.disc-sweep-probe: ' -> 2
  -- SKIPPED dev/runner-process/runner-process.scenario.disc-sweep-probe: ... (ended missing-fixture, exit 13, as expected)
  -- SKIPPED release/runner-process/runner-process.scenario.disc-sweep-probe: ... (same)
runner-process result file: ok 36 runner-process.scenario.disc-sweep-probe # SKIP ...
gate line: tests-expected=5114 tests-executed=5108 tests-skipped=6 tests-missing=0
```

Exactly one visible skip line per preset: the marker cannot be dropped from one file only, and the skip
is visible rather than a silent pass.

### T-19-10 (medium) -- the probe becoming unbounded or a phase blocker

*Mitigation as built.* A fixed six-offset list, `max_bound=16`, `wall_bound=300`, `run_wall_ms=20000`
(`tools/disc-sweep.sh:52-54`), the bound checked before the output exists (`:89`, `:94`), the runner's own
`--wall-timeout-ms` on every run (`:138`), and an elapsed-time re-check between runs (`:124-127`).

*Command and result (live).*

```
--max-runs 100000 --out T/refs/b.tsv -> rc 2, "the run count 100000 is above the bound 16", output-exists=no
--max-runs 2      --out T/refs/ok.tsv -> rc 0, header "2 of 6 offsets (run-count cap 2, bound 16)", 2 rows
absent images                          -> rc 77, "SKIPPED disc-sweep: no operator images (D11)"
```

The bound refuses before the first run, the cap actually caps (so the option is not inert), and an
inconclusive run is an outcome in the table rather than a failure of the phase.

### T-19-SC (high) -- a dependency introduced

*Mitigation as built / measured.* Not applicable, asserted as such:

```
for f in tools/disc-sweep.sh tests/stic/stic_disc.c core/src/stic/stic_regs.c \
         core/src/stic/stic_events.c core/src/machine/machine_bus.c cmake/IvsSticTable.cmake \
         data/scenarios/disc-sweep-probe.ivss; do
  grep -cE 'npm|pip |cargo|yarn|pnpm|package\.json|requirements\.txt' "$f"; done   -> 0 for every file
git log 57fe055..HEAD -- '*package.json' '*package-lock.json' 'requirements*.txt' 'Cargo.toml' \
    'Cargo.lock' 'pyproject.toml'                                                  -> (empty)
```

No package manager is named in any file this phase touches, no manifest moved, and `package-legitimacy`
has nothing to gate.

## Accepted Risks Log

No threat is accepted or transferred. The rows below are **open non-blocking debt carried from the review
and the verification** -- none is a threat mitigation gap, and none counts toward `threats_open`. Each is
recorded with the reason it is not one, so it is not re-litigated.

| Risk ID | Threat Ref | Rationale | Accepted By | Date |
|---------|------------|-----------|-------------|------|
| R-19-01 | none (IN-01) | Two file headers (`stic_events.c:15-16`, `machine_bus.c:20-31`) describe the changed events without naming the deciding parameter. Both branches carry an accurate inline comment at the decision, and `stic_params.h` is where the parameters are documented. No mitigation reads a header. Remedy belongs to the next commit that touches those files (§2.12 append-only). | gsd-security-auditor | 2026-11-27 |
| R-19-02 | none (IN-02) | `sd_hits.cycle[]` is written (`stic_disc.c:181`) and never read. A dead diagnostic field: it carries no observable, and every observable the phase registers is read from `ivs_machine_stic_state`. No behavioural effect. | gsd-security-auditor | 2026-11-27 |
| R-19-03 | T-19-07 (adjacent, not open) | The `else()` of the gating policy select (`cmake/IvsSticTable.cmake:378-383`) is unreachable, and `gating_apply` is directory-scoped, so a stale value could in principle leak across `foreach(st_profiles)` iterations. It cannot manifest here: the tree registers **one** profile (`ls data/profiles/*.json` = 1), `st_policy` is fail-closed (any value but `apply` is a fatal configure error), the emitted value is range-checked (`machine_stic_params.c:226`) and the byte-identity evidence above holds. A latent robustness item for a future multi-profile tree, not an open mitigation: T-19-07's mechanism is present and measured. | gsd-security-auditor | 2026-11-27 |
| R-19-04 | T-19-08 (adjacent, not open) | `19-DECISION.md` states the reading of Exit A's trigger only in the appended, post-measurement section (IN-05). T-19-08's mitigation is intact and verified above: the enumerations -- the part that could have been fitted -- were committed before the run, in a different commit, and the exit commit equals them. The reading changed no setting: the exit taken is B, no default was decided, and §2.6/D-54 give the same exit for a diagonal with no preference under either parse. Remedy is a `PLAN-LESSONS` entry owed to the next plan carrying an exit choice. | gsd-security-auditor | 2026-11-27 |
| R-19-05 | none (§2.7 split) | Ten case ids (plan 19-01) and two (plan 19-03) landed one task before their registry rows. `important-gates.sh` derives its accepted set from `Complete` requirement ids and refuses a row naming a `Pending` one, so the split was forced. Each id shipped its liveness guard and its committed stand-in in the same commit as the id; only the registry row moved, into the commit that flips VID-07. §2.7 is a guardrail, not a phase criterion, and the six rows are present (`important-gates.ids:702-707`, `gates=123 lines=378`). | gsd-security-auditor | 2026-11-27 |
| R-19-06 | T-19-01 (adjacent, not open) | G1's registered observable is the blocked-write counter rather than criterion 1's parenthetical `frame 0 displayed`, because that reading also moves when the gating field flips. The criterion's reading is still asserted in the same case with its coupling measured, and the entangled reading is committed as the stand-in the diagonal judge must reject. The re-derivation was recorded before the sweep, not after a result. | gsd-security-auditor | 2026-11-27 |
| R-19-07 | none (lesson 34) | Plan 19-03's Task 2 predicate (`... \(Phase 19 sweep[^.]*\.` **and** the id inside the match) is unsatisfiable, because `[^.]` cannot cross the dot in `stic.`. The executor ran an equivalent two-limb form; I re-measured the intent directly (one *new* dated sentence per rival row, `Observation, ` count up by exactly one in each of C1/C6/C82/C94, the sweep id inside it, `status: open`, policy prefix and resolver intact). A predicate defect, not a record defect; no note's content was changed to satisfy a scanner. | gsd-security-auditor | 2026-11-27 |

## Unregistered threat flags

None. No SUMMARY carries a `## Threat Flags` section (`grep -i threat 19-0*-SUMMARY.md` -> no match), and
every artifact the phase added -- four guests, five stand-ins, six judges, the sweep and its stand-in, the
probe and its scenario, the six registry rows -- maps to a declared threat id above. The probe's
`tools/disc-sweep.sh` is the only new executable, and its disclosure, path-containment and boundedness
limbs are exactly `T-19-03` and `T-19-10`.

## Outstanding evidence (recorded as not run, never as a pass)

| Item | State | Measured reason | What would discharge it |
|------|-------|-----------------|--------------------------|
| `sh tools/gate-linux.sh` (container gate, `GUARDRAILS-v0.4` §4) | **not run** | `timeout 20 docker info` -> rc 124 (no answer); `build/gate-linux.log` is 0 bytes, so no `gate: PASS` line exists to read. No `ivs-*` image, container or volume was created. | one container gate on a host whose daemon answers, ending `gate: PASS` in `build/gate-linux.log` |

No threat in this register depends on that item, and no id cited above is weakened by it: every one ran
under the local gate on this host.

## Security Audit Trail

| Audit Date | Threats Total | Closed | Open | Run By |
|------------|---------------|--------|------|--------|
| 2026-11-27 | 11 | 11 | 0 | gsd-security-auditor (pi, subagent), ASVS L1, `block_on: high` |

## Sign-Off

- [x] All threats have a disposition (`mitigate`; none accepted, none transferred)
- [x] Accepted risks documented in the Accepted Risks Log (none; non-blocking debt recorded with reasons)
- [x] `threats_open: 0` confirmed -- no open threat at or above `high`
- [x] `status: secured` set in frontmatter
- [x] No file under implementation was modified by this audit; no gate step, fault, golden, claim,
      installed file or handoff row was added (`steps=38/38`, `faults=238/238`, `goldens=29`, `claims=29`,
      `provisional=28`, `prefix-files=11`, `handoffs=8`)

**Approval:** verified 2026-11-27
