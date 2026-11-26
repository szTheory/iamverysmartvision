---
phase: 13-rights-ledger-and-enforced-claim-ceiling
reviewed: 2026-11-25T23:12:02Z
depth: standard
files_reviewed: 19
files_reviewed_list:
  - tools/check/rights-lib.cmake
  - tools/check/manifests.cmake
  - tools/check/files.cmake
  - tools/check/kinds/claim.cmake
  - tools/check/selftest-inventory.sh
  - tools/check/steps/important-gates.sh
  - tools/check/steps/runner-selftest.sh
  - data/sources.json
  - data/expected/rights.ids
  - data/expected/rights-pinned.ids
  - data/expected/runner-selftest.ids
  - data/expected/important-gates.ids
  - data/expected/controls/rights.ids
  - data/expected/controls/claims.ids
  - data/expected/controls/important-gates.ids
  - data/rights/libretro-abi.json
  - data/rights/cfg-container.json
  - data/rights/save-state.json
  - docs/RIGHTS.md
findings:
  critical: 1
  warning: 1
  info: 5
  total: 7
  necessity: 0
status: issues_found
dispositions:
  fixed:
    - CR-01
    - WR-01
  deferred:
    - IN-01
    - IN-02
    - IN-03
    - IN-04
    - IN-05
---

# Phase 13: Code Review Report

**Reviewed:** 2026-11-25T23:12:02Z
**Depth:** standard
**Files Reviewed:** 19
**Status:** issues_found

## Summary

Scope: the 19 paths above. Beyond them I also read all ten `data/rights/*.json` rows, the six
new control directories and the `_good` toy tree they overlay, `tools/check/lib.cmake`,
`tools/check/records.cmake`, `tools/check/schema/{claim,source}.txt`,
`tools/check/steps/sources.sh` and `tools/check/ci-summary.sh`, because every limb under review
is a caller or a data file of one of those.

**Evidence basis: executed, not inferred.** I ran the nine touched steps against a
`git ls-files` FILELIST (`manifests pass … rights=10 rights-cleared=1`, `files pass
tracked=2723 binaries-listed=0 rights-pinned=4/4`, `claims pass claims=28 provisional=27`,
`important-gates pass gates=114 lines=352`, `runner-selftest pass cases=14`,
`sources pass sources=13`, `text pass`, `integrity pass files=2723`, `hygiene pass`), the whole
control suite (`controls=401/401`), and a series of adversarial scratch mutations on a copy of
the `_good` tree. Where a claim below says "measured", it is a command I ran in this session.

**What holds up.** All eight phase-specific properties I was asked to verify are implemented
and observable. The two defects below sit beside them rather than inside them: CR-01 is a
fail-open one level *above* item 1's key-shape check (the ledger's file discovery, not its row
grammar), and WR-01 is a gap inside item 3's pin handling (the stale direction is right; the
suppression direction is not).

1. *Fail-closed shape checking.* A misspelled `dispositon` yields **both**
   `rights-missing-key: toy-owner-image/disposition` and
   `rights-unknown-key: toy-owner-image/dispositon` — the unknown-key direction really is
   closed, not just the missing-key direction (`rights-lib.cmake:62-81`; the two `gate_fail` calls are at `:69` and `:79`).
2. *The bounded token match.* `(^|[^A-Za-z0-9_-])<esc>([^A-Za-z0-9_-]|$)` behaves exactly as
   documented: measured `save state` does **not** match `save-state`, `grom-image` does **not**
   match `rom-image`, `save_state` does **not** match `save-state`, and `foo-save-state` does
   **not** match `save-state`. The boundary class is live-guarded by the real tree: I patched a
   copy of `rights-lib.cmake` to a plain substring match and the `files` step went red with
   three findings — `rights-token: .cfg tools/release.sh`,
   `rights-token: .cfg tools/check/steps/pkg-install.sh` and
   `rights-token: bank-switch docs/RIGHTS.md` (the last is `bank-switch` inside
   "bank-switched"). A regression from the character class to a substring match is therefore
   caught, not assumed. The pin grammar also cannot rot into a wildcard: `* foo` is two fields,
   never matches an observed pair, and is refused as `rights-pinned-stale`.
3. *Stale pin is a failure, pairs are deduplicated.* `gate_fail(rights-pinned-stale)` at
   `rights-lib.cmake:408`; `list(REMOVE_DUPLICATES seen)` at `:383` deduplicates by the
   `"<token> <path>"` pair string, and the single `REGEX MATCH` (not `MATCHALL`) makes a file
   that spells a token twice contribute one pair.
