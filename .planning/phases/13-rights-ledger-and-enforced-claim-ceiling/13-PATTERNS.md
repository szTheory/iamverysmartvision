# Phase 13: Rights Ledger and Enforced Claim Ceiling - Pattern Map

**Mapped:** 2026-11-26
**Files analyzed:** 40 (13 new data/checker files, 10 ledger rows, 6 control dirs, 11 modified files)
**Analogs found:** 38 / 40 (2 with no close analog; 2 with a documented analog divergence)

Every path below is git-tracked source (verified with `git ls-files -- <path>`). No
`.gsd/capabilities/` mirror paths are named. Every excerpt is quoted verbatim from the file
and line range shown.

---

## File Classification

### New files

| New File | Role | Data Flow | Closest Analog | Match Quality |
|----------|------|-----------|----------------|---------------|
| `data/rights/<slug>.json` x10 | data / record | request-response (declarative lookup) | `fixtures/<id>/manifest.json` (per-file record) + `data/claims.json` (collection record) | role-match (see **Divergence D1**) |
| `data/expected/rights.ids` | data / inventory | set equality | `data/expected/claims.ids` | exact |
| `data/expected/rights-pinned.ids` | data / pin list | pair-keyed suppression | `data/expected/hygiene-history-pinned.ids` | exact |
| `data/expected/runner-selftest.ids` | data / inventory | set membership (one-directional) | `data/expected/pkg-selftests.ids` | exact |
| `tools/check/schema/rights.txt` | config / schema | declarative | `tools/check/schema/claim.txt` | exact — **superseded, see the amendment below the table** |
| `tools/check/kinds/rights.cmake` | hook / checker | batch over records | `tools/check/kinds/claim.cmake` | role-match — **superseded, see the amendment below the table** |
| `tools/check/steps/*` | — | — | — | **none — no new step** (CONTEXT invariant) |
| `tools/check/controls/important-gates-excluded-stale/` | test / negative control | event-driven (step invocation) | `tools/check/controls/sources-src-token-whole/` | role-match |
| `tools/check/controls/rights-*` (RTS-01) | test / negative control | event-driven | `tools/check/controls/manifests-output-anchor/` | role-match |
| `tools/check/controls/rights-*` (RTS-02) | test / negative control | event-driven | `tools/check/controls/sources-src-token-whole/` | role-match |
| `tools/check/controls/claim-established-non-source/` | test / negative control | event-driven | `tools/check/controls/claims-overstated-standing/` | exact |
| `tools/check/controls/claim-established-without-tier1/` | test / negative control | event-driven | `tools/check/controls/claims-overstated-standing/` | exact |
| `tools/check/controls/claim-established-positive/` | test / negative control (FORBID-only positive) | event-driven | `tools/check/controls/important-gates-complete-uncovered/` | role-match (see **Pitfall P5**) |
| `tools/check/controls/_good/tree/data/rights/*.json` + toy `rights.ids`/`rights-pinned.ids` | test fixture | — | `_good/tree/data/claims.json` + `_good/tree/data/expected/claims.ids` | exact |
| `data/sources.json` + `data/expected/sources.ids` (one `reference-only` row) | data / registry | append-only | existing `capture: reference-only` rows (`SRC-intv-wiki`) | exact |

**Amendment (2026-11-26, plan-phase revision 2): D1 resolved to form (a), the per-file loop.**
The two rows above naming `tools/check/schema/rights.txt` and `tools/check/kinds/rights.cmake` as
new files are superseded and no such file is created. Plan 13-01 creates
`tools/check/rights-lib.cmake` instead — `rights_select` + `rights_check` (plan 13-01), then
`rights_rows` + `rights_scan_naming` (plan 13-03) — called from `tools/check/manifests.cmake` and
`tools/check/files.cmake`. There is no `records.cmake` kind, no `schema/rights.txt` and no
`schema/rights.txt`-driven `keys`/`req` grammar for the ledger: the per-file loop copies
`manifest-lib.cmake:186-216`'s FILELIST-driven selection and does its own exact-key-set check, so
the key set lives in `rights-lib.cmake` rather than in the schema language. The
"`tools/check/schema/rights.txt`" and "`tools/check/kinds/rights.cmake`" pattern sections below
remain the grammar references they are (the `claim.txt` schema language table, the hook
conventions and anti-patterns); read them as the design precedent, not as file plans. The
consequence for the plan set: `data/expected/gate-steps/rights.ids` still names `manifests` and
`files` only, and `steps` stays 38 because the new functions are library code called from two
existing steps, never a third step.

### Modified files

| Modified File | Role | Data Flow | Closest Analog (same file, the block to extend) | Match Quality |
|---------------|------|-----------|-------------------------------------------------|---------------|
| `tools/check/manifests.cmake` | checker | batch | its own `read_ids` inventory block `:453-464` | exact (in-place) |
| `tools/check/files.cmake` | checker | batch scan | its own `FILELIST` loop + `gate_stat` tail `:89-99`, `:163-175` | exact (in-place) |
| `tools/check/kinds/claim.cmake` | hook | batch | its own `depends_on` loop `:157-177` | exact (in-place) |
| `tools/check/steps/important-gates.sh` | step | batch | its own inventory loop `:130-147` and phase table `:37-57` | exact (in-place) |
| `tools/check/steps/runner-selftest.sh` | step | batch | `tools/check/steps/pkg-install.sh:832-851` | exact (cross-file idiom) |
| `data/expected/important-gates.ids` | data / registry | append | its own Phase 11 block `:472-484` | exact (in-place) |
| `data/expected/controls/important-gates.ids` | data / inventory | append | its own Plan 09-05 block `:19-26` | exact (in-place) |
| `data/expected/controls/claims.ids` | data / inventory | append | its own `claims-overstated-standing` line | exact (in-place) |
| `data/expected/controls/rights.ids` | data / inventory | append | its own Plan 09-07 block | exact (in-place) |
| 13 toy files (2 toy records + 1 overlay copy + 10 toy registries) | test fixture | append | commit `16afb31` diff shape | exact |
| `docs/RIGHTS.md` | doc | — | its own `## How this is checked` section `:48-58` | exact (in-place) |
| `.planning/REQUIREMENTS.md` | record | — | its own Traceability rows `:104-107` | exact (in-place) |
| `.planning/ci-runs.tsv` | data | append | its own last rows | exact (in-place) |

---

## Pattern Assignments

### `data/rights/<slug>.json` (data record, declarative lookup)

**Analog:** `data/claims.json` (collection-record grammar) + `fixtures/FX-*/manifest.json`
(one-file-per-record grammar, which is the actual physical shape here).

**Divergence D1 — resolve this before writing the schema.** CONTEXT fixes **one file per
row** (`data/rights/<slug>.json`), but `records.cmake` reads **one file containing a
collection** (`schema/*.txt` line `file <path>` + `array <key>`, `records.cmake:53-77`,
`:225-244`). RESEARCH's "fourth `records.cmake` kind" therefore needs one of two mechanical
forms:

- **(a) per-file loop** — copy `manifest_load_all`'s FILELIST-driven selection
  (`tools/check/manifest-lib.cmake:186-216`) and call a `check_shape`-style per-object
  validator (`tools/check/manifests.cmake:41-66`). This is the only shape that matches
  "one file per row" literally.
- **(b) single collection** — one `data/rights.json` with a `rights` array, exactly like
  `claims.json`, and `records.cmake`/`schema/rights.txt`/`kinds/rights.cmake` reused verbatim.
  This contradicts CONTEXT's file-per-row and would move `rights.ids` membership onto one file.

Do not invent a third form. Both legal forms below are quoted so the planner can pick one and
copy the right half.

**Imports / preamble pattern** (`tools/check/manifests.cmake:1-30`, verbatim excerpt):

```cmake
# SPDX-License-Identifier: MIT
# Fail-closed fixture-manifest checker (docs/fixture-manifest.md, schema_version 1).
# Inputs (passed with -D):
#   ROOT      the tree under test (required)
#   FILELIST  file with one repo-relative path per line (required)
# ...
# Tokens: manifest-parse, manifest-unknown-key, manifest-missing-key, manifest-format,
#   manifest-id-dir-mismatch, manifest-duplicate-id, manifest-id-missing, manifest-id-extra,
#   ...
# Success line: "manifests pass manifests=<n> effective-yes=<a> effective-no=<b>".
cmake_minimum_required(VERSION 3.25)

foreach(required ROOT FILELIST)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "files.cmake is missing ${required}")
  endif()
endforeach()
include("${CMAKE_CURRENT_LIST_DIR}/lib.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/manifest-lib.cmake")
```

**FILELIST-driven per-file collection load** (`tools/check/manifest-lib.cmake:186-216`,
verbatim excerpt — the pattern for form (a)):

```cmake
function(manifest_load_all report)
  manifest_filelist_read()
  file(READ "${FILELIST}" txt)
  gate_lines(lines "${txt}")
  set(sel "")
  foreach(l IN LISTS lines)
    gate_unprotect(l "${l}")
    if(l MATCHES "^fixtures/[A-Za-z0-9._-]+/manifest[.]json$")
      list(APPEND sel "${l}")
    endif()
  endforeach()
  list(SORT sel)
  ...
  foreach(p IN LISTS sel)
    if(NOT EXISTS "${ROOT}/${p}")
      continue()
    endif()
    file(READ "${ROOT}/${p}" doc)
    string(JSON t ERROR_VARIABLE e TYPE "${doc}")
    if(NOT "${e}" STREQUAL "NOTFOUND" OR NOT t STREQUAL "OBJECT")
      if(report)
        gate_fail(manifest-parse "${p}")
      endif()
      continue()
    endif()
```

