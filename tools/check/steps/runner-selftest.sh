#!/bin/sh
# SPDX-License-Identifier: MIT
# Step runner-selftest: the negative-control runner itself rejects bad controls (D-12).
# Builds a throwaway controls tree (a copy of _good plus nine synthetic controls that use
# the real profile step) and an inventory missing one of them, runs the real runner on it
# and requires a nonzero exit and all seven runner tokens, each naming the right control
# (control-expect-blank twice: an empty line, and a whitespace-only last line without a
# newline; control-token-missing twice: a wrong single token, and a second token missing after
# a first one that matches; control-toy-drift: an overlay registry row missing with no
# DEVIATION file; control-deviation-unused: a DEVIATION line that matches no difference),
# plus one well-formed control (two matching tokens, a FORBID file with a blank line and a
# last line without a newline) that the runner must accept. The blank-line and token cases pin
# the runner's builtin blank check and one-pass token match (D-45 lever, plan 04-LEVER-01);
# the two drift cases pin the toy-registry check (A-09, EV-04). These tokens live here because
# a control directory cannot prove them: the control runner would have to run itself. The
# pass-line case below proves control-pass-line-not-last the same way, through a copied runner.
# Reads ROOT, CHECK_DIR and FILELIST only (ROOT and FILELIST are unused: the tree is synthetic).
# One proof needs more than a synthetic controls tree: the pass-line guard lives in the runner's
# own good_one, so it can only be reached by a step whose output continues after its pass line.
# That case runs a COPY of the runner whose CHECK_DIR is a copy of tools/check with one printing
# line appended to steps/profile.sh, so this step also owns proofs needing a copied CHECK_DIR; the
# memmap generator-missing guard (EV-03 item 5) uses the same copy, because its path is derived
# from CHECK_DIR and no control directory can reach it.
# One more proof has no control and no copied runner: the pass-line rule the gate replay applies to
# all 38 steps (review WR-02) is a file, tools/check/pass-line.sh, and the case near the end proves
# that file refuses both shapes. The selftest-inventory rule (EV-07) is a file for the same reason,
# tools/check/selftest-inventory.sh: a guard inlined in this step would be reachable only by the step
# running itself, so the case `st-no-such-case` runs that file against a renamed inventory and
# requires its refusal, and against the real inventory and requires its acceptance.
# Tokens: runner-selftest-missing: <token>; runner-selftest-inventory: <path> (absent inventory, or an
# inventory with no non-comment line).
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
# The case names this run actually passed, so the inventory check at the tail can refuse a name the
# step does not run (EV-07: a registry row must not point at a renamed or deleted case).
ran="$tmp/ran"; : > "$ran"

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

# Toy registry drifts from the good tree with no DEVIATION file: one row removed, undeclared.
reg=data/expected/important-gates.ids
mkdir -p "$ctl/st-toy-drift/overlay/data/expected"
printf 'profile\n' > "$ctl/st-toy-drift/CHECK"
printf 'FAIL profile-file-missing\n' > "$ctl/st-toy-drift/EXPECT"
printf 'delete %s\n' "$good_file" > "$ctl/st-toy-drift/MAKE"
grep -v -x -F -e 'toy-regs fault GOOD_FAULT VID-01' "$ctl/_good/tree/$reg" > "$ctl/st-toy-drift/overlay/$reg"

# Toy registry identical to the good tree, but DEVIATION declares a difference that is not there.
mkdir -p "$ctl/st-deviation-unused/overlay/data/expected"
printf 'profile\n' > "$ctl/st-deviation-unused/CHECK"
printf 'FAIL profile-file-missing\n' > "$ctl/st-deviation-unused/EXPECT"
printf 'delete %s\n' "$good_file" > "$ctl/st-deviation-unused/MAKE"
cp "$ctl/_good/tree/$reg" "$ctl/st-deviation-unused/overlay/$reg"
printf '> toy-nothing fault GOOD_FAULT NOPE-01\n' > "$ctl/st-deviation-unused/DEVIATION"

printf '%s\n' st-expect-blank st-expect-space st-exit-zero st-token-missing st-second-token-missing \
  st-forbidden-token st-toy-drift st-deviation-unused st-ok > "$inv/selftest.ids"

if CONTROLS_DIR="$ctl" CONTROLS_INVENTORY_DIR="$inv" sh "$CHECK_DIR/run-controls.sh" > "$tmp/out" 2>&1; then
  bad runner-selftest-missing "runner-exit-nonzero"
