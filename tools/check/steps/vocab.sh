#!/bin/sh
# SPDX-License-Identifier: MIT
# Step vocab: source-authority tags and evidence tiers (kinds tags, evidence-tiers).
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
rc=0
tags=0
tiers=0
for kind in tags evidence-tiers; do
  out=$(cmake -DROOT="$ROOT" -DKIND="$kind" -P "$CHECK_DIR/records.cmake" 2>&1) || rc=1
  printf '%s\n' "$out"
  n=$(printf '%s\n' "$out" | sed -n "s/^-- $kind pass records=\([0-9][0-9]*\).*/\1/p")
  [ -n "$n" ] || rc=1
  if [ "$kind" = tags ]; then tags=$n; else tiers=$n; fi
done
[ "$rc" -eq 0 ] || exit 1
printf -- '-- vocab pass tags=%s tiers=%s\n' "$tags" "$tiers"
