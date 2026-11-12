#!/bin/sh
# SPDX-License-Identifier: MIT
# Step psg-vectors: the PSG rule inventory and the line-based PSG vector files (AUD-01, D-07, D-10, D-23; plan 05-01).
# Two checkers, both pure (H6): the record driver for data/psg/rules.json (kind psg-rule, schema/psg-rule.txt and
# kinds/psg-rule.cmake) and tools/check/psg-vectors.cmake for the vector files, their inventories and the rule coverage.
# Tokens: psg-rule-* (kinds/psg-rule.cmake), psg-vector-*, psg-rule-without-vector, psg-rule-unknown-default-unresolved
#   (psg-vectors.cmake), plus the generic psg-rule-* record tokens of records.cmake.
# Stats: psg-rules=<rules> psg-vectors=<vectors> psg-policies=<policy ids>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
rc=0
out=$(cmake -DROOT="$ROOT" -DKIND=psg-rule -P "$CHECK_DIR/records.cmake" 2>&1) || rc=1
printf '%s\n' "$out" | grep -v '^-- psg-rule pass '
rstats=$(printf '%s\n' "$out" | sed -n 's/^-- psg-rule pass records=[0-9][0-9]* //p')
out=$(cmake -DROOT="$ROOT" -DFILELIST="$FILELIST" -DCHECK_DIR="$CHECK_DIR" \
  -P "$CHECK_DIR/psg-vectors.cmake" 2>&1) || rc=1
printf '%s\n' "$out" | grep -v '^-- psgvec pass '
vstats=$(printf '%s\n' "$out" | sed -n 's/^-- psgvec pass //p')
[ "$rc" -eq 0 ] || exit 1
[ -n "$rstats" ] && [ -n "$vstats" ] || exit 1
printf -- '-- psg-vectors pass %s %s\n' "$rstats" "$vstats"
