# Phase 9: Registered Evidence and Gate Cost - Pattern Map

**Mapped:** 2026-11-24
**Files analyzed:** 26 (6 in wave A1, 16 in wave A2, 4 bookkeeping records)
**Analogs found:** 24 / 26 (2 have no analog: the exclusion ledger and the `.planning/` toy records have no
same-shape predecessor — their closest neighbours are named and their gaps stated)

Every path below is **git-tracked source**, verified with `git ls-files -- <path>` (non-empty) in this
session. No analog is a gitignored install/runtime mirror. Every `path:line` citation is a reading of the
file made this session, and the quoted text sits beside the citation so it is checkable.

---

## File Classification

| New/Modified File | Role | Data Flow | Closest Analog | Match Quality |
|---|---|---|---|---|
| `tests/runner-process-tap.sh` | test suite script | process-spawn / streaming | same file: `mutant_case` (`:522-563`), `self_control` (`:1539-1553`), `rt_refuse` (`:898-910`) | exact (in-file) |
| `data/expected/runner-scenarios.ids` | inventory data | config / batch | same file's column vocabulary (`:1-13`) | exact (in-file) |
| `data/expected/scenario-pairs.ids` | inventory data | config / batch | same file (`:1-12`) | exact (in-file) |
| `data/expected/tests/runner-process.ids` | test inventory | config / batch | same file's comment blocks (`:14-18`, `:100-103`) | exact (in-file) |
| `tests/state/state_restore.c` | C test case | batch loop | same file: `rs_run_family` (`:669-691`), `su_call_row` (`tests/state/state_util.c:432-493`) | exact (in-file) |
| `tests/state/state_fuzz.c` | C test case | batch loop | same file: `fz_run_flips` (`:915-935`) | exact (in-file) |
| `tools/check/steps/important-gates.sh` | gate step (pure checker) | batch / request-response | same file: the five inventory loops (`:53-70`) and the `nreq` awk (`:79-105`) | exact (in-file) |
| `data/expected/important-gates.ids` | registry data | config / batch | same file (`:1-14`, 396 lines, 306 non-comment) | exact (in-file) |
| `tools/check/controls/_good/tree/data/expected/important-gates.ids` | toy registry data | config / batch | same file (37 lines, 27 non-comment) | exact (in-file) |
| `tools/check/controls/important-gates-*/overlay/data/expected/important-gates.ids` (×9) | toy registry overlay | config / batch | `important-gates-bad-req-phase5/overlay/...` | exact |
| `data/expected/important-gates-excluded.ids` **(new)** | exclusion ledger data | config / batch | `data/expected/cp1610-flag-exempt.ids` + `data/expected/cp1610-skips.ids` | role-match (no `<REQ> <reason>` ledger exists) |
| `tools/check/run-controls.sh` | control runner | batch / process-spawn | same file: inventory consistency block (`:249-268`), `good_one` (`:156-166`) | exact (in-file) |
| `tools/check/steps/sources.sh` | gate step (pure checker) | batch / transform | same file (`:38-46`) | exact (in-file) |
| `tools/check/manifests.cmake` | checker module (cmake) | batch / transform | same file: `path_ok` (`:87-95`), `check_entries` (`:97-131`) | exact (in-file) |
| `tools/check/steps/memmap.sh` | gate step (producer) | process-spawn | same file: the `cmake -P` invocation (`:24-32`) | exact (in-file) |
| `cmake/IvsMemmapProbes.cmake` | generator script (cmake) | file-I/O / fail-closed | same file (`:1-19`) | exact (in-file) |
| `data/expected/controls/*.ids` | control-token inventory | config / batch | `data/expected/controls/gate-hardening.ids`, `.../important-gates.ids` | exact |
| `tools/check/controls/<new>/` (new control dirs) | control fixture | file-I/O | `glossary-no-files/`, `sources-no-files/`, `manifests-digest-mismatch/`, `memmap-literal-orphan/` | exact |
| `tools/check/controls/_good/tree/.planning/REQUIREMENTS.md` **(new)** | toy requirement record | config / batch | `_good/tree/.planning/PROJECT.md` (shape only) | role-match |
| `tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md` **(new)** | toy requirement record | config / batch | same as above | role-match |
| `data/expected/controls/run-controls.ids` **(new, or extend `gate-hardening.ids`)** | control-token inventory | config / batch | `data/expected/controls/gate-hardening.ids` | role-match |
| `tools/gate.sh` | gate driver | batch / orchestration | same file: step loop (`:104-124`), `step_time` (`:40-43`), `keep_fails` (`:37`) | exact (in-file) |
| `.planning/PROJECT.md` | decision record | prose | same file's D-register row shape (`:242`) | exact (in-file) |
| `.planning/ops/RELEASE-OPS.md` | ops record | prose | same file (`:11-13`, `:31`, `:65`) | exact (in-file) |
| `core/src/machine/machine_digest.c` | C source comment | prose | same file (`:1-20`) | exact (in-file) |
| `docs/runner.md` | operator doc | prose | same file's golden-command block (`:247-256`) | exact (in-file) |
| live record for the 7 draft `VALIDATION.md` dispositions | ledger record | prose table | `08-ACCEPTED-DEBT.md` §1 (`:11-23`) | role-match |

---

## Pattern Assignments

### Wave A1 — cost lever and trims

---

#### `tests/runner-process-tap.sh` (test suite script, process-spawn)

**Analog: same file.** Three patterns are copied, one per change.

**(1) The two-mode switch — copy `mutant_case` (`:522-563`), verbatim header + body:**

```sh
# mutant_case <id> <listing> <cart file name> <scenario id> <addr list> <to list> <exit> <outcome> <detail pattern> [stub] (plan 08-lever2, D-40):
# the arguments of run_mutant, and the one place that decides what a row of any mutant inventory costs. With the build configuration Release
# (the fifth argument of this script) it is run_mutant: the full run and the judgement of outcome, exit code and detail. In any other
# configuration (the sanitized Debug preset) it is the static judgement, which runs nothing: ...
mut_full=0
mut_static=0
mutant_case() {
  if [ "$config" = Release ]; then
    mut_full=$((mut_full + 1))
    run_mutant "$@"
    return 0
  fi
  mut_static=$((mut_static + 1))
  ran=0
```

`$config` is the script's optional fifth argument — `tests/runner-process-tap.sh:106`:

```sh
config=${5:-}
```

with the usage line at `:100`:

```sh
# Usage: sh runner-process-tap.sh <source root> <ivs-run> <ivs_pack> <images dir> [<build configuration>]
```

The invariant this buys is already stated at `:89-90`:

```
# ... the ids are the same in both presets, no id is skip-ok and tests-skipped does not move (D-40).
```

**(2) The scenario loop and its column vocabulary — `:437-461`:**

```sh
while read -r id x oc class skip args rest; do
  case $id in '' | '#'*) continue ;; esac
  ok=1
  : > "$tmp/why"
  if [ -n "$rest" ] || [ -z "$args" ]; then
    ok=0; why inventory "not six columns"
  fi
  case $class in deterministic | host-dependent) ;; *) ok=0; why inventory "class '$class'" ;; esac
  case $skip in run | skip-ok) ;; *) ok=0; why inventory "skip '$skip'" ;; esac
```

**`release-only` is a NEW column-5 value, not a reuse.** CONTEXT's "Reusable Assets" claims
`data/expected/runner-scenarios.ids` "already has the column-5 mode vocabulary (`run`, `release-only`)";
measured this session, `grep -rn 'release-only' tests/ data/ tools/check/steps/` returns **no match**. The
edit is to `:445` (`case $skip in run | skip-ok)`) plus the header prose at `:8-10`.

**(3) The id accounting — `result()` at `:419-436`.** A `release-only` scenario must emit an ordinary
`ok` line with **no** `# SKIP`:

```sh
result() {
  n=$((n + 1))
  if [ "$ok" -ne 1 ]; then
    printf 'not ok %s %s\n' "$n" "$1" >> "$tmp/results"
    ...
  elif [ -n "${2:-}" ]; then
    printf 'ok %s %s # SKIP %s\n' "$n" "$1" "$2" >> "$tmp/results"
    skipped=$((skipped + 1))
  else
    printf 'ok %s %s\n' "$n" "$1" >> "$tmp/results"
    executed=$((executed + 1))
  fi
}
```

**(4) Trim (a) — copy `rt_refuse` (`:898-910`), which is the whole block being trimmed:**

