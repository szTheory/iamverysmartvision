---
phase: 10-reachable-runs-and-presentation-policy
asvs_level: 1
status: findings
high: 0
medium: 2
low: 4
verdict: secured
threats_total: 23
threats_closed: 23
threats_open: 0
audited: 2026-11-25T12:59:26Z
auditor: Claude (gsd-security-auditor)
tree: cf4a3dc
phase_base: d3952ea
block_on: high
notes: >-
  The register carries 28 T-10 rows across six plans but 23 distinct ids, because T-10-SC
  (no package manager is invoked, nothing is installed) is declared once per plan and is one
  threat. threats_total counts distinct ids. No finding is at or above block_on, so
  threats_open is 0; the two medium findings sit past the edge of a declared mitigation and
  are recorded rather than counted as open threats.
---

# Phase 10 Security Audit: Reachable Runs and Presentation Policy

**Audited tree:** `cf4a3dc` (phase branch; the only later change from the verified `153fd51`
is the append-only `10-VERIFICATION.md` close-out — `git diff --stat 153fd51..HEAD` names
nothing else). Phase base `d3952ea`.
**ASVS level 1** — the depth applied is "the declared mitigation is PRESENT at the site the
plan names, and no greppable bypass of it exists on this tree". Nothing here was accepted
from `10-VERIFICATION.md` or `10-REVIEW.md`; every row below quotes a probe I ran in this
session.
**Adversarial stance:** every mitigation was assumed absent until a probe placed it at a
file:line, and then assumed defeatable until a probe failed to defeat it.

The surface is not a web surface. It is: (a) public prose that must not become a colour
claim, (b) a handoff ledger that a claim must not cite as evidence, (c) a claims predicate
that must not accept a handoff-shaped artifact, and (d) a release-time script that unpacks
an archive and builds it in a temp directory. Threats that this project has no surface for
are listed under **Categories not applicable** rather than invented.

## Threat register verification

`mitigated` = the declared mitigation is present at the cited site and survived a bypass
attempt. `accepted` = the declared disposition is accept and the recording it requires is
present.

