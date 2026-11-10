#!/bin/sh
# SPDX-License-Identifier: MIT
# Public-repo hygiene (AUTONOMY.md section 6): no home or scratch path, no secret-shaped
# token, no password assignment and no non-noreply email in anything that is published.
#
#   sh tools/hygiene.sh --tree      every tracked or untracked-not-ignored file (gate step)
#   sh tools/hygiene.sh --history   every commit tree, message, identity and file name
#                                   (publication and release step, not a gate step)
#
# Env for --tree: ROOT (repository root; default git toplevel) and FILELIST (newline list of
# paths relative to ROOT; default git ls-files --cached --others --exclude-standard).
# Findings name the rule and the file (or short commit), never the matched text, so the
# output is safe to paste anywhere. Exit status is 1 if anything was found, 2 on usage.
set -eu
LC_ALL=C
export LC_ALL

HOME_PATH='/Users/[A-Za-z0-9][A-Za-z0-9._-]*|/home/[A-Za-z0-9][A-Za-z0-9._-]*|-Users-[A-Za-z0-9][A-Za-z0-9._]*-|[A-Za-z]:\\Users\\[A-Za-z0-9]'
# Applied to grep -o output (the match alone), so anchors are exact.
HOME_OK='^/home/runner$|^~/[.]claude$'
TOKEN='gh[pousr]_[A-Za-z0-9]{36}|github_pat_[A-Za-z0-9_]{20,}|AKIA[0-9A-Z]{16}|-----BEGIN [A-Z ]*PRIVATE KEY-----'
# More provider shapes. Every character class directly follows a literal that the rule text itself
# does not satisfy (a bracket expression is not a class member), so this file never matches itself.
TOKEN_NEW='xox[baprs]-[A-Za-z0-9-]{10,}|sk-ant-[A-Za-z0-9_-]{20,}|AIza[0-9A-Za-z_-]{35}|glpat-[A-Za-z0-9_-]{20}|ASIA[0-9A-Z]{16}'
# Used with grep -i. Value starts alphanumeric and has 8 or more value characters, so the bare
# word, a backticked prefix and placeholders such as <...> or $VAR do not match.
# PW_NEW (WR-03): the key may be followed by a closing quote (JSON or YAML "password": "..."); the
# password keyword also covers passwd; secret, api_key and token assignments need a quoted value of
# 16 or more value characters so that prose such as "token: expired" is not a finding.
PW_ASSIGN='[p]assword[ ]*[=:][ ]*["'"'"']?[A-Za-z0-9][A-Za-z0-9._~!@#%^&*+/-]{7,}'
PW_NEW='[p]ass(word|wd)["'"'"']?[ ]*[=:][ ]*["'"'"']?[A-Za-z0-9][A-Za-z0-9._~!@#%^&*+/-]{7,}'
PW_NEW="$PW_NEW"'|(secret|api[_-]?key|token)[A-Za-z_]*["'"'"']?[ ]*[=:][ ]*["'"'"'][A-Za-z0-9._~+/-]{16,}'
PW_NEW="$PW_NEW"'|aws_secret_access_key[ ]*[=:][ ]*["'"'"']?[A-Za-z0-9/+]{40}'
# Full rule set: held against the tree and against every commit after the history baseline.
# Commits at or before the baseline (tools/check/hygiene-baseline.txt) were published, or are the
# review record, before these shapes existed and cannot be rewritten; they keep the rules above.
# HOME_NEW (WR-04) adds the scratch and temp-directory forms R4 forbids and tilde-relative paths. A
# tilde path to the tool config directory is allowed (HOME_OK). Names need 3 or more characters so
# that two chance bytes in a binary file do not read as a path.
HOME_NEW='/private/tmp/[A-Za-z0-9._-]+|/private/var/[A-Za-z0-9._-]+|/var/folders/[A-Za-z0-9]|/tmp/claude-[A-Za-z0-9._-]+|/root/[A-Za-z0-9.]|~/[A-Za-z0-9._-]{3,}|-private-tmp-[A-Za-z0-9]'
HOME_ALL="$HOME_PATH|$HOME_NEW"
TOKEN_ALL="$TOKEN|$TOKEN_NEW"
PW_ALL=$PW_NEW
EMAIL='[A-Za-z0-9._%+-]+@[A-Za-z0-9-]+(\.[A-Za-z0-9-]+)*\.[A-Za-z][A-Za-z]+'
# Applied to grep -o output; anchored so a look-alike suffix domain is still a finding.
EMAIL_OK='@users\.noreply\.github\.com$|^noreply@github\.com$|@example\.(com|org|net)$|^git@github\.com$|^noreply@anthropic\.com$'
# Binary-by-name extensions come from the one list the gate also reads (tools/check/binary-ext.txt).
here=$(cd "$(dirname "$0")" && pwd) || exit 1
BINARY_NAME=
if [ -s "$here/check/binary-ext.txt" ]; then
  BINARY_NAME='\.('$(grep -v -e '^#' -e '^[[:space:]]*$' "$here/check/binary-ext.txt" | tr '\n' '|' | sed 's/|$//')')$'
