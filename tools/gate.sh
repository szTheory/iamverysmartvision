#!/bin/sh
# SPDX-License-Identifier: MIT
# The single local/CI command: sh tools/gate.sh (no arguments). See SKELETON.md section 1.
LC_ALL=C; export LC_ALL
here=$(cd "$(dirname "$0")" && pwd) || exit 1
ROOT=$(cd "$here/.." && pwd) || exit 1
CHECK_DIR="$ROOT/tools/check"
GATE_TMP=$(mktemp -d "${TMPDIR:-/tmp}/gate.XXXXXX") || exit 1
trap 'rm -rf "$GATE_TMP"' EXIT
trap 'exit 1' HUP INT TERM
FILELIST="$GATE_TMP/filelist"
export ROOT CHECK_DIR FILELIST GATE_TMP
fail=0
tokens=""
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; fail=1; }

command -v cmake >/dev/null 2>&1 || { bad gate-no-cmake cmake; printf 'gate: FAIL\n'; exit 1; }
command -v ninja >/dev/null 2>&1 || { bad gate-no-ninja ninja; printf 'gate: FAIL\n'; exit 1; }
command -v "${CC:-cc}" >/dev/null 2>&1 || { bad gate-no-cc "${CC:-cc}"; printf 'gate: FAIL\n'; exit 1; }

# FILELIST: tracked plus untracked-not-ignored paths. A failing git must not read as a clean list.
cd "$ROOT" || exit 1
if ! git -c core.quotePath=false ls-files --cached --others --exclude-standard > "$GATE_TMP/files.raw"; then
  bad gate-filelist "git ls-files failed"
  printf 'gate: FAIL\n'
  exit 1
fi
: > "$GATE_TMP/files.keep"
while IFS= read -r f; do
  case $f in
    *[!A-Za-z0-9._/-]*) printf '%s\n' "$f" >> "$GATE_TMP/files.keep" ;;
    *) if [ -e "$f" ]; then printf '%s\n' "$f" >> "$GATE_TMP/files.keep"; fi ;;
  esac
done < "$GATE_TMP/files.raw"
sort -u "$GATE_TMP/files.keep" > "$FILELIST"

# Results directory contract: a fresh build/gate/ per run; NONCE is the basename of this run's temp dir.
# The C steps export-read IVS_GATE_DIR and IVS_TEST_NONCE; build/ is ignored, so nothing enters FILELIST.
NONCE=$(basename "$GATE_TMP")
IVS_GATE_DIR="$ROOT/build/gate"
if rm -rf "$IVS_GATE_DIR" && mkdir -p "$IVS_GATE_DIR" && printf '%s\n' "$NONCE" > "$IVS_GATE_DIR/NONCE"; then :; else
  bad gate-results-dir build/gate
  printf 'gate: FAIL\n'
  exit 1
fi
IVS_TEST_NONCE=$NONCE
export IVS_GATE_DIR IVS_TEST_NONCE

# Step inventory: step scripts must equal the union of data/expected/gate-steps/*.ids.
: > "$GATE_TMP/steps.have"
for s in "$CHECK_DIR"/steps/*.sh; do
  [ -f "$s" ] || continue
  basename "$s" .sh >> "$GATE_TMP/steps.have"
done
sort -u "$GATE_TMP/steps.have" -o "$GATE_TMP/steps.have"
: > "$GATE_TMP/steps.want"
for f in "$ROOT"/data/expected/gate-steps/*.ids; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } NF { print $1 }' "$f" >> "$GATE_TMP/steps.want"
done
sort -u "$GATE_TMP/steps.want" -o "$GATE_TMP/steps.want"
if [ ! -s "$GATE_TMP/steps.want" ]; then bad gate-steps-empty "data/expected/gate-steps"; fi
want=$(grep -c . "$GATE_TMP/steps.want")
for name in $(comm -23 "$GATE_TMP/steps.want" "$GATE_TMP/steps.have"); do bad gate-step-missing "$name"; done
for name in $(comm -13 "$GATE_TMP/steps.want" "$GATE_TMP/steps.have"); do bad gate-step-extra "$name"; done

# Run every step (sorted, never stopping at the first failure).
passed=0
for name in $(cat "$GATE_TMP/steps.have"); do
  if sh "$CHECK_DIR/steps/$name.sh" > "$GATE_TMP/step.out" 2>&1; then
    cat "$GATE_TMP/step.out"
    if grep -q -F -e "-- $name pass" "$GATE_TMP/step.out"; then
      passed=$((passed + 1))
      toks=$(sed -n "s/^-- $name pass //p" "$GATE_TMP/step.out" | tr '\n' ' ')
      tokens="$tokens$toks"
    else
      bad gate-step-no-pass-line "$name"
    fi
  else
    cat "$GATE_TMP/step.out"
    bad gate-step-failed "$name"
  fi
done

# Negative controls.
ctl_ok=0
ctl_total=0
if sh "$CHECK_DIR/run-controls.sh" > "$GATE_TMP/controls.out" 2>&1; then :; else bad gate-controls-failed "run-controls"; fi
cat "$GATE_TMP/controls.out"
line=$(sed -n 's/^controls=\([0-9][0-9]*\)\/\([0-9][0-9]*\)$/\1 \2/p' "$GATE_TMP/controls.out" | tail -n 1)
if [ -n "$line" ]; then
  ctl_ok=${line% *}
  ctl_total=${line#* }
else
  bad gate-controls-line "no controls=<ok>/<total> line"
fi
if [ "$ctl_total" -eq 0 ]; then bad gate-controls-empty "no controls ran"; fi

status=PASS
[ "$fail" -eq 0 ] || status=FAIL
tokens=${tokens% }
printf 'gate: %s steps=%s/%s controls=%s/%s %s\n' "$status" "$passed" "$want" "$ctl_ok" "$ctl_total" "$tokens"
[ "$fail" -eq 0 ]
