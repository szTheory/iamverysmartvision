#!/bin/sh
# SPDX-License-Identifier: MIT
# Negative-control runner (SKELETON.md section 3).
# Each control dir holds CHECK (step name), EXPECT (one token per line), optional
# FORBID, overlay/ (copied over a fresh copy of _good/tree) and MAKE (transform lines).
# Env: CONTROLS_DIR (default tools/check/controls), CONTROLS_INVENTORY_DIR
# (default data/expected/controls). Final line: controls=<ok>/<total>.
LC_ALL=C; export LC_ALL
here=$(cd "$(dirname "$0")" && pwd) || exit 1
CHECK_DIR=$here
ROOT=$(cd "$here/../.." && pwd) || exit 1
CONTROLS_DIR=${CONTROLS_DIR:-$CHECK_DIR/controls}
CONTROLS_INVENTORY_DIR=${CONTROLS_INVENTORY_DIR:-$ROOT/data/expected/controls}
export CHECK_DIR
tmp=$(mktemp -d "${TMPDIR:-/tmp}/ctl.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
bad=0
total=0
ok=0
fail() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; bad=1; }

# Apply MAKE lines inside tree $1 from file $2. Bad bytes exist only here, assembled at run time.
apply_make() {
  _t=$1
  while IFS= read -r _line || [ -n "$_line" ]; do
    set -f
    # shellcheck disable=SC2086
    set -- $_line
    set +f
    [ "$#" -gt 0 ] || continue
    case $1 in '#'*) continue ;; esac
    _verb=$1
    _p=${2:-}
    case $_p in
      /*|*..*) fail control-make-path "$_p"; return 1 ;;
    esac
    case $_verb in
      delete) rm -f "$_t/$_p" ;;
      empty) : > "$_t/$_p" ;;
      write) mkdir -p "$(dirname "$_t/$_p")"; printf '%s\n' "${3:-x}" > "$_t/$_p" ;;
      rename)
        case ${3:-} in /*|*..*) fail control-make-path "${3:-}"; return 1 ;; esac
        mkdir -p "$(dirname "$_t/${3:-}")"; mv "$_t/$_p" "$_t/${3:-}" ;;
      crlf) awk '{ printf "%s\r\n", $0 }' "$_t/$_p" > "$_t/$_p.tmp" && mv "$_t/$_p.tmp" "$_t/$_p" ;;
      bom) { printf '\357\273\277'; cat "$_t/$_p"; } > "$_t/$_p.tmp" && mv "$_t/$_p.tmp" "$_t/$_p" ;;
      nonascii) printf '\303\251' >> "$_t/$_p" ;;
      homepath) printf '/%s/%s/x\n' Users ctlexample >> "$_t/$_p" ;;
      secret) printf '%s%s_%s\n' gh p abcdefghijklmnopqrstuvwxyz0123456789 >> "$_t/$_p" ;;
      password) printf '%s%s=%s\n' pass word Abcdefgh1234 >> "$_t/$_p" ;;
      email) printf '%s@%s\n' ctl mail.test >> "$_t/$_p" ;;
      *) fail control-make-verb "$_verb"; return 1 ;;
    esac
  done < "$2"
  return 0
}

# Build the temp tree $1 for control dir $2 (empty $2 = pristine good tree).
build_tree() {
  rm -rf "$1"
  mkdir -p "$1"
  cp -R "$CONTROLS_DIR/_good/tree/." "$1/" || return 1
  if [ -n "$2" ]; then
    if [ -d "$2/overlay" ]; then cp -R "$2/overlay/." "$1/" || return 1; fi
    if [ -f "$2/MAKE" ]; then apply_make "$1" "$2/MAKE" || return 1; fi
  fi
  (cd "$1" && find . -type f | sed 's|^\./||' | sort) > "$tmp/filelist"
  return 0
}

run_step() {
  ROOT="$tmp/tree" FILELIST="$tmp/filelist" CHECK_DIR="$CHECK_DIR" sh "$CHECK_DIR/steps/$1.sh" > "$tmp/out" 2>&1
}

# Control directories and inventory.
: > "$tmp/ctl.have"
for d in "$CONTROLS_DIR"/*/; do
  [ -d "$d" ] || continue
  n=$(basename "$d")
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

# Positive control: every distinct CHECK step must pass on the pristine good tree.
: > "$tmp/checks"
for n in $(cat "$tmp/ctl.have"); do
  [ -f "$CONTROLS_DIR/$n/CHECK" ] && head -n 1 "$CONTROLS_DIR/$n/CHECK" >> "$tmp/checks"
done
sort -u "$tmp/checks" -o "$tmp/checks"
for step in $(cat "$tmp/checks"); do
  if [ ! -f "$CHECK_DIR/steps/$step.sh" ]; then continue; fi
  build_tree "$tmp/tree" "" || { fail control-good-failed "$step"; continue; }
  if run_step "$step" && grep -q -F -e "-- $step pass" "$tmp/out"; then
    printf 'control ok   _good/%s\n' "$step"
  else
    fail control-good-failed "$step"
    cat "$tmp/out"
  fi
done

# Negative controls.
for n in $(cat "$tmp/ctl.have"); do
  d="$CONTROLS_DIR/$n"
  total=$((total + 1))
  step=
  [ -f "$d/CHECK" ] && step=$(head -n 1 "$d/CHECK")
  if [ -z "$step" ] || [ ! -f "$CHECK_DIR/steps/$step.sh" ]; then
    fail control-unknown-step "$n"; continue
  fi
  if [ ! -s "$d/EXPECT" ] || grep -q '^[[:space:]]*$' "$d/EXPECT"; then
    fail control-expect-blank "$n"; continue
  fi
  build_tree "$tmp/tree" "$d" || { fail control-build "$n"; continue; }
  if run_step "$step"; then
    fail control-exit-zero "$n"; continue
  fi
  missing=0
  while IFS= read -r tok || [ -n "$tok" ]; do
    grep -q -F -e "$tok" "$tmp/out" || missing=1
  done < "$d/EXPECT"
  if [ "$missing" -ne 0 ]; then
    fail control-token-missing "$n"; continue
  fi
  forbidden=0
  if [ -f "$d/FORBID" ]; then
    while IFS= read -r tok || [ -n "$tok" ]; do
      [ -n "$tok" ] || continue
      if grep -q -F -e "$tok" "$tmp/out"; then forbidden=1; fi
    done < "$d/FORBID"
  fi
  if [ "$forbidden" -ne 0 ]; then
    fail control-forbidden-token "$n"; continue
  fi
  printf 'control ok   %s\n' "$n"
  ok=$((ok + 1))
done

printf 'controls=%s/%s\n' "$ok" "$total"
[ "$bad" -eq 0 ]
