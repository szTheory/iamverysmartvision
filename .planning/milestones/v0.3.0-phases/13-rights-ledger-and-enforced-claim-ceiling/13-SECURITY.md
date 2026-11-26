---
phase: "13"
slug: "rights-ledger-and-enforced-claim-ceiling"
status: secured
# threats_open = count of OPEN threats at or above workflow.security_block_on severity (high).
threats_open: 0
asvs_level: 1
created: "2026-11-25"
audited: "2026-11-25T23:33Z"
auditor: "gsd-security-auditor (pi, subagent)"
---

# Phase 13 -- Security

> Per-phase security contract: threat register, accepted risks, and audit trail.

**Verdict: SECURED -- 27 register rows across five plans, 21 distinct threat ids, 21 closed,
0 open.** `block_on: high`, `asvs_level: 1`.

## Basis of this audit

This phase has no network, auth, session, crypto or new file-read surface. Its security property
is **inverted**: the risk is a control that can be satisfied vacuously, or a refusal that is
silently not applied. Every mitigation below was therefore checked by making the control fail,
not by reading the plan. Evidence classes, used consistently in the register:

- **code** -- `file:line` where the refusal is emitted.
- **live** -- a command run in this session against the committed tree or a copy of the pristine
  toy tree (`tools/check/controls/_good/tree`, copied to `/tmp`; **no source was modified**).
- **control** -- the committed stand-in that makes the refusal fire for its own reasons, confirmed
  by `control ok <name>` inside `controls=401/401`.
- **review** -- a measurement already taken by `13-REVIEW.md`, cited rather than re-asserted when
  re-running it was not cheap. The two measurements that mattered most were re-run anyway
  (boundary-class liveness, and the CR-01/WR-01 fixes).

Final state measurement, one command:

```
sh tools/gate.sh
-> gate: PASS steps=38/38 controls=401/401 ... faults=238/238 ... goldens=29 ...
  rights=10 rights-cleared=1 rights-pinned=4/4 gates=114 lines=352 claims=28 provisional=27
  sources=13 refs-verified=3/4 tracked=2725 binaries-listed=0   (gate-time total=116)
```

Per-step lines used as evidence: `-- manifests pass manifests=11 effective-yes=8 effective-no=3
rights=10 rights-cleared=1`; `-- files pass tracked=2725 binaries-listed=0 rights-pinned=4/4`;
`-- claims pass claims=28 provisional=27`; `-- important-gates pass gates=114 lines=352`;
`-- runner-selftest pass cases=14`; `-- sources pass sources=13 refs-verified=3/4`;
`-- ci-log pass ci-runs=97`.

## Trust Boundaries

| Boundary | Description | Data Crossing |
|----------|-------------|---------------|
| data file -> checker | `data/rights/*.json`, `data/sources.json`, `data/expected/*.ids` and `data/claims.json` are parsed input; a misspelled key, a wrong type or an unlisted file must be refused, never ignored | third-party-content dispositions, source citations, inventory baselines |
| control overlay -> step | `tools/check/controls/**` is test data copied over a pristine tree; its planted defects must be the only reason the step fails | planted refusals, toy registries |
| data-declared token -> tracked path | the refused spelling set is data; the scope is a path predicate. A too-wide match refuses the core, a too-narrow one refuses nothing | refused spellings, six operator pages |
| pin file -> scan result | `data/expected/rights-pinned.ids` is the only thing that suppresses a naming finding | `(token, path)` suppression pairs |
| claim row -> profile field tag | a claim's tier comes from `data/profiles/*.json`, read once per profile and cached | evidence tiers, claim standing |
| registry row -> inventory id | a registry row is accepted only when the id it names exists in an inventory of the mechanism kind it declares | gate names, requirement ids |
| hosted run record -> cost decision | the checkpoint arithmetic reads committed rows; a fabricated or duplicated row would certify a number nobody measured | `.planning/ci-runs.tsv` |

---

## Threat Register

