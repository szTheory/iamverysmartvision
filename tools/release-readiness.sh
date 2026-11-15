#!/bin/sh
# SPDX-License-Identifier: MIT
# Release-readiness generator (plan 08-02; D-27, D-30, D-33): the computable sections of the milestone
# close record, printed as markdown. Read-only: it writes no file outside one mktemp directory and runs
# no git write. Not a gate step (only its selftest is run by the release-assets step).
#
#   sh tools/release-readiness.sh [--allow-dirty] [--history] [--release]
#   sh tools/release-readiness.sh --selftest
#   sh tools/release-readiness.sh --hosted        (ORCHESTRATOR ONLY, see below)
#
# Local mode prints, in this order:
#   A Identity  the commit, its date, and the tree id of each of core runner tools data docs fixtures tests
#   F Claims    the claim table of data/claims.json (tools/check/claims-table.cmake), counts by tier and
#               standing; refused (exit 1) when any claim stands at tier measured-hardware, because no
#               hardware measurement was performed (D10)
#   G Rights    the pinned install list, the scan of tracked file names against tools/check/binary-ext.txt
#               (expected none), the fixture census, and the name check (no tracked path holds the word
#               Intellivision in any case)
#   J Hygiene   sh tools/hygiene.sh --tree; with --history also sh tools/hygiene.sh --history (it takes
#               minutes and belongs to the release, never to an executor), whose findings are compared by
#               (token, short sha) pair with data/expected/hygiene-history-pinned.ids: an accepted pair is
#               printed as FAIL with its recorded disposition (D-33), a pair not on the list as NEW FINDING
#               (exit 1). A history run that had findings never prints a verdict of success. The history
#               baseline file is never touched and no rule is widened.
#   P4          with --release: row P4 of the release checklist, which requires a section for the version
#               of the header macros in CHANGELOG.md ("P4: ok", or "P4: FAIL release-changelog-section-missing"
#               and exit 1); without --release the row reads "P4: not checked".
# The working tree must be clean (tracked files equal HEAD) unless --allow-dirty is given: exit 2 otherwise.
# --selftest runs four cases on synthetic inputs (a measured-hardware claim, a new history finding, the
# pinned history findings, a dirty tree) and prints "selftest <ok>/4" as its last line.
# --hosted is the only mode that calls the GitHub CLI (read-only list and view calls). It is for the
# orchestrator alone, because the API rate limit is shared by every agent; no executor passes it and no
# selftest or gate step calls it.
# IVS_READINESS_ROOT (test hook) replaces the repository root, which is otherwise the parent of this
# script's directory.
# Exit: 0 ok, 1 a check failed or a new finding, 2 usage error, refusal (dirty tree) or a missing tool.
LC_ALL=C; export LC_ALL

usage() {
  sed -n '3,36p' "$0" | sed 's/^# \{0,1\}//'
}
refuse() { printf 'release-readiness: refused: %s\n' "$1" >&2; exit 2; }

