#!/bin/sh
# SPDX-License-Identifier: MIT
# Step pkg-install: the installed, relocated package and its independent consumer (PKG-01,
# D-33..D-40, plans 04-22 and 04-23).
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
#   consumer   (D-36, D-39, plan 04-23) tests/installed-consumer copied to the temp dir, configured
#              with -DCMAKE_PREFIX_PATH=<moved prefix>, registries off and the path variables unset
#              in a subshell, built with strict C17 flags, run on build/release/images/
#              {boot-stub.exec,diag-cpu-1.ivsc,diag-ctrl-psg-1.ivsc}; exit 0 required; stdout kept as
#              build/gate/pkg-consumer.txt and equal to data/expected/pkg-consumer.golden without
#              its # lines; its digest.<domain> lines equal data/golden/diag-input-pass.digests, and
#              (plan 05-11, D-19) its ctrl.digest.<domain> lines and observed domains equal
#              data/golden/diag-ctrl-psg-pass.digests (audio and psglog observed, the composite aside).
#   independence (D-37 d, e) `ninja -t deps`: no path under ROOT, no ivs*.h outside the moved
#              prefix, nothing else under the temp dir but the source copy, and ivs.h,
#              ivs_machine.h, ivs_run.h, ivs_status.h present under the prefix; `ninja -t
#              commands`: one link command, naming <prefix>/lib/libivs_core.a and no other
#              ivs_core archive; the compile command has the strict flags and no -isystem;
#              ivs_DIR of the consumer cache lies under the prefix (pwd -P); `nm -P -u` of
#              main.c.o, ivs_* names normalized, equals data/expected/pkg-consumer-calls.ids.
#   c++ link   (D-38) tests/installed-consumer/main.cpp compiled and linked by ${CXX:-c++}
#              -std=c++17 directly against the installed headers and archive, then run; a
#              visible `-- pkg-cxx-link SKIP` line without a C++ compiler.
#   selftest   thirteen in-step cases (D-40, exact inventory data/expected/pkg-selftests.ids), each on a
#              copy or the deleted prefix and each required to fail for its intended reason:
#              core-src-include, missing-symbol, ghost-decl, unprefixed, extra-file, baked-path,
#              version-refused (the next minor, the next major and an older minor are refused;
#              the built major.minor is found under the moved prefix), old-prefix (the deleted
#              original is not found), leak-detector (a consumer copy built with -I core/src and an
#              internal header: it builds, and the deps checker reports pkg-leak), golden-drift
#              (the consumer output with one digest nibble changed: pkg-golden-diff), and, from plan
#              05-11, ctrl-digest-drift (one ctrl.digest nibble changed: pkg-runner-golden-diff),
#              ctrl-domain-missing (the ctrl.digest.psglog line dropped: pkg-runner-golden-diff) and
#              ctrl-peek-wrong (the peeked $01FE changed: pkg-golden-diff).
# Every temp file is under one mktemp dir outside the repo, removed on exit; the step writes
# nothing else but build/gate/pkg-consumer.txt. Logs are printed only on failure.
# Tokens: pkg-install-no-tree: <path>, pkg-install-failed: <what>, pkg-install-relocate: <what>,
#   pkg-inventory-missing: <path>, pkg-inventory-extra: <path>, pkg-inventory-list: <what>,
#   pkg-header-diff: <name>, pkg-archive-diff: <what>, pkg-path-leak: <path>,
#   pkg-symbol-missing: <name>, pkg-symbol-api-drift: <detail>, pkg-symbol-namespace: <name>,
#   pkg-symbol-tool: <what>, pkg-header-alone: <case or what>, pkg-selftest-inventory: <what>,
#   pkg-selftest-missing: <case>, pkg-selftest-<case>: <detail>, pkg-consumer-tmp: <what>,
#   pkg-consumer-images: <paths>, pkg-consumer-copy: <what>, pkg-consumer-configure: <what>,
#   pkg-consumer-build: <what>, pkg-consumer-run: <exit>, pkg-consumer-output: <path>,
#   pkg-golden-diff: <what>, pkg-leak: <path>, pkg-deps-missing: <what>, pkg-link-path: <what>,
#   pkg-consumer-flags: <what>, pkg-ivs-dir: <what>, pkg-runner-golden-diff: <what>,
#   pkg-consumer-calls: <what>, pkg-cxx-link: <what>, plus core-symbols.sh's tokens.
# Stats: prefix-files=<n> archive=<sha256|members> public-symbols=<declared names> defined=<defined externals>
#   hdr-alone=<ok>/<cases> (skipped C++ cases are not ok; hdr-skip=<n> is appended when nonzero)
#   consumer-lines=<stdout lines of the consumer> cxx-link=<ok|skip> pkg-selftest=<ok>/<total>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
T=$(mktemp -d "${TMPDIR:-/tmp}/pkg-install.XXXXXX") || exit 1
trap 'rm -rf "$T"' EXIT
trap 'exit 1' HUP INT TERM
# Canonical form (a TMPDIR ending in / gives a double slash that CMake would normalize away).
T=$(cd "$T" && pwd) || exit 1
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
# archive member headers (GNU nm prints `<archive>[<member>]:`, which splits into several fields
# when the path holds a space, so the whole line is tested; nm also runs on a relative name),
# keep the first field, drop `@ver`, strip one Darwin underscore.
norm() {
  awk -v darwin="$darwin" '
    NF < 2 { next }
    $0 ~ /:$/ { next }
    { n = $1; sub(/@.*/, "", n); if (darwin && n ~ /^_/) n = substr(n, 2); if (n != "") print n }'
}
# The leak strings, one per line in $T/leak.pats: ROOT's core/ and build/ paths, logical and
# physical.
proot=$(cd "$ROOT" && pwd -P) || exit 1
printf '%s\n' "$ROOT/core" "$ROOT/build" "$proot/core" "$proot/build" | sort -u > "$T/leak.pats"

