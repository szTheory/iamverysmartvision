#!/bin/sh
# SPDX-License-Identifier: MIT
# Step runner-selftest: the negative-control runner itself rejects bad controls (D-12).
# Builds a throwaway controls tree (a copy of _good plus four synthetic controls that use
# the real profile step) and an inventory missing one of them, runs the real runner on it
# and requires a nonzero exit and all four runner tokens, each naming the right control.
# Reads ROOT, CHECK_DIR and FILELIST only (ROOT and FILELIST are unused: the tree is synthetic).
# Tokens: runner-selftest-missing: <token>.
# Stats: cases=<n>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/selftest.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }

ctl="$tmp/controls"
inv="$tmp/inventory"
mkdir -p "$ctl" "$inv"
cp -R "$CHECK_DIR/controls/_good" "$ctl/_good" || { bad runner-selftest-missing "setup"; exit 1; }
good_file=data/profiles/intv-2609-ntsc-family-p1.json

# Blank EXPECT line: would match every output, so the runner must refuse it.
mkdir -p "$ctl/st-expect-blank"
printf 'profile\n' > "$ctl/st-expect-blank/CHECK"
printf 'FAIL profile-file-missing\n\n' > "$ctl/st-expect-blank/EXPECT"
printf 'delete %s\n' "$good_file" > "$ctl/st-expect-blank/MAKE"

# Vacuous control: changes nothing, so the real step passes and the runner must refuse it.
mkdir -p "$ctl/st-exit-zero"
printf 'profile\n' > "$ctl/st-exit-zero/CHECK"
printf 'FAIL profile-file-missing\n' > "$ctl/st-exit-zero/EXPECT"

# Wrong token: the step fails, but not with the expected token.
mkdir -p "$ctl/st-token-missing"
printf 'profile\n' > "$ctl/st-token-missing/CHECK"
printf 'FAIL selftest-no-such-token\n' > "$ctl/st-token-missing/EXPECT"
printf 'delete %s\n' "$good_file" > "$ctl/st-token-missing/MAKE"

# Uninventoried control: a well-formed directory that is absent from the inventory.
mkdir -p "$ctl/st-not-in-inventory"
printf 'profile\n' > "$ctl/st-not-in-inventory/CHECK"
printf 'FAIL profile-file-missing\n' > "$ctl/st-not-in-inventory/EXPECT"
printf 'delete %s\n' "$good_file" > "$ctl/st-not-in-inventory/MAKE"

printf '%s\n' st-expect-blank st-exit-zero st-token-missing > "$inv/selftest.ids"

if CONTROLS_DIR="$ctl" CONTROLS_INVENTORY_DIR="$inv" sh "$CHECK_DIR/run-controls.sh" > "$tmp/out" 2>&1; then
  bad runner-selftest-missing "runner-exit-nonzero"
fi
cases=0
for pair in control-expect-blank:st-expect-blank control-exit-zero:st-exit-zero \
  control-token-missing:st-token-missing control-not-in-inventory:st-not-in-inventory; do
  tok=${pair%%:*}
  name=${pair#*:}
  if grep -q -F -e "-- FAIL $tok: $name" "$tmp/out"; then
    cases=$((cases + 1))
  else
    bad runner-selftest-missing "$tok"
  fi
done
if grep -q -F -e "control-good-failed" "$tmp/out"; then
  bad runner-selftest-missing "positive-control-on-good-tree"
fi

[ "$rc" -eq 0 ] || exit 1
printf -- '-- runner-selftest pass cases=%s\n' "$cases"
