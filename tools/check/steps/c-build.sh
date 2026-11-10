#!/bin/sh
# SPDX-License-Identifier: MIT
# Step c-build: configure and build the dev and release presets (D-19, D-22), plus the
# named-fault executables (target ivs_faults) in the dev tree (D-18), then fail on any
# tracked or untracked-not-ignored core/**/*.c that the build does not compile (the build
# writes its repo-relative source list to build/dev/ivs-core-sources.txt). Build logs are
# printed only on failure.
# Tokens: c-build-failed: <preset>, c-build-unlisted-source: <path>.
# Stats: cc=<CMAKE_C_COMPILER_ID of the dev tree> c-presets=<n>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/c-build.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }
cd "$ROOT" || exit 1

built=0
for preset in dev release; do
  if cmake --preset "$preset" > "$tmp/$preset.configure.log" 2>&1 \
    && cmake --build --preset "$preset" > "$tmp/$preset.build.log" 2>&1; then
    built=$((built + 1))
  else
    cat "$tmp/$preset.configure.log"
    [ -f "$tmp/$preset.build.log" ] && cat "$tmp/$preset.build.log"
    bad c-build-failed "$preset"
    continue
  fi
  if [ "$preset" = dev ] && ! cmake --build --preset dev --target ivs_faults > "$tmp/dev.faults.log" 2>&1; then
    cat "$tmp/dev.faults.log"
    bad c-build-failed "dev (ivs_faults)"
  fi
done

listed="$ROOT/build/dev/ivs-core-sources.txt"
grep -E '^core/.*[.]c$' "$FILELIST" | sort -u > "$tmp/tracked"
if [ -f "$listed" ]; then
  sort -u "$listed" > "$tmp/listed"
  for p in $(comm -23 "$tmp/tracked" "$tmp/listed"); do bad c-build-unlisted-source "$p"; done
else
  bad c-build-failed "dev (no ivs-core-sources.txt)"
fi

cc=
cfg=$(ls "$ROOT"/build/dev/CMakeFiles/*/CMakeCCompiler.cmake 2> /dev/null | sort | head -n 1)
if [ -n "$cfg" ]; then
  cc=$(sed -n 's/^set(CMAKE_C_COMPILER_ID "\([^"]*\)")$/\1/p' "$cfg" | head -n 1)
fi
[ -n "$cc" ] || bad c-build-failed "dev (no compiler id)"

[ "$rc" -eq 0 ] || exit 1
printf -- '-- c-build pass cc=%s c-presets=%s\n' "$cc" "$built"