| Threat ID | Category | Severity | Disposition | Verdict | Evidence (site and probe) |
|---|---|---|---|---|---|
| T-10-01 | Repudiation | high | mitigate | mitigated | The archive route is executed, not quoted: `sh tools/release-rehearse.sh --no-gate --source-route` → `rehearse: source-archive: equal (5831707 bytes)`, `rehearse: source-route: ivs-v0.1.0-source.tar.gz, golden match (report 131 lines, 124 of 124 compared), prefix ivs-v0.1.0`, `rehearse: ok`, 154 s wall. The golden is named by path at `tools/release-rehearse.sh:192`; extraction and cold build at `:166-181`; the runner is invoked from the extracted prefix at `:184-186`. `git status --porcelain` was unchanged after two full runs. |
| T-10-02 | Repudiation | medium | mitigate | mitigated | `grep -c -F 'nothing downloadable is runnable' README.md docs/packaging.md` → `1` and `1`. `docs/packaging.md:121` is the phrase line and is byte-identical to `origin/main` (`git show origin/main:docs/packaging.md \| grep -F …` returns the same line 121; the only packaging.md hunk in the phase diff is the `release-rehearse` bullet at :111). `README.md:17` keeps its reason clause. The 34-byte figure is wrong — see L-3. |
| T-10-03 | Information disclosure | medium | mitigate | mitigated | `git ls-files \| grep -E '\.(wav\|ppm\|vraw\|bin\|rom)$'` → none. `.gitignore:18,20` hold `*.wav` and `*.ppm`. `git diff --stat origin/main..HEAD -- tools/check/binary-ext.txt .gitignore` → empty, so both refusal lists are unchanged. `tools/check/steps/text.sh:31` filters the FILELIST to `README.md\|CHANGELOG.md\|docs/**/*.md`, so the new `docs/running.md` and `docs/presentation.md` are scanned in every gate. |
| T-10-04 | Repudiation | high | mitigate | mitigated | `grep -c -E '^[1-6]\. ' docs/presentation.md` → `6` (lines 12, 18, 31, 38, 43, 50). `grep -c '\[claim:' docs/presentation.md` → `0`: the document mints nothing. Running the step's own claim-word pipeline over the file (`text.sh:46-50`) returns no untagged line. `git diff --stat origin/main..HEAD -- data/claims.json data/expected/claims.ids` → empty, so nothing was written into the claim ledger. |
| T-10-05 | Tampering | medium | mitigate | mitigated | The cited region is where the document says: `tools/check/core-symbols.sh:171` is `if (tolower(n) ~ /(palette\|rgb\|ppm\|framepreview)/) print n`, `:163-173` is the rule block, `:38-41` is its label block. The named control runs and is green in isolation: `CONTROLS_DIR=… CONTROLS_INVENTORY_DIR=… sh tools/check/run-controls.sh` → `control ok   core-symbols-palette-defined`, `controls=1/1`. |
| T-10-06 | Repudiation | high | mitigate | mitigated | `git merge-base --is-ancestor 51515f1 b5ad01d` → true, i.e. the colour mechanism precedes the policy in the direction history runs. The inversion is recorded in `10-CONTEXT.md:201` (A-02) and stated in the document itself at `docs/presentation.md:81-90`. No sentence in the document claims the policy came first. |
| T-10-07 | Repudiation | high | mitigate | mitigated | The grammar at `tools/check/steps/claims.sh:43-62` admits exactly `handed-off`/`not-performed` and the literal pair `evidence:` + `none`. `data/expected/handoffs.ids` carries no allow-empty directive (its header at `:16-26` refuses emptiness in prose; the reader refuses it in code — see L-2 for the stale idiom). Control `claims-handoff-row` is green. My probes P4/P5/P6 (below) confirm a wrong evidence field, a disposition outside the two, and a fifth token are each refused. A duplicate row is not — see M-1, which sits past this declared mitigation rather than inside it. |
| T-10-08 | Repudiation | high | mitigate | mitigated | `docs/handoffs.md:3-4` states "not a check, not a gate step, not evidence and not citable by a claim"; `:7-12` states that an answer is a result and never a claim. Every identity is a size, a mode or a hash: `:25` (21674 bytes, 0644), `:30` (refpcm.sha256), `:46` and `:67` (45806 bytes, 0600, `P6`/`159 96`/`255`), `:87` (exit 77). `grep -c '\[claim:' docs/handoffs.md` → `0`. |
| T-10-09 | Information disclosure | medium | mitigate | mitigated | No `.wav`, `.ppm` or image is tracked (`git ls-files` probe above). `.gitignore:18,20` and the unchanged `binary-ext.txt` both refuse the extensions by name. `docs/handoffs.md` quotes sizes, modes and one digest line and commits no bytes: the packet names `build/listen/…wav` and a `--frame-ppm` output path, both outside the tree. |
| T-10-10 | Tampering | low | accept | accepted | The relationship is recorded where the disposition says: `data/expected/handoffs.ids:34-38` ("five ids but not five human acts… a later milestone must not book a third act"), and `docs/handoffs.md:7-12` gives the disposition vocabulary no place to hide a pass. No third listening act is bookable. |
| T-10-11 | Repudiation | high | mitigate | mitigated | `10-REHEARSAL-TRANSCRIPT.md:1-11` states what the record is not before any figure ("not a gate step, not a gate result, not evidence for any claim and it carries no tier"). `grep -E 'installed-host-behavior\|source-expectation\|measured-hardware\|\[claim:'` over it → none. It lives under `.planning/`, which `tools/check/steps/text.sh:31` does not scan, so it cannot become public claim text by accident. `claims=28` is unmoved. |
| T-10-12 | Tampering | high | mitigate | mitigated | The copy is *checked*, not trusted: `tools/release-rehearse.sh:100-111`. Probed in three states — unmodified copy → the guard passes and the run proceeds; `route_live` mutated (the `n == 1` clause dropped) → `rehearse: FAIL: route_live has drifted from tools/check/steps/golden.sh (live_count)`; `route_filter` mutated (the scenario/outcome class removed) → `rehearse: FAIL: route_filter has drifted from tools/check/steps/golden.sh (filter_report)`. The liveness half is `[ -n "$_src" ]`; the vacuity half is `[ -s "$work/route/want" ]` at `:190`, which refuses a filter that matched nothing. |
| T-10-13 | Information disclosure | medium | mitigate | mitigated | `mktemp -d` creates the workspace mode `drwx------` (probe), the EXIT/HUP/INT/TERM traps at `tools/release-rehearse.sh:108-110` remove it, and the transcript writes the root as `<work>` (`10-REHEARSAL-TRANSCRIPT.md:44-45`). The archive is produced by `git archive HEAD --prefix="ivs-$tag/"` (`tools/release.sh:170`), which cannot carry `..` or an absolute path (git refuses both: probe), and a hostile tarball extracted with the same `tar -xzf … -C` command leaves `..` refused and a leading `/` stripped (probe). `git status --porcelain` after a full `--source-route` run is unchanged. Two conditional residuals, L-4 and M-2's enabling clause. |
| T-10-14 | Denial of service | low | accept | accepted | Measured, not estimated: `/usr/bin/time -p sh tools/release-rehearse.sh --no-gate --source-route` → `real 154.44  user 25.06  sys 12.19`. `git diff --stat origin/main..HEAD -- .github/` → empty, so no hosted cost is added. The mode is release-time only (`grep -rn 'release-rehearse' tools/ .github/` finds only the script itself and its own usage line; `grep -n rehears tools/gate.sh tools/gate-linux.sh` finds nothing). |
| T-10-15 | Repudiation | high | mitigate | mitigated | The predicate normalises before it matches: `tools/check/kinds/claim.cmake:125-134` (`file(REAL_PATH)` on both ROOT and the artifact, `RELATIVE_PATH`, `TOLOWER`, then five clauses). All 28 live artifacts were re-measured through that same normalisation in Python → **none** matches a refused clause. No claim was silently refused or silently accepted. The committed stand-in is green and covers all five classes plus three spellings: `control ok   claims-handoff-artifact` with eight `EXPECT` lines. Bypass probes refused: `./out/./x.wav`, `OUT/X.WAV`, `data/../build/…`, `https://example.com/x.wav`, `out/*.wav`, and symlinks whose target resolves inside the tree (`out/ev-inside.report`, `out/ev-ledger`). One case is accepted — M-2. |
| T-10-16 | Tampering | high | mitigate | mitigated | `tools/check/steps/claims.sh:35-77`: the ledger is required (`no ledger` at `:37`), readable (`unreadable` at `:41`) and its id set pinned in both directions across `:67-77`, with the pass line last at `:81`. Four committed controls prove each refusal for its own reason, all green in isolation: `control ok   claims-handoff-missing` (`no ledger`), `-unreadable` (`unreadable`), `-id-missing` (`missing id D11`), `-id-extra` (`unknown id X-1`), each with an eight-line `FORBID` naming its siblings. A duplicated id and a wrong disposition for a right id are *not* refused — M-1. |
| T-10-17 | Repudiation | medium | mitigate | mitigated | `tools/check/steps/text.sh:47` gains exactly `looks\|sounds` and nothing else from that family. The two words fire: `A looks-like console.` and `It sounds-like the arcade.` are reported; `Overlooks nothing.` and `Sounding board.` are not, so `-w` is intact. The eight pre-existing words and the `playable-guest-progress` exemption still work (six other `text-*` controls green in the same isolated run, `controls=8/8`), and the committed control `text-untagged-look-listen` fires on both overlay lines. One spelling escapes — L-1. |
| T-10-18 | Denial of service | low | accept | accepted | The three control ids are registered (`data/expected/controls/claims.ids` carries `claims-handoff-artifact`, `claims-handoff-row`, `text-untagged-look-listen`; the review's fix added four more, so `controls` moves 386→393 and nothing else does). No timed loop or build is added; each control is one step invocation on the toy tree. Accepted as declared. |
| T-10-19 | Tampering | high | mitigate | mitigated | The flip and its rows are one commit: `git show --stat b14e1a4` → `.planning/REQUIREMENTS.md`, `data/expected/important-gates-excluded.ids`, `data/expected/important-gates.ids`, `tools/check/steps/important-gates.sh`. The identity recomputed with the step's own awk: **Complete 57 − Excluded 12 = Accepted 45 = Registered 45**, and both set differences are empty (`comm -23` and `comm -13` both print nothing). |
| T-10-20 | Repudiation | high | mitigate | mitigated | Append-only is measured, not asserted: `git show --numstat 4ff8a7c` → `12 0`, `14 0`, `17 0`, `21 0` — zero deleted lines in all four corrected records. |
| T-10-21 | Denial of service | low | accept | accepted | The phase's cost is four local gate runs and one container run, recorded in `10-VERIFICATION.md`. No retry loop exists in the changed scripts; `tools/release-rehearse.sh` calls `fail` (exit 1) on every failed step rather than retrying. |
| T-10-22 | Information disclosure | medium | mitigate | mitigated | Over all 75 files in `git diff --name-only origin/main..HEAD`: no tilde-home prefix, no absolute macOS home prefix, no absolute Linux home prefix, no project temp-dir prefix and no macOS per-user temp prefix, as literals. The two grep hits are not leaks - `10-04-PLAN.md:158` contains a *regex character class* the plan's own verify uses (an absolute-home prefix followed by a character class), and `tools/check/run-controls.sh:132` is a pre-existing control that *generates* a tilde-home scratch path to test hygiene. A secret-shaped scan (`BEGIN ... PRIVATE KEY`, `ghp_`, `github_pat_`, `sk-`, `AKIA`, `xox[baprs]-`) returns no hit inside the phase diff; the single repository-wide hit is a pre-existing 01-REVIEW record describing what that scanner catches. `git diff --stat origin/main..HEAD -- tools/gate.sh tools/gate-linux.sh .github/ CMakeLists.txt CMakePresets.json` -> empty. |
| T-10-SC | Tampering | high | accept | accepted | One id for six rows (`10-01-PLAN.md:194` … `10-06-PLAN.md:276`). No package-manager invocation exists in any changed file — the only grep hits for `(npm\|pip\|cargo\|gem\|brew) (install\|add)` are the six plan rows themselves. `CMakeLists.txt` and `CMakePresets.json` are unchanged, so no dependency was added. `prefix-files=11` recomputes: `data/expected/pkg-install-files.ids` holds exactly 11 rows and none names a `runner/` file. |

## Findings

No finding is at or above `block_on: high`. `threats_open` is 0.

### HIGH

None.

### MEDIUM

#### M-1 — The handoff ledger accepts a duplicated id and a right-id/wrong-disposition row

**File:** `tools/check/steps/claims.sh:67-77` (the row-set reader), `data/expected/handoffs.ids:3` ("One line per item, four space-separated tokens").

**What.** The reader pins the id *set* but neither the row *count* per id nor the id→disposition *map*. `{ if (!($1 in seen)) ord[++nr] = $1; seen[$1] = 1 }` deduplicates on first sight, so a second row for an id is never examined after the grammar pass, and the grammar pass only requires `$2` to be one of the two legal tokens.

Probes on a `git archive HEAD` extract, run through `ROOT=… CHECK_DIR=… sh tools/check/steps/claims.sh`:

```
P1  D11 appended a second time, same disposition        -> -- claims pass  EXIT=0   accepted
P2  D11 appended a second time, OTHER disposition        -> -- claims pass  EXIT=0   accepted
P3  the D10 row's disposition flipped not-performed->handed-off
                                                         -> -- claims pass  EXIT=0   accepted
P4  D10 evidence: measured                               -> -- FAIL claim-handoff-row: line 4  EXIT=1
P5  D10 disposition "done"                               -> -- FAIL claim-handoff-row: line 4  EXIT=1
P6  D10 evidence: none extra                             -> -- FAIL claim-handoff-row: line 4  EXIT=1
```

So of the four shapes worth worrying about, the step refuses a disposition outside the pair and an `evidence:` value that is not `none`, and refuses neither a duplicate id nor a wrong disposition for a right id. P2 is the sharpest: one item can be recorded as both `handed-off` and `not-performed` in the same file, and the step prints a pass line.

**Why it matters.** The ledger exists so that a later milestone cites a row instead of re-asking the owner (`docs/handoffs.md:3-4`, GUARDRAILS §5.7). A duplicate row makes `docs/handoffs.md` — which states each item's disposition in prose at `:7-12`, `:20-30`, `:60-67` — disagree with the ledger while both are individually well-formed, and nothing in the gate compares them. P3 means the ledger can say D10 was "offered with its procedure" when the truth recorded in `docs/handoffs.md:70-87` is that it needs physical hardware no tool here can supply; a milestone that cites the row asks the owner for the wrong act.

**Why this is not an open threat.** The declared mitigations for T-10-07 and T-10-16 are present and do exactly what they say — the grammar admits two dispositions, the literal `evidence: none`, the file is required, and the id set is pinned both ways. Neither plan claimed a per-id row-count or an id→disposition pin. This is a gap past the declared scope, at medium, not a missing mitigation.

**Note on convention.** The project already has the mechanism for the *other* ledger: `tools/check/controls/claims-duplicate-id/EXPECT` is `FAIL claim-duplicate-id: CL-0001` and that control is green in the same isolated run. `data/claims.json` refuses a duplicate id by token; `data/expected/handoffs.ids` does not.

**Suggested action.** Add a duplicate-row refusal to the awk at `:67-77` (the `seen[$1]` map already exists; the missing part is a second row for a known id producing a finding rather than being skipped), and either pin the five id→disposition pairs beside `HANDOFF_IDS` at `:30` or have `docs/handoffs.md`'s prose be generated from the ledger. Prove both with a control in the shape of `claims-handoff-id-extra`.

---

#### M-2 — A claim whose artifact is a symlink resolving outside the repository is accepted

**File:** `tools/check/kinds/claim.cmake:118-121` (the residual, recorded in the token prose), `:125-134` (the normalised predicate that does not catch it).

**What.** The code is honest about this and I am confirming it rather than discovering it:

```
# Residual, named rather than papered over: a symlink whose target lies
# OUTSIDE the repository normalises to a `../`-relative path, which no clause below matches
```

Probe on a copy of `tools/check/controls/_good/tree` with `ln -s /tmp/extbuild/x.report out/ev-outside.report` and a claim whose `artifact` is `out/ev-outside.report` (the target exists, so `claim-dangling-path` cannot fire; `claims.ids` extended so `claim-id-extra` cannot):

```
-- claim pass records=2 claims=2 provisional=0
-- claims pass claims=2 provisional=0                     EXIT=0   accepted
```

The same construction pointing *into* the tree is refused, which isolates the escape to the resolution step and not to the symlink idea:

```
out/ev-inside.report -> ../build/gate/dev/runner/toy.report  -> -- FAIL claim-handoff-artifact: CL-0003
out/ev-ledger        -> ../data/expected/handoffs.ids        -> -- FAIL claim-handoff-artifact: CL-0004
```

**Why it matters.** T-10-15's vector is "a build product, an operator render or a timing record made to look like evidence". An external build directory reached through one committed symlink is exactly that, and `file(RELATIVE_PATH)` reports it as `../../tmp/extbuild/x.report`, which matches none of `^build/`, `[.]wav$`, `[.]ppm$`, `[.]tsv$`, `data/expected/handoffs.ids`.

**The enabling condition, which nothing refuses.** A search of the whole check tree for symlink handling returns three hits and no refusal: `tools/check/kinds/claim.cmake` (the `REAL_PATH` call), `tools/check/run-controls.sh` (the `symlink` MAKE verb, which itself refuses an absolute target or a `..`), and `tools/check/controls/claims-handoff-artifact/MAKE`. No step enumerates tree entry types, `tools/hygiene.sh` scans contents rather than modes, and `tools/check/release-assets.sh:166` lists archive members by *name* only (`tar -tzf`), so a symlink member passes its path and prefix rules. `git ls-files -s | awk '$1=="120000"'` is empty on this tree — the property is true today and is pinned by nothing.

**What a lexical predicate fundamentally cannot close.** `^build/`, `[.]wav$`, `[.]ppm$`, `[.]tsv$` and one `STREQUAL` are predicates over a *string*. Normalising first (which the fix did) converts the string into a canonical in-tree path, and that closes `./`, `//`, `..`, case and in-tree symlinks — I verified all five are refused. It cannot close a path whose canonical form leaves the repository, because "is this file inside the tree" is not a property of the path at all; it is a property of the filesystem. A predicate can only refuse `^[.][.]/` *after* normalisation, which is a different and weaker rule ("the artifact must resolve under ROOT") than "the artifact must not be a build product". The correct closure is structural: refuse the symlink at the point it enters the tree (`git ls-files -s` type check or a `find -type l` in an existing step), or refuse any normalised artifact beginning `../` at `claim.cmake:129`. Note also that `^[.][.]/` alone would not be enough for A→B→outside chains unless the resolution is repeated, which `REAL_PATH` does do.

**Why this is not an open threat.** The declared T-10-15 mitigation — "one `MATCHES` group refuses the five handoff-shaped classes by its own token" — is present and effective for every artifact that resolves inside the repository, which is every artifact the 28 live claims name (all re-measured, none matches). The out-of-repo target is not one of the five declared classes; it is a sixth shape the phase found after the fact and recorded. Medium, not high: it requires a committed symlink whose target is outside the tree, which is a one-line, reviewable tree change, and `claim-dangling-path` already refuses the same claim once the external target is gone.

**Suggested action.** Prefer the structural refusal (a `find -type l` or `git ls-files -s` type check in an existing step) over widening the lexical predicate; if the predicate is widened, refuse a normalised path starting `../` at `:129` and add the case to `claims-handoff-artifact`'s overlay, which already has the run-time `symlink` MAKE verb for the in-tree direction.

### LOW

#### L-1 — `looks_like` escapes the claim-word boundary

**File:** `tools/check/steps/text.sh:47`.

`-w` treats `_` as a word constituent, so `looks_like` and `sounds_like` are not word-boundary matches and pass. Measured on a probe file through the step's own pipeline:

```
caught:      A looks-like console. / It sounds-like the arcade. / The picture looks  like … /
             The tone looks<U+00A0>like … / Looks right to me. / It SOUNDS right.
not caught:  This looks_like a console.
not caught (recorded scope, review I-01): The sound is right. / The picture looked right.
```

Severity low rather than medium because the escaping spelling is a code-identifier form that does not occur in this project's prose, and because the same word still needs a `[nc]` or a claim tag when written the way prose writes it. The recorded scope decision (I-01) covers the noun/other-tense forms and is not re-reported here.

**Suggested action.** If the family is widened, use `(^|[^[:alnum:]])looks([^[:alnum:]]|$)` rather than `-w`, and add `looks_like` to the committed control's overlay.

#### L-2 — The ledger's header contradicts its own reader

**File:** `data/expected/handoffs.ids:16-18` versus `tools/check/steps/claims.sh:35-42`.

The file says "A MISSING file means an empty set, not a missing inventory… its reader must guard on existence with `if(EXISTS ...)`". The reader does the opposite, correctly: `[ ! -e "$hled" ]` → `-- FAIL claim-handoff-row: no ledger` (exit 1), because B-01's fix made the ledger required. The semantics the header describes are the pre-fix semantics; the idiom it names (`if(EXISTS ...)`, a CMake call) is also not what the reader is (a POSIX `[ -e ]`/`[ -f ]` pair). `10-VERIFICATION.md` calls this cosmetic; it is low but not cosmetic, because this header is the first thing a future editor of the ledger reads, and it currently licenses deleting the file. The enforcement is right and the prose is wrong; the prose is the part that will be believed.

**Suggested action.** Replace `:16-18` with the semantics the step implements and name the step rather than a CMake idiom.

#### L-3 — The threat model's phrase figure is 34 bytes; the phrase is 32

**File:** `10-01-PLAN.md:192` (T-10-02's mitigation plan).

`printf 'nothing downloadable is runnable' | wc -c` → **32**. The mitigation itself is sound — the check is `grep -c -F` on the exact substring, which cannot drift with a byte count — so this is a quoted-figure defect in a register row, not a check defect. Reported because AGENTS.md makes "a quoted figure that disagrees with the file" a defect rather than a fact to reuse, and because the figure is the only quantitative claim in that row.

**Suggested action.** Correct the figure, or replace it with "the exact 32-byte substring" derived from the grep.

#### L-4 — `TMPDIR` pointing inside the repository puts the extraction workspace in the tree

**File:** `tools/release-rehearse.sh:108` — `work=$(mktemp -d "${TMPDIR:-/tmp}/release-rehearse.XXXXXX")`.

The header at `:37` states unconditionally that the mode "writes nothing into the repository", and T-10-13's mitigation repeats it. That is true for the default `TMPDIR` and true for any `TMPDIR` outside the tree, which is every normal invocation — I confirmed it by running the mode and finding `git status --porcelain` unchanged. It is false if an operator runs the rehearsal with `TMPDIR` set to a path inside the repository: the extraction, the cold `cmake --preset release` build and the report all land there. `mktemp -d` itself is sound (mode `drwx------`, verified). Severity low: the condition is operator-controlled and self-inflicted, the workspace is removed by the trap at `:109-110`, and any leftover is caught by the `hygiene --tree` step on the next gate run.

**Suggested action.** Either assert `case $work in "$ROOT"/*) fail … ;; esac` after `mktemp`, or soften the header's absolute sentence to name the default.

## Categories not applicable

These were considered and rejected as surfaces this project does not have. Naming them is the point: a STRIDE table that fills every box for a release-time shell script teaches a reader to skip the column.

| Category | Why not applicable here |
|---|---|
| **Spoofing** | No authentication, no session, no identity, no credentials, no network listener anywhere in the phase or in the changed files. The rehearsal runs as the invoking user in a local temp directory. The only "identity" in play is a commit hash and a git author, both of which the project already pins. |
| **Elevation of privilege** | No privilege boundary is crossed. The rehearsal creates files as the invoking user inside a `mktemp -d` it owns (mode 0700, verified) and installs nothing — `prefix-files=11` and no runner file in the install list. `tar -xzf` does not preserve setuid (verified: mode `4755` in a hostile member extracts as `0755`). The threats the *release* path could raise are T-10-SC's and T-10-13's, both dispositioned. |
| **Information disclosure — network / secret exfiltration** | Nothing in the phase opens a socket, fetches a URL, or reads a secret. `.github/` is unchanged, so no workflow gained a network step. T-10-22 is the applicable disclosure threat and it is about text leaking a host path into a public record, not about a secret leaving. |
| **Denial of service — remote or untrusted-input** | The only untrusted-input surface the phase adds is markdown a maintainer writes, which is read by `grep`/`awk` in bounded passes. The `.tsv`, `..` and glob probes show the claims reader rejects rather than expands. T-10-14, T-10-18 and T-10-21 cover the only DoS shape that exists here — local cost — and all three are accepted with measurements. |
| **Tampering — supply chain / package manager** | The project installs no packages at any point. T-10-SC is declared in all six plans, accepted, and I verified no invocation exists and neither `CMakeLists.txt` nor `CMakePresets.json` moved. |
| **Colour as a hardware claim (this project's own, non-STRIDE category)** | Not a generic web category, and it is the phase's central risk, so it is verified rather than dismissed — see T-10-03, T-10-04 and the palette sweep in **Probes run**. |

## Probes run

Every row was executed in this session on `cf4a3dc`. Where a probe needed a mutation, it was made on a `git archive HEAD` extract under `/tmp`, never on the working tree.

**Rehearsal (T-10-01, T-10-12, T-10-13, T-10-14)**

| # | Probe | Result |
|---|---|---|
| 1 | `sh tools/release-rehearse.sh --no-gate --source-route` twice, `/usr/bin/time -p` | `real 154.44`; `source-archive: equal (5831707 bytes)`; `source-route: ivs-v0.1.0-source.tar.gz, golden match (report 131 lines, 124 of 124 compared), prefix ivs-v0.1.0`; `rehearse: ok` |
| 2 | `git status --porcelain` before and after both runs | unchanged — only the orchestrator's `.planning/ROADMAP.md`, `STATE.md`, `state.json` |
| 3 | `sed` the two copied functions out of `tools/release-rehearse.sh` and out of `tools/check/steps/golden.sh`, rename, `diff` | bodies identical (906 bytes each); only the concatenation order differs |
| 4 | mutate `route_live` in a scratch skeleton (`$ROOT/tools/release-rehearse.sh` + `$ROOT/tools/check/steps/golden.sh`), drop the `n == 1` clause | `rehearse: FAIL: route_live has drifted from tools/check/steps/golden.sh (live_count)` |
| 5 | mutate `route_filter` in the same skeleton, remove the scenario/outcome class | `rehearse: FAIL: route_filter has drifted from tools/check/steps/golden.sh (filter_report)` |
| 6 | unmodified copy in the same skeleton | guard passes, run proceeds to `dry run 1` |
| 7 | remove `golden.sh` from the skeleton | `sed: … No such file` then `rehearse: FAIL: route_filter has drifted` — the `[ -n "$_src" ]` liveness half fails closed |
| 8 | `mktemp -d "${TMPDIR:-/tmp}/probe.XXXXXX"; ls -ld` | `drwx------` (0700) |
| 9 | hostile tarball (absolute member, `p/../../escaped-probe`, `p/link-out -> /tmp/…`, mode `4755`) extracted with `tar -xzf … -C dest` | `Removing leading '/' from member names`; `p/../../escaped-probe: Path contains '..'`; tar exit 1; nothing written outside `dest`; setuid not preserved (`-rwxr-xr-x`) |
| 10 | `git update-index --cacheinfo … '../escape'` and `'/abs'` | `error: Invalid path` for both — `git archive` cannot produce them |
| 11 | `git ls-files -s \| awk '$1=="120000"'` | empty — no tracked symlink today |
| 12 | `grep -rn 'release-rehearse' tools/ .github/`; `grep -n rehears tools/gate.sh tools/gate-linux.sh` | only the script's own usage/comment lines; nothing in a gate or a workflow |

**Claims predicate (T-10-15, T-10-07, T-10-16, M-2)**

| # | Probe | Result |
|---|---|---|
| 13 | all 28 live `artifact` values re-run through the same normalisation in Python (`realpath`, `relpath`, `lower`) | `artifacts measured: 28`, `matching a refused clause: none` |
| 14 | scratch `_good/tree` + claims: `./out/./x.wav`, `OUT/X.WAV`, `out/x.ppm`, `data/../build/…`, `https://example.com/x.wav`, `out/*.wav` | every one refused: `claim-handoff-artifact` for the four path shapes, `claim-dangling-path` for `..`, the URL and the glob (and both tokens for the `..` case) |
| 15 | the same tree + `ln -s ../build/gate/dev/runner/toy.report out/ev-inside.report` and `ln -s ../data/expected/handoffs.ids out/ev-ledger` | `claim-handoff-artifact: CL-0003`, `: CL-0004` — inside-tree symlinks are closed |
| 16 | the same tree + `ln -s /tmp/extbuild/x.report out/ev-outside.report` (target exists) | `-- claims pass claims=2 provisional=0`, **exit 0** — accepted; M-2 |
| 17 | `data/expected/handoffs.ids` mutated six ways on an extract (P1–P6 above) | duplicate id and wrong-disposition accepted; wrong `evidence:`, wrong disposition token and a fifth field refused |
| 18 | eight controls in isolation (`CONTROLS_DIR`/`CONTROLS_INVENTORY_DIR` filtered to `_good`, `claims-duplicate-id`, the five `claims-handoff-*`, `text-untagged-look-listen`) | `controls=8/8`, every one `control ok` |
| 19 | `core-symbols-palette-defined` alone in the same harness | `control ok   core-symbols-palette-defined`, `controls=1/1` |
| 20 | `sed -n '38,41p;163,173p' tools/check/core-symbols.sh` | the label block, the rule block and `:171`'s match line are where the document cites them |
| 21 | `git diff --stat origin/main..HEAD -- data/claims.json data/expected/claims.ids` | empty — the claim ledger is untouched |

**Claim words (T-10-17, L-1)**

| # | Probe | Result |
|---|---|---|
| 22 | the step's own pipeline over a 12-line probe file | caught 1,2,3,4,5,6 (`looks-like`, `sounds-like`, `looks  like`, `looks<U+00A0>like`, `Looks`, `SOUNDS`); not caught `looks_like`, `lookslike`, `Overlooks`, `Sounding`, `The sound is right.`, `The picture looked right.` |
| 23 | `cat tools/check/controls/text-untagged-look-listen/EXPECT` and its overlay | two overlay lines, two `EXPECT` lines, control green |
| 24 | `grep -c -E '^[1-6]\. ' docs/presentation.md` | `6` |

**Policy cannot become a colour claim (T-10-03, T-10-04)**

| # | Probe | Result |
|---|---|---|
| 25 | `grep -c '\[claim:' docs/presentation.md docs/handoffs.md` | `0` and `0` |
| 26 | the claim-word pipeline over `docs/presentation.md` alone | no untagged line — `looks like` at `:44` and `sounds like` at `:45` each carry `[nc]` on their own line |
| 27 | `grep -rn 'palette' runner/*.c runner/*.h` | exactly one array definition, `runner/frame_export.c:12` (`static const uint8_t fp_palette[…][3]`); the other hits are the accessor `:42`, its uses `:47`/`:97`, and prose |
| 28 | `grep -rln 'palette' data/golden/` | no file matches |
| 29 | `awk` over `data/expected/pkg-install-files.ids` | 11 rows, **0** naming a `runner/` file |
| 30 | `grep -rl` for the palette's RGB triples (`{0, 0, 0}`, `34, 34, 170`, `255, 0, 255`, `221, 221, 51`) and a spaceless form under `data/` | no file matches — the palette is in no golden and no digest |
| 31 | `grep -c '\.ppm' data/claims.json` | `0` |
| 32 | `grep -ci palette data/claims.json` and a per-claim JSON dump | the two hits are negations — CL-0018 "palette, scaling and aspect are absent from the core", CL-0023 "no palette is claimed" — i.e. the palette is named only to say it is not claimed |
| 33 | `grep -n 'not STIC colour data' runner/frame_export.h runner/main.c` | `frame_export.h:4` and `main.c:113`, as clause 2 says |

**Register-wide**

| # | Probe | Result |
|---|---|---|
| 34 | `grep -c -F 'nothing downloadable is runnable' README.md docs/packaging.md`; `printf … \| wc -c` | `1`, `1`; **32 bytes** (the plan says 34 — L-3) |
| 35 | `git merge-base --is-ancestor 51515f1 b5ad01d` | true — mechanism before policy |
| 36 | identity recomputed with the step's own awk over `REQUIREMENTS.md` and the milestone records | Complete 57, Excluded 12, Accepted 45, Registered 45; `comm -23` and `comm -13` both empty |
| 37 | `git show --stat b14e1a4` | the flip and both inventories in one commit |
| 38 | `git show --numstat 4ff8a7c` | `12 0 / 14 0 / 17 0 / 21 0` — zero deleted lines |
| 39 | `git ls-files \| grep -E '\.(bin\|rom\|ivsc\|wav\|ppm\|vraw\|int\|itv\|gz\|tar)$'` | none |
| 40 | `git diff --stat origin/main..HEAD -- tools/check/binary-ext.txt .gitignore` | empty |
| 41 | `git diff --stat origin/main..HEAD -- .github/ CMakeLists.txt CMakePresets.json tools/gate.sh tools/gate-linux.sh` | empty |
| 42 | a grep for the four home-path shapes (tilde-home, absolute macOS home, absolute Linux home, macOS per-user temp) over the 75 changed files | two hits, neither a leak: a regex character class in `10-04-PLAN.md:158`, a pre-existing control generator at `tools/check/run-controls.sh:132` |
| 43 | secret-shaped `git grep` (private-key headers, `ghp_`, `github_pat_`, `sk-`, `AKIA`, `xox[baprs]-`) | no hit in the phase diff |
| 44 | `grep -rn -E 'symbolic\|symlink\|-type l\|readlink\|120000'` over `tools/check/**` | three hits, no refusal — the enabling condition of M-2 |
| 45 | `git ls-tree -r --name-only HEAD tools/check/steps/ \| grep -c '\.sh$'`; `git diff --stat origin/main..HEAD -- data/expected/gate-steps/` | `38`; empty — no new gate step |
| 46 | `grep -n 'EV-05\|TOOL-04\|PKG-05' data/expected/important-gates.ids`; excluded rows | three registry rows at `:469-471`; 12 exclusion rows |
| 47 | `grep -E 'installed-host-behavior\|source-expectation\|measured-hardware\|\[claim:' 10-REHEARSAL-TRANSCRIPT.md` | none — the transcript carries no tier and no claim reference |

**Nothing was written outside this record.** Every mutation was made on a `/tmp` extract or a `mktemp -d` skeleton; the working tree shows only the orchestrator's three pre-existing modifications.

## Unfixed review items carried, not re-reported

`10-REVIEW.md`'s W-04 (the nine-line tail is a hand-quoted figure), W-05 (the golden's excluded-key enumeration is incomplete) and I-01 (the two-word look/listen list) remain open items in `10-VERIFICATION.md`. They are accuracy defects in prose, not security findings, and none of them creates a path past a declared mitigation; I-01's narrow list is re-confirmed above (probe 22) and L-1 is its one new lexical shape.

## SECURITY COMPLETE

**23 threats verified, 23 closed, 0 open** (high 0, medium 0 blocking, low 0 blocking).
Findings: **0 high, 2 medium, 4 low.** Verdict: **secured** — every declared mitigation is
present at the site its plan names, the two medium findings sit past the edge of what those
mitigations declared, and nothing on this tree is at or above the `high` blocking threshold.
