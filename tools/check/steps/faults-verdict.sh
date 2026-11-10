#!/bin/sh
# SPDX-License-Identifier: MIT
# Step faults-verdict: pure checker over the named-fault results the c-faults step wrote
# (D-18, A-09). Reads only $ROOT/build/gate/, $ROOT/data/expected/ and the FILELIST core
# sources; never builds and never runs a binary, so the control runner can drive it.
# Inventory data/expected/faults/cpu.ids: a `# suite=<suite>` header, then one line
# `<FAULT_ID> <must-fail> <allowed-prefixes> <must-pass>` per fault (prefixes |-separated).
# Per fault, build/gate/faults/<ID>/<suite>.txt must exist, carry the gate nonce and list
# exactly the ids of data/expected/tests/<suite>.ids; the must-fail vector fails, every failing
# vector starts with an allowed prefix and the must-pass vector passes. Sites: every
# `#ifdef IVS_FAULT_<ID>` / `#if defined(IVS_FAULT_<ID>)` in core/src/**/*.{c,h} must name a
# listed non-GEN_ id, and every listed non-GEN_ id must have a site.
# All findings are printed, sorted. Tokens: fault-not-detected, fault-wrong-reason,
#   fault-collateral, fault-missing, fault-list-mismatch, fault-stale, fault-unknown-vector,
#   fault-orphan-site, fault-missing-site.
# Stats: faults=<faults with no finding>/<listed faults>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/faults-verdict.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
find="$tmp/findings"
: > "$find"
bad() { printf '%s %s\n' "$1" "$2" >> "$find"; }

gate="$ROOT/build/gate"
ids="$ROOT/data/expected/faults/cpu.ids"
total=0
ok=0
: > "$tmp/fault-ids"

suite=
if [ -f "$ids" ]; then
  suite=$(sed -n 's/^#[[:space:]]*suite=\([a-z0-9][a-z0-9-]*\).*$/\1/p' "$ids" | head -n 1)
else
  bad fault-missing "data/expected/faults/cpu.ids"
fi
[ -f "$ids" ] && [ -z "$suite" ] && bad fault-list-mismatch "data/expected/faults/cpu.ids (no suite header)"

: > "$tmp/suite.ids"
if [ -n "$suite" ]; then
  if [ -f "$ROOT/data/expected/tests/$suite.ids" ]; then
    awk '/^#/ { next } NF { print $1 }' "$ROOT/data/expected/tests/$suite.ids" | sort -u > "$tmp/suite.ids"
  else
    bad fault-list-mismatch "data/expected/tests/$suite.ids (missing)"
  fi
fi

nonce=
[ -f "$gate/NONCE" ] && nonce=$(head -n 1 "$gate/NONCE")

if [ -n "$suite" ]; then
  awk '/^#/ { next } NF { print }' "$ids" > "$tmp/lines"
  while IFS= read -r l; do
    set -f
    # shellcheck disable=SC2086
    set -- $l
    set +f
    id=$1
    total=$((total + 1))
    before=$(grep -c . "$find")
    case $id in
      *[!A-Z0-9_]*) bad fault-missing-site "$id (bad id)"; continue ;;
    esac
    case $id in GEN_*) ;; *) printf '%s\n' "$id" >> "$tmp/fault-ids" ;; esac
    if [ "$#" -ne 4 ]; then
      bad fault-unknown-vector "$id (line needs 4 columns)"
      continue
    fi
    mf=$2
    prefixes=$3
    mp=$4
    grep -q -x -F -e "$mf" "$tmp/suite.ids" || bad fault-unknown-vector "$id $mf"
    grep -q -x -F -e "$mp" "$tmp/suite.ids" || bad fault-unknown-vector "$id $mp"
    f="$gate/faults/$id/$suite.txt"
    if [ ! -f "$f" ]; then
      bad fault-missing "$id"
      continue
    fi
    awk '
      /^# suite=[^ ]* nonce=/ { split($3, b, "="); print "NONCE " b[2]; next }
      /^#/ { next }
      /^ok [0-9][0-9]* [^ ]/ { print "OK " $3; next }
      /^not ok [0-9][0-9]* [^ ]/ { print "NOTOK " $4; next }
    ' "$f" > "$tmp/res"
    have_nonce=$(sed -n 's/^NONCE //p' "$tmp/res" | head -n 1)
    if [ -z "$nonce" ] || [ "$have_nonce" != "$nonce" ]; then bad fault-stale "$id"; fi
    sed -n 's/^\(NOT\)*OK //p' "$tmp/res" | sort -u > "$tmp/res.ids"
    if [ "$(sed -n 's/^\(NOT\)*OK //p' "$tmp/res" | grep -c .)" -ne "$(grep -c . "$tmp/res.ids")" ] \
      || ! cmp -s "$tmp/res.ids" "$tmp/suite.ids"; then
      bad fault-list-mismatch "$id"
    fi
    grep -q -x -F -e "OK $mf" "$tmp/res" && bad fault-not-detected "$id $mf"
    grep -q -x -F -e "NOTOK $mp" "$tmp/res" && bad fault-collateral "$id $mp"
    for v in $(sed -n 's/^NOTOK //p' "$tmp/res"); do
      matched=0
      old_ifs=$IFS
      IFS='|'
      set -f
      for p in $prefixes; do
        case $v in "$p"*) matched=1 ;; esac
      done
      set +f
      IFS=$old_ifs
      [ "$matched" -eq 1 ] || bad fault-wrong-reason "$id $v"
    done
    after=$(grep -c . "$find")
    [ "$before" -eq "$after" ] && ok=$((ok + 1))
  done < "$tmp/lines"
  [ "$total" -gt 0 ] || bad fault-missing "data/expected/faults/cpu.ids (no faults)"
fi

# Sites in the core sources against the non-GEN_ fault ids.
: > "$tmp/sites"
for p in $(grep -E '^core/src/.*[.](c|h)$' "$FILELIST" | sort -u); do
  [ -f "$ROOT/$p" ] || continue
  grep -E '^[[:space:]]*#[[:space:]]*if' "$ROOT/$p" \
    | grep -E 'IVS_FAULT_[A-Za-z0-9_]' \
    | sed -n 's/^.*IVS_FAULT_\([A-Za-z0-9_]*\).*$/\1/p' \
    | while IFS= read -r s; do printf '%s %s\n' "$s" "$p"; done >> "$tmp/sites"
done
sort -u "$tmp/fault-ids" -o "$tmp/fault-ids"
awk '{ print $1 }' "$tmp/sites" | sort -u > "$tmp/site-ids"
while IFS= read -r l; do
  s=${l%% *}
  grep -q -x -F -e "$s" "$tmp/fault-ids" || bad fault-orphan-site "IVS_FAULT_$s ${l#* }"
done < "$tmp/sites"
for s in $(comm -23 "$tmp/fault-ids" "$tmp/site-ids"); do bad fault-missing-site "$s"; done

if [ -s "$find" ]; then
  sort -u "$find" | while IFS= read -r l; do
    printf -- '-- FAIL %s: %s\n' "${l%% *}" "${l#* }"
  done
  exit 1
fi
printf -- '-- faults-verdict pass faults=%s/%s\n' "$ok" "$total"
