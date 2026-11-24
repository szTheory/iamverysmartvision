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
#       Five cases (plan 08-02, D-30) judge the version and changelog modes on synthetic inputs, each
#       a header with the three IVS_API_VERSION defines plus a config version file with a
#       PACKAGE_VERSION line, or a CHANGELOG.md:
#         version-skew             a tag that differs from the header, and a package version that
#                                  differs from the header, are each reported with both values
#         version-tag-form         the tag v1.0 is refused for its form
#         version-agree-pass       tag, header and package all 0.1.0: passes (the positive case)
#         changelog-section-missing a CHANGELOG with only an Unreleased section has no 0.1.0 section
#         changelog-section-pass   a CHANGELOG with a section headed ## [0.1.0] - draft passes
#       A last case, readiness-selftest, is listed only in the real inventory (the toy tree has no
#       readiness script): it runs tools/release-readiness.sh --selftest and requires its last line
#       selftest 4/4; a missing script while the case is listed is release-selftest-missing.
#   (c) holds the claim table of the release notes to the claim ledger (plan 08-02, D-27): when
#       CHANGELOG.md is in FILELIST, the lines strictly between the markers claims-table-begin and
#       claims-table-end must equal, byte for byte, the output of tools/check/claims-table.cmake on ROOT
#       (the table is rendered from data/claims.json, never typed), so a commit that changes
#       data/claims.json regenerates the table in the same commit; and when tools/release.sh is in
#       FILELIST but CHANGELOG.md is not, the release has no notes file.
# Reads ROOT, CHECK_DIR and FILELIST only. Every temp file is under one mktemp dir removed on exit.
# Tokens: release-assets-source-binary: <path>, release-assets-ext-list: <what>,
#   release-selftest-inventory: <what>, release-selftest-missing: <case>, release-selftest-<case>: <detail>,
#   release-changelog-claims-drift: CHANGELOG.md (the detail is markers when the marker pair is not exactly
#   one begin line followed by one end line), release-changelog-missing: CHANGELOG.md, and the tokens of
#   claims-table.cmake when it cannot render (release-claims-measured-hardware, release-claims-unreadable).
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

