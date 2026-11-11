#!/bin/sh
# SPDX-License-Identifier: MIT
# Step c-faults: build the named-fault executables (target ivs_faults, dev preset) and run each
# fault of every data/expected/faults/*.ids file (sorted; D-37) once, writing its TAP result to
# <results>/faults/<ID>/<suite>.txt with the gate nonce (D-18), where <suite> comes from the
# `# suite=<name>` header of the file that lists the id. A fault run is expected to exit
# non-zero; this step fails only when a binary is absent, no result file is written, a file has
# no suite header or an id is listed twice. The faults-verdict step judges the results.
# Results dir: IVS_GATE_DIR (set by tools/gate.sh), else $ROOT/build/gate; the nonce is read
# from <results>/NONCE (created with a direct-run value when absent).
# Tokens: c-faults-run: <ID or what>, c-faults-run: no suite in <file>,
#   c-faults-run: duplicate <ID>. Stats: fault-runs=<n> (summed over files).
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/c-faults.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }
cd "$ROOT" || exit 1

# Every faults file and its suite, checked before the build so a bad inventory stops early.
: > "$tmp/files"
for ids in "$ROOT"/data/expected/faults/*.ids; do
  [ -f "$ids" ] || continue
  rel=${ids#"$ROOT"/}
  suite=$(sed -n 's/^#[[:space:]]*suite=\([a-z0-9][a-z0-9-]*\).*$/\1/p' "$ids" | head -n 1)
  if [ -z "$suite" ]; then
    bad c-faults-run "no suite in $rel"
    continue
  fi
  printf '%s %s\n' "$suite" "$rel" >> "$tmp/files"
done
if [ ! -s "$tmp/files" ] && [ "$rc" -eq 0 ]; then
  bad c-faults-run "data/expected/faults/*.ids (none)"
fi
[ "$rc" -eq 0 ] || exit 1

gate_dir=${IVS_GATE_DIR:-$ROOT/build/gate}
mkdir -p "$gate_dir" || { bad c-faults-run "results dir"; exit 1; }
if [ ! -f "$gate_dir/NONCE" ]; then
  printf 'direct.%s\n' "$$" > "$gate_dir/NONCE" || { bad c-faults-run "NONCE"; exit 1; }
fi
nonce=$(cat "$gate_dir/NONCE")

if ! cmake --build --preset dev --target ivs_faults > "$tmp/build.log" 2>&1; then
  cat "$tmp/build.log"
  bad c-faults-run "build ivs_faults"
  exit 1
fi

out_root="$gate_dir/faults"
rm -rf "$out_root"
mkdir -p "$out_root" || { bad c-faults-run "faults dir"; exit 1; }
runs=0
: > "$tmp/seen"
while read -r suite rel; do
  for id in $(awk '/^#/ { next } NF { print $1 }' "$ROOT/$rel" | sort); do
    case $id in
      *[!A-Z0-9_]*) bad c-faults-run "$id (bad id)"; continue ;;
    esac
    if grep -q -x -F -e "$id" "$tmp/seen"; then
      bad c-faults-run "duplicate $id ($rel)"
      continue
    fi
    printf '%s\n' "$id" >> "$tmp/seen"
    exe="$ROOT/build/dev/tests/ivs_fault_$id"
    if [ ! -x "$exe" ]; then
      bad c-faults-run "$id (no binary)"
      continue
    fi
    mkdir -p "$out_root/$id" || { bad c-faults-run "$id (dir)"; continue; }
    IVS_TEST_OUT_DIR="$out_root/$id" IVS_TEST_NONCE="$nonce" "$exe" > "$tmp/$id.log" 2>&1 < /dev/null || :
    if [ -s "$out_root/$id/$suite.txt" ]; then
      runs=$((runs + 1))
    else
      cat "$tmp/$id.log"
      bad c-faults-run "$id (no $suite.txt)"
    fi
  done
done < "$tmp/files"

[ "$rc" -eq 0 ] || exit 1
printf -- '-- c-faults pass fault-runs=%s\n' "$runs"