self=$0
case $self in /*) ;; *) self=$(pwd)/$self ;; esac

# history_compare <findings-file> <pinned-file>: the output lines of hygiene --history have the form
# "-- FAIL hygiene-<kind>: <detail>"; each is paired as "<token> <detail>" (the detail is the short sha for
# a commit finding) and compared with the pinned pairs, never by token alone (T-08-06). Prints one line per
# pair, then a count line; status 1 when any pair is not pinned.
history_compare() {
  awk -v pin="$2" '
    BEGIN {
      while ((getline l < pin) > 0) {
        sub(/^[ \t]+/, "", l)
        if (l == "" || substr(l, 1, 1) == "#") continue
        if (split(l, w, " ") == 2) {
          key = w[1] " " w[2]
          if (!(key in pinned)) { pinned[key] = 1; pord[++np] = key }
        }
      }
      close(pin)
    }
    /^-- FAIL hygiene-/ {
      line = $0
      sub(/^-- FAIL /, "", line)
      i = index(line, ": ")
      if (i == 0) next
      key = substr(line, 1, i - 1) " " substr(line, i + 2)
      if (key in seen) next
      seen[key] = 1
      order[++k] = key
    }
    END {
      acc = 0; bad = 0
      for (j = 1; j <= k; j++) {
        if (order[j] in pinned) { printf "FAIL (accepted by recorded disposition, D-33): %s\n", order[j]; acc++ }
        else { printf "NEW FINDING: %s\n", order[j]; bad++ }
      }
      for (j = 1; j <= np; j++) if (!(pord[j] in seen)) printf "NOTE: pinned pair not found in this run: %s\n", pord[j]
      printf "history findings: %d (accepted %d, new %d)\n", k, acc, bad
      exit (bad > 0 ? 1 : 0)
    }' "$1"
}

# stgit <git args>: git on the throwaway repository of the selftest with a fixed identity.
stgit() {
  git -C "$st_repo" -c user.name=ctl -c user.email=noreply@github.com -c commit.gpgsign=false "$@"
}

# selftest: four cases on synthetic inputs; every temporary path is under one mktemp directory.
selftest() {
  st_root=${IVS_READINESS_ROOT:-$(cd "$(dirname "$self")/.." && pwd)} || exit 2
  st_tmp=$(mktemp -d "${TMPDIR:-/tmp}/release-readiness.XXXXXX") || exit 2
  trap 'rm -rf "$st_tmp"' EXIT
  trap 'exit 2' HUP INT TERM
  st_ok=0
  st_pin="$st_root/data/expected/hygiene-history-pinned.ids"

  # claim-measured-hardware: the table generator refuses a claim at tier measured-hardware.
  st_name=claim-measured-hardware
  printf '{"claims":[{"id":"CL-0001","tier":"measured-hardware","standing":"provisional","artifact":"data/x"}]}\n' \
    > "$st_tmp/claims.json"
  st_s=0
  cmake -DROOT="$st_root" -DCLAIMS="$st_tmp/claims.json" -P "$st_root/tools/check/claims-table.cmake" \
    > "$st_tmp/c1.out" 2> "$st_tmp/c1.err" || st_s=$?
  if [ "$st_s" -ne 0 ] && grep -q -x -F 'release-claims-measured-hardware: CL-0001' "$st_tmp/c1.out"; then
    st_ok=$((st_ok + 1)); printf 'selftest %s ok\n' "$st_name"
  else
    printf 'selftest %s FAIL: exit %s, no refusal line\n' "$st_name" "$st_s"
  fi

  # The pinned pairs as hygiene --history would print them.
  awk '/^#/ { next } NF == 2 { printf "-- FAIL %s: %s\n", $1, $2 }' "$st_pin" > "$st_tmp/pinned.find" 2> /dev/null

  # history-new-finding: the pinned pairs plus one more must fail with NEW FINDING.
  st_name=history-new-finding
  { cat "$st_tmp/pinned.find"; printf -- '-- FAIL hygiene-history-home-path: 0123456789\n'; } > "$st_tmp/new.find"
  st_s=0
  history_compare "$st_tmp/new.find" "$st_pin" > "$st_tmp/c2.out" 2>&1 || st_s=$?
  if [ -s "$st_tmp/pinned.find" ] && [ "$st_s" -eq 1 ] \
    && grep -q -x -F 'NEW FINDING: hygiene-history-home-path 0123456789' "$st_tmp/c2.out"; then
    st_ok=$((st_ok + 1)); printf 'selftest %s ok\n' "$st_name"
  else
    printf 'selftest %s FAIL: exit %s, no NEW FINDING line\n' "$st_name" "$st_s"
  fi

  # history-pinned-ok: exactly the pinned pairs exit 0, carry the disposition wording and no success verdict.
  st_name=history-pinned-ok
  st_s=0
  history_compare "$st_tmp/pinned.find" "$st_pin" > "$st_tmp/c3.out" 2>&1 || st_s=$?
  st_n=$(grep -c '^FAIL (accepted by recorded disposition, D-33): ' "$st_tmp/c3.out")
  st_want=$(grep -c . "$st_tmp/pinned.find")
  if [ -s "$st_tmp/pinned.find" ] && [ "$st_s" -eq 0 ] && [ "$st_n" -eq "$st_want" ] \
    && ! grep -q -i -e 'NEW FINDING' -e 'pass' "$st_tmp/c3.out"; then
    st_ok=$((st_ok + 1)); printf 'selftest %s ok\n' "$st_name"
  else
    printf 'selftest %s FAIL: exit %s, %s of %s lines carry the disposition\n' "$st_name" "$st_s" "$st_n" "$st_want"
  fi

  # dirty-tree-refused: in a throwaway repository with one tracked file modified, local mode exits 2.
  # No user, system or template configuration is read: every git call passes the identity and signing
  # options with -c, the environment names no global file and the repository has no hook template.
  st_name=dirty-tree-refused
  st_repo="$st_tmp/repo"
  GIT_CONFIG_GLOBAL=/dev/null
  GIT_CONFIG_NOSYSTEM=1
  export GIT_CONFIG_GLOBAL GIT_CONFIG_NOSYSTEM
  unset GIT_DIR GIT_WORK_TREE GIT_INDEX_FILE
  st_s=1
  if mkdir "$st_repo" && stgit init -q --template= \
    && printf 'one\n' > "$st_repo/a" && stgit add a \
    && stgit commit -q -m seed > /dev/null 2>&1 \
    && printf 'two\n' > "$st_repo/a"; then
    st_s=0
    IVS_READINESS_ROOT="$st_repo" sh "$self" > "$st_tmp/c4.out" 2> "$st_tmp/c4.err" < /dev/null || st_s=$?
  fi
  if [ "$st_s" -eq 2 ] && ! grep -q '^## ' "$st_tmp/c4.out"; then
    st_ok=$((st_ok + 1)); printf 'selftest %s ok\n' "$st_name"
  else
    printf 'selftest %s FAIL: exit %s\n' "$st_name" "$st_s"
  fi

  printf 'selftest %s/4\n' "$st_ok"
  [ "$st_ok" -eq 4 ]
}

# hosted: read-only GitHub calls for sections B and C of the close record. ORCHESTRATOR ONLY.
hosted() {
  command -v gh > /dev/null 2>&1 || refuse "the GitHub CLI is required for --hosted (orchestrator only)"
  printf '# Release readiness, hosted inputs (read-only, orchestrator only)\n'
  printf '\n## B Runs on main (id, head sha, event, conclusion, created)\n\n'
  gh run list --branch main --limit 5 --json databaseId,headSha,event,conclusion,createdAt \
    --jq '.[] | [.databaseId, .headSha, .event, .conclusion, .createdAt] | @tsv' || rc=1
  printf '\n## C Open pull requests (number, title)\n\n'
  gh pr list --state open --json number,title --jq '.[] | [.number, .title] | @tsv' || rc=1
  printf '\n## C Open issues (number, title)\n\n'
  gh issue list --state open --json number,title --jq '.[] | [.number, .title] | @tsv' || rc=1
  printf '\n## C Rulesets (id, name, target, enforcement)\n\n'
  gh api 'repos/{owner}/{repo}/rulesets' --jq '.[] | [.id, .name, .target, .enforcement] | @tsv' || rc=1
  printf '\n## C Releases\n\n'
  gh release list --limit 10 || rc=1
}

allow_dirty=0
history=0
release=0
mode=local
while [ "$#" -gt 0 ]; do
  case $1 in
    --allow-dirty) allow_dirty=1 ;;
    --history) history=1 ;;
    --release) release=1 ;;
    --selftest) mode=selftest ;;
    --hosted) mode=hosted ;;
    -h | --help) usage; exit 0 ;;
    *) usage >&2; exit 2 ;;
  esac
  shift
done
if [ "$mode" != local ] && [ "$allow_dirty$history$release" != 000 ]; then usage >&2; exit 2; fi

rc=0
if [ "$mode" = selftest ]; then
  selftest
  exit $?
fi
if [ "$mode" = hosted ]; then
  hosted
  exit $rc
fi

# ---- local mode ----
unset FILELIST
for tool in git awk cmake sed grep sort; do
  command -v "$tool" > /dev/null 2>&1 || refuse "$tool not found"
done
root=${IVS_READINESS_ROOT:-$(cd "$(dirname "$self")/.." && pwd)} || exit 2
cd "$root" || exit 2
git rev-parse --is-inside-work-tree > /dev/null 2>&1 || refuse "$root is not a git work tree"
dirty=
if [ -n "$(git status --porcelain --untracked-files=no 2> /dev/null)" ]; then dirty=1; fi
if [ -n "$dirty" ] && [ "$allow_dirty" -eq 0 ]; then
  refuse "tracked files differ from HEAD (commit them, or pass --allow-dirty for a local look)"
fi
tmp=$(mktemp -d "${TMPDIR:-/tmp}/release-readiness.XXXXXX") || exit 2
trap 'rm -rf "$tmp"' EXIT
trap 'exit 2' HUP INT TERM

printf '# Release readiness (generated, read-only)\n'

# ---- A Identity ----
printf '\n## A Identity\n\n'
head_sha=$(git rev-parse HEAD) || exit 2
printf -- '- commit: %s\n' "$head_sha"
printf -- '- commit date (UTC): %s\n' \
  "$(TZ=UTC git log -1 --format=%cd --date=format-local:%Y-%m-%dT%H:%M:%SZ HEAD)"
if [ -n "$dirty" ]; then
  printf -- '- working tree: dirty (tracked files differ from HEAD; --allow-dirty); the ids below are those of HEAD\n'
else
  printf -- '- working tree: clean\n'
fi
for d in core runner tools data docs fixtures tests; do
  if id=$(git rev-parse --verify "HEAD:$d" 2> /dev/null); then
    printf -- '- tree %s: %s\n' "$d" "$id"
  else
    printf -- '- tree %s: ABSENT\n' "$d"
    rc=1
  fi
done

# ---- F Claims ----
printf '\n## F Claims\n\n'
if cmake -DROOT="$root" -P "$root/tools/check/claims-table.cmake" > "$tmp/claims.out" 2> "$tmp/claims.err" < /dev/null; then
  cat "$tmp/claims.out"
  printf '\nBy tier:'
  awk -F' [|] ' '/^[|] CL-/ { c[$2]++; if (!($2 in s)) { s[$2] = 1; o[++n] = $2 } }
    END { for (i = 1; i <= n; i++) printf " %s=%d", o[i], c[o[i]]; printf "\n" }' "$tmp/claims.out"
else
  rc=1
  printf 'FAIL: the claim table was refused\n\n'
  sed -n '1,20p' "$tmp/claims.out"
fi

# ---- G Rights and package audit ----
printf '\n## G Rights and package audit\n\n'
pin=data/expected/pkg-install-files.ids
if [ -f "$pin" ]; then
  grep -v -e '^#' -e '^[[:space:]]*$' "$pin" > "$tmp/install.list"
  printf -- '- pinned install list (%s): %s files\n' "$pin" "$(grep -c . "$tmp/install.list")"
  sed 's/^/  - /' "$tmp/install.list"
else
  printf -- '- pinned install list: MISSING (%s)\n' "$pin"
  rc=1
fi
git ls-files > "$tmp/tracked.list" || exit 2
extfile=tools/check/binary-ext.txt
if [ -f "$extfile" ]; then
  awk -v extfile="$extfile" '
    BEGIN {
      while ((getline l < extfile) > 0) {
        sub(/^[ \t]+/, "", l)
        if (l == "" || substr(l, 1, 1) == "#") continue
        sub(/[ \t].*$/, "", l)
        bin[tolower(l)] = 1
      }
      close(extfile)
    }
    {
      base = $0
      sub(/^.*\//, "", base)
      k = split(base, part, ".")
      if (k >= 2 && (tolower(part[k]) in bin)) print
    }' "$tmp/tracked.list" > "$tmp/binhits"
  nb=$(grep -c . "$tmp/binhits")
  printf -- '- tracked binaries by name (git ls-files against %s): %s\n' "$extfile" "$nb"
  if [ "$nb" -ne 0 ]; then
    sed -n '1,20p' "$tmp/binhits" | sed 's/^/  - FAIL /'
    rc=1
  fi
else
  printf -- '- tracked binaries by name: MISSING (%s)\n' "$extfile"
  rc=1
fi
fx=data/expected/fixtures.ids
if [ -f "$fx" ]; then
  grep -v -e '^#' -e '^[[:space:]]*$' "$fx" | awk '{ print $1 }' > "$tmp/fixtures.list"
  fy=0
  fn=0
  fu=0
  fm=0
  while IFS= read -r f; do
    m=fixtures/$f/manifest.json
    if [ ! -f "$m" ]; then fm=$((fm + 1)); continue; fi
    if grep -q '"redistributable": "yes"' "$m"; then fy=$((fy + 1))
    elif grep -q '"redistributable": "no"' "$m"; then fn=$((fn + 1))
    else fu=$((fu + 1)); fi
  done < "$tmp/fixtures.list"
  printf -- '- fixture census (%s): %s manifests, redistributable yes %s, no %s, unknown or other %s, missing %s\n' \
    "$fx" "$(grep -c . "$tmp/fixtures.list")" "$fy" "$fn" "$fu" "$fm"
  if [ "$fm" -ne 0 ]; then rc=1; fi
else
  printf -- '- fixture census: MISSING (%s)\n' "$fx"
  rc=1
fi
grep -i 'intellivision' "$tmp/tracked.list" > "$tmp/names.hit"
nn=$(grep -c . "$tmp/names.hit")
printf -- '- name check (tracked paths holding the word Intellivision in any case): %s\n' "$nn"
if [ "$nn" -ne 0 ]; then
  sed -n '1,20p' "$tmp/names.hit" | sed 's/^/  - FAIL /'
  rc=1
fi

# ---- J Hygiene ----
printf '\n## J Hygiene\n\n'
ts=0
ROOT="$root" sh "$root/tools/hygiene.sh" --tree > "$tmp/tree.out" 2> "$tmp/tree.err" < /dev/null || ts=$?
if [ "$ts" -eq 0 ]; then
  printf -- '- tree scan (sh tools/hygiene.sh --tree), exit 0: %s\n' "$(tail -n 1 "$tmp/tree.out")"
else
  printf -- '- tree scan (sh tools/hygiene.sh --tree), exit %s:\n' "$ts"
  sed -n '1,20p' "$tmp/tree.out" | sed 's/^/  - /'
  rc=1
fi
if [ "$history" -eq 1 ]; then
  hs=0
  (cd "$root" && sh tools/hygiene.sh --history) > "$tmp/hist.out" 2> "$tmp/hist.err" < /dev/null || hs=$?
  grep '^-- FAIL hygiene-' "$tmp/hist.out" > "$tmp/hist.find"
  if [ "$hs" -ge 2 ] || { [ "$hs" -ne 0 ] && [ ! -s "$tmp/hist.find" ]; }; then
    printf -- '- history scan (sh tools/hygiene.sh --history): did not complete, exit %s\n' "$hs"
    rc=1
  else
    printf -- '- history scan (sh tools/hygiene.sh --history), exit %s; findings compared by (token, short sha) with data/expected/hygiene-history-pinned.ids:\n' "$hs"
    hc=0
    history_compare "$tmp/hist.find" data/expected/hygiene-history-pinned.ids > "$tmp/hist.cmp" || hc=$?
    sed 's/^/  - /' "$tmp/hist.cmp"
    [ "$hc" -eq 0 ] || rc=1
    if [ -s "$tmp/hist.find" ]; then
      printf -- '- disposition (D-33): accepted without a rewrite as a delegated decision; the baseline file is not moved and no rule is widened; a new finding fails this check; the owner may veto, a rewrite being an owner-only stop\n'
    fi
  fi
else
  printf -- '- history scan: not run (a release step: pass --history; it takes minutes)\n'
fi

# ---- P4 ----
printf '\n## P4\n\n'
if [ "$release" -eq 1 ]; then
  hdr=core/include/ivs/ivs.h
  vmaj=$(sed -n 's/^#define IVS_API_VERSION_MAJOR \([0-9][0-9]*\)[uU]\{0,1\}$/\1/p' "$hdr" 2> /dev/null)
  vmin=$(sed -n 's/^#define IVS_API_VERSION_MINOR \([0-9][0-9]*\)[uU]\{0,1\}$/\1/p' "$hdr" 2> /dev/null)
  vpat=$(sed -n 's/^#define IVS_API_VERSION_PATCH \([0-9][0-9]*\)[uU]\{0,1\}$/\1/p' "$hdr" 2> /dev/null)
  if [ -n "$vmaj" ] && [ -n "$vmin" ] && [ -n "$vpat" ] \
    && sh tools/check/release-assets.sh changelog "$vmaj.$vmin.$vpat" CHANGELOG.md > "$tmp/p4.out" 2>&1; then
    printf 'P4: ok\n'
  else
    printf 'P4: FAIL release-changelog-section-missing\n'
    if [ -s "$tmp/p4.out" ]; then sed -n '1,3p' "$tmp/p4.out"; fi
    rc=1
  fi
else
  printf 'P4: not checked\n'
fi

exit $rc
