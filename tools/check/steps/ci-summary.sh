#!/bin/sh
# SPDX-License-Identifier: MIT
# Step ci-summary: self-test of the job-summary helper tools/check/ci-summary.sh (D-42, D-44).
# The helper is not a step, so controls cannot drive it; this step runs the real helper on
# synthetic gate-tmp directories under its own temp dir and checks one assertion per case.
# The cases are exactly the names in data/expected/ci-summary-selftests.ids (first token of each
# non-comment line); a listed case this script does not know is a failure, so the inventory and
# the script cannot drift apart silently. Every helper run sets GITHUB_STEP_SUMMARY explicitly
# (to a temp file, an unwritable path, or unset), so a CI run never appends to the real summary.
# Reads ROOT, CHECK_DIR and FILELIST only (FILELIST is unused: the inputs are synthetic).
# Tokens: ci-summary-selftest-inventory: <what>, ci-summary-selftest-missing: <case>,
#   ci-summary-selftest-<case>: <detail>.
# Stats: ci-summary-selftest=<ok>/<total>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/cisummary.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }
helper="$CHECK_DIR/ci-summary.sh"
inv="$ROOT/data/expected/ci-summary-selftests.ids"

# Fresh synthetic gate-tmp dir $tmp/<case>/g and summary file $tmp/<case>/sum.md.
mkcase() { mkdir -p "$tmp/$1/g" && : > "$tmp/$1/sum.md"; }
# Render case $1 into $tmp/$1/sum.md; helper stdout and stderr go to $tmp/$1/noise.
render() { GITHUB_STEP_SUMMARY="$tmp/$1/sum.md" sh "$helper" "$tmp/$1/g" > "$tmp/$1/noise" 2>&1; }
# Table rows (lines starting with "| ", minus header and separator) of section $2 in file $1.
rows() {
  awk -v sec="#### $2" '
    $0 == sec { on = 1; next }
    /^#### / { on = 0 }
    on && /^\| / && $0 != "| --- | --- |" && $0 !~ /^\| (token|step) \|/ { print }' "$1"
}
# Lines between the opening and closing fence of the Failures section in file $1.
fails_block() {
  awk '/^#### Failures/ { on = 1; next } on && /^```$/ { n++; next } on && n == 1 { print }' "$1"
}

