#!/bin/sh
# SPDX-License-Identifier: MIT
# Step golden: goldens and the dev versus release judge (RUN-05; D-11, D-19, D-21, A-01; plan
# 04-21). A pure checker: it reads only FILELIST paths under data/ and the results dir that suite
# runner-process filled during c-tests (build/gate/<preset>/runner/<id>.report, plan 04-20), and
# writes nothing outside its temp dir. It never writes a golden: only `ivs-run --update-golden`,
# run by a person, does (docs/runner.md). Results dir: IVS_GATE_DIR (set by tools/gate.sh, and by
# the control runner to its tree's build/gate), else $ROOT/build/gate.
# Goldens: every FILELIST path data/golden/<name>.expect (predicates) or .digests (repeatability
# digests); any other file there is golden-mismatch (the single-name `.golden` was withdrawn, A-01).
# Rules, per golden:
#   - exactly one `# tier:` naming an id of data/expected/evidence-tiers.ids
#     (golden-header-missing-tier) and exactly one non-empty `# oracle:`
#     (golden-header-missing-oracle);
#   - oracle `self-recorded` needs tier `deterministic-repeatability`, and a .digests golden is
#     always self-recorded (golden-self-recorded-wrong-tier);
#   - exactly one `# scenario: <id>` naming a FILELIST file data/scenarios/<id>.ivss whose id is
#     in data/expected/runner-scenarios.ids; a `# profile:` line, when present, equals the
#     scenario's profile (golden-mismatch); a scenario of class `host-dependent` (column 4) has no
#     golden (golden-host-dependent: the wall watchdog and signals are never golden, D-21); a
#     scenario with `content <role> user` has no golden (golden-user-scenario-tracked);
#   - the dev report build/gate/dev/runner/<id>.report exists (golden-mismatch);
#   - .expect: its `assert` lines equal the scenario's `assert` lines in order, at least one, and
#     its body holds nothing else (golden-expect-drift);
#   - .digests: one `# domains:` line `cpu=observed mem=observed images=observed
#     instance=observed video=<v> audio=<v> input=<v>` with v `observed` or `none`
#     (golden-mismatch when missing or malformed); a live domain is observed exactly when its
#     report count (output.video.bytes, output.audio.bytes, input.events) is a decimal other than
#     0: an observed domain without content, or a digest line of a domain marked none, is
#     golden-empty-domain-observed (no hash of nothing is a recorded fact), a domain with content
#     marked none is golden-mismatch; the body (lines not starting with `#`) equals the dev
#     report filtered to the golden key list (golden-mismatch).
# Golden key list (the same list ivs-run --update-golden writes; docs/runner.md): scenario.id,
#   scenario.profile, outcome, outcome.detail, exit.code, slot.*, step.*, cpu.*, input.*,
#   digest.cpu, digest.mem, digest.images, digest.instance, digest.composite,
#   output.video.bytes, output.audio.bytes, and digest.video, digest.audio, digest.input for the
#   observed live domains, in report order.
# Dev versus release (Pitfall 11: one ctest run sees one tree): the union of report names in
#   build/gate/dev/runner and build/gate/release/runner; an id absent from the inventory is
#   golden-mismatch; an id of class `host-dependent` is skipped and counted; every other id needs
#   both reports, byte-equal once the `host.calls` lines are removed (golden-dev-release-diff). No
#   dev report at all is golden-mismatch (the producer did not run), never a vacuous pass.
# Tokens: golden-header-missing-tier, golden-header-missing-oracle,
#   golden-self-recorded-wrong-tier, golden-empty-domain-observed, golden-mismatch,
#   golden-expect-drift, golden-dev-release-diff, golden-user-scenario-tracked,
#   golden-host-dependent.
# Stats: goldens=<golden files> reports=<dev reports> host-dependent=<skipped report ids>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/golden.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
find="$tmp/findings"
: > "$find"
bad() { printf '%s %s\n' "$1" "$2" >> "$find"; }

gate=${IVS_GATE_DIR:-$ROOT/build/gate}
inv=data/expected/runner-scenarios.ids
tiers=data/expected/evidence-tiers.ids

# Inventory: `<id> <class>`; six columns, class deterministic or host-dependent.
: > "$tmp/inv"
if grep -q -x -F -e "$inv" "$FILELIST" && [ -f "$ROOT/$inv" ]; then
  awk -v f="$inv" '
    /^#/ || NF == 0 { next }
    NF != 6 || ($4 != "deterministic" && $4 != "host-dependent") {
      print "BAD " f " line " FNR; next
    }
    { print $1 " " $4 }' "$ROOT/$inv" > "$tmp/inv.raw"
  grep '^BAD ' "$tmp/inv.raw" | while IFS= read -r l; do printf 'golden-mismatch %s\n' "${l#BAD }"; done >> "$find"
  grep -v '^BAD ' "$tmp/inv.raw" | sort -u > "$tmp/inv"
