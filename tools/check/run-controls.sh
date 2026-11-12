#!/bin/sh
# SPDX-License-Identifier: MIT
# Negative-control runner (SKELETON.md section 3).
# Each control dir holds CHECK (step name), EXPECT (one token per line), optional
# FORBID, overlay/ (copied over a fresh copy of _good/tree) and MAKE (transform lines).
# Env: CONTROLS_DIR (default tools/check/controls), CONTROLS_INVENTORY_DIR
# (default data/expected/controls). Final line: controls=<ok>/<total>.
# Controls run in JOBS lanes (D-22 lever 2, A-09): each control gets its own work dir
# (tree, filelist, step output, result lines), so lanes share no temp state. Lane i takes
# every JOBS-th entry of the sorted list; results are printed afterwards in sorted order,
# so the output does not depend on scheduling.
# CONTROLS_END_FILE (optional): the runner writes its end time (date +%s) there; it is not
# passed on to the steps. On HUP, INT or TERM the runner stops and terminates every process
# below it (lanes, their steps and the compilers those start) before it exits, so an
# interrupted gate leaves no orphans (review WR-04).
LC_ALL=C; export LC_ALL
end_file=${CONTROLS_END_FILE:-}
unset CONTROLS_END_FILE
here=$(cd "$(dirname "$0")" && pwd) || exit 1
CHECK_DIR=$here
ROOT=$(cd "$here/../.." && pwd) || exit 1
CONTROLS_DIR=${CONTROLS_DIR:-$CHECK_DIR/controls}
CONTROLS_INVENTORY_DIR=${CONTROLS_INVENTORY_DIR:-$ROOT/data/expected/controls}
export CHECK_DIR
tmp=$(mktemp -d "${TMPDIR:-/tmp}/ctl.XXXXXX") || exit 1

# Every process below this runner, from one POSIX `ps -A -o pid= -o ppid=` snapshot. A lane is
# an asynchronous list, so it and everything it starts ignore SIGINT, and a lane killed alone
# would leave its step running under init: the whole tree is collected first, stopped so it
# cannot grow, collected again (three rounds catch a child forked during a snapshot), and only
# then terminated and continued.
descendants() {
  ps -A -o pid= -o ppid= 2> /dev/null | awk -v root="$$" '
    { par[$1] = $2; ids[NR] = $1 }
    END {
      want[root] = 1
      more = 1
      while (more) {
        more = 0
        for (i = 1; i <= NR; i++) {
          p = ids[i]
          if (!(p in want) && (par[p] in want)) { want[p] = 1; more = 1 }
        }
      }
      for (p in want) if (p != root) print p
    }'
}
stop_tree() {
  _all=
  for _round in 1 2 3; do
    _pids=$(descendants)
    [ -n "$_pids" ] || continue
    # shellcheck disable=SC2086
    kill -s STOP $_pids 2> /dev/null
    _all="$_all $_pids"
  done
  if [ -n "$_all" ]; then
    # shellcheck disable=SC2086
    kill -s TERM $_all 2> /dev/null
    # shellcheck disable=SC2086
    kill -s CONT $_all 2> /dev/null
  fi
  wait
}
trap 'rm -rf "$tmp"' EXIT
trap 'stop_tree; exit 1' HUP INT TERM
JOBS=4
bad=0
total=0
ok=0
fail() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; bad=1; }

