#!/bin/sh
# SPDX-License-Identifier: MIT
# Step hygiene-history: runs the --history mode of tools/hygiene.sh over a throwaway one-commit
# repository holding the FILELIST files (WR-06). --history walks real history and is a publication
# step, not a gate step; this step keeps its git code path alive in the gate, and lets a control
# prove that a leak committed to history, or an unreadable object, fails it.
# The repository lives in a temp directory (git dir outside ROOT, ROOT as work tree), so the real
# repository, its index and its config are never touched.
# Test hook: a FILELIST file named selftest-corrupt-blob whose first line is a repo-relative path
# makes the step delete that path's loose object after committing (the unreadable-object control).
# Reads ROOT, CHECK_DIR and FILELIST only.
# Tokens: hygiene-history-setup: <what>, plus every token of hygiene.sh --history.
# Stats: history-commits=<n>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/hygiene-history.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; }

# A clean git environment: no user or system config, no hooks, no template files.
GIT_CONFIG_GLOBAL=/dev/null
GIT_CONFIG_NOSYSTEM=1
export GIT_CONFIG_GLOBAL GIT_CONFIG_NOSYSTEM
unset GIT_DIR GIT_WORK_TREE GIT_INDEX_FILE
repo="$tmp/repo"
gdir="$repo/.git"
t() { git --git-dir="$gdir" --work-tree="$ROOT" "$@"; }

git init -q --template= "$repo" > /dev/null 2>&1 || { bad hygiene-history-setup "git init"; exit 1; }
if ! tr '\n' '\000' < "$FILELIST" | xargs -0 git --git-dir="$gdir" --work-tree="$ROOT" add -f -- > "$tmp/add.out" 2>&1; then
  bad hygiene-history-setup "git add"
  sed -n '1,3p' "$tmp/add.out"
  exit 1
fi
if ! t -c user.name=gate -c user.email=noreply@github.com -c commit.gpgsign=false commit -q --no-verify \
  -m "gate history self-test" > "$tmp/commit.out" 2>&1; then
  bad hygiene-history-setup "git commit"
  sed -n '1,3p' "$tmp/commit.out"
  exit 1
fi

if grep -q -x -F 'selftest-corrupt-blob' "$FILELIST"; then
  victim=$(head -n 1 "$ROOT/selftest-corrupt-blob")
  case $victim in ''|*[!A-Za-z0-9._/-]*) bad hygiene-history-setup "corrupt-blob path"; exit 1 ;; esac
  id=$(t rev-parse "HEAD:$victim" 2> /dev/null) || { bad hygiene-history-setup "corrupt-blob lookup"; exit 1; }
  rest=${id#??}
  rm -f "$gdir/objects/${id%"$rest"}/$rest"
fi

rc=0
GIT_DIR=$gdir GIT_WORK_TREE=$ROOT sh "$CHECK_DIR/../hygiene.sh" --history > "$tmp/out" 2>&1 || rc=1
n=$(sed -n 's/^-- hygiene pass mode=history commits=\([0-9][0-9]*\)$/\1/p' "$tmp/out")
grep -v -e '^-- hygiene pass ' "$tmp/out"
[ "$rc" -eq 0 ] || exit 1
[ -n "$n" ] || { bad hygiene-history-setup "no pass line from hygiene.sh"; exit 1; }
printf -- '-- hygiene-history pass history-commits=%s\n' "$n"