fi
cases=0
for pair in control-expect-blank:st-expect-blank control-expect-blank:st-expect-space \
  control-exit-zero:st-exit-zero \
  control-token-missing:st-token-missing control-not-in-inventory:st-not-in-inventory \
  control-token-missing:st-second-token-missing control-forbidden-token:st-forbidden-token \
  control-toy-drift:st-toy-drift control-deviation-unused:st-deviation-unused; do
  tok=${pair%%:*}
  name=${pair#*:}
  if grep -q -F -e "-- FAIL $tok: $name" "$tmp/out"; then
    cases=$((cases + 1))
    printf '%s\n' "$name" >> "$ran"
  else
    bad runner-selftest-missing "$tok"
  fi
done
if grep -q -x -F -e 'control ok   st-ok' "$tmp/out" && ! grep -q -e '^-- FAIL [a-z-]*: st-ok$' "$tmp/out"; then
  cases=$((cases + 1))
  printf '%s\n' st-ok >> "$ran"
else
  bad runner-selftest-missing "accepted-control"
fi
if grep -q -F -e "control-good-failed" "$tmp/out"; then
  bad runner-selftest-missing "positive-control-on-good-tree"
fi

# Pass line not last (A-02/O2, EV-03). A control directory cannot reach the guard: the control
# runner would have to run itself. So copy the runner and its CHECK_DIR (everything but
# controls/, then controls/_good) and append one line that PRINTS after profile.sh's pass line.
# A bare '#' line is shell source and emits nothing, which would leave the guard silent and
# this case failing as runner-selftest-missing - a stand-in that cannot make the guard fire.
# The copy sits at <tmp>/checkcopy/tools/check so that its ../.. is <tmp>/checkcopy: a directory
# this case owns, which carries no cmake/. The generator-missing case below depends on that.
cc="$tmp/checkcopy/tools/check"
mkdir -p "$cc/controls"
for _e in "$CHECK_DIR"/*; do
  case ${_e##*/} in controls) continue ;; esac
  cp -R "$_e" "$cc/" || bad runner-selftest-missing "checkcopy"
done
cp -R "$CHECK_DIR/controls/_good" "$cc/controls/_good" || bad runner-selftest-missing "checkcopy-good"
printf '%s\n' "printf -- '# trailing comment after the pass line\\n'" >> "$cc/steps/profile.sh"
pl="$tmp/passline"
mkdir -p "$pl/controls/st-pass-line" "$pl/inventory"
cp -R "$CHECK_DIR/controls/_good" "$pl/controls/_good"
printf 'profile\n' > "$pl/controls/st-pass-line/CHECK"
printf 'FAIL profile-file-missing\n' > "$pl/controls/st-pass-line/EXPECT"
printf 'delete %s\n' "$good_file" > "$pl/controls/st-pass-line/MAKE"
printf '%s\n' st-pass-line > "$pl/inventory/selftest.ids"
if CONTROLS_DIR="$pl/controls" CONTROLS_INVENTORY_DIR="$pl/inventory" \
  sh "$cc/run-controls.sh" > "$tmp/passline.out" 2>&1; then
  bad runner-selftest-missing "pass-line-runner-exit-nonzero"
fi
if grep -q -F -e '-- FAIL control-pass-line-not-last: profile' "$tmp/passline.out"; then
  # Print the observed refusal so the guard's proof is visible in every gate log rather than living
  # only in a temp file this run throws away - but NOT at the start of the line. tools/gate.sh's
  # keep_fails collects every line matching '^-- FAIL ' into fail.lines, which ci-summary.sh renders
  # into the job summary; a refusal this case EXPECTS to observe would then be advertised as a
  # failure of a passing run. The '-- runner-selftest proof ' prefix keeps the token text readable
  # and keeps it out of that set.
  printf -- '-- runner-selftest proof control-pass-line-not-last: profile (observed: '
  grep -F -e '-- FAIL control-pass-line-not-last: profile' "$tmp/passline.out" | tr -d '\n'
  printf -- ')\n'
  cases=$((cases + 1))
  printf '%s\n' st-pass-line >> "$ran"
else
  bad runner-selftest-missing "control-pass-line-not-last"
fi

# Pass-line rule, gate side (review WR-02). The rule reaches all 38 steps only in tools/gate.sh's
# replay, and a gate run needs cmake, ninja and a compiler, so neither a control directory nor a
# case here can reach it by running the gate. The rule is therefore a file,
# tools/check/pass-line.sh, which the replay calls once per step; this case proves that file refuses
# both shapes rather than one: a verdict with a non-empty line after it (exit 2) and no verdict line
# at all (exit 1), and accepts the one shape the 38 steps must produce (exit 0). Without it the
# gate-side rule would be a check whose failure is recorded only as prose - the lesson-20 defect.
plout="$tmp/passline"
printf -- '-- profile pass stats=1\n' > "$plout.ok"
printf -- '-- profile pass stats=1\nstill reporting\n' > "$plout.late"
printf -- 'no verdict here\n' > "$plout.none"
pl_bad=0
sh "$CHECK_DIR/pass-line.sh" profile "$plout.ok" || pl_bad=1
sh "$CHECK_DIR/pass-line.sh" profile "$plout.late"
[ "$?" -eq 2 ] || pl_bad=1
sh "$CHECK_DIR/pass-line.sh" profile "$plout.none"
[ "$?" -eq 1 ] || pl_bad=1
if [ "$pl_bad" -eq 0 ]; then
  # Printed with the same non-collected prefix as the other proofs above.
  printf -- '-- runner-selftest proof pass-line-not-last: profile (exit 2 observed)\n'
  cases=$((cases + 1))
