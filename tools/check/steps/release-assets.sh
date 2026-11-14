#!/bin/sh
# SPDX-License-Identifier: MIT
# Step release-assets: the content-based release asset check stays alive in the gate (plan 08-01, D-32,
# threat T-08-02). tools/check/release-assets.sh is a library that judges the MEMBERS of a release
# archive, never its outer file name (binary-ext.txt lists gz and tar, so an outer-name rule would refuse
# the very .tar.gz assets); tools/release.sh runs it on the real assets at release time. This step
#   (a) scans every FILELIST path (the source archive of a release is git ls-files, which FILELIST equals
#       once everything is committed) for a last extension on tools/check/binary-ext.txt, and
#   (b) selftests the library on synthetic archives built under a temp dir with tar -czf. The cases are
#       exactly the names in data/expected/release-assets-selftests.ids (first token of each non-comment
#       line); a listed case this script does not know is a failure, so the inventory and the script
#       cannot drift apart silently. Every case but outer-name-pass must fail for its intended token;
#       outer-name-pass is the one positive case and must exit 0:
#         source-extra-member   a member that is not in the list is reported by name
#         source-missing-member a listed path without a member is reported by name
#         install-extra-member  an extra library member in an install archive is reported by name
#         rom-member            a .rom member that the list itself names is still refused (the binary
#                               rule does not depend on the list)
#         outer-name-pass       a clean archive whose outer name is ivs-test.tar.gz passes
# Reads ROOT, CHECK_DIR and FILELIST only. Every temp file is under one mktemp dir removed on exit.
# Tokens: release-assets-source-binary: <path>, release-assets-ext-list: <what>,
#   release-selftest-inventory: <what>, release-selftest-missing: <case>, release-selftest-<case>: <detail>.
# Stats: release-selftest=<ok>/<total>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/release-assets.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }
helper="$CHECK_DIR/release-assets.sh"
inv="$ROOT/data/expected/release-assets-selftests.ids"
extfile="$CHECK_DIR/binary-ext.txt"

# (a) Tree-level scan: a FILELIST path whose last extension is on the binary list.
if [ ! -f "$extfile" ] || ! grep -v -e '^[[:space:]]*#' -e '^[[:space:]]*$' "$extfile" | grep -q .; then
  bad release-assets-ext-list "binary-ext.txt"
