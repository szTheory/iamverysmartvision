#!/bin/sh
# SPDX-License-Identifier: MIT
# Host shim export and import check (plan 15-02, TOOL-08, D-48, D-35). Not a gate step: called by the
# runner-process case runner-process.host.exports, by tools/check/run-controls.sh when a control names
# it, and by a plan's own verify.
# Usage: sh host-symbols.sh [--allow-prefix P]... <object> [<ids>] [<imports>]
#   (no object: usage, exit 2; the two lists are positional AFTER the object, the same argument order
#   core-symbols.sh uses so the two scripts are read the same way)
# Env: NM (default nm), CHECK_DIR (directory of the reviewed allowlist; default: this script's
#   directory), ROOT (repository root; default: two levels above this script).
# The two contracts resolve by default from ${ROOT}/data/expected/ (host-symbols.ids, host-imports.ids):
#   neither file lives in CHECK_DIR, so defaulting them there would be a check that cannot run at all,
#   and both callers - the case and a plan's verify - pass no ids path.
# EXPORTED set (`$NM -P -g --defined-only`), compared in BOTH directions against host-symbols.ids:
#   comm -23 -> host-symbols-missing: <name>   a listed name the object does not export
#   comm -13 -> host-symbols-extra: <name>     an exported name the list does not carry
# The two directions are both live and they are not symmetric: Apple's ld ERRORS on an export-list name
# the link does not define, while GNU's ld accepts it silently, so on Linux only the missing direction
# catches a list/artifact disagreement.
# The `ivs_` limb of criterion 1 is the exact-set comparison above, and this is why: the one `ivs_` name
# the object may export is ivs_retro_digest, which the ids file pins, so any other ivs_ name is
# host-symbols-extra by construction. The palette limb is one explicit grep over the exported set
# (host-symbols-palette) - the exact-set comparison already implies it, but the criterion states it, so
# it is one line and not an inference. core-symbols.sh cannot be run on this object (the shim MUST
# define the palette entry through runner/frame_export.c), which is why this check restates the four
# words over the exported names only.
# IMPORTED set (`$NM -P -u`): undefined minus core-symbols-allow.txt, minus host-imports.ids and minus
# every --allow-prefix. Any remainder is host-symbols-import: <name>. That is
# the rule that makes "the shim opens no file" a static fact: fopen, open, read and stat are matched by
# no prefix, so they cannot pass. --allow-prefix is core-symbols.sh's mechanism reused verbatim,
# validation included (an empty prefix, or one outside [A-Za-z0-9_.$], is a usage error and exits 2) and
# semantics included (a name is allowed when the prefix is its HEAD; never a substring). It is required,
# not polish: host/CMakeLists.txt links ivs_san on every preset, so the dev object necessarily imports
# the ASan and UBSan runtime entry points, and without prefixes imports=0 would be unreachable on the
# only preset the case and a plan's verify run on - a check that fails on a correct object.
# Names are normalized exactly as core-symbols.sh's norm() does: skip lines with fewer than two fields,
# skip archive-member headers ending in ':', take field 1, drop an `@version` suffix, strip one Darwin
# leading underscore.
# Tokens: host-symbols-missing, host-symbols-extra, host-symbols-palette, host-symbols-import,
#   host-symbols-missing-file, host-symbols-list-missing, host-symbols-list-empty, host-symbols-nm-failed,
#   host-symbols-no-exports.
# Pass: -- host-symbols pass exported=<n> imports=<n>, where imports= is the count of DISALLOWED imports
#   (what remains after both allowlists and every prefix), so it is 0 on a correct object and nonzero
#   exactly when a forbidden name appeared. The count of allowed imports is not reported: it varies with
#   the toolchain's fortify and stack-protector forms and would make the assertion fragile for no
#   evidence. No `nm` dump is printed: the evidence is a parsed, sorted name set.
# Exit: 0 pass, 1 a finding, 2 a usage error or a missing input.
LC_ALL=C; export LC_ALL
NM=${NM:-nm}
usage() {
  printf 'usage: sh host-symbols.sh [--allow-prefix P]... <object> [<ids>] [<imports>]\n' >&2
  exit 2
}
prefixes=
while [ "$#" -gt 0 ]; do
  case $1 in
    --allow-prefix)
      # An empty prefix would allow every name, so it is a usage error.
      [ "$#" -ge 2 ] || usage
      case $2 in '' | *[!A-Za-z0-9_.\$]*) usage ;; esac
      prefixes="$prefixes $2"
      shift 2 ;;
    --) shift; break ;;
    -*) usage ;;
    *) break ;;
  esac