# check_paths <prefix>: no file under <prefix> contains a leak string. Prints FAIL lines.
# A grep error (status above 1) is a failure too, never a clean file.
check_paths() {
  cp_rc=0
  cp_dir=$1
  (cd "$cp_dir" && find . ! -type d) | sed 's|^\./||' | sort > "$T/paths.list"
  [ -s "$T/paths.list" ] || { printf -- '-- FAIL pkg-path-leak: %s\n' "no files under the prefix"; return 1; }
  while IFS= read -r cp_f; do
    cp_st=0
    grep -q -a -F -f "$T/leak.pats" "$cp_dir/$cp_f" || cp_st=$?
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
  cs_dir=$(dirname "$2")
  cs_base=$(basename "$2")
  if ! (cd "$cs_dir" && "$NM" -P -g --defined-only "./$cs_base") > "$T/sym.nm" 2> "$T/sym.nm.err"; then
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
check_paths "$pb" || rc=1

# 5. Symbol rules, then the import allowlist on the installed archive.
pubsyms=0
defined=0
if [ -f "$arch" ]; then
  check_symbols "$pb/include" "$arch" || rc=1
  pubsyms=$(grep -c . "$T/sym.decl")
  defined=$(grep -c . "$T/sym.def")
  # Run on a relative name: GNU nm's member headers would carry the prefix path and its space.
  if ! (cd "$pb/lib" && CHECK_DIR="$CHECK_DIR" sh "$CHECK_DIR/core-symbols.sh" ./libivs_core.a) > "$T/core-symbols.out" 2>&1; then
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

# 6b. The independent consumer (D-36, D-39, plan 04-23): tests/installed-consumer copied outside
# the repository, configured against the moved prefix only, built, run on the release tree's
# images, and its stdout compared with the pin.
gate_dir=${IVS_GATE_DIR:-$ROOT/build/gate}
cgold="$ROOT/data/expected/pkg-consumer.golden"
case $T in
  "$ROOT"/*|"$proot"/*) bad pkg-consumer-tmp "the temp dir is inside the repository"; exit 1 ;;
esac
# consumer_configure <src> <build> [-D...]: configure with the moved prefix as the only package
# source; the package and path variables of the environment are unset in a subshell.
consumer_configure() {
  cc_src=$1
  cc_bld=$2
  shift 2
  (
    unset CMAKE_PREFIX_PATH CMAKE_MODULE_PATH CMAKE_INSTALL_PREFIX CMAKE_TOOLCHAIN_FILE ivs_DIR ivs_ROOT IVS_ROOT \
      CPATH C_INCLUDE_PATH CPLUS_INCLUDE_PATH OBJC_INCLUDE_PATH LIBRARY_PATH CFLAGS CPPFLAGS LDFLAGS \
      LD_LIBRARY_PATH DYLD_LIBRARY_PATH DYLD_FALLBACK_LIBRARY_PATH DYLD_FRAMEWORK_PATH DYLD_INSERT_LIBRARIES
    cd "$T" || exit 1
    cmake -G Ninja -S "$cc_src" -B "$cc_bld" -DCMAKE_PREFIX_PATH="$pb" \
      -DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF -DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=OFF "$@"
  )
}
# The pin without its comment header.
golden_body() { grep -v '^#' "$cgold"; }
# check_golden <output>: the output equals the pin body. Prints FAIL lines.
check_golden() {
  if [ ! -f "$cgold" ]; then
    printf -- '-- FAIL pkg-golden-diff: %s\n' "data/expected/pkg-consumer.golden missing"
    return 1
  fi
  golden_body > "$T/golden.body"
  if ! cmp -s "$T/golden.body" "$1"; then
    printf -- '-- FAIL pkg-golden-diff: %s\n' "consumer output differs from data/expected/pkg-consumer.golden"
    return 1
  fi
  return 0
}
pT=$(cd "$T" && pwd -P) || exit 1
# check_deps <build dir> <consumer source copy>: the ninja dependency records of every consumer
# object (D-37 d, Pitfall 8). Lines carry a fixed 4-space indent and the paths may hold spaces, so
# the indent is stripped and nothing is split into fields. No path may lie under the repository
# (logical or physical form); an ivs*.h header outside the moved prefix, or any path under the temp
# dir other than the prefix and the source copy, is a leak too; and ivs.h, ivs_machine.h,
# ivs_run.h and ivs_status.h must be present under the moved prefix (positive presence, so an
# empty record cannot pass). Prints FAIL lines; leaves the prefix-relative names in $T/deps.prefix.
check_deps() {
  cd_rc=0
  cd_src=$2
  cd_psrc=$(cd "$cd_src" && pwd -P) || cd_psrc=$cd_src
  if ! (cd "$1" && ninja -t deps) > "$T/deps.raw" 2> "$T/deps.err"; then
    cat "$T/deps.err"
    printf -- '-- FAIL pkg-deps-missing: %s\n' "ninja -t deps failed"
    return 1
  fi
  sed -n 's/^    //p' "$T/deps.raw" > "$T/deps.list"
  : > "$T/deps.prefix"
  while IFS= read -r cd_p; do
    case $cd_p in
      "$ROOT"/*|"$proot"/*) printf -- '-- FAIL pkg-leak: %s\n' "$cd_p"; cd_rc=1; continue ;;
      "$pb"/*) printf '%s\n' "${cd_p#"$pb"/}" >> "$T/deps.prefix"; continue ;;
      "$ppb"/*) printf '%s\n' "${cd_p#"$ppb"/}" >> "$T/deps.prefix"; continue ;;
      "$cd_src"/*|"$cd_psrc"/*) ;;
      "$T"/*|"$pT"/*) printf -- '-- FAIL pkg-leak: %s\n' "$cd_p"; cd_rc=1; continue ;;
    esac
    case ${cd_p##*/} in
      ivs*.h) printf -- '-- FAIL pkg-leak: %s\n' "$cd_p"; cd_rc=1 ;;
    esac
  done < "$T/deps.list"
  for cd_h in ivs.h ivs_machine.h ivs_run.h ivs_status.h; do
    if ! grep -q -x -F -e "include/ivs/$cd_h" "$T/deps.prefix"; then
      printf -- '-- FAIL pkg-deps-missing: %s\n' "include/ivs/$cd_h under the moved prefix"
      cd_rc=1
    fi
  done
  return "$cd_rc"
}
# check_link <build dir>: one link command names libivs_core.a, it is <prefix>/lib/libivs_core.a
# and no other ivs_core archive (D-37 e); the compile command of main.c.o has the strict flags
# and no -isystem (NO_SYSTEM_FROM_IMPORTED took effect). Prints FAIL lines.
check_link() {
  cl_rc=0
  if ! (cd "$1" && ninja -t commands consumer) > "$T/cmds" 2> "$T/cmds.err"; then
    cat "$T/cmds.err"
    printf -- '-- FAIL pkg-link-path: %s\n' "ninja -t commands failed"
    return 1
  fi
  grep -F -e 'libivs_core.a' "$T/cmds" > "$T/link.line"
  cl_n=$(grep -c . "$T/link.line")
  if [ "$cl_n" -ne 1 ]; then
    printf -- '-- FAIL pkg-link-path: %s\n' "$cl_n commands name libivs_core.a"
    cl_rc=1
  elif ! grep -q -F -e "$pb/lib/libivs_core.a" "$T/link.line" && ! grep -q -F -e "$ppb/lib/libivs_core.a" "$T/link.line"; then
    printf -- '-- FAIL pkg-link-path: %s\n' "the link line does not name <prefix>/lib/libivs_core.a"
    cl_rc=1
  fi
  cl_k=$(awk '{ n += gsub(/ivs_core/, "") } END { print n + 0 }' "$T/link.line")
  if [ "$cl_k" -ne 1 ]; then
    printf -- '-- FAIL pkg-link-path: %s\n' "$cl_k ivs_core names on the link line"
    cl_rc=1
  fi
  grep -F -e '-o CMakeFiles/consumer.dir/main.c.o' "$T/cmds" > "$T/compile.line"
  if [ "$(grep -c . "$T/compile.line")" -ne 1 ]; then
    printf -- '-- FAIL pkg-consumer-flags: %s\n' "no single compile command for main.c.o"
    cl_rc=1
  else
    for cl_f in -std=c17 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror; do
      grep -q -F -e " $cl_f " "$T/compile.line" || { printf -- '-- FAIL pkg-consumer-flags: %s\n' "missing $cl_f"; cl_rc=1; }
    done
    if grep -q -F -e '-isystem' "$T/compile.line"; then
      printf -- '-- FAIL pkg-consumer-flags: %s\n' "-isystem in the compile command"
      cl_rc=1
    fi
  fi
  return "$cl_rc"
}
# check_ivs_dir <build dir>: ivs_DIR of the consumer cache lies under the moved prefix, compared
# in pwd -P form. Prints FAIL lines.
check_ivs_dir() {
  ci_dir=$(sed -n 's/^ivs_DIR:PATH=//p' "$1/CMakeCache.txt")
  if [ -z "$ci_dir" ] || [ ! -d "$ci_dir" ]; then
    printf -- '-- FAIL pkg-ivs-dir: %s\n' "ivs_DIR unset or not a directory: '$ci_dir'"
    return 1
  fi
  ci_pdir=$(cd "$ci_dir" && pwd -P) || ci_pdir=
  case $ci_pdir in
    "$ppb"/*) return 0 ;;
  esac
  printf -- '-- FAIL pkg-ivs-dir: %s\n' "$ci_dir is not under the moved prefix"
  return 1
}
# check_one_golden <golden> <prefix> <output>: the consumer's <prefix>digest.<domain> lines equal the
# digest.<domain> lines of the runner golden of the same scenario (D-24, D-39), and its domains are
# exactly the golden's observed ones (the composite apart). Prints FAIL lines.
check_one_golden() {
  co_gold=$1; co_pre=$2; co_out=$3
  co_lab=$(printf '%s' "$co_pre" | tr -d '\\')
  if [ ! -f "$co_gold" ]; then
    printf -- '-- FAIL pkg-runner-golden-diff: %s\n' "${co_gold#"$ROOT/"} missing"
    return 1
  fi
  cr_rc=0
  grep '^digest\.' "$co_gold" | sort > "$T/rg.want"
  sed -n "s/^${co_pre}digest\./digest./p" "$co_out" | sort > "$T/rg.have"
  if [ ! -s "$T/rg.want" ] || ! cmp -s "$T/rg.want" "$T/rg.have"; then
    printf -- '-- FAIL pkg-runner-golden-diff: %s\n' \
      "${co_lab}golden only: $(comm -23 "$T/rg.want" "$T/rg.have" | awk '{ print $1 }' | tr '\n' ' ')consumer only: $(comm -13 "$T/rg.want" "$T/rg.have" | awk '{ print $1 }' | tr '\n' ' ')"
    cr_rc=1
  fi
  sed -n 's/^# domains: //p' "$co_gold" | tr ' ' '\n' | sed -n 's/=observed$//p' | sort > "$T/rg.dom.want"
  sed -n "s/^${co_pre}digest\.\([a-z]*\) .*/\1/p" "$co_out" | grep -v -x -e composite | sort > "$T/rg.dom.have"
  if [ ! -s "$T/rg.dom.want" ] || ! cmp -s "$T/rg.dom.want" "$T/rg.dom.have"; then
    printf -- '-- FAIL pkg-runner-golden-diff: %s\n' \
      "${co_lab}observed domains: golden $(tr '\n' ' ' < "$T/rg.dom.want")consumer $(tr '\n' ' ' < "$T/rg.dom.have")"
    cr_rc=1
  fi
  return "$cr_rc"
}
# check_runner_golden <output>: the replay of diag-input-pass against its golden, and (plan 05-11) the
# replay of diag-ctrl-psg-pass (lines prefixed ctrl.) against data/golden/diag-ctrl-psg-pass.digests.
rgold="$ROOT/data/golden/diag-input-pass.digests"
rgold2="$ROOT/data/golden/diag-ctrl-psg-pass.digests"
check_runner_golden() {
  crg_rc=0
  check_one_golden "$rgold" "" "$1" || crg_rc=1
  check_one_golden "$rgold2" "ctrl\\." "$1" || crg_rc=1
  return "$crg_rc"
}
# check_calls <object>: the ivs_* names the consumer object references (nm -u, normalized) equal
# data/expected/pkg-consumer-calls.ids. Prints FAIL lines.
callids="$ROOT/data/expected/pkg-consumer-calls.ids"
check_calls() {
  ck_rc=0
  if [ ! -f "$callids" ]; then
    printf -- '-- FAIL pkg-consumer-calls: %s\n' "data/expected/pkg-consumer-calls.ids missing"
    return 1
  fi
  ck_dir=$(dirname "$1")
  ck_base=$(basename "$1")
  if ! (cd "$ck_dir" && "$NM" -P -u "./$ck_base") > "$T/calls.nm" 2> "$T/calls.nm.err"; then
    cat "$T/calls.nm.err"
    printf -- '-- FAIL pkg-consumer-calls: %s\n' "nm -u failed"
    return 1
  fi
  norm < "$T/calls.nm" | grep '^ivs_' | sort -u > "$T/calls.have"
  awk '/^#/ { next } NF { print $1 }' "$callids" | sort -u > "$T/calls.want"
  [ -s "$T/calls.want" ] || { printf -- '-- FAIL pkg-consumer-calls: %s\n' "no names"; return 1; }
  comm -23 "$T/calls.want" "$T/calls.have" > "$T/calls.missing"
  comm -13 "$T/calls.want" "$T/calls.have" > "$T/calls.extra"
  while IFS= read -r ck_n; do printf -- '-- FAIL pkg-consumer-calls: %s\n' "not referenced: $ck_n"; ck_rc=1; done < "$T/calls.missing"
  while IFS= read -r ck_n; do printf -- '-- FAIL pkg-consumer-calls: %s\n' "not pinned: $ck_n"; ck_rc=1; done < "$T/calls.extra"
  return "$ck_rc"
}
csrc="$T/consumer src"
cbld="$T/consumer build"
cout="$T/consumer.out"
consumer_lines=0
img_exec="$rel/images/boot-stub.exec"
img_cart="$rel/images/diag-cpu-1.ivsc"
img_cart2="$rel/images/diag-ctrl-psg-1.ivsc"
consumer_built=0
consumer_ok=0
if [ ! -f "$img_exec" ] || [ ! -f "$img_cart" ] || [ ! -f "$img_cart2" ]; then
  bad pkg-consumer-images "build/release/images/{boot-stub.exec,diag-cpu-1.ivsc,diag-ctrl-psg-1.ivsc}"
