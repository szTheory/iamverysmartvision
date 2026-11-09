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
HOME_OK='^/home/runner$'
TOKEN='gh[pousr]_[A-Za-z0-9]{36}|github_pat_[A-Za-z0-9_]{20,}|AKIA[0-9A-Z]{16}|-----BEGIN [A-Z ]*PRIVATE KEY-----'
# Used with grep -i. Value starts alphanumeric and has 8 or more value characters, so the bare
# word, a backticked prefix and placeholders such as <...> or $VAR do not match.
PW_ASSIGN='[p]assword[ ]*[=:][ ]*["'"'"']?[A-Za-z0-9][A-Za-z0-9._~!@#%^&*+/-]{7,}'
EMAIL='[A-Za-z0-9._%+-]+@[A-Za-z0-9-]+(\.[A-Za-z0-9-]+)*\.[A-Za-z][A-Za-z]+'
# Applied to grep -o output; anchored so a look-alike suffix domain is still a finding.
EMAIL_OK='@users\.noreply\.github\.com$|^noreply@github\.com$|@example\.(com|org|net)$|^git@github\.com$|^noreply@anthropic\.com$'
BINARY_NAME='\.(int|bin|rom|itv|cfg|cc3|pdf|png|jpe?g|gif)$'

fail=0
found() { printf -- '-- FAIL hygiene-%s: %s\n' "$1" "$2"; fail=1; }

usage() { printf 'usage: sh tools/hygiene.sh --tree | --history\n' >&2; exit 2; }
[ $# -eq 1 ] || usage
mode=$1
case $mode in --tree|--history) ;; *) usage ;; esac

tmp=$(mktemp -d "${TMPDIR:-/tmp}/hygiene.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM

# True if file $1 has a match of ERE $2 that is not allowed by ERE $3 (grep flags in $4).
file_leaks() { grep -I -o $4 -E -e "$2" "$1" 2>/dev/null | grep -v -E -e "$3" | grep -q .; }

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

  # Batch candidate scans, then filter the few candidates one by one.
  cand() { # $1 regex, $2 grep flags
    if [ -s "$tmp/ok" ]; then xargs -0 grep -I -l $2 -E -e "$1" < "$tmp/ok" 2>/dev/null || true; fi
  }
  cand "$HOME_PATH" "" | sort -u > "$tmp/c.home"
  while IFS= read -r f; do
    [ -n "$f" ] || continue
    if file_leaks "$f" "$HOME_PATH" "$HOME_OK" ""; then found home-path "${f#./}"; fi
  done < "$tmp/c.home"
  { cand "$TOKEN" ""; cand "$PW_ASSIGN" "-i"; } | sort -u > "$tmp/c.secret"
  while IFS= read -r f; do
    [ -n "$f" ] || continue
    found secret "${f#./}"
  done < "$tmp/c.secret"
  cand "$EMAIL" "" | sort -u > "$tmp/c.email"
  while IFS= read -r f; do
    [ -n "$f" ] || continue
    if file_leaks "$f" "$EMAIL" "$EMAIL_OK" ""; then found email "${f#./}"; fi
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
while IFS= read -r c; do
  [ -n "$c" ] || continue
  s=$(printf '%s' "$c" | cut -c1-10)
  if git grep -I -h -o -E -e "$HOME_PATH" "$c" -- . 2>/dev/null | grep -v -E -e "$HOME_OK" | grep -q .; then
    found history-home-path "$s"
  fi
  if { git grep -I -l -E -e "$TOKEN" "$c" -- . 2>/dev/null; git grep -I -l -i -E -e "$PW_ASSIGN" "$c" -- . 2>/dev/null; } | grep -q .; then
    found history-secret "$s"
  fi
  git log -1 --format=%B "$c" > "$tmp/msg"
  if file_leaks "$tmp/msg" "$HOME_PATH" "$HOME_OK" ""; then found message-home-path "$s"; fi
  if file_leaks "$tmp/msg" "$TOKEN" '^$' "" || file_leaks "$tmp/msg" "$PW_ASSIGN" '^$' "-i"; then
    found message-secret "$s"
  fi
  if file_leaks "$tmp/msg" "$EMAIL" "$EMAIL_OK" ""; then found message-email "$s"; fi
done < "$tmp/commits"
git log --all --format='%ae%n%ce' | sort -u > "$tmp/idents"
while IFS= read -r e; do
  [ -n "$e" ] || continue
  if ! printf '%s\n' "$e" | grep -q -E -e "$EMAIL_OK"; then found identity "identity-not-noreply"; fi
done < "$tmp/idents"
git log --all --name-only --format= | sort -u > "$tmp/names"
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
