---
phase: 09-registered-evidence-and-gate-cost
asvs_level: 1
status: passed
high: 0
medium: 0
low: 4
verdict: SECURED
block_on: high
threats_total: 18
threats_closed: 18
threats_open: 0
unregistered_flags: 1
audited: 2026-11-26
auditor: gsd-security-auditor
tree: fd0ef31
phase_base: e172643
source_under_audit: "git diff --stat e172643..fd0ef31 -- tools tests cmake = 61 files, 1284 insertions, 88 deletions"
# The register is the union of the nine plans' <threat_model> blocks: T-09-01 .. T-09-17 plus
# T-09-SC, which appears in all nine plans with one id, one severity and one disposition and is
# verified once here. Severity order critical > high > medium > low; block_on = high, so an open
# threat at medium or low is recorded and does not count toward threats_open.
severity_split: "high 12, medium 5, low 1"
dispositions: "mitigate 16, accept 2"
---

# Phase 9: Registered Evidence and Gate Cost — Security Audit

**Verdict:** `SECURED` — 18/18 registered threats verified, `threats_open: 0`, 4 low findings
recorded, none blocking at the `high` threshold.

This phase is a **cost and evidence-durability** phase. It touches no network, no authentication, no
user-input parsing beyond a markdown table and a set of one-line ledger files, no filesystem writes
outside `build/` and the step-local `mktemp -d` directories, no secrets and no third-party bytes. The
threat model it actually has is the one its own plans declare: **a public CI gate that decides whether
evidence exists, and the records that gate reads.** The audit below verifies each declared
disposition at the site the plan names, then examines the one genuinely new surface the phase creates
— the buffered replay partition in `tools/gate.sh`.

Nothing in this file was written from a SUMMARY or a VERIFICATION claim. Every "verified" cell below
is a `file:line` I read on tree `fd0ef31`, and every probe in `## Probes run` was executed in this
audit, on a scratch copy under `/tmp` or read-only against the tree.

---

## Threat register verification

One row per distinct threat id. `file:line` is the site that implements the claimed mitigation on
tree `fd0ef31`; "verified by" names the probe that shows the mechanism is live rather than merely
present.

