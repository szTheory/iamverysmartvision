#!/bin/sh
# SPDX-License-Identifier: MIT
# Step tests-inventory: pure checker over the TAP result files the c-tests step wrote (D-17,
# RUN-03). Reads only the results dir and $ROOT/data/expected/; never builds, never runs a
# test, so the control runner can drive it with text overlays. Results dir: IVS_GATE_DIR (set
# by tools/gate.sh, and by the control runner to its tree's build/gate), else $ROOT/build/gate,
# the same default the producer c-tests uses (review IN-07); build/gate below names it.
# Presets: dev release. Suites: stems of data/expected/tests/*.ids (first token per non-#
# line is an expected id; a second token `skip-ok` permits that id to be skipped).
# Per preset: build/gate/NONCE must exist; the `Test #N: <name>` lines of
# build/gate/<preset>/ctest-list.txt must equal the suite set. Per suite and preset:
# build/gate/<preset>/<suite>.txt must exist, carry the header `# suite=<s> nonce=<NONCE>`,
# a `1..N` plan equal to its result lines and a trailer whose counts match; ids are unique,
# none failed, and the result id set equals the ids file; every `# source <path> <sha256>`
# line is re-hashed.
# Tokens: tests-missing, tests-unlisted, tests-duplicate, tests-unexpected-skip, tests-stale,
#   tests-truncated, tests-plan-mismatch, tests-failed, tests-suite-missing,
#   tests-inventory-empty, tests-ctest-drift, tests-vectors-digest.
# Stats: tests-expected=<ids x presets> tests-executed=<n> tests-skipped=<k> tests-missing=0.
# Output (plan 07-10, D-31): before the pass line (and before any FAIL line) one line
#   `-- SKIPPED <preset>/<suite>/<id>: <reason>` per skipped id and preset, the reason being the text
#   after `# SKIP` in the result line; the number of such lines equals `tests-skipped=`.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/tests-inventory.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
find="$tmp/findings"
: > "$find"
bad() { printf '%s %s\n' "$1" "$2" >> "$find"; }

presets="dev release"
gate=${IVS_GATE_DIR:-$ROOT/build/gate}

