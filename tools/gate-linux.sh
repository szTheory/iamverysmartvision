#!/bin/sh
# SPDX-License-Identifier: MIT
# Local pre-push helper, not a gate step and not run in CI: runs `sh tools/gate.sh` on the
# current working tree inside a throwaway Ubuntu 24.04 container (gcc 13, GNU tools, dash as
# /bin/sh, cmake 3.28), the toolchain of the hosted runner, so a hosted-only failure shows up
# before a CI round trip.
#
#   sh tools/gate-linux.sh
#
# The tree sent is `git ls-files -co --exclude-standard` (tracked plus untracked-not-ignored,
# the gate's own file list), streamed into the container as a tar archive; nothing is mounted
# and nothing in the working tree is written except the log. Inside, the helper installs the
# packages, makes the copy a git work tree (git init, git add) and runs the gate.
# Prints every `-- FAIL` line, the gate-time line and the summary line, then
# `gate-linux: exit <status>`; the full log is build/gate-linux.log (build/ is ignored).
# Exit: the gate's exit status; 2 when docker is missing or the tree cannot be listed.
# Env: GATE_LINUX_IMAGE (default ubuntu:24.04), GATE_LINUX_PLATFORM (default linux/amd64, the
#   hosted runner's architecture; empty means the docker default). On an arm64 host amd64 runs
#   emulated and the gate takes several minutes.
LC_ALL=C; export LC_ALL
here=$(cd "$(dirname "$0")" && pwd) || exit 2
ROOT=$(cd "$here/.." && pwd) || exit 2
image=${GATE_LINUX_IMAGE:-ubuntu:24.04}
platform=${GATE_LINUX_PLATFORM-linux/amd64}
command -v docker > /dev/null 2>&1 || { printf 'gate-linux: docker not found\n' >&2; exit 2; }
cd "$ROOT" || exit 2
mkdir -p build || exit 2
log=build/gate-linux.log
list=build/gate-linux.files
if ! git -c core.quotePath=false ls-files -co --exclude-standard > "$list.raw"; then
  printf 'gate-linux: git ls-files failed\n' >&2
  exit 2
fi
# Keep the paths that exist (a deleted, not yet staged file is listed but cannot be sent).
while IFS= read -r f; do
  [ -e "$f" ] && printf '%s\n' "$f"
done < "$list.raw" > "$list"
rm -f "$list.raw"
set -- run --rm -i
[ -n "$platform" ] && set -- "$@" --platform "$platform"
# The container script: unpack the tree from stdin, install the toolchain, run the gate.
inner='set -u
mkdir -p /w && cd /w && tar -xf - || exit 2
export DEBIAN_FRONTEND=noninteractive
{ apt-get update -q && apt-get install -y -q \
    cmake ninja-build gcc g++ dash git ca-certificates python3 coreutils; } > /apt.log 2>&1 \
  || { tail -n 20 /apt.log; echo "gate-linux: apt-get failed"; exit 2; }
git init -q && git add -A || exit 2
echo "gate-linux: $(uname -m), $(gcc --version | head -n 1), $(cmake --version | head -n 1)"
sh tools/gate.sh'
# Paths of the list are relative to ROOT; tar reads them from the list file. COPYFILE_DISABLE
# keeps macOS tar from adding AppleDouble ._ entries.
COPYFILE_DISABLE=1; export COPYFILE_DISABLE
st=0
tar -cf - -T "$list" | docker "$@" "$image" sh -c "$inner" > "$log" 2>&1 || st=$?
grep -e '^gate-linux: ' -e '^-- FAIL ' -e '^gate-time ' -e '^gate: ' "$log"
printf 'gate-linux: exit %s (log %s)\n' "$st" "$log"
exit "$st"
