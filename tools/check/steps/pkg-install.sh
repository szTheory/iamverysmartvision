#!/bin/sh
# SPDX-License-Identifier: MIT
# Step pkg-install: the installed, relocated package (PKG-01, D-33, D-34, D-37, plan 04-22).
# Installs the release tree that c-build already built (no third build; c-build sorts before
# this step) into a temp prefix, moves the prefix into a path that contains a space, deletes
# the original prefix and asserts it is gone, then checks the moved prefix:
#   inventory  the installed files (relative paths, sorted) equal data/expected/pkg-install-files.ids
#              exactly; this is also the guard that no image or runner enters the package.
#   content    every installed header is `cmp`-equal to core/include/ivs/<name>, and the
#              installed archive's SHA-256 (cmake -E sha256sum) equals build/release's (D-37 b).
#              On Darwin only, where CMake re-runs ranlib at install and restamps the index
#              member header, a hash difference falls back to a member-wise proof (same `ar t`
#              list, every member byte-equal, the index included); the stat says which held.
#   paths      no installed file and not the archive contains `$ROOT/core` or `$ROOT/build`,
#              in the logical or the physical (`pwd -P`) form of ROOT (grep -a -F, D-37 c).
#   symbols    (D-38) the declared set (the name before the first `(` of every line of the
#              installed headers that starts, after blanks, with `IVS_API `) is nonempty, each
#              name is a defined external of the installed archive, and the set equals the call
#              names of data/api/api-effects.json (read with cmake -P, string(JSON)); every
#              defined external matches ^ivs_. Names are normalized as core-symbols.sh does (one
#              Darwin leading underscore and any @version dropped). core-symbols.sh then runs on
#              the installed archive with the unchanged allowlist.
#   alone      tests/headers-tap.sh with the prefix include dir: every C17 case ok and every
#              C++17 case ok or a visible `# SKIP` (D-38, Pitfall 14; no new test ids).
# Every temp file is under one mktemp dir outside the repo, removed on exit; the step writes
# nothing else. Logs are printed only on failure.
# Tokens: pkg-install-no-tree: <path>, pkg-install-failed: <what>, pkg-install-relocate: <what>,
#   pkg-inventory-missing: <path>, pkg-inventory-extra: <path>, pkg-inventory-list: <what>,
#   pkg-header-diff: <name>, pkg-archive-diff: <what>, pkg-path-leak: <path>,
#   pkg-symbol-missing: <name>, pkg-symbol-api-drift: <detail>, pkg-symbol-namespace: <name>,
#   pkg-symbol-tool: <what>, pkg-header-alone: <case or what>, plus core-symbols.sh's tokens.
# Stats: prefix-files=<n> archive=<sha256|members> public-symbols=<declared names> defined=<defined externals>
#   hdr-alone=<ok>/<cases> (skipped C++ cases are not ok; hdr-skip=<n> is appended when nonzero).
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
T=$(mktemp -d "${TMPDIR:-/tmp}/pkg-install.XXXXXX") || exit 1
trap 'rm -rf "$T"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }
cd "$ROOT" || exit 1
rel="$ROOT/build/release"
inv="$ROOT/data/expected/pkg-install-files.ids"

# check_inventory <prefix>: the files under <prefix> equal the inventory. Prints FAIL lines.
check_inventory() {
  ci_rc=0
  if [ ! -f "$inv" ]; then
    printf -- '-- FAIL pkg-inventory-list: %s\n' "data/expected/pkg-install-files.ids"
    return 1
  fi
  awk '/^#/ { next } NF { print $1 }' "$inv" | sort -u > "$T/inv.want"
  if [ ! -s "$T/inv.want" ]; then
    printf -- '-- FAIL pkg-inventory-list: %s\n' "no paths"
    return 1
  fi
  (cd "$1" && find . ! -type d) | sed 's|^\./||' | sort -u > "$T/inv.have"
  comm -23 "$T/inv.want" "$T/inv.have" > "$T/inv.missing"
  comm -13 "$T/inv.want" "$T/inv.have" > "$T/inv.extra"
  while IFS= read -r ci_p; do printf -- '-- FAIL pkg-inventory-missing: %s\n' "$ci_p"; ci_rc=1; done < "$T/inv.missing"
  while IFS= read -r ci_p; do printf -- '-- FAIL pkg-inventory-extra: %s\n' "$ci_p"; ci_rc=1; done < "$T/inv.extra"
  return "$ci_rc"
}