4. *`rights_rows()` tolerance.* Measured four malformed-row shapes against the `files` step —
   `tokens` absent, `tokens` a string, unparseable JSON, top-level array — and each produced
   `files pass … rights-pinned=0/0` with **no** unhandled CMake error and no finding, while the
   `manifests` step named the defect (`rights-missing-key: …/tokens`,
   `rights-format: …/tokens`, `rights-parse: …`). The degradation contract holds.
5. *D-50's two conditions.* Condition (a) is placed before the artifact check and before every
   `return()` in `hook_record`, so no early exit can skip it. Condition (b) cannot be vacuous:
   measured `depends_on: []` → `claim-blank: CL-0001/depends_on` and an absent `depends_on` →
   `claim-missing: CL-0001/depends_on`, both from `schema/claim.txt:27`. `claim-overstated` is
   retained and still reachable — it now co-fires with (b) for the weak tags rather than being
   subsumed, so no refusal was deleted.
6. *The mutant inventory guard.* Measured: renaming `st-toy-drift` in
   `data/expected/runner-selftest.ids` makes the step print
   `-- FAIL runner-selftest-missing: st-toy-drift-renamed` and exit 1 with no pass line (the
   file was restored and `git diff` is clean). The committed `st-no-such-case` stand-in keeps
   that guard live and also proves the accepting direction, so the rule cannot become one that
   refuses everything.
7. *No vacuous control.* All six new directories fail for their intended reason and each
   `EXPECT` is satisfiable: `controls=401/401`. `claim-established-positive` is the honest
   shape (EXPECT an unrelated `claim-dangling-path`, FORBID both new tokens) because
   `ctl_one` refuses `control-exit-zero`. `important-gates-excluded-stale` keeps the good
   tree's `FND-98` row so its four `FORBID` tokens cannot fire for the wrong reason.
8. *SPDX / ASCII / process hygiene.* `rights-lib.cmake` and `selftest-inventory.sh` carry the
   MIT SPDX line; `integrity` passes at 2723 files with no `integrity-non-ascii`; the new
   `runner-selftest` block writes only into `$tmp`, which its `EXIT` trap removes, and
   `selftest-inventory.sh` creates no files at all.

**What does not.** The ledger's own *file discovery* is the one place the fail-closed posture
inverts (CR-01): a row whose filename does not match the selection regex is invisible to both
limbs, so its refusals are silently not applied and the gate stays green. One documented
contract is half-implemented (WR-01): pin canonicalisation is applied to the stale direction but
not to the suppression direction, so a tab-separated pin is reported as an *unpinned*
occurrence. Five lower-severity observations follow.

No necessity finding was provable. The one branch that looks like an impossible-state guard —
`if(NOT EXISTS "${ROOT}/${p}") continue()` in `rights_scan_naming` — is a guard on `FILELIST`,
which is caller-supplied file data and therefore a trust boundary, so it is outside the
necessity rubric by its own negative-space rule.

## Critical Issues

### CR-01: A ledger row whose filename does not match the selection regex is silently ignored by both limbs

**File:** `tools/check/rights-lib.cmake:41` (selection), `:116-122` (`rights_check`'s empty-ledger
guard), and the identical selection used by `rights_rows()` at `:275-276`

**Issue:** `rights_select()` keeps only paths matching
`^data/rights/[a-z0-9][a-z0-9-]*[.]json$`. Any other tracked path under `data/rights/` —
`data/rights/Toy-Upper.json`, `data/rights/bank_modes.json`, `data/rights/.draft.json`,
`data/rights/sub/row.json` — is dropped from `sel` by both the `manifests` limb and the `files`
limb. There is no reverse-direction check: nothing compares the set of tracked `data/rights/`
paths against the selected set, so the row is not judged, does not appear in `ids`, never
trips `rights-id-missing` or `rights-id-extra` (its `id` is simply absent from both sides of the
set equality), and **its `tokens` are never refused**.

Measured on a copy of `_good/tree`: adding `data/rights/Toy-Upper.json` with
`"tokens": ["toy-upper-token"]` and planting `toy-upper-token` in `runner/b.c` produced

```
-- manifests pass manifests=2 effective-yes=1 effective-no=1 rights=2 rights-cleared=1
```

