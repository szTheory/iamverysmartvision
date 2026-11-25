#!/bin/sh
# SPDX-License-Identifier: MIT
# Local rehearsal of the release job (plan 08-02; D-29, D-32, assumption A6). NOT a gate step, never run
# by CI, and it never calls the GitHub CLI: it finds non-determinism and a wrong tag glob before a tag is
# pushed, so that the first release job is not the first time the script is exercised.
#
#   sh tools/release-rehearse.sh [--no-gate] [--release] [--source-route]
#
# It runs, in this order:
#   1. tools/release.sh as a dry run into a first temporary directory (the gate included unless --no-gate,
#      which reuses the existing build/release and its images);
#   2. the same dry run again into a second directory, always with --no-gate (the gate just ran; the point
#      is to repeat the packaging of the same commit and builds);
#   3. the two source archives must be byte-equal (a function of the commit);
#   4. --source-route only: the source archive step 3 proved byte-equal is extracted into this run's
#      workspace, configured and built cold with the release preset, taken through demo-lamp-tone-pass to
#      its report, and that report is compared with data/golden/demo-lamp-tone-pass.digests using a copy of
#      the gate's own filter (route_live and route_filter below, whose comment records where the copy came
#      from and why it stops on error). The archive name and the extraction prefix are read from the run,
#      never written here, so a version bump needs no edit in this file;
#   5. the two install archives are compared and the result is printed: "install-archive: equal" for a
#      byte-equal pair, "install-archive: DIFFERENT" (exit 1) otherwise. One host effect is named and
#      bounded: on Darwin the install step re-runs ranlib, which restamps the time field of the first
#      member header of lib/libivs_core.a (bytes 25 to 36 of the file) and nothing else; a pair whose only
#      difference is exactly that prints "install-archive: equal-except-ranlib-stamp" and is accepted
#      there, with the note that byte equality is then decided on Linux (the hosted job's platform);
#   6. the tag glob of .github/workflows/release.yml (its single tag item) is tried on sample tags with a
#      shell case: v0.1.0 and v1.2.3 must match; v1.0, v0.1, vfoo, v and 1.0.0 must not;
#   7. sh tools/release-readiness.sh --allow-dirty, of which sections A, F and G are printed.
#   --release  passes --release to both dry runs and to the readiness call: the run halts first, with
#              release-changelog-section-missing, when CHANGELOG.md has no section for the version of the
#              header macros. Without it the section is not required (changelog-section: not required).
#   --no-gate  both dry runs reuse the existing builds.
#   --source-route  also rehearse the source route (step 4): the archive a release carries is unpacked,
#              built cold and compared with the committed golden. Release-time only: it runs no gate step,
#              adds no control and moves no count, and it writes nothing into the repository.
# Every temporary path is under one mktemp directory removed on exit. The tar mode that ran is printed.
# Exit: 0 ok, 1 a check failed, 2 usage error or a missing tool.
LC_ALL=C; export LC_ALL

usage() {
  sed -n '3,38p' "$0" | sed 's/^# \{0,1\}//'
}
refuse() { printf 'rehearse: refused: %s\n' "$1" >&2; exit 2; }
fail() { printf 'rehearse: FAIL: %s\n' "$1"; exit 1; }

# The golden comparison of the --source-route mode. Both functions are copies of the gate's own program in
# tools/check/steps/golden.sh: route_live is live_count (:159-161) and route_filter is filter_report
# (:146-156), copied verbatim because this script is release-time and cannot source a gate step. The copy
# is safe because it fails closed: a stale copy of the key list selects a different set of lines, so the
# cmp against the committed golden differs and the mode exits 1. There is no second oracle here - the
# right-hand side is data/golden/demo-lamp-tone-pass.digests, the same file the gate's golden step reads.
# Each is opened with its name and closed with a brace in column 1 so the pair can be extracted from this
# file and re-run on its own.
route_live() {
  awk -v k="$1" '$1 == k { v = $2; n++ } END { print (n == 1 && v ~ /^[0-9]+$/ && v !~ /^0+$/) ? 1 : 0 }' "$2"
}
route_filter() {
  awk -v v="$1" -v a="$2" -v i="$3" -v p="$4" '
    $1 ~ /^(slot|step|cpu|input)\./ { print; next }
    $1 ~ /^(scenario\.id|scenario\.profile|outcome|outcome\.detail|exit\.code)$/ { print; next }
    $1 ~ /^digest\.(cpu|mem|images|instance|composite)$/ { print; next }
    $1 ~ /^output\.(video|audio)\.bytes$/ { print; next }
    ($1 ~ /^(digest\.video|output\.video\.frames|video\.geometry|video\.row_lines|video\.frame\.last_tick|video\.chain\.check)$/ && v == 1) { print; next }
    ($1 ~ /^(digest\.audio|output\.audio\.samples|output\.audio\.changes|audio\.sample_ticks)$/ && a == 1) { print; next }
    ($1 == "digest.input" && i == 1) { print; next }
    ($1 ~ /^(digest\.psglog|output\.psglog\.records|output\.psglog\.bytes)$/ && p == 1) { print }'
}