# Apply MAKE lines inside tree $1 from file $2. Bad bytes exist only here, assembled at run time.
apply_make() {
  _t=$1
  while IFS= read -r _line || [ -n "$_line" ]; do
    set -f
    # shellcheck disable=SC2086
    set -- $_line
    set +f
    [ "$#" -gt 0 ] || continue
    case $1 in '#'*) continue ;; esac
    _verb=$1
    _p=${2:-}
    case $_p in
      /*|*..*) fail control-make-path "$_p"; return 1 ;;
    esac
    case $_verb in
      delete) rm -f "$_t/$_p" ;;
      empty) : > "$_t/$_p" ;;
      write) mkdir -p "$(dirname "$_t/$_p")"; printf '%s\n' "${3:-x}" > "$_t/$_p" ;;
      rename)
        case ${3:-} in /*|*..*) fail control-make-path "${3:-}"; return 1 ;; esac
        mkdir -p "$(dirname "$_t/${3:-}")"; mv "$_t/$_p" "$_t/${3:-}" ;;
      crlf) awk '{ printf "%s\r\n", $0 }' "$_t/$_p" > "$_t/$_p.tmp" && mv "$_t/$_p.tmp" "$_t/$_p" ;;
      bom) { printf '\357\273\277'; cat "$_t/$_p"; } > "$_t/$_p.tmp" && mv "$_t/$_p.tmp" "$_t/$_p" ;;
      nonascii) printf '\303\251' >> "$_t/$_p" ;;
      homepath) printf '/%s/%s/x\n' Users ctlexample >> "$_t/$_p" ;;
      secret) printf '%s%s_%s\n' gh p abcdefghijklmnopqrstuvwxyz0123456789 >> "$_t/$_p" ;;
      password) printf '%s%s=%s\n' pass word Abcdefgh1234 >> "$_t/$_p" ;;
      binary) mkdir -p "$(dirname "$_t/$_p")"; awk 'BEGIN { for (i = 0; i < 2048; i++) print i }' \
        | tr '0123456789\n' '\200\201\202\203\204\205\206\207\210\211\000' > "$_t/$_p" ;;
      highbytes) mkdir -p "$(dirname "$_t/$_p")"; awk 'BEGIN { for (i = 0; i < 2048; i++) print i }' \
        | tr '0123456789\n' '\200\201\202\203\204\205\206\207\210\211\212' > "$_t/$_p" ;;
      nul) printf 'a\000b\n' >> "$_t/$_p" ;;
      scratchpath)
        # Appends one scratch, temp or tilde path of the kind named by $3, assembled from split pieces.
        case ${3:-} in
          private-tmp) printf '/%s/%s/%s/x\n' private tmp ctlexample >> "$_t/$_p" ;;
          var-folders) printf '/%s/%s/ab/x\n' var folders >> "$_t/$_p" ;;
          tmp-claude) printf '/%s/%s-501/x\n' tmp claude >> "$_t/$_p" ;;
          root-home) printf '/%s/%s/x\n' root .ssh >> "$_t/$_p" ;;
          tilde) printf '~/%s/x\n' projects >> "$_t/$_p" ;;
          *) fail control-make-verb "scratchpath ${3:-}"; return 1 ;;
        esac ;;
      credential)
        # Appends one line of the credential shape named by $3, assembled from split pieces.
        case ${3:-} in
          json-password) printf '{"%s%s": "%s"}\n' pass word Abcdefgh1234 >> "$_t/$_p" ;;
          api-key) printf '%s_%s = "%s"\n' api key abcdef1234567890abcdef >> "$_t/$_p" ;;
          slack) printf '%s%s-%s\n' xo xb 1234567890abcdef >> "$_t/$_p" ;;
          anthropic) printf '%s-%s-%s\n' sk ant api03abcdefghijklmnopqrstuvwxyz >> "$_t/$_p" ;;
          google) printf '%s%s%s\n' AI za 0123456789abcdefghijABCDEFGHIJ-_123 >> "$_t/$_p" ;;
          *) fail control-make-verb "credential ${3:-}"; return 1 ;;
        esac ;;
      email) printf '%s@%s\n' ctl mail.test >> "$_t/$_p" ;;
      *) fail control-make-verb "$_verb"; return 1 ;;
    esac
  done < "$2"
  return 0
}

# Build the tree $1/tree and its list $1/filelist in work dir $1 for control dir $2
# (empty $2 = pristine good tree). The work dir is new, so the good tree is copied as tree in
# one cp (D-45 lever, plan 04-LEVER-01: no rm and mkdir per control); a full copy, never links,
# because MAKE verbs append to tree files in place.
build_tree() {
  if [ -e "$1/tree" ]; then rm -rf "$1/tree"; fi
  cp -R "$CONTROLS_DIR/_good/tree" "$1/tree" || return 1
  if [ -n "$2" ]; then
    if [ -d "$2/overlay" ]; then cp -R "$2/overlay/." "$1/tree/" || return 1; fi
    if [ -f "$2/MAKE" ]; then apply_make "$1/tree" "$2/MAKE" || return 1; fi
  fi
  (cd "$1/tree" && find . -type f | sed 's|^\./||' | sort) > "$1/filelist"
  return 0
}

# Run step $2 on the tree of work dir $1; its output goes to $1/out. The results dir is the
# tree's build/gate, never the one the gate exported for the real steps (review IN-07).
run_step() {
  ROOT="$1/tree" FILELIST="$1/filelist" CHECK_DIR="$CHECK_DIR" IVS_GATE_DIR="$1/tree/build/gate" \
    sh "$CHECK_DIR/steps/$2.sh" > "$1/out" 2>&1
}

# Positive control for step $1 in work dir $2: prints its result lines, touches $2/ok on success.
good_one() {
  build_tree "$2" "" || { fail control-good-failed "$1"; return 0; }
  if run_step "$2" "$1" && grep -q -F -e "-- $1 pass" "$2/out"; then
    printf 'control ok   _good/%s\n' "$1"
    : > "$2/ok"
  else
    fail control-good-failed "$1"
    cat "$2/out"
  fi
}

# Negative control $1 in work dir $2: prints its result lines, touches $2/ok when it is ok.
ctl_one() {
  _d="$CONTROLS_DIR/$1"
  _step=
  if [ -f "$_d/CHECK" ]; then IFS= read -r _step < "$_d/CHECK" || :; fi
  if [ -z "$_step" ] || [ ! -f "$CHECK_DIR/steps/$_step.sh" ]; then
    fail control-unknown-step "$1"; return 0
  fi
  # A blank EXPECT line (only whitespace) would match every output. Checked with shell builtins,
  # line by line as grep '^[[:space:]]*$' did (D-45 lever, plan 04-LEVER-01).
  _blank=0
  if [ -s "$_d/EXPECT" ]; then
    while IFS= read -r _tok || [ -n "$_tok" ]; do
      case $_tok in *[![:space:]]*) ;; *) _blank=1 ;; esac
    done < "$_d/EXPECT"
  else
    _blank=1
  fi
  if [ "$_blank" -ne 0 ]; then
    fail control-expect-blank "$1"; return 0
  fi
  build_tree "$2" "$_d" || { fail control-build "$1"; return 0; }
  if run_step "$2" "$_step"; then
    fail control-exit-zero "$1"; return 0
  fi
  # Tokens are fixed strings matched within an output line (as grep -F): every EXPECT line must
  # occur and no non-empty FORBID line may. One awk pass over the output (D-45 lever, plan
  # 04-LEVER-01) replaces one grep per token; an awk failure counts as a missing token.
  _ff=
  if [ -f "$_d/FORBID" ]; then _ff="$_d/FORBID"; fi
  _v=$(awk -v ef="$_d/EXPECT" -v ff="$_ff" '
    BEGIN {
      ne = 0; nf = 0; fhit = 0
      while ((getline t < ef) > 0) e[++ne] = t
      close(ef)
      if (ff != "") {
        while ((getline t < ff) > 0) if (t != "") f[++nf] = t
        close(ff)
      }
    }
    {
      for (i = 1; i <= ne; i++) if (!(i in ehit) && index($0, e[i]) > 0) ehit[i] = 1
      for (i = 1; i <= nf; i++) if (index($0, f[i]) > 0) fhit = 1
    }
    END {
      for (i = 1; i <= ne; i++) if (!(i in ehit)) { print "missing"; exit }
      print (fhit ? "forbidden" : "clean")
    }' "$2/out") || _v=missing
  case $_v in
    clean) ;;
    forbidden) fail control-forbidden-token "$1"; return 0 ;;
    *) fail control-token-missing "$1"; return 0 ;;
  esac
  printf 'control ok   %s\n' "$1"
  : > "$2/ok"
}

# Run function $2 for every entry of list file $1 in JOBS lanes; entry e gets work dir
# $tmp/$2/e and its result lines go to $tmp/$2/e/result. Returns after every lane ends.
run_lanes() {
  _list=$1
  _fn=$2
  # Every work dir in one mkdir before the lanes start (D-45 lever, plan 04-LEVER-01).
  set --
  while IFS= read -r _e; do set -- "$@" "$tmp/$_fn/$_e"; done < "$_list"
  if [ "$#" -gt 0 ]; then mkdir -p "$@"; fi
  _lane=0
  while [ "$_lane" -lt "$JOBS" ]; do
    (
      _k=0
      while IFS= read -r _e; do
        if [ $((_k % JOBS)) -eq "$_lane" ]; then
          "$_fn" "$_e" "$tmp/$_fn/$_e" > "$tmp/$_fn/$_e/result" 2>&1 < /dev/null
        fi
        _k=$((_k + 1))
      done < "$_list"
    ) &
    _lane=$((_lane + 1))
  done
  wait
}

# Control directories and inventory.
: > "$tmp/ctl.have"
for d in "$CONTROLS_DIR"/*/; do
  [ -d "$d" ] || continue
  n=${d%/}
  n=${n##*/}
  [ "$n" = _good ] && continue
  printf '%s\n' "$n" >> "$tmp/ctl.have"
done
sort -u "$tmp/ctl.have" -o "$tmp/ctl.have"
: > "$tmp/ctl.want"
for f in "$CONTROLS_INVENTORY_DIR"/*.ids; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } NF { print $1 }' "$f" >> "$tmp/ctl.want"
done
sort -u "$tmp/ctl.want" -o "$tmp/ctl.want"
if [ ! -s "$tmp/ctl.want" ]; then fail control-inventory-empty "inventory"; fi
if [ ! -s "$tmp/ctl.have" ]; then fail control-none "no control directories"; fi
for n in $(comm -13 "$tmp/ctl.want" "$tmp/ctl.have"); do fail control-not-in-inventory "$n"; done
for n in $(comm -23 "$tmp/ctl.want" "$tmp/ctl.have"); do fail control-inventory-orphan "$n"; done

# Positive control: every distinct CHECK step must pass on the pristine good tree.
: > "$tmp/checks"
for n in $(cat "$tmp/ctl.have"); do
  _c=
  if [ -f "$CONTROLS_DIR/$n/CHECK" ] && { IFS= read -r _c < "$CONTROLS_DIR/$n/CHECK" || [ -n "$_c" ]; }; then
    printf '%s\n' "$_c" >> "$tmp/checks"
  fi
done
sort -u "$tmp/checks" -o "$tmp/checks"
: > "$tmp/goodsteps"
for step in $(cat "$tmp/checks"); do
  if [ -f "$CHECK_DIR/steps/$step.sh" ]; then printf '%s\n' "$step" >> "$tmp/goodsteps"; fi
done
# Result files are printed in list order: in one cat when every one exists (D-45 lever, plan
# 04-LEVER-01; the same bytes as one cat per file), else one by one, naming each missing file.
run_lanes "$tmp/goodsteps" good_one
set --
each=0
for step in $(cat "$tmp/goodsteps"); do
  if [ -f "$tmp/good_one/$step/result" ]; then set -- "$@" "$tmp/good_one/$step/result"; else each=1; fi
  [ -f "$tmp/good_one/$step/ok" ] || bad=1
done
if [ "$each" -eq 0 ]; then
  if [ "$#" -gt 0 ]; then cat "$@" 2> /dev/null || fail control-good-failed results; fi
else
  for step in $(cat "$tmp/goodsteps"); do
    cat "$tmp/good_one/$step/result" 2> /dev/null || printf -- '-- FAIL %s: %s\n' control-good-failed "$step"
  done
fi

# Negative controls, then their results in sorted order.
run_lanes "$tmp/ctl.have" ctl_one
set --
each=0
for n in $(cat "$tmp/ctl.have"); do
  total=$((total + 1))
  if [ -f "$tmp/ctl_one/$n/result" ]; then set -- "$@" "$tmp/ctl_one/$n/result"; else each=1; fi
  if [ -f "$tmp/ctl_one/$n/ok" ]; then ok=$((ok + 1)); else bad=1; fi
done
if [ "$each" -eq 0 ]; then
  if [ "$#" -gt 0 ]; then cat "$@" 2> /dev/null || fail control-build results; fi
else
  for n in $(cat "$tmp/ctl.have"); do
    cat "$tmp/ctl_one/$n/result" 2> /dev/null || printf -- '-- FAIL %s: %s\n' control-build "$n"
  done
fi

printf 'controls=%s/%s\n' "$ok" "$total"
if [ -n "$end_file" ]; then date +%s > "$end_file"; fi
[ "$bad" -eq 0 ]
