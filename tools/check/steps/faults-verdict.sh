#!/bin/sh
# SPDX-License-Identifier: MIT
# Step faults-verdict: pure checker over the named-fault results the c-faults step wrote
# (D-18, A-09). Reads only the results dir, $ROOT/data/expected/ and the FILELIST core and
# runner sources; never builds and never runs a binary, so the control runner can drive it. Results
# dir: IVS_GATE_DIR (set by tools/gate.sh, and by the control runner to its tree's build/gate),
# else $ROOT/build/gate, the same default the producer c-faults uses (review IN-07); build/gate
# below names it.
# Inventory: every data/expected/faults/*.ids file (sorted; D-37), each with a `# suite=<suite>`
# header, then one line `<FAULT_ID> <must-fail> <allowed-prefixes> <must-pass>` per fault
# (prefixes |-separated). A file without a suite header, or whose data/expected/tests/<suite>.ids
# is absent, is fault-suite-missing; an id listed twice (in one file or across files) is
# fault-duplicate-id and is judged only where it first appears.
# Per fault, build/gate/faults/<ID>/<suite>.txt must exist, carry the gate nonce and list
# exactly the ids of data/expected/tests/<suite>.ids; the must-fail vector fails, every failing
# vector starts with an allowed prefix and the must-pass vector passes. Sites: every
# `#ifdef IVS_FAULT_<ID>` / `#if defined(IVS_FAULT_<ID>)` in core/src/**/*.{c,h} and
# runner/**/*.{c,h} (repo-relative owner TUs, D-24, plan 04-03) must name a
# non-GEN_ id listed in some faults file, and every listed non-GEN_ id must have a site (both
# against the union of the non-GEN_ ids of all files, H3).
# All findings are printed, sorted. Tokens: fault-not-detected, fault-wrong-reason,
#   fault-collateral, fault-missing, fault-list-mismatch, fault-stale, fault-unknown-vector,
#   fault-orphan-site, fault-missing-site, fault-suite-missing, fault-duplicate-id.
# Stats: faults=<faults with no finding>/<listed faults> (summed over files).
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/faults-verdict.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
find="$tmp/findings"
: > "$find"
bad() { printf '%s %s\n' "$1" "$2" >> "$find"; }

gate=${IVS_GATE_DIR:-$ROOT/build/gate}
total=0
ok=0
: > "$tmp/fault-ids"
: > "$tmp/seen"
nonce=
[ -f "$gate/NONCE" ] && nonce=$(head -n 1 "$gate/NONCE")

