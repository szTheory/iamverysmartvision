# Phase 19 Plan 01: Discriminating the Display Path Summary

Authored guests, parameter-driven branches and the display-latch profile row that make the
four rival display-path rules separable at the machine-state seam.

## Baselines

start-commit: 57fe0550248d0e814b1b5903b9ada185ae9bb83a

Measured at `start-commit` before the first edit of this plan:

| Count | Value | Read from |
|---|---|---|
| `stic` inventory data lines | 99 | `grep -v '^#' data/expected/tests/stic.ids \| awk 'NF' \| wc -l` |
| `profile-fields.ids` data lines | 154 | `grep -v '^#' data/expected/profile-fields.ids \| awk 'NF' \| wc -l` |
| `important-gates.ids` data lines | 372 | `grep -v '^#' data/expected/important-gates.ids \| awk 'NF' \| wc -l` |
| `important-gates.ids` distinct first columns | 122 | `grep -v '^#' data/expected/important-gates.ids \| awk 'NF{print $1}' \| sort -u \| wc -l` |
| `ivsn-literals.ids` data lines | 26 | `grep -v '^#' data/expected/ivsn-literals.ids \| awk 'NF' \| wc -l` |
| control directories (excluding `_good`) | 405 | `ls -d tools/check/controls/*/ \| grep -v _good \| wc -l` |
| gate steps | 38 | `ls tools/check/steps/*.sh \| wc -l` |
| profile `fields` members | 154 | `data/profiles/intv-2609-ntsc-family-p1.json` |
| rows tagged `unknown-default-chosen` | 47 | same file, `tag == "unknown-default-chosen"` |

Gate summary line measured at `start-commit` (sh only, before the first edit):

```
gate: PASS steps=38/38 controls=405/405 ... profile-keys=154 goldens=29 ... gates=122 lines=372 ... records=154 fields=154 unknown-fields=47 ... tests-expected=5082 tests-executed=5078 tests-skipped=4 tests-missing=0 ... stic-rules=67 stic-rows=64 ...
```

`stic` suite measured at `start-commit`, before the first edit (`IVS_TEST_OUT_DIR=<tmp>
ctest --preset dev -R '^stic$'`): `# suite=stic executed=99 skipped=0 failed=0`, 99 `ok` lines,
`1..99`. This is the "before" half of the byte-identity evidence for the three parameter
branches: the same suite, same 99 ids, same results after Task 2.

## Deviations

### Rule 3 - Blocking issue: G1's gating-coupling limb lands with the member, in Task 2's commit

The plan's Task 1 step (4) asks `stic.disc.g1-window` for a coupling limb "under a stack copy
with the gating member at 0", while Task 1 step (8) and Task 1's own verify require that no
parameter digest and no snapshot literal move in Task 1.

Those two are mutually unsatisfiable as written: the parameter digest is the SHA-256 of the
*generated row text* (`cmake/IvsSticTable.cmake` `st_body_digest(row_digest "${row_text}")`), so
a member the generator emits changes the digest and therefore all five digest-carrying literals
of `data/expected/ivsn-literals.ids`. Adding the member in Task 1 would move the digest in Task 1
and again in Task 2 — two retargets, with a red `state` suite between them, which is exactly what
lesson 1 forbids and what the plan's own Task 2 preamble forbids ("Splitting this into two commits
would move the digest twice and leave a red gate between them").

Resolution: the `access_gating_when_off` member, its generator emission, its range check, its
branch in `machine_bus.c` **and** `stic.disc.g1-window`'s gating-coupling limb all land in Task 2's
commit, which is the commit that already pays the single retarget. Task 1's case still records the
coupling it can measure on the tree as it stands — the criterion guest's displayed word takes both
values under the **window** field alone (limbs 1 and 2), which is the same statement the plan wants
("the displayed word is not a diagonal witness") and is what makes the blocked-write counter the
registered witness. Task 2 extends the same case with the two gating limbs. `tests/stic/stic_disc.c`
is in this plan's `files_modified`, so Task 2 touching it is inside the plan's enumeration.

### The §2.7 registry split (derived-accepted-set)

§2.7 requires every new check to ship a must-fail mechanism registered in
`data/expected/important-gates.ids` under its requirement, in the same commit as the check. That is
impossible in this plan: `tools/check/steps/important-gates.sh` **derives** its accepted set from
the requirement records' `Complete` ids and refuses a row naming a `Pending` requirement, and
VID-07 stays `Pending` until plan 19-03's second task flips it. The derived-accepted-set rule
therefore forces the split: all ten ids this plan registers (`stic.disc.g1-window`,
`stic.judge.disc-g1-dead`, `stic.disc.displayed-equivalence`, `stic.judge.disc-equivalence-dead`
in Task 1; `stic.disc.g2-gating`, `stic.disc.g3-read`, `stic.disc.g4-latch`,
`stic.judge.disc-g2-dead`, `stic.judge.disc-g3-dead`, `stic.judge.disc-g4-dead` in Task 3) land
without a registry row here, and their rows land in plan 19-03's Task 2 commit with the VID-07
flip. Phase 18's observation O4 is the precedent. Each id ships its liveness guard and its
committed stand-in in the same commit as the id, so only the registry row is deferred.