fi

fail=0
found() { printf -- '-- FAIL hygiene-%s: %s\n' "$1" "$2"; fail=1; }

usage() { printf 'usage: sh tools/hygiene.sh --tree | --history\n' >&2; exit 2; }
[ $# -eq 1 ] || usage
mode=$1
case $mode in --tree|--history) ;; *) usage ;; esac
if [ -z "$BINARY_NAME" ]; then found unreadable "binary-ext.txt"; exit 1; fi

tmp=$(mktemp -d "${TMPDIR:-/tmp}/hygiene.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM

# True if file $1 has a match of ERE $2 that is not allowed by ERE $3 (grep flags in $4; $5 is the
# name reported if grep itself fails, default $1). A grep failure (exit status 2 or more) is a
# finding, never "no match": unreadable input must not read as clean.
file_leaks() {
  _s=0
  grep -a -o $4 -E -e "$2" "$1" > "$tmp/fl.out" 2> "$tmp/fl.err" || _s=$?
  if [ "$_s" -ge 2 ]; then
    found grep-error "${5:-$1}"
    sed -n '1,3p' "$tmp/fl.err" >&2
    return 1
  fi
  grep -a -v -E -e "$3" "$tmp/fl.out" | grep -q .
}

if [ "$mode" = --tree ]; then
  if [ -n "${ROOT:-}" ]; then root=$ROOT; else root=$(git rev-parse --show-toplevel) || exit 1; fi
  cd "$root" || exit 1
  if [ -n "${FILELIST:-}" ]; then
    [ -f "$FILELIST" ] || { found unreadable "FILELIST"; printf -- '-- hygiene: no file list\n'; exit 1; }
    cp "$FILELIST" "$tmp/files"
  else
    git -c core.quotePath=false ls-files --cached --others --exclude-standard > "$tmp/files.raw" \
      || { found unreadable "git-ls-files"; exit 1; }
    : > "$tmp/files"
    while IFS= read -r f; do
      if [ -e "$f" ]; then printf '%s\n' "$f" >> "$tmp/files"; fi
    done < "$tmp/files.raw"
  fi
  n=$(grep -c . "$tmp/files" || true)
  if [ "$n" -eq 0 ]; then found unreadable "empty-file-list"; exit 1; fi

  # Readability is checked per file (fail closed); the content scans are batched.
  : > "$tmp/ok"
  while IFS= read -r f; do
    [ -n "$f" ] || continue
    if [ -f "$f" ] && [ -r "$f" ]; then printf './%s\0' "$f" >> "$tmp/ok"; else found unreadable "$f"; fi
  done < "$tmp/files"

  # Batch candidate scans, then filter the few candidates one by one. The batch runs grep through
  # a wrapper that turns grep status 2 or more into status 255, which makes xargs stop and fail:
  # status 1 (no match in a batch) is normal, an error is a finding.
  cand() { # $1 regex, $2 grep flags; prints candidate files
    [ -s "$tmp/ok" ] || return 0
    GF=$2 PAT=$1 xargs -0 sh -c 'grep -a -l $GF -E -e "$PAT" "$@"; s=$?; [ "$s" -le 1 ] || exit 255' sh \
      < "$tmp/ok" 2> "$tmp/cand.err" || {
      found grep-error "candidate-scan"
      sed -n '1,3p' "$tmp/cand.err" >&2
    }
  }
  cand "$HOME_ALL" "" > "$tmp/c.raw"
  sort -u "$tmp/c.raw" > "$tmp/c.home"
  while IFS= read -r f; do
    [ -n "$f" ] || continue
    if file_leaks "$f" "$HOME_ALL" "$HOME_OK" "" "${f#./}"; then found home-path "${f#./}"; fi
  done < "$tmp/c.home"
  { cand "$TOKEN_ALL" ""; cand "$PW_ALL" "-i"; } > "$tmp/c.raw"
  sort -u "$tmp/c.raw" > "$tmp/c.secret"
  while IFS= read -r f; do
    [ -n "$f" ] || continue
    found secret "${f#./}"
  done < "$tmp/c.secret"
  cand "$EMAIL" "" > "$tmp/c.raw"
  sort -u "$tmp/c.raw" > "$tmp/c.email"
  while IFS= read -r f; do
    [ -n "$f" ] || continue
    if file_leaks "$f" "$EMAIL" "$EMAIL_OK" "" "${f#./}"; then found email "${f#./}"; fi
  done < "$tmp/c.email"

  if [ "$fail" -eq 0 ]; then printf -- '-- hygiene pass mode=tree hygiene-files=%s\n' "$n"; fi
  [ "$fail" -eq 0 ]
  exit $?
fi

# --history: publication check over every commit reachable from any ref.
git() { command git -c core.quotePath=false "$@"; }
cd "$(git rev-parse --show-toplevel)" || exit 1
git rev-list --all > "$tmp/commits" || { found unreadable "git-rev-list"; exit 1; }
n=$(grep -c . "$tmp/commits" || true)
# History baseline: first non-comment line of tools/check/hygiene-baseline.txt, if it names a
# commit of this repository. Without one (for example a fresh repository) every commit is strict.
base_commit=
if [ -f "$here/check/hygiene-baseline.txt" ]; then
  b=$(grep -v -e '^#' -e '^[[:space:]]*$' "$here/check/hygiene-baseline.txt" | head -n 1 || true)
  if [ -n "$b" ] && git cat-file -e "$b^{commit}" 2>/dev/null; then base_commit=$b; fi
fi
# git grep wrapper: prints to $tmp/hg.out. Status 0 (match) and 1 (no match) are normal; any other
# status is a finding, and so is an "error:" or "fatal:" line on stderr (git grep reports an
# unreadable object that way while still exiting 1, which would otherwise read as "no match").
hgrep() {
  _s=0
  git grep "$@" > "$tmp/hg.out" 2> "$tmp/hg.err" || _s=$?
  if [ "$_s" -ge 2 ] || grep -q -E '^(error|fatal):' "$tmp/hg.err"; then
    found history-grep-error "$s"
    sed -n '1,3p' "$tmp/hg.err" >&2
    return 1
  fi
  return 0
}
while IFS= read -r c; do
  [ -n "$c" ] || continue
  s=$(printf '%s' "$c" | cut -c1-10)
  h=$HOME_ALL; t=$TOKEN_ALL; w=$PW_ALL
  if [ -n "$base_commit" ] && git merge-base --is-ancestor "$c" "$base_commit"; then
    h=$HOME_PATH; t=$TOKEN; w=$PW_ASSIGN
  fi
  if hgrep -a -h -o -E -e "$h" "$c" -- . && grep -a -v -E -e "$HOME_OK" "$tmp/hg.out" | grep -q .; then
    found history-home-path "$s"
  fi
  leak=0
  if hgrep -a -l -E -e "$t" "$c" -- . && [ -s "$tmp/hg.out" ]; then leak=1; fi
  if hgrep -a -l -i -E -e "$w" "$c" -- . && [ -s "$tmp/hg.out" ]; then leak=1; fi
  if [ "$leak" -eq 1 ]; then found history-secret "$s"; fi
  if ! git log -1 --format=%B "$c" > "$tmp/msg" 2> "$tmp/hg.err"; then
    found history-log-error "$s"
    continue
  fi
  if file_leaks "$tmp/msg" "$h" "$HOME_OK" "" "message-$s"; then found message-home-path "$s"; fi
  if file_leaks "$tmp/msg" "$t" '^$' "" "message-$s" || file_leaks "$tmp/msg" "$w" '^$' "-i" "message-$s"; then
    found message-secret "$s"
  fi
  if file_leaks "$tmp/msg" "$EMAIL" "$EMAIL_OK" "" "message-$s"; then found message-email "$s"; fi
done < "$tmp/commits"
git log --all --format='%ae%n%ce' > "$tmp/idents.raw" || { found unreadable "git-log-identities"; exit 1; }
sort -u "$tmp/idents.raw" > "$tmp/idents"
while IFS= read -r e; do
  [ -n "$e" ] || continue
  if ! printf '%s\n' "$e" | grep -q -E -e "$EMAIL_OK"; then found identity "identity-not-noreply"; fi
done < "$tmp/idents"
git log --all --name-only --format= > "$tmp/names.raw" || { found unreadable "git-log-names"; exit 1; }
sort -u "$tmp/names.raw" > "$tmp/names"
while IFS= read -r p; do
  [ -n "$p" ] || continue
  base=${p##*/}
  hit=0
  if printf '%s\n' "$p" | grep -q -i -E -e "$BINARY_NAME"; then hit=1; fi
  case $p in refs/*) hit=1 ;; esac
  case $base in .env|.env.*) hit=1 ;; esac
  if [ "$hit" -eq 1 ]; then found history-binary "$p"; fi
done < "$tmp/names"
if [ "$fail" -eq 0 ]; then printf -- '-- hygiene pass mode=history commits=%s\n' "$n"; fi
[ "$fail" -eq 0 ]