For form (a), the selection regex becomes `^data/rights/[a-z0-9-]+[.]json$`, and the slug in
the path is compared with the record's `id` exactly as `manifests.cmake:299-303` compares
`dirname` with `id` (`manifest-id-dir-mismatch`).

**Fail-closed shape check** (`tools/check/manifests.cmake:41-66`, verbatim) — the reason a
bespoke key loop is forbidden:

```cmake
# Presence, type and allowed keys of one object. ARGN holds "key:TYPE|TYPE" specs;
# every listed key is required and every other member is an unknown key.
function(check_shape lbl prefix obj)
  set(allowed "")
  foreach(spec IN LISTS ARGN)
    string(FIND "${spec}" ":" c)
    string(SUBSTRING "${spec}" 0 ${c} key)
    math(EXPR c1 "${c} + 1")
    string(SUBSTRING "${spec}" ${c1} -1 types)
    string(REPLACE "|" ";" types "${types}")
    list(APPEND allowed "${key}")
    json_type(t "${obj}" "${key}")
    if(t STREQUAL "MISSING")
      gate_fail(manifest-missing-key "${lbl}/${prefix}${key}")
    elseif(NOT "${t}" IN_LIST types)
      gate_fail(manifest-format "${lbl}/${prefix}${key}")
    endif()
  endforeach()
  json_members(mem "${obj}")
  foreach(m IN LISTS mem)
    if(NOT "${m}" IN_LIST allowed)
      gate_fail(manifest-unknown-key "${lbl}/${prefix}${m}")
    endif()
  endforeach()
endfunction()
```

**Nullable-key grammar** (the `source` key is a `SRC-` id or `null`) — copy
`tools/check/schema/source.txt:11-25`'s `TYPE|NULL` convention and `records.cmake:246-256`'s
`req` handling of `NULL`.

**Conventions this analog demonstrates**
- SPDX line first, then a `# Tokens:` inventory, then `# Success line:` — the file documents
  its own contract before its code.
- `gate_fail(<token>, <detail>)` where `<token>` is `<kind>-<rule>` and `<detail>` is a path or
  `id/field`. Never a bare sentence.
- Every required key has its own `req`/`check_shape` entry, so a **misspelled** key is an
  unknown key, not a silently-absent one.
- Fail-closed on unparseable input; `continue()` rather than a partial pass.
- `data/` files must be ASCII-only (`tools/check/integrity.cmake:60-63`).

**Anti-patterns the analog guards against**
- A hand-written key loop that accepts `dispositon` (misspelled) because it only checks the
  keys it knows.
- An enum checked against a hard-coded list in code instead of `enumfrom`/`data/tiers.json`
  (`schema/claim.txt:24`), so the list and the data can drift.
- `nonblank tokens` on a row whose `tokens` is legally `[]` — `records.cmake:280-300` treats an
  empty ARRAY as blank, so `core-info-strings` (RESEARCH A6) must **not** carry a `nonblank
  tokens` rule.

**Canonical row values** (from RESEARCH "The ten rows"): `id` = the slug, `item` = the human
phrase ("save state"), `disposition` ∈ `cleared|refused|owner-local-only`, `source` = `SRC-…`
or `null`, `taken` = `"nothing"` or a description, `tokens` = an array of artifact spellings,
`note`. Keep `item` free of the forbidden spellings so the row does not pin itself
(Pitfall P3).

---

### `data/expected/rights.ids` (data inventory, set equality)

**Analog:** `data/expected/claims.ids` (verbatim, full file head):

```
# Claim ids (FND-02); must equal the ids in data/claims.json.
CL-0001
CL-0002
...
CL-0028
```

and `data/expected/sources.ids:1` (`# Owning plan 01-07: registry ids (add-only).`).

**The consuming pattern** — `tools/check/manifests.cmake:453-464` (verbatim), which
`records.cmake:314-326` repeats for every `inventory`-bearing kind:

```cmake
read_ids(want "data/expected/fixtures.ids" manifest)
foreach(i IN LISTS want)
  if(NOT "${i}" IN_LIST uniq)
    gate_fail(manifest-id-missing "${i}")
  endif()
endforeach()
foreach(i IN LISTS uniq)
  if(NOT "${i}" IN_LIST want)
    gate_fail(manifest-id-extra "${i}")
  endif()
endforeach()
```

**Conventions**
- One id per line, `#` comments, sorted C-locale bytewise, no duplicates
  (`lib.cmake:75-116` fails on `-inventory-missing`, `-inventory-empty`,
  `-inventory-duplicate` for free).
- Header names the owning plan and states the invariant ("must equal the ids in …").
- Add-only is stated in the header when it is true (`sources.ids`), because deletion is then a
  behaviour change with its own diff.
- Ten ids, no `#allow-empty:` line — the file is a membership baseline, not an optional set.

**Anti-patterns**
- Listing ids in the data file and not the inventory (or vice versa): both directions fire, so
  a one-sided edit is red either way.
- Using `#allow-empty:` to silence an accidental empty inventory; only two files in the tree
  carry it and both record why (`cp1610-skips.ids:2`, `cp1610-coverage-pending.ids:4`).

---

### `data/expected/rights-pinned.ids` (data pin list, pair-keyed suppression)

**Analog:** `data/expected/hygiene-history-pinned.ids` — the closest existing `(token, path)`
pin list, and the file RESEARCH R3 says to mirror. Verbatim head:

```
# Owning plan 08-02 (D-33): the six findings of sh tools/hygiene.sh --history that are accepted without a
# history rewrite, one "<token> <10-character commit sha>" pair per line. Read by tools/release-readiness.sh.
# Disposition: a delegated decision (accepted without a rewrite). The matched text of each is the shape of a
# scratch-directory path, not a private path with identifying content. A rewrite is an owner-only hard stop and
# would not purge the pull-request refs. The history baseline file (tools/check/hygiene-baseline.txt) is never
# moved and no rule is widened. A finding that is not on this list, whatever its token, fails the release check
# (a token alone never matches: the pair does). The owner may veto this disposition and choose a rewrite.
...
hygiene-history-home-path eaa832e3d3
hygiene-history-home-path 92c4dc7d37
```

**The reader grammar to copy verbatim** — `tools/release-readiness.sh:52-63`:

```awk
    BEGIN {
      while ((getline l < pin) > 0) {
        sub(/^[ \t]+/, "", l)
        if (l == "" || substr(l, 1, 1) == "#") continue
        if (split(l, w, " ") == 2) {
          key = w[1] " " w[2]
          if (!(key in pinned)) { pinned[key] = 1; pord[++np] = key }
        }
      }
      close(pin)
    }
```

`split(l, w, " ") == 2` is the whole grammar: exactly two whitespace-separated fields. A pin
line can therefore not rot into a wildcard.

**The one deliberate divergence** — `tools/release-readiness.sh:80` makes a pin that no longer
matches a `NOTE`:

```awk
      for (j = 1; j <= np; j++) if (!(pord[j] in seen)) printf "NOTE: pinned pair not found in this run: %s\n", pord[j]
```

For rights this must be a **failure** (`rights-pinned-stale`), because a rights pin names a
path that can be deleted while a history pin names a commit that cannot. The CMake shape to
write, in `files.cmake` between `:163` and `:173`, is quoted in RESEARCH R3b and reproduced
under `tools/check/files.cmake` below.

**Canonical content** — exactly the four measured pins, sorted:

```
exec-image docs/owner-images.md
exec-image tools/owner-run.sh
grom-image docs/owner-images.md
grom-image tools/owner-run.sh
```