| Threat | Category | Sev. | Disposition | Claimed mitigation | Implementing file:line | Verdict |
|---|---|---|---|---|---|---|
| **T-09-01** the static judgement of a folded scenario (`static_scenario` / `derivation_equal`) | Tampering | high | mitigate | compares against the frozen derivation and its count line, refuses an empty derivation block, proved rejecting a one-value change on every run in a subshell with its own `why` target | `tests/runner-process-tap.sh:455` (`derivation_equal`), `:464` (`variant-derivation-empty`), `:470-478` (assert count vs `docs/video/demo-lamp-tone-1-derivation.md:316`), `:480-495` (header + id), `:496-513` (input transform), `:514-516` (whole-body equality), `:528-545` (`variant_negative`, subshell target at `:535`), called from `:553-560` and `:618-620` | **mitigated** |
| **T-09-02** the dev-versus-release judge | Repudiation | high | mitigate | a `release-only` id is skipped **by name** and counted; the missing-report refusal is untouched for every other id | `tools/check/steps/golden.sh:416-418` (skip), `:420-427` (pair refusal untouched), `:436-442` (count from the inventory, each row still needing its release report) | **mitigated** |
| **T-09-03** column 5 of `data/expected/runner-scenarios.ids` | Elevation of privilege | medium | mitigate | ordinary `ok` line (never a skip), `tests-skipped` stays 4, and `tests-inventory` still requires the id set to equal the inventory on both presets | `tests/runner-process-tap.sh:601` (vocabulary), `:604-620` (mode switch, `result` path unchanged), `tools/check/steps/tests-inventory.sh:166-167` (`comm -23`/`comm -13` per preset), token `release-only=2` in the frozen summary line | **mitigated** |
| **T-09-04** `su_call_row` / `rs_run_family` (trim c) | Tampering | high | mitigate | the whole-instance `memcmp` and the `produced` accumulation are outside the digest branch; sampling is a second field | `tests/state/state_util.c:479` (`memcmp`, outside), `:487` (the only branch), `:496` (`produced`, outside), `:435`/`:538` (row index threaded), `tests/state/state_util.h:182-185` (`digest_sampled` is a second flag), `tests/state/state_restore.c:1588` (the one caller that sets mode 2) | **mitigated** |
| **T-09-05** `fz_run_flips` (trim d) | Tampering | medium | mitigate | the offset loop bound, `fz_pick` and `masks[]` untouched; the acceptance refusal stays the guard that a flip moves the digest | `tests/state/state_fuzz.c:925` (bound), `:927` (`fz_pick`), `:923` (`masks[]`), `:931-933` (the only change), `:954` (acceptance refusal) | **mitigated** — with the citation defect F3-3 |
| **T-09-06** the `.planning/CI-EFFICIENCY.md` entry | Repudiation | high | mitigate | every figure recomputed in the commit that prints it; the load condition stated as taken; the checkpoint word left to the orchestrator | `.planning/CI-EFFICIENCY.md:1675-1681` (the recomputation command), `:1685-1691` (wave A1 window), `:1736-1747` (the row's window, recomputed), `:1782` (exactly one `phaseA1:` line) | **mitigated** |
| **T-09-07** the registry and the accepted set | Tampering | high | mitigate | every row verified against the step's own resolution loops before writing; refuses an id in no inventory and a requirement with no line | `tools/check/steps/important-gates.sh:124-141` (the inventories), `:160` (accepted set), `:167-171` (4 columns, gate name, mechanism, accepted requirement), `:172` (id in its inventory), `:177-181` (both requirement refusals) | **mitigated** |
| **T-09-08** the ten toy registries | Repudiation | medium | mitigate | 17 lines appended at the end of each, zero deletions, no comment line, so the numstat count is the row count | all ten files: `git diff --numstat e172643..fd0ef31` → `17/0` each, `0` added `#` lines; positive control passes on the good tree | **mitigated** |
| **T-09-09** the derived must-be-covered set and the exclusion ledger | Tampering | high | mitigate | set derived from records outside the registry, a different file, stale exclusion refused, both directions proved by one committed control | `tools/check/steps/important-gates.sh:88-96` (derivation), `:99-122` (ledger, format enforced), `:160` (subtraction), `:177` (`-complete-uncovered`), `:179` (`-excluded-stale`); controls `important-gates-complete-uncovered`, `-excluded-malformed`, `-uncovered-gate` in `data/expected/controls/important-gates.ids` | **mitigated** — residual F2 |
| **T-09-10** a vacuous derivation | Denial of service | high | mitigate | the toy records list exactly the toy registry's ids, so the good-tree positive control fails if the derivation reads nothing; the real arithmetic is checked | `tools/check/controls/_good/tree/.planning/REQUIREMENTS.md` + `…/milestones/v0.1.0-REQUIREMENTS.md` (45 Complete − 1 excluded = 44 = 44 registry rows, re-derived in probe P4); real tree 54 − 10 = 44 (probe P1); deleting the records refuses (probe T10) | **mitigated** |
| **T-09-11** the toy-registry drift check | Tampering | high | mitigate | both directions refused, comments excluded for a stated reason, both tokens proved by committed cases | `tools/check/run-controls.sh:299-321` (`control-toy-drift` at `:315`, `control-deviation-unused` at `:318`), cases at `tools/check/steps/runner-selftest.sh:94-105` and `:118` | **mitigated** — both tokens observed firing in probe P3 |
| **T-09-12** the `control-pass-line-not-last` guard | Tampering | medium | mitigate | the guard is on the step side where no control directory can reach it, proved by a committed case that runs a copied `CHECK_DIR` | `tools/check/run-controls.sh:172-186` (`good_one`), `tools/check/pass-line.sh:19-26` (the gate-side rule as a file), proofs `runner-selftest.sh:139-176` | **mitigated** — both proofs observed in probe P3 |
| **T-09-13** `SRC-` extraction and output anchoring | Tampering | high | mitigate | the class is measured against the pristine tree (0 unregistered tokens) and its control pins both reproductions; the anchoring rule's second root is measured and re-derived | `tools/check/steps/sources.sh:43-48` (`SRC-[A-Za-z0-9_-]+`), `tools/check/manifests.cmake:134-139` (two roots, raised before the digest comparison); controls `sources-src-token-whole`, `manifests-output-anchor` | **mitigated** — residual F4 |
| **T-09-14** the memmap generator control | Repudiation | medium | mitigate | the generator runs first, its refusal is additive with its own first message line, the step does not return before its own checker, and `memmap-generator-missing` is proved by a committed case | `tools/check/steps/memmap.sh:45-51` (liveness guard), `:52-63` (additive refusal, `rc=1` only), `cmake/IvsMemmapProbes.cmake:141` (the only write, `"${OUT}"`); control `memmap-generator-refused`; proof observed in P3 | **mitigated** |
| **T-09-15** the zero-document item | Repudiation | low | accept | closed by re-running two existing controls and quoting their guards; the two remaining uncontrolled tokens recorded as a declined extension | `data/expected/controls/gate-hardening.ids:2-3` (`glossary-no-files`, `sources-no-files`), both control directories present; the declined extension is `.planning/STATE.md:557` | **accepted** — recorded here and in the phase's open-item 2 |
| **T-09-16** `tools/gate.sh`'s step loop and replay | Tampering | high | mitigate | every stdout-affecting emission produced during the sorted replay; `keep_fails` in the replay; the partition line makes a class change visible; the normalised before/after diff is a required verify | `tools/gate.sh:136-141` (execution buffers only), `:163-193` (replay emits body, fails, tokens, step-time), `:166` (`keep_fails` in the replay), `:121`/`:129` (derived partition, printed); committed capture `09-PREFLIGHT-BEFORE-SH.log` (20 926 bytes) | **mitigated** — residual F1 is the new surface |
| **T-09-17** the bookkeeping commit and the requirement close | Tampering | high | mitigate | every correction appended, never a rewrite; the one core touch is a comment; the status flip and its ledger rows land in one commit | `git diff e172643..fd0ef31 -- .planning/milestones/ .planning/phases/0*` → empty (archives untouched); the flip commit `31821b4` carries `REQUIREMENTS.md` + `important-gates-excluded.ids` together; `.planning/config.json` unchanged and `.planning/state.json` unstaged | **mitigated** |
| **T-09-SC** npm/pip/cargo installs (declared in all nine plans) | Tampering | high | accept | no package manager invoked and no dependency added | verified factually, not by assertion: `git diff e172643..fd0ef31` adds **zero** `npm`/`pip`/`cargo`/`gem`/`apt-get`/`yarn` invocations outside plan prose (probe P6); no `package*.json`, `requirements.txt`, `Cargo.*`, `Gemfile`, `poetry.lock`, `vcpkg`/`conanfile` touched; `cmake -P` runs an in-repo script | **accepted** — this file is the accepted-risks record |

**Closed: 18/18. Blocking-open: 0. Non-blocking-open: 0.** No registered threat is unimplemented; the
four low findings below are residuals of mechanisms that are present and working, plus one
unregistered surface.

---

## The tampering class this phase actually has

The real surface is a public CI gate that decides whether evidence exists. The question asked of
every input is: **can a change to this make the gate pass while evidence is missing or wrong?** Each
answer below is a probe I ran on a scratch copy of the tree (`/tmp/sec09/root`), not a reading.

| Input the gate reads | Can it make the gate pass while evidence is missing? | What stops it | Is the stopper itself checked? |
|---|---|---|---|
| a registry line in `data/expected/important-gates.ids` | **No, in the two directions that matter.** Deleting the lines for an accepted requirement refuses `important-gates-complete-uncovered` **and** `-uncovered-gate` (probe T1). Adding a row for an id in no inventory refuses `-unknown-id` (T2). Adding a row for a requirement outside the derived accepted set refuses `-unknown-id line n` (T4a) | `important-gates.sh:167-172`, `:177-181` | Yes — committed controls `important-gates-unknown-id`, `-uncovered-gate`, `-complete-uncovered` pin each token, and `run-controls.sh` requires each to exit non-zero with its token |
| **a registry row pointed at the wrong but existing mechanism** | **Yes.** Replacing `profile-record control profile-missing-key FND-01` with `profile-record control claims-unknown-tier FND-01` (a real control, unrelated to FND-01) **passes** (T13). The gate proves an id *exists and is judged*, never that it *proves this requirement* | nothing machine-side — the non-vacuity sentence in each row's comment plus review | No. This is the register's own stated boundary ("a row that resolves to a mechanism that passes vacuously is a false claim", `09-04-PLAN.md` trust boundaries) and the disposition is "verified against the step's five resolution loops **before it is written**", i.e. a write-time human act. **Not a gap against the declared disposition**; recorded so a later reader does not assume a semantic binding exists |
| a ledger row in `data/expected/important-gates-excluded.ids` | **Partly.** A *malformed* row is refused (`-excluded-malformed`, T12); emptying or deleting the file refuses ten `-complete-uncovered` (T11/T11b); a row whose requirement is not Complete refuses `-excluded-stale` (T7b). **A well-formed row for a Complete requirement, together with the deletion of that requirement's registry rows, passes** (T4b) | the two-file shape of that edit (an addition in the ledger *and* a deletion in the registry — one alone is refused, T4a), the mandated reason text, and human review | No machine check exists on the reason's truth, and none can exist. **Finding F2** |
| an inventory column — `runner-scenarios.ids` column 5 | **No.** Marking an id `release-only` removes the dev-report requirement in `golden.sh`, but `tests-inventory` still requires the id set to equal the inventory on **both** presets (`tests-inventory.sh:166-167`), and the id is in `data/expected/tests/runner-process.ids`. The count `release-only=2` is in the frozen summary token line | `tests-inventory.sh:166-167`; the token string | Yes — the token line is compared token-for-token by the verifier (`09-VERIFICATION.md` frontmatter `gates_rerun_*`), and `host-dependent=3`/`release-only=2` would move visibly on any column-4 or column-5 edit |
| an inventory column — any `data/expected/{controls,faults,tests}/*.ids` | **No.** Deleting an inventory line while its control/fault/test still exists refuses `control-inventory-orphan` (`run-controls.sh:293`); adding a control with no line refuses `control-not-in-inventory` (`:292`). A deleted inventory entry also makes any registry row naming it `-unknown-id` | `run-controls.sh:290-293`, `important-gates.sh:172` | Yes — both directions are one-line `comm`s with committed controls |
| a `DEVIATION` file | **No, for the gate's verdict.** It can only *permit* a difference between a toy overlay and the good tree, and both directions are refused: an undeclared difference is `control-toy-drift`, a declaration matching nothing is `control-deviation-unused` (`run-controls.sh:314-319`). It cannot suppress a step's own token, because each control still pins `EXPECT`/`FORBID` | `run-controls.sh:299-321` | Yes — `runner-selftest.sh:94-105` builds both cases and `:118` requires both tokens; observed in probe P3. The check has **no count of overlays compared** (review IN-06, still open) — deleting the good-tree registry removes the check, but the `_good` positive control then fails on `important-gates-uncovered-gate` |
| a `.planning/` requirement record | **No.** Downgrading a registered requirement to `Partial` refuses `-unknown-id` on its own rows (T7); downgrading an excluded one refuses `-excluded-stale` (T7b); deleting the records entirely refuses all ten `-excluded-stale` (T10). The derivation is *not* vacuous-safe on its own (`nreq=0` would find nothing), but the 330 registry rows then become `-unknown-id` | `important-gates.sh:88-96`, `:167-181` | Yes — committed controls over `.planning/` overlays (`important-gates-complete-uncovered`, `-bad-req-phase5/6`, `-req-uncovered-phase5/6/7`) |

**Summary of the tampering class:** every *single-file* edit that would let the gate pass without
evidence is refused by a committed, control-pinned mechanism. The two edits that succeed — a
semantically wrong but resolving registry row (T13), and a well-formed exclusion plus the matching
registry deletion (T4b) — both require a coherent multi-line diff whose only stopper is review. That
is the design recorded in amendment A-01 and in `09-04-PLAN.md`'s trust-boundary table, not an
implementation gap; F2 records the residual.

---

## New surface: the buffered replay partition in `tools/gate.sh`

The partition buffers each step's `stdout`+`stderr` into `$GATE_TMP/step.out.<name>` and replays it.
The four questions asked of it:

| Question | Finding |
|---|---|
| predictable or world-writable temp path? | **No.** `GATE_TMP=$(mktemp -d "${TMPDIR:-/tmp}/gate.XXXXXX") \|\| exit 1` (`tools/gate.sh:27`) — random 6-character suffix, created by `mkdtemp` with mode **0700**, owner-only, and the assignment **fails closed**: with `TMPDIR` unset-but-bad the gate exits 1 before any step runs (probe P7: `mktemp: mkdtemp failed … rc=1`; `stat` → `mode=700 owner=jon`). The whole template is quoted, so a `TMPDIR` with spaces is passed as one argument |
| a buffer filename that can collide or be steered by a step name? | **Not steerable, but collidable by construction.** `$name` is `basename` of a `tools/check/steps/*.sh` match (`:118-120`), so it can contain no `/`; two steps cannot share a name because the glob yields one file per name and `gate-step-missing`/`gate-step-extra` (`:126-127`) force the name set to equal `data/expected/gate-steps/*.ids`. The names are *deterministic*, not random: `step.out.<name>` is guessable by any step |
| a step able to write another step's buffer? | **Yes.** `GATE_TMP` is exported (`:37`) and every step runs with the same uid and the same directory, so a step can open `$GATE_TMP/step.out.<other>` and the replay at `:165` prints, `:166` collects `-- FAIL` lines from, `:179` pass-line-checks and `:183` tokenises the **forged** bytes. Reproduced: probe B1 shows `aaa`'s body replaced by a line written by `zzz`. The **verdict cannot be flipped** — `rc.<name>` is written by the driver from the step's own exit status (`:137-141`) and is not in the step's reach, so a forged "pass" body for a failing step still yields `gate-step-failed` (`:189-190`). What *is* reachable is the step's body, the `-- FAIL` lines fed to `ci-summary.sh`, and the summary-line tokens. **Finding F1**, recorded as the phase's one `unregistered_flag` |
| the `mktemp -d` mode and the trap | **Sound.** `trap on_exit EXIT` (`:34`) with `rm -rf "$GATE_TMP"` quoted (`:31`), plus `trap 'exit 1' HUP INT TERM` (`:35`) so a signal routes through the EXIT trap rather than past it; `on_exit` also kills and reaps the background control runner (`:29-30`). Proved: a gate run that early-exits leaves no new temp dir (probe P8: `gate.*` dirs before=1 after=1). One **pre-existing** `gate.jZ7Z9O` remains from an *interrupted pre-partition run* (it holds `step.out`, the single-buffer name) — mode 0700, containing only the filelist and public check output, no secrets and no third-party bytes. A `SIGKILL` cannot be trapped, so that residue class is inherent, not a phase defect |

---

## New shell and CMake code

| Area | Finding |
|---|---|
| argument quoting | All new expansions are quoted at every site I checked; `dash -n` parses all nine changed shell files cleanly. `shellcheck -s sh -S warning` on the changed files reports five warnings, all of them intentional: `SC2046` on `$(cat "$GATE_TMP/t.$name")` (the value is `<epoch> <epoch>` from the gate's own `printf`), `SC2088` on the deliberate literal `~/projects/x` a control writes, `SC2097/SC2098` on `run_step`'s exported prefix (the point is that the `sh` child sees them), and `SC2034` on a pre-existing unused loop variable |
| `$ROOT` / `$FILELIST` trust | Both are derived by `cd … && pwd` or `mktemp -d`, exported, and consumed quoted. `sources.sh:48` greps `"$ROOT/$f"` with `$f` read from `$tmp/scan`, which is itself filtered from `$FILELIST` by two anchored `^data/.*\.json$` / `^docs/.*\.md$` / `^README\.md$` patterns (`sources.sh:32-37`). No new code path takes a path from a report, a manifest or a record and opens it without the pre-existing `path_ok` guard |
| `awk` over repository-controlled files | The awk programs are fixed and the repository content is data only: no `getline` from a command, no `system()`, no `-v` value used as a program. `awk -v id="$1"` in `golden.sh:140-141` and `-v i="$_pi"` in `runner-process-tap.sh` do interpret backslash escapes in the value, but every id reaching them comes from an inventory whose shape is pinned. `grep -F -e "$id"` / `grep -q -x -F -e "$_l"` are used everywhere a token or a ledger line is matched, so a ledger line beginning with `-` cannot become an option |
| `cmake -P` in `memmap.sh` | **Writes nowhere but the step's own `mktemp -d`.** `memmap.sh:39-40` creates `tmp=$(mktemp -d "${TMPDIR:-/tmp}/memmap.XXXXXX")` with an EXIT trap, and passes `-DOUT="$tmp/ivs_memmap_probes.inc"`; `cmake/IvsMemmapProbes.cmake` contains exactly one write, `file(WRITE "${OUT}" …)` at `:141`, and no `execute_process`, no `configure_file`, no `file(REMOVE)` — verified by grep. `-DROOT` is read-only. The generator's path is derived from `CHECK_DIR` (`memmap.sh:43`), not `ROOT`, which is why a control tree cannot reach it and why the committed `st-memmap-generator-missing` case is the liveness guard |
| the pass-line rule as a file | `tools/check/pass-line.sh:21-26` compares the last non-empty line with a `case` pattern and falls back to `grep -q -F` for the "verdict present but not last" case, so the two failure shapes (exit 1 vs exit 2) are distinguished without a regex over step output. `$1` is interpolated into a shell *pattern* (`:23`) and `$name` into a `sed` *regex* (`tools/gate.sh:183`) — a step name containing a glob or regex metacharacter would widen those two matches. Reachable only by an in-repo step rename that the ids inventory would also have to accept, and the effect is a looser pass-line match, never a verdict flip. Recorded as a hardening note, not a finding |
| `manifests.cmake` anchoring | `check_entries(… "${dirname}")` is called only from `check_manifest` (`:252`) where `dirname` comes from `^fixtures/([A-Za-z0-9._-]+)/` (`:169`), so it is never empty and the `NOT "${ARGV3}" STREQUAL ""` guard at `:135` cannot be used to skip the rule. `manifest_load_all` selects only `^fixtures/[A-Za-z0-9._-]+/manifest[.]json$` (`manifest-lib.cmake:182`), so a top-level `fixtures/manifest.json` is never processed. **Residual F4**: `dirname` is interpolated into a CMake regex, and the `^data/` root is broader than the two measured files need |

---

## Rights and secrets

| Check | Result |
|---|---|
| third-party bytes added | **None.** Every file the phase adds is text: the added-file list has no non-text member (probe P6), and no `.bin`/`.rom`/`.ivsc`/`.exec`/`.grom`/`.wav`/`.ppm`/`.pdf` is tracked at all (`git ls-files \| grep -cE '\.(bin\|rom\|ivsc\|exec\|grom\|wav\|ppm\|pdf)$'` → **0**) |
| `tools/check/binary-ext.txt` and `.gitignore` | **Unchanged across the phase** — `git diff --stat e172643..fd0ef31 -- .gitignore tools/check/binary-ext.txt` is empty |
| secrets | **None.** No `ghp_`/`gho_`/`sk-ant`/`AKIA`/`xox[bp]-`/private-key block anywhere in the phase diff. The control runner's `MAKE` verbs assemble secret-*shaped* strings from split pieces (`run-controls.sh:113-114`, `:130-139`) so that no credential literal exists in the tree — unchanged by this phase and still only in a temp tree |
| absolute home paths | **None.** The only `/Users/` matches in the phase diff are two `.planning/` sentences quoting this very check (`09-VERIFICATION.md`, `09-REVIEW.md`); no tracked file contains an absolute home path |
| `.planning/` writes by a gate step | The `important-gates` step **reads** `.planning/` and writes nothing there: its only writes are `$tmp` (its own `mktemp -d`) and `build/gate`. `run-controls.sh` runs every step against a *copy* of the good tree with `IVS_GATE_DIR="$1/tree/build/gate"` (`:165`), so no control can write into the real tree |

---

## The fixture set is not a production trust boundary

`tools/check/controls/_good/tree`, every `tools/check/controls/<name>/overlay`, and every `MAKE` /
`DEVIATION` / `EXPECT` / `FORBID` file are **test fixtures**. They are inputs the control runner
copies into a private work directory (`run-controls.sh:151-160`), mutates deliberately, and drives a
step against; they are not read by the gate as evidence, they are not installed, and they are not a
production trust boundary. A fixture that is wrong makes a control meaningless — which is exactly
what the drift check, the inventory set-equality and the `_good` positive control exist to detect —
but a wrong fixture is a **test-quality** defect, not a vulnerability, and nothing here is reported
as one. The same applies to `MAKE`'s deliberate `../`-refusing path guard (`run-controls.sh:100`,
`:107`): it is a correctness guard on a fixture verb, not a sandbox.

---

## Findings

Ordered by severity. Nothing here blocks at `block_on: high`; `threats_open` is **0**.

### F1 — LOW — a step can write another step's output buffer, so a step's body and tokens are not attributable (new surface, `unregistered_flag`)

**File:** `tools/gate.sh:37` (exports `GATE_TMP`), `:137` (buffer path), `:165-166` (replay `cat` +
`keep_fails`), `:179` (pass-line check), `:183` (token extraction).

**What.** `GATE_TMP` is exported and every step runs as the same user in the same directory, with a
**deterministic** buffer name `step.out.<name>`. A step can therefore overwrite a later step's
buffer; the replay prints it, collects its `-- FAIL` lines into `fail.lines` (and so into the job
summary) and tokenises it. Reproduced in probe B1.

**Why it is not worse.** The **verdict** cannot be flipped: `rc.<name>` is written by the driver from
the step's own exit status (`:137-141`) and the pass/fail branch reads that file (`:173`), so a forged
"pass" body for a failing step still yields `gate-step-failed`. And to write the buffer at all, a
contributor must modify a step script — which is already arbitrary code execution inside the gate and
can print any token directly. No unprivileged process can reach the directory: mode 0700, random
suffix, owner-only.

**Why it is still worth recording.** Before the partition, a step's output reached the terminal from
that step's own file descriptors. Now a step's printed body and the tokens it contributes to the
summary line can be produced by a different step, which is an attribution weakening in the artefact
this phase exists to make durable. It has no threat in the register — the phase's own `## Threat
Flags` sections say "None beyond the plan's register" — so it is recorded as the one
`unregistered_flag`.

**Options, none taken here (this audit writes no implementation):** keep the per-step `rc` as the sole
verdict input (already true) and say so in the header; or write each buffer into a per-step
`mktemp -d`; or have the replay re-read the buffer's inode/mtime against the step's start time.

### F2 — LOW — the exclusion ledger's reason is unchecked free text, so a well-formed row plus the matching registry deletion silently removes a requirement from the must-be-covered set

**File:** `tools/check/steps/important-gates.sh:104-121` (format only: two fields, an id shape, ≥ 3
underscore-joined words, not `none`), `:160` (the subtraction), `data/expected/important-gates-excluded.ids:26-36`.

**What.** Probe T4b: appending
`FND-01 reviewed_by_the_owner_and_deferred_to_a_later_phase` to the ledger **and** deleting FND-01's
two registry rows makes the step pass (`gates=106 lines=328`, exit 0). Either half alone is refused
(T4a refuses the two rows as `-unknown-id`; T1 refuses the deletion as `-complete-uncovered`).

**Assessment.** The declared disposition (T-09-09) is implemented exactly as claimed, and the
register states this boundary itself (`09-05-PLAN.md` trust boundaries: "a ledger that can grow
silently is a way to delete the check"). The two-directional refusals, the separate file, the
mandatory reason and the malformed-row refusal all work. What remains is that the gate cannot judge
whether a reason is *true*, and the only stopper for a coherent two-file edit is review. Recorded so
that a later reader prices the ledger's growth as a review responsibility rather than as a
machine-checked one; the reversal (a per-category cap, or an `expected` count for the ledger) is not
proposed here.

### F3 — LOW — three stale pointers inside the security mechanisms' own prose (already recorded as review IN-02 / IN-03, still open on the tree)

**Files and what disagrees with the file:**

| `file:line` | The text | The tree |
|---|---|---|
| `data/expected/important-gates-excluded.ids:24` | "the file holds **seven** rows and emptiness is not legal here" | ten rows (`REL-01`, `API-01`, `API-05`, `API-06`, `RUN-03`, `PKG-02`, `EV-01`..`EV-04`) |
| `tools/check/steps/important-gates.sh:28` | "the **six** requirements it lists then fail `important-gates-complete-uncovered`" | the same ten |
| `tests/state/state_fuzz.c:919` and `data/expected/tests/state.ids:107` | "`c.accepted != 0`, `tests/state/state_fuzz.c:945-948`" | the function opens at `:946` and the refusal is at `:954` |

**Why it matters for this audit.** These are the pointers a reader uses to predict a refusal set and
to locate T-09-05's *substitute* guard. The mechanisms themselves are present and fire (probes T11,
T12 and the code at `state_fuzz.c:954`); what is wrong is the arithmetic and the line number. AGENTS.md
("a quoted figure that disagrees with the file is a defect to correct") makes this a defect rather
than a style note. Both are already recorded as open review items IN-02 and IN-03; this audit does not
re-litigate them, only confirms they are still open on `fd0ef31`.

### F4 — LOW — the manifest output-anchoring rule can be widened by one character per dot in a fixture directory name, and its `data/` root is broader than the measurement required

**File:** `tools/check/manifests.cmake:134-139`.

**What.** `dirname` is captured by `^fixtures/([A-Za-z0-9._-]+)/` (`:169`) — a class that admits `.` —
and is then interpolated into a CMake regex as `"^fixtures/${ARGV3}/"` (`:136`). CMake regex treats `.`
as "any character", so a fixture directory named `FX.good` would anchor to `^fixtures/FX.good/`, which
also matches `fixtures/FXxgood/…`. Separately, the second root is `^data/` rather than the two paths
the comment cites (`data/cart/ivsc-parse.json`, `data/cart/ivsc-load.json`), so a manifest may declare
**any** tracked `data/` file as its own output.

**Impact.** Neither is a gate bypass: the digest and size of a committed output are still verified
against the file (`:330-338`), so a manifest cannot lie about bytes — only about provenance, and only
among in-tree paths that already exist. Both roots are in-tree and the anchoring's job (keeping an
output out of `core/`, `runner/`, `tools/`) is still done. The phase measured and documented the
`data/` choice as deliberate; what is unmeasured is its breadth, and the `.`-wildcard is an
unintended consequence of interpolating a captured name into a regex. Fix, one line either way: escape
the dots (`string(REPLACE "." "\\." d "${dirname}")`) and/or tighten `^data/` to `^data/cart/`.

---

## Categories not applicable

Each of these has **no surface in this project or in this phase**; none was invented as a finding.

| Category | Why it is not applicable |
|---|---|
| CSRF | There is no HTTP server, no cookie, no browser session and no state-changing form anywhere in the tree. The gate is a local/CI shell command; the only "requests" it makes are `git`, `cmake`, `cc` and `ps` invocations. |
| Session fixation / session management | No sessions, no authentication, no login, no user accounts, no tokens. The nearest thing is `IVS_TEST_NONCE`, which is the basename of the run's own `mktemp -d` (`tools/gate.sh:98-104`) and is a freshness marker for results files, not a credential. |
| SQL injection / ORM injection | No database, no SQL, no query layer. Records are markdown tables, `.ids` files and JSON read by `cmake`'s `string(JSON)` and by `awk`. |
| Command injection from untrusted input | Every command word in the changed code is a quoted variable holding a path the script derived itself; the only externally-influenced text is repository content (inventories, records, ledgers), and it reaches `awk`/`grep -F` as *data*. The one pattern-interpolation note is recorded above (pass-line `case`, token `sed`) and requires an in-repo step rename. |
| Path traversal / directory escape | The surfaces that exist are guarded and were already so: `path_ok` refuses `..` and a leading `/` in manifests (`manifests.cmake:88-93`), `apply_make` refuses `..` and absolute `MAKE` paths (`run-controls.sh:100`, `:107`), `tests-inventory` refuses a `..` source path (`tests-inventory.sh:170`), and `sources.sh` only opens paths matched by two anchored FILELIST patterns. The phase adds no new path taken from a report or a record. |
| XSS / template injection | The project renders no HTML and ships no web UI. `ci-summary.sh` writes a markdown job summary; it strips backticks (pinned by the `ci-summary` mutant `backticks-stripped`) and the phase changed nothing in it. |
| SSRF / network egress | The phase makes **no network call at all** — no `curl`, `wget`, `gh`, `npm`, `pip`, `cargo` in the diff (probe P6). The gate's only external commands are `git`, `cmake`, `ninja`, `cc`, `ps`, `date`, `awk`, `sed`, `grep`, `comm`, `sort`, `diff`. |
| Deserialisation of untrusted data | The only structured parse is `string(JSON)` over `fixtures/*/manifest.json`, which is pre-existing and fail-closed (`manifest-parse`, `manifest-unknown-key`); the phase adds one anchoring rule to it and no new parser. `.ids` and ledger files are read line-wise by `awk`. |
| Privilege escalation / sandbox escape | Every command runs as the invoking user; there is no setuid, no capability, no container-escape surface in the changed code. The `tools/gate-linux.sh` container is a *caller*, unchanged by this phase, and the phase leaves no `ivs-` container or volume behind (`09-VERIFICATION.md`). |
| Cryptography misuse | No cryptographic operation is added. `cmake -E sha256sum` and `file(SHA256)` are used only as identity digests over fixtures, which is pre-existing and unchanged in kind. |
| Third-party component risk | No dependency was added and no package manager was invoked (T-09-SC, verified in probe P6). |
| Data exposure / PII | The gate's temp directory holds a file list of the public repository, check output, and timing lines. No personal data, no credentials, no operator content (the operator-content route is `skip-ok` precisely so that owner files never enter the gate — `golden-user-content-not-skip-ok`, unchanged). |

---

## Probes run

All probes were executed in this audit. Repository-modifying probes ran against a scratch copy at
`/tmp/sec09/root` (a copy of `.planning/` and `data/` plus the one step under test); no tracked file
was modified, and `git status --short` after the audit shows only the orchestrator's pre-existing
`.planning/` state changes.

| # | Probe | Result |
|---|---|---|
| **P1** | `ROOT=$PWD CHECK_DIR=$PWD/tools/check FILELIST=<git ls-files> sh tools/check/steps/important-gates.sh` on the real tree | `-- important-gates pass gates=107 lines=330`, exit 0 — the registry resolves, the derived set is satisfied |
| **P2** | The identity arithmetic re-derived independently | Complete 54 − excluded 10 = **44** = 44 distinct registered requirements; the toy tree: 45 − 1 = 44 = 44 rows |
| **P3** | `sh tools/check/steps/runner-selftest.sh` | `-- runner-selftest pass cases=13`, including `proof control-pass-line-not-last: profile (observed …)`, `proof pass-line-not-last: profile (exit 2 observed)` and `proof memmap-generator-missing: cmake/IvsMemmapProbes.cmake (observed)`; the drift and unused-deviation tokens are required by cases 8–9 |
| **P4** | The `_good` positive control: `important-gates` on `controls/_good/tree` with its own filelist | `-- important-gates pass gates=43 lines=44`, exit 0 — the toy derivation reads its records (a vacuous read would make all 44 rows `-unknown-id`) |
| **P5** | `derivation_equal`'s comparison re-implemented independently over the frozen derivation | `demo-lamp-tone-shift` derivation block 65 lines vs scenario body 65 lines → **EQUAL**; `demo-lamp-tone-swap` 58 vs 58 → **EQUAL**; `Counts:` at `docs/video/demo-lamp-tone-1-derivation.md:316` reads 55/29/55/49; the `assert mem 02F1 == 0003` stand-in line exists once in each variant; the derivation document is **unchanged** by the phase (`git diff` empty) |
| **P6** | Rights and secrets sweep over the phase diff | 0 tracked `.bin/.rom/.ivsc/.exec/.grom/.wav/.ppm/.pdf`; `.gitignore` and `tools/check/binary-ext.txt` unchanged; 0 secret-shaped additions; 0 real absolute home paths; 0 non-text added files; 0 `npm/pip/cargo/gem/apt-get/yarn` invocations outside plan prose; 0 dependency files touched |
| **P7** | `mktemp -d "${TMPDIR:-/tmp}/gate.XXXXXX"` behaviour | mode **700**, owner-only, random 6-char suffix; fails **closed** (`rc=1`, no directory) when the parent is missing |
| **P8** | Temp-dir lifecycle: `CC=no-such-compiler sh tools/gate.sh` (early exit through the EXIT trap) | `-- FAIL gate-no-cc`, `gate: FAIL`, `rc=1`; `gate.*` dirs before = after = 1, and the single leftover predates the partition (it holds `step.out`, the pre-partition name) |
| **P9** | `shellcheck -s sh -S warning` + `dash -n` on all nine changed shell files | `dash -n` clean on all nine; five shellcheck warnings, each shown above to be intentional |
| **B1** | Buffer-forgery reproduction: two synthetic steps where `zzz` writes `$GATE_TMP/step.out.aaa`, then the gate's own buffer-and-replay loop | `aaa`'s body replayed as the bytes `zzz` wrote — F1 confirmed, while `rc.aaa` remained the driver's own file (verdict not flippable) |
| **T1** | Delete the two `FND-01` registry rows | `FAIL important-gates-complete-uncovered: FND-01` + `FAIL important-gates-uncovered-gate: FND-01`, `rc=1` |
| **T2/T4a** | Add a row for an id in no inventory; add a ledger row for a still-registered requirement | `FAIL important-gates-unknown-id: …` both ways, `rc=1` |
| **T4b** | Well-formed ledger row for `FND-01` **plus** deletion of its two registry rows | **passes** (`gates=106 lines=328`, `rc=0`) — F2 |
| **T7** | Downgrade `FND-01` to `Partial` in `.planning/milestones/v0.1.0-REQUIREMENTS.md` | `FAIL important-gates-unknown-id: line 404/405`, `rc=1` |
| **T7b** | Downgrade the excluded `EV-01` to `Pending` in `.planning/REQUIREMENTS.md` | `FAIL important-gates-excluded-stale: EV-01`, `rc=1` |
| **T10** | Delete every requirement record (the vacuous-derivation case) | ten `FAIL important-gates-excluded-stale`, `rc=1` |
| **T11/T11b** | Empty the ledger; delete the ledger file | ten `FAIL important-gates-complete-uncovered` each, `rc=1` |
| **T12** | Append a malformed ledger row (`FND-01 none`) | `FAIL important-gates-excluded-malformed: line 37: FND-01 none`, `rc=1` |
| **T13** | Point a registry row at a real but unrelated mechanism | **passes** — the documented write-time-verification boundary, not a gap against the disposition |
| **T-09-06** | Recompute the last-five median with the entry's own documented command | `7.4167 7.5333 7.6957 8.2326 8.3810` → median **7.6957**, exactly what the entry prints; `grep -c '^phaseA1:'` → **1**, `discharged-below-budget` |
| **T-09-08** | `git diff --numstat` over the ten toy registries | `17/0` on all ten, **0** added `#` lines, so the count is the row count |
| **T-09-17** | Archives and bookkeeping | `.planning/milestones/` and `.planning/phases/0*` **unchanged** across the phase; the flip commit `31821b4` carries `REQUIREMENTS.md` and the ledger together |
| **Partition** | Re-derive the preflight class independently over `tools/check/steps/*.sh` | `preflight=29 producers=9 total=38`, producer set identical to the recorded one |

---

## SECURITY COMPLETE

- Threats in the register: **18** (`T-09-01` … `T-09-17`, `T-09-SC`)
- Closed: **18/18** — 16 mitigated, 2 accepted (`T-09-15`, `T-09-SC`)
- `threats_open`: **0** (none at or above `block_on: high`)
- Findings: **high 0 · medium 0 · low 4** (`F1` buffer attribution, `F2` ledger reason, `F3` stale
  pointers, `F4` anchoring breadth)
- Unregistered flags: **1** (`F1`)
- Categories not applicable: **13**, each with its reason
- Probes run: **24** (P1–P9, B1, T1–T13, plus the T-09-06/08/17/partition recomputations)
- Implementation files modified by this audit: **none** — this file is the only write