done
[ "$#" -ge 1 ] || usage
obj=$1
shift
[ -f "$obj" ] || { printf -- '-- FAIL host-symbols-missing-file: %s\n' "$obj"; exit 2; }
here=$(cd "$(dirname "$0")" && pwd) || exit 2
listdir=${CHECK_DIR:-$here}
root=${ROOT:-$(cd "$here/../.." && pwd)}
ids=${1:-$root/data/expected/host-symbols.ids}
imports_list=${2:-$root/data/expected/host-imports.ids}
if [ ! -f "$ids" ]; then
  printf -- '-- FAIL host-symbols-list-missing: %s\n' "$ids"
  exit 2
fi
if [ ! -f "$imports_list" ]; then
  printf -- '-- FAIL host-symbols-list-missing: %s\n' "$imports_list"
  exit 2
fi
if [ ! -f "$listdir/core-symbols-allow.txt" ]; then
  printf -- '-- FAIL host-symbols-list-missing: %s\n' "$listdir/core-symbols-allow.txt"
  exit 2
fi
tmp=$(mktemp -d "${TMPDIR:-/tmp}/host-symbols.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }

darwin=0
[ "$(uname -s 2> /dev/null)" = Darwin ] && darwin=1
# Symbol lines of `nm -P` output, normalized exactly as core-symbols.sh does.
norm() {
  awk -v darwin="$darwin" '
    NF < 2 { next }
    $1 ~ /:$/ { next }
    { n = $1; sub(/@.*/, "", n); if (darwin && n ~ /^_/) n = substr(n, 2); if (n != "") print n }'
}
# run_nm <out> <nm args...>: a nonzero nm exit is a failure, except when its only complaint is
# "no symbols" (an object with nothing external), which reads as empty output.
run_nm() {
  _out=$1; shift
  if "$NM" "$@" > "$_out" 2> "$tmp/nm.err"; then return 0; fi
  if [ -s "$tmp/nm.err" ] && ! grep -v -q -e 'no symbols' "$tmp/nm.err"; then return 0; fi
  cat "$tmp/nm.err"
  return 1
}

# The pin and the two allowlists.
awk '/^#/ { next } NF { print $1 }' "$ids" | sort -u > "$tmp/want"
[ -s "$tmp/want" ] || bad host-symbols-list-empty host-symbols.ids
awk '/^#/ { next } NF { print $1 }' "$imports_list" | sort -u > "$tmp/host.allow"
[ -s "$tmp/host.allow" ] || bad host-symbols-list-empty host-imports.ids
awk '/^#/ { next } NF { print $1 }' "$listdir/core-symbols-allow.txt" | sort -u > "$tmp/core.allow"
[ -s "$tmp/core.allow" ] || bad host-symbols-list-empty core-symbols-allow.txt
sort -u "$tmp/host.allow" "$tmp/core.allow" > "$tmp/allowed"

# The exported set.
if ! run_nm "$tmp/d.raw" -P -g --defined-only "$obj"; then
  bad host-symbols-nm-failed "$obj"
else
  norm < "$tmp/d.raw" | sort -u > "$tmp/have"
  if [ ! -s "$tmp/have" ]; then
    bad host-symbols-no-exports "$obj"
  else
    comm -23 "$tmp/want" "$tmp/have" | while IFS= read -r n; do
      printf -- '-- FAIL host-symbols-missing: %s\n' "$n"
    done
    comm -13 "$tmp/want" "$tmp/have" | while IFS= read -r n; do
      printf -- '-- FAIL host-symbols-extra: %s\n' "$n"
    done
    [ -z "$(comm -23 "$tmp/want" "$tmp/have")" ] || rc=1
    [ -z "$(comm -13 "$tmp/want" "$tmp/have")" ] || rc=1
    grep -i -E 'palette|rgb|ppm|framepreview' "$tmp/have" | while IFS= read -r n; do
      printf -- '-- FAIL host-symbols-palette: %s\n' "$n"
    done
    [ -z "$(grep -i -E 'palette|rgb|ppm|framepreview' "$tmp/have")" ] || rc=1
  fi
fi

# The imported set: undefined minus both allowlists minus every prefix.
: > "$tmp/forbidden"
if ! run_nm "$tmp/u.raw" -P -u "$obj"; then
  bad host-symbols-nm-failed "$obj"
else
  norm < "$tmp/u.raw" | sort -u | comm -23 - "$tmp/allowed" \
    | awk -v p="$prefixes" 'BEGIN { n = split(p, a, " ") }
        { for (k = 1; k <= n; k++) if (index($0, a[k]) == 1) next; print }' > "$tmp/forbidden"
  while IFS= read -r n; do
    printf -- '-- FAIL host-symbols-import: %s\n' "$n"
  done < "$tmp/forbidden"
  [ ! -s "$tmp/forbidden" ] || rc=1
fi

[ "$rc" -eq 0 ] || exit 1
n_imports=0
[ ! -s "$tmp/forbidden" ] || n_imports=$(grep -c . "$tmp/forbidden")
printf -- '-- host-symbols pass exported=%s imports=%s\n' "$(grep -c . "$tmp/want")" "$n_imports"
