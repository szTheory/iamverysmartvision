#!/bin/sh
# SPDX-License-Identifier: MIT
# Step runner-selftest: the negative-control runner itself rejects bad controls (D-12).
# Builds a throwaway controls tree (a copy of _good plus eight synthetic controls that use
# the real profile step) and an inventory missing one of them, runs the real runner on it
# and requires a nonzero exit and all five runner tokens, each naming the right control
# (control-expect-blank twice: an empty line, and a whitespace-only last line without a
# newline; control-token-missing twice: a wrong single token, and a second token missing after
# a first one that matches), plus one well-formed control (two matching tokens, a FORBID file
# with a blank line and a last line without a newline) that the runner must accept. The
# blank-line and token cases pin the runner's builtin blank check and one-pass token match
# (D-45 lever, plan 04-LEVER-01).
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

# Whitespace-only last EXPECT line without a newline: blank too.
mkdir -p "$ctl/st-expect-space"
printf 'profile\n' > "$ctl/st-expect-space/CHECK"
printf 'FAIL profile-file-missing\n \t' > "$ctl/st-expect-space/EXPECT"
printf 'delete %s\n' "$good_file" > "$ctl/st-expect-space/MAKE"

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

# Second token missing: the first EXPECT token matches, the second does not; every token counts.
mkdir -p "$ctl/st-second-token-missing"
printf 'profile\n' > "$ctl/st-second-token-missing/CHECK"
printf 'FAIL profile-file-missing\nFAIL selftest-no-such-token\n' > "$ctl/st-second-token-missing/EXPECT"
printf 'delete %s\n' "$good_file" > "$ctl/st-second-token-missing/MAKE"

# Forbidden token: the expected token is there, and so is a FORBID token.
mkdir -p "$ctl/st-forbidden-token"
printf 'profile\n' > "$ctl/st-forbidden-token/CHECK"
printf 'FAIL profile-file-missing\n' > "$ctl/st-forbidden-token/EXPECT"
printf 'selftest-no-such-token\n-- FAIL profile\n' > "$ctl/st-forbidden-token/FORBID"
printf 'delete %s\n' "$good_file" > "$ctl/st-forbidden-token/MAKE"

# Well-formed control: both tokens match, no FORBID token occurs (blank FORBID lines are skipped).
mkdir -p "$ctl/st-ok"
printf 'profile\n' > "$ctl/st-ok/CHECK"
printf 'FAIL profile-file-missing\nprofile-file-missing' > "$ctl/st-ok/EXPECT"
printf '\nselftest-no-such-token' > "$ctl/st-ok/FORBID"
printf 'delete %s\n' "$good_file" > "$ctl/st-ok/MAKE"

printf '%s\n' st-expect-blank st-expect-space st-exit-zero st-token-missing st-second-token-missing \
  st-forbidden-token st-ok > "$inv/selftest.ids"

if CONTROLS_DIR="$ctl" CONTROLS_INVENTORY_DIR="$inv" sh "$CHECK_DIR/run-controls.sh" > "$tmp/out" 2>&1; then
  bad runner-selftest-missing "runner-exit-nonzero"
fi
cases=0
for pair in control-expect-blank:st-expect-blank control-expect-blank:st-expect-space \
  control-exit-zero:st-exit-zero \
  control-token-missing:st-token-missing control-not-in-inventory:st-not-in-inventory \
  control-token-missing:st-second-token-missing control-forbidden-token:st-forbidden-token; do
  tok=${pair%%:*}
  name=${pair#*:}
  if grep -q -F -e "-- FAIL $tok: $name" "$tmp/out"; then
    cases=$((cases + 1))
  else
    bad runner-selftest-missing "$tok"
  fi
done
if grep -q -x -F -e 'control ok   st-ok' "$tmp/out" && ! grep -q -e '^-- FAIL [a-z-]*: st-ok$' "$tmp/out"; then
  cases=$((cases + 1))
else
  bad runner-selftest-missing "accepted-control"
fi
if grep -q -F -e "control-good-failed" "$tmp/out"; then
  bad runner-selftest-missing "positive-control-on-good-tree"
fi

[ "$rc" -eq 0 ] || exit 1
printf -- '-- runner-selftest pass cases=%s\n' "$cases"