```sh
rt_refuse() {
  _rn=$1; _rm=$2; shift 2
  run_one "$tmp" "rt-refuse-$_rn" "$ivs_run" --content-dir "$content" "$@"
  [ "$rc" -eq 14 ] && [ "$hung" -eq 0 ] && [ ! -s "$tmp/runs/rt-refuse-$_rn.out" ] || { ok=0; why roundtrip-refuse "$_rn: exit $rc, hung $hung or a report"; }
  [ "$(sed -n 1p "$tmp/runs/rt-refuse-$_rn.err")" = "ivs-run: $_rm" ] || { ok=0; why roundtrip-refuse "$_rn: the first stderr line is not 'ivs-run: $_rm'"; }
  [ "$(sed -n 2p "$tmp/runs/rt-refuse-$_rn.err")" = 'usage: ivs-run [options] <scenario.ivss>' ] || { ok=0; why roundtrip-refuse "$_rn: the usage text does not follow"; }
  grep -q -F -e '--roundtrip-at-ticks <n>' "$tmp/runs/rt-refuse-$_rn.err" || { ok=0; why roundtrip-refuse "$_rn: the usage text does not name the option"; }
}
rt_refuse zero 'option value out of range' --roundtrip-at-ticks 0 "$scen/diag-boot-pass.ivss"
rt_refuse above-max 'option value out of range' --roundtrip-at-ticks 9223372036854775809 "$scen/diag-boot-pass.ivss"
rt_refuse no-value 'option without a value' "$scen/diag-boot-pass.ivss" --roundtrip-at-ticks
rt_refuse twice 'option given twice' --roundtrip-at-ticks 600 --roundtrip-at-ticks 700 "$scen/diag-boot-pass.ivss"
result runner-process.roundtrip.refuse-range
```

The trim keeps `rt_refuse zero …` live and replaces the other three with static judgements against the
option table; the `result` call at `:910` is unchanged, so the id's identity and its inventory line do not
move.

**(5) The derivation-equality judgement's stand-in vehicle — `self_control` (`:1539-1553`), verbatim:**

```sh
# self_control <slug> <token> <check...>: runs the check; ok only when it fails with <token>.
self_control() {
  _slug=$1; _tok=$2; shift 2
  : > "$tmp/why"
  if "$@"; then
    ok=0
    : > "$tmp/why"
    why judge-vacuous "the check accepted the fake runner of $_slug"
  elif grep -q -e "^$_tok: " "$tmp/why"; then
    ok=1
  else
    ok=0
    { printf 'judge-wrong-reason: the check failed, but without %s:\n' "$_tok"; cat "$tmp/why"; } > "$tmp/why.2"
    mv "$tmp/why.2" "$tmp/why"
  fi
  result "runner-process.judge.$_slug"
}
```

Its existing call sites at `:1555-1561` show the calling convention (slug, expected token, the check and
its arguments). Amendment A-03 forbids adding a new `runner-process.judge.*` id in A1, so this pattern is
used either by extending an existing `self_control` call (`body-differs`, `:1561`) or by running the
judgement once inside the variant case as an ordinary `ok` line.

**(6) The pair judge and its two-mode seam — `:914-940`:**

```sh
pairs="$root/data/expected/scenario-pairs.ids"
while read -r pa pb pdiff peq prest; do
  case $pa in '' | '#'*) continue ;; esac
  ...
  [ -z "$prest" ] || { ok=0; why inventory "more than four columns"; }
  ...
  if [ "$ok" -eq 1 ]; then
    check_pair "$pa" "$pb" "$tmp/runs/scen-$pa.out" "$tmp/runs/scen-$pb.out" "$pdl" "$pel" || ok=0
  fi
  result "runner-process.pair.$pa.$pb"
done < "$pairs"
```

`check_pair` (`:374-380`) refuses `pair-no-report` when either report is empty — on dev the shift report
will not exist, so the pair needs the same switch here (inventory-only judgement on dev).

**Conventions that must be replicated**

- SPDX header line 2, `#!/bin/sh` line 1; `LC_ALL=C; export LC_ALL` (`:104`).
- `ok=1` / `: > "$tmp/why"` / `why <token> "<detail>"` / `result <id>` per case — never a bare `exit`.
- Token vocabulary is free-form kebab (`why inventory`, `why pair-no-report`, `why roundtrip-refuse`).
- A cost decision is visible in the output: `# mutants: configuration=<c> full=<n> static=<m>` before the
  trailer (`:97`). A `release-only` count belongs in the same diagnostic line.
- Every mode switch names the plan and the decision in its docstring (`(plan 08-lever2, D-40)`).

**Hazards visible in this file**

- **Pitfall 3:** a `release-only` implemented as a skip moves `tests-skipped` 4 → 8. `tests-inventory.sh:163`
  refuses `tests-unexpected-skip` for any id not marked `skip-ok` in `data/expected/tests/runner-process.ids`;
  the two existing `skip-ok` ids are `:42` and `:70` and neither is a demo-lamp scenario.
- **Pitfall 4:** dropping the pair on dev yields `pair-no-report` (`:375`) or `tests-missing`
  (`tests-inventory.sh:161`).
- **Pitfall 1:** the derivation slice must drop fence lines and *leading* `#` lines from the derivation and
  drop everything up to and including `ivss 1` plus the `id`/`profile`/`content`/`capture` header lines from
  the scenario. Verified rule in 09-RESEARCH §1.5.
- **Pitfall 2 / A-04:** the variant's `id` line cannot equal the pass scenario's — `id demo-lamp-tone-shift`
  vs `id demo-lamp-tone-pass`.

---

#### `data/expected/runner-scenarios.ids` (inventory data, config)

**Analog: same file.** Header `:1-13`, verbatim (relevant columns):

```
# Runner scenarios (plan 04-20, D-20, D-21, D-23): `<id> <exit> <outcome> <class> <skip>
# <operator-args>`, six fixed columns, the first token the id. Each id names
# data/scenarios/<id>.ivss; suite runner-process (tests/runner-process-tap.sh) runs every line
# through the real ivs-run and requires the exit code and outcome given here.
#   class          deterministic, or host-dependent (wall time, signals): the golden step of plan
#                  04-21 excludes host-dependent reports from the dev versus release comparison
#                  and refuses goldens for them (D-21: the wall watchdog is never golden).
#   skip           run, or skip-ok: the scenario needs operator content the gate never has; it is
#                  judged like any other and a passing judgement is a visible skip.
#   operator-args  extra ivs-run words joined by `,`, or `-` for none.
```

The two lines lever (3) moves — `:37-38`:

```
demo-lamp-tone-shift 0 pass deterministic run -
demo-lamp-tone-swap 0 pass deterministic run -
```

**Conventions:** one line per scenario, six space-separated columns, `-` for an empty operator-arg list,
`#` for comments, no trailing whitespace. Column 5's documented vocabulary lives in the header (`:8-10`) and
in `tests/runner-process-tap.sh:445`; both must gain `release-only` in the same commit or the header becomes
a lie the step cannot detect.

**Hazards:** `tests-inventory.sh` does **not** read column 5 (it reads `data/expected/tests/*.ids`), so the
inventory cannot catch a mismatch — only the tap script's `case` at `:445` can. Changing column 5 also
changes nothing in `tests-expected`, which is the point and also the reason a typo would go unnoticed
except by the tap script.

---

#### `data/expected/scenario-pairs.ids` (inventory data, config)

**Analog: same file.** Grammar at `:1-6`, the shift pair at `:12`:

```
# Grammar: `<idA> <idB> differ=<k,..> equal=<k,..>`, four columns, either list may be `-` but not both. ...
# demo-lamp-tone-pass against demo-lamp-tone-shift: every event 2,000 ticks later in the same frames, so the delivered frames are
# byte-identical (D-06); the other digests are not related here (the input digest differs by construction).
demo-lamp-tone-pass demo-lamp-tone-shift differ=- equal=digest.video
```

**Conventions:** exactly four columns, enforced by `tests/runner-process-tap.sh:922`
(`[ -z "$prest" ] || { ok=0; why inventory "more than four columns"; }`). A pair line has no mode column.

**Recommendation the plan should adopt (09-RESEARCH §1.4):** do **not** add a mode column to this file.
Derive the pair's mode from the two scenario lines' column 5 in `$inv` — the loop already greps `$inv` at
`:930-932`:

```sh
    awk -v i="$_i" '$1 == i && $4 == "deterministic" { f = 1 } END { exit !f }' "$inv" \
      || { ok=0; why inventory "scenario $_i is not a deterministic line of data/expected/runner-scenarios.ids"; }
```

**Hazard:** a fifth column here breaks nine existing pair lines at once and the only signal is a
`why inventory "more than four columns"` finding with no line number.

---

#### `data/expected/tests/runner-process.ids` (test inventory, config)

**Analog: same file.** The comment block that names the affected ids — `:16-18`:

```
# Plan 07-07 (D-23, D-06, A-16): the four scenarios demo-lamp-tone-{pass,idle,shift,swap} are scenario cases;
#   runner-process.repeat/slices/cwd.demo-lamp-tone-pass repeat the pass scenario only (cost cap);
#   runner-process.pair.<idA>.<idB>: the metamorphic judge, one case per line of data/expected/scenario-pairs.ids (two reports of the
```

and the id lines at `:58-61`, `:137`.

**Conventions:** one id per line, `#` comment blocks introduced by `# Plan NN-NN (D-nn, ...):` naming the
deciding plan and decision; ids are dotted and lower-case (`runner-process.<family>.<slug>`); the second
token is `skip-ok` only where a skip is legitimate. **A1 changes comment lines only** — no id line moves,
because the id set must be equal on both presets (`tests-inventory.sh:161` `tests-missing`, `:163`
`tests-unexpected-skip`).

**Hazard:** `tests-expected` is ids × presets (`tests-inventory.sh:51-52`), so any id addition moves the
token by 2 on each preset pair; A1 must move it by 0.

