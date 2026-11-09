#!/bin/sh
# SPDX-License-Identifier: MIT
# Step ci-parity: CI runs exactly the local gate, SHA-pinned and read-only (D-23).
# Reads ROOT, CHECK_DIR and FILELIST only. Every check runs before the step exits.
# Inspected files: every FILELIST path under .github/workflows/, not only gate.yml.
# Rules, line based (no YAML parser, no dependency); every line is read with comments and
# surrounding spaces removed, and keys are matched with or without spaces before the colon:
#   - gate.yml must exist and contain the step `run: sh tools/gate.sh`; no `run:` anywhere may
#     be anything but that step or `run: cmake --version`
#   - every `uses:` is pinned to a 40-hex commit SHA
#   - no `continue-on-error`, no `if:`, no `pull_request_target`
#   - a top-level `permissions:` block holding only `contents: read` exists in every workflow
#     and any other `permissions` key (job level) is a block holding only `contents: read`
#   - no flow-style `{` mapping outside `${{ ... }}` expressions (it would hide keys from the scan)
# Limits: anchors, multi-document YAML and block-scalar bodies are not interpreted; a line inside
# a `run: |` body that looks like a key is reported (fail closed).
# Tokens: ci-parity-missing-workflow, ci-parity-missing-gate, ci-parity-extra-run: <file>:<line>,
#   ci-unpinned-action: <file>:<line>, ci-permissions: <file>[:<line>],
#   ci-parity-continue-on-error, ci-parity-conditional, ci-parity-pull-request-target,
#   ci-parity-flow-style (each <file>:<line>).
# Stats: ci-uses=<n> (count of pinned uses: lines), ci-workflows=<n>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/ciparity.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }

grep -E '^[.]github/workflows/[^/]+$' "$FILELIST" | sort -u > "$tmp/wf"
if ! grep -q -x -F '.github/workflows/gate.yml' "$tmp/wf" || [ ! -f "$ROOT/.github/workflows/gate.yml" ]; then
  bad ci-parity-missing-workflow ".github/workflows/gate.yml"
fi

uses=0
nwf=0
while IFS= read -r p; do
  [ -n "$p" ] || continue
  [ -f "$ROOT/$p" ] || continue
  nwf=$((nwf + 1))
  # awk prints "<token> <detail>" lines and a final "USES <n> GATE <n> TOPPERM <0|1>" line.
  awk -v file="$p" '
    function indent_of(s,   m) { match(s, /^ */); return RLENGTH }
    function emit(tok, n) { printf "%s %s:%d\n", tok, file, n }
    BEGIN { sq = sprintf("%c", 39); uses = 0; gate = 0; topperm = 0; inperm = 0; permind = -1; permtop = 0; permok = 1; permseen = 0 }
    {
      raw = $0
      ind = indent_of(raw)
      line = raw
      sub(/^[ \t]*#.*$/, "", line)
      sub(/[ \t]+#.*$/, "", line)
      sub(/[ \t]+$/, "", line)
      if (line ~ /^[ \t]*$/) next
      # Leave a permissions block when the indent drops back to the key line or less.
      if (inperm && ind <= permind) {
        if (permtop && permok && permseen) topperm = 1
        inperm = 0
      }
      body = line
      sub(/^[ \t]*-[ \t]+/, "", body)
      sub(/^[ \t]+/, "", body)
      tmpline = body
      gsub(/\$\{\{[^}]*\}\}/, "", tmpline)
      if (tmpline ~ /[{]/) emit("ci-parity-flow-style", NR)
      key = body
      val = ""
      p = index(body, ":")
      if (p > 0) {
        key = substr(body, 1, p - 1)
        val = substr(body, p + 1)
        sub(/^[ \t]+/, "", val)
      }
      gsub(/["\t ]/, "", key)
      gsub(sq, "", key)
      if (body ~ /pull_request_target/) emit("ci-parity-pull-request-target", NR)
      if (inperm) {
        if (key == "contents" && val == "read") { permseen = 1 }
        else { permok = 0; emit("ci-permissions", NR) }
        next
      }
      if (key == "continue-on-error") emit("ci-parity-continue-on-error", NR)
      else if (key == "if") emit("ci-parity-conditional", NR)
      else if (key == "permissions") {
        if (val == "") { inperm = 1; permind = ind; permtop = (ind == 0); permok = 1; permseen = 0 }
        else emit("ci-permissions", NR)
      }
      else if (key == "run") {
        if (val == "sh tools/gate.sh") gate++
        else if (val == "cmake --version") { }
        else emit("ci-parity-extra-run", NR)
      }
      else if (key == "uses") {
        ref = ""
        q = index(val, "@")
        if (q > 0) { ref = substr(val, q + 1); sub(/[ \t].*$/, "", ref) }
        if (length(ref) == 40 && ref ~ /^[0-9a-f]+$/) uses++
        else emit("ci-unpinned-action", NR)
      }
    }
    END {
      if (inperm && permtop && permok && permseen) topperm = 1
      printf "USES %d GATE %d TOPPERM %d\n", uses, gate, topperm
    }
  ' "$ROOT/$p" > "$tmp/awk.out"
  while IFS= read -r l; do
    case $l in
      'USES '*)
        u=$(printf '%s\n' "$l" | awk '{ print $2 }')
        g=$(printf '%s\n' "$l" | awk '{ print $4 }')
        t=$(printf '%s\n' "$l" | awk '{ print $6 }')
        uses=$((uses + u))
        if [ "$p" = ".github/workflows/gate.yml" ] && [ "$g" -eq 0 ]; then
          bad ci-parity-missing-gate "run: sh tools/gate.sh"
        fi
        if [ "$t" -eq 0 ]; then bad ci-permissions "$p top-level permissions contents: read"; fi
        ;;
      *) bad "${l%% *}" "${l#* }" ;;
    esac
  done < "$tmp/awk.out"
done < "$tmp/wf"

[ "$rc" -eq 0 ] || exit 1
printf -- '-- ci-parity pass ci-uses=%s ci-workflows=%s\n' "$uses" "$nwf"
