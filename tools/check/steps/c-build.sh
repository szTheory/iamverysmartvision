#!/bin/sh
# SPDX-License-Identifier: MIT
# Step c-build: configure and build the dev and release presets (D-19, D-22), plus the
# named-fault executables (target ivs_faults) in the dev tree (D-18), then fail on any
# tracked or untracked-not-ignored core/**/*.c, runner/**/*.c, tools/pack/**/*.c, tools/pcm/**/*.c or
# tools/asm/**/*.c that the build does not compile (the build writes its repo-relative source lists to
# build/dev/ivs-core-sources.txt, since plan 04-04 (D-24) build/dev/ivs-runner-sources.txt, since plan 04-14
# (D-30) build/dev/ivs-pack-sources.txt, since plan 05-10 (D-21, A-07) build/dev/ivs-pcm-sources.txt and
# since plan 07-03 (D-17) build/dev/ivs-asm-sources.txt).
# The control tree (tools/check/controls/_good/tree) is a toy CMake project with the same presets and
# the five lists, so this step has a positive and a negative control per tool directory
# (c-build-unlisted-pcm-source, c-build-unlisted-asm-source).
# Build logs are printed only on failure.
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

# Each item: <tracked-path regex> <repo-relative source list the build writes to build/dev>.
for mod in '^core/.*[.]c$ ivs-core-sources.txt' '^runner/.*[.]c$ ivs-runner-sources.txt' \
  '^tools/pack/.*[.]c$ ivs-pack-sources.txt' '^tools/pcm/.*[.]c$ ivs-pcm-sources.txt' \
  '^tools/asm/.*[.]c$ ivs-asm-sources.txt'; do
  re=${mod%% *}
  name=${mod#* }
  listed="$ROOT/build/dev/$name"
  grep -E "$re" "$FILELIST" | sort -u > "$tmp/tracked"
  if [ -f "$listed" ]; then
    sort -u "$listed" > "$tmp/listed"
    for p in $(comm -23 "$tmp/tracked" "$tmp/listed"); do bad c-build-unlisted-source "$p"; done
  else
    bad c-build-failed "dev (no $name)"
  fi
done

# The compiler id comes from the CMakeFiles/<version>/ directory of the CMake version that wrote
# the cache, so a stale directory left by an older CMake is never read (review IN-04).
cc=
cfg=
cache="$ROOT/build/dev/CMakeCache.txt"
if [ -f "$cache" ]; then
  ver=$(awk -F= '/^CMAKE_CACHE_MAJOR_VERSION:/ { a = $2 } /^CMAKE_CACHE_MINOR_VERSION:/ { b = $2 }
    /^CMAKE_CACHE_PATCH_VERSION:/ { c = $2 } END { if (a != "" && b != "" && c != "") print a "." b "." c }' "$cache")
  if [ -n "$ver" ] && [ -f "$ROOT/build/dev/CMakeFiles/$ver/CMakeCCompiler.cmake" ]; then
    cfg="$ROOT/build/dev/CMakeFiles/$ver/CMakeCCompiler.cmake"
  fi
fi
if [ -n "$cfg" ]; then
  cc=$(sed -n 's/^set(CMAKE_C_COMPILER_ID "\([^"]*\)")$/\1/p' "$cfg" | head -n 1)
fi
[ -n "$cc" ] || bad c-build-failed "dev (no compiler id)"

[ "$rc" -eq 0 ] || exit 1
printf -- '-- c-build pass cc=%s c-presets=%s\n' "$cc" "$built"