# Suites and their expected ids.
: > "$tmp/suites"
for f in "$ROOT"/data/expected/tests/*.ids; do
  [ -f "$f" ] || continue
  basename "$f" .ids >> "$tmp/suites"
done
sort -u "$tmp/suites" -o "$tmp/suites"
[ -s "$tmp/suites" ] || bad tests-inventory-empty "data/expected/tests"
expected=0
for s in $(cat "$tmp/suites"); do
  awk '/^#/ { next } NF { print $1 }' "$ROOT/data/expected/tests/$s.ids" | sort -u > "$tmp/$s.want"
  awk '/^#/ { next } NF >= 2 && $2 == "skip-ok" { print $1 }' "$ROOT/data/expected/tests/$s.ids" \
    | sort -u > "$tmp/$s.skipok"
  n=$(grep -c . "$tmp/$s.want")
  [ "$n" -gt 0 ] || bad tests-inventory-empty "$s"
  for p in $presets; do expected=$((expected + n)); done
done

nonce=
if [ -f "$gate/NONCE" ]; then
  nonce=$(head -n 1 "$gate/NONCE")
fi
[ -n "$nonce" ] || bad tests-stale "NONCE"

executed=0
skipped=0
: > "$tmp/sources"
: > "$tmp/skipped.lines"
for p in $presets; do
  # ctest -N listing against the suite set.
  if [ -f "$gate/$p/ctest-list.txt" ]; then
    sed -n 's/^[[:space:]]*Test[[:space:]]*#[0-9][0-9]*:[[:space:]]*\([^[:space:]]*\).*$/\1/p' \
      "$gate/$p/ctest-list.txt" | sort -u > "$tmp/ctest.$p"
  else
    : > "$tmp/ctest.$p"
    bad tests-ctest-drift "$p/ctest-list.txt"
  fi
  for name in $(comm -3 "$tmp/suites" "$tmp/ctest.$p" | tr -d '\t'); do
    bad tests-ctest-drift "$p/$name"
  done

  for s in $(cat "$tmp/suites"); do
    f="$gate/$p/$s.txt"
    if [ ! -f "$f" ]; then
      bad tests-suite-missing "$p/$s"
      continue
    fi
    awk -v suite="$s" -v nonce="$nonce" -v tag="$p/$s" '
      function tok(t, d) { print t " " d }
      /^# suite=[^ ]* nonce=/ {
        if (header) { tok("tests-stale", tag " (second header)") }
        header = 1
        split($2, a, "="); split($3, b, "=")
        if (a[2] != suite || b[2] != nonce || NF != 3) { tok("tests-stale", tag) }
        next
      }
      /^# suite=[^ ]* executed=[0-9]* skipped=[0-9]* failed=[0-9]*$/ {
        trailer = 1
        split($3, e, "="); split($4, k, "="); split($5, x, "=")
        t_exec = e[2] + 0; t_skip = k[2] + 0; t_fail = x[2] + 0
        next
      }
      /^# source / {
        if (NF != 4) { tok("tests-vectors-digest", tag " (malformed source line)"); next }
        print "SOURCE " $3 " " $4
        next
      }
      /^#/ { next }
      /^1\.\.[0-9][0-9]*$/ {
        if (plan != "") { tok("tests-plan-mismatch", tag " (second plan)") }
        plan = substr($0, 4) + 0
        next
      }
      /^ok [0-9][0-9]* [^ ]/ || /^not ok [0-9][0-9]* [^ ]/ {
        if (trailer) { tok("tests-truncated", tag " (result after trailer)") }
        n++
        if ($1 == "ok") { num = $2; id = $3; skip = ($4 == "#" && $5 == "SKIP") }
        else { num = $3; id = $4; skip = 0; nfail++; tok("tests-failed", tag "/" id) }
        if (num + 0 != n) { tok("tests-plan-mismatch", tag " (result " num " out of sequence)") }
        if (id in seen) { tok("tests-duplicate", tag "/" id) }
        seen[id] = 1
        if (skip) {
          r = ""
          for (i = 6; i <= NF; i++) r = r (i > 6 ? " " : "") $i
          if (r == "") r = "no reason given"
          nskip++; print "SKIP " id " " r
        } else { nexec++; print "RUN " id }
        next
      }
      NF { tok("tests-truncated", tag " (unparsed line " NR ")") }
      END {
        if (!header) { tok("tests-stale", tag " (no header)") }
        if (!trailer) { tok("tests-truncated", tag) }
        if (plan == "" || plan + 0 != n) { tok("tests-plan-mismatch", tag " (plan " plan ", results " n ")") }
        else if (trailer && (t_exec != nexec || t_skip != nskip || t_fail != nfail)) {
          tok("tests-plan-mismatch", tag " (trailer counts)")
        }
        print "SUMMARY " nexec + 0 " " nskip + 0
      }' "$f" > "$tmp/awk.out"

    : > "$tmp/have"
    : > "$tmp/skips"
    while IFS= read -r l; do
      set -f
      # shellcheck disable=SC2086
      set -- $l
      set +f
      case $1 in
        RUN) printf '%s\n' "$2" >> "$tmp/have" ;;
        SKIP)
          printf '%s\n' "$2" >> "$tmp/have"
          printf '%s\n' "$2" >> "$tmp/skips"
          # The reason is everything after the id: the text after `# SKIP` in the result line.
          r=${l#SKIP }
          r=${r#* }
          printf '%s/%s/%s: %s\n' "$p" "$s" "$2" "$r" >> "$tmp/skipped.lines"
          ;;
        SOURCE) printf '%s %s\n' "$2" "$3" >> "$tmp/sources" ;;
        SUMMARY) executed=$((executed + $2)); skipped=$((skipped + $3)) ;;
        *) bad "$1" "${l#* }" ;;
      esac
    done < "$tmp/awk.out"
    sort -u "$tmp/have" -o "$tmp/have"
    sort -u "$tmp/skips" -o "$tmp/skips"
    for id in $(comm -23 "$tmp/$s.want" "$tmp/have"); do bad tests-missing "$p/$s/$id"; done
    for id in $(comm -13 "$tmp/$s.want" "$tmp/have"); do bad tests-unlisted "$p/$s/$id"; done
    for id in $(comm -23 "$tmp/skips" "$tmp/$s.skipok"); do bad tests-unexpected-skip "$p/$s/$id"; done
  done
done

# Re-hash every vector source named by a result file.
sort -u "$tmp/sources" -o "$tmp/sources"
while IFS= read -r l; do
  path=${l% *}
  want=${l#* }
  case $path in
    /*|*..*|*[!A-Za-z0-9._/-]*) bad tests-vectors-digest "$path (bad path)"; continue ;;
  esac
  if [ ! -f "$ROOT/$path" ]; then
    bad tests-vectors-digest "$path (missing)"
    continue
  fi
  got=$(cmake -E sha256sum "$ROOT/$path" 2> /dev/null) || { bad tests-vectors-digest "$path (hash failed)"; continue; }
  got=${got%% *}
  [ "$got" = "$want" ] || bad tests-vectors-digest "$path"
done < "$tmp/sources"

# One visible line per skipped id and preset (plan 07-10, D-31), in the order preset, suite, id of
# the loop above; not a finding and not a FAIL line, so the gate's FAIL summary ignores it. The
# `tests-skipped=` token below counts the same ids.
while IFS= read -r l; do
  printf -- '-- SKIPPED %s\n' "$l"
done < "$tmp/skipped.lines"

if [ -s "$find" ]; then
  sort -u "$find" | while IFS= read -r l; do
    printf -- '-- FAIL %s: %s\n' "${l%% *}" "${l#* }"
  done
  exit 1
fi
printf -- '-- tests-inventory pass tests-expected=%s tests-executed=%s tests-skipped=%s tests-missing=0\n' \
  "$expected" "$executed" "$skipped"
