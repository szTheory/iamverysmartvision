#!/bin/sh
# SPDX-License-Identifier: MIT
# Release job script (plan 08-01, D-29, D-32): the single run line of .github/workflows/release.yml.
#
#   sh tools/release.sh [--publish] [--no-gate] [--tag vX.Y.Z] [--out DIR]
#
# Default is a dry run that never touches the GitHub CLI. It builds, in order: the tag, the gate
# (unless --no-gate), the deterministic source archive (git archive with the prefix ivs-<tag>/, then
# gzip -n), the install archive of the release preset (cmake --install into a staging prefix, members
# sorted, mtime fixed to the commit time, owner 0), the content check of both archives
# (tools/check/release-assets.sh: members equal git ls-files and data/expected/pkg-install-files.ids, no
# binary extension on any member), SHA256SUMS (cmake -E sha256sum), and the installed consumer built
# against the very install archive that ships and compared with data/expected/pkg-consumer.golden.
#   --publish  only inside GitHub Actions (GITHUB_ACTIONS=true, else exit 2), never with --no-gate: after
#              every check, creates the release as a draft pre-release with the gh CLI
#              (--draft --prerelease --verify-tag --latest=false). It is the only path that calls gh.
#   --no-gate  skip tools/gate.sh; needs build/release and its images (local rehearsal only).
#   --tag T    the tag, strictly v<digits>.<digits>.<digits>; default GITHUB_REF_NAME, else v plus the
#              three IVS_API_VERSION macros of core/include/ivs/ivs.h.
#   --out DIR  asset directory; default build/release-assets (recreated); another DIR must be absent or
#              empty. Assets: ivs-<tag>-source.tar.gz, ivs-<tag>-install-<os>-<arch>.tar.gz, SHA256SUMS.
# The release body is the matching section of CHANGELOG.md, headed with the version in square brackets;
# --publish refuses, before any gh call, when that section is absent or empty.
# Every temporary path is under one mktemp directory removed on exit. No absolute path of this host is
# written into an asset (the install tree is scanned for the checkout path).
# Exit: 0 ok, 1 a check failed, 2 usage error or refusal.
LC_ALL=C; export LC_ALL

usage() {
  sed -n '3,26p' "$0" | sed 's/^# \{0,1\}//'
}
refuse() { printf 'release: refused: %s\n' "$1" >&2; exit 2; }
fail() { printf 'release: FAIL: %s\n' "$1" >&2; exit 1; }

publish=0
nogate=0
tag=
out=
while [ "$#" -gt 0 ]; do
  case $1 in
    --publish) publish=1 ;;
    --no-gate) nogate=1 ;;
    --tag)
      [ "$#" -ge 2 ] || { usage >&2; exit 2; }
      tag=$2
      shift
      ;;
    --out)
      [ "$#" -ge 2 ] || { usage >&2; exit 2; }
      out=$2
      shift
      ;;
    -h | --help) usage; exit 0 ;;
    *) usage >&2; exit 2 ;;
  esac
  shift
done

# The publish guard comes before any other work: --publish exists only inside GitHub Actions.
if [ "$publish" -eq 1 ]; then
  [ "${GITHUB_ACTIONS:-}" = true ] || refuse "--publish only runs inside GitHub Actions"
  [ "$nogate" -eq 0 ] || refuse "--publish does not skip the gate"
fi

here=$(cd "$(dirname "$0")" && pwd) || exit 2
ROOT=$(cd "$here/.." && pwd) || exit 2
cd "$ROOT" || exit 2
for tool in git tar gzip awk cmake; do
  command -v "$tool" > /dev/null 2>&1 || refuse "$tool not found"
done
if [ "$publish" -eq 1 ]; then command -v gh > /dev/null 2>&1 || refuse "gh not found"; fi

# (a) The tag.
if [ -z "$tag" ]; then tag=${GITHUB_REF_NAME:-}; fi
if [ -z "$tag" ]; then
  hdr=core/include/ivs/ivs.h
  vmaj=$(sed -n 's/^#define IVS_API_VERSION_MAJOR \([0-9][0-9]*\)[uU]\{0,1\}$/\1/p' "$hdr")
  vmin=$(sed -n 's/^#define IVS_API_VERSION_MINOR \([0-9][0-9]*\)[uU]\{0,1\}$/\1/p' "$hdr")
  vpat=$(sed -n 's/^#define IVS_API_VERSION_PATCH \([0-9][0-9]*\)[uU]\{0,1\}$/\1/p' "$hdr")
  [ -n "$vmaj" ] && [ -n "$vmin" ] && [ -n "$vpat" ] || refuse "cannot read the IVS_API_VERSION macros of $hdr"
  tag=v$vmaj.$vmin.$vpat
