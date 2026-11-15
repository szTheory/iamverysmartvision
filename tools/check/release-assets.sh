#!/bin/sh
# SPDX-License-Identifier: MIT
# Release asset member checker (plan 08-01, D-32, threat T-08-02). A library, not a gate step: the gate
# step release-assets (tools/check/steps/release-assets.sh) proves on synthetic archives that this
# checker can fail and can pass, and tools/release.sh runs it on the real release archives.
#
#   sh tools/check/release-assets.sh source  <archive> <prefix> <listfile>
#   sh tools/check/release-assets.sh install <archive> <prefix> <listfile>
#   sh tools/check/release-assets.sh version <tag> <ivs.h> <config-version-file>
#   sh tools/check/release-assets.sh changelog <version> <CHANGELOG.md>
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
#
# Mode version (plan 08-02, D-30, threat T-08-05): the three version numbers of a release agree.
# <ivs.h> holds the three IVS_API_VERSION_MAJOR, _MINOR and _PATCH defines (a trailing u is
# accepted); <config-version-file> is the installed ivsConfigVersion.cmake, whose PACKAGE_VERSION is
# the version a consumer sees as ivs_VERSION. The tag must be exactly v<digits>.<digits>.<digits>;
# its digits, the header macros and the package version must be equal. Findings, one per line:
#   release-version-tag-form: <tag>             the tag has another form (the tag is then not compared)
#   release-version-skew: <a> <x> differs from <b> <y>
#                                               two of the three numbers differ (the pairs are
#                                               tag and header, header and package)
#   release-version-unreadable: <file>          an input is missing or holds no single usable line
# Exit 0 prints "release-assets version ok <X.Y.Z>".
# Mode changelog: <CHANGELOG.md> holds a line that is exactly "## [<version>]" or starts with
# "## [<version>] " (version without the v, digits only). Findings:
#   release-changelog-section-missing: <version>
#   release-changelog-unreadable: <file>
# Exit 0 prints "release-assets changelog ok <version>". Both modes only read their inputs.
LC_ALL=C; export LC_ALL

usage() {
  {
    printf '%s\n' 'usage: sh tools/check/release-assets.sh source|install <archive> <prefix> <listfile>'
    printf '%s\n' '       sh tools/check/release-assets.sh version <tag> <ivs.h> <config-version-file>'
    printf '%s\n' '       sh tools/check/release-assets.sh changelog <version> <CHANGELOG.md>'
  } >&2
}

# hdr_part <MAJOR|MINOR|PATCH> <header>: prints the number of the one matching define, status 1 when
# the header has none or several.
hdr_part() {
  hp_v=$(sed -n "s/^#define IVS_API_VERSION_$1 \([0-9][0-9]*\)[uU]\{0,1\}\$/\1/p" "$2" 2> /dev/null)
  case $hp_v in '' | *'
'*) return 1 ;; esac
  printf '%s\n' "$hp_v"
}

# mode version <tag> <header> <config-version-file>
mode_version() {
  mv_tag=$1
  mv_hdr=$2
  mv_cfg=$3
  mv_rc=0
  for mv_f in "$mv_hdr" "$mv_cfg"; do
    if [ ! -f "$mv_f" ] || [ ! -r "$mv_f" ]; then
      printf 'release-version-unreadable: %s\n' "$mv_f"
      mv_rc=1
    fi
  done
  [ "$mv_rc" -eq 0 ] || return 1
  mv_maj=$(hdr_part MAJOR "$mv_hdr") && mv_min=$(hdr_part MINOR "$mv_hdr") && mv_pat=$(hdr_part PATCH "$mv_hdr") || {
    printf 'release-version-unreadable: %s\n' "$mv_hdr"
    return 1
  }
  mv_h=$mv_maj.$mv_min.$mv_pat
  mv_p=$(sed -n 's/^set(PACKAGE_VERSION "\([0-9][0-9]*[.][0-9][0-9]*[.][0-9][0-9]*\)")$/\1/p' "$mv_cfg" 2> /dev/null)
  case $mv_p in '' | *'
'*)
    printf 'release-version-unreadable: %s\n' "$mv_cfg"
    return 1
    ;;
  esac
  mv_form=1
  case $mv_tag in *'
'*) mv_form=0 ;; esac
  if [ "$mv_form" -eq 1 ] && ! printf '%s\n' "$mv_tag" | grep -E -q '^v[0-9]+[.][0-9]+[.][0-9]+$'; then mv_form=0; fi
  if [ "$mv_form" -eq 0 ]; then
    printf 'release-version-tag-form: %s\n' "$(printf '%s' "$mv_tag" | tr '\n' ' ')"
    mv_rc=1
  else
    mv_t=${mv_tag#v}
    if [ "$mv_t" != "$mv_h" ]; then
      printf 'release-version-skew: tag %s differs from header %s\n' "$mv_t" "$mv_h"
      mv_rc=1
    fi
  fi
  if [ "$mv_h" != "$mv_p" ]; then
    printf 'release-version-skew: header %s differs from package %s\n' "$mv_h" "$mv_p"
    mv_rc=1
  fi
  [ "$mv_rc" -eq 0 ] || return 1
  printf 'release-assets version ok %s\n' "$mv_h"
}

# mode changelog <version> <CHANGELOG.md>
mode_changelog() {
  mc_ver=$1
  mc_file=$2
  if [ ! -f "$mc_file" ] || [ ! -r "$mc_file" ]; then
    printf 'release-changelog-unreadable: %s\n' "$mc_file"
    return 1
  fi
  if awk -v h="## [$mc_ver]" '{ sub(/\r$/, "") } $0 == h || index($0, h " ") == 1 { f = 1 } END { exit !f }' "$mc_file"; then
    printf 'release-assets changelog ok %s\n' "$mc_ver"
    return 0
  fi
  printf 'release-changelog-section-missing: %s\n' "$mc_ver"
  return 1
}

case ${1:-} in
  version)
    [ "$#" -eq 4 ] || { usage; exit 2; }
    mode_version "$2" "$3" "$4"
    exit $?
    ;;
  changelog)
    [ "$#" -eq 3 ] || { usage; exit 2; }
    printf '%s\n' "$2" | grep -E -q '^[0-9]+[.][0-9]+[.][0-9]+$' || { usage; exit 2; }
    mode_changelog "$2" "$3"
    exit $?
    ;;
esac

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