NM=${NM:-nm}
darwin=0
[ "$(uname -s 2> /dev/null)" = Darwin ] && darwin=1
# Symbol names of `nm -P` lines, normalized as core-symbols.sh's norm(): skip blank lines and
# archive member headers, keep the first field, drop `@ver`, strip one Darwin underscore.
norm() {
  awk -v darwin="$darwin" '
    NF < 2 { next }
    $1 ~ /:$/ { next }
    { n = $1; sub(/@.*/, "", n); if (darwin && n ~ /^_/) n = substr(n, 2); if (n != "") print n }'
}
# The leak strings: ROOT's core/ and build/ paths, logical and physical.
proot=$(cd "$ROOT" && pwd -P) || exit 1
set -- -e "$ROOT/core" -e "$ROOT/build"
[ "$proot" = "$ROOT" ] || set -- "$@" -e "$proot/core" -e "$proot/build"

# check_paths <prefix> <grep -e args...>: no file under <prefix> contains a leak string. Prints
# FAIL lines.
# A grep error (status above 1) is a failure too, never a clean file.
check_paths() {
  cp_rc=0
  cp_dir=$1
  shift
  (cd "$cp_dir" && find . ! -type d) | sed 's|^\./||' | sort > "$T/paths.list"
  [ -s "$T/paths.list" ] || { printf -- '-- FAIL pkg-path-leak: %s\n' "no files under the prefix"; return 1; }
  while IFS= read -r cp_f; do
    cp_st=0
    grep -q -a -F "$@" "$cp_dir/$cp_f" || cp_st=$?
    case $cp_st in
      0) printf -- '-- FAIL pkg-path-leak: %s\n' "$cp_f"; cp_rc=1 ;;
      1) ;;
      *) printf -- '-- FAIL pkg-path-leak: %s (grep status %s)\n' "$cp_f" "$cp_st"; cp_rc=1 ;;
    esac
  done < "$T/paths.list"
  return "$cp_rc"
}

# The JSON call names, once (cmake -P, string(JSON); a temp script, floor 3.25).
cat > "$T/calls.cmake" << 'CMK'
cmake_minimum_required(VERSION 3.25)
file(READ "${JSON}" doc)
string(JSON n ERROR_VARIABLE err LENGTH "${doc}" calls)
if(err OR NOT n GREATER 0)
  message(FATAL_ERROR "no calls array: ${err}")
endif()
set(out "")
math(EXPR last "${n} - 1")
foreach(i RANGE ${last})
  string(JSON name GET "${doc}" calls ${i} name)
  string(APPEND out "${name}\n")
endforeach()
file(WRITE "${OUT}" "${out}")
CMK
if cmake -DJSON="$ROOT/data/api/api-effects.json" -DOUT="$T/calls.raw" -P "$T/calls.cmake" > "$T/calls.log" 2>&1; then
  sort -u "$T/calls.raw" > "$T/calls"
else
  cat "$T/calls.log"
  : > "$T/calls"
fi