elif ! cp -R "$ROOT/tests/installed-consumer" "$csrc"; then
  bad pkg-consumer-copy "tests/installed-consumer"
elif ! consumer_configure "$csrc" "$cbld" > "$T/consumer.cfg.log" 2>&1; then
  cat "$T/consumer.cfg.log"
  bad pkg-consumer-configure "find_package(ivs 0.1 CONFIG REQUIRED) against the moved prefix"
elif ! cmake --build "$cbld" > "$T/consumer.build.log" 2>&1; then
  cat "$T/consumer.build.log"
  bad pkg-consumer-build "strict C17 build"
else
  consumer_built=1
  cst=0
  "$cbld/consumer" "$img_exec" "$img_cart" "$img_cart2" > "$cout" 2> "$T/consumer.err" || cst=$?
  mkdir -p "$gate_dir" && cp "$cout" "$gate_dir/pkg-consumer.txt" 2> /dev/null || bad pkg-consumer-output "build/gate/pkg-consumer.txt"
  consumer_lines=$(grep -c . "$cout")
  if [ "$cst" -ne 0 ]; then
    sed 's/^/# /' "$cout" "$T/consumer.err"
    bad pkg-consumer-run "exit $cst"
  else
    consumer_ok=1
  fi
  if ! check_golden "$cout"; then
    diff "$T/golden.body" "$cout" | sed 's/^/# /' | head -n 20
    rc=1
  fi