**Conventions**
- Header states: the owning plan, the pair grammar, the disposition, **why the pins are
  legitimate** ("the operator's own argument placeholders, so the names *are* the operator
  interface"), and what happens to a finding that is not pinned.
- Appended rows carry an `# Appended (<phase> …):` comment with the reason — see the Phase 10
  block at the tail of `hygiene-history-pinned.ids`.
- Sorted, no duplicates (`read_ids` will refuse duplicates).

**Anti-patterns**
- A token-only pin list. A token alone matches every instance; the pair is the permission.
- A pin whose justification is convenience. The four pins are legitimate because the names are
  the operator interface, not because they were already there.
- Widening a rule by deleting a pin. A stale pin now fails, so deletion is a red gate, not a
  silent widening.
- Pinning `docs/RIGHTS.md` ten times because the page lists the items — see Pitfall P3.

---

### `data/expected/runner-selftest.ids` (data inventory, one-directional membership)

**Analog:** `data/expected/pkg-selftests.ids` — the exact idiom RESEARCH R1f names. Verbatim
head:

```
# Owning plans 04-22 and 04-23: cases of the in-step selftest of the gate step pkg-install (D-40).
# Each case mutates a COPY (or probes a deleted path) and must fail for its intended reason; one case
# name per line; the step runs exactly these and fails on a name it does not know
# (pkg-selftest-missing). The last two (leak-detector, golden-drift) concern the consumer (04-23); plan 05-11
# adds three for the replay of diag-ctrl-psg-pass: ctrl-digest-drift (a ctrl.digest nibble changed), ctrl-domain-missing
# (the ctrl.digest.psglog line dropped) and ctrl-peek-wrong (the peeked $01FE changed from 0x007E). Plan 06-13 adds
# two for the replay of diag-stic-fgbg-pass: stic-domain-missing (the stic.digest.video line dropped) and stic-digest-drift (a
# stic.digest nibble changed). Plan 07-09 adds two for the replay of demo-lamp-tone-pass: scn-domain-missing (the scn.digest.audio
# line dropped) and scn-digest-drift (a scn.digest.video nibble changed). Plan 08-16 adds two for the fifth replay, the snapshot of the same program: snap-domain-missing
# (the snap.digest.audio line dropped) and snap-digest-drift (a snap.digest.video nibble changed).
core-src-include
missing-symbol
...
```

**The consumer loop to copy verbatim** — `tools/check/steps/pkg-install.sh:832-851`:

```sh
sinv="$ROOT/data/expected/pkg-selftests.ids"
st_ok=0
st_total=0
if [ -f "$sinv" ]; then
  awk '/^#/ { next } NF { print $1 }' "$sinv" > "$T/st.cases"
  [ -s "$T/st.cases" ] || bad pkg-selftest-inventory "no cases"
  while IFS= read -r c; do
    st_total=$((st_total + 1))
    cst=0
    run_case "$c" < /dev/null || cst=$?
    case $cst in
      0) st_ok=$((st_ok + 1)) ;;
      2) bad pkg-selftest-missing "$c" ;;
      *) bad "pkg-selftest-$c" "$detail" ;;
    esac
  done < "$T/st.cases"
else
  bad pkg-selftest-inventory "data/expected/pkg-selftests.ids"
fi
```

Three load-bearing halves: the `*) return 2 ;;` sentinel that makes an unknown case name
`-missing` rather than a case failure; the `[ -s ]`/`else` guard that makes an absent or empty
inventory its own token instead of a silent zero-case pass; and the header sentence "the step
runs exactly these and fails on a name it does not know".

**Canonical content** (RESEARCH R1d) — four names, three existing plus one minted:

```
st-toy-drift
st-deviation-unused
st-pass-line
st-memmap-generator-missing
```

**Conventions**
- Header names the owning plan, the four mechanisms, and the invariant in one paragraph.
- One case name per line, `st-` prefix, kebab-case, no `#allow-empty:` (a zero-case inventory
  is a refusal).
- The file is read with `awk '/^#/ { next } NF { print $1 }'`, so a trailing note column would
  be tolerated — do not add one; the header carries the prose.

**Anti-patterns**
- A hard-coded `for c in a b c` in the step. That is exactly what the inventory exists to
  prevent: the registry could then name a case the step no longer runs.
- Registering the token `memmap-generator-missing` as the id instead of minting the case name
  `st-memmap-generator-missing`. The `mutant` lookup is by exact string
  (`important-gates.sh:178`), and every other `st-` name in the tree uses the case-name form.

---

### `tools/check/schema/rights.txt` (config schema, declarative)

**Analog:** `tools/check/schema/claim.txt` — verbatim, full file:

```
# Claim ledger (FND-02, D-08, D-09). Records are the elements of claims; ids are CL-NNNN.
file data/claims.json
topkeys schema_version note claims
top schema_version NUMBER
top note STRING
top claims ARRAY
array claims
idkey id
inventory data/expected/claims.ids
keys id statement profile tier artifact depends_on standing note
req id STRING
req statement STRING
req profile STRING
req tier STRING
req artifact STRING
req depends_on ARRAY
req standing STRING
req note STRING
nonblank statement
nonblank profile
nonblank artifact
nonblank note
match id ^CL-[0-9][0-9][0-9][0-9]$
enumfrom tier data/tiers.json evidence_tiers id
match standing ^(established|provisional)$
# WR-05: an established claim must name the profile fields it rests on, or the weakest-field rule is vacuous.
when standing=established nonblank depends_on
hook claim
```

**The schema language, as `records.cmake` actually implements it**
(`records.cmake:40-96` parses; `:236-311` applies):

| Directive | Line | Effect |
|-----------|------|--------|
| `file <path>` | `:53-54` | the one record file |
| `topkeys a b c` | `:59-61` | every other top-level member is `<kind>-unknown-topkey` |
| `top <key> <TYPE>` | `:67-68` | presence + type at top level |
| `array <key>` / `members <key>` | `:69-71` | the collection mode |
| `idkey <key>` | `:72-73` | record id |
| `inventory <path>` | `:74-75` | exact set equality, both directions |
| `keys a b c` | `:62-64` | any other member is `<kind>-unknown-key` |
| `req <key> <TYPE[|TYPE]>` | `:255-268` | missing → `<kind>-missing`; wrong type → `<kind>-type` |
| `nonblank <key>` | `:269-300` | empty string/NULL/empty array → `<kind>-blank` |
| `match <key> <regex>` | `:301-311` | → `<kind>-format` |
| `enumfrom <key> <file> <array> <idkey>` | `:311-324` | → `<kind>-unknown-ref` |
| `when <key>=<value> <rule>` | `:225-253` | conditional rule |
| `hook <name>` | `:76-77` | includes `kinds/<name>.cmake` |

**Canonical shape for form (b)** (a single `data/rights.json` collection, if that form is
chosen) — model it directly on `schema/claim.txt`:

```
file data/rights.json
topkeys schema_version note rights
array rights
idkey id
inventory data/expected/rights.ids
keys id item disposition source taken tokens note
req id STRING
req item STRING
req disposition STRING
req source STRING|NULL
req taken STRING
req tokens ARRAY
req note STRING
nonblank item
nonblank taken
nonblank note
match id ^[a-z0-9][a-z0-9-]*$
match disposition ^(cleared|refused|owner-local-only)$
hook rights
```

Note what is deliberately **absent**: no `nonblank tokens` (A6's empty list is legal), no
`nonblank source` (`null` is legal for three dispositions), and no `enumfrom source` — source
resolvability is a `data/sources.json` **membership** question answered in the hook, not an
`enumfrom` on `sources.ids`, because the schema's `enumfrom` cannot express "required only when
`disposition == cleared`" without a `when` on a NULL-valued key.

**Conventions**
- Comment line first stating the kind, the record shape and the id grammar.
- `keys` is the **complete** member set: a misspelling is an unknown key, never a missing one.
- Nullable keys use `STRING|NULL`; a required non-null string uses `STRING`.
- Enums come from data (`enumfrom`), or from a `match` alternation when the vocabulary is the
  schema's own (`match disposition ^(cleared|refused|owner-local-only)$`).
- A conditional requirement is a `when` line plus a comment saying *why* it exists (the
  `# WR-05:` comment above `when standing=established nonblank depends_on` is the model).

**Anti-patterns**
- A schema with no `keys` line: then any extra key is accepted and a typo silently no-ops.
- Encoding a cross-file rule in `enumfrom` when it needs a conditional; put it in the hook.
- `nonblank` on a legally-empty array (`records.cmake:288-299` treats `[]` as blank).

---

### `tools/check/kinds/rights.cmake` (hook, batch over records)

**Analog:** `tools/check/kinds/claim.cmake` — the hook with the closest shape (per-record
enum/type reads, a cross-file lookup, a stat pair, and a rule that fires only for one
`standing`). Verbatim preamble:

```cmake
# SPDX-License-Identifier: MIT
# Hook for kind claim (SKELETON.md section 4).
# Failure tokens: claim-order: <id>, claim-unknown-profile: <id>, claim-dangling-path: <id>,
#   claim-handoff-artifact: <id>,
#   claim-unknown-field: <id>/<field>, claim-overstated: <id>,
#   claim-measured-without-d10: <id>, claim-type: <id>/depends_on.
# Stats: claims=<n>, provisional=<n>.
# The profile file is read once per profile id and cached in GLOBAL properties.
cmake_minimum_required(VERSION 3.25)
```

and the two required function names, at `:54` and `:180-185`:

```cmake
function(hook_record id rec)
...
endfunction()

function(hook_finish)
  gate_count(nc claim-total)
  gate_count(np claim-provisional)
  gate_stat(claims "${nc}")
  gate_stat(provisional "${np}")
endfunction()
```

**The "read one optional string field" idiom** (`claim.cmake:70-93`, verbatim) — copy this for
`disposition`/`source`/`taken`:

```cmake
  set(profile "")
  json_type(pt "${rec}" profile)
  if(pt STREQUAL STRING)
    string(JSON profile GET "${rec}" profile)
  endif()
```

**The "member of another registry" idiom** — `claim.cmake:139-152` resolves a profile id to a
cached file, and `source.cmake` resolves through the inventory. For a `cleared` row's `source`,
the check is **membership in `data/sources.json`**, which `records.cmake:150-177`'s `enum_ids`
already implements (and caches in a GLOBAL property):

```cmake
function(enum_ids out relpath arraykey idkey)
  set(prop "ENUM:${relpath}:${arraykey}:${idkey}")
  get_property(isset GLOBAL PROPERTY "${prop}" SET)
  if(NOT isset)
    ...
```

**Conventions**
- The `# Failure tokens:` block is the file's contract; add every new token there **and** to the
  `# Tokens:` block of the step that surfaces it (`steps/claims.sh:17-20` shows the pattern).
- `gate_incr` in `hook_record`, `gate_count` + `gate_stat` in `hook_finish`. Never `gate_stat`
  inside the per-record loop.
- A cross-file lookup is cached in a GLOBAL property keyed by the file+array+id, so N records
  cost one read.
- `return()` early when the precondition (a resolvable profile / a `cleared` disposition) is
  absent, rather than guarding every later line.

**Anti-patterns**
- Consulting `data/sources.json`'s `rights` field for a `cleared` row's resolvability. CONTEXT
  locks this off: that field answers *the reference document's own* rights (it is `unknown` for
  every source — `schema/source.txt:31` pins `match rights ^unknown$`), so consulting it would
  make every `cleared` row impossible.
- A second `gate_finish`: only `records.cmake` emits it (`records.cmake:333`).
- Re-reading `data/sources.json` per row.

---

### `tools/check/files.cmake` (checker, batch scan — MODIFIED)

**Analog:** the same file. Two insertion points, both quoted verbatim.

**The sorted `FILELIST` list, already in scope** (`files.cmake:89-99`):

```cmake
file(READ "${FILELIST}" list_text)
gate_lines(lines "${list_text}")
set(paths "")
foreach(l IN LISTS lines)
  gate_unprotect(l "${l}")
  if(NOT l STREQUAL "")
    list(APPEND paths "${l}")
  endif()
endforeach()
list(SORT paths)
list(ntracked "0")
```

(the live line is `list(SORT paths)` then `list(LENGTH paths ntracked)`).

**The tail where the pin reverse direction belongs** (`files.cmake:163-175`, verbatim):

```cmake
endforeach()

list(REMOVE_DUPLICATES fixture_dirs)
list(SORT fixture_dirs)
foreach(d IN LISTS fixture_dirs)
  if(NOT "${d}" IN_LIST manifest_dirs)
    gate_fail(files-fixture-without-manifest "${d}")
  endif()
endforeach()

gate_stat(tracked "${ntracked}")
gate_stat(binaries-listed "${nbin}")
gate_finish(files)
```

Insert between the `endforeach()` at `:163` and `gate_stat(tracked …)` at `:173`. That is the
only correct position: the token scan has finished (the observed `(token, path)` set is
complete), `paths` is still in scope, and the finding lands before `gate_finish` — which is what
makes it a `FAIL` line rather than a `NOTE`.

**The bounded-token extraction idiom** — `tools/check/steps/sources.sh:44-51`, with its
measured rationale at `:36-43`:

```sh
# Whole-token extraction (EV-03 item 1). The class is [A-Za-z0-9_-] and carries NO dot, and the
# quantifier is +, so the match ends only where the token ends: a truncating class silently turns
# 'SRC-bsr-2609_nope' into the registered 'SRC-bsr-2609' (the step then exits zero) and a
# lowercase-only class ignores 'SRC-Nope-Doc' entirely. ...
while IFS= read -r f; do
  grep -a -o -E 'SRC-[A-Za-z0-9_-]+' "$ROOT/$f" >> "$tmp/refs"
done < "$tmp/scan"
sort -u "$tmp/refs" | while IFS= read -r id; do
  grep -q -x -F -e "$id" "$tmp/ids" || printf -- '-- FAIL source-unknown-ref: %s\n' "$id"
done > "$tmp/unknown"
```

The rights naming limb is this idiom **inverted**: the token is planted in the data, so the
match must require that the character before and after the token is **outside** `[A-Za-z0-9_-]`
(CMake: `string(REGEX MATCH "(^|[^A-Za-z0-9_-])<token>([^A-Za-z0-9_-]|$)" …)`, or in `sh`:
`grep -a -n -E "(^|[^A-Za-z0-9_-])$(escaped)([^A-Za-z0-9_-]|$)"`). Verified by measurement:
`rom-image` does **not** match inside `grom-image` under that class, because the preceding
character is `g`.

**The pin reverse direction to write** (RESEARCH R3b, verbatim):

```cmake
# data/expected/rights-pinned.ids: one "<token> <path>" pair per line, exactly two fields.
# A pin that no longer matches is a failure, not a NOTE: history pins a commit, which cannot
# disappear; a rights pin names a path, which can, and a stale pin means the rule silently widened.
read_ids(pins "data/expected/rights-pinned.ids" rights-pinned)
foreach(pin IN LISTS pins)
  string(REGEX MATCH "^[^ ]+ +[^ ]+$" two_field "${pin}")
  if(two_field STREQUAL "")
    gate_fail(rights-pinned-malformed "${pin}")
    continue()
  endif()
  if(NOT "${pin}" IN_LIST seen)
    gate_fail(rights-pinned-stale "${pin}")
  endif()
endforeach()
```

`read_ids` is reusable even though a pin is two fields: it treats the whole stripped line as one
id, which **is** the pair key, and it gives `rights-pinned-inventory-missing|empty|duplicate`
for free (`lib.cmake:79`, `:104`, `:112-114`).

**Conventions**
- The header block (`files.cmake:1-18`) lists every rule as `<scope> -> <token>`, then a
  `Tokens also:` line, then `Success line:`. Extend all three.
- The scan is anchored to `FILELIST` paths, never a filesystem walk — this is what keeps
  `tools/check/controls/**/overlay/**` out of the real tree's scan (the same anchoring decision
  `steps/sources.sh:5-6` documents for its own scan).