# mkver <case> <header version> <config version>: a synthetic ivs.h with the three version defines (the u
# suffix as in the real header) and a synthetic config version file, in $tmp/<case>.
mkver() {
  mv_case=$1
  mkdir -p "$tmp/$mv_case" || return 1
  mv_rest=${2#*.}
  printf '#define IVS_API_VERSION_MAJOR %su\n#define IVS_API_VERSION_MINOR %su\n#define IVS_API_VERSION_PATCH %su\n' \
    "${2%%.*}" "${mv_rest%%.*}" "${mv_rest#*.}" > "$tmp/$mv_case/ivs.h" || return 1
  printf 'set(PACKAGE_VERSION "%s")\n' "$3" > "$tmp/$mv_case/ivsConfigVersion.cmake"
}
# runver <case> <tag>: the version mode on the synthetic inputs; output in $tmp/<case>/lib.out, status in $lib_st.
runver() {
  lib_st=0
  sh "$helper" version "$2" "$tmp/$1/ivs.h" "$tmp/$1/ivsConfigVersion.cmake" > "$tmp/$1/lib.out" 2>&1 || lib_st=$?
}
# runchg <case> <version>: the changelog mode on $tmp/<case>/CHANGELOG.md.
runchg() {
  lib_st=0
  sh "$helper" changelog "$2" "$tmp/$1/CHANGELOG.md" > "$tmp/$1/lib.out" 2>&1 || lib_st=$?
}
# wantpass <case> <line>: the last library run exited 0 and printed exactly this line.
wantpass() {
  [ "$lib_st" -eq 0 ] || { detail="library exit $lib_st, want 0"; return 1; }
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
    version-skew)
      mkver "$c" 0.1.0 0.1.0 || { detail="cannot build the inputs"; return 1; }
      runver "$c" v0.1.1
      wantfail "$c" 'release-version-skew: tag 0.1.1 differs from header 0.1.0' || return 1
      mkver "$c" 0.1.0 0.1.1 || { detail="cannot build the inputs"; return 1; }
      runver "$c" v0.1.0
      wantfail "$c" 'release-version-skew: header 0.1.0 differs from package 0.1.1' || return 1
      ;;
    version-tag-form)
      mkver "$c" 0.1.0 0.1.0 || { detail="cannot build the inputs"; return 1; }
      runver "$c" v1.0
      wantfail "$c" 'release-version-tag-form: v1.0' || return 1
      ;;
    version-agree-pass)
      mkver "$c" 0.1.0 0.1.0 || { detail="cannot build the inputs"; return 1; }
      runver "$c" v0.1.0
      wantpass "$c" 'release-assets version ok 0.1.0' || return 1
      ;;
    changelog-section-missing)
      mkdir -p "$tmp/$c" || { detail="cannot build the inputs"; return 1; }
      printf '# Changelog\n\n## [Unreleased]\n\nnotes\n' > "$tmp/$c/CHANGELOG.md" || { detail="cannot build the inputs"; return 1; }
      runchg "$c" 0.1.0
      wantfail "$c" 'release-changelog-section-missing: 0.1.0' || return 1
      ;;
    changelog-section-pass)
      mkdir -p "$tmp/$c" || { detail="cannot build the inputs"; return 1; }
      printf '# Changelog\n\n## [Unreleased]\n\nnotes\n\n## [0.1.0] - draft\n\nnotes\n' > "$tmp/$c/CHANGELOG.md" \
        || { detail="cannot build the inputs"; return 1; }
      runchg "$c" 0.1.0
      wantpass "$c" 'release-assets changelog ok 0.1.0' || return 1
      ;;
    readiness-selftest)
      rd="$ROOT/tools/release-readiness.sh"
      [ -f "$rd" ] || return 2
      mkdir -p "$tmp/$c" || { detail="cannot build the inputs"; return 1; }
      rd_st=0
      sh "$rd" --selftest > "$tmp/$c/lib.out" 2>&1 < /dev/null || rd_st=$?
      [ "$rd_st" -eq 0 ] || { detail="readiness selftest exit $rd_st, want 0"; return 1; }
      [ "$(tail -n 1 "$tmp/$c/lib.out")" = 'selftest 4/4' ] || { detail="last line is not 'selftest 4/4'"; return 1; }
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

# (c) The claim table of CHANGELOG.md against the claim ledger, and the notes file of a release.
if grep -q -x -F 'CHANGELOG.md' "$FILELIST" && [ -f "$ROOT/CHANGELOG.md" ]; then
  rendered=1
  cmake -DROOT="$ROOT" -P "$CHECK_DIR/claims-table.cmake" > "$tmp/table.want" 2> "$tmp/table.err" < /dev/null || rendered=0
  if [ "$rendered" -eq 0 ]; then
    # The generator prints its findings on stdout: one line per finding, token first.
    if grep -q '^release-claims-' "$tmp/table.want"; then
      sed -n 's/^\(release-claims-[a-z-]*: .*\)$/-- FAIL \1/p' "$tmp/table.want" | sed -n '1,20p'
    else
      bad release-claims-unreadable "claims-table.cmake"
    fi
    rc=1
  elif awk -v b='<!-- claims-table-begin -->' -v e='<!-- claims-table-end -->' '
    $0 == b { if (st != 0) bad = 1; st = 1; next }
    $0 == e { if (st != 1) bad = 1; st = 2; next }
    st == 1 { print }
    END { if (st != 2 || bad) exit 3 }' "$ROOT/CHANGELOG.md" > "$tmp/table.have"; then
    cmp -s "$tmp/table.want" "$tmp/table.have" || bad release-changelog-claims-drift "CHANGELOG.md"
  else
    bad release-changelog-claims-drift "CHANGELOG.md markers"
  fi
elif grep -q -x -F 'tools/release.sh' "$FILELIST"; then
  bad release-changelog-missing "CHANGELOG.md"
fi

[ "$rc" -eq 0 ] || exit 1
printf -- '-- release-assets pass release-selftest=%s/%s\n' "$ok" "$total"