---

#### `tests/state/state_restore.c` (C test case, batch loop)

**Analog: same file.** The case being trimmed — `:1584-1596`:

```c
static int rs_atomic_every_refusal(void)
{
  uint32_t bad = 0u;
  size_t i;
  uint32_t produced = 0u;
  for (i = 0u; i < RS_FAMILIES; i++) {
    bad += rs_run_family(&rs_families[i], 1, 1, &produced);
  }
  if (!rs_args(&produced, 1)) {
    bad++;
  }
  return bad == 0u ? IVS_TEST_PASS : IVS_TEST_FAIL;
}
```

The third argument `1` is `digests`; `rs_run_family` (`:669-691`) sets it:

```c
static uint32_t rs_run_family(const rs_fam *f, int poisoned, int digests, uint32_t *produced)
{
  ...
  su_run_init(&u, rs_snap, rs_snap_n, poisoned);
  u.digests = digests;
  bad = su_run_rows(&u, &t);
```

**Conventions that must be replicated**

- Case docstring is a `/* id (D-nn, ...): <the property> ... */` block immediately above the function —
  see `:1578-1583`, which also names the fault the id must fail (`RESTORE_MUTATES_BEFORE_VALIDATE`).
- The case registers in one table row (`:1767`):
  `{"state.restore.atomic-every-refusal", rs_atomic_every_refusal, NULL},`.
- `IVS_TEST_PASS` / `IVS_TEST_FAIL` only; a diagnostic via `ivs_test_diag` names the row and the field.
- A cost change is justified in the docstring with its plan and decision tag (`plan 08-15, cost stop rule`).

**Hazards visible**

- The whole-instance comparison at `tests/state/state_util.c:476` is **outside** the `u->digests` branch
  (`:482`), so it survives the trim by construction:
  ```c
  if (memcmp(x->storage, u->ref[w->tsel], IVS_MACHINE_BYTES) != 0) {
  ```
- The liveness guard `produced` is accumulated at `state_util.c:491` and must stay unconditional:
  ```c
  if (ok && w->reason >= 58u && w->reason <= 81u) {
    u->produced |= 1u << (w->reason - 58u);
  }
  ```
- The row index is in scope in `su_run_rows` (`:505`: `for (r = 0u; r < t->n; r++) {`) but is **not**
  passed to `su_call_row` (`:432`), so the edit is one extra parameter or a `u->row_i` field.

---

#### `tests/state/state_fuzz.c` (C test case, batch loop)

**Analog: same file.** The unsealed sweep — `:915-935`:

```c
static void fz_run_flips(fz_ctx *c, const fz_plan *p, int sealed)
{
  static const uint8_t masks[2] = {0xFFu, 0x01u};
  uint32_t k;
  for (k = 0u; k < (uint32_t)fz.n; k++) {
    uint32_t j;
    if (!fz_pick(p, k)) {
      continue;
    }
    for (j = 0u; j < 2u; j++) {
      if (sealed && j == 1u && k >= FZ_HDR && k < (uint32_t)fz.n - IVS_SNAP_TRAILER_BYTES && fz.pk_kind[k] == FZ_K_VALUE && fz.row_free[fz.pk_row[k]]) {
        continue;
      }
      ...
      fz_flip_case(c, k, masks[j], sealed, k + j, p->twice);
    }
  }
}
```

and the case's liveness guard — `:937-950`:

```c
static int fz_byte_flips(void)
{
  fz_ctx c;
  int ok;
  ST_CHECK(fz_get());
  fz_ctx_init(&c, "state.fuzz.byte-flips");
  fz_run_flips(&c, &fz_full, 0);
  ok = fz_finish(&c);
  if (c.accepted != 0u) {
    ivs_test_diag("%s: a flip with the trailer left as it was was accepted", c.name);
    ok = IVS_TEST_FAIL;
  }
  return ok;
}
```

`fz_full` is `{0u, 1u, 0}` (`:826`) — stride 1, no fault thinning.

**Conventions:** the trim adds a `if (!sealed && j == 1u) { continue; }` beside the two existing `sealed &&`
guards, leaving `k < fz.n`, `fz_pick` and `masks[]` untouched, so "the offset set unchanged" holds by
construction. The docstring above `fz_run_flips` (`:905-914`) already explains why the second value of a
free-row byte adds nothing; the trim's sentence belongs there.

**Hazards:** the "flip *moves* the digest" guard has **no named mechanism** in the tree
(`grep -n 'moves the digest' tests/state/state_fuzz.c` → nothing). The operational guard is the acceptance
refusal at `:945-948` quoted above. The plan should say that in one sentence rather than invent a guard
(09-RESEARCH §Q2 trim (d)).

---

### Wave A2 — the registry, the step, the controls

---

#### `tools/check/steps/important-gates.sh` (gate step, pure checker)

**Analog: same file.** This is the highest-value file in the phase; four separate patterns are copied.

**(1) The header / SPDX / purity note — `:1-4`, verbatim:**

```sh
#!/bin/sh
# SPDX-License-Identifier: MIT
# Step important-gates: the machine-checked form of RUN-04 (D-46, plan 04-24). Pure checker: reads
# only $ROOT/data/expected/, never builds or runs anything, so the control runner can drive it.
```

The purity sentence at `:4` must be amended once EV-04 reads `$ROOT/.planning/`. **Precedent for the
amendment exists in two shipped steps** — `tools/check/steps/ci-log.sh:3` and `:24`:

```sh
# Step ci-log: format check of the hosted run record .planning/ci-runs.tsv (PKG-04, D-44).
rec="$ROOT/.planning/ci-runs.tsv"
```

and `tools/check/steps/glossary.sh:64-66`:

```sh
proj="$ROOT/.planning/PROJECT.md"
...
    bad glossary-d2-unlinked ".planning/PROJECT.md"
```

**(2) The header's phase enumeration — `:9-13`, the defect this phase exists to remove:**

```
# named mechanism that is shown to fail for its intended reason elsewhere in the gate; this step
# checks that the registry names only mechanisms that exist and covers every Phase 4, Phase 5,
# Phase 6, Phase 7 and Phase 8 requirement (plan 05-12 added the Phase 5 set, plan 06-14 the Phase 6 set, plan 07-02
# the Phase 7 requirement SCN-01, plan 07-10 SCN-02 together with its first registry lines, plan 08-08 the
```

and `:17-30`, which enumerates the accepted `<REQ>` ids by phase — Phase 4 at `:18`, Phase 5 at `:19`,
Phase 6 at `:20`, Phase 8 at `:20-25`, Phase 7 at `:25-28`. The enumeration therefore covers Phases 4 to 8
only; FND-01..07, CPU-01..05 and MEM-01..05 are Phase 1/2/3 requirements and are absent from it.

**(3) The token prose — `:37-40`, verbatim:**

```
# Tokens: important-gates-unknown-id: line <n> (not 4 columns, unknown mechanism, bad gate name,
#   or a requirement outside the Phase 4, Phase 5, Phase 6, Phase 7 and Phase 8 sets) or <mechanism> <id> line <n> (the id
#   is in no inventory of its mechanism); important-gates-uncovered-gate: <REQ> (a Phase 4, Phase 5, Phase 6, Phase 7 or
#   Phase 8 requirement with no registry line; also the registry file missing).
```

**(4) The accepted set / must-be-covered set — `:84-85`, verbatim:**

```awk
    nreq = split("API-02 API-04 RUN-01 RUN-02 RUN-04 RUN-05 RUN-06 PKG-01 PKG-03 PKG-04 INP-01 INP-02 INP-03 API-03 AUD-01 AUD-02 VID-01 VID-02 VID-03 VID-04 VID-05 VID-06 SCN-01 SCN-02 STA-01 STA-02 PRF-01", reqs, " ")
    for (i = 1; i <= nreq; i++) accepted[reqs[i]] = 1
```

and the coverage refusal at `:102`:

```awk
    for (i = 1; i <= nreq; i++) if (!(reqs[i] in covered)) bad("important-gates-uncovered-gate", reqs[i])
```

**(5) The five-inventory resolution — `:53-70`, the exact loops any new mechanism must resolve against:**

```sh
# The three inventories, one `<mechanism> <id>` line each.
: > "$tmp/known"
for f in "$exp"/faults/*.ids; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } NF { print "fault", $1 }' "$f" >> "$tmp/known"
done
for f in "$exp"/controls/*.ids; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } NF { print "control", $1 }' "$f" >> "$tmp/known"
done
for f in "$exp/pkg-selftests.ids" "$exp/ci-summary-selftests.ids"; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } NF { print "mutant", $1 }' "$f" >> "$tmp/known"
done
for f in "$exp"/tests/*.ids; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } $1 ~ /^[a-z0-9-]+[.](judge|mutant)[.][^ ]+$/ { print "mutant", $1 }' "$f" >> "$tmp/known"
done
```

**(6) The finding/stats/output tail — `:107-111`, the shape every pure step ends with:**

```sh
if [ -s "$find" ]; then
  sort -u "$find" | while IFS= read -r l; do printf -- '-- FAIL %s: %s\n' "${l%% *}" "${l#* }"; done
  exit 1
fi
printf -- '-- important-gates pass %s\n' "$(cat "$tmp/stats")"
```