nogate=0
release=0
srcroute=0
while [ "$#" -gt 0 ]; do
  case $1 in
    --no-gate) nogate=1 ;;
    --release) release=1 ;;
    --source-route) srcroute=1 ;;
    -h | --help) usage; exit 0 ;;
    *) usage >&2; exit 2 ;;
  esac
  shift
done

here=$(cd "$(dirname "$0")" && pwd) || exit 2
ROOT=$(cd "$here/.." && pwd) || exit 2
self="$here/$(basename "$0")"
cd "$ROOT" || exit 2
for tool in git tar gzip awk cmake cmp mktemp find sort; do
  command -v "$tool" > /dev/null 2>&1 || refuse "$tool not found"
done

# The copy above is CHECKED, not trusted (PLAN-LESSONS 20: an equivalence check needs a guard that
# the thing it compares really happened, and here the comment is the only other evidence). Both
# functions must stay byte-identical to the gate's own program at tools/check/steps/golden.sh,
# renamed; the source is extracted by name from that file and the copy by name from this one, so the
# comparison is against the code that actually runs, not against a second transcription of it. Run
# on every rehearsal rather than only with --source-route, because a stale copy is a false statement
# in this file even on a run that does not use it.
drift() {
  # $1 is the name in the gate step, $2 the name of the copy in this script.
  _src=$(sed -n "/^$1() {/,/^}/p" tools/check/steps/golden.sh | sed "s/^$1() {/$2() {/")
  _cpy=$(sed -n "/^$2() {/,/^}/p" "$self")
  [ -n "$_src" ] && [ "$_src" = "$_cpy" ] \
    || fail "$2 has drifted from tools/check/steps/golden.sh ($1)"
}
drift filter_report route_filter
drift live_count route_live
work=$(mktemp -d "${TMPDIR:-/tmp}/release-rehearse.XXXXXX") || exit 2
trap 'rm -rf "$work"' EXIT
trap 'exit 2' HUP INT TERM

rel_flag=
if [ "$release" -eq 1 ]; then rel_flag=--release; fi

# 1 and 2: the dry run, twice.
run1_flags=$rel_flag
if [ "$nogate" -eq 1 ]; then run1_flags="${run1_flags:+$run1_flags }--no-gate"; fi
printf 'rehearse: dry run 1 (sh tools/release.sh%s)\n' "${run1_flags:+ $run1_flags}"
mkdir "$work/a" || exit 2
st=0
# shellcheck disable=SC2086
sh tools/release.sh $run1_flags --out "$work/a" > "$work/run1.log" 2>&1 < /dev/null || st=$?
if [ "$st" -ne 0 ]; then
  sed 's/^/  /' "$work/run1.log"
  fail "dry run 1 exited $st"
fi
sed -n 's/^release: \(tag .*\)$/  \1/p; s/^release: \(install archive built with .*\)$/  \1/p; s/^release: \(notes: .*\)$/  \1/p' "$work/run1.log"
tar_mode=$(sed -n 's/^release: install archive built with \([a-z-]*\) .*$/\1/p' "$work/run1.log")
printf 'rehearse: tar mode %s\n' "${tar_mode:-unknown}"