- `gate_stat` keys are added, never repurposed; `tracked=<n>` will move because new files are
  tracked (RESEARCH Pitfall 8).

**Anti-patterns**
- A `docs/**` glob for the naming scope. The six operator pages are a data list; a glob would
  sweep in `docs/provenance/`, `docs/stic/`, `docs/video/` (reference notes that legitimately
  quote sources) and would silently widen with every new doc file.
- A global `grep` for `cfg` / `ivsc`. Both are *refusal surfaces* in
  `tools/check/binary-ext.txt:5,8` and `.gitignore:7,9`; removing either deletes a refusal.
  Measured: `cfg` occurs in 9 files across the four scopes.
- Bare `exec`, `rom`, `grom` as refused tokens. Measured file counts in the four scopes:
  `exec` 98, `rom` 182, `grom` 29. They are hardware/API vocabulary (`ivs_exec`,
  `memsys_grom_read`), not artifact names.
- `grep -w`. It treats `-` as a word separator, so `save-state` would match inside `save state`
  and `rom-image` inside `grom-image`.

---

### `tools/check/kinds/claim.cmake` (hook — MODIFIED, D-50)

**Analog:** the same file, `hook_record`'s `depends_on` loop. Verbatim `:157-177`:

```cmake
  set(weak FALSE)
  foreach(i RANGE 0 ${dlast})
    json_type(et "${rec}" depends_on ${i})
    if(NOT et STREQUAL "STRING")
      gate_fail(claim-type "${id}/depends_on")
      continue()
    endif()
    string(JSON field GET "${rec}" depends_on ${i})
    if(NOT "${field}" IN_LIST known)
      gate_fail(claim-unknown-field "${id}/${field}")
      continue()
    endif()
    get_property(ftag GLOBAL PROPERTY "CLAIM_PROFILE_TAG:${profile}:${field}")
    if("${ftag}" MATCHES "^(tier4-community|tier5-emulator-hint|inference|unknown-default-chosen)$")
      set(weak TRUE)
    endif()
  endforeach()
  # Weakest-field rule: an established claim may not rest on a weak field.
  if(weak AND standing STREQUAL "established")
    gate_fail(claim-overstated "${id}")
  endif()
```

D-50 adds two conditions **in this same function**: (a) `standing STREQUAL "established"` AND
NOT `tier STREQUAL "source-expectation"` → new token; (b) inside the loop, an established claim
whose field tag is not `tier1-primary-doc` → a second new token. The existing
`claim-overstated` rule stays; it is strictly weaker and firing both is legal (both are
findings, and `EXPECT` is a substring match).

**Why the early return cannot make (b) vacuous** — `claim.cmake:145-156`:

```cmake
  json_type(dt "${rec}" depends_on)
  if(NOT dt STREQUAL ARRAY)
    return()
  endif()
  string(JSON dn LENGTH "${rec}" depends_on)
  if(dn EQUAL 0)
    return()
  endif()
```

`schema/claim.txt:27` (`when standing=established nonblank depends_on`) already refuses an
established claim with no `depends_on`, so the early return is unreachable for the case D-50
judges.

**Conventions**
- The `# Failure tokens:` header block is the register; two new tokens go there.
- An established-only rule reads `standing`, which `hook_record` already extracts at `:88-92`
  alongside `tier` at `:93-97`. Do not re-read them.
- Field tags come from the GLOBAL property cache `CLAIM_PROFILE_TAG:<profile>:<path>`
  (`claim.cmake:20-48`); never re-open the profile.
- Keep the comment above the rule: it states *why* the rule is not vacuous.

**Anti-patterns**
- Editing `CL-0002` to test the ceiling. CONTEXT locks the positive to a toy row, so
  `data/claims.json` and `data/expected/claims.ids` do not move for a test.
- A "tier1-only" allow-list that also drops the existing weak-field rule. D-50 is strictly
  stronger; keep both.
- Reading `tier` from a `NULL`/missing value without a type guard (`json_type` first, always).

---

### `tools/check/steps/important-gates.sh` (step — MODIFIED, one glob + one table block)

**Analog:** the same file.

**The inventory loop, exactly as it stands** (`important-gates.sh:130-147`, verbatim):

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

**The one-line change** (RESEARCH R1b): add `"$exp/runner-selftest.ids"` to the `pkg-selftests`
line. The `[ -f "$f" ] || continue` means the toy tree needs no new file, because
`_good/tree/data/expected/` has no `runner-selftest.ids` (verified: `ls
tools/check/controls/_good/tree/data/expected/` → no such file).

**The prose that must move with it** (`important-gates.sh:60-63`, verbatim):

```
#   mutant   a case of data/expected/pkg-selftests.ids or data/expected/ci-summary-selftests.ids
#            (in-step selftests), or a test id `<suite>.judge.<slug>` or `<suite>.mutant.<slug>`
#            of any data/expected/tests/*.ids (suite self-controls, judged by tests-inventory).
```

and its mirror in `data/expected/important-gates.ids:8-9` (verbatim):

```
# (EXPECT token); a mutant is an in-step selftest case (pkg-selftests.ids, ci-summary-selftests.ids)
# or a suite self-control `<suite>.judge.<slug>` / `<suite>.mutant.<slug>` that passes only when
```

**The phase table to extend** (`important-gates.sh:37-57`, verbatim tail):

