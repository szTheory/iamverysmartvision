#!/bin/sh
# SPDX-License-Identifier: MIT
# Step state (plan 08-09, D-38, D-44): the pure checks of the hidden-state oracle of the snapshot work. No C is built: the step reads the TEXT of the
# digest field table (core/src/machine/machine_digest.c) and the data files, and holds them together.
#   Inputs (all under ROOT): the table source above, data/state/schema.literal.txt (the hand-derived canonical descriptor text, the preimage of the
#   schema digest), data/expected/state-checklist.ids (one line per row: row class witness test sweep), data/expected/state-cuts.ids (the named
#   hard-point cuts), data/state/tests-pending.ids (test ids that no suite registers yet, with the owner plan), data/expected/ivsn-literals.ids (the
#   pinned schema digest) and data/expected/tests/*.ids (the registered test ids). The constants that the table text names (counts such as the
#   register-file sizes, the domain ids) are read as object-like define lines of the table source and of every FILELIST header under core/src and
#   core/include, outside the branches of a fault-conditional ifdef; a product or sum of numbers and such names is evaluated, anything else is an
#   unresolved token.
#   Table parse: every IVS_DF call (six arguments: name, member, width, count, domain, kind; a seventh argument is the class token) and every use of
#   the slot macro (its body is read from the macro definition, so the suffix list is never copied here) gives one row in table order with its
#   width, count, domain number, kind and class. The class tokens of the source are IVS_SNAP_STATE, IVS_SNAP_IMAGE, IVS_SNAP_BOUND and IVS_SNAP_PAD; the
#   literal 0u (until plan 08-10 puts classes into the source) means that the source carries no class and the source comparison is skipped.
#   Checks and tokens (each finding is one `-- FAIL <token>: <detail>` line, in sorted order):
#     state-input-missing: <path>              an input file is absent
#     state-literals-missing: <what>           data/expected/ivsn-literals.ids is absent or has no schema-digest-sha256 line
#     state-table-parse: <detail>              the table source cannot be parsed (no table, unbalanced call, a name that is not a string literal)
#     state-table-unresolved: <row> <expr>     a width, count or domain that does not evaluate
#     state-table-dup: <row>                   a row name twice in the table
#     state-schema-form, state-checklist-form, state-cut-form, state-pending-form: <detail>   a malformed line of that file
#     state-row-missing: <row> (checklist|schema)    a table row with no line of that file
#     state-line-extra: <row> (checklist|schema)     a line of that file that names no table row
#     state-line-dup: <row> (checklist|schema)       a row named twice in that file
#     state-schema-order: <row>                the schema literal lists the rows in another order than the table (it is hashed whole)
#     state-kind-drift, state-domain-drift, state-shape-drift: <row> (...)   a schema line against the table (kind; domain; width and count)
#     state-class-unknown: <row> <token>       a class that is none of STATE, IMAGE_BEARING, BOUND, PAD (checklist, schema or source)
#     state-class-drift: <row> (...)           the checklist class against the schema class, and (when the source carries one) the source class
#     state-class-kind: <row>                  a PAD kind row that is not class PAD, a class PAD row that is not a PAD kind, a MAGIC or PROFILE row that is not BOUND
#     state-witness-unknown: <row> <witness>   a STATE row whose witness is not a cut of state-cuts.ids
#     state-nonstate-witness: <row>            a row of another class that has a witness or a sweep
#     state-cut-unused: <cut>                  a cut that is the witness of no row
#     state-cut-dup: <cut>                     a cut id twice
#     state-test-unknown: <row> <test>         a test id that is neither registered in data/expected/tests/*.ids nor a pending line
#     state-pending-owner: <id> <owner>        an owner outside 08-11 to 08-17
#     state-pending-stale: <id>                a pending id that is also registered (the owner plan deletes its lines when it registers)
#     state-exempt-no-reason: <row>            an exemption without a reason of at least three words (joined by underscores)
#     state-sweep-unknown: <row> <value>       a sweep column that is neither ROW nor an exemption
#     state-schema-stale: <pinned> <actual>    the SHA-256 of data/state/schema.literal.txt (cmake -E sha256sum) differs from the pin
# Reads ROOT, CHECK_DIR and FILELIST only; writes only to its temp dir.
# Stats: state-rows=<table rows> state-cuts=<cuts>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/state.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
find="$tmp/find"
: > "$find"
fail() { printf -- '-- FAIL %s: %s\n' "$1" "$2" >> "$find"; }

TABLE=core/src/machine/machine_digest.c
SCHEMA=data/state/schema.literal.txt
CHECK=data/expected/state-checklist.ids
CUTS=data/expected/state-cuts.ids
PEND=data/state/tests-pending.ids
LIT=data/expected/ivsn-literals.ids