# check_symbols <include dir> <archive>: the D-38 symbol rules. Prints FAIL lines; leaves the
# declared and defined name lists in $T/sym.decl and $T/sym.def.
check_symbols() {
  cs_rc=0
  : > "$T/sym.decl"
  : > "$T/sym.def"
  for cs_h in "$1"/ivs/*.h; do
    [ -f "$cs_h" ] || continue
    awk '/^[ \t]*IVS_API[ \t]/ {
        s = $0; i = index(s, "(")
        if (i == 0) { print "?"; next }
        s = substr(s, 1, i - 1); sub(/[ \t]+$/, "", s)
        n = split(s, a, /[^A-Za-z0-9_]+/); print (n > 0 && a[n] != "" ? a[n] : "?")
      }' "$cs_h" >> "$T/sym.decl"
  done
  sort -u "$T/sym.decl" -o "$T/sym.decl"
  if ! "$NM" -P -g --defined-only "$2" > "$T/sym.nm" 2> "$T/sym.nm.err"; then
    cat "$T/sym.nm.err"
    printf -- '-- FAIL pkg-symbol-tool: %s\n' "nm failed on $2"
    return 1
  fi
  norm < "$T/sym.nm" | sort -u > "$T/sym.def"
  if [ ! -s "$T/sym.decl" ]; then
    printf -- '-- FAIL pkg-symbol-api-drift: %s\n' "no IVS_API declaration in the installed headers"
    cs_rc=1
  fi
  if [ ! -s "$T/calls" ]; then
    printf -- '-- FAIL pkg-symbol-api-drift: %s\n' "no call names read from data/api/api-effects.json"
    cs_rc=1
  fi
  comm -23 "$T/sym.decl" "$T/sym.def" > "$T/sym.missing"
  while IFS= read -r cs_n; do printf -- '-- FAIL pkg-symbol-missing: %s\n' "$cs_n"; cs_rc=1; done < "$T/sym.missing"
  comm -3 "$T/sym.decl" "$T/calls" > "$T/sym.drift"
  if [ -s "$T/sym.drift" ]; then
    printf -- '-- FAIL pkg-symbol-api-drift: %s\n' \
      "headers only: $(comm -23 "$T/sym.decl" "$T/calls" | tr '\n' ' ')json only: $(comm -13 "$T/sym.decl" "$T/calls" | tr '\n' ' ')"
    cs_rc=1
  fi
  grep -v '^ivs_' "$T/sym.def" > "$T/sym.ns"
  while IFS= read -r cs_n; do printf -- '-- FAIL pkg-symbol-namespace: %s\n' "$cs_n"; cs_rc=1; done < "$T/sym.ns"
  return "$cs_rc"
}

# 1. Install, relocate, delete the original.
[ -f "$rel/CMakeCache.txt" ] || { bad pkg-install-no-tree build/release; exit 1; }
pa="$T/stage/prefix-a"
pb="$T/reloc pfx/prefix-b"
mkdir -p "$T/stage" "$T/reloc pfx" || exit 1
if ! cmake --install "$rel" --prefix "$pa" > "$T/install.log" 2>&1; then
  cat "$T/install.log"
  bad pkg-install-failed "cmake --install build/release"
  exit 1
fi
mv "$pa" "$pb" || { bad pkg-install-relocate "mv to a path with a space"; exit 1; }
rm -rf "$pa"
[ ! -e "$pa" ] || { bad pkg-install-relocate "original prefix still exists"; exit 1; }
[ -d "$pb" ] || { bad pkg-install-relocate "moved prefix missing"; exit 1; }

# 2. Exact inventory.
check_inventory "$pb" || rc=1
files=$(grep -c . "$T/inv.have")

# 3. Content: headers byte-equal to the source, archive byte-equal to the build tree.
for h in "$pb"/include/ivs/*.h; do
  [ -f "$h" ] || continue
  name=$(basename "$h")
  cmp -s "$h" "$ROOT/core/include/ivs/$name" || bad pkg-header-diff "$name"
done
arch="$pb/lib/libivs_core.a"
barch="$rel/core/libivs_core.a"
archive_mode=sha256
if [ -f "$arch" ] && [ -f "$barch" ]; then
  h1=$(cmake -E sha256sum "$arch" | awk '{ print $1 }')
  h2=$(cmake -E sha256sum "$barch" | awk '{ print $1 }')
  if [ -z "$h1" ]; then
    bad pkg-archive-diff "cannot hash lib/libivs_core.a"
  elif [ "$h1" != "$h2" ]; then
    if [ "$darwin" -eq 1 ]; then
      # CMake re-runs ranlib on an installed static library on Apple, which restamps the
      # symbol-index member header. There the proof is member-wise: the same member list in
      # the same order and every member (the index included) byte-equal.
      archive_mode=members
      mkdir -p "$T/ar.inst" "$T/ar.build"
      if (cd "$T/ar.inst" && ar x "$arch") > "$T/ar.log" 2>&1 && (cd "$T/ar.build" && ar x "$barch") >> "$T/ar.log" 2>&1 \
        && ar t "$arch" > "$T/ar.inst.list" 2>> "$T/ar.log" && ar t "$barch" > "$T/ar.build.list" 2>> "$T/ar.log"; then
        cmp -s "$T/ar.inst.list" "$T/ar.build.list" || bad pkg-archive-diff "member lists differ"
        while IFS= read -r m; do
          cmp -s "$T/ar.inst/$m" "$T/ar.build/$m" || bad pkg-archive-diff "member $m"
        done < "$T/ar.build.list"
      else
        cat "$T/ar.log"
        bad pkg-archive-diff "ar failed"
      fi
    else
      bad pkg-archive-diff "lib/libivs_core.a vs build/release/core/libivs_core.a (sha256)"
    fi
  fi
else
  bad pkg-archive-diff "archive missing (installed or build tree)"
fi

# 4. No repo path in any installed file or the archive.
check_paths "$pb" "$@" || rc=1

# 5. Symbol rules, then the import allowlist on the installed archive.
pubsyms=0
defined=0
if [ -f "$arch" ]; then
  check_symbols "$pb/include" "$arch" || rc=1
  pubsyms=$(grep -c . "$T/sym.decl")
  defined=$(grep -c . "$T/sym.def")
  if ! CHECK_DIR="$CHECK_DIR" sh "$CHECK_DIR/core-symbols.sh" "$arch" > "$T/core-symbols.out" 2>&1; then
    cat "$T/core-symbols.out"
    rc=1
  fi
fi

# 6. Each installed header alone, C17 and C++17 (headers-tap.sh with the prefix include dir).
hdr_ok=0
hdr_n=0
hdr_skip=0
st=0
IVS_TEST_OUT_DIR= sh "$ROOT/tests/headers-tap.sh" "$ROOT" "$pb/include" > "$T/alone.tap" 2>&1 || st=$?
if [ "$st" -ne 0 ]; then
  sed 's/^/# /' "$T/alone.tap"
  bad pkg-header-alone "headers-tap.sh exit $st"
fi
while IFS= read -r l; do
  case $l in
    'ok '*' # SKIP '*)
      hdr_n=$((hdr_n + 1)); hdr_skip=$((hdr_skip + 1))
      case $l in *' headers.cxx-'*) ;; *) bad pkg-header-alone "C17 case skipped: $l" ;; esac ;;
    'ok '*) hdr_n=$((hdr_n + 1)); hdr_ok=$((hdr_ok + 1)) ;;
    'not ok '*) hdr_n=$((hdr_n + 1)); bad pkg-header-alone "${l#not ok }" ;;
  esac
done < "$T/alone.tap"
[ "$hdr_n" -gt 0 ] || bad pkg-header-alone "no cases"
hdr_stat="hdr-alone=$hdr_ok/$hdr_n"
[ "$hdr_skip" -eq 0 ] || hdr_stat="$hdr_stat hdr-skip=$hdr_skip"

[ "$rc" -eq 0 ] || exit 1
printf -- '-- pkg-install pass prefix-files=%s archive=%s public-symbols=%s defined=%s %s\n' \
  "$files" "$archive_mode" "$pubsyms" "$defined" "$hdr_stat"
