#!/bin/sh
# SPDX-License-Identifier: MIT
# Owner-local run helper (plan 07-11, D-30, D-31, T-07-26 to T-07-28; plan 14-03, SCN-03, D-47). NOT a gate step: the gate never passes it an image and CI never runs it.
# It loads the owner's own EXEC and GROM images, and optionally a cartridge container, into ivs-run through a temporary copy of
# data/scenarios/owner-exec-title.ivss that carries `expect-sha256` lines taken from the owner's lock refs/owner-images.sha256, so the core
# itself checks the bytes it loads (docs/owner-images.md).
# The cartridge is an OPTION (`--cart <cart-image>`), never a third positional (plan 14-03, SCN-03). Four checkable reasons: nine existing
# test ids pass exactly two positionals; the lock's grammar is asserted against a `cart` role in tests/runner-process-tap.sh, so the lock
# keeps exactly two roles and that example stays a refusal; the template insertion asserts two `expect-sha256` lines; and the container's
# identity is already in the report as slot.cart.sha256, which is what SCN-03 asks for. The container is an IVSC v1 file the project's own
# wrap tool (ivs_flatwrap) wrote from the operator's flat file at the address the operator stated: this route forwards a path and parses
# nothing, so it names and detects no other format. There is no size check for it here: the core refuses a short container and a long one at
# load with a precise report line, so a route-level size rule would be a second registration of one refusal. The exec and grom exact sizes
# exist because those shapes are exact.
# The inputs are the owner's, not redistributable; the output is private observation, not evidence and no claim.
# Usage: sh tools/owner-run.sh [--preset dev|release] [--record] [--cart <cart-image>] <exec-image> <grom-image>
#   (default release; build/<preset>/runner/ivs-run must be built; --record writes the lock from the two images and refuses to replace an
#   existing lock; --cart forwards the container to the runner and adds `content cart user` to the temporary copy only when it is given)
# Env: CI and GITHUB_ACTIONS must be empty or unset. TMPDIR is honoured for the temporary copy (refused inside the work tree).
# Refuses (one stderr line, exit 2), in this order: a CI environment; bad arguments; an image path inside the work tree outside refs/; an
#   image that is not a regular file of exactly 8192 (EXEC) or 2048 (GROM) bytes; a cartridge that is not a regular file; a missing lock, or
#   a lock line that is not 64 lower-case hex digits, two spaces and exec or grom; a missing runner or template. No output-file option is
#   passed to the runner.
# Exit: 2 refusal; 77 neither image exists (SKIPPED line on stdout); otherwise the exit code of ivs-run, whose report is on stdout.
LC_ALL=C; export LC_ALL
unset CDPATH   # a set CDPATH would let cd resolve a relative directory somewhere else and print it
die() { printf 'owner-run: %s\n' "$1" >&2; exit 2; }
[ -z "${CI:-}" ] && [ -z "${GITHUB_ACTIONS:-}" ] || die 'refusing to run in CI (CI or GITHUB_ACTIONS is set): owner images are never used there'
usage='usage: owner-run.sh [--preset dev|release] [--record] [--cart <cart-image>] <exec-image> <grom-image>'
case $0 in */*) here=${0%/*} ;; *) here=. ;; esac
root=$(cd -P -- "$here/.." 2> /dev/null && pwd -P) || die 'cannot find the repository root'
preset=release; record=0; cart=
while [ "$#" -gt 0 ]; do
  case $1 in
    --preset) [ "$#" -ge 2 ] || die "$usage"; preset=$2; shift 2 ;;
    --record) record=1; shift ;;
    --cart) [ "$#" -ge 2 ] || die "$usage"; cart=$2; shift 2 ;;
    *) break ;;
  esac
done
case $preset in dev | release) ;; *) die "$usage" ;; esac
[ "$#" -eq 2 ] || die "$usage"
ex=$1; gr=$2
if [ ! -e "$ex" ] && [ ! -e "$gr" ]; then
  printf 'SKIPPED owner-run: no operator images (D11)\n'
  exit 77
fi
for p in "$ex" "$gr" "$cart"; do
  [ -n "$p" ] || continue   # the cartridge is optional: an empty value is not a path
  case $p in */*) d=${p%/*} ;; *) d=. ;; esac
  # `--` ends the options of cd (a directory named -x is a directory); a directory that cannot be entered is never a pass: no file can be
  # read in it either, so the exact-size check below refuses the image.
  d=$(cd -P -- "${d:-/}" 2> /dev/null && pwd -P) || continue
  case $d/ in "$root/refs/"*) ;; "$root/"*) die "an image is inside the work tree outside refs/: $p" ;; esac
done
sized() { [ -f "$2" ] && [ "$(wc -c < "$2" | tr -d ' ')" = "$3" ] || die "the $1 image is not a regular file of exactly $3 bytes"; }
sized exec "$ex" 8192
sized grom "$gr" 2048
[ -z "$cart" ] || [ -f "$cart" ] || die "the cart image is not a regular file: $cart"
chk() { case $1 in '' | *[!0-9a-f]*) return 1 ;; esac; [ "${#1}" -eq 64 ]; }
dig() { cmake -E sha256sum "$1" 2> /dev/null | { read -r h _; printf '%s' "$h"; }; }
lock=$root/refs/owner-images.sha256
if [ "$record" -eq 1 ]; then
  [ ! -e "$lock" ] || die 'refs/owner-images.sha256 exists: remove it to record again'
  de=$(dig "$ex")
  dg=$(dig "$gr")
  chk "$de" && chk "$dg" || die 'cannot digest the images (cmake -E sha256sum)'
  mkdir -p "$root/refs" && printf '%s  exec\n%s  grom\n' "$de" "$dg" > "$lock" || die 'cannot write refs/owner-images.sha256'
fi
[ -f "$lock" ] || die 'no lock refs/owner-images.sha256 (write it with --record)'
de=; dg=
while IFS= read -r l || [ -n "$l" ]; do
  h=${l%%  *}
  r=${l#"$h  "}
  chk "$h" && [ "$l" = "$h  $r" ] || die 'bad lock line: want 64 lower-case hex digits, two spaces, exec or grom'
  case $r in
    exec) [ -z "$de" ] && de=$h || die 'bad lock: exec twice' ;;
    grom) [ -z "$dg" ] && dg=$h || die 'bad lock: grom twice' ;;
    *) die 'bad lock line: the role is not exec or grom' ;;
  esac
done < "$lock"
[ -n "$de" ] && [ -n "$dg" ] || die 'bad lock: exec or grom is missing'
run=$root/build/$preset/runner/ivs-run
tpl=$root/data/scenarios/owner-exec-title.ivss
[ -x "$run" ] || die "no runner build/$preset/runner/ivs-run (cmake --build --preset $preset)"
[ -f "$tpl" ] || die 'no template data/scenarios/owner-exec-title.ivss'
tmp=$(mktemp -d "${TMPDIR:-/tmp}/owner-run.XXXXXX" 2> /dev/null) || die 'cannot make a temporary directory'
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
case $(cd -P -- "$tmp" && pwd -P)/ in "$root/"*) die 'TMPDIR is inside the work tree' ;; esac
# The insertion is conditional: with no cartridge the temporary copy is byte-identical to the pre-plan one, and the assertion below
# counts the `content cart user` lines against the arguments so the switch cannot silently invert.
awk -v e="$de" -v g="$dg" -v c="$cart" '{ print } /^# owner-run: expect-sha256 lines are inserted after this line$/ { print "expect-sha256 exec " e; print "expect-sha256 grom " g; if (c != "") print "content cart user" }' \
  "$tpl" > "$tmp/scn.ivss"
[ "$(grep -c '^expect-sha256 ' "$tmp/scn.ivss")" -eq 2 ] || die 'the template has no marker line'
want=0
[ -z "$cart" ] || want=1
[ "$(grep -c -x 'content cart user' "$tmp/scn.ivss")" -eq "$want" ] \
  || die 'the temporary scenario copy does not carry the cartridge role the arguments state'
printf '%s\n' "OWNER-LOCAL RUN (D11): inputs are the owner's, not redistributable, not run in CI, not evidence, no claim"
rc=0
if [ -z "$cart" ]; then
  "$run" --exec "$ex" --grom "$gr" "$tmp/scn.ivss" || rc=$?
else
  "$run" --exec "$ex" --grom "$gr" --cart "$cart" "$tmp/scn.ivss" || rc=$?
fi
exit "$rc"