**Conventions that must be replicated**

- Env guards are the first executable lines: `: "${ROOT:?ROOT is not set}"` (`:43-45`), then
  `LC_ALL=C; export LC_ALL` (`:42`) above them.
- Findings accumulate as `<token> <detail>` lines in `$tmp/findings`, printed **sorted and unique** at the
  end; never `exit` mid-analysis.
- Stats are one `k=v` line, printed after the pass marker, and the pass line is
  `-- <step-name> pass <stats>`.
- Token names are `<step>-<kebab-word>`; a token with a variable part takes it after `: `.
- `<gate>` is `^[a-z0-9][a-z0-9-]*$` (`:93`), `<mechanism>` is exactly `fault|control|mutant`, `<REQ>` must be
  in `accepted[]`.
- `.planning/` reads are anchored at `$ROOT` and named in the finding detail
  (`glossary.sh:66`: `bad glossary-d2-unlinked ".planning/PROJECT.md"`).

**Hazards visible**

- **Pitfall 5:** a naive "every Complete requirement must have a line" derivation goes red on the pristine
  tree for `API-01 API-05 API-06 RUN-03 PKG-02 REL-01` (50 Complete, 27 registered, EV-02 adds 17 → 44).
  Amendment A-01's exclusion ledger is the resolution.
- **`:73` already emits `important-gates-uncovered-gate` with a free-text detail** when the registry file is
  missing — a new `uncovered` branch must not collide with that detail string.
- `accepted[]` is built from the same `reqs[]` array that drives the coverage check (`:85`, `:102`), so a
  derived set replaces `reqs[]` construction, not just the membership test.
- The step's `sort -u "$find"` at `:108` means a duplicate finding is invisible; a control that relies on
  two occurrences must use two *different* details.

---

#### `data/expected/important-gates.ids` (registry data, config)

**Analog: same file.** Header `:1-14` and one representative section, verbatim:

```
# Important-gate registry (plan 04-24, D-46, RUN-04), checked by the gate step important-gates.
# ...
# Columns: <gate> <fault|control|mutant> <id> <REQ>. A fault is judged by faults-verdict (must-fail
# vector, allowed reason prefixes, must-pass vector); a control by tools/check/run-controls.sh
# (EXPECT token); a mutant is an in-step selftest case (pkg-selftests.ids, ci-summary-selftests.ids)
# or a suite self-control `<suite>.judge.<slug>` / `<suite>.mutant.<slug>` that passes only when
# its check fails for the intended reason. The audit behind this list is in
# .planning/phases/04-headless-runner-and-installed-consumer/04-24-SUMMARY.md.
# Phase 5 (plan 05-12) extends the registry to INP-01 INP-02 INP-03 API-03 AUD-01 AUD-02 (section below the Phase 4 lines).

# API-02: absolute run bound, overshoot reported, slicing invariance (suite run, runner slicing),
# timebase growth of ivs_info, pinned stop reasons.
run-bound fault RUN_TARGET_UNCHECKED API-02
```

The file is 396 lines, 306 non-comment, and its last line is
`prf01-digest-first mutant bench.judge.refuse-has-no-timing PRF-01`. Measured: `grep -c 'FND-\|CPU-\|MEM-'
data/expected/important-gates.ids` → **0**, so all 24 rows are new.

**Conventions:** `<gate> <mechanism> <id> <REQ>`, four space-separated columns; a `# <REQ>: <the property in
prose>` block introduces each requirement's rows; a `# Phase N (plan NN-NN, D-nn): ...` block introduces each
plan's extension; blank lines separate sections.

**Hazards:** amendment A-08 — the 24 rows name 17 gate names that do not exist today, so the predicted delta
is `gates=90 → 107` and `lines=306 → 330`. A prediction of `gates=90` is a stop, not a re-bless.

---

#### The ten toy registries (toy registry data / overlay)

**Analogs: `tools/check/controls/_good/tree/data/expected/important-gates.ids` (37 lines, 27 non-comment)
and the nine `important-gates-*/overlay/data/expected/important-gates.ids`.**

`_good` header, verbatim (`:1-4`):

```
# Toy important-gate registry of the known-good control tree (plan 04-24, D-46): one line per
# Phase 4 requirement and, since plan 05-12, per Phase 5 requirement, resolving to the toy fault GOOD_FAULT (data/expected/faults/cpu.ids), the
# toy control toy-control (data/expected/controls/toy.ids) and the toy selftest toy-selftest
# (data/expected/pkg-selftests.ids).
```

`_good` last line: `toy-prf01 fault GOOD_FAULT PRF-01`. An overlay header, verbatim
(`important-gates-bad-req-phase5/overlay/...:1-2`):

```
# Toy registry of control important-gates-bad-req-phase5 (plan 05-12): the good toy registry plus a
# line whose requirement is a Phase 5 id outside the six accepted ones.
```

Measured line numbers the EXPECT tokens depend on:
`important-gates-bad-req-phase5/overlay/...:19` is `toy-input fault GOOD_FAULT INP-04`;
`important-gates-bad-req-phase6/overlay/...:24` is `toy-video fault GOOD_FAULT VID-99`.

**Conventions:** a whole-file overlay (no `MAKE` — `ls tools/check/controls/important-gates-*/MAKE` finds
nothing); each replaces the good tree's 4-line header with its own 2-line header and carries the good tree's
body; toy ids are `toy-<slug>`, toy mechanisms are `GOOD_FAULT`, `GOOD_FAULT2`, `GOOD_RUNNER`,
`toy-control`, `toy-selftest`; one toy line per accepted REQ per phase, each preceded by a
`# Phase N (plan NN-NN): ...` comment.

**Hazards**

- **Pitfall 6 (the sharpest one):** the 17 new toy lines must be **appended at the end** of all ten files.
  Inserting anywhere else shifts line 19 and line 24 and the two positional EXPECT tokens fire for the wrong
  reason (`control-token-missing`).
- `_good` must move or the whole controls run goes red: `run-controls.sh:158` requires the step to pass on
  the pristine good tree and print its pass line.
- The two `important-gates-bad-req-*` controls need **no** retargeting: `INP-04` and `VID-99` are outside both
  the old and the new accepted sets. Say so in the plan rather than omitting the clause.

---

#### `data/expected/important-gates-excluded.ids` (new) (exclusion ledger data, config)

**No exact analog.** Closest same-shape ledgers in `data/expected/`:

- `data/expected/cp1610-skips.ids` — the tightest shape precedent, verbatim in full:

```
# CP-1610 vectors that are expected skips: '<vector id> <discrepancy id>' per line (D-16, D-17).
#allow-empty: no expected skips (A-05)
```

- `data/expected/cp1610-flag-exempt.ids` — an exemption ledger whose *docstring carries the reason per
  group* and whose lines are bare tokens:

```
# Flag cases that cannot occur (D-16, plan 02-05): '<ROW>/flag-set:<F>' or '<ROW>/flag-clear:<F>'.
# Only impossible cases belong here: CLRC can never set C and SETC can never clear it (pdf 56).
CLRC/flag-set:C
```

- `data/expected/state-sweep-notes.ids` — a 3-column appended-note ledger with a long grammar docstring; its
  prose (`:5-8`) is the model for "why a line here is not an edit of the frozen file".

**Conventions to replicate (all four are load-bearing)**

1. `#` docstring first, naming the owning plan and the decision tag, then the grammar on its own line:
   `# ... : '<REQ> <reason>' per line ...`.
2. A comment per group explaining *why* each entry is there — `cp1610-flag-exempt.ids` puts the reason in the
   comment, not on the line. For a `<REQ> <reason>` file the reason is the second field, so the docstring must
   say what a reason may contain (words joined by `_` is the established style — see
   `data/expected/state-sweep-notes.ids:6`: "a colon and a reason (words joined by underscores, at least
   three)").
3. `#allow-empty: <why it may be empty>` when emptiness is legal. Here it is **not** legal (six rows), so the
   absence of an `#allow-empty` line is itself the contract.
4. One line per entry, no trailing whitespace, `LC_ALL=C` sort order.

**Hazard (unique to this file):** the step must refuse a Complete requirement that is **neither** registered
**nor** excluded, and must not silently accept an exclusion whose REQ is not Complete (otherwise the
exclusion set rots). The exclusion set must be a *different file* from the registry — that is the whole
property (A-01).

---

#### `tools/check/run-controls.sh` (control runner, batch / process-spawn)

**Analog: same file.** Three patterns are copied.

**(1) The inventory-consistency block — `:249-268`, verbatim (this is where the drift check belongs):**