*27 register rows across the five plans resolve to 21 distinct ids: `T-13-SC` is carried by all
five plans (one shared threat, verified once), and `T-13-16` is unused -- commit `4cc2114`
renumbered 13-05's two rows to `T-13-20`/`T-13-21` after the phase's highest in-use id.*

| Threat ID | Plan(s) | Category | Component | Severity | Disposition | Mitigation | Evidence | Status |
|-----------|---------|----------|-----------|----------|-------------|------------|----------|--------|
| T-13-01 | 13-01 | Tampering | `rights_check` key/type check | high | mitigate | exact key set with per-key types; missing -> `rights-missing-key`, misspelled -> `rights-unknown-key`, absent key short-circuits the value rules | code `tools/check/rights-lib.cmake:62-88` (`gate_fail` at `:74`, `:77`, `:84`; the short-circuit at `:174-176`); **live** M1: renaming `disposition`->`dispositon` on the toy tree prints **both** `-- FAIL rights-missing-key: toy-owner-image/disposition` and `-- FAIL rights-unknown-key: toy-owner-image/dispositon` (the unknown-key direction is closed, not just the missing one); control `rights-ledger` FORBIDs `rights-parse`/`rights-disposition` so a checker that does not short-circuit fails it | closed |
| T-13-02 | 13-01 | Repudiation | a `cleared` row with no resolvable source | high | mitigate | `rights-source-missing` for a null source on a `cleared` row; `rights-source-unresolved` for a non-member id, resolved by membership in `data/sources.json` and never the record's own `rights` field | code `rights-lib.cmake:221-235` (`gate_fail` at `:227`, `:232`); sources read once at `:94-118` and fail closed to the empty set; **live** M2a -> `-- FAIL rights-source-missing: toy-reference-doc`; M2b (`SRC-not-registered`) -> `-- FAIL rights-source-unresolved: toy-reference-doc/SRC-not-registered`; live path exercised on the real tree by `libretro-abi` -> `rights=10 rights-cleared=1`; control `rights-ledger` EXPECTs `FAIL rights-source-missing: toy-cleared` | closed |
| T-13-03 | 13-01 | Tampering | inventory set equality | medium | mitigate | `read_ids` plus both directions, so a deleted row and a deleted inventory line are red either way | code `rights-lib.cmake:236-259` (`:252` missing, `:257` extra); **live** M3 (inventory line deleted) -> `-- FAIL rights-id-extra: toy-owner-image`; M3b (inventory line with no row) -> `-- FAIL rights-id-missing: toy-ghost`; control `rights-ledger` EXPECTs `FAIL rights-id-extra: toy-stray` and FORBIDs `rights-id-missing` | closed |
| T-13-04 | 13-01 | Repudiation | the operator table drifting from the ledger | medium | mitigate | `rights-doc-row` keys on each row's `item` phrase verbatim, never on `id`, so the page never has to spell a refused token and a row with no operator row fails | code `rights-lib.cmake:262-279` (`gate_fail` at `:273`, `:278`); **live** M5 (item phrase deleted from the toy `docs/RIGHTS.md`) -> `-- FAIL rights-doc-row: toy-reference-doc`; live tree: all 10 `item` phrases present in `docs/RIGHTS.md`; control `rights-ledger` EXPECTs `FAIL rights-doc-row: toy-cleared` | closed |
| T-13-05 | 13-01 | Information disclosure | the new source record's citation | medium | mitigate | `url`/`accessed` come from a bounded fetch, recorded as observed; a 404 of a wrong URL is recorded too; the note says why nothing is hashed and that nothing is vendored; no fetch => `A7 open` in the SUMMARY | `data/sources.json` `SRC-libretro-api-docs`: `url` = `https://docs.libretro.com/development/cores/developing-cores/`, `accessed` `2026-11-25`, `capture` `reference-only`, `rights` `unknown`, `authority_tier` `tier5-emulator-hint`, `sha256`/`size_bytes`/`pages`/`local_path` null; note carries the licence observation as an observation and says the ledger row is the disposition; `13-01-SUMMARY.md:160-169` (section "A7 discharged") records the URL that answered, the 81,258-byte 200, and the `404` observed first at the wrong path | closed |
| T-13-06 | 13-03 | Tampering | the bounded token match | high | mitigate | boundary class `[^A-Za-z0-9_-]` on both sides with the token's dot escaped; never a substring and never a word-oriented match | code `rights-lib.cmake:395-396` (escape, then the bounded `REGEX MATCH`); **live** on the toy tree -- `toy image`, `toy_image`, `foo-toy-image`, `toy-imagex`, `xtoy-image` all leave `files pass`, while `a toy-image b` prints `-- FAIL rights-token: toy-image runner/a.c`; **live liveness of the class**: a copy of `tools/check` patched to a plain substring match run against the real `git ls-files` FILELIST goes red with exactly the three findings the review measured (`-- FAIL rights-token: .cfg tools/release.sh`, `... .cfg tools/check/steps/pkg-install.sh`, `... bank-switch docs/RIGHTS.md`), so a regression to substring matching is caught, not assumed; control `rights-naming` plants `FAIL rights-token: toy-image runner/planted_open.c` | closed |
| T-13-07 | 13-03 | Tampering | `data/expected/rights-pinned.ids` rotting into a wildcard | high | mitigate | two-field grammar (`rights-pinned-malformed`) plus the reverse direction as a failure (`rights-pinned-stale`); a pin is compared in one canonical spelling | code `rights-lib.cmake:414-439` (`:426` malformed, `:433` stale, `:438` unpinned); **live** M7 `* runner/a.c` -> `-- FAIL rights-pinned-stale: * runner/a.c` (a wildcard is two fields, never matches, and is refused); M7b three-field pin -> `-- FAIL rights-pinned-malformed`; M7c pin naming a path with no occurrence -> `-- FAIL rights-pinned-stale: toy-image tools/gone.sh`; control `rights-naming` EXPECTs `FAIL rights-pinned-stale: grom-image tools/nowhere.sh`; real tree `rights-pinned=4/4` | closed |
| T-13-08 | 13-03 | Elevation of privilege | scope widening by a glob | medium | mitigate | scope is three path prefixes plus six **exact** page paths, never `docs/**`; the controls prefix is excluded with the reason recorded | code `rights-lib.cmake:352-380`: `set(pages "README.md" "docs/running.md" "docs/owner-images.md" "docs/packaging.md" "docs/diagnostics.md" "docs/RIGHTS.md")` compared with `STREQUAL`, plus `^(core/src/\|runner/\|tools/)` and `NOT p MATCHES "^tools/check/controls/"`; the header states why a glob is refused (`:341-346`); `grep -n 'docs/\*\*' tools/check/rights-lib.cmake tools/check/files.cmake` -> no match | closed |
| T-13-09 | 13-03 | Repudiation | a claim rule that can pass vacuously | high | mitigate | rule (a) sits before the artifact check and before every early `return()`; rule (b) is reachable because the schema already refuses an established claim with no `depends_on`; two must-fail controls plus a legal positive that forbids both tokens | code `tools/check/kinds/claim.cmake:103-108` (condition (a) sits before the artifact check at `:110-150` and before every early `return()` at `:152`, `:158`, `:163`, `:167`), `:180-187` (condition (b)); **live** on the toy tree -- `depends_on: []` -> `-- FAIL claim-blank: CL-0001/depends_on`, absent `depends_on` -> `-- FAIL claim-missing: CL-0001/depends_on` (so (b)'s loop is always reached for an established row); controls `claim-established-non-source` (EXPECT `FAIL claim-established-non-source: CL-0001`), `claim-established-without-tier1` (EXPECT `FAIL claim-established-without-tier1: CL-0001/model`), and `claim-established-positive` (EXPECT an unrelated `claim-dangling-path`, FORBID both new tokens -- the honest positive shape, since `ctl_one` refuses `control-exit-zero`) | closed |
| T-13-10 | 13-03 | Tampering | the live ledger moved to make a test pass | medium | mitigate | the positive is a toy row; the live ledger's counts are the criterion that catches an edit to it | **live** `-- claims pass claims=28 provisional=27` on the real tree, and the toy positive overlays the **toy** `data/claims.json` (1 row) only; `git diff` of the phase range shows `data/claims.json` unchanged | closed |
| T-13-11 | 13-04 | Repudiation | a registry row resolving only because the id was hard-coded | high | mitigate | the four `mutant` ids come from a committed inventory the selftest must actually run; an inventory name the step does not run is refused | code `tools/check/steps/runner-selftest.sh:135` (`bad runner-selftest-missing "$tok"`) with the accepting half at `:241-260`; inventory `data/expected/runner-selftest.ids` names `st-toy-drift st-deviation-unused st-pass-line st-memmap-generator-missing`, and the registry rows in `data/expected/important-gates.ids:528-530,534` name the same four; **live** the step prints its own stand-in proof `-- runner-selftest proof runner-selftest-missing: st-no-such-case (observed: -- FAIL runner-selftest-missing: st-no-such-case)` then `-- runner-selftest pass cases=14`; **review** renaming `st-toy-drift` produced `-- FAIL runner-selftest-missing: st-toy-drift-renamed` and no pass line (file restored, `git diff` clean) | closed |
| T-13-12 | 13-04 | Tampering | a `control` row with no directory of that name | medium | mitigate | `run-controls.sh` compares the directory set and the inventory set in both directions | code `tools/check/run-controls.sh:297-298` (`comm -13` -> `control-not-in-inventory`, `comm -23` -> `control-inventory-orphan`); **live** removing `rights-ledger` from a copied `data/expected/controls/rights.ids` -> `-- FAIL control-not-in-inventory: rights-ledger`; `control ok rights-ledger` on the unmodified corpus; the reverse direction (`comm -23` -> `control-inventory-orphan`) is the same two-line predicate read the other way | closed |
| T-13-13 | 13-04 | Repudiation | a row naming a requirement outside the derived accepted set | high | mitigate | the EV-07 flip lands in the same commit as the first registration row, and `important-gates-unknown-id` is the finding that catches a split | `git show --stat 2c32390` carries `.planning/REQUIREMENTS.md` (2 changed lines, the checkbox + traceability flip) **and** the first registration rows; `.planning/REQUIREMENTS.md:104-107` now reads `RTS-01/RTS-02/EV-06/EV-07 ... Complete`; `data/expected/important-gates.ids:491,499,507-509,525-534` names RTS-01, RTS-02, EV-06 and EV-07; **live** `-- important-gates pass gates=114 lines=352` with `rows-pending=0 policies-pending=0` in the gate line | closed |
| T-13-14 | 13-04 | Tampering | toy coverage silently deleted | medium | mitigate | one appended line per file across the toy registries, zero deletions, no `DEVIATION`; `control-toy-drift` and `control-deviation-unused` are both refused and both directions are checked | `git show --numstat 2c32390` -> ten `overlay/data/expected/important-gates.ids` files at `+1 -0`, the toy registry at `+1 -0`, `data/expected/controls/important-gates.ids` `+5 -0`; `git diff --numstat 9362fda~1..HEAD -- tools/check/controls/_good/tree/data/expected/` -> `+1 -0`, `+1 -0`, `+3 -0`, zero deletions anywhere in a registry (`b1abccc`'s `3 -3` on `data/expected/important-gates.ids` are three **comment** lines); the two new phase controls carry no `DEVIATION`; **live** undeclared overlay difference -> `-- FAIL control-toy-drift: ...`, and an unused declaration -> `-- FAIL control-deviation-unused: ...` (`controls=400/401`) | closed |
| T-13-15 | 13-04 | Repudiation | a quoted, stale baseline | medium | mitigate | `gates=`/`lines=` are the step's own runtime stats, and the criterion is that printed line plus arithmetic over the committed file; the deviation from RESEARCH's under-counted prediction is recorded | **live** `-- important-gates pass gates=114 lines=352`; arithmetic over the committed file: `awk '!/^#/ && NF' data/expected/important-gates.ids \| wc -l` -> **352**, equal to the printed stat; the derivation (113 + 1 new gate name, 342 + 10 rows, RESEARCH's 347 having omitted the five RTS/EV rows) is recorded in `.planning/STATE.md` `[Phase 13]` | closed |
| T-13-17 | 13-02 | Repudiation | a question the milestone will ask with no written disposition | high | mitigate | the nine rows are the RTS-01 row set; the inventory's both-directions equality makes a missing row red rather than absent | **live** `data/rights/` holds **10** rows and `data/expected/rights.ids` **10** ids (`bank-modes cfg-container commercial-rom core-info-strings exec-image flat-cartridge-image grom-image libretro-abi rendered-frame save-state`), the step prints `rights=10`, and both directions of the equality were measured under T-13-03; `docs/RIGHTS.md` carries all ten `item` phrases verbatim | closed |
| T-13-18 | 13-02 | Tampering | an operator page that pins itself | medium | mitigate | every `item` phrase is free of its own row's `tokens` under the character-bounded rule before the row lands | **live** replayed the bounded predicate over all ten rows: **none** of the ten `item` phrases contains any of its own `tokens` (`item phrases containing their own token (bounded): none`), so the page that must name every row cannot refuse itself | closed |
| T-13-19 | 13-02 | Information disclosure | a `cleared` row added to make a row look answered | medium | mitigate | every new row is `refused` or `owner-local-only` with `source: null`; `libretro-abi` stays the only `cleared` row and `rights-cleared=1` is the count that would catch a second one | **live** `grep -l '"cleared"' data/rights/*.json` -> `data/rights/libretro-abi.json` only; `-- manifests pass ... rights=10 rights-cleared=1`; the nine other rows carry `source: null` and dispositions `refused` (4) / `owner-local-only` (5) | closed |
| T-13-20 | 13-05 | Repudiation | a quoted instead of recomputed median | high | mitigate | the five `cpu_s` values and the median are printed and the middle value is the recorded one; the frozen 361/118 figures are named stale in the same record; recomputed again once the phase's own row landed | `13-05-SUMMARY.md:186-197` prints the five rows and `cpu_s sorted: 357, 363, 366, 435, 497 -> middle value 366`; **live recompute** over the committed file at this commit: `tail -5 .planning/ci-runs.tsv` -> `cpu_s 366 363 497 357 360` -> sorted `357 360 363 366 497` -> **median 363**, `gate_s` **121**; `.planning/STATE.md` carries that post-row recompute in the phase's own commit `e2d3a40` with the window named (`45683886915 ... 45717868904`) and the verdict unchanged (`363 > 362.935`), so the record agrees with the file rather than quoting it; `13-05-SUMMARY.md:359` scopes its own medians to "the 96 committed rows" | closed |
| T-13-21 | 13-05 | Repudiation | a fabricated hosted row | high | mitigate | the row comes from `sh tools/ci-record.sh <run-id>` for a run id `gh run list` reported for a phase commit; `ci-log` refuses a duplicate run id, a malformed row and an out-of-order date; the not-recorded branch is a visible line | **live** `-- ci-log pass ci-runs=97`; the phase's row is `.planning/ci-runs.tsv:98` = `2026-11-25 45717868904 14d72b86d3eb... pull_request success 3 5 34 86 120 136 4 360` (13 columns, real run id, commit `14d72b8` = the phase-close commit), appended by the phase's own commit `e2d3a40` (`1 +` on `ci-runs.tsv`); refusals in code `tools/check/steps/ci-log.sh:13-14,52,54`; the pre-row state was the visible branch, not a silent gap (`13-05-SUMMARY.md`: "`ci-log pass ci-runs=96` -- unchanged, so no row was appended and none was fabricated") | closed |
| T-13-SC | 13-01...13-05 | Tampering | npm/pip/cargo installs | high | mitigate | no package is installed by any of the five plans; D-48 refuses vendoring the libretro header | **live** `git diff --stat 9362fda~1..HEAD -- '*package*.json' '*lock*' '*.lock' CMakeLists.txt 'cmake/*'` -> empty; no `FetchContent`/vendored header: `grep -rln libretro --include=*.h --include=*.c --include=*.cmake` -> no match; `SRC-libretro-api-docs` is `reference-only` with `sha256: null` and its note records "nothing is vendored (D-48)"; the new `tools/check/rights-lib.cmake` is project-authored, SPDX-tagged, and `integrity` passes at 2725 files with no non-ASCII finding | closed |

*Status: closed / open / open -- below {block_on} threshold (non-blocking). Severity order:
critical > high > medium > low; only open threats at or above `high` count toward `threats_open`.*

---

## The two code-review defects, re-verified as fixed

Both were fail-open shapes at exactly the level this phase exists to close, so each was re-run
rather than taken from the review's disposition line.

| Defect | Fix commit | Re-measured this session |
|--------|-----------|--------------------------|
| **CR-01** -- a ledger row whose filename the selection regex drops is silently ignored by **both** limbs, so its `tokens` are never refused | `17461c4` | code `rights-lib.cmake:126-139` refuses `rights-unlisted: <path>` for every tracked `data/rights/` path the selection does not keep; **live** M4: adding `data/rights/toy_upper.json` (well-formed, `tokens: ["toy-upper-token"]`) to the toy tree -> `-- FAIL rights-unlisted: data/rights/toy_upper.json` (the pre-fix behaviour was a green `manifests pass`); committed stand-in `tools/check/controls/rights-ledger/overlay/data/rights/toy_stray.json` with `FAIL rights-unlisted: data/rights/toy_stray.json` as the control's fifth EXPECT line -- `control ok rights-ledger`. Liveness: with the reverse loop removed the control reports `control-token-missing` (review) and its EXPECT set no longer closes. Counts unmoved: `steps=38 faults=238 goldens=29 controls=401` |
| **WR-01** -- pin canonicalisation applied to the stale direction only, so a tab-separated pin was counted as matched and then reported as *unpinned* | `2cd0947` | code `rights-lib.cmake:415-436`: `pins_canon` is built once and **both** directions compare against it; **live** M6b/M6c/M6d on the toy tree -- `toy-image runner/a.c`, `toy-image<TAB>runner/a.c` and `toy-image  runner/a.c` (two spaces) each yield `-- files pass ... rights-pinned=1/1`; committed stand-in `tools/check/controls/rights-naming/overlay/data/expected/rights-pinned.ids` spells its matching pair with a tab plus a second tab; real tree unchanged at `rights-pinned=4/4` |

---

## Accepted Risks Log

No threat in the register is `accept` or `transfer`; every row is `mitigate` and closed. The rows
below are **non-register residuals** -- they carry no threat id, are all below the `high` block
threshold, and are recorded here so they are not re-litigated as fresh findings.

| Risk ID | Threat Ref | Severity | Rationale | Accepted By | Date |
|---------|------------|----------|-----------|-------------|------|
| R-13-A | -- (`13-REVIEW.md` IN-01) | medium | `core/include/ivs/` is outside the naming scope, so the shipped API declaration surface is unscanned. This matches RTS-02's **literal** wording (`core/src`, `runner/`, `tools/`, six operator pages) and is therefore a coverage gap against intent, not an implementation defect against the frozen requirement. Widening the prefix moves every pin and every control overlay -- a requirement change with its own diff. | owner-delegated autonomous operation (AGENTS.md); deferred to the milestone that re-opens RTS-02 | 2026-11-25 |
| R-13-B | -- (`IN-02`) | low | `libretro-abi` is a `cleared` row listing `tokens: ["libretro"]`, which the naming limb never refuses (`cleared` rows are inert by design), while the sibling `core-info-strings` row documents the opposite convention with `tokens: []`. Measured: no occurrence of the spelling exists in the scanned scopes, so **nothing is currently mis-enforced**; the defect is that the field looks enforced. Editing a ledger row is GUARDRAILS V1's retarget unit and an owner-facing content decision. | owner-delegated autonomous operation; ledger content decision | 2026-11-25 |
| R-13-C | -- (`IN-03`) | low | The token grammar admits punctuation-only tokens (a bare `.`), which the bounded match turns into "any period at a boundary". No row uses such a token and the failure is loud (`rights-token`), so this is a footgun in a guard, not a live defect. Tightening the grammar is a new refusal rule needing its own must-fail stand-in and control. | owner-delegated autonomous operation; next change to RTS-01 | 2026-11-25 |
| R-13-D | -- (`IN-04`) | low | The operator page's disposition column is unchecked, so a row flipped `refused`->`owner-local-only` with the page left saying `refused` would pass. Measured today: all ten page dispositions agree with their rows, so the gap is latent. A new token (`rights-doc-disposition`) is a new mechanism under GUARDRAILS section 2 rule 4. | owner-delegated autonomous operation; scoped follow-up | 2026-11-25 |
| R-13-E | -- (`IN-05`) | low | `steps/runner-selftest.sh:254` restates `data/expected/runner-selftest.ids` inside the step that reads it as data. Measured (review): a fifth inventory name fails **both** directions, so the duplication cannot rot silently; rewriting it would edit the stand-in whose failure mode is currently proved. | owner-delegated autonomous operation; tidiness only | 2026-11-25 |
| R-13-F | -- (auditor observation) | low | `control-make-path` and `control-make-verb` (`run-controls.sh:99-101`, `:107`, `:112`) refuse an absolute or `..`-bearing `MAKE` target and an unknown verb, but **no committed control proves either able to fire** (`grep -rln 'control-make-path\|control-make-verb' tools/check/controls/` -> no match). The predicates are one-line `case` tests with the fail-closed shape, no `MAKE` file in the corpus carries a traversal spelling (measured), and no overlay path contains `..`. A GUARDRAILS section 2.4 stand-in gap, not a live hole. | owner-delegated autonomous operation; no register row declares this surface | 2026-11-25 |
| R-13-G | -- (auditor observation) | low | The naming limb is **case-sensitive**: measured, `TOY-IMAGE` in a scoped file produces no `rights-token`, while `a toy-image b` does. A refused spelling in upper case would therefore pass. No register row declares case, no occurrence exists in the scanned scopes today, and the requirement's wording is about the spelling; recorded so the next RTS-02 change decides it deliberately. | owner-delegated autonomous operation; recorded rather than silently narrowed | 2026-11-25 |

*Accepted risks do not resurface in future audit runs.*

---

## Security Audit Trail

| Audit Date | Threats Total | Closed | Open | Run By |
|------------|---------------|--------|------|--------|
| 2026-11-25 | 21 (27 register rows) | 21 | 0 | gsd-security-auditor (pi), ASVS L1, `block_on: high` |

**What was executed, not inferred.** `sh tools/gate.sh` (PASS, 116 s); `run-controls.sh`
(`controls=401/401`, plus three further runs on copied corpora for the both-direction probes);
the `manifests`, `files`, `claims`, `important-gates`, `runner-selftest`, `sources` and `ci-log`
steps run directly against a `git ls-files` FILELIST; fourteen adversarial mutations on a `/tmp`
copy of `tools/check/controls/_good/tree` (misspelled key, null source, unregistered source,
inventory line deleted, inventory line orphaned, unselected ledger path, operator row removed,
unpinned occurrence, three pin spellings, wildcard pin, three-field pin, stale pin); seven
bounded-match probes on the same copy; one patched copy of `tools/check` to prove the boundary
class is live; one mutated controls inventory to prove `control-not-in-inventory`; one mutated
control overlay to prove both `control-toy-drift` and `control-deviation-unused`; and the
arithmetic over `.planning/ci-runs.tsv` and `data/expected/important-gates.ids`.

**Nothing was written outside this file.** The tree was inspected read-only; every mutation lived
in `/tmp/aud13` and was removed (`git status --short` empty at close, no stashes, no branches
switched).

---

## Sign-Off

- [x] All threats have a disposition (`mitigate` x 21; no `accept` or `transfer` rows)
- [x] Accepted risks documented in the Accepted Risks Log (seven non-register residuals, all
      below the `high` block threshold)
- [x] `threats_open: 0` confirmed -- no open threat at or above `high`
- [x] Each mitigation is shown working by a command or a `file:line`, not by the plan's claim
