#!/bin/sh
# SPDX-License-Identifier: MIT
# The selftest-inventory rule (EV-07, D-09-06; the tools/check/pass-line.sh precedent, review WR-02).
# A `mutant` registry row is only honest if the case it names is a case the step actually runs, so
# tools/check/steps/runner-selftest.sh calls this file once per run with the list of case names it
# recorded as each case passed. Every non-comment name of the inventory must appear in that list.
# Usage: sh selftest-inventory.sh <inventory> <ran>
# Exit 0: every inventory name was run.
# Exit 1: the inventory is absent, holds no non-comment line, or names a case the step did not run.
# Exit 3: wrong usage.
# This rule lives in a file of its own rather than inlined in the step because a guard inside the step
# is reachable only by the step running itself, and a scratch copy reverted before the commit is not a
# stand-in GUARDRAILS-v0.3 2.4 accepts. The committed case `st-no-such-case` in
# tools/check/steps/runner-selftest.sh runs this file against a renamed inventory on every gate run.
LC_ALL=C; export LC_ALL
[ "$#" -eq 2 ] || exit 3
inv=$1
ran=$2
if [ ! -f "$inv" ]; then
  printf -- '-- FAIL runner-selftest-inventory: %s\n' "$inv"
  exit 1
fi
names=$(awk '/^#/ { next } NF { print $1 }' "$inv")
if [ -z "$names" ]; then
  printf -- '-- FAIL runner-selftest-inventory: no cases\n'
  exit 1
fi
bad=0
for name in $names; do
  if [ ! -f "$ran" ] || ! grep -q -x -F -e "$name" "$ran"; then
    printf -- '-- FAIL runner-selftest-missing: %s\n' "$name"
    bad=1
  fi
done
exit "$bad"