```
# The phase table below records which plan added which set. It is history for the reviewer, not the
# definition, so it cannot silently omit a requirement.
#   Phase 1  FND-01 FND-02 FND-03 FND-04 FND-05 FND-06 FND-07                      plan 09-04
...
#   Phase 10 EV-05 plan 10-06 (the three controls of the handoff ledger and of the look/listen prose:
#            the two claims controls and the one text control, the first rows either gate name has had)
#   Phase 11 TOOL-01 plan 11-05 (the runner-record gate: the two input-log faults, the writer's dropped
#            event line and the replay's one-tick shift, and the two judge self-controls of the
#            recorded-and-replayed comparison; the accepted set is derived, so the row's nreq limb is
#            gone and this table and the token prose are the only parts a new requirement retargets)
```

Insert a four-line Phase 13 block after `:57`, in the same two-line-continuation shape.
`16afb31` added exactly four lines here for the same reason ("the R2 mechanism retarget: a
Phase 11 line in the step header"), so four lines is the precedent.

**The row judgement a new row must satisfy** (`important-gates.sh:169-180`, verbatim):

```awk
  {
    lines++
    if (NF != 4 || $1 !~ /^[a-z0-9][a-z0-9-]*$/ || ($2 != "fault" && $2 != "control" && $2 != "mutant") \
        || !($4 in accepted)) {
      bad("important-gates-unknown-id", "line " FNR)
      next
    }
    if (!(($2 " " $3) in known)) { bad("important-gates-unknown-id", $2 " " $3 " line " FNR); next }
    if (!($1 in gates)) { gates[$1] = 1; ngates++ }
    covered[$4] = 1
  }
```

**Conventions**
- `<gate>` is a free lower-case name; `16afb31` set the precedent of one new gate name per
  requirement's mechanism set.
- The derived accepted set is `Complete − excluded` (`:93-131`), so a row naming a `Pending`
  requirement is red on the commit that adds it (Pitfall P1).
- `gates=`/`lines=` are **runtime stats** (`:188`), never header fields. Re-derive by running
  the step.
- The step's own docstring states every token, its detail shape and the two-directional
  behaviour. Extend it when the mechanism set changes.

**Anti-patterns**
- Quoting `lines=337` / `gates=110` from a frozen doc. Both are stale in places today; the
  measured post-phase figure is `gates=114 lines=347` (RESEARCH R5a).
- A `mutant` row for a mechanism that already has a control directory — a second registration of
  one proof (RESEARCH R1).
- Hard-coding the four selftest case names in the registry's `known` set. That breaks the step's
  own documented invariant and lets the registry name a case the selftest no longer runs.

---

### `tools/check/steps/runner-selftest.sh` (step — MODIFIED, read its case inventory)

**Analog:** `tools/check/steps/pkg-install.sh:832-851` (quoted under
`data/expected/runner-selftest.ids` above). This is a **cross-file idiom**, not an in-place one,
because `runner-selftest.sh` has no inventory today.

**The current tail, where the check belongs** (`runner-selftest.sh:227-228`, verbatim):

```sh
[ "$rc" -eq 0 ] || exit 1
printf -- '-- runner-selftest pass cases=%s\n' "$cases"
```

**The three existing case names and where they are minted** (`runner-selftest.sh`):
- `st-toy-drift` — `:94-100` builds `"$ctl/st-toy-drift/overlay/data/expected"`; `:121` pairs
  `control-toy-drift:st-toy-drift`.
- `st-deviation-unused` — `:102-108` builds it; `:121` pairs it.
- `st-pass-line` — `:155` `mkdir -p "$pl/controls/st-pass-line" "$pl/inventory"`; `:160`
  `printf '%s\n' st-pass-line > "$pl/inventory/selftest.ids"`.

**The fourth has no case name at all** (`runner-selftest.sh:205-226`). Its own comment explains
why, and that comment is the model for how to record a minted name:

```sh
# Generator missing (EV-03 item 5). The memmap step derives the generator's path from CHECK_DIR,
# and a control may only change the tree, so no control directory can reach this guard. The copy
# above has no sibling cmake/, so the step must report the liveness guard and stop before invoking
# cmake at all. Without this case the guard's failure would be recorded only as prose - the
# lesson-20 defect GUARDRAILS 2.3 exists to prevent. The filelist is a runtime artifact; the
# script's own source must never spell the tree's generated directory name, because a step's
# partition class is derived by grepping this file.
mg_tree="$cc/controls/_good/tree"
...
elif printf '%s\n' "$mgout" | grep -q -F -e '-- FAIL memmap-generator-missing: cmake/IvsMemmapProbes.cmake'; then
  # Printed with the same non-collected prefix as the pass-line proof above.
  printf -- '-- runner-selftest proof memmap-generator-missing: cmake/IvsMemmapProbes.cmake (observed)\n'
  cases=$((cases + 1))
else
  bad runner-selftest-missing "memmap-generator-missing"
fi
```

**The refusal-printing convention this step already uses** (`runner-selftest.sh:167-176`,
verbatim) — a proof line must **not** start with `-- FAIL `:

```sh
  # Print the observed refusal so the guard's proof is visible in every gate log rather than living
  # only in a temp file this run throws away - but NOT at the start of the line. tools/gate.sh's
  # keep_fails collects every line matching '^-- FAIL ' into fail.lines, which ci-summary.sh renders
  # into the job summary; a refusal this case EXPECTS to observe would then be advertised as a
  # failure of a passing run. The '-- runner-selftest proof ' prefix keeps the token text readable
  # and keeps it out of that set.
  printf -- '-- runner-selftest proof control-pass-line-not-last: profile (observed: '
```

**Conventions**
- Header block states every token and its detail shape: `# Tokens: runner-selftest-missing:
  <token>.` (`:26`) — add `runner-selftest-inventory: <path>` beside it.
- `bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }` (`:32`), `rc` checked once at the
  tail.
- `cases=<n>` is the stat; a new inventory must **not** change it (the cases already run).
- A proof line that reports an *observed* refusal uses the `-- runner-selftest proof ` prefix so
  `gate.sh`'s `keep_fails` does not advertise it.

**Anti-patterns**
- An inventory read that only checks one direction (case exists → run it). The direction that
  matters is the other one: an inventory name the step does not run must be `-missing`, so a
  renamed or deleted case cannot leave a registry row pointing at nothing.
- A silent zero-case pass when the inventory is absent or empty. That is its own token.
- Printing an expected refusal at column 0.

---

### `data/expected/important-gates.ids` (registry — MODIFIED, 10 rows + baseline)

**Analog:** the same file's Phase 11 block — the closest precedent in both content and commit
shape. Verbatim `:472-484`:

```
# Phase 11 (plan 11-05): TOOL-01's four mechanisms under one new gate name, runner-record. The recorded
# ...
# only when the no-log witness is rejected as vacuous. TOOL-01 takes no exclusion-ledger row: its
# mechanisms are real, and an exclusion row would be the registry declining to cover what it just built.
runner-record fault RECORD_DROPS_LAST_EVENT TOOL-01
runner-record fault REPLAY_APPLIES_LATE TOOL-01
runner-record mutant runner.judge.record-replay-differs TOOL-01
runner-record mutant runner.judge.record-replay-inert TOOL-01
```

**The ten rows to append** (RESEARCH R1h) — one new gate name each per requirement set, four new
gate names total:

| # | row |
|---|-----|
| 1 | `d09-06-registration control important-gates-complete-uncovered EV-07` |
| 2 | `d09-06-registration control important-gates-excluded-stale EV-07` |
| 3 | `d09-06-registration control important-gates-excluded-malformed EV-07` |
| 4 | `d09-06-registration mutant st-toy-drift EV-07` |
| 5 | `d09-06-registration mutant st-deviation-unused EV-07` |
| 6 | `d09-06-registration mutant st-pass-line EV-07` |
| 7 | `d09-06-registration control sources-src-token-whole EV-07` |
| 8 | `d09-06-registration control manifests-output-anchor EV-07` |
| 9 | `d09-06-registration control memmap-generator-refused EV-07` |
| 10 | `d09-06-registration mutant st-memmap-generator-missing EV-07` |

plus `rights-ledger` (RTS-01), `rights-naming` (RTS-02), `claim-ceiling` (EV-06).

**Conventions**
- A comment block above each requirement's rows stating *what the mechanism set is* and, where
  one exists, *why no exclusion row*.
- Exactly four whitespace-separated columns; no trailing comment column.
- The header's `# Columns:` block (`:5-9`) and the mutant-source prose (`:8-9`) are part of the
  contract and move with the mechanism set.
- The baseline is not in the header: `lines=` is a runtime stat.

**Anti-patterns**
- An exclusion-ledger row to make the identity close. `important-gates-excluded.ids:14-20`
  documents that an exclusion row is the registry declining to cover what it just built.
- Adding rows in one commit and the requirement flip in a later "re-bless" commit
  (GUARDRAILS V7/V11; `16afb31` shows the single-commit shape).

---

### `data/sources.json` + `data/expected/sources.ids` (registry — MODIFIED, one row)

**Analog:** the existing `reference-only` rows. Verbatim `SRC-intv-wiki` (`data/sources.json:65-80`):

```json
    {
      "id": "SRC-intv-wiki",
      "title": "Intellivision community wiki (Hand Controller and timing pages)",
      "url": "https://wiki.intellivision.us/index.php/Hand_Controller",
      "wayback": null,
      "accessed": "2026-11-09",
      "capture": "reference-only",
      "sha256": null,
      "size_bytes": null,
      "pages": null,
      "rights": "unknown",
      "authority_tier": "tier4-community",
      "local_path": null,
      "note": "Community wiki; wiki pages are editable and change, so a hash of one revision would not identify the document. ..."
    },
```

**The `reference-only` rule the row must satisfy** — `tools/check/kinds/source.cmake:63-83`
(verbatim):

```cmake
  elseif(capture STREQUAL "reference-only")
    if(NOT shat STREQUAL NULL)
      gate_fail(source-format "${id}/sha256")
    endif()
    if(NOT szt STREQUAL NULL)
      gate_fail(source-format "${id}/size_bytes")
    endif()
    if(NOT lpt STREQUAL NULL)
      gate_fail(source-format "${id}/local_path")
    endif()
    # The note must say why the document is not hashed.
    set(note "")
    json_type(nt "${rec}" note)
    if(nt STREQUAL STRING)
      string(JSON note GET "${rec}" note)
    endif()
    string(TOLOWER "${note}" lnote)
    string(FIND "${lnote}" "hash" hpos)
    if(hpos LESS 0)
      gate_fail(source-reference-only-unexplained "${id}")
    endif()
  endif()
```

The lower-cased `note` **must contain the substring `hash`**. `sha256`, `size_bytes` and
`local_path` must be JSON `null` (not absent, not `""`).

**The full key set the row must carry** — `tools/check/schema/source.txt:11-33` (quoted in
RESEARCH R2a). Every one of the 14 keys is `req`; `rights` is pinned to `^unknown$` at `:31`.

**The stat that must not move** — `source.cmake:10-13` and `:87-93`:

```cmake
function(hook_record id rec)
  gate_incr(source-total)
...
function(hook_finish)
  gate_count(n source-total)
  gate_count(nh source-hashed)
  gate_count(nv source-verified)
  gate_stat(sources "${n}")
  gate_stat(refs-verified "${nv}/${nh}")
```

A `reference-only` row increments `source-total` only. Predicted: `sources=13
refs-verified=3/4`. Predicting `refs-verified=4/5` is RESEARCH Pitfall 9.

**Both files, one commit.** `schema/source.txt:10` is `inventory data/expected/sources.ids`, and
`records.cmake:314-326` is exact set equality in both directions. `sources.ids:1` states
`(add-only)`, so append.

**Conventions**
- 14 keys in document order, one per line, 2-space object indent, `null` for nullable.
- `note` is a full sentence naming the *reason* and the disposition, and usually the phase that
  will re-read it.
- `authority_tier` is an `enumfrom` value from `data/tiers.json`; the precedent for an
  emulator's own documentation is `tier5-emulator-hint` (`SRC-mame-intv`), with
  `tier4-community` the defensible alternative.
- `rights` stays `unknown`; the record is not a licence finding.

**Anti-patterns**
- A note that explains the reason without the literal substring `hash`.
- `sha256: ""` or an omitted key instead of `null`.
- Adding the row to `data/sources.json` without `sources.ids` (red either way).
- Vendoring the document under `third_party/` — D-48 refuses it.

---

### `tools/check/controls/<new>/` (negative controls — 6 new dirs)

**Analog A — the plain EXPECT/FORBID shape.** `tools/check/controls/sources-src-token-whole/`
(three files, verbatim):

```
CHECK:   sources
EXPECT:  FAIL source-unknown-ref: SRC-bsr-2609_nope
         FAIL source-unknown-ref: SRC-Nope-Doc
FORBID:  sources-no-files
```
plus `overlay/docs/evidence-tiers.md`.

**Analog B — the anchor control.** `tools/check/controls/manifests-output-anchor/`:

```
CHECK:   manifests
EXPECT:  FAIL manifest-format: FX-good/outputs/path
```
(no `FORBID` file at all; `overlay/fixtures/FX-good/manifest.json`). A control with no `FORBID`
is legal and normal (`run-controls.sh:230-233` treats an absent file as no forbidden tokens).

**Analog C — the two-EXPECT control with a reason for the FORBID list.** Verbatim
`tools/check/controls/important-gates-complete-uncovered/` (CHECK/EXPECT/FORBID), and the
inventory comment that explains it (`data/expected/controls/important-gates.ids:19-23`):

```
# Plan 09-05 (EV-04, A-01): the derived must-be-covered set, both directions. A Complete requirement
# with no registry line and no exclusion (important-gates-complete-uncovered) and an exclusion whose
# requirement is not Complete (important-gates-excluded-stale), in one run. The FORBID names
# important-gates-unknown-id only: a Complete-and-unregistered requirement also trips the generic
# important-gates-uncovered-gate, so forbidding that token would make the control unsatisfiable.
important-gates-complete-uncovered
```

**The runner contract a control must satisfy** — `tools/check/run-controls.sh:198-252`
(verbatim excerpt):

```sh
ctl_one() {
  _d="$CONTROLS_DIR/$1"
  _step=
  if [ -f "$_d/CHECK" ]; then IFS= read -r _step < "$_d/CHECK" || :; fi
  if [ -z "$_step" ] || [ ! -f "$CHECK_DIR/steps/$_step.sh" ]; then
    fail control-unknown-step "$1"; return 0
  fi
  # A blank EXPECT line (only whitespace) would match every output. ...
  if [ "$_blank" -ne 0 ]; then
    fail control-expect-blank "$1"; return 0
  fi
  build_tree "$2" "$_d" || { fail control-build "$1"; return 0; }
  if run_step "$2" "$_step"; then
    fail control-exit-zero "$1"; return 0
  fi
  # Tokens are fixed strings matched within an output line (as grep -F): every EXPECT line must
  # occur and no non-empty FORBID line may. ...
```

**The three hard rules this enforces**
1. A control **must** make its step exit non-zero (`control-exit-zero`) — a control can never
   express "this passes". See Pitfall P5.
2. Every `EXPECT` line must occur somewhere in the output; every non-empty `FORBID` line must
   occur **nowhere**. Both are substring matches within one line, so two `EXPECT` lines are
   legal and normal.
3. The directory name must appear in exactly one `data/expected/controls/*.ids` file, and every
   inventory entry must have a directory (`run-controls.sh:297-298`, both `comm` directions).

**The overlay mechanism** — `run-controls.sh:90-130` supports `MAKE` verbs (`delete`, `empty`,
`write`, `rename`, `symlink`, `crlf`, `bom`, `nonascii`, `binary`, `nul`, …) as an alternative
to a whole-file `overlay/`. `write <path> <text>` is the cheapest way to plant a token or a
malformed row without committing a whole-file copy.

**Conventions**
- `CHECK` holds one step name, no arguments, no newline issues (`IFS= read -r`).
- `EXPECT` lines are copied **verbatim** from the real step's output, including the
  `FAIL <token>: <detail>` shape. Do not paraphrase a detail.
- `FORBID` names the *other* tokens the overlay could plausibly trip. Its job is to prove the
  control failed for the intended reason alone.
- `overlay/` is a partial tree copied over a pristine `_good/tree`; `MAKE` is a transform. Never
  both for the same file.
- A control whose overlay touches `data/expected/important-gates.ids` needs a `DEVIATION`
  declaration for each difference (`run-controls.sh:300-326`) — except for the intended
  one-line append, which is applied to all ten files so no difference arises (RESEARCH R4).
- One control dir per requirement's **limbs**, folded to the six-directory budget
  (RESEARCH: 6 × 0.31 = 1.86 of 2.0 cpu-s; a seventh breaks the line).

**Anti-patterns**
- A `claim-established-positive` control whose overlay makes the step **pass** → `control-exit-zero`.
  Express the positive as `EXPECT <some other refusal>` + `FORBID claim-established-non-source
  claim-established-without-tier1` (Pitfall P5).
- A blank `EXPECT` line (matches everything) → `control-expect-blank`.
- A `DEVIATION` entry for an intended toy-registry addition → `control-deviation-unused`.
- A control that names a step that does not exist → `control-unknown-step`.
- A `MAKE` path with `/*` or `*..*` → `control-make-path`.

---

### `docs/RIGHTS.md` (documentation — MODIFIED, operator-facing table)

**Analog:** the same file's `## How this is checked` section — the shape a new limb must extend.
Verbatim `:48-58`:

```
## How this is checked

- The `manifests` step reads every `fixtures/<id>/manifest.json` and fails closed on a missing or misspelled key, an unproven `yes`, committed bytes without effective `yes`, or a digest that does not match.
- The `files` step fails on any tracked path under `refs/`, any tracked `.env` file, any tracked file that is binary by name (`tools/check/binary-ext.txt`) or by content (a NUL byte, or mostly non-text bytes) and is not a committed output of an effective `yes` manifest, and any unlisted file inside a fixture directory.
- Each failure class has a negative control that must fail for its intended reason.
- The checks compare recorded identities and file names.
  They cannot decide whether the author of a licence statement had the authority to give it, and a binary that is deliberately made to look like text (no NUL byte, mostly printable bytes, or a text extension from `tools/check/text-ext.txt` and no NUL byte) is not detected.
  A committed output of an effective `yes` manifest that has no extension in the binary list still fails the NUL rule of the `integrity` step, so give such outputs a binary extension.
```

The existing `## Never committed` list (`:9-18`) is the *prose* twin of the ten ledger rows; the
new table belongs beside it, and each row's `item` phrase should match that list's vocabulary
("EXEC and GROM images", "Game ROMs and cartridge images", "Rendered page images of a scan").

**The `text`-step tripwire this page is subject to** — `tools/check/steps/text.sh:4-12` and
`:44-48`:

```sh
# Claim words (D-27): supports, compatible, passes, accurate, verified, runs, plays, playable, looks, sounds.
# ...
claim_hits() {
  grep -a -n -i -w -E 'supports|compatible|passes|accurate|verified|runs|plays|playable|looks|sounds' \
    | grep -a -v -E '\[claim:CL-[0-9]{4}\]|\[nc\]' \
    | sed -n 's/^\([0-9][0-9]*\):.*/\1/p'
}
```

**Conventions**
- One sentence per bullet, dispositional voice ("may be named", "may not be committed"), never a
  behaviour claim.
- `[nc]` on a line where a claim word is unavoidable; `[claim:CL-NNNN]` only for a real claim.
- The page states its own limits in a closing paragraph — that is the house style, not optional
  boilerplate.
- Every row's `item` phrase is distinct from its `tokens` (Pitfall P3).

**Anti-patterns**
- A table cell containing `supports`, `passes`, `runs`, `plays`, `looks` or `sounds` →
  `text-claim-untagged: docs/RIGHTS.md:<line>`.
- Spelling a refused token in the table to explain the rule, without a deliberate
  `(token, docs/RIGHTS.md)` pin and its own diff.
- Making `docs/RIGHTS.md` the tenth pin source by accident (ten `(token, docs/RIGHTS.md)` pins in
  the first draft of `rights-pinned.ids` is the warning sign).

---

### The 13 toy files (test fixtures — MODIFIED, one line each)

**Analog:** commit `16afb31` (TOOL-01 close) — the exact diff shape to reproduce. Verbatim
`git show --stat` excerpt and the two appended lines:

```
 .planning/REQUIREMENTS.md                                   |  4 ++--
 data/expected/important-gates.ids                           | 13 +++++++++++++
 tools/check/controls/_good/tree/.planning/REQUIREMENTS.md   |  1 +
 .../_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md  |  1 +
 .../controls/_good/tree/data/expected/important-gates.ids   |  1 +
 .../overlay/data/expected/important-gates.ids               |  1 +
```

```diff
@@ -54,3 +54,4 @@ is consistent by construction: the derived must-be-covered set equals the toy re
 | STA-02 | Phase 8 | Complete |
 | PRF-01 | Phase 8 | Complete |
 | FND-98 | Phase 1 | Complete |
+| TOOL-01 | Phase 11 | Complete |
```

```diff
@@ -52,3 +52,4 @@ toy-mem02 fault GOOD_FAULT MEM-02
 toy-mem03 fault GOOD_FAULT MEM-03
 toy-mem04 fault GOOD_FAULT MEM-04
 toy-mem05 fault GOOD_FAULT MEM-05
+toy-tool01 fault GOOD_FAULT TOOL-01
```

**The exact line formats to write** (RESEARCH R4b):

- toy requirement row, appended to `_good/tree/.planning/REQUIREMENTS.md`,
  `_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md`, and the whole-file overlay copy
  `important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md`:

```
| EV-98 | Phase 13 | Complete |
```

- toy registry line, appended to each of the ten whole-file toy registries:

```
toy-phase13 fault GOOD_FAULT EV-98
```

`GOOD_FAULT` is a real toy fault (`_good/tree/data/expected/faults/cpu.ids:2`), `toy-phase13` is
a legal gate name (`^[a-z0-9][a-z0-9-]*$`), and the four columns satisfy
`important-gates.sh:171-176`. The id resolves as a `fault`, which is why no toy
`runner-selftest.ids` is needed.

**The ten registries** (RESEARCH R4 list, verified): `_good/tree/data/expected/
important-gates.ids` plus the nine overlays under `important-gates-{bad-req-phase5,
bad-req-phase6, req-uncovered-phase5, req-uncovered-phase6, req-uncovered-phase7,
uncovered-gate, unknown-id, unknown-id-phase5, unknown-id-phase6}/overlay/data/expected/`.

**The drift check the edit must not trip** — `run-controls.sh:300-326` (verbatim excerpt):

```sh
    {
      comm -23 "$tmp/reg.good" "$tmp/reg.ov" | sed 's/^/< /'
      comm -13 "$tmp/reg.good" "$tmp/reg.ov" | sed 's/^/> /'
    } | sort > "$tmp/reg.diff"
    : > "$tmp/reg.decl"
    if [ -f "$CONTROLS_DIR/$n/DEVIATION" ]; then
      awk 'NF' "$CONTROLS_DIR/$n/DEVIATION" | sort > "$tmp/reg.decl"
    fi
    while IFS= read -r _l; do
      grep -q -x -F -e "$_l" "$tmp/reg.decl" || fail control-toy-drift "$n: $_l"
    done < "$tmp/reg.diff"
    while IFS= read -r _l; do
      grep -q -x -F -e "$_l" "$tmp/reg.diff" || fail control-deviation-unused "$n: $_l"
    done < "$tmp/reg.decl"
```

**Conventions**
- Comments are excluded from the comparison, because every overlay replaces the good tree's
  header by design. Only non-comment lines matter.
- One line, zero deletions, in **all ten** files in the same commit. Measured: all nine overlays
  differ from `_good` by exactly one declared line today, and all ten files end with
  `toy-tool01 fault GOOD_FAULT TOOL-01`, so a single same-line append leaves every existing
  difference untouched.
- The toy record is a **curated mirror**, not an automatic one: it is `live − EV-05 + FND-98`
  (measured id diff). Adding one row to the live record does not imply one row here; the toy id
  is chosen separately and verified unused.
- No `DEVIATION` file: the addition is intended in all ten, and a `DEVIATION` entry would trip
  `control-deviation-unused`.

**Anti-patterns**
- Adding the line to `_good` but not all nine overlays (or vice versa) → `control-toy-drift`.
- Four lines per registry (one per closed requirement) instead of one — RESEARCH A4 fixes it at
  one.
- Reusing `FND-96`/`FND-99`/`EV-99`: all three are already in use (`FND-96` is reserved by
  RESEARCH R1g for the stale-exclusion overlay; `FND-99`/`EV-99` live in the
  `important-gates-complete-uncovered` overlay). `EV-98` is verified unused tree-wide.

---

### `.planning/REQUIREMENTS.md` (record — MODIFIED, four flips + four rows)

**Analog:** the same file, `:104-107` (verbatim):

```
| RTS-01 | Phase 13 | Pending |
| RTS-02 | Phase 13 | Pending |
| EV-06 | Phase 13 | Pending |
| EV-07 | Phase 13 | Pending |
```

and the checkbox lines at `:23-24`, `:32-33` (`- [ ] **RTS-01**: …` → `- [x] **RTS-01**: …`).

**The derivation that consumes it** — `important-gates.sh:93-103` (verbatim):

```sh
for f in "$ROOT/.planning/REQUIREMENTS.md" "$ROOT"/.planning/milestones/*-REQUIREMENTS.md; do
  [ -f "$f" ] || continue
  awk -F'|' '/^\| *[A-Z]+-[0-9]+ *\|/ {
      id = $2; st = $4
      gsub(/^ +| +$/, "", id); gsub(/^ +| +$/, "", st)
      if (st ~ /^Complete/) print id
    }' "$f" >> "$tmp/complete"
done
```

Note it parses **only the Traceability table**, not the checkbox lines. The checkbox flip is a
record convention; the table flip is what the gate reads. Do both.

**Conventions**
- Status is a prefix match (`^Complete`), so `Complete` and `Complete (2026-11-26)` both work.
- The two deletions in `16afb31` were exactly these two-line pairs (checkbox + table row).
- Four flips + four table edits = the `4 ++--` line in `16afb31`'s stat.
- Same commit as the registry rows (Pitfall P1).

**Anti-patterns**
- Flipping the checkbox and not the table: the gate stays red and the record disagrees with
  itself.
- Flipping a requirement whose rows are in a later commit → `important-gates-complete-uncovered`
  and `important-gates-uncovered-gate` both fire.

---

### `.planning/ci-runs.tsv` (data — MODIFIED, one appended row)

**Analog:** the same file's header and last rows (verbatim):

```
date	run_id	sha	event	conclusion	queue_s	setup_s	build_s	test_s	gate_s	total_s	ncpu	cpu_s
...
2026-11-25	45683331636	d9e1cf2c9790d53804a04aace7cc6ffb3168bab4	pull_request	success	2	2	46	102	148	159	4	497
2026-11-25	45683886915	6620dd0f8083cb43ecf012166dd806f901468d40	push	success	3	6	33	88	121	140	4	357
```

13 tab-separated columns, append-only, no comment lines. The `ci-log` step
(`tools/check/steps/ci-log.sh`) is the only gate over its shape.

**The arithmetic to recompute, not quote** (RESEARCH "cost-checkpoint arithmetic"): last five
rows in file order → `cpu_s` median **366**, `gate_s` median **121**. The frozen
`361`/`118` figures are already 5 cpu-s stale. Because the measured median already exceeds
`361 + 2.0` with an unchanged token line, the honest verdict for this phase's checkpoint is
**`reprice`**, not `discharged-within-allowance` — unless the token line moves by the predicted
amount.

**Conventions**
- `-1` in the numeric columns means "not measured" (see the 2026-11-11 `failure` row).
- Append only; never rewrite a row.
- Record exactly one of `discharged-within-allowance` / `reprice` / `lever-required` alongside
  it, per CONTEXT.

**Anti-patterns**
- Quoting a median from a doc instead of computing it from this file (AGENTS.md: "Recompute,
  never quote").
- Adding a CI measurement run just to fill the row. CONTEXT: recompute from the committed rows.

---

## Shared Patterns

### S1. Failure contract: `gate_fail` + one `gate_finish`

**Source:** `tools/check/lib.cmake:8-11`, `:35-44` (verbatim):

```cmake
function(gate_fail token detail)
  message(STATUS "FAIL ${token}: ${detail}")
  set_property(GLOBAL APPEND PROPERTY GATE_FAILURES "${token}")
endfunction()
...
function(gate_finish name)
  get_property(failures GLOBAL PROPERTY GATE_FAILURES)
  get_property(stats GLOBAL PROPERTY GATE_STATS)
  list(LENGTH failures n)
  if(n GREATER 0)
    message(FATAL_ERROR "${name}: ${n} failure(s)")
  endif()
  string(REPLACE ";" " " stats "${stats}")
  message(STATUS "${name} pass ${stats}")
endfunction()
```

**Apply to:** `kinds/rights.cmake`, the `files.cmake` insert, the `claim.cmake` insert.
**Rules:** `<token>` is `<kind>-<rule>`; `<detail>` is a path or `id/field`; exactly one
`gate_finish` per checker, at the tail; `gate_stat` only in a finish path.

### S2. Inventory membership via `read_ids`

**Source:** `tools/check/lib.cmake:75-116`. Gives `<kind>-inventory-missing`,
`-inventory-empty`, `-inventory-duplicate` for free; sorted unique output.
**Apply to:** `rights.ids`, `rights-pinned.ids`, and the pin reverse direction.
**Note:** `#allow-empty: <reason>` is the only escape, and only two files in the tree use it.

### S3. SPDX header on every new `tools/**/*.{sh,cmake}`

**Source:** `tools/check/integrity.cmake:69-79` (verbatim excerpt):

```cmake
  if(p MATCHES "^tools/.*[.](sh|cmake)$" OR p MATCHES "^[.]github/workflows/[^/]*[.]yml$"
     OR p MATCHES "^(core|include|tests)/.*[.](c|h|sh|cmake)$" OR p MATCHES "^cmake/.*[.]cmake$"
     ...
    string(FIND "${content}" "SPDX-License-Identifier: MIT" pos)
    if(pos EQUAL -1)
      gate_fail(integrity-missing-spdx "${p}")
    endif()
  endif()
```

**Apply to:** `schema/rights.txt` (a `.txt`, exempt), `kinds/rights.cmake` (**required**), any
new control-dir `MAKE`/`CHECK`/`EXPECT` (exempt — not `.sh`/`.cmake`), the `files.cmake` and
`claim.cmake` edits (already present).

### S4. ASCII-only data

**Source:** `tools/check/integrity.cmake:60-63` (verbatim):

```cmake
  if(p MATCHES "^(data|fixtures|tools|[.]github|core|include|tests|cmake|runner)/" OR p MATCHES "^CMakeLists[.]txt$")
    if(content MATCHES "[^\t\n -~]")
      gate_fail(integrity-non-ascii "${p}")
    endif()
  endif()
```

**Apply to:** all ten `data/rights/*.json`, `data/expected/*.ids`, `data/sources.json`.
**Rule:** plain ASCII in `item`/`taken`/`note`/`tokens` — no em dash, no curly quote, no
non-breaking space.

### S5. The `# Tokens:` docstring block

**Source:** every step (`steps/sources.sh:6`, `steps/claims.sh:17-20`,
`steps/important-gates.sh:64-73`) and every hook (`kinds/claim.cmake:3-6`,
`kinds/source.cmake:3-5`).
**Apply to:** every new or modified checker.
**Rule:** list every token with its detail shape, then the stats line. A new token that is not in
this block is an undocumented contract.

### S6. Two-directional ledgers

**Source:** `important-gates.sh:185-187`, `manifests.cmake:453-464`,
`run-controls.sh:297-298`, `records.cmake:314-326`.
**Rule:** every membership relation this phase adds is checked **both** ways, each direction with
its own token, except the one-directional selftest inventory (where the missing-name direction is
the only one that can be wrong).

### S7. Same-commit retargets (GUARDRAILS V1/V2/V4/V11)

**Source:** commit `16afb31` (16 files, 32 insertions, 2 deletions, one commit).
**Apply to:** every ledger row (→ file + `rights.ids` + `docs/RIGHTS.md` row + reading id), every
refused spelling (→ tokens + pinned ids + control overlay), every `established` standing (→
mechanism + two must-fail controls + one positive), every D-09-06 mechanism (→ registry row).
**Rule:** a count or baseline edited in a later "re-bless" commit is a defect, not a
convenience.

---

## No Analog Found

| File | Role | Data Flow | Reason |
|------|------|-----------|--------|
| the bounded-token **naming scan** (inside `files.cmake`) | checker limb | batch scan | No existing checker scans a *data-declared* token set over a *path-prefix* scope. The closest is `steps/sources.sh`'s whole-token extraction, which is the inverse direction (token found in a file → is it registered?). Use RESEARCH Pattern 3 + the boundary-class reasoning at `sources.sh:36-43`. |
| `claim-established-positive` (the legal-positive control) | test | event-driven | No existing control expresses "this passes": `ctl_one` requires a non-zero exit by construction (`run-controls.sh:219-221`). The nearest shape is `important-gates-complete-uncovered`'s two-EXPECT + narrow-FORBID, which is what RESEARCH Pitfall 5 prescribes. |

Two further items are **analog divergences**, not absences, and are flagged in place:
**D1** — the per-file vs. single-collection ledger form (`data/rights/<slug>.json` vs.
`records.cmake`'s one-file contract), and the pin reverse direction's NOTE→FAIL promotion
(`release-readiness.sh:80` vs. `files.cmake`).

---

## Pitfalls Carried Forward (from RESEARCH, with their anchors)

| # | Pitfall | Anchor | Warning sign |
|---|---------|--------|--------------|
| P1 | A registry row names a `Pending` requirement | `important-gates.sh:173-175`; `.planning/REQUIREMENTS.md:104-107` | `important-gates-unknown-id: line <n>` |
| P2 | The token match is not character-bounded | `sources.sh:44` class | a `rights-token` finding on a file nobody edited |
| P3 | `docs/RIGHTS.md` pins itself ten times | CONTEXT's pinned page list includes it | ten `(token, docs/RIGHTS.md)` pins in the first draft |
| P4 | `docs/RIGHTS.md` prose trips the claim-word tripwire | `steps/text.sh:4-12`, `:44-48` | `text-claim-untagged: docs/RIGHTS.md:<line>` |
| P5 | A control tries to express "this passes" | `run-controls.sh:219-221` | `control-exit-zero: claim-established-positive` |
| P6 | Missing SPDX / non-ASCII data | `integrity.cmake:60-79` | `integrity-missing-spdx`, `integrity-non-ascii` |
| P7 | Toy registry drift / unused deviation | `run-controls.sh:300-326` | `control-toy-drift`, `control-deviation-unused` |
| P8 | A predicted token delta stated as "unchanged" | `gate.sh:241-249`, `files.cmake:89-99` | a cost-checkpoint row whose token line does not match the prediction |
| P9 | `refs-verified` denominator predicted as 5 | `kinds/source.cmake:22-23`, `:57-58`, `:87-93` | a predicted `refs-verified=4/5` |

**Measured baselines to predict from** (RESEARCH, `git ls-files` → 2664):

```
-- files pass tracked=2664 binaries-listed=0        -> tracked=2664+N
-- manifests pass manifests=11 effective-yes=8 effective-no=3   -> unchanged
-- claims pass claims=28 provisional=27             -> unchanged
-- important-gates pass gates=110 lines=337         -> gates=114 lines=347
-- sources pass sources=12 refs-verified=3/4        -> sources=13 refs-verified=3/4
-- runner-selftest pass cases=13                    -> unchanged
run-controls.sh: controls=395/395                   -> controls=401/401
gate summary: steps=38/38 faults=238/238 goldens=29 -> unchanged
```

---

## Metadata

**Analog search scope:** `tools/check/**` (all checkers, hooks, schemas, steps, controls,
`_good/tree`), `data/**` (records, expected inventories, tiers, profiles), `docs/**` (RIGHTS,
owner-images, provenance), `.planning/**` (REQUIREMENTS, ci-runs, milestones), `tools/gate.sh`,
`tools/release-readiness.sh`, `tools/owner-run.sh`, `.gitignore`.

**Files read this session:** `13-CONTEXT.md`, `13-RESEARCH.md` (full), `lib.cmake`,
`manifest-lib.cmake`, `manifests.cmake`, `files.cmake`, `records.cmake`, `kinds/claim.cmake`,
`kinds/source.cmake`, `schema/claim.txt`, `schema/source.txt`, `steps/important-gates.sh`,
`steps/runner-selftest.sh`, `steps/claims.sh`, `steps/sources.sh`, `steps/text.sh`,
`steps/manifests.sh`, `steps/files.sh`, `steps/pkg-install.sh` (selftest block),
`run-controls.sh` (header, `good_one`, `ctl_one`, MAKE verbs, toy drift, inventory),
`integrity.cmake` (SPDX/ASCII), `data/expected/{claims.ids, hygiene-history-pinned.ids,
important-gates.ids, important-gates-excluded.ids, sources.ids, pkg-selftests.ids,
ci-summary-selftests.ids, gate-steps/rights.ids, gate-steps/claims.ids, controls/rights.ids,
controls/claims.ids, controls/important-gates.ids}`, `data/{claims.json, sources.json}`,
`data/expected/controls/memmap.ids`, `docs/RIGHTS.md`, `tools/check/binary-ext.txt`,
`.gitignore`, `tools/check/controls/{sources-src-token-whole, manifests-output-anchor}/**`,
`tools/check/controls/_good/tree/{data/claims.json, data/profiles/…, .planning/REQUIREMENTS.md}`,
`.planning/REQUIREMENTS.md`, `.planning/ci-runs.tsv`, commit `16afb31`.

**Pattern extraction date:** 2026-11-26
**Read-only:** no source file was modified; this document is the only write.