else
  bad runner-selftest-missing "pass-line-not-last"
fi

# Generator missing (EV-03 item 5). The memmap step derives the generator's path from CHECK_DIR,
# and a control may only change the tree, so no control directory can reach this guard. The copy
# above has no sibling cmake/, so the step must report the liveness guard and stop before invoking
# cmake at all. Without this case the guard's failure would be recorded only as prose - the
# lesson-20 defect GUARDRAILS 2.3 exists to prevent. The filelist is a runtime artifact; the
# script's own source must never spell the tree's generated directory name, because a step's
# partition class is derived by grepping this file.
mg_tree="$cc/controls/_good/tree"
mg_fl="$tmp/memmapgen.filelist"
find "$mg_tree" -type f | sed "s|^$mg_tree/||" | sort > "$mg_fl"
mgout=$(ROOT="$mg_tree" CHECK_DIR="$cc" FILELIST="$mg_fl" sh "$cc/steps/memmap.sh" 2>&1)
mgrc=$?
if [ "$mgrc" -eq 0 ]; then
  bad runner-selftest-missing "memmap-generator-missing"
elif printf '%s\n' "$mgout" | grep -q -F -e '-- FAIL memmap-generator-missing: cmake/IvsMemmapProbes.cmake'; then
  # Printed with the same non-collected prefix as the pass-line proof above.
  printf -- '-- runner-selftest proof memmap-generator-missing: cmake/IvsMemmapProbes.cmake (observed)\n'
  cases=$((cases + 1))
  printf '%s\n' st-memmap-generator-missing >> "$ran"
else
  bad runner-selftest-missing "memmap-generator-missing"
fi

# The case inventory (EV-07, D-09-06). data/expected/runner-selftest.ids names the four mechanisms
# whose only proof is a case of this step, and the important-gates registry resolves a `mutant` row by
# those names. The direction that matters is the one below: a name in the inventory that this run did
# not pass is runner-selftest-missing, so a renamed or deleted case cannot leave a registry row
# pointing at nothing. The rule itself is tools/check/selftest-inventory.sh, and the case after this
# one proves that file can fail.
sh "$CHECK_DIR/selftest-inventory.sh" "$ROOT/data/expected/runner-selftest.ids" "$ran" || rc=1

# Selftest-inventory rule, stand-in (EV-07). The guard above is a file no control directory can reach
# - the control runner would have to run this step - so its failure is committed here as a case: a
# renamed copy of the real inventory must be refused with the renamed name, and the real inventory
# against a complete `ran` list must be accepted, which is what keeps the refusal from being a rule
# that refuses everything.
si_real="$ROOT/data/expected/runner-selftest.ids"
si_bad="$tmp/runner-selftest.renamed.ids"
si_ran="$tmp/ran.all"
printf '%s\n' st-toy-drift st-deviation-unused st-pass-line st-memmap-generator-missing > "$si_ran"
si_ok=1
if sed 's/^st-pass-line$/st-no-such-case/' "$si_real" > "$si_bad" \
  && grep -q -x -F -e 'st-no-such-case' "$si_bad"; then
  :
else
  si_ok=0
fi
if sh "$CHECK_DIR/selftest-inventory.sh" "$si_bad" "$si_ran" > "$tmp/si.bad" 2>&1; then
  si_ok=0
fi
grep -q -F -e '-- FAIL runner-selftest-missing: st-no-such-case' "$tmp/si.bad" || si_ok=0
sh "$CHECK_DIR/selftest-inventory.sh" "$si_real" "$si_ran" > "$tmp/si.good" 2>&1 || si_ok=0
if [ "$si_ok" -eq 1 ]; then
  # Printed with the same non-collected prefix as the proofs above.
  printf -- '-- runner-selftest proof runner-selftest-missing: st-no-such-case (observed: '
  grep -F -e '-- FAIL runner-selftest-missing: st-no-such-case' "$tmp/si.bad" | tr -d '\n'
  printf -- ')\n'
  cases=$((cases + 1))
else
  bad runner-selftest-missing "selftest-inventory"
fi

[ "$rc" -eq 0 ] || exit 1
printf -- '-- runner-selftest pass cases=%s\n' "$cases"