fi
# Independence and agreement, from the consumer build's own records (D-37 d, e; D-39).
if [ "$consumer_built" -eq 1 ]; then
  check_deps "$cbld" "$csrc" || rc=1
  check_link "$cbld" || rc=1
  check_ivs_dir "$cbld" || rc=1
  check_calls "$cbld/CMakeFiles/consumer.dir/main.c.o" || rc=1
  if [ "$consumer_ok" -eq 1 ]; then
    check_runner_golden "$cout" || rc=1
  fi
fi
# Direct C++ compile and link of a tiny main against the installed headers and archive (D-38):
# a missing extern "C" shows at link time. A visible skip without a C++ compiler.
cxx=${CXX:-c++}
cxx_stat=skip
if ! command -v "$cxx" > /dev/null 2>&1; then
  printf -- '-- pkg-cxx-link SKIP no C++ compiler (%s)\n' "$cxx"
elif [ ! -f "$csrc/main.cpp" ]; then
  bad pkg-cxx-link "tests/installed-consumer/main.cpp missing from the copy"
else
  mkdir -p "$T/cxx"
  if ! "$cxx" -std=c++17 -pedantic -Wall -Wextra -Werror -I "$pb/include" -o "$T/cxx/cxx-consumer" "$csrc/main.cpp" \
    "$pb/lib/libivs_core.a" > "$T/cxx/build.log" 2>&1; then
    cat "$T/cxx/build.log"
    bad pkg-cxx-link "compile or link against the installed archive"
  elif ! "$T/cxx/cxx-consumer" > "$T/cxx/out" 2>&1 || ! grep -q -x -F -e 'cxx-link ok=yes' "$T/cxx/out"; then
    cat "$T/cxx/out"
    bad pkg-cxx-link "run"
  else
    cxx_stat=ok
  fi
