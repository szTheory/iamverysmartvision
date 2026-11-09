#!/bin/sh
# SPDX-License-Identifier: MIT
# Step ci-parity: the CI workflow runs exactly the local gate, SHA-pinned and read-only (D-23).
# Reads ROOT, CHECK_DIR and FILELIST only. Every check runs before the step exits.
# Tokens: ci-parity-missing-workflow, ci-parity-missing-gate, ci-parity-extra-run: <line>,
#   ci-unpinned-action: <line>, ci-permissions.
# Stats: ci-uses=<n> (count of pinned uses: lines).
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }

wf="$ROOT/.github/workflows/gate.yml"
if [ ! -f "$wf" ]; then
  bad ci-parity-missing-workflow ".github/workflows/gate.yml"
  exit 1
fi

gate=0
uses=0
n=0
while IFS= read -r line || [ -n "$line" ]; do
  n=$((n + 1))
  # Strip an optional list dash so `- run:` and `run:` are handled alike.
  body=$(printf '%s\n' "$line" | sed 's/^[[:space:]]*-[[:space:]][[:space:]]*//; s/^[[:space:]]*//; s/[[:space:]]*$//')
  case $body in
    'run: sh tools/gate.sh') gate=$((gate + 1)) ;;
    'run: cmake --version') : ;;
    run:*) bad ci-parity-extra-run "line $n" ;;
    uses:*)
      case $body in
        *@*) ref=$(printf '%s\n' "$body" | sed 's/^[^@]*@//; s/[[:space:]].*$//') ;;
        *) ref= ;;
      esac
      if [ "${#ref}" -eq 40 ] && printf '%s\n' "$ref" | grep -E '^[0-9a-f]+$' > /dev/null; then
        uses=$((uses + 1))
      else
        bad ci-unpinned-action "line $n"
      fi
      ;;
  esac
done < "$wf"

if [ "$gate" -eq 0 ]; then bad ci-parity-missing-gate "run: sh tools/gate.sh"; fi
if ! grep -E '^[[:space:]]+contents:[[:space:]]*read[[:space:]]*$' "$wf" > /dev/null; then
  bad ci-permissions "contents: read"
fi

[ "$rc" -eq 0 ] || exit 1
printf -- '-- ci-parity pass ci-uses=%s\n' "$uses"
