#!/bin/sh
# SPDX-License-Identifier: MIT
# Operator listening helper (plan 05-10, D-21, D-22). NOT a gate step: nothing in the gate runs it except the
# one refusal test (runner-process.listen-demo.refuses-user). It renders one scenario to a mono 16-bit WAV with
# the runner's --audio-wav (the reference converter of tools/pcm, "reference conversion v1, converter
# repeatability only") into the gitignored build tree and prints the WAV size and the report's refpcm.sha256,
# so an owner can listen and record the hash. The helper proves the EXPORT PATH (an exclusively created file,
# its size, its SHA-256) and nothing else: the result is never a claim, never hardware evidence, never a
# digest of the raw audio evidence.
# The default scenario diag-ctrl-psg-pass (plan 05-11) is a diagnostic that writes its tone period late, and a
# period write takes effect only after the 4096-step first reload (about 65,536 ticks) while the program halts
# near tick 16,750 (plan 05-11 measured 16,752), so its WAV is about 5 ms of near-silence: there is no listenable tone in any Phase 5
# artifact (a longer guest is a later phase), and the D-22 listening handoff is recorded as not performable.
# Rights: a scenario whose text has a `content <role> user` line (the operator's own image) is REFUSED before
# anything is built: derived audio of rights-bearing content stays out of shared records.
# Usage: sh tools/listen-demo.sh [scenario-id]      (default diag-ctrl-psg-pass; data/scenarios/<id>.ivss)
# Env: none read (a fixed locale, no path from the environment).
# Output: build/listen/<id>.wav (a previous file of that name is removed first: the runner creates it
#   exclusively) and build/listen/<id>.report; on stdout the scenario id, the WAV path and size and refpcm.sha256.
# Exit: 0 rendered; 2 a refusal (a `content ... user` line), an unknown or malformed scenario id, or a missing
#   tool; 1 the build or the run failed (the runner's outcome is then not pass).
LC_ALL=C; export LC_ALL
root=$(cd "$(dirname "$0")/.." 2> /dev/null && pwd) || { printf 'listen-demo: cannot find the repository root\n' >&2; exit 2; }
id=${1:-diag-ctrl-psg-pass}
[ "$#" -le 1 ] || { printf 'usage: sh tools/listen-demo.sh [scenario-id]\n' >&2; exit 2; }
case $id in
  ''|*[!a-z0-9-]*) printf 'listen-demo: scenario id must be [a-z0-9-]+\n' >&2; exit 2 ;;
esac
scn="$root/data/scenarios/$id.ivss"
[ -f "$scn" ] || { printf 'listen-demo: no scenario data/scenarios/%s.ivss\n' "$id" >&2; exit 2; }
for t in awk sed cmake; do
  command -v "$t" > /dev/null 2>&1 || { printf 'listen-demo: missing tool: %s\n' "$t" >&2; exit 2; }
done
# The rights rule, before any build: a `content <role> user` line (comments are not directives).
if awk '$1 == "content" && $3 == "user" { f = 1 } END { exit !f }' "$scn"; then
  printf 'listen-demo: refusing %s: it has a `content ... user` line; derived audio of rights-bearing content stays out of shared records\n' "$id" >&2
  exit 2
fi
cd "$root" || exit 2
cmake --preset release > /dev/null 2>&1 || { printf 'listen-demo: cmake --preset release failed\n' >&2; exit 1; }
cmake --build --preset release --target ivs_run ivs_images > /dev/null 2>&1 || { printf 'listen-demo: the release build failed\n' >&2; exit 1; }
out=build/listen
mkdir -p "$out" || exit 1
rm -f "$out/$id.wav" "$out/$id.report"
rc=0
build/release/runner/ivs-run --content-dir build/release/images --audio-wav "$out/$id.wav" "$scn" > "$out/$id.report" || rc=$?
if [ "$rc" -ne 0 ]; then
  printf 'listen-demo: ivs-run ended with exit %s (see %s/%s.report); the WAV is not a result\n' "$rc" "$out" "$id" >&2
  exit 1
fi
size=$(wc -c < "$out/$id.wav" | tr -d ' ')
sha=$(sed -n 's/^refpcm\.sha256 //p' "$out/$id.report")
printf 'scenario %s\nwav %s/%s.wav (%s bytes)\nrefpcm.sha256 %s\n' "$id" "$out" "$id" "$size" "$sha"
printf 'reference conversion v1, converter repeatability only: export path check, not a claim\n'