fi

# 7. Selftests (D-40): each case mutates a copy under $T/st/<case> (or probes the deleted
# prefix) and must fail for its intended reason; the cases are exactly the names of
# data/expected/pkg-selftests.ids. Controls cannot drive this step (it needs a built tree).
cc=${CC:-cc}
vmaj=$(sed -n 's/^#define IVS_API_VERSION_MAJOR \([0-9][0-9]*\)[uU]\{0,1\}$/\1/p' "$ROOT/core/include/ivs/ivs.h")
vmin=$(sed -n 's/^#define IVS_API_VERSION_MINOR \([0-9][0-9]*\)[uU]\{0,1\}$/\1/p' "$ROOT/core/include/ivs/ivs.h")
vpat=$(sed -n 's/^#define IVS_API_VERSION_PATCH \([0-9][0-9]*\)[uU]\{0,1\}$/\1/p' "$ROOT/core/include/ivs/ivs.h")
ppb=$(cd "$pb" && pwd -P) || exit 1
mkdir -p "$T/probe/src"
cat > "$T/probe/src/CMakeLists.txt" << 'CMK'
cmake_minimum_required(VERSION 3.25)
project(pkgprobe LANGUAGES NONE)
find_package(ivs ${IVS_WANT} CONFIG REQUIRED NO_PACKAGE_ROOT_PATH NO_CMAKE_ENVIRONMENT_PATH
  NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_PACKAGE_REGISTRY NO_CMAKE_SYSTEM_PATH
  NO_CMAKE_INSTALL_PREFIX NO_CMAKE_SYSTEM_PACKAGE_REGISTRY)