printf 'rehearse: dry run 2 (sh tools/release.sh%s --no-gate)\n' "${rel_flag:+ $rel_flag}"
mkdir "$work/b" || exit 2
st=0
# shellcheck disable=SC2086
sh tools/release.sh $rel_flag --no-gate --out "$work/b" > "$work/run2.log" 2>&1 < /dev/null || st=$?
if [ "$st" -ne 0 ]; then
  sed 's/^/  /' "$work/run2.log"
  fail "dry run 2 exited $st"
fi

if [ "$release" -eq 0 ]; then printf 'rehearse: changelog-section: not required\n'; else printf 'rehearse: changelog-section: present\n'; fi

# 3: the source archives.
src_a=$(ls "$work/a" | grep -e '-source[.]tar[.]gz$' | head -n 1)
src_b=$(ls "$work/b" | grep -e '-source[.]tar[.]gz$' | head -n 1)
[ -n "$src_a" ] && [ "$src_a" = "$src_b" ] || fail "the two runs did not write the same source archive name"
if cmp -s "$work/a/$src_a" "$work/b/$src_b"; then
  printf 'rehearse: source-archive: equal (%s bytes)\n' "$(wc -c < "$work/a/$src_a" | tr -d ' ')"
else
  fail "source-archive: DIFFERENT (the source archive must be a function of the commit)"
fi

# 4: the source route (--source-route). Release-time only, and it consumes what step 3 already proved: the
# archive is not rebuilt and not re-hashed here. The name comes from the workspace listing and the prefix
# from the archive's first member, so a version bump needs no edit in this file.
if [ "$srcroute" -eq 1 ]; then
  [ -f "$work/a/$src_a" ] || fail "source-route: the source archive $src_a is absent from the first dry run"
  first=$(tar -tzf "$work/a/$src_a" | sed -n '1p')
  case $first in
    */) ;;
    *) fail "source-route: the first member of $src_a ($first) does not end in a slash" ;;
  esac
  prefix=${first%/}
  [ -n "$prefix" ] || fail "source-route: the first member of $src_a has an empty prefix"
  mkdir "$work/route" || exit 2
  tar -xzf "$work/a/$src_a" -C "$work/route" || fail "source-route: extraction of $src_a into the workspace failed"
  [ -d "$work/route/$prefix" ] || fail "source-route: extraction produced no $prefix directory"
  st=0
  (cd "$work/route/$prefix" && cmake --preset release) > "$work/route/configure.log" 2>&1 < /dev/null || st=$?
  if [ "$st" -ne 0 ]; then
    sed 's/^/  /' "$work/route/configure.log"
    fail "source-route: configure (cmake --preset release) exited $st"
  fi
  st=0
  (cd "$work/route/$prefix" && cmake --build --preset release) > "$work/route/build.log" 2>&1 < /dev/null || st=$?
  if [ "$st" -ne 0 ]; then
    sed 's/^/  /' "$work/route/build.log"
    fail "source-route: build (cmake --build --preset release) exited $st"
  fi
  rep="$work/route/report"
  st=0
  (cd "$work/route/$prefix" && ./build/release/runner/ivs-run --content-dir build/release/images \
    data/scenarios/demo-lamp-tone-pass.ivss) > "$rep" 2> "$work/route/run.err" < /dev/null || st=$?
  if [ "$st" -ne 0 ]; then
    sed 's/^/  /' "$work/route/run.err"
    fail "source-route: the runner on demo-lamp-tone-pass exited $st"
  fi
  [ -s "$rep" ] || fail "source-route: the runner wrote an empty report"
  route_filter "$(route_live output.video.bytes "$rep")" "$(route_live output.audio.bytes "$rep")" \
    "$(route_live input.events "$rep")" "$(route_live output.psglog.bytes "$rep")" \
    < "$rep" > "$work/route/want"
  [ -s "$work/route/want" ] || fail "source-route: the filtered report is empty (the filter matched nothing)"
  awk '/^#/ { next } { print }' data/golden/demo-lamp-tone-pass.digests > "$work/route/have"
  cmp -s "$work/route/want" "$work/route/have" \
    || fail "source-route: the filtered report differs from data/golden/demo-lamp-tone-pass.digests"
  printf 'rehearse: source-route: %s, golden match (report %s lines, %s of %s compared), prefix %s\n' \
    "$src_a" "$(wc -l < "$rep" | tr -d ' ')" "$(wc -l < "$work/route/want" | tr -d ' ')" \
    "$(wc -l < "$work/route/have" | tr -d ' ')" "$prefix"
