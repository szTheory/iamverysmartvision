#!/bin/sh
# SPDX-License-Identifier: MIT
# The pass-line rule (A-02/O2, EV-03; review WR-02): a step that exits 0 must print its verdict
# `-- <step> pass ...` as the LAST non-empty line of its output. A step that prints its verdict and
# then keeps reporting has not finished when it claims to have passed, so anything after it is a
# refusal.
# Usage: sh pass-line.sh <step-name> <output-file>
# Exit 0: the last non-empty line starts `-- <step> pass`.
# Exit 1: the output holds no `-- <step> pass` line at all (the step printed no verdict).
# Exit 2: a pass line is present and a non-empty line follows it (the verdict is not the last one).
# Exit 3: wrong usage, or the output file is unreadable.
# This file exists so the rule can be enforced where every step is reachable: tools/gate.sh's replay
# calls it once per step, which covers all 38 including the five producers (c-faults, c-tests,
# ci-summary, pkg-install, runner-selftest) that no control directory names. run-controls.sh keeps
# its own inline copy of the rule inside good_one; that copy is the control-reachable form, and the
# st-gate-pass-line case of tools/check/steps/runner-selftest.sh is the committed stand-in that
# proves this file refuses both shapes rather than only one.
LC_ALL=C; export LC_ALL
[ "$#" -eq 2 ] || exit 3
[ -f "$2" ] || exit 3
last=$(awk 'NF { l = $0 } END { print l }' "$2")
case $last in
  "-- $1 pass"*) exit 0 ;;
esac
if grep -q -F -e "-- $1 pass" "$2"; then exit 2; fi
exit 1
