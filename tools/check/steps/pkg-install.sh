#!/bin/sh
# SPDX-License-Identifier: MIT
# Step pkg-install: the installed, relocated package (PKG-01, D-33, D-34, D-37, plan 04-22).
# Installs the release tree that c-build already built (no third build; c-build sorts before
# this step) into a temp prefix, moves the prefix into a path that contains a space, deletes
# the original prefix and asserts it is gone, then checks the moved prefix:
#   inventory  the installed files (relative paths, sorted) equal data/expected/pkg-install-files.ids
#              exactly; this is also the guard that no image or runner enters the package.
# Every temp file is under one mktemp dir outside the repo, removed on exit; the step writes
# nothing else. Logs are printed only on failure.
# Tokens: pkg-install-no-tree: <path>, pkg-install-failed: <what>, pkg-install-relocate: <what>,
#   pkg-inventory-missing: <path>, pkg-inventory-extra: <path>, pkg-inventory-list: <what>.
# Stats: prefix-files=<n>.
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

[ "$rc" -eq 0 ] || exit 1
printf -- '-- pkg-install pass prefix-files=%s\n' "$files"