fi

# 5: the install archives.
ins_a=$(ls "$work/a" | grep -e '-install-.*[.]tar[.]gz$' | head -n 1)
ins_b=$(ls "$work/b" | grep -e '-install-.*[.]tar[.]gz$' | head -n 1)
[ -n "$ins_a" ] && [ "$ins_a" = "$ins_b" ] || fail "the two runs did not write the same install archive name"
if cmp -s "$work/a/$ins_a" "$work/b/$ins_b"; then
  printf 'install-archive: equal (%s bytes)\n' "$(wc -c < "$work/a/$ins_a" | tr -d ' ')"
else
  mkdir "$work/xa" "$work/xb" || exit 2
  (cd "$work/xa" && tar -xzf "$work/a/$ins_a") || fail "cannot extract the first install archive"
  (cd "$work/xb" && tar -xzf "$work/b/$ins_b") || fail "cannot extract the second install archive"
  (cd "$work/xa" && find . -type f | sort) | while IFS= read -r f; do
    cmp -s "$work/xa/$f" "$work/xb/$f" || printf '%s\n' "$f"
  done > "$work/differing"
  nd=$(grep -c . "$work/differing")
  lib=$(sed -n '1p' "$work/differing")
  accepted=0
  if [ "$(uname -s)" = Darwin ] && [ "$nd" -eq 1 ]; then
    case $lib in
      */lib/libivs_core.a)
        if [ "$(wc -c < "$work/xa/$lib")" -eq "$(wc -c < "$work/xb/$lib")" ] \
          && cmp -l "$work/xa/$lib" "$work/xb/$lib" | awk '$1 < 25 || $1 > 36 { bad = 1 } END { exit bad }'; then
          accepted=1
        fi
        ;;
    esac
  fi
  if [ "$accepted" -eq 1 ]; then
    printf 'install-archive: equal-except-ranlib-stamp (Darwin: only the time field of the first member header of lib/libivs_core.a differs, bytes 25 to 36; byte equality is decided on Linux)\n'
  else
    printf 'install-archive: DIFFERENT (%s member(s) differ' "$nd"
    if [ "$nd" -gt 0 ]; then printf ': %s' "$(sed -n '1,5p' "$work/differing" | tr '\n' ' ')"; fi
    printf ')\n'
    fail "the two install archives differ: non-determinism must be found before the tag"
  fi
fi

# 6: the tag glob of the release workflow.
awk '
  /^on:/ { inon = 1; next }
  inon && /^[^ #]/ { inon = 0 }
  inon && /^    tags:[ ]*$/ { intags = 1; next }
  intags && /^      - / { sub(/^      - /, ""); gsub(/[\047"]/, ""); print; next }
  intags { intags = 0 }
' .github/workflows/release.yml > "$work/tagitems"
[ "$(grep -c . "$work/tagitems")" -eq 1 ] || fail "release.yml must hold exactly one tag item"
glob=$(sed -n '1p' "$work/tagitems")
for t in v0.1.0 v1.2.3; do
  case $t in $glob) ;; *) fail "the tag glob $glob does not match $t" ;; esac
done
for t in v1.0 v0.1 vfoo v 1.0.0; do
  case $t in $glob) fail "the tag glob $glob matches $t" ;; *) ;; esac
done
printf 'rehearse: tag-glob: %s matches v0.1.0 and v1.2.3, not v1.0, v0.1, vfoo, v, 1.0.0\n' "$glob"

# 7: the readiness sections.
rr_flags=--allow-dirty
if [ "$release" -eq 1 ]; then rr_flags="--allow-dirty --release"; fi
st=0
# shellcheck disable=SC2086
sh tools/release-readiness.sh $rr_flags > "$work/readiness.out" 2>&1 < /dev/null || st=$?
awk '/^## J /{ stop = 1 } !stop { print }' "$work/readiness.out"
grep '^P4: ' "$work/readiness.out"
[ "$st" -eq 0 ] || fail "the readiness script exited $st"
printf 'rehearse: ok\n'
