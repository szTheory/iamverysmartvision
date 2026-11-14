#!/bin/sh
# SPDX-License-Identifier: MIT
# Step video: the Phase 6 oracle files of the first STIC (VID-01..VID-04; D-01, D-02, D-12, D-13, D-18; plan 06-01).
# Two checkers, both pure (H6): the record driver for data/stic/rules.json (kind stic-rule, schema/stic-rule.txt and
# kinds/stic-rule.cmake) and tools/check/video.cmake for the page-reading verdict keys, the register-address literal, the
# cross-checks of rule fields and test families against the profile, the suites and the pending lists, the geometry, card
# and chain literals and the hand-derived frames with their conservation and drift keys.
# Tokens: stic-rule-* (kinds/stic-rule.cmake and the generic record tokens of records.cmake), video-* (video.cmake).
# Stats: stic-rules=<rules> stic-rows=<register rows> video-frames=<frames> video-hash-ties=<hash asserts tied to a frame literal> video-frame-pairs=<declared frame pairs>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
rc=0
out=$(cmake -DROOT="$ROOT" -DKIND=stic-rule -P "$CHECK_DIR/records.cmake" 2>&1) || rc=1
printf '%s\n' "$out" | grep -v '^-- stic-rule pass '
rstats=$(printf '%s\n' "$out" | sed -n 's/^-- stic-rule pass records=[0-9][0-9]* //p')
out=$(cmake -DROOT="$ROOT" -DFILELIST="$FILELIST" -DCHECK_DIR="$CHECK_DIR" \
  -P "$CHECK_DIR/video.cmake" 2>&1) || rc=1
printf '%s\n' "$out" | grep -v '^-- videochk pass '
vstats=$(printf '%s\n' "$out" | sed -n 's/^-- videochk pass //p')
[ "$rc" -eq 0 ] || exit 1
[ -n "$rstats" ] && [ -n "$vstats" ] || exit 1
printf -- '-- video pass %s %s\n' "$rstats" "$vstats"