```sh
# Control directories and inventory.
: > "$tmp/ctl.have"
for d in "$CONTROLS_DIR"/*/; do
  [ -d "$d" ] || continue
  n=${d%/}
  n=${n##*/}
  [ "$n" = _good ] && continue
  printf '%s\n' "$n" >> "$tmp/ctl.have"
done
sort -u "$tmp/ctl.have" -o "$tmp/ctl.have"
: > "$tmp/ctl.want"
for f in "$CONTROLS_INVENTORY_DIR"/*.ids; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } NF { print $1 }' "$f" >> "$tmp/ctl.want"
done
sort -u "$tmp/ctl.want" -o "$tmp/ctl.want"
if [ ! -s "$tmp/ctl.want" ]; then fail control-inventory-empty "inventory"; fi
if [ ! -s "$tmp/ctl.have" ]; then fail control-none "no control directories"; fi
for n in $(comm -13 "$tmp/ctl.want" "$tmp/ctl.have"); do fail control-not-in-inventory "$n"; done
for n in $(comm -23 "$tmp/ctl.want" "$tmp/ctl.have"); do fail control-inventory-orphan "$n"; done
```

**(2) The positive control — `good_one` (`:156-166`), verbatim (this is where the per-step pass-line guard
goes per A-02/O2):**

```sh
# Positive control for step $1 in work dir $2: prints its result lines, touches $2/ok on success.
good_one() {
  build_tree "$2" "" || { fail control-good-failed "$1"; return 0; }
  if run_step "$2" "$1" && grep -q -F -e "-- $1 pass" "$2/out"; then
    printf 'control ok   _good/%s\n' "$1"
    : > "$2/ok"
  else
    fail control-good-failed "$1"
    cat "$2/out"
  fi
}
```

**(3) The negative control's four gates — `ctl_one` (`:168-222`).** In order: `control-unknown-step`
(`:173`), `control-expect-blank` (`:184`), `control-exit-zero` (`:190`), then the one-pass awk token match
whose outcomes are `control-forbidden-token` (`:217`) and `control-token-missing` (`:218`).

**(4) Tree construction — `build_tree` (`:137-148`), the exact overlay + MAKE semantics:**

```sh
build_tree() {
  if [ -e "$1/tree" ]; then rm -rf "$1/tree"; fi
  cp -R "$CONTROLS_DIR/_good/tree" "$1/tree" || return 1
  if [ -n "$2" ]; then
    if [ -d "$2/overlay" ]; then cp -R "$2/overlay/." "$1/tree/" || return 1; fi
    if [ -f "$2/MAKE" ]; then apply_make "$1/tree" "$2/MAKE" || return 1; fi
  fi
  (cd "$1/tree" && find . -type f | sed 's|^\./||' | sort) > "$1/filelist"
  return 0
}
```

**(5) The step invocation contract — `run_step` (`:150-153`), verbatim:**

```sh
run_step() {
  ROOT="$1/tree" FILELIST="$1/filelist" CHECK_DIR="$CHECK_DIR" IVS_GATE_DIR="$1/tree/build/gate" \
    sh "$CHECK_DIR/steps/$2.sh" > "$1/out" 2>&1
}
```

**Conventions that must be replicated**

- Header `:1-16` documents `CHECK`/`EXPECT`/`FORBID`/`overlay/`/`MAKE`, the env vars and the final
  `controls=<ok>/<total>` line. A new control kind must be added to that header prose in the same commit.
- Findings use `fail <token> <detail>` (`:71`): `fail() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; bad=1; }`.
- The runner prints `controls=%s/%s` as its last line (`:317`) and exits non-zero when `bad` is set.
- `_good` is skipped by name when the control set is built (`:255`: `[ "$n" = _good ] && continue`).

**Hazards visible**

- **The drift check must compare sorted non-comment lines in both directions** (A-09): every overlay replaces
  the 4-line header by design, so a comment-inclusive comparison fires on all nine.
- **A `DEVIATION` file that cannot rot** needs both directions: any diff line not declared, and any
  declaration that matches no diff line (`control-deviation-unused`).
- **A control whose `CHECK` names `gate` recurses** — `tools/gate.sh` starts `run-controls.sh`. `CHECK` must
  name a file in `tools/check/steps/` (`:170`: `[ ! -f "$CHECK_DIR/steps/$_step.sh" ]`). Pitfall 7.
- **The control runner's own tokens cannot be proven by a control** — they are proven by
  `tools/check/steps/runner-selftest.sh` (`:1-14`), which builds a synthetic controls tree. New tokens
  `control-toy-drift` and `control-deviation-unused` belong there, not in a control directory.

---

#### `tools/check/steps/sources.sh` (gate step, pure checker)

**Analog: same file.** The extraction to change — `:38-46`, verbatim:

```sh
: > "$tmp/refs"
while IFS= read -r f; do
  grep -a -o -E 'SRC-[a-z0-9-]+' "$ROOT/$f" >> "$tmp/refs"
done < "$tmp/scan"
sort -u "$tmp/refs" | while IFS= read -r id; do
  grep -q -x -F -e "$id" "$tmp/ids" || printf -- '-- FAIL source-unknown-ref: %s\n' "$id"
done > "$tmp/unknown"
cat "$tmp/unknown"
[ -s "$tmp/unknown" ] && rc=1
```

and the zero-document refusal already landed — `:33-36`:

```sh
if ! grep -q '^data/' "$tmp/scan"; then
  printf -- '-- FAIL sources-no-files: data/\n'
  exit 1
fi
```

**Conventions:** the token prose is at `:7`; the resolution loop already uses `grep -q -x -F` (`:43`), so
**only the extraction regex at `:40` changes** for IN-01. `rc` accumulates (`:16`), the pass line is
re-printed from the cmake stats (`:49-51`).

**Hazard:** the fix `SRC-[A-Za-z0-9_.-]*[A-Za-z0-9]` must keep `-x -F` matching exact — a token that the new
regex produces but the inventory does not list must fail as `source-unknown-ref`, and the existing
`source-unknown-ref` token already has a control (`data/expected/controls/provenance.ids:6`), so EV-03 item 1
adds **no new token**.

---

#### `tools/check/manifests.cmake` (checker module, batch / transform)

**Analog: same file.** `path_ok` — `:87-95`, verbatim:

```cmake
# Safe repository-relative path: charset, no leading slash, no parent segment.
function(path_ok out p)
  if(p MATCHES "${PATH_RE}" AND NOT p MATCHES "^/" AND NOT p MATCHES "(^|/)[.][.](/|$)")
    set(${out} TRUE PARENT_SCOPE)
  else()
    set(${out} FALSE PARENT_SCOPE)
  endif()
endfunction()
```

`check_entries` — `:97-131`, with the call site that must change at `:124-127`:

```cmake
      path_ok(ok "${p}")
      if(NOT ok)
        gate_fail(manifest-format "${lbl}/${key}/path")
      endif()
```

and the `dirname` availability the anchoring needs — `:184-186`:

```cmake
  if(it STREQUAL "STRING" AND NOT id STREQUAL dirname)
    gate_fail(manifest-id-dir-mismatch "${dirname}")
  endif()
```

The three array call sites are `:240-242`:
```cmake
  check_entries("${lbl}" outputs "${doc}")
  check_entries("${lbl}" notices "${doc}")
```
(the `sources` one is inside the `recipe` object, `:224`).

**Conventions:** a rule is a `function` with a documented one-line comment above it; a refusal is
`gate_fail(<token> "<label>/<path>")`; the token prose lives in the step wrapper's header
(`tools/check/steps/manifests.sh` is an 8-line delegation, so the prose is in the `.cmake` file's own
header `:1-8`).

**Hazard (ordering, 09-RESEARCH §Q6 item 4):** the anchoring refusal must be raised **before** the digest
comparison, or a second token fires and the EXPECT is ambiguous. The `check_entries` pass already runs before
the digest block, so the new branch belongs inside `path_ok`'s caller at `:124-127`, not in a later pass.

---

#### `tools/check/steps/memmap.sh` + `cmake/IvsMemmapProbes.cmake` (step + generator)

**Analog: same file for both.** The `cmake -P` invocation shape to copy — `memmap.sh:24-32`, verbatim:

```sh
gate_dir=${IVS_GATE_DIR:-$ROOT/build/gate}
out=$(cmake -DROOT="$ROOT" -DFILELIST="$FILELIST" -DCHECK_DIR="$CHECK_DIR" -DGATE_DIR="$gate_dir" \
  -P "$CHECK_DIR/memmap.cmake" 2>&1) || rc=1
# The checker's own pass line is re-printed below, once, with the step's name.
printf '%s\n' "$out" | grep -v '^-- memmap pass '
stats=$(printf '%s\n' "$out" | sed -n 's/^-- memmap pass //p')
[ "$rc" -eq 0 ] || exit 1
[ -n "$stats" ] || exit 1
printf -- '-- memmap pass %s\n' "$stats"
```

The generator's own contract — `cmake/IvsMemmapProbes.cmake:4-19`, verbatim (excerpt):

```
# Usage: cmake -DROOT=<repo> -DOUT=<build>/tests/ivs_memmap_probes.inc -P cmake/IvsMemmapProbes.cmake
...
# Fail closed (message `IvsMemmapProbes: <file>[:<line>]: <reason>`): no literal file; a row that
# does not match the grammar of the literal header; LO above HI; a device outside the literal
# vocabulary; a width above 16; a mirror that is not a power of two; a view that does not tile
# 0000-FFFF exactly once.
```

and its fail helper — `:30-32`:

```cmake
function(mp_fail where msg)
  message(FATAL_ERROR "IvsMemmapProbes: ${where}: ${msg}")
endfunction()
```