missing=0
for f in "$TABLE" "$SCHEMA" "$CHECK" "$CUTS" "$PEND"; do
  if [ ! -f "$ROOT/$f" ]; then
    fail state-input-missing "$f"
    missing=1
  fi
done
if [ ! -f "$ROOT/$LIT" ]; then
  fail state-literals-missing "$LIT"
  missing=1
fi
if [ "$missing" -ne 0 ]; then
  sort -u "$find"
  exit 1
fi

# Constants: object-like define lines of the table source and of every header under core/src and core/include, outside the branches of an
# fault-conditional ifdef (the else branch counts). Output: <name> TAB <body>.
{ printf '%s\n' "$TABLE"; grep -E '^core/(src|include)/.*[.]h$' "$FILELIST" | sort -u; } > "$tmp/hdrs"
set --
while IFS= read -r f; do set -- "$@" "$f"; done < "$tmp/hdrs"
(cd "$ROOT" && awk '
FNR == 1 { skip = 0 }
{
  line = $0
  if (line ~ /^[ \t]*#[ \t]*ifdef[ \t]+IVS_FAULT_/) { skip = 1; next }
  if (line ~ /^[ \t]*#[ \t]*(else|endif)/) { skip = 0; next }
  if (skip) next
  if (line !~ /^[ \t]*#[ \t]*define[ \t]+[A-Za-z_][A-Za-z0-9_]*[ \t]+[^ \t]/) next
  sub(/^[ \t]*#[ \t]*define[ \t]+/, "", line)
  name = line
  sub(/[ \t].*$/, "", name)
  body = substr(line, length(name) + 1)
  sub(/\/\*.*$/, "", body)
  sub(/^[ \t]+/, "", body)
  sub(/[ \t]+$/, "", body)
  if (body ~ /^[A-Za-z0-9_ \t*+()]+$/) printf "%s\t%s\n", name, body
}' "$@") > "$tmp/consts"

# Table parse: rows in table order, `<name> <width> <count> <domain> <kind> <class>`.
: > "$tmp/table"
awk -v consts="$tmp/consts" -v findfile="$find" -v rowsfile="$tmp/table" '
function trim(s) { sub(/^[ \t]+/, "", s); sub(/[ \t]+$/, "", s); return s }
function perr(m) { printf "-- FAIL state-table-parse: %s\n", m >> findfile }
function unres(nm, ex) { printf "-- FAIL state-table-unresolved: %s %s\n", nm, ex >> findfile }
# A product or sum of numbers (with an optional u suffix) and defined names; -1 when anything does not evaluate.
function ev(e, d,    np, parts, i, m, f, k, prod, sum, t, v) {
  if (d > 8) return -1
  gsub(/[ \t()]/, "", e)
  if (e == "") return -1
  np = split(e, parts, "+")
  sum = 0
  for (i = 1; i <= np; i++) {
    m = split(parts[i], f, "*")
    prod = 1
    for (k = 1; k <= m; k++) {
      t = f[k]
      if (t ~ /^[0-9][0-9]*[uU]?$/) { sub(/[uU]$/, "", t); v = t + 0 }
      else if (t in cv) { v = ev(cv[t], d + 1); if (v < 0) return -1 }
      else return -1
      prod = prod * v
    }
    sum = sum + prod
  }
  return sum
}
# A row name: string literals, and in a macro body the parameter name replaced by the label.
function nameof(e, pname, label,    n, i, parts, out, p) {
  n = split(e, parts, "\"")
  if (n % 2 == 0) return ""
  out = ""
  for (i = 1; i <= n; i++) {
    p = trim(parts[i])
    if (i % 2 == 0) out = out parts[i]
    else if (p != "") { if (pname != "" && p == pname) out = out label; else return "" }
  }
  return out
}
function clsmap(tok) {
  if (tok == "0u" || tok == "0") return "-"
  if (tok == "IVS_SNAP_STATE") return "STATE"
  if (tok == "IVS_SNAP_IMAGE") return "IMAGE_BEARING"
  if (tok == "IVS_SNAP_BOUND") return "BOUND"
  if (tok == "IVS_SNAP_PAD") return "PAD"
  return "?" tok
}
# The top-level arguments of the call whose open parenthesis is at index p of t: AV[1..ARGN], ENDP is the index of the closing parenthesis.
function getargs(t, p,    n, depth, instr, i, c, cur, len) {
  n = 0; depth = 0; instr = 0; cur = ""; len = length(t)
  for (i = p; i <= len; i++) {
    c = substr(t, i, 1)
    if (instr) { cur = cur c; if (c == "\"") instr = 0; continue }
    if (c == "\"") { instr = 1; cur = cur c; continue }
    if (c == "(") { depth++; if (depth > 1) cur = cur c; continue }
    if (c == ")") {
      depth--
      if (depth == 0) { n++; AV[n] = trim(cur); ARGN = n; ENDP = i; return 1 }
      cur = cur c
      continue
    }
    if (c == "," && depth == 1) { n++; AV[n] = trim(cur); cur = ""; continue }
    cur = cur c
  }
  return 0
}
# Every IVS_DF call of t (and, when top is set, every IVS_DF_SLOT use, expanded through the macro body) in order of appearance.
function scan(t, pname, label, top,    pos, rest, a, b, st, q, isslot, a1, a2, a3, a4, a5, a6, a7, an, ep, nm, w, c, d, k, cl, lbl) {
  pos = 1
  while (pos <= length(t)) {
    rest = substr(t, pos)
    a = index(rest, "IVS_DF(")
    b = 0
    if (top) b = index(rest, "IVS_DF_SLOT(")
    if (a == 0 && b == 0) break
    isslot = 0
    if (a == 0 || (b != 0 && b < a)) { st = b; isslot = 1; q = b + length("IVS_DF_SLOT") } else { st = a; q = a + length("IVS_DF") }
    if (st > 1 && substr(rest, st - 1, 1) ~ /[A-Za-z0-9_]/) { pos = pos + st; continue }
    if (!getargs(rest, q)) { perr("unbalanced call near " substr(rest, st, 40)); return }
    an = ARGN; ep = ENDP
    a1 = AV[1]; a2 = AV[2]; a3 = AV[3]; a4 = AV[4]; a5 = AV[5]; a6 = AV[6]; a7 = AV[7]
    pos = pos + ep
    if (isslot) {
      lbl = a2
      if (an != 2 || lbl !~ /^"[^"]*"$/) { perr("slot macro use with arguments " a1 " " a2); continue }
      if (slotscan == "") { perr("slot macro use without a macro definition"); continue }
      lbl = substr(lbl, 2, length(lbl) - 2)
      scan(slotscan, slotparam2, lbl, 0)
      continue
    }
    if (an != 6 && an != 7) { perr("IVS_DF with " an " arguments near " a1); continue }
    nm = nameof(a1, pname, label)
    if (nm == "") { perr("a row name is not a string literal: " a1); continue }
    w = ev(a3, 0); c = ev(a4, 0); d = ev(a5, 0)
    if (w < 0) { unres(nm, a3); continue }
    if (c < 0) { unres(nm, a4); continue }
    if (d < 0) { unres(nm, a5); continue }
    k = a6
    sub(/^IVS_DIGEST_KIND_/, "", k)
    if (k != "UINT" && k != "PROFILE" && k != "MAGIC" && k != "PAD") { perr("row " nm " has the kind " a6); continue }
    cl = "-"
    if (an == 7) cl = clsmap(a7)
    printf "%s %d %d %d %s %s\n", nm, w, c, d, k, cl >> rowsfile
  }
}
BEGIN {
  while ((getline l < consts) > 0) {
    i = index(l, "\t")
    if (i > 0) cv[substr(l, 1, i - 1)] = substr(l, i + 1)
  }
  close(consts)
}
{
  line = $0
  hascont = (line ~ /\\[ \t]*$/)
  sub(/\\[ \t]*$/, "", line)
  if (incont) {
    if (target == "slot") slotbody = slotbody " " line
    incont = hascont
    next
  }
  if (line ~ /^[ \t]*#/) {
    target = "other"
    if (line ~ /^[ \t]*#[ \t]*define[ \t]+IVS_DF_SLOT[ \t]*\(/) {
      target = "slot"
      slotbody = line
      sub(/^[ \t]*#[ \t]*define[ \t]+/, "", slotbody)
    }
    incont = hascont
    next
  }
  code = code "\n" line
}
END {
  # Block comments, then line comments, then newlines to blanks.
  out = ""
  t = code
  while ((s = index(t, "/*")) > 0) {
    out = out substr(t, 1, s - 1) " "
    t = substr(t, s + 2)
    e = index(t, "*/")
    if (e == 0) { t = ""; break }
    t = substr(t, e + 2)
  }
  code = out t
  while ((s = index(code, "//")) > 0) {
    e = index(substr(code, s), "\n")
    if (e == 0) code = substr(code, 1, s - 1)
    else code = substr(code, 1, s - 1) substr(code, s + e - 1)
  }
  gsub(/\n/, " ", code)
  if (slotbody != "") {
    p = index(slotbody, "IVS_DF_SLOT(")
    if (!getargs(slotbody, p + length("IVS_DF_SLOT"))) perr("unbalanced slot macro header")
    else { slotparam2 = AV[2]; slotscan = substr(slotbody, ENDP + 1) }
  }
  p = index(code, "ivs_digest_fields[]")
  if (p == 0) { perr("no table ivs_digest_fields[] in the source"); exit }
  t = substr(code, p)
  e = index(t, "};")
  if (e == 0) { perr("the table is not closed by a brace and a semicolon"); exit }
  scan(substr(t, 1, e), "", "", 1)
}' "$ROOT/$TABLE"

# Normalised inputs.
: > "$tmp/schema"
awk -v findfile="$find" '
NR == 1 { if ($0 != "IVSSCHEMA 1") printf "-- FAIL state-schema-form: line 1 is not IVSSCHEMA 1\n" >> findfile; next }
{
  if (NF != 6 || $0 != $1 " " $2 " " $3 " " $4 " " $5 " " $6 || $0 ~ /\r/) { printf "-- FAIL state-schema-form: line %d\n", NR >> findfile; next }
  print
}' "$ROOT/$SCHEMA" > "$tmp/schema"
awk -v findfile="$find" '
/^#/ || NF == 0 { next }
{
  if (NF != 5) { printf "-- FAIL state-checklist-form: line %d has %d fields\n", NR, NF >> findfile; next }
  print
}' "$ROOT/$CHECK" > "$tmp/check"
awk -v findfile="$find" '
/^#/ || NF == 0 { next }
{
  if (NF != 4 || $1 !~ /^HP-[A-Z0-9-]*$/ || $2 !~ /^[A-Za-z0-9_-][A-Za-z0-9_-]*$/ || ($3 != "pending-08-14" && $3 !~ /^[0-9][0-9]*$/) ||
      ($4 != "cpu-points" && $4 != "stic-points" && $4 != "device-points")) {
    printf "-- FAIL state-cut-form: line %d\n", NR >> findfile
    next
  }
  print
}' "$ROOT/$CUTS" > "$tmp/cuts"
awk -v findfile="$find" '
/^#/ || NF == 0 { next }
{
  if (NF != 2) { printf "-- FAIL state-pending-form: line %d\n", NR >> findfile; next }
  print
}' "$ROOT/$PEND" > "$tmp/pend"
: > "$tmp/reg"
for f in "$ROOT"/data/expected/tests/*.ids; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } NF { print $1 }' "$f" >> "$tmp/reg"
done

# The comparisons.
awk -v findfile="$find" -v tfile="$tmp/table" -v sfile="$tmp/schema" -v cfile="$tmp/check" -v ufile="$tmp/cuts" -v pfile="$tmp/pend" -v rfile="$tmp/reg" '
function out(tok, det) { printf "-- FAIL %s: %s\n", tok, det >> findfile }
function validclass(c) { return c == "STATE" || c == "IMAGE_BEARING" || c == "BOUND" || c == "PAD" }
BEGIN {
  nt = 0
  while ((getline l < tfile) > 0) {
    split(l, f, " ")
    nt++; tn[nt] = f[1]
    if (f[1] in tseen) out("state-table-dup", f[1])
    tseen[f[1]] = 1
    tw[f[1]] = f[2]; tc[f[1]] = f[3]; td[f[1]] = f[4]; tk[f[1]] = f[5]; tcl[f[1]] = f[6]
  }
  ns = 0
  while ((getline l < sfile) > 0) {
    split(l, f, " ")
    ns++; sn[ns] = f[1]
    if (f[1] in sseen) out("state-line-dup", f[1] " (schema)")
    sseen[f[1]] = 1
    sw[f[1]] = f[2]; sc[f[1]] = f[3]; sd[f[1]] = f[4]; sk[f[1]] = f[5]; scl[f[1]] = f[6]
  }
  nc = 0
  while ((getline l < cfile) > 0) {
    split(l, f, " ")
    nc++; cn[nc] = f[1]
    if (f[1] in cseen) out("state-line-dup", f[1] " (checklist)")
    cseen[f[1]] = 1
    ccl[f[1]] = f[2]; cwit[f[1]] = f[3]; ctest[f[1]] = f[4]; csw[f[1]] = f[5]
  }
  nu = 0
  while ((getline l < ufile) > 0) {
    split(l, f, " ")
    nu++; un[nu] = f[1]
    if (f[1] in useen) out("state-cut-dup", f[1])
    useen[f[1]] = 1
  }
  np = 0
  while ((getline l < pfile) > 0) {
    split(l, f, " ")
    np++; pid[np] = f[1]; pown[np] = f[2]
    pend[f[1]] = 1
  }
  while ((getline l < rfile) > 0) reg[l] = 1

  # Row sets.
  bad_s = 0
  for (i = 1; i <= nt; i++) {
    r = tn[i]
    if (!(r in sseen)) { out("state-row-missing", r " (schema)"); bad_s = 1 }
    if (!(r in cseen)) out("state-row-missing", r " (checklist)")
  }
  for (i = 1; i <= ns; i++) if (!(sn[i] in tseen)) { out("state-line-extra", sn[i] " (schema)"); bad_s = 1 }
  for (i = 1; i <= nc; i++) if (!(cn[i] in tseen)) out("state-line-extra", cn[i] " (checklist)")
  if (!bad_s && nt == ns) {
    for (i = 1; i <= nt; i++) if (sn[i] != tn[i]) { out("state-schema-order", tn[i]); break }
  }

  # Schema against the table.
  for (i = 1; i <= nt; i++) {
    r = tn[i]
    if (!(r in sseen)) continue
    if (sk[r] != tk[r]) out("state-kind-drift", r " (table " tk[r] " schema " sk[r] ")")
    if (sd[r] != td[r]) out("state-domain-drift", r " (table " td[r] " schema " sd[r] ")")
    if (sw[r] != tw[r] || sc[r] != tc[r]) out("state-shape-drift", r " (table " tw[r] "x" tc[r] " schema " sw[r] "x" sc[r] ")")
  }

  # Classes, witnesses, tests and the sweep column.
  for (i = 1; i <= nc; i++) {
    r = cn[i]
    c = ccl[r]
    if (!validclass(c)) { out("state-class-unknown", r " " c); continue }
    if ((r in sseen) && !validclass(scl[r])) out("state-class-unknown", r " " scl[r] " (schema)")
    else if ((r in sseen) && scl[r] != c) out("state-class-drift", r " (checklist " c " schema " scl[r] ")")
    if (r in tseen) {
      if (substr(tcl[r], 1, 1) == "?") out("state-class-unknown", r " " substr(tcl[r], 2) " (source)")
      else if (tcl[r] != "-" && tcl[r] != c) out("state-class-drift", r " (source " tcl[r] " checklist " c ")")
      k = tk[r]
      if ((k == "PAD" && c != "PAD") || (c == "PAD" && k != "PAD") || ((k == "MAGIC" || k == "PROFILE") && c != "BOUND")) out("state-class-kind", r " (kind " k " class " c ")")
    }
    if (c == "STATE") {
      w = cwit[r]
      if (!(w in useen)) out("state-witness-unknown", r " " w)
      else used[w] = 1
      s = csw[r]
      if (s == "ROW") { }
      else if (substr(s, 1, 7) == "EXEMPT:") {
        reason = substr(s, 8)
        nw = split(reason, ws, "_")
        okr = (nw >= 3)
        for (j = 1; j <= nw; j++) if (ws[j] == "") okr = 0
        if (!okr) out("state-exempt-no-reason", r)
      }
      else out("state-sweep-unknown", r " " s)
    } else {
      if (cwit[r] != "-" || csw[r] != "-") out("state-nonstate-witness", r)
    }
    t = ctest[r]
    if (t !~ /^[A-Za-z0-9][A-Za-z0-9._-]*$/ || (!(t in reg) && !(t in pend))) out("state-test-unknown", r " " t)
  }
  for (i = 1; i <= nu; i++) if (!(un[i] in used)) out("state-cut-unused", un[i])

  # Pending lines.
  for (i = 1; i <= np; i++) {
    if (pown[i] !~ /^08-1[1-7]$/) out("state-pending-owner", pid[i] " " pown[i])
    if (pid[i] in reg) out("state-pending-stale", pid[i])
  }
}' /dev/null

# The pinned schema digest.
want=$(awk '/^#/ { next } $1 == "schema-digest-sha256" { print $2; exit }' "$ROOT/$LIT")
if [ -z "$want" ]; then
  fail state-literals-missing "schema-digest-sha256 in $LIT"
else
  got=$(cmake -E sha256sum "$ROOT/$SCHEMA" | awk '{ print $1 }')
  if [ "$want" != "$got" ]; then fail state-schema-stale "pinned $want actual $got"; fi
fi

rows=$(grep -c . "$tmp/table")
cuts=$(grep -c . "$tmp/cuts")
if [ -s "$find" ]; then
  sort -u "$find"
  exit 1
fi
printf -- '-- state pass state-rows=%s state-cuts=%s\n' "$rows" "$cuts"
