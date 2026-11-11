#!/bin/sh
# SPDX-License-Identifier: MIT
# Step api-effects: the API effect table data/api/api-effects.json against the status header,
# the status pin, the IVS_API declarations of core/include/ivs/*.h (D-06 symbol list equality)
# and the test inventories (D-09, D-15, API-06; tools/check/api-effects.cmake), plus the generic
# define pins of the public headers (stop reasons, refusal reasons; plan 04-08, D-04, D-05).
# Static: it reads only ROOT and writes nothing.
# Tokens: api-status-renumbered, api-status-unpinned, api-status-reserved, api-call-undocumented,
#   api-call-undeclared, api-refusal-no-test, api-effect-unregistered, api-dead-status,
#   api-effects-format, api-define-unpinned, api-define-renumbered, api-define-orphan,
#   api-define-duplicate (rules in api-effects.cmake).
# Stats: api-calls=<calls> api-statuses=<pinned statuses> api-rows=<refusal rows>
#   pinned-defines=<pin lines over every define class>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
rc=0
out=$(cmake -DROOT="$ROOT" -DCHECK_DIR="$CHECK_DIR" -P "$CHECK_DIR/api-effects.cmake" 2>&1) || rc=1
# The checker's own pass line is re-printed below, once.
printf '%s\n' "$out" | grep -v '^-- api-effects pass '
stats=$(printf '%s\n' "$out" | sed -n 's/^-- api-effects pass //p')
[ "$rc" -eq 0 ] || exit 1
[ -n "$stats" ] || exit 1
printf -- '-- api-effects pass %s\n' "$stats"