**Conventions:** a new invocation follows `memmap.sh`'s `out=$(cmake -D… -P …) || rc=1` shape, writes `-DOUT`
into the step's own `mktemp -d`, strips the child's pass line, and maps failure to one new token in the
step's `# Tokens:` prose (`memmap.sh:12-16`). The step's header at `:8-9` says "writes nothing" — that
sentence must be amended to "writes only inside its own temporary directory" in the same commit.

**Hazards**

- **Pitfall 10:** the control's fixture must be a defect the *step* does not also refuse, or two `-- FAIL`
  lines appear and the EXPECT is ambiguous. `mirror=3` is refused by the generator ("not a power of two") —
  verify `memmap.cmake` does not also refuse it, or pick `LO > HI`.
- The step's `grep -v '^-- memmap pass '` at `:28` strips the child's pass line; a second child that prints
  its own pass line needs the same strip or the summary gains a stray line.
- `memmap` references `build/` (`:24`), so it is a **producer** step and the preflight partition is
  undisturbed (09-RESEARCH §7.2).

---

#### `data/expected/controls/*.ids` (control-token inventory, config)

**Analog: `data/expected/controls/gate-hardening.ids`, verbatim in full:**

```
# Negative controls for the D-23 gate hardening (plan 02-01).
glossary-no-files
sources-no-files
integrity-missing-spdx-c
integrity-non-ascii-c
```

and `data/expected/controls/important-gates.ids:1-2`:

```
# Negative controls for the important-gates step (plan 04-24, D-46, RUN-04): a registry id that no
# inventory of its mechanism lists, and a Phase 4 requirement without a registry line.
```