and no `rights-token` finding for `runner/b.c` (the only failure in that run was the
pre-existing planted pair). Both steps stayed green on the unlisted row. This is exactly the
failure class the phase exists to close — a refusal silently not applied — one level above the
key-shape check the phase did get right, and it is the same idiom the tree already applies to
fixtures (`files-unlisted-fixture-file`, `files.cmake:145-172`, `gate_fail` at `:169`: "a file inside
`fixtures/<dir>/` that its manifest does not list"). `sources.sh` does scan
`^data/.*[.]json$`, so such a row's `SRC-` references are still resolved, which makes the gap
harder to notice: half the machinery sees the file and half does not.

Nothing in `13-01-SUMMARY.md` records this as a residual or a limit, so it is an unrecorded
fail-open rather than a deliberate narrowing.

**Fix:** add the reverse direction to `rights_check()`, anchored the same way `rights_select()`
is so control overlays and `_good` copies are never selected:

```cmake
# After rights_select(sel) and the empty-ledger guard: every tracked path under data/rights/ must
# be a selected row. Without this a misnamed file (an uppercase slug, an underscore, a nested
# directory) is invisible to both limbs and its tokens are silently not refused.
function(rights_select_stray out sel)
  file(READ "${FILELIST}" txt)
  gate_lines(lines "${txt}")
  set(stray "")
  foreach(l IN LISTS lines)
    gate_unprotect(l "${l}")
    if(l MATCHES "^data/rights/" AND NOT "${l}" IN_LIST sel)
      list(APPEND stray "${l}")
    endif()
  endforeach()
  set(${out} "${stray}" PARENT_SCOPE)
endfunction()
```

and in `rights_check()`, beside `rights-ledger-empty`:

```cmake
  rights_select_stray(stray "${sel}")
  foreach(p IN LISTS stray)
    gate_fail(rights-unlisted "${p}")
  endforeach()
```

Add `rights-unlisted: <path>` to the token list in the file header, to the token list in
`manifests.cmake`'s header, and add one `EXPECT` line (`FAIL rights-unlisted:
data/rights/toy_stray.json`) to the existing `rights-ledger` control overlay — one new planted
file, no new control directory, so the priced cost does not move.

**Disposition: fixed in `17461c4`.** `rights_check()` now reads FILELIST in the reverse direction
and refuses `rights-unlisted: <path>` for every tracked path under `data/rights/` that
`rights_select()`'s regex does not select. The committed stand-in is
`tools/check/controls/rights-ledger/overlay/data/rights/toy_stray.json` (a well-formed row whose
underscore keeps it out of the selection) with `FAIL rights-unlisted: data/rights/toy_stray.json`
as its EXPECT line; the control now fails for its five intended reasons and nothing else. Liveness
measured, not assumed: with the reverse-direction loop removed from a copy of `tools/check`, the
same control loses that EXPECT line and reports `control-token-missing`. `controls` stays 401,
`steps` stays 38, `faults` stays 238, `goldens` stays 29.

## Warnings

### WR-01: Pin canonicalisation is applied to the stale direction only, so a tab-separated pin is reported as an unpinned occurrence

**File:** `tools/check/rights-lib.cmake:391-414`

**Issue:** The stale/suppression pair uses two different spellings of the same list. The stale
direction canonicalises each pin (`string(REPLACE "\t" " ")`, then `" +"` collapse, `:396-401`)
and compares against `seen`, so a tab or a run of spaces is accepted as a separator — as the
comment at `:396-397` states and as `13-03-SUMMARY.md:197` repeats ("This also makes the
comparison canonical, so a tab or a run of spaces is a separator rather than a malformed line").
The suppression direction then compares the observed pair against the **raw** `pins` list at
`:410-413`, so the canonical form is never consulted.

Measured on a copy of `_good/tree` with `toy-image` planted in `runner/a.c`:

| `data/expected/rights-pinned.ids` content | Result |
|---|---|
| `toy-image runner/a.c` | `files pass … rights-pinned=1/1` |
| `toy-image<TAB>runner/a.c` | `-- FAIL rights-token: toy-image runner/a.c` |
| `toy-image  runner/a.c` (two spaces) | `-- FAIL rights-token: toy-image runner/a.c` |

In the last two rows the pin *is* recognised as well-formed and *is* counted as matching
(no `rights-pinned-stale`), and the same run then reports the occurrence as unpinned. So the
finding names the wrong defect: the author is told the spelling is unpinned when the pin line
names exactly that pair, and the documented "a tab is a separator" contract is false in the
direction that matters. The failure is loud, not silent, so no refusal is lost — but the
diagnostic is misleading and the SUMMARY's claim is wrong.

**Fix:** canonicalise once and compare both directions against the canonical list.

```cmake
  read_ids(pins "data/expected/rights-pinned.ids" rights-pinned)
  set(pins_canon "")
  set(npin 0)
  set(nmatch 0)
  foreach(pin IN LISTS pins)
    math(EXPR npin "${npin} + 1")
    string(REPLACE "\t" " " key "${pin}")
    string(REGEX REPLACE " +" " " key "${key}")
    string(REGEX MATCH "^[^ ]+ [^ ]+$" two_field "${key}")
    if(two_field STREQUAL "")
      gate_fail(rights-pinned-malformed "${pin}")
      continue()
    endif()
    list(APPEND pins_canon "${key}")
    if("${key}" IN_LIST seen)
      math(EXPR nmatch "${nmatch} + 1")
    else()
      gate_fail(rights-pinned-stale "${key}")
    endif()
  endforeach()
  foreach(pair IN LISTS seen)
    if(NOT "${pair}" IN_LIST pins_canon)
      gate_fail(rights-token "${pair}")
    endif()
  endforeach()
```

`rights-naming`'s overlay already carries the two-space/tab case only implicitly; add a
tab-separated pin to that overlay's `rights-pinned.ids` so the canonical direction is observed,
or the same rot returns unnoticed.

**Disposition: fixed in `2cd0947`.** The canonical list is now built once (`pins_canon`) and both
directions compare against it — the stale check and the suppression loop — so a tab- or
multi-space-separated pin suppresses the occurrence it names. The committed stand-in is
`tools/check/controls/rights-naming/overlay/data/expected/rights-pinned.ids`, whose matching pair
is now separated by a tab and a second tab (the tab replacement and the run-of-spaces collapse are
both observed). Liveness measured: with the comparison reverted to the raw `pins` list, that
control hits its FORBID token `rights-token: toy-image core/src/common/planted_pinned.h`. The
contract now holds as written in `rights-lib.cmake`'s comment and `13-03-SUMMARY.md:197`; the
real tree is unchanged at `rights-pinned=4/4`.

## Info

### IN-01: `core/include/ivs/` is outside the naming scope, so the shipped API declaration surface is unscanned

**File:** `tools/check/rights-lib.cmake:351-361`

**Issue:** The scope test is `^(core/src/|runner/|tools/)` plus the six pages. `core/include/ivs/`
holds the project's public headers (`ivs_machine.h`, `ivs_run.h`, `ivs_status.h`) and is not
scanned, so a refused spelling can be declared there while the same spelling in `core/src/` is
refused — e.g. a `bank-switch` entry point (both `bank-switch` and `bankswitch` are `refused`
tokens) would pass in a header and fail in its definition. This matches RTS-02's literal wording
(`core/src`, `runner/`, `tools/`, operator pages), so it is not an implementation error against
the frozen requirement; it is a coverage gap against the requirement's intent, and the ledger's
own words are "the operator-facing nouns an item must never be named as", which a header
satisfies.

**Fix:** add `core/include/` to the prefix alternation and to `docs/RIGHTS.md`'s scope sentence,
or record the exclusion as a deliberate limit in the `rights-lib.cmake` header beside the
`tools/check/controls/` exclusion, which is documented for exactly this reason.

**Disposition: deferred.** It is a coverage question against RTS-02's literal wording, not an
implementation defect: the frozen requirement names `core/src`, `runner/`, `tools/` and the
operator pages, and widening the scope is a requirement change with its own diff (a new scope
prefix moves the control overlays and every pin). Left for the milestone that re-opens RTS-02.

### IN-02: `libretro-abi` declares a token the mechanism can never act on, while the sibling row documents the opposite convention

**File:** `data/rights/libretro-abi.json:7` (`"tokens": ["libretro"]`), versus
`data/rights/core-info-strings.json:7` (`"tokens": []`) and its note at `:8`

**Issue:** The naming limb scans only rows whose disposition is `refused` or
`owner-local-only` (`rights-lib.cmake:342`), so a `cleared` row's `tokens` are validated by
`rights-token-format` and then discarded. `core-info-strings` — a row whose own field set is likewise governed by another row —
chose the empty list, and its note at `:8` says why. The
`libretro-abi` row is `cleared` and lists `libretro`, whose note does not say the spelling is
inert. A reader of the ledger — which is the authoritative "may this be named" record — will
reasonably believe `libretro` is refused in `core/src`, `runner/` and `tools/`; measured, it is
not (no occurrence exists in those scopes today, so nothing is currently mis-enforced). Two rows
in the same phase, facing the same question — may a row list a spelling it cannot refuse — made
opposite choices.

**Fix:** either drop the list to `[]` and let the note carry the reasoning, matching
`core-info-strings`, or state in `libretro-abi`'s note that a `cleared` row's tokens are never
refused. Do not leave the field looking enforced.

**Disposition: deferred.** Both offered fixes edit a ledger row, and a ledger row is GUARDRAILS
V1's retarget unit (`data/rights/<slug>.json` + `rights.ids` + the `docs/RIGHTS.md` row + the
reading id in one commit). That is a content decision about what a `cleared` row may list, not a
code-review fix, and nothing is currently mis-enforced (no occurrence of the spelling exists in
the scanned scopes). It belongs to the owner-facing ledger, not to this fix pass.

### IN-03: The token grammar admits punctuation-only tokens, which the bounded match turns into near-universal matches

**File:** `tools/check/rights-lib.cmake:197`

**Issue:** `^[.A-Za-z0-9][A-Za-z0-9._-]*$` accepts a bare `.` or `-`. Under the bounded match
those become `(^|[^A-Za-z0-9_-])\.([^A-Za-z0-9_-]|$)`, i.e. "any period at a boundary". Measured:
a row with `"tokens": ["."]` passes `rights-token-format` (the `manifests` step stays green) and
produces `-- FAIL rights-token: . core/src/common/ivs_prelude.h`. A token of `-` would fire on
most C files. No current row uses such a token, and the failure is loud rather than silent, so
this is a footgun in a guard rather than a live defect — but the guard exists precisely to keep
tokens "a single character-bounded word", and a punctuation-only token is not a word.

**Fix:** require the token to contain at least one alphanumeric, e.g.
`^[.A-Za-z0-9][A-Za-z0-9._-]*$` plus `if(NOT tok MATCHES "[A-Za-z0-9]")`, both reporting
`rights-token-format`.

**Disposition: deferred.** The reviewer's own reading is a footgun in a guard rather than a live
defect (no row uses such a token, and the failure is loud). Tightening the grammar is a new refusal
rule with its own must-fail stand-in and a control overlay, which the phase's priced cost does not
carry; it is the right next change to RTS-01, not a review fix.

### IN-04: The operator page's disposition column is unchecked, so page/ledger drift is invisible

**File:** `tools/check/rights-lib.cmake:243-259`

**Issue:** `rights-doc-row` asserts only that the page contains the row's `item` phrase
(`string(FIND "${doctxt}" "${ditem}" pos)`). `docs/RIGHTS.md` states each row's disposition in
the adjacent table cell, and `docs/RIGHTS.md:26-28` describes the page as the operator-facing
statement of "what may never be tracked, what may, and how the gate catches an accident". A row
flipped from `refused` to `owner-local-only` in the ledger with the page left saying `refused`
passes the gate. Measured today: all ten page dispositions agree with their rows, so this is a
latent gap, not a present inconsistency.

**Fix:** after the item match, also require the same table line to carry the row's disposition,
e.g. locate the line containing `ditem` and assert it contains `` `disposition` ``; a mismatch
is `rights-doc-disposition: <id>`.

**Disposition: deferred.** A new refusal token is a new mechanism: GUARDRAILS §2 rule 4 wants a
must-fail registration, a liveness guard and a committed stand-in in one commit, plus its own
control overlay. All ten page dispositions agree with their rows today, so nothing is red now;
this is a scoped follow-up, not a defect this pass fixes.

### IN-05: The selftest-inventory stand-in hardcodes a second copy of the four case names

**File:** `tools/check/steps/runner-selftest.sh:254`

**Issue:** `printf '%s\n' st-toy-drift st-deviation-unused st-pass-line
st-memmap-generator-missing > "$si_ran"` restates `data/expected/runner-selftest.ids` in the
step that is supposed to read it as data. The accepting half then compares the real inventory
against this hardcoded list, so a fifth inventory name fails the case loudly (measured: renaming
`st-toy-drift` produced both `runner-selftest-missing: st-toy-drift-renamed` and
`runner-selftest-missing: selftest-inventory`) — the duplication cannot rot silently. It is
still a second source of truth in a step whose whole point is that the inventory is the only one.

**Fix:** build `si_ran` from the same `$ran` list the guard consumes plus a single renamed line,
e.g. copy `"$ran"` to `"$si_ran"` and append the renamed name, so the stand-in cannot disagree
with the inventory by construction.

**Disposition: deferred.** The reviewer measured that the duplication cannot rot silently (a
fifth inventory name fails both directions), so this is a tidiness change inside a selftest that
is itself the guard; rewriting it would edit the stand-in whose failure mode is currently proved.
Not worth the churn in a review-fix commit.

---

_Reviewed: 2026-11-25T23:12:02Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
