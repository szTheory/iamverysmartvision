#!/bin/sh
# SPDX-License-Identifier: MIT
# Step input: the controller literal and its policy inventory (D-02, D-03; plan 05-01; tools/check/input.cmake).
# A pure checker (H6): it reads only the tree under test (FILELIST paths and data/expected/*.ids) and writes
# nothing. Plan 05-05 adds the comparison of the literal with the profile fields input.control.* (the generated
# table is compared with the literal by suite input, input.table.matches-literal).
# Tokens: input-literal-grammar, input-literal-name-set, input-pins-mismatch, input-wiki-mismatch,
#   input-byte-mismatch, input-tag, input-source-unknown, input-anchor, input-policy-inventory and, from
#   plan 05-05, input-profile-mask-mismatch, input-profile-field-missing, input-profile-field-shape
#   (rules in input.cmake).
# Stats: input-rows=<rows over all literal files> input-policies=<result classes> input-profile-fields=<fields>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
rc=0
out=$(cmake -DROOT="$ROOT" -DFILELIST="$FILELIST" -DCHECK_DIR="$CHECK_DIR" \
  -P "$CHECK_DIR/input.cmake" 2>&1) || rc=1
# The checker's own pass line is re-printed below, once, with the step's name.
printf '%s\n' "$out" | grep -v '^-- input pass '
stats=$(printf '%s\n' "$out" | sed -n 's/^-- input pass //p')
[ "$rc" -eq 0 ] || exit 1
[ -n "$stats" ] || exit 1
printf -- '-- input pass %s\n' "$stats"