fi
printf '%s\n' "$tag" | grep -E -q '^v[0-9]+[.][0-9]+[.][0-9]+$' || refuse "the tag '$tag' is not v<digits>.<digits>.<digits>"
version=${tag#v}

# The notes file: the CHANGELOG section of the version. A missing section is a refusal for --publish
# (before any gh call); a dry run only reports it.
work=$(mktemp -d "${TMPDIR:-/tmp}/release.XXXXXX") || exit 2
trap 'rm -rf "$work"' EXIT
trap 'exit 2' HUP INT TERM
notes="$work/notes.md"
if [ -f CHANGELOG.md ]; then
  awk -v h="## [$version]" '
    index($0, h) == 1 { on = 1; next }
    on && /^## \[/ { exit }
    on { print }' CHANGELOG.md > "$notes"
else
  : > "$notes"
fi
if grep -q '[^[:space:]]' "$notes"; then
  printf 'release: notes: CHANGELOG.md section [%s] present\n' "$version"
elif [ "$publish" -eq 1 ]; then
  refuse "CHANGELOG.md has no section [$version], nothing to publish as the release body"
else
  printf 'release: notes: CHANGELOG.md section [%s] absent (a dry run does not need it)\n' "$version"
fi

# The output directory: the default is recreated, any other must be absent or empty.
default_out=build/release-assets
[ -n "$out" ] || out=$default_out
case $out in /*) ;; *) out=$ROOT/$out ;; esac
if [ "$out" = "$ROOT/$default_out" ]; then
  rm -rf "$out"
elif [ -e "$out" ] && { [ ! -d "$out" ] || [ -n "$(ls -A "$out")" ]; }; then
  refuse "the output directory $out must be absent or empty"
fi
mkdir -p "$out" || exit 2

printf 'release: tag %s, publish=%s, gate=%s\n' "$tag" "$publish" "$([ "$nogate" -eq 1 ] && echo skipped || echo run)"
if [ -n "$(git status --porcelain --untracked-files=no 2> /dev/null)" ]; then
  printf 'release: note: tracked files differ from HEAD; the assets are built from HEAD\n'
fi

# (b) The gate, or the local rehearsal prerequisites.
rel=build/release
imgs="boot-stub.exec diag-cpu-1.ivsc diag-ctrl-psg-1.ivsc diag-stic-1.ivsc boot-stub-2.exec lamp-tone.ivsc"
if [ "$nogate" -eq 0 ]; then
  sh tools/gate.sh || fail "tools/gate.sh did not pass"
fi
[ -f "$rel/CMakeCache.txt" ] || refuse "$rel does not exist (run sh tools/gate.sh first or drop --no-gate)"
for i in $imgs; do
  [ -f "$rel/images/$i" ] || refuse "$rel/images/$i does not exist (run sh tools/gate.sh first or drop --no-gate)"
done

# (c) The assets.
ct=$(git log -1 --format=%ct) || fail "git log"
case $ct in '' | *[!0-9]*) fail "the commit time is not a number" ;; esac
os=$(uname -s | tr 'A-Z' 'a-z')
arch=$(uname -m | tr 'A-Z' 'a-z')
src_name=ivs-$tag-source.tar.gz
inst_dir=ivs-$tag-install-$os-$arch
inst_name=$inst_dir.tar.gz

git archive --format=tar --prefix="ivs-$tag/" HEAD > "$work/src.tar" || fail "git archive"
gzip -n -c "$work/src.tar" > "$out/$src_name" || fail "gzip of the source archive"

stage="$work/stage"
mkdir -p "$stage" || exit 2
if ! cmake --install "$rel" --prefix "$stage/$inst_dir" > "$work/install.log" 2>&1; then
  cat "$work/install.log"
  fail "cmake --install $rel"
fi
if tar --version 2> /dev/null | head -n 1 | grep -q 'GNU tar'; then
  tar_mode=gnu-tar
  (cd "$stage" && tar --sort=name --mtime="@$ct" --owner=0 --group=0 --numeric-owner -cf "$work/inst.tar" "$inst_dir") \
    || fail "tar of the install tree"
else
  tar_mode=cmake-tar
  iso=$(date -u -r "$ct" +"%Y-%m-%d %H:%M:%S UTC" 2> /dev/null || date -u -d "@$ct" +"%Y-%m-%d %H:%M:%S UTC" 2> /dev/null) \
    || fail "cannot format the commit time"
  (cd "$stage" && find "$inst_dir" ! -type d | sort > "$work/inst.members") || fail "listing the install tree"
  (cd "$stage" && cmake -E tar cf "$work/inst.tar" "--mtime=$iso" --format=gnutar "--files-from=$work/inst.members") \
    || fail "cmake -E tar of the install tree"
fi
printf 'release: install archive built with %s (members sorted, mtime of the commit)\n' "$tar_mode"
gzip -n -c "$work/inst.tar" > "$out/$inst_name" || fail "gzip of the install archive"

# (d) The content check of both archives, member by member.
git ls-files > "$work/src.list" || fail "git ls-files"
CHECK_DIR="$ROOT/tools/check"; export CHECK_DIR
sh tools/check/release-assets.sh source "$out/$src_name" "ivs-$tag" "$work/src.list" || fail "source archive content"
sh tools/check/release-assets.sh install "$out/$inst_name" "$inst_dir" data/expected/pkg-install-files.ids \
  || fail "install archive content"
unset CHECK_DIR

# (e) SHA256SUMS over the assets, sorted by name, LF-terminated.
printf '%s\n%s\n' "$inst_name" "$src_name" | sort > "$work/asset.names"
(
  cd "$out" || exit 1
  set --
  while IFS= read -r n; do set -- "$@" "$n"; done < "$work/asset.names"
  cmake -E sha256sum "$@"
) > "$work/SHA256SUMS" || fail "cmake -E sha256sum"
[ "$(grep -c . "$work/SHA256SUMS")" -eq 2 ] || fail "SHA256SUMS does not hold two lines"
cp "$work/SHA256SUMS" "$out/SHA256SUMS" || exit 2

# (f) The installed consumer against the very install archive that ships.
cons="$work/consumer"
mkdir -p "$cons/prefix" || exit 2
(cd "$cons/prefix" && tar -xzf "$out/$inst_name") || fail "extracting the install archive"
pfx="$cons/prefix/$inst_dir"
[ -d "$pfx" ] || fail "the install archive has no top directory $inst_dir"
# No path of this host inside the shipped install tree, in the logical or the physical form.
phys=$(cd "$ROOT" && pwd -P) || phys=$ROOT
if grep -r -a -l -F -e "$ROOT" -e "$phys" "$cons/prefix" > "$work/leak.out" 2> /dev/null; then
  fail "the install archive holds a path of this checkout: $(sed -n '1p' "$work/leak.out" | sed "s|$cons/prefix/||")"
fi
cp -R tests/installed-consumer "$cons/src" || exit 2
if ! (
  unset CMAKE_PREFIX_PATH CMAKE_MODULE_PATH CMAKE_INSTALL_PREFIX CMAKE_TOOLCHAIN_FILE ivs_DIR ivs_ROOT IVS_ROOT \
    CPATH C_INCLUDE_PATH CPLUS_INCLUDE_PATH OBJC_INCLUDE_PATH LIBRARY_PATH CFLAGS CPPFLAGS LDFLAGS \
    LD_LIBRARY_PATH DYLD_LIBRARY_PATH DYLD_FALLBACK_LIBRARY_PATH DYLD_FRAMEWORK_PATH DYLD_INSERT_LIBRARIES
  cmake -G Ninja -S "$cons/src" -B "$cons/build" -DCMAKE_PREFIX_PATH="$pfx" \
    -DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF -DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=OFF
) > "$work/consumer.cfg.log" 2>&1; then
  cat "$work/consumer.cfg.log"
  fail "configuring the consumer against the install archive"
fi
if ! cmake --build "$cons/build" > "$work/consumer.build.log" 2>&1; then
  cat "$work/consumer.build.log"
  fail "building the consumer against the install archive"
fi
set --
for i in $imgs; do set -- "$@" "$rel/images/$i"; done
cst=0
"$cons/build/consumer" "$@" > "$work/consumer.out" 2> "$work/consumer.err" || cst=$?
if [ "$cst" -ne 0 ]; then
  sed 's/^/# /' "$work/consumer.out" "$work/consumer.err"
  fail "the consumer exited $cst"
fi
grep -v '^#' data/expected/pkg-consumer.golden > "$work/golden.body"
if ! cmp -s "$work/golden.body" "$work/consumer.out"; then
  diff "$work/golden.body" "$work/consumer.out" | sed 's/^/# /' | head -n 20
  fail "the consumer output differs from data/expected/pkg-consumer.golden"
fi
printf 'release: consumer against %s: output equals data/expected/pkg-consumer.golden (%s lines)\n' \
  "$inst_name" "$(grep -c . "$work/consumer.out")"

printf 'release: assets in %s\n' "${out#"$ROOT"/}"
for n in "$src_name" "$inst_name" SHA256SUMS; do
  printf 'release:   %s %s bytes\n' "$n" "$(wc -c < "$out/$n" | tr -d ' ')"
done
sed 's/^/release:   sha256 /' "$out/SHA256SUMS"

# (g) Publish: the draft pre-release, only inside GitHub Actions, only after every check above.
if [ "$publish" -eq 1 ]; then
  gh release create "$tag" "$out/$src_name" "$out/$inst_name" "$out/SHA256SUMS" \
    --draft --prerelease --verify-tag --latest=false --title "$tag" --notes-file "$notes" \
    || fail "gh release create"
  printf 'release: draft pre-release %s created\n' "$tag"
else
  printf 'release: dry run complete, nothing was published\n'
fi