else
  bad golden-mismatch "$inv missing"
fi
: > "$tmp/tiers"
if grep -q -x -F -e "$tiers" "$FILELIST" && [ -f "$ROOT/$tiers" ]; then
  awk '/^#/ || NF == 0 { next } { print $1 }' "$ROOT/$tiers" > "$tmp/tiers"
fi
[ -s "$tmp/tiers" ] || bad golden-header-missing-tier "$tiers missing or empty"

class_of() { awk -v id="$1" '$1 == id { print $2; exit }' "$tmp/inv"; }
in_list() { grep -q -x -F -e "$1" "$FILELIST"; }

# The golden key list, applied to a report on stdin; v, a and i are 1 for observed live domains.
filter_report() {
  awk -v v="$1" -v a="$2" -v i="$3" '
    $1 ~ /^(slot|step|cpu|input)\./ { print; next }
    $1 ~ /^(scenario\.id|scenario\.profile|outcome|outcome\.detail|exit\.code)$/ { print; next }
    $1 ~ /^digest\.(cpu|mem|images|instance|composite)$/ { print; next }
    $1 ~ /^output\.(video|audio)\.bytes$/ { print; next }
    ($1 == "digest.video" && v == 1) || ($1 == "digest.audio" && a == 1) || ($1 == "digest.input" && i == 1) { print }'
}

# A live count from a report: 1 when the key has a decimal value other than 0.
live_count() {
  awk -v k="$1" '$1 == k { v = $2; n++ } END { print (n == 1 && v ~ /^[0-9]+$/ && v !~ /^0+$/) ? 1 : 0 }' "$2"
}