# Runs case $1: status 0 pass, 1 fail (detail in $detail), 2 unknown case.
run_case() {
  detail=
  c=$1
  case $c in
    render-headline)
      mkcase "$c"
      printf 'gate: PASS steps=3/3 controls=2/2 alpha=1\n' > "$tmp/$c/g/summary.line"
      render "$c" || { detail="helper exit nonzero"; return 1; }
      grep -q -x '### gate: PASS' "$tmp/$c/sum.md" || { detail="no status headline"; return 1; }
      grep -q -x 'steps=3/3 controls=2/2 alpha=1' "$tmp/$c/sum.md" || { detail="tokens absent"; return 1; }
      ;;
    skips-none)
      mkcase "$c"
      printf 'gate: PASS steps=3/3 alpha=0\n' > "$tmp/$c/g/summary.line"
      printf 'step-time a 1\n' > "$tmp/$c/g/step.times"
      render "$c" || { detail="helper exit nonzero"; return 1; }
      rows "$tmp/$c/sum.md" 'Skips and missing' > "$tmp/$c/rows"
      [ "$(grep -c . "$tmp/$c/rows")" -eq 1 ] && grep -q -x -F '| none | |' "$tmp/$c/rows" ||
        { detail="want exactly one none row"; return 1; }
      ;;
    skips-listed)
      mkcase "$c"
      printf 'gate: FAIL steps=3/3 tests-skipped=2 pkg-missing=1 alpha=5\n' > "$tmp/$c/g/summary.line"
      render "$c" || { detail="helper exit nonzero"; return 1; }
      rows "$tmp/$c/sum.md" 'Skips and missing' > "$tmp/$c/rows"
      grep -q -x -F '| tests-skipped | 2 |' "$tmp/$c/rows" || { detail="tests-skipped not listed"; return 1; }
      grep -q -x -F '| pkg-missing | 1 |' "$tmp/$c/rows" || { detail="pkg-missing not listed"; return 1; }
      [ "$(grep -c . "$tmp/$c/rows")" -eq 2 ] || { detail="unexpected rows"; return 1; }
      ;;
    slowest-eight)
      mkcase "$c"
      printf 'gate: PASS steps=12/12\n' > "$tmp/$c/g/summary.line"
      for p in s01:3 s02:17 s03:5 s04:11 s05:2 s06:13 s07:7 s08:19 s09:1 s10:23 s11:29 s12:4; do
        printf 'step-time %s %s\n' "${p%:*}" "${p#*:}" >> "$tmp/$c/g/step.times"
      done
      render "$c" || { detail="helper exit nonzero"; return 1; }
      got=$(rows "$tmp/$c/sum.md" 'Slowest steps' | awk '{ printf "%s:%s ", $2, $4 }')
      want='s11:29 s10:23 s08:19 s02:17 s06:13 s04:11 s07:7 s03:5 '
      [ "$got" = "$want" ] || { detail="got '$got'"; return 1; }
      ;;
    fail-cap-40)
      mkcase "$c"
      printf 'gate: FAIL steps=1/1\n' > "$tmp/$c/g/summary.line"
      i=1
      while [ "$i" -le 45 ]; do printf -- '-- FAIL tok-%s: detail\n' "$i" >> "$tmp/$c/g/fail.lines"; i=$((i + 1)); done
      render "$c" || { detail="helper exit nonzero"; return 1; }
      n=$(fails_block "$tmp/$c/sum.md" | grep -c '^-- FAIL ')
      [ "$n" -eq 40 ] || { detail="$n FAIL lines, want 40"; return 1; }
      fails_block "$tmp/$c/sum.md" | grep -q -x -F -e '-- FAIL tok-40: detail' || { detail="not the first 40"; return 1; }
      grep -q -x -F '(+5 more)' "$tmp/$c/sum.md" || { detail="no (+5 more) line"; return 1; }
      ;;
    backticks-stripped)
      mkcase "$c"
      printf 'gate: FAIL steps=1/1\n' > "$tmp/$c/g/summary.line"
      printf -- '-- FAIL tok: a`b```c` <x>\n' > "$tmp/$c/g/fail.lines"
      render "$c" || { detail="helper exit nonzero"; return 1; }
      fails_block "$tmp/$c/sum.md" | grep -q -x -F -e '-- FAIL tok: abc <x>' || { detail="line not rendered stripped"; return 1; }
      if grep '`' "$tmp/$c/sum.md" | grep -q -v -x '```'; then detail="backtick outside a fence line"; return 1; fi
      ;;
    unwritable-keeps-exit)
      mkcase "$c"
      printf 'gate: PASS steps=1/1\n' > "$tmp/$c/g/summary.line"
      h=0
      GITHUB_STEP_SUMMARY="$tmp/$c/no-such-dir/sum.md" sh "$helper" "$tmp/$c/g" > "$tmp/$c/noise" 2>&1 || h=$?
      [ "$h" -eq 0 ] || { detail="helper exit $h with an unwritable path"; return 1; }
      [ ! -s "$tmp/$c/noise" ] || { detail="helper printed output"; return 1; }
      # The gate's trap form: the shell's exit status must survive the helper in an EXIT trap.
      s=0
      GITHUB_STEP_SUMMARY="$tmp/$c/no-such-dir/sum.md" H="$helper" G="$tmp/$c/g" \
        sh -c 'trap '\''[ -z "${GITHUB_STEP_SUMMARY:-}" ] || sh "$H" "$G" > /dev/null 2>&1 || :'\'' EXIT; exit 3' \
        > "$tmp/$c/noise2" 2>&1 || s=$?
      [ "$s" -eq 3 ] || { detail="surrounding exit status $s, want 3"; return 1; }
      s=0
      GITHUB_STEP_SUMMARY="$tmp/$c/sum.md" H="$helper" G="$tmp/$c/g" \
        sh -c 'trap '\''[ -z "${GITHUB_STEP_SUMMARY:-}" ] || sh "$H" "$G" > /dev/null 2>&1 || :'\'' EXIT; false' \
        > "$tmp/$c/noise2" 2>&1 || s=$?
      [ "$s" -eq 1 ] || { detail="surrounding exit status $s after a written summary, want 1"; return 1; }
      ;;
    unset-writes-nothing)
      mkcase "$c"
      rm -f "$tmp/$c/sum.md"
      printf 'gate: PASS steps=1/1\n' > "$tmp/$c/g/summary.line"
      (unset GITHUB_STEP_SUMMARY; sh "$helper" "$tmp/$c/g") > "$tmp/$c/noise" 2>&1 || { detail="helper exit nonzero (unset)"; return 1; }
      GITHUB_STEP_SUMMARY= sh "$helper" "$tmp/$c/g" >> "$tmp/$c/noise" 2>&1 || { detail="helper exit nonzero (empty)"; return 1; }
      [ ! -s "$tmp/$c/noise" ] || { detail="helper printed output"; return 1; }
      ls "$tmp/$c" "$tmp/$c/g" > "$tmp/$c.ls" 2>&1
      if grep -q 'sum.md' "$tmp/$c.ls"; then detail="a file was created"; return 1; fi
      ;;
    cpu-dash-format)
      got=$(printf '0m0.000000s 0m0.000000s\n0m1.500000s 0m0.250000s\n' | sh "$helper" --cpu)
      [ "$got" = 'gate-cpu user=1.50 sys=0.25' ] || { detail="got '$got'"; return 1; }
      ;;
    cpu-sh-format)
      got=$(printf '0m0.002s 0m0.005s\n2m3.500s 0m0.750s\n' | sh "$helper" --cpu)
      [ "$got" = 'gate-cpu user=123.50 sys=0.75' ] || { detail="got '$got'"; return 1; }
      ;;
    cpu-garbage-unknown)
      got=$(printf 'garbage\n' | sh "$helper" --cpu)
      [ "$got" = 'gate-cpu unknown' ] || { detail="got '$got' for garbage"; return 1; }
      got=$(printf '0m0.0s 0m0.0s\n1.5 0.25\n' | sh "$helper" --cpu)
      [ "$got" = 'gate-cpu unknown' ] || { detail="got '$got' for bare seconds"; return 1; }
      ;;
    *) return 2 ;;
  esac
  return 0
}

[ -f "$inv" ] || { bad ci-summary-selftest-inventory "data/expected/ci-summary-selftests.ids"; exit 1; }
awk '/^#/ { next } NF { print $1 }' "$inv" > "$tmp/cases"
[ -s "$tmp/cases" ] || bad ci-summary-selftest-inventory "no cases"
ok=0
total=0
while IFS= read -r name; do
  total=$((total + 1))
  st=0
  run_case "$name" < /dev/null || st=$?
  case $st in
    0) ok=$((ok + 1)) ;;
    2) bad ci-summary-selftest-missing "$name" ;;
    *) bad "ci-summary-selftest-$name" "$detail" ;;
  esac
done < "$tmp/cases"

[ "$rc" -eq 0 ] || exit 1
printf -- '-- ci-summary pass ci-summary-selftest=%s/%s\n' "$ok" "$total"