message(STATUS "pkgprobe version=${ivs_VERSION} dir=${ivs_DIR}")
CMK
# probe <name> <prefix> <version>: configure the LANGUAGES NONE probe against <prefix> asking for
# <version>, with the package path variables unset; output in $T/probe/<name>.log.
probe() {
  (
    unset CMAKE_PREFIX_PATH ivs_DIR ivs_ROOT IVS_ROOT CMAKE_INSTALL_PREFIX
    cmake -G Ninja -S "$T/probe/src" -B "$T/probe/$1" -DCMAKE_PREFIX_PATH="$2" -DIVS_WANT="$3" \
      -DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF -DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=OFF
  ) > "$T/probe/$1.log" 2>&1
}
# run_case <name>: 0 the case failed as intended, 1 it did not ($detail says why), 2 unknown.
run_case() {
  detail=
  d="$T/st/$1"
  mkdir -p "$d"
  case $1 in
    core-src-include)
      # An internal core/src header is not reachable from the prefix include dir alone, and the
      # same TU compiles once core/src is added (so the failure is the include path).
      printf '#include "common/ivs_sha256.h"\nint main(void) { return 0; }\n' > "$d/tu.c"
      if "$cc" -std=c17 -fsyntax-only -I "$pb/include" "$d/tu.c" > "$d/out" 2>&1; then
        detail="compiled with only the prefix include dir"; return 1
      fi
      grep -q -F 'ivs_sha256.h' "$d/out" || { detail="failed without naming ivs_sha256.h"; return 1; }
      "$cc" -std=c17 -fsyntax-only -I "$pb/include" -I "$ROOT/core/src" "$d/tu.c" > "$d/out2" 2>&1 \
        || { detail="does not compile with core/src either"; return 1; }
      ;;
    missing-symbol)
      cp "$arch" "$d/lib.a" || { detail="copy"; return 1; }
      ar d "$d/lib.a" machine_load.c.o > "$d/ar.log" 2>&1 || { detail="ar d machine_load.c.o failed"; return 1; }
      if check_symbols "$pb/include" "$d/lib.a" > "$d/out"; then detail="symbol rules passed"; return 1; fi
      grep -q -x -F -e '-- FAIL pkg-symbol-missing: ivs_machine_load' "$d/out" \
        || { detail="no pkg-symbol-missing: ivs_machine_load"; return 1; }
      ;;
    ghost-decl)
      cp -R "$pb/include" "$d/include" || { detail="copy"; return 1; }
      printf 'IVS_API int ivs_ghost(void);\n' >> "$d/include/ivs/ivs.h"
      if check_symbols "$d/include" "$arch" > "$d/out"; then detail="symbol rules passed"; return 1; fi
      grep -q -x -F -e '-- FAIL pkg-symbol-missing: ivs_ghost' "$d/out" \
        || { detail="no pkg-symbol-missing: ivs_ghost"; return 1; }
      ;;
    unprefixed)
      printf 'int sha256_init(void);\nint sha256_init(void) { return 0; }\n' > "$d/sha.c"
      cp "$arch" "$d/lib.a" || { detail="copy"; return 1; }
      "$cc" -std=c17 -c "$d/sha.c" -o "$d/sha.o" > "$d/cc.log" 2>&1 || { detail="cc failed"; return 1; }
      ar r "$d/lib.a" "$d/sha.o" > "$d/ar.log" 2>&1 || { detail="ar r failed"; return 1; }
      if check_symbols "$pb/include" "$d/lib.a" > "$d/out"; then detail="symbol rules passed"; return 1; fi
      grep -q -x -F -e '-- FAIL pkg-symbol-namespace: sha256_init' "$d/out" \
        || { detail="no pkg-symbol-namespace: sha256_init"; return 1; }
      ;;
    extra-file)
      cp -R "$pb" "$d/prefix" || { detail="copy"; return 1; }
      mkdir -p "$d/prefix/share/ivs" && printf 'stray\n' > "$d/prefix/share/ivs/stray.txt"
      if check_inventory "$d/prefix" > "$d/out"; then detail="inventory passed"; return 1; fi
      grep -q -x -F -e '-- FAIL pkg-inventory-extra: share/ivs/stray.txt' "$d/out" \
        || { detail="no pkg-inventory-extra: share/ivs/stray.txt"; return 1; }
      ;;
    baked-path)
      cp -R "$pb" "$d/prefix" || { detail="copy"; return 1; }
      printf '# %s/build/release\n' "$ROOT" >> "$d/prefix/lib/cmake/ivs/ivsTargets.cmake"
      if check_paths "$d/prefix" > "$d/out"; then detail="path check passed"; return 1; fi
      grep -q -x -F -e '-- FAIL pkg-path-leak: lib/cmake/ivs/ivsTargets.cmake' "$d/out" \
        || { detail="no pkg-path-leak: lib/cmake/ivs/ivsTargets.cmake"; return 1; }
      ;;
    version-refused)
      # SameMinorVersion: the built major.minor is found under the moved prefix (positive),
      # the next minor and the next major are refused (D-40: 0.2 and 1 for 0.1.0).
      [ -n "$vmaj" ] && [ -n "$vmin" ] && [ -n "$vpat" ] || { detail="cannot parse the ivs.h version"; return 1; }
      probe accept "$pb" "$vmaj.$vmin" || { detail="find_package($vmaj.$vmin) failed: $(tail -n 3 "$T/probe/accept.log" | tr '\n' ' ')"; return 1; }
      got=$(sed -n 's/^-- pkgprobe version=\([^ ]*\) dir=\(.*\)$/\1/p' "$T/probe/accept.log")
      gdir=$(sed -n 's/^-- pkgprobe version=\([^ ]*\) dir=\(.*\)$/\2/p' "$T/probe/accept.log")
      [ "$got" = "$vmaj.$vmin.$vpat" ] || { detail="found version '$got', wanted $vmaj.$vmin.$vpat"; return 1; }
      case $gdir in
        "$pb"/*|"$ppb"/*) ;;
        *) detail="ivs_DIR '$gdir' is not under the moved prefix"; return 1 ;;
      esac
      # An older minor too (when there is one): AnyNewerVersion would accept it, SameMinorVersion
      # must not.
      ws="$vmaj.$((vmin + 1)) $((vmaj + 1))"
      [ "$vmin" -eq 0 ] || ws="$ws $vmaj.$((vmin - 1))"
      for w in $ws; do
        if probe "refuse-$w" "$pb" "$w"; then detail="find_package($w) accepted $vmaj.$vmin.$vpat"; return 1; fi
        grep -q -F "$vmaj.$vmin.$vpat" "$T/probe/refuse-$w.log" \
          || { detail="find_package($w) failed without naming the rejected version"; return 1; }
      done
      ;;
    old-prefix)
      [ ! -e "$pa" ] || { detail="the original prefix still exists"; return 1; }
      if probe old "$pa" "$vmaj.$vmin"; then detail="find_package found a package under the deleted prefix"; return 1; fi
      grep -q -F 'ivsConfig.cmake' "$T/probe/old.log" || { detail="failed without the not-found message"; return 1; }
      ;;
    leak-detector)
      # A consumer copy that adds the repository's core/src include dir and a TU including an
      # internal header: it builds (the compiler alone does not object), and the dependency
      # checker must report pkg-leak naming that header (proves the detector, not the compiler).
      cp -R "$ROOT/tests/installed-consumer" "$d/src" || { detail="copy"; return 1; }
      printf '#include "common/ivs_sha256.h"\nint ivs_consumer_leak(void);\nint ivs_consumer_leak(void) { return 0; }\n' \
        > "$d/src/leak.c"
      printf 'target_sources(consumer PRIVATE leak.c)\ntarget_include_directories(consumer PRIVATE "${IVS_LEAK_DIR}")\n' \
        >> "$d/src/CMakeLists.txt"
      consumer_configure "$d/src" "$d/build" -DIVS_LEAK_DIR="$ROOT/core/src" > "$d/cfg.log" 2>&1 \
        || { detail="configure failed: $(tail -n 3 "$d/cfg.log" | tr '\n' ' ')"; return 1; }
      cmake --build "$d/build" > "$d/build.log" 2>&1 \
        || { detail="the leaking copy does not build: $(tail -n 3 "$d/build.log" | tr '\n' ' ')"; return 1; }
      if check_deps "$d/build" "$d/src" > "$d/out"; then detail="dependency check passed"; return 1; fi
      grep -q -x -F -e "-- FAIL pkg-leak: $ROOT/core/src/common/ivs_sha256.h" "$d/out" \
        || grep -q -x -F -e "-- FAIL pkg-leak: $proot/core/src/common/ivs_sha256.h" "$d/out" \
        || { detail="no pkg-leak naming core/src/common/ivs_sha256.h"; return 1; }
      ;;
    golden-drift)
      # The consumer output with one digest nibble changed must fail the pin (pkg-golden-diff);
      # the unchanged output matches it (so the failure is the nibble).
      [ -s "$cout" ] || { detail="no consumer output"; return 1; }
      check_golden "$cout" > "$d/pos" || { detail="the unchanged output does not match the pin"; return 1; }
      awk 'done == 0 && /^digest\./ {
          c = substr($0, length($0), 1); $0 = substr($0, 1, length($0) - 1) (c == "0" ? "1" : "0"); done = 1
        } { print }' "$cout" > "$d/drift.txt"
      if cmp -s "$cout" "$d/drift.txt"; then detail="the mutation changed nothing"; return 1; fi
      if check_golden "$d/drift.txt" > "$d/out"; then detail="golden check passed"; return 1; fi
      grep -q -F -e '-- FAIL pkg-golden-diff:' "$d/out" || { detail="no pkg-golden-diff"; return 1; }
      ;;
    ctrl-digest-drift)
      # One nibble of a ctrl.digest line changed must fail the comparison with the runner golden of
      # diag-ctrl-psg-pass (pkg-runner-golden-diff); the unchanged output matches it.
      [ -s "$cout" ] || { detail="no consumer output"; return 1; }
      check_one_golden "$rgold2" "ctrl\\." "$cout" > "$d/pos" || { detail="the unchanged output does not match the runner golden"; return 1; }
      awk 'done == 0 && /^ctrl\.digest\./ {
          c = substr($0, length($0), 1); $0 = substr($0, 1, length($0) - 1) (c == "0" ? "1" : "0"); done = 1
        } { print }' "$cout" > "$d/drift.txt"
      if cmp -s "$cout" "$d/drift.txt"; then detail="the mutation changed nothing"; return 1; fi
      if check_one_golden "$rgold2" "ctrl\\." "$d/drift.txt" > "$d/out"; then detail="runner golden check passed"; return 1; fi
      grep -q -F -e '-- FAIL pkg-runner-golden-diff:' "$d/out" || { detail="no pkg-runner-golden-diff"; return 1; }
      ;;
    ctrl-domain-missing)
      # The ctrl.digest.psglog line dropped (a consumer that never read the write log) must fail the
      # comparison: the observed domains differ from the golden's.
      [ -s "$cout" ] || { detail="no consumer output"; return 1; }
      grep -v '^ctrl\.digest\.psglog ' "$cout" > "$d/missing.txt"
      if cmp -s "$cout" "$d/missing.txt"; then detail="the output has no ctrl.digest.psglog line"; return 1; fi
      if check_one_golden "$rgold2" "ctrl\\." "$d/missing.txt" > "$d/out"; then detail="runner golden check passed"; return 1; fi
      grep -q -F -e 'ctrl.observed domains' "$d/out" || { detail="no observed-domains finding"; return 1; }
      ;;
    ctrl-peek-wrong)
      # The peeked $01FE after the first event is pinned (0x007E for KEY_1); another value fails the
      # consumer pin (pkg-golden-diff) while the unchanged output matches it.
      [ -s "$cout" ] || { detail="no consumer output"; return 1; }
      grep -q -x -F -e 'ctrl.peek 01FE=0x007E' "$cout" || { detail="the consumer did not print ctrl.peek 01FE=0x007E"; return 1; }
      check_golden "$cout" > "$d/pos" || { detail="the unchanged output does not match the pin"; return 1; }
      sed 's/^ctrl\.peek 01FE=0x007E$/ctrl.peek 01FE=0x007F/' "$cout" > "$d/peek.txt"
      if cmp -s "$cout" "$d/peek.txt"; then detail="the mutation changed nothing"; return 1; fi
      if check_golden "$d/peek.txt" > "$d/out"; then detail="golden check passed"; return 1; fi
      grep -q -F -e '-- FAIL pkg-golden-diff:' "$d/out" || { detail="no pkg-golden-diff"; return 1; }
      ;;
    *) return 2 ;;
  esac
  return 0
}
sinv="$ROOT/data/expected/pkg-selftests.ids"
st_ok=0
st_total=0
if [ -f "$sinv" ]; then
  awk '/^#/ { next } NF { print $1 }' "$sinv" > "$T/st.cases"
  [ -s "$T/st.cases" ] || bad pkg-selftest-inventory "no cases"
  while IFS= read -r c; do
    st_total=$((st_total + 1))
    cst=0
    run_case "$c" < /dev/null || cst=$?
    case $cst in
      0) st_ok=$((st_ok + 1)) ;;
      2) bad pkg-selftest-missing "$c" ;;
      *) bad "pkg-selftest-$c" "$detail" ;;
    esac
  done < "$T/st.cases"
else
  bad pkg-selftest-inventory "data/expected/pkg-selftests.ids"
fi

[ "$rc" -eq 0 ] || exit 1
printf -- '-- pkg-install pass prefix-files=%s archive=%s public-symbols=%s defined=%s %s consumer-lines=%s cxx-link=%s pkg-selftest=%s/%s\n' \
  "$files" "$archive_mode" "$pubsyms" "$defined" "$hdr_stat" "$consumer_lines" "$cxx_stat" "$st_ok" "$st_total"