goldens=0
grep -E '^data/golden/[^/]+$' "$FILELIST" > "$tmp/goldens" || :
while IFS= read -r g; do
  case $g in
    *.expect) kind=expect ;;
    *.digests) kind=digests ;;
    *) bad golden-mismatch "$g (not a .expect or .digests golden)"; continue ;;
  esac
  [ -f "$ROOT/$g" ] || { bad golden-mismatch "$g (unreadable)"; continue; }
  goldens=$((goldens + 1))
  # Header: findings as `F <token> <detail>`, values as `V <name> <value>`.
  awk -v g="$g" -v kind="$kind" -v tiers="$tmp/tiers" '
    BEGIN { while ((getline l < tiers) > 0) ok[l] = 1; close(tiers) }
    function trim(x) { sub(/^[ \t]+/, "", x); sub(/[ \t]+$/, "", x); return x }
    /^# tier:/ { nt++; t = trim(substr($0, 8)); next }
    /^# oracle:/ { no++; o = trim(substr($0, 10)); next }
    /^# scenario:/ { ns++; s = trim(substr($0, 12)); next }
    /^# profile:/ { np++; p = trim(substr($0, 11)); next }
    /^# domains:/ { nd++; d = trim(substr($0, 11)); next }
    END {
      tier_ok = (nt == 1 && (t in ok))
      oracle_ok = (no == 1 && o != "")
      if (!tier_ok) print "F golden-header-missing-tier " g
      if (!oracle_ok) print "F golden-header-missing-oracle " g
      if (tier_ok && oracle_ok && o == "self-recorded" && t != "deterministic-repeatability")
        print "F golden-self-recorded-wrong-tier " g " (oracle self-recorded, tier " t ")"
      if (kind == "digests" && oracle_ok && o != "self-recorded")
        print "F golden-self-recorded-wrong-tier " g " (a .digests golden is self-recorded, oracle " o ")"
      if (ns != 1 || s !~ /^[a-z0-9-]+$/) print "F golden-mismatch " g " (scenario header missing or malformed)"
      else print "V scenario " s
      if (np > 1 || (np == 1 && p !~ /^[a-z0-9-]+$/)) print "F golden-mismatch " g " (profile header malformed)"
      else if (np == 1) print "V profile " p
      if (kind == "digests") {
        live = "^cpu=observed mem=observed images=observed instance=observed video=(observed|none) audio=(observed|none) input=(observed|none)$"
        if (nd != 1 || d !~ live) print "F golden-mismatch " g " (domains header missing or malformed)"
        else {
          split(d, w, " ")
          print "V domains " substr(w[5], 7) " " substr(w[6], 7) " " substr(w[7], 7)
        }
      }
    }' "$ROOT/$g" > "$tmp/head"
  sed -n 's/^F //p' "$tmp/head" >> "$find"
  scn=$(sed -n 's/^V scenario //p' "$tmp/head")
  prof=$(sed -n 's/^V profile //p' "$tmp/head")
  doms=$(sed -n 's/^V domains //p' "$tmp/head")
  [ -n "$scn" ] || continue

  sf=data/scenarios/$scn.ivss
  if ! in_list "$sf" || [ ! -f "$ROOT/$sf" ]; then
    bad golden-mismatch "$g (scenario $sf missing)"
    continue
  fi
  cls=$(class_of "$scn")
  case $cls in
    deterministic) ;;
    host-dependent) bad golden-host-dependent "$g (scenario $scn is host-dependent: never golden)" ;;
    *) bad golden-mismatch "$g (scenario $scn not in $inv)" ;;
  esac
  if awk '$1 == "content" && $3 == "user" { f = 1 } END { exit !f }' "$ROOT/$sf"; then
    bad golden-user-scenario-tracked "$g (scenario $scn has user content)"
  fi
  if [ -n "$prof" ]; then
    sprof=$(awk '$1 == "profile" { print $2; exit }' "$ROOT/$sf")
    [ "$prof" = "$sprof" ] || bad golden-mismatch "$g (profile $prof, scenario profile $sprof)"
  fi

  if [ "$kind" = expect ]; then
    awk '$1 == "assert" { $1 = $1; print }' "$ROOT/$sf" > "$tmp/want"
    awk '/^#/ || NF == 0 { next } $1 == "assert" { $1 = $1; print; next } { print "STRAY " $0 }' \
      "$ROOT/$g" > "$tmp/have"
    if [ ! -s "$tmp/want" ] || ! cmp -s "$tmp/want" "$tmp/have"; then
      bad golden-expect-drift "$g (assert lines differ from $sf)"
    fi
  fi

  rep="$gate/dev/runner/$scn.report"
  if [ ! -f "$rep" ]; then
    bad golden-mismatch "$g (no report build/gate/dev/runner/$scn.report)"
    continue
  fi
  [ "$kind" = digests ] || continue
  [ -n "$doms" ] || continue
  # shellcheck disable=SC2086
  set -- $doms
  hv=$1 ha=$2 hi=$3
  lv=$(live_count output.video.bytes "$rep")
  la=$(live_count output.audio.bytes "$rep")
  li=$(live_count input.events "$rep")
  for d in video:$hv:$lv audio:$ha:$la input:$hi:$li; do
    name=${d%%:*}; rest=${d#*:}; mark=${rest%%:*}; have=${rest#*:}
    if [ "$mark" = observed ] && [ "$have" -eq 0 ]; then
      bad golden-empty-domain-observed "$g ($name marked observed, the report has no $name content)"
    fi
    if [ "$mark" = none ] && grep -q "^digest\.$name " "$ROOT/$g"; then
      bad golden-empty-domain-observed "$g (digest.$name recorded for a domain marked none)"
    fi
    if [ "$mark" = none ] && [ "$have" -eq 1 ]; then
      bad golden-mismatch "$g ($name has content but is marked none)"
    fi
  done
  filter_report "$lv" "$la" "$li" < "$rep" > "$tmp/want"
  awk '/^#/ { next } { print }' "$ROOT/$g" > "$tmp/have"
  cmp -s "$tmp/want" "$tmp/have" || bad golden-mismatch "$g (body differs from build/gate/dev/runner/$scn.report)"
done < "$tmp/goldens"

# Dev versus release.
: > "$tmp/ids"
reports=0
for p in dev release; do
  for f in "$gate/$p/runner/"*.report; do
    [ -f "$f" ] || continue
    [ "$p" = dev ] && reports=$((reports + 1))
    n=${f##*/}
    printf '%s\n' "${n%.report}" >> "$tmp/ids"
  done
done
sort -u "$tmp/ids" -o "$tmp/ids"
[ "$reports" -gt 0 ] || bad golden-mismatch "no report in build/gate/dev/runner (suite runner-process did not run)"
hostdep=0
while IFS= read -r id; do
  cls=$(class_of "$id")
  case $cls in
    host-dependent) hostdep=$((hostdep + 1)); continue ;;
    deterministic) ;;
    *) bad golden-mismatch "report $id not in $inv"; continue ;;
  esac
  dv="$gate/dev/runner/$id.report"
  rl="$gate/release/runner/$id.report"
  if [ ! -f "$dv" ] || [ ! -f "$rl" ]; then
    bad golden-dev-release-diff "$id (a report of one preset is missing)"
    continue
  fi
  awk '$1 != "host.calls"' "$dv" > "$tmp/dv"
  awk '$1 != "host.calls"' "$rl" > "$tmp/rl"
  cmp -s "$tmp/dv" "$tmp/rl" || bad golden-dev-release-diff "$id (dev and release reports differ apart from host.calls)"
done < "$tmp/ids"

if [ -s "$find" ]; then
  sort -u "$find" | while IFS= read -r l; do
    printf -- '-- FAIL %s: %s\n' "${l%% *}" "${l#* }"
  done
  exit 1
fi
printf -- '-- golden pass goldens=%s reports=%s host-dependent=%s\n' "$goldens" "$reports" "$hostdep"