else
  awk -v extfile="$extfile" '
    BEGIN {
      while ((getline l < extfile) > 0) {
        sub(/^[ \t]+/, "", l)
        if (l == "" || substr(l, 1, 1) == "#") continue
        sub(/[ \t].*$/, "", l)
        bin[tolower(l)] = 1
      }
      close(extfile)
    }
    {
      base = $0
      sub(/^.*\//, "", base)
      k = split(base, part, ".")
      if (k >= 2 && (tolower(part[k]) in bin)) print
    }' "$FILELIST" | sort > "$tmp/binhits"
  sed -n '1,20p' "$tmp/binhits" | while IFS= read -r p; do
    printf -- '-- FAIL release-assets-source-binary: %s\n' "$p"
  done
  [ -s "$tmp/binhits" ] && rc=1
fi

# (b) Selftest of the library.
# mkarch <case> <archive name> <prefix> <member>...: $tmp/<case>/<name> holds the members (one line of
# text each) under <prefix>/. COPYFILE_DISABLE keeps macOS tar from adding metadata members.
mkarch() {
  ma_case=$1
  ma_name=$2
  ma_prefix=$3
  shift 3
  ma_src="$tmp/$ma_case/src"
  mkdir -p "$ma_src/$ma_prefix" || return 1
  for ma_m in "$@"; do
    mkdir -p "$(dirname "$ma_src/$ma_prefix/$ma_m")" || return 1
    printf 'x\n' > "$ma_src/$ma_prefix/$ma_m" || return 1
  done
  (cd "$ma_src" && COPYFILE_DISABLE=1 tar -czf "../$ma_name" "$ma_prefix") > "$tmp/$ma_case/tar.out" 2>&1
}
# runlib <case> <mode> <archive name> <prefix> <list entry>...: runs the library on the archive of the
# case against a list of the entries; output in $tmp/<case>/lib.out, exit status in $lib_st.
runlib() {
  rl_case=$1
  rl_mode=$2
  rl_name=$3
  rl_prefix=$4
  shift 4
  : > "$tmp/$rl_case/list"
  for rl_e in "$@"; do printf '%s\n' "$rl_e" >> "$tmp/$rl_case/list"; done
  lib_st=0
  CHECK_DIR="$CHECK_DIR" sh "$helper" "$rl_mode" "$tmp/$rl_case/$rl_name" "$rl_prefix" "$tmp/$rl_case/list" \
    > "$tmp/$rl_case/lib.out" 2>&1 || lib_st=$?
}
# wantfail <case> <line>: the last library run exited 1 and printed exactly this line.
wantfail() {
  [ "$lib_st" -eq 1 ] || { detail="library exit $lib_st, want 1"; return 1; }
  grep -q -x -F -e "$2" "$tmp/$1/lib.out" || { detail="no line '$2'"; return 1; }
  return 0
}

# Runs case $1: status 0 behaved as stated, 1 did not ($detail says why), 2 unknown case.
run_case() {
  detail=
  c=$1
  case $c in
    source-extra-member)
      mkdir -p "$tmp/$c" && mkarch "$c" a.tar.gz ivs-x a b extra || { detail="cannot build the archive"; return 1; }
      runlib "$c" source a.tar.gz ivs-x a b
      wantfail "$c" 'release-assets-extra-member: extra' || return 1
      ;;
    source-missing-member)
      mkdir -p "$tmp/$c" && mkarch "$c" a.tar.gz ivs-x a || { detail="cannot build the archive"; return 1; }
      runlib "$c" source a.tar.gz ivs-x a b
      wantfail "$c" 'release-assets-missing-member: b' || return 1
      ;;
    install-extra-member)
      mkdir -p "$tmp/$c" && mkarch "$c" a.tar.gz ivs-x include/ivs/ivs.h lib/libivs_core.a lib/libextra.a \
        || { detail="cannot build the archive"; return 1; }
      runlib "$c" install a.tar.gz ivs-x include/ivs/ivs.h lib/libivs_core.a
      wantfail "$c" 'release-assets-extra-member: lib/libextra.a' || return 1
      ;;
    rom-member)
      mkdir -p "$tmp/$c" && mkarch "$c" a.tar.gz ivs-x include/ivs/ivs.h share/doc/ivs/diag.rom \
        || { detail="cannot build the archive"; return 1; }
      runlib "$c" install a.tar.gz ivs-x include/ivs/ivs.h share/doc/ivs/diag.rom
      wantfail "$c" 'release-assets-binary-member: share/doc/ivs/diag.rom' || return 1
      if grep -q -e 'release-assets-extra-member' -e 'release-assets-missing-member' "$tmp/$c/lib.out"; then
        detail="the list was not honoured, so the binary rule is not the only finding"
        return 1
      fi
      ;;
    outer-name-pass)
      mkdir -p "$tmp/$c" && mkarch "$c" ivs-test.tar.gz ivs-test include/ivs/ivs.h lib/libivs_core.a \
        || { detail="cannot build the archive"; return 1; }
      runlib "$c" install ivs-test.tar.gz ivs-test include/ivs/ivs.h lib/libivs_core.a
      [ "$lib_st" -eq 0 ] || { detail="library exit $lib_st, want 0 for a clean archive named .tar.gz"; return 1; }
      grep -q -x -F -e 'release-assets install ok members=2' "$tmp/$c/lib.out" || { detail="no ok line"; return 1; }
      ;;
    *) return 2 ;;
  esac
  return 0
}

if [ ! -f "$helper" ]; then
  bad release-selftest-inventory "tools/check/release-assets.sh"
  exit 1
fi
if [ ! -f "$inv" ]; then
  bad release-selftest-inventory "data/expected/release-assets-selftests.ids"
  exit 1
fi
awk '/^#/ { next } NF { print $1 }' "$inv" > "$tmp/cases"
[ -s "$tmp/cases" ] || bad release-selftest-inventory "no cases"
ok=0
total=0
while IFS= read -r name; do
  total=$((total + 1))
  st=0
  run_case "$name" < /dev/null || st=$?
  case $st in
    0) ok=$((ok + 1)) ;;
    2) bad release-selftest-missing "$name" ;;
    *) bad "release-selftest-$name" "$detail" ;;
  esac
done < "$tmp/cases"

[ "$rc" -eq 0 ] || exit 1
printf -- '-- release-assets pass release-selftest=%s/%s\n' "$ok" "$total"
