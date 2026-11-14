#!/bin/sh
# SPDX-License-Identifier: MIT
# Release asset member checker (plan 08-01, D-32, threat T-08-02). A library, not a gate step: the gate
# step release-assets (tools/check/steps/release-assets.sh) proves on synthetic archives that this
# checker can fail and can pass, and tools/release.sh runs it on the real release archives.
#
#   sh tools/check/release-assets.sh source  <archive> <prefix> <listfile>
#   sh tools/check/release-assets.sh install <archive> <prefix> <listfile>
#
# The check is on the MEMBERS of the archive, never on its outer file name: binary-ext.txt lists gz
# and tar, so a rule on the outer name would refuse the very .tar.gz assets of a release.
# <archive> is a gzip-compressed tar (read with tar -tzf, which GNU tar and bsdtar both support).
# <prefix> is the one top-level directory every member lies under (no slash). <listfile> holds one
# path per line relative to the prefix (repo-relative for source, install-relative for install);
# comment lines and blank lines are ignored. The mode only names the report: both modes apply the
# same rules.
# Rules, each finding printed as one line "<token>: <detail>" (at most 20 lines per token, then
# "<token>: (+<n> more)"):
#   release-assets-unreadable      the archive cannot be listed, or holds no member
#   release-assets-path            a member name is absolute, has a .. segment, or holds a byte
#                                  outside printable ASCII
#   release-assets-prefix          a member does not lie under <prefix>/
#   release-assets-extra-member    a member (directory entries are dropped) is not in the list, or
#                                  occurs twice
#   release-assets-missing-member  a listed path has no member
#   release-assets-binary-member   the last extension of a member file name (lower case, after the
#                                  last dot) is a line of binary-ext.txt, whatever the list says
# The extension list is read from ${CHECK_DIR:-<this directory>}/binary-ext.txt, never copied here
# and never read from the archive. Nothing is written outside a mktemp directory removed on exit.
# Exit status: 0 no finding (prints "release-assets <mode> ok members=<n>"), 1 findings, 2 usage
# error or a missing input file.
LC_ALL=C; export LC_ALL

usage() {
  {
    printf '%s\n' 'usage: sh tools/check/release-assets.sh source|install <archive> <prefix> <listfile>'
  } >&2
}

[ "$#" -eq 4 ] || { usage; exit 2; }
mode=$1
archive=$2
prefix=$3
list=$4
case $mode in source | install) ;; *) usage; exit 2 ;; esac
case $prefix in '' | . | .. | */*) printf '%s\n' 'release-assets: the prefix must be one directory name' >&2; exit 2 ;; esac
here=$(cd "$(dirname "$0")" && pwd) || exit 2
extfile=${CHECK_DIR:-$here}/binary-ext.txt
[ -f "$list" ] || { printf '%s\n' 'release-assets: the list file does not exist' >&2; exit 2; }
if [ ! -f "$extfile" ] || ! grep -v -e '^[[:space:]]*#' -e '^[[:space:]]*$' "$extfile" | grep -q .; then
  printf '%s\n' 'release-assets: binary-ext.txt is missing or has no extension' >&2
  exit 2
fi

tmp=$(mktemp -d "${TMPDIR:-/tmp}/release-assets.XXXXXX") || exit 2
trap 'rm -rf "$tmp"' EXIT
trap 'exit 2' HUP INT TERM

if ! tar -tzf "$archive" > "$tmp/raw" 2> "$tmp/err"; then
  printf 'release-assets-unreadable: %s (%s)\n' "$archive" "$(sed -n '1p' "$tmp/err")"
  exit 1
fi

# One awk pass over the listing: path and prefix rules on every entry, then the member names
# (prefix removed, directory entries dropped) to $tmp/members, duplicates and binary members found
# on the way. Findings go to stdout, capped per token.
: > "$tmp/members"
awk -v prefix="$prefix" -v extfile="$extfile" -v mfile="$tmp/members" '
  function emit(tok, detail) {
    cnt[tok]++
    if (cnt[tok] <= 20) printf "%s: %s\n", tok, detail
  }
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
    name = $0
    if (name ~ /[^ -~]/) { emit("release-assets-path", name); next }
    isdir = 0
    if (substr(name, length(name), 1) == "/") { isdir = 1; name = substr(name, 1, length(name) - 1) }
    if (substr(name, 1, 1) == "/" || name == ".." || name ~ /(^|\/)\.\.(\/|$)/) { emit("release-assets-path", $0); next }
    if (isdir && name == prefix) next
    if (substr(name, 1, length(prefix) + 1) != prefix "/") { emit("release-assets-prefix", $0); next }
    if (isdir) next
    m = substr(name, length(prefix) + 2)
    if (m in seen) { emit("release-assets-extra-member", m " (duplicate)"); next }
    seen[m] = 1
    print m > mfile
    base = m
    sub(/^.*\//, "", base)
    k = split(base, part, ".")
    if (k >= 2 && (tolower(part[k]) in bin)) emit("release-assets-binary-member", m)
  }
  END {
    split("release-assets-path release-assets-prefix release-assets-extra-member release-assets-binary-member", toks, " ")
    for (i = 1; i <= 4; i++) if (cnt[toks[i]] > 20) printf "%s: (+%d more)\n", toks[i], cnt[toks[i]] - 20
  }' "$tmp/raw" > "$tmp/findings"

sort -u "$tmp/members" > "$tmp/m.sorted"
awk '
  { sub(/^[ \t]+/, ""); sub(/[ \t\r]+$/, "") }
  $0 == "" || substr($0, 1, 1) == "#" { next }
  { print }' "$list" | sort -u > "$tmp/l.sorted"
comm -13 "$tmp/l.sorted" "$tmp/m.sorted" > "$tmp/extra"
comm -23 "$tmp/l.sorted" "$tmp/m.sorted" > "$tmp/missing"

# emit_list <token> <file>: one finding per line of the file, at most 20, then a count line.
emit_list() {
  sed -n '1,20p' "$2" | while IFS= read -r el_m; do printf '%s: %s\n' "$1" "$el_m"; done
  el_n=$(grep -c . "$2")
  [ "$el_n" -le 20 ] || printf '%s: (+%s more)\n' "$1" "$((el_n - 20))"
}

n=$(grep -c . "$tmp/members")
rc=0
{
  cat "$tmp/findings"
  if [ -s "$tmp/extra" ]; then emit_list release-assets-extra-member "$tmp/extra"; fi
  if [ -s "$tmp/missing" ]; then emit_list release-assets-missing-member "$tmp/missing"; fi
  if [ "$n" -eq 0 ]; then printf 'release-assets-unreadable: %s (no member)\n' "$archive"; fi
} > "$tmp/all"
if [ -s "$tmp/all" ]; then
  cat "$tmp/all"
  rc=1
fi
[ "$rc" -eq 0 ] || exit 1
printf 'release-assets %s ok members=%s\n' "$mode" "$n"