nfiles=0
for ids in "$ROOT"/data/expected/faults/*.ids; do
  [ -f "$ids" ] || continue
  nfiles=$((nfiles + 1))
  rel=${ids#"$ROOT"/}
  suite=$(sed -n 's/^#[[:space:]]*suite=\([a-z0-9][a-z0-9-]*\).*$/\1/p' "$ids" | head -n 1)
  : > "$tmp/suite.ids"
  if [ -z "$suite" ]; then
    bad fault-suite-missing "$rel (no suite header)"
  elif [ -f "$ROOT/data/expected/tests/$suite.ids" ]; then
    awk '/^#/ { next } NF { print $1 }' "$ROOT/data/expected/tests/$suite.ids" | sort -u > "$tmp/suite.ids"
  else
    bad fault-suite-missing "$rel (data/expected/tests/$suite.ids missing)"
    suite=
  fi
  # Result files that exist ([ -f ], a builtin), by fault id, for the awk pass below.
  : > "$tmp/present"
  if [ -n "$suite" ]; then
    for r in "$gate"/faults/*/"$suite.txt"; do
      [ -f "$r" ] || continue
      r=${r%/*}
      printf '%s\n' "${r##*/}" >> "$tmp/present"
    done
  fi
  # One awk pass judges every fault of this file (D-45 lever, plan 04-LEVER-01: it replaces about
  # 17 processes per fault with one per file and reaches the same findings and counts). It
  # appends its findings to $find, the ids it accepts to seen (and the non-GEN_ ones to
  # fault-ids), and prints `<faults> <lines> <ok>`. A fault is ok when it adds no finding.
  # Fields split on blanks, as `set -- $l` did; the prefixes split on `|` as the shell's
  # IFS='|' field splitting did (a trailing `|` adds no field). An empty field, as in `a||b` or
  # `|a`, is refused as fault-unknown-vector and never matches (review IN-07: an explicit length
  # test, since index(v, "") is 1 on BWK awk but 0 on gawk and mawk).
  counts=$(awk -v rel="$rel" -v suite="$suite" -v gate="$gate" -v nonce="$nonce" \
    -v seenf="$tmp/seen" -v idsf="$tmp/fault-ids" -v suitef="$tmp/suite.ids" \
    -v presentf="$tmp/present" -v findf="$find" '
    function bad(t, d) { print t " " d >> findf; nfind++ }
    # Judge result file f of fault id (must-fail mf, prefixes pre, must-pass mp).
    function judge(id, f, mf, pre, mp,   line, a, b, v, have, gotnonce, n, k, hasempty, dup, \
                   nuniq, uniq, okv, notv, nnot, notlist, p, np, matched, i, j) {
      have = ""; gotnonce = 0; hasempty = 0; dup = 0; nuniq = 0; nnot = 0
      split("", uniq); split("", okv); split("", notv); split("", notlist)
      while ((getline line < f) > 0) {
        n = split(line, a)
        if (line ~ /^# suite=[^ ]* nonce=/) {
          if (!gotnonce) { split(a[3], b, "="); have = b[2]; gotnonce = 1 }
          continue
        }
        if (line ~ /^#/) continue
        if (line ~ /^ok [0-9][0-9]* [^ ]/) { v = a[3]; okv[v] = 1 }
        else if (line ~ /^not ok [0-9][0-9]* [^ ]/) { v = a[4]; notv[v] = 1; notlist[++nnot] = v }
        else continue
        if (v == "") { hasempty = 1; continue }
        if (v in uniq) dup = 1
        else { uniq[v] = 1; nuniq++ }
      }
      close(f)
      if (nonce == "" || have != nonce) bad("fault-stale", id)
      k = (dup || hasempty || nuniq != nsids)
      if (!k) for (v in uniq) if (!(v in sids)) { k = 1; break }
      if (k) bad("fault-list-mismatch", id)
      if (mf in okv) bad("fault-not-detected", id " " mf)
      if (mp in notv) bad("fault-collateral", id " " mp)
      np = split(pre, p, "|")
      if (np > 1 && p[np] == "") np--
      for (i = 1; i <= nnot; i++) {
        v = notlist[i]
        if (v == "") continue
        matched = 0
        # An empty field never matches (an explicit length test: index(v, "") is 1 on BWK awk
        # and 0 on gawk and mawk); a line with one is refused as fault-unknown-vector below.
        for (j = 1; j <= np; j++) if (length(p[j]) > 0 && index(v, p[j]) == 1) { matched = 1; break }
        if (!matched) bad("fault-wrong-reason", id " " v)
      }
    }
    BEGIN {
      while ((getline l < seenf) > 0) seen[l] = 1
      close(seenf)
      nsids = 0
      while ((getline l < suitef) > 0) { sids[l] = 1; nsids++ }
      close(suitef)
      while ((getline l < presentf) > 0) present[l] = 1
      close(presentf)
      total = 0; nlines = 0; ok = 0; nfind = 0
    }
    /^#/ { next }
    NF == 0 { next }
    {
      id = $1
      total++; nlines++
      before = nfind
      if (id ~ /[^A-Z0-9_]/) { bad("fault-missing-site", id " (bad id)"); next }
      if (id in seen) { bad("fault-duplicate-id", id " (" rel ")"); next }
      seen[id] = 1
      print id >> seenf
      if (id !~ /^GEN_/) print id >> idsf
      # A file whose suite is unusable is already a finding; its ids still count for the
      # duplicate and site checks, but there is nothing to judge them against.
      if (suite == "") next
      if (NF != 4) { bad("fault-unknown-vector", id " (line needs 4 columns)"); next }
      if (!($2 in sids)) bad("fault-unknown-vector", id " " $2)
      if (!($4 in sids)) bad("fault-unknown-vector", id " " $4)
      nq = split($3, q, "|")
      if (nq > 1 && q[nq] == "") nq--
      for (k = 1; k <= nq; k++) if (length(q[k]) == 0) { bad("fault-unknown-vector", id " (empty prefix)"); break }
      if (!(id in present)) { bad("fault-missing", id); next }
      judge(id, gate "/faults/" id "/" suite ".txt", $2, $3, $4)
      if (nfind == before) ok++
    }
    END {
      close(findf); close(seenf); close(idsf)
      print total, nlines, ok
    }' "$ids") || counts=
  case $counts in
    *[!0-9' ']*|'') bad fault-missing "$rel (judge failed)"; continue ;;
  esac
  set -- $counts
  total=$((total + $1))
  nlines=$2
  ok=$((ok + $3))
  [ "$nlines" -gt 0 ] || bad fault-missing "$rel (no faults)"
done
[ "$nfiles" -gt 0 ] || bad fault-missing "data/expected/faults/*.ids (none)"

# Sites in the owner sources (core/src and runner/, D-24) against the union of the non-GEN_
# fault ids of every file (H3).
: > "$tmp/sites"
for p in $(grep -E '^(core/src|runner)/.*[.](c|h)$' "$FILELIST" | sort -u); do
  [ -f "$ROOT/$p" ] || continue
  grep -E '^[[:space:]]*#[[:space:]]*if' "$ROOT/$p" \
    | grep -E 'IVS_FAULT_[A-Za-z0-9_]' \
    | sed -n 's/^.*IVS_FAULT_\([A-Za-z0-9_]*\).*$/\1/p' \
    | while IFS= read -r s; do printf '%s %s\n' "$s" "$p"; done >> "$tmp/sites"
done
sort -u "$tmp/fault-ids" -o "$tmp/fault-ids"
awk '{ print $1 }' "$tmp/sites" | sort -u > "$tmp/site-ids"
while IFS= read -r l; do
  s=${l%% *}
  grep -q -x -F -e "$s" "$tmp/fault-ids" || bad fault-orphan-site "IVS_FAULT_$s ${l#* }"
done < "$tmp/sites"
for s in $(comm -23 "$tmp/fault-ids" "$tmp/site-ids"); do bad fault-missing-site "$s"; done

if [ -s "$find" ]; then
  sort -u "$find" | while IFS= read -r l; do
    printf -- '-- FAIL %s: %s\n' "${l%% *}" "${l#* }"
  done
  exit 1
fi
printf -- '-- faults-verdict pass faults=%s/%s\n' "$ok" "$total"
