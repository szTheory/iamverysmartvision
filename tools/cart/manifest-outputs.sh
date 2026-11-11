#!/bin/sh
# SPDX-License-Identifier: MIT
# Manifest refresh helper for the IVSC fixtures (D-34, plan 03-07; RESEARCH Pitfall 9).
# Usage: sh tools/cart/manifest-outputs.sh                 print the outputs entries
#        sh tools/cart/manifest-outputs.sh --check <manifest>  exit 1 when they differ
# Reads data/expected/cart-fixture-files.ids and prints, in listed order, one outputs entry
# {"path": "data/cart/<stem>.json", "sha256": <cmake -E sha256sum>, "size_bytes": <wc -c>} per
# listed stem, comma-separated and indented for the "outputs" array of
# fixtures/FX-ivsc-vectors/manifest.json. --check compares those lines with the lines between
# `"outputs": [` and its closing `],` in the given manifest. A local aid only: the gate's
# manifests step is the enforcement (it re-hashes every output of every manifest).
# Rule for every plan: any edit of a data/cart/*.json file or of cart-fixture-files.ids reruns
# this helper and updates the manifest outputs in the same commit.
LC_ALL=C; export LC_ALL
here=$(cd "$(dirname "$0")" && pwd) || exit 1
ROOT=$(cd "$here/../.." && pwd) || exit 1
ids="$ROOT/data/expected/cart-fixture-files.ids"

entries() {
  [ -f "$ids" ] || { printf 'manifest-outputs: %s is missing\n' "$ids" >&2; return 1; }
  stems=$(awk '/^#/ { next } NF { print $1 }' "$ids") || return 1
  [ -n "$stems" ] || { printf 'manifest-outputs: no stem listed\n' >&2; return 1; }
  n=0
  for stem in $stems; do
    case $stem in
      *[!a-z0-9-]*) printf 'manifest-outputs: bad stem %s\n' "$stem" >&2; return 1 ;;
    esac
    rel="data/cart/$stem.json"
    [ -f "$ROOT/$rel" ] || { printf 'manifest-outputs: %s is missing\n' "$rel" >&2; return 1; }
    sum=$(cmake -E sha256sum "$ROOT/$rel") || return 1
    sum=${sum%% *}
    size=$(wc -c < "$ROOT/$rel" | tr -d ' ') || return 1
    if [ "$n" -gt 0 ]; then
      printf '    },\n'
    fi
    printf '    {\n      "path": "%s",\n      "sha256": "%s",\n      "size_bytes": %s\n' "$rel" "$sum" "$size"
    n=$((n + 1))
  done
  printf '    }\n'
}

case ${1-} in
  '')
    entries
    ;;
  --check)
    manifest=${2-}
    [ -n "$manifest" ] && [ -f "$manifest" ] || { printf 'manifest-outputs: --check needs a manifest file\n' >&2; exit 2; }
    tmp=$(mktemp -d "${TMPDIR:-/tmp}/manifest-outputs.XXXXXX") || exit 1
    trap 'rm -rf "$tmp"' EXIT
    trap 'exit 1' HUP INT TERM
    entries > "$tmp/want" || exit 1
    awk 'in_out && /^  \],?$/ { exit } in_out { print } /^  "outputs": \[$/ { in_out = 1 }' "$manifest" > "$tmp/have"
    if cmp -s "$tmp/want" "$tmp/have"; then
      printf 'manifest-outputs: %s outputs are current\n' "$manifest"
    else
      printf 'manifest-outputs: %s outputs differ; run sh tools/cart/manifest-outputs.sh and paste the entries\n' "$manifest" >&2
      diff "$tmp/have" "$tmp/want" >&2
      exit 1
    fi
    ;;
  *)
    printf 'usage: sh tools/cart/manifest-outputs.sh [--check <manifest>]\n' >&2
    exit 2
    ;;
esac