**Conventions:** one control-directory name per line; `#` docstring names the owning plan, the decision tag
and the token(s) the controls pin; a `# Plan NN-NN (...): ...` block introduces each plan's additions. The
inventory filename matches the step name for a single-step family (`glossary.ids`, `memmap.ids`,
`important-gates.ids`, `sources`'s tokens live in `provenance.ids`) and is free-form for a cross-step family
(`gate-hardening.ids`, `rights.ids`).

**Where the new tokens go**

| New token | Inventory file | Precedent |
|---|---|---|
| EV-03 item 1 (`source-unknown-ref`, reused) | no change — the control name goes in `data/expected/controls/provenance.ids` | `provenance.ids:6` already lists `sources-unknown-ref` |
| EV-03 item 4 (`manifest-format`, reused) | no change — control name in `data/expected/controls/rights.ids` | `rights.ids` (20 lines) already lists `manifests-*` |
| EV-03 item 5 (`memmap-generator-refused`, new) | `data/expected/controls/memmap.ids` | `memmap.ids:2-15` |
| EV-03 item 2 extensions | `data/expected/controls/gate-hardening.ids` | the file's own purpose |
| EV-04 (a) (`important-gates-complete-uncovered`, new) | `data/expected/controls/important-gates.ids` | `important-gates.ids:2-16` |
| EV-04 (b) (`control-toy-drift`, `control-deviation-unused`, new) | new `data/expected/controls/run-controls.ids` **or** `gate-hardening.ids` | neither exists yet; the drift token is a runner token, so `runner-selftest.sh` must also prove it |

**Hazard:** `run-controls.sh:267-268` refuses both a control directory absent from the inventory
(`control-not-in-inventory`) and an inventory line with no directory (`control-inventory-orphan`), so a new
control and its inventory line must land in one commit or the whole gate goes red.

---

#### New control directories under `tools/check/controls/` (control fixture, file-I/O)

Four representative shapes, verbatim. **A pure step control with a `MAKE` delete** —
`glossary-no-files/`:

```
CHECK:  glossary
EXPECT: FAIL glossary-no-files:
MAKE:   delete docs/evidence-tiers.md
        delete docs/time-base.md
        delete docs/cp1610/reference-notes.md
        delete docs/stic/reference-notes.md
```

`tools/check/controls/sources-no-files/` has the same three-file shape with 13 `delete` lines.
**A pure step control with a `MAKE` write** — `manifests-digest-mismatch/`:

```
CHECK:  manifests
EXPECT: FAIL manifest-digest-mismatch: FX-good
MAKE:   write fixtures/FX-good/payload.txt changed
```

**A control with EXPECT + FORBID** — `memmap-literal-orphan/`:

```
CHECK:  memmap
EXPECT: FAIL memmap-literal-orphan:
FORBID: memmap-literal-missing
MAKE:   write data/memmap/other-profile.literal.txt x
```

**A whole-file overlay control** — `important-gates-bad-req-phase5/`:

```
CHECK:                  important-gates
EXPECT:                 FAIL important-gates-unknown-id: line 19
FORBID:                 important-gates-uncovered-gate
overlay/data/expected/important-gates.ids
```

**The exact shape of a control directory**

```
tools/check/controls/<control-name>/
  CHECK              one line: the step name (a file in tools/check/steps/)
  EXPECT             one token per line, matched as a fixed substring of one output line
  FORBID             optional; one token per line; no non-empty line may match
  MAKE               optional; one transform per line (see verbs below)
  overlay/           optional; copied over a fresh copy of _good/tree, same relative layout
  DEVIATION          new in this phase (A-09); one declared overlay difference per line
```

**The `MAKE` verb vocabulary** — measured from `run-controls.sh:74-136`: `delete`, `empty`, `write`,
`rename`, `crlf`, `bom`, `nonascii`, `homepath`, `secret`, `password`, `binary`, `highbytes`, `nul`,
`scratchpath <private-tmp|var-folders|tmp-claude|root-home|tilde>`, `credential <json-password|api-key|slack|anthropic|google>`,
`email`. A verb not in that list is `control-make-verb` (`:127`). Paths are refused if absolute or containing
`..` (`:86`: `case $_p in /*|*..*) fail control-make-path "$_p"; return 1 ;; esac`).

**How `run-controls.sh` decides pass/fail** — `ctl_one` (`:168-222`), in order:

1. `CHECK` missing or naming a non-existent step → `control-unknown-step`, and the control is **not**
   counted ok (`:171-174`).
2. `EXPECT` missing, empty, or containing a blank/whitespace-only line → `control-expect-blank`
   (`:176-186`). A blank line would match every output.
3. `build_tree` fails → `control-build` (`:188`).
4. The step **exits zero** → `control-exit-zero` (`:189-191`). This is the core contract: a negative control
   passes only when the step under test **refuses**.
5. One awk pass over the output (`:197-212`): every non-empty `EXPECT` line must occur as a substring
   (missing → `control-token-missing`, `:218`); no non-empty `FORBID` line may occur
   (`control-forbidden-token`, `:215`). An awk failure counts as missing (`:213`: `|| _v=missing`).
6. Success prints `control ok   <name>` (`:220`) and touches `$2/ok`.

The positive control `good_one` (`:156-166`) additionally requires the step to **pass** on the pristine good
tree **and** to print `-- <name> pass`.

**Conventions that must be replicated**

- `EXPECT` pins the token **with its colon and its identifying detail** where the token has one
  (`FAIL manifest-digest-mismatch: FX-good`, not `FAIL manifest-digest-mismatch:`). IN-05 of
  `01-REVIEW.md:262-266` calls the bare-token form a weakness.
- `FORBID` names the token that a *wrong* reason would produce — every `important-gates-*` control uses the
  sibling token. This is how "fails for its intended reason and no other" is made mechanical.
- One control per intended reason; the directory name is the token name or a close derivative
  (`glossary-no-files` → `glossary-no-files`, `important-gates-unknown-id` → three variants suffixed by
  phase).
- No `#` comment files inside a control directory — the shape is exactly the six names above.

**Hazards**

- A control that passes on the good tree **and** the bad tree is the "check that cannot fail" of lesson 20.
  Pitfall 7.
- `EXPECT` is a **substring** match, not a line match — a token that also appears inside another finding's
  detail matches for the wrong reason. `FORBID` is the mitigation.
- `MAKE write` writes `${3:-x}` plus a newline only (`:83`), so a multi-token fixture line is impossible via
  `write`; use an `overlay/` file.

---

#### `tools/check/controls/_good/tree/.planning/REQUIREMENTS.md` + `.../milestones/v0.1.0-REQUIREMENTS.md` (new) (toy requirement records, config)

**No exact analog** — the good tree's `.planning/` currently holds exactly two files:
`tools/check/controls/_good/tree/.planning/PROJECT.md` and `.../.planning/ci-runs.tsv` (verified).

**Shape to copy — the real records' traceability table.** `.planning/milestones/v0.1.0-REQUIREMENTS.md:242`:

```
| REL-01 | Phase 8 | Complete (stage 2 recorded) |
```

and `.planning/REQUIREMENTS.md:97-99`:

```
| EV-01 | Phase 9 | Pending |
| EV-02 | Phase 9 | Pending |
| EV-03 | Phase 9 | Pending |
```

**The parse rule (A-01):** match `^\| *[A-Z]+-[0-9]+ *\|`, take field 2 as the id and field 4 as the status,
treat `Complete*` as a prefix match. **Do not** parse the `- [x] **REQ-NN**:` checkbox lines — the
traceability table is the record the project already treats as authoritative.

**Conventions:** the toy records are **minimal** — a `## Traceability` table listing exactly the REQ ids the
toy registry covers, all `Complete`. They are then consistent with the toy registry by construction
(09-RESEARCH §Q5.2 consequence 1). A copy of the real record is a drift hazard and must not be used.

**Hazards**

- `denylist.sh:36-38` skips `.planning/*` by name:
  ```sh
  case $f in
    .planning/*|.claude/*|docs/provenance/*|tools/check/*|docs/provenance.md|data/sources.json) continue ;;
  esac
  ```
  so adding `.planning/` files to the good tree cannot trip the provenance scan.
- The good tree's `filelist` is `find . -type f | sort` (`run-controls.sh:144`), so new `.planning/` files
  enter every toy step's `FILELIST`. Only `ci-log`, `denylist` and `glossary` read `.planning/` today
  (`grep -ln '\.planning' tools/check/steps/*.sh`), and `denylist` skips it.
- With minimal records the `_good` positive control is consistent by construction; with copied records it is
  not.

---

#### `tools/gate.sh` (gate driver, batch / orchestration)

**Analog: same file.** The step loop — `:104-124`, verbatim:

```sh
# Run every step (sorted, never stopping at the first failure).
passed=0
for name in $(cat "$GATE_TMP/steps.have"); do
  t0=$(date +%s)
  if sh "$CHECK_DIR/steps/$name.sh" > "$GATE_TMP/step.out" 2>&1; then
    cat "$GATE_TMP/step.out"
    keep_fails "$GATE_TMP/step.out"
    if grep -q -F -e "-- $name pass" "$GATE_TMP/step.out"; then
      passed=$((passed + 1))
      toks=$(sed -n "s/^-- $name pass //p" "$GATE_TMP/step.out" | tr '\n' ' ')
      tokens="$tokens$toks"
    else
      bad gate-step-no-pass-line "$name"
    fi
  else
    cat "$GATE_TMP/step.out"
    keep_fails "$GATE_TMP/step.out"
    bad gate-step-failed "$name"
  fi
  step_time "$name" "$(date +%s)" "$t0"
done
```

The two helpers that must be replayed in sorted order — `:37` and `:40-43`:

```sh
keep_fails() { grep '^-- FAIL ' "$1" >> "$GATE_TMP/fail.lines"; }
# Wall-clock timing (whole seconds, POSIX date): one `step-time <name> <s>` line per step, so a
# hosted log shows where the time goes. Not tokens: the summary line ignores them.
step_time() {
  printf 'step-time %s %s\n' "$1" $(($2 - $3))
  printf 'step-time %s %s\n' "$1" $(($2 - $3)) >> "$GATE_TMP/step.times"
}
```

The summary line — `:179`:

```sh
printf 'gate: %s steps=%s/%s controls=%s/%s %s\n' "$status" "$passed" "$want" "$ctl_ok" "$ctl_total" "$tokens" > "$GATE_TMP/summary.line"
```

The step list is sorted at `:92` (`sort -u "$GATE_TMP/steps.have" -o "$GATE_TMP/steps.have"`), so today's
execution order **is** sorted name order.

**Conventions:** `bad()` (`:29-33`) writes both to stdout and to `$GATE_TMP/fail.lines`; measurement lines
(`step-time`, `gate-env`, `gate-cpu`, `gate-time`) are documented in the header (`:4-6`) as **not tokens**;
`GATE_TMP` and friends are exported once (`:26`).

**Hazards**

- **Pitfall 8:** `tokens="$tokens$toks"` (`:114`) is inside the run loop; a partition that replays bodies but
  not the token accumulation reorders the summary tail. `passed`, the `bad` lines and `step-time` must be
  built during the replay too (A-05).
- **Pitfall 9:** the before/after diff can never close literally — `step-time`, `gate-cpu`, `gate-time` and
  `gate-env` carry measurements. Normalise those four line shapes.
- `$GATE_TMP/step.out` is one reused filename (`:108`, `:118`); a buffered partition needs
  `$GATE_TMP/step.out.<name>` or the 38 outputs overwrite each other.
- `setup`, `controls` and `controls-wait` step-time lines are outside the loop (`:84`, `:138-139`) and keep
  their positions.
- `keep_fails` appends to `$GATE_TMP/fail.lines`, which `ci-summary.sh` renders into
  `GITHUB_STEP_SUMMARY` (`:19-22`), so it must run during the replay, not during execution.

---

### Bookkeeping (one commit of its own)

---

#### `.planning/PROJECT.md` (decision record, prose)

**Analog: same file's D-register row, `:242`.** The row is one table cell pair and carries the claim, the
mechanism, the owner authorization and the reversal evidence in one line:

```
| REL-01 / D-31, D-42 | The milestone tool must not create a tag. `.planning/config.json` carries `git.create_tag: true`, ... The key reads `true` in the working-tree copy at the Phase 8 close, so the change is still to be made | Reversal evidence: ... |
```

**Measured correction (A-06):** `.planning/config.json:15` already reads `"create_tag": false,`. The D-31 item
is a **prose** fix only. An executor must never stage `.planning/config.json`.

**Convention:** a correction is appended, never a rewrite — the historical sentence stays and the correction
is added beside it (CONTEXT: "the historical line stays and the correction is appended"). The row keeps its
`| <ids> | <claim> | <reversal evidence> |` shape.

---

#### `.planning/ops/RELEASE-OPS.md` (ops record, prose)

**Analog: same file.** Three edits, at `:13`, `:31` and `:65`:

```
`.planning/config.json` carries `git.create_tag: true`. The milestone tool (`gsd-complete-milestone`) would create, and offer to push, a tag in the style of the milestone label (v1.0). ...
```
```
- P3: rights: the tracked-binary scan is empty, the install list equals the pinned ten files, the manifest census is unchanged or explained, ...
```
```
3. Open the `gsd/records-m1-close` branch and make the `git.create_tag` change of item a as its first commit, alone, before the milestone tool is used.
```

**Measured correction (A-06):** `data/expected/pkg-install-files.ids` has **11** non-comment lines, so "ten"
becomes "eleven". The `create_tag` prose is stale for the same reason as `PROJECT.md:242`.

**Convention:** the section heading carries the decision ids — `## (a) The git.create_tag footgun (D-31, D-42)`
(`:11`) — and each numbered step is one action, imperative, with the file named.

---

#### `core/src/machine/machine_digest.c` (C source comment, prose)

**Analog: same file, `:12-14`, verbatim:**

```c
 * and 19 PSGLOG (chain and record count, plan 05-03, D-18). The composite hashes the digests of domains 1..4 under a
 * version with the domain ids ("IVSSTATE"); it is version 2 since plan 05-03 (deliberate, D-18:
 * guest-visible PSG state is in it) and never includes 16..19, so an observation row changes only
 * its own domain's digest (A-03).
```

**Measured correction (A-06):** `core/src/machine/machine.h:157` reads
`#define IVS_DIGEST_COMPOSITE_VERSION 3u` and `core/include/ivs/ivs_machine.h:352` says "composite version 3,
plan 06-05", so the sentence becomes version 3. **Not** line 407 — that comment describes a fault site and is
correct in context.

**Conventions:** file-header comment block starts with `/* SPDX-License-Identifier: MIT */` on line 1 and the
domain map on lines 2+; every historical change names its plan and decision tag (`plan 05-03, D-18`); line
width is the file's own (~120 columns, wrapped with a leading ` * `).

**Hazard:** the file is a **core** source. This phase's boundary says no change to the core — a comment
correction is the one permitted exception and the plan must say so explicitly, or the executor's own scope
rule refuses it.

---

#### `docs/runner.md` (operator doc, prose)

**Analog: same file's golden-command block, `:247-256`, verbatim (excerpt):**

```
Goldens are written only by `ivs-run --update-golden <file>`, run by a person; a changed golden is a reviewed diff. The gate never writes one: the step only compares. The tracked goldens were written with:

```
build/dev/runner/ivs-run --content-dir build/dev/images --update-golden data/golden/diag-cpu-1.digests data/scenarios/diag-boot-pass.ivss
build/dev/runner/ivs-run --content-dir build/dev/images --update-golden data/golden/diag-input-pass.digests data/scenarios/diag-input-pass.ivss
...
build/dev/runner/ivs-run --content-dir build/dev/images --update-golden data/golden/demo-lamp-tone-idle.digests data/scenarios/demo-lamp-tone-idle.ivss
```
```

**Measured gap (A-06):** `grep -n bench docs/runner.md` → **no match**. The two bench goldens
(`data/golden/bench-lamp-idle.digests`, `data/golden/bench-lamp-input.digests`) have no writing command
anywhere. GUARDRAILS R11 (`:28`) requires "the writing command added to `docs/runner.md`" for a new golden.

**Conventions:** a fenced ```` ``` ```` block of one command per line, `build/dev/runner/ivs-run
--content-dir build/dev/images --update-golden data/golden/<name>.digests data/scenarios/<scenario>.ivss`;
the surrounding prose says who runs it and that the gate never writes a golden.

**Hazards:** the bench goldens are **scenario** goldens (the workload's `scenario` column, `docs/bench.md:92`),
so the command form is identical to the seven above. `bench-lamp-snapshot` reuses `bench-lamp-idle`'s golden
(`docs/bench.md:97`) and therefore gets no command of its own.

---

#### Live record for the seven draft `VALIDATION.md` dispositions (ledger record, prose table)

**Analog: `08-ACCEPTED-DEBT.md` §1, `:11-23`.** Header, verbatim:

```
## 1. The six draft validation files (Phases 2 to 7)

Each file is a per-phase validation contract seeded by plan-phase: a test infrastructure table, a sampling rate, a per-task verification map, a Wave 0 list, a manual-only list and a sign-off block. [OBS] Each is still `status: draft` with `wave_0_complete: false` and `Approval: pending`; `nyquist_compliant: true` is set in all six. [OBS] `.planning/phases/01-evidence-profile-and-rights-foundations/01-VALIDATION.md` is `validated` and is not one of the six.
```

and the table row shape:

```
| File | Status | What the draft was meant to hold | Why a re-run adds nothing now | Where the evidence is |
|------|--------|----------------------------------|-------------------------------|-----------------------|
| `.planning/phases/02-cp-1610-cpu-core/02-VALIDATION.md` | draft | ... | ... | ... |
```

The seventh file is already dispositioned in prose at `:23`: "`.planning/phases/08-…/08-VALIDATION.md` is
Phase 8's own file and is also `draft`; it is outside the six of D-34 and is left to the close record of plan
08-21."

**Measured:** eight `VALIDATION.md` files exist; `01` is `status: validated`, `02`–`08` are `status: draft`
(verified by `grep -m1 '^status:'` over each). **Seven** drafts.

**Conventions:** `[OBS]` / `[INF]` / `[REC]` labels; a `**Written:** <date> by plan <NN-NN> (D-nn).
**Purpose:** ...` lead paragraph; one table row per item with the file path in backticks; **never rewrite an
archive** — the disposition is a row in a *live* record that cites the archive.

**Hazard:** CONTEXT says "by citing `08-ACCEPTED-DEBT.md` §1 in a live record — never a rewrite of an
archive". The live record must be chosen deliberately (`.planning/STATE.md`'s Deferred Items table is the
established live record — `08-ACCEPTED-DEBT.md:26-27` points at it: "The table is in `.planning/STATE.md`")
and the row must name the milestone the item now waits for, per `INDEX.md` rule 5.

---

## Shared Patterns

### A pure step's five-part shape
**Source:** `tools/check/steps/important-gates.sh:1-51`, `tools/check/steps/sources.sh:1-16`,
`tools/check/steps/glossary.sh:1-16`
**Apply to:** `important-gates.sh`, `sources.sh`, and the memmap step's new token
1. `#!/bin/sh` then `# SPDX-License-Identifier: MIT`.
2. `# Step <name>: <what it checks> (<REQ>, <D-nn>, plan <NN-NN>).` + purity sentence.
3. `# Tokens: <token>: <detail>, ...` then `# Stats: k=v ...`.
4. `LC_ALL=C; export LC_ALL` then `: "${ROOT:?...}"` / `: "${CHECK_DIR:?...}"` / `: "${FILELIST:?...}"`.
5. `tmp=$(mktemp -d ...)` + two traps, then accumulate findings, then
   `sort -u` + `printf -- '-- FAIL %s: %s\n'` + `exit 1`, else `printf -- '-- <name> pass %s\n' "$stats"`.

### The finding/token/EXPECT chain
**Source:** `important-gates.sh:107-111` → `run-controls.sh:193-218` → `data/expected/controls/*.ids`
**Apply to:** every new token in this phase
A token is emitted as `-- FAIL <token>: <detail>`; the control's `EXPECT` pins a substring of that line; the
control's name is listed in `data/expected/controls/<family>.ids`. All three move in one commit or
`run-controls.sh:267-268` refuses the tree.

### The two-mode cost switch
**Source:** `tests/runner-process-tap.sh:532-538` (`mutant_case`), `:106` (`config=${5:-}`)
**Apply to:** the two demo-lamp scenarios and the pass/shift pair
Full run on `Release`; static judgement otherwise; the **same** ids on both presets; no `# SKIP`; a
`# <family>: configuration=<c> full=<n> static=<m>` diagnostic line makes the mode visible.

### The preserving condition beside the trim
**Source:** `tests/state/state_util.c:476` (whole-instance `memcmp`, outside the digest branch) and `:491`
(`produced`, unconditional)
**Apply to:** trims (a), (c), (d)
A trim lands with its preserving condition in the same commit; the condition is the assertion the trim does
**not** touch, named by file:line in the plan.

### The oracle-first citation
**Source:** `docs/video/demo-lamp-tone-1-derivation.md:320/394, 398/434, 438/512, 516/578` (the four
`asserts-begin`/`asserts-end` marker pairs) and `:316` (`Counts: pass 55 asserts, idle 29, shift 55, swap 49.`)
**Apply to:** the lever (3) judgement
The judgement compares files and runs nothing; the frozen derivation is the oracle and the plan cites its
line numbers rather than restating the expected values.

### `.planning/` read from a step
**Source:** `tools/check/steps/ci-log.sh:3,24`; `tools/check/steps/glossary.sh:64-66`
**Apply to:** `important-gates.sh` (EV-04)
Anchor at `$ROOT`, name the file in the finding detail, and amend the step's purity sentence in the same
commit. Both precedent steps are preflight by the partition predicate, so reading `.planning/` does not
disturb the partition.

---

## No Analog Found

| File | Role | Data Flow | Reason |
|------|------|-----------|--------|
| `data/expected/important-gates-excluded.ids` | exclusion ledger | config/batch | No `<REQ> <reason>` ledger exists. Closest: `cp1610-skips.ids` (two-column, `#allow-empty`), `cp1610-flag-exempt.ids` (reason in the comment, not the line), `state-sweep-notes.ids` (3-column with a long grammar docstring). The plan should copy `cp1610-skips.ids`' docstring grammar and `state-sweep-notes.ids`' reason-word style, and add the two-directional staleness rule this phase needs. |
| `tools/check/controls/_good/tree/.planning/REQUIREMENTS.md` and `.../milestones/v0.1.0-REQUIREMENTS.md` | toy requirement records | config/batch | The good tree's `.planning/` holds only `PROJECT.md` and `ci-runs.tsv`. The shape to copy is the **real** records' `## Traceability` table (`.planning/milestones/v0.1.0-REQUIREMENTS.md:242`), but minimal — exactly the ids the toy registry covers, all `Complete`. |
| `DEVIATION` files under `tools/check/controls/important-gates-*/` | control declaration | config/batch | No `DEVIATION` file exists anywhere in the tree (`find tools/check/controls -name DEVIATION` → empty). The format is A-09's proposal: one line per permitted difference, `<` or `>` followed by the exact line; `run-controls.sh` recomputes `diff <(sort good-noncomment) <(sort toy-noncomment)` and refuses both an undeclared diff line and a declaration that matches none. |
| `data/expected/controls/run-controls.ids` | control-token inventory | config/batch | No inventory file names a `control-*` runner token; the runner's own tokens are proven inside `tools/check/steps/runner-selftest.sh` (`:1-14`). Either extend `gate-hardening.ids` or create this file, and in both cases add the token to `runner-selftest.sh`'s required-token list. |

---

## Metadata

**Analog search scope:** `tools/check/` (steps, controls, cmake modules, runner), `tools/`, `tests/`
(runner-process, state), `data/expected/` (registries, inventories, ledgers, control ids), `cmake/`,
`core/src/machine/`, `docs/`, `.planning/` and `.planning/ops/`.

**Files scanned:** 38 step scripts, 380 control directories (six inspected in full), 37 control inventory
files, 12 fault inventories, 63 `data/expected/*.ids` ledgers, 2 C state test files, 1 tap script (1733
lines, targeted reads only), 1 gate driver, 1 control runner, 1 cmake checker module, 1 cmake generator,
4 bookkeeping records.

**Tracking verified:** every analog path named above returns a non-empty `git ls-files -- <path>`; no
gitignored install/runtime mirror is named. The good tree
(`tools/check/controls/_good/tree`, 120 files) is tracked source, not a synced mirror.

**Measured facts this map relies on (recomputed, not quoted):**
`awk '!/^#/ && NF' data/expected/important-gates.ids | wc -l` → 306;
`grep -c 'FND-\|CPU-\|MEM-' data/expected/important-gates.ids` → 0;
`awk 'END{print NR}' data/expected/important-gates.ids` → 396;
`ls -d tools/check/controls/*/ | grep -v _good | wc -l` → 380;
`find tools/check/controls/_good/tree -type f | wc -l` → 120;
`grep -rn 'release-only' tests/ data/ tools/check/steps/` → no match;
`find tools/check/controls -name DEVIATION` → empty;
`find .planning -name '*VALIDATION.md'` → 8 files, `01` validated, `02`–`08` draft;
`awk '!/^#/ && NF' data/expected/pkg-install-files.ids | wc -l` → 11;
`grep -n bench docs/runner.md` → no match.

**Pattern extraction date:** 2026-11-24
