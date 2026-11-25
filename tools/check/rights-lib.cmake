# SPDX-License-Identifier: MIT
# Rights-ledger checker library (RTS-01, D-46), include()d from manifests.cmake.
# One file per ledger row at data/rights/<slug>.json. The FILELIST selection below is the per-file
# form of manifest-lib.cmake's manifest_load_all() loop (13-PATTERNS divergence D1, resolved to form
# (a)): CONTEXT fixes one file per row, so there is no single data/rights.json collection and no
# schema-language kind. The exact key set lives here, in one place, rather than in a schema file.
# A row is: id, item, disposition in {cleared, refused, owner-local-only}, source (a SRC- id or
# null), taken ("nothing" is legal), tokens (an array; an empty array is a recorded answer) and note.
# Tokens: rights-ledger-empty: <dir>, rights-parse: <path>, rights-missing-key: <slug>/<key>,
#   rights-format: <slug>/<key>, rights-unknown-key: <slug>/<key>, rights-id-mismatch: <path>,
#   rights-disposition: <slug>, rights-blank: <slug>/<key>, rights-token-format: <slug>/<token>,
#   rights-source-missing: <slug>, rights-source-unresolved: <slug>/<src>,
#   rights-duplicate-id: <id>, rights-id-missing: <id>, rights-id-extra: <id>,
#   rights-doc-missing: docs/RIGHTS.md, rights-doc-row: <id>, plus the three
#   rights-inventory-missing, rights-inventory-empty and rights-inventory-duplicate that read_ids()
#   produces for data/expected/rights.ids.
# Called from: manifests.cmake, once, after the three manifest gate_stat() calls and immediately
#   before gate_finish(manifests). The position is load-bearing: gate_finish() prints GATE_STATS in
#   insertion order, so registering the two rights stats last is what makes the step's printed
#   success line equal the order its own header documents.
# Stats: rights=<n> (ledger paths selected from FILELIST), rights-cleared=<k>.
# Limit: the ledger answers "may this be named, read or shipped". It cannot decide whether the
#   author of a licence statement had the authority to give it.
cmake_minimum_required(VERSION 3.25)

# Ledger paths from FILELIST: data/rights/<slug>.json, sorted. Anchored at the repository root, so
# a control overlay or a good-tree copy under tools/check/controls/ is never selected on the real
# tree (the same anchoring decision steps/sources.sh documents for its own scan).
function(rights_select out)
  file(READ "${FILELIST}" txt)
  gate_lines(lines "${txt}")
  set(sel "")
  foreach(l IN LISTS lines)
    gate_unprotect(l "${l}")
    if(l MATCHES "^data/rights/[a-z0-9][a-z0-9-]*[.]json$")
      list(APPEND sel "${l}")
    endif()
  endforeach()
  list(SORT sel)
  set(${out} "${sel}" PARENT_SCOPE)
endfunction()

# Presence, type and allowed keys of one ledger row. ARGN holds "key:TYPE|TYPE" specs; every listed
# key is required and every other member is an unknown key, so a misspelled `dispositon` is an
# unknown key rather than a silently ignored one (the fail-closed shape check of manifests.cmake,
# restated here because the ledger is not a records.cmake kind).
# Sets out_bad TRUE when a key is absent, a type is wrong, or a member is unknown. The caller then
# skips that row's value rules: a row whose `disposition` is missing is refused once, by name, and
# is not also judged against the disposition enum for a value it does not have. The `rights-ledger`
# control leans on this, so the short-circuit is a contract, not an optimisation.
function(rights_shape out_bad slug obj)
  set(bad FALSE)
  set(allowed "")
  foreach(spec IN LISTS ARGN)
    string(FIND "${spec}" ":" c)
    string(SUBSTRING "${spec}" 0 ${c} key)
    math(EXPR c1 "${c} + 1")
    string(SUBSTRING "${spec}" ${c1} -1 types)
    string(REPLACE "|" ";" types "${types}")
    list(APPEND allowed "${key}")
    json_type(t "${obj}" "${key}")
    if(t STREQUAL "MISSING")
      gate_fail(rights-missing-key "${slug}/${key}")
      set(bad TRUE)
    elseif(NOT "${t}" IN_LIST types)
      gate_fail(rights-format "${slug}/${key}")
      set(bad TRUE)
    endif()
  endforeach()
  json_members(mem "${obj}")
  foreach(m IN LISTS mem)
    if(NOT "${m}" IN_LIST allowed)
      gate_fail(rights-unknown-key "${slug}/${m}")
      set(bad TRUE)
    endif()
  endforeach()
  set(${out_bad} "${bad}" PARENT_SCOPE)
endfunction()

# Ids of the sources registry (data/sources.json), read once for the whole ledger. A missing or
# unparseable registry yields the empty set, which is fail-closed: every non-null `source` is then
# rights-source-unresolved rather than silently accepted.
function(rights_source_ids out)
  set(ids "")
  set(f "${ROOT}/data/sources.json")
  if(EXISTS "${f}")
    file(READ "${f}" doc)
    string(JSON t ERROR_VARIABLE e TYPE "${doc}")
    if("${e}" STREQUAL "NOTFOUND" AND t STREQUAL "OBJECT")
      json_type(st "${doc}" sources)
      if(st STREQUAL "ARRAY")
        string(JSON n LENGTH "${doc}" sources)
        if(n GREATER 0)
          math(EXPR last "${n} - 1")
          foreach(i RANGE 0 ${last})
            json_type(it "${doc}" sources ${i} id)
            if(it STREQUAL "STRING")
              string(JSON sid GET "${doc}" sources ${i} id)
              list(APPEND ids "${sid}")
            endif()
          endforeach()
        endif()
      endif()
    endif()
  endif()
  set(${out} "${ids}" PARENT_SCOPE)
endfunction()

# Judge every ledger row, then the inventory, then the operator page.
function(rights_check)
  rights_select(sel)
  list(LENGTH sel nsel)
  if(nsel EQUAL 0)
    gate_fail(rights-ledger-empty "data/rights")
  endif()
  rights_source_ids(srcids)
  set(doc_path "docs/RIGHTS.md")
  set(ids "")
  set(doc_ids "")
  set(doc_items "")
  set(ncleared 0)

  foreach(p IN LISTS sel)
    string(REGEX REPLACE "^data/rights/(.*)[.]json$" "\\1" slug "${p}")
    file(READ "${ROOT}/${p}" doc)
    string(JSON t ERROR_VARIABLE e TYPE "${doc}")
    if(NOT "${e}" STREQUAL "NOTFOUND" OR NOT t STREQUAL "OBJECT")
      gate_fail(rights-parse "${p}")
      continue()
    endif()
    # The row's identity is collected before the shape verdict, because the inventory and the
    # operator page are separate limbs: a row whose `disposition` is missing is refused by name for
    # that, and must not also produce a spurious rights-id-missing for an id it does have. A row
    # with no readable id is the one case where the inventory does speak, and it should.
    json_type(idt "${doc}" id)
    if(idt STREQUAL "STRING")
      string(JSON id GET "${doc}" id)
      list(APPEND ids "${id}")
      json_type(it "${doc}" item)
      if(it STREQUAL "STRING")
        string(JSON item GET "${doc}" item)
        list(APPEND doc_ids "${id}")
        list(APPEND doc_items "${item}")
      endif()
    endif()

    rights_shape(shape_bad "${slug}" "${doc}"
      "id:STRING" "item:STRING" "disposition:STRING" "source:STRING|NULL"
      "taken:STRING" "tokens:ARRAY" "note:STRING")
    if(shape_bad)
      continue()
    endif()

    string(JSON disposition GET "${doc}" disposition)

    # The slug in the path is the row's id, mirroring manifest-id-dir-mismatch: a rename that moves
    # the file and not the id (or the reverse) is refused rather than silently accepted.
    if(NOT id STREQUAL slug)
      gate_fail(rights-id-mismatch "${p}")
    endif()
    if(NOT "${disposition}" MATCHES "^(cleared|refused|owner-local-only)$")
      gate_fail(rights-disposition "${slug}")
    endif()
    if(disposition STREQUAL "cleared")
      math(EXPR ncleared "${ncleared} + 1")
    endif()

    # item, taken and note must say something. tokens is deliberately absent from this list: an
    # empty array is a recorded answer ("this row refuses no spelling"), not a blank cell.
    foreach(k IN ITEMS item taken note)
      string(JSON v GET "${doc}" ${k})
      string(STRIP "${v}" v)
      if(v STREQUAL "")
        gate_fail(rights-blank "${slug}/${k}")
      endif()
    endforeach()

    # Every token is a spelling a refused item must never appear as, so it must be a single
    # character-bounded word: the class admits dots, digits, letters, underscores and hyphens and
    # nothing else, so a token can neither carry a space nor start with punctuation.
    string(JSON ntok LENGTH "${doc}" tokens)
    if(ntok GREATER 0)
      math(EXPR tlast "${ntok} - 1")
      foreach(i RANGE 0 ${tlast})
        json_type(tt "${doc}" tokens ${i})
        if(NOT tt STREQUAL "STRING")
          gate_fail(rights-format "${slug}/tokens/${i}")
          continue()
        endif()
        string(JSON tok GET "${doc}" tokens ${i})
        if(NOT tok MATCHES "^[.A-Za-z0-9][A-Za-z0-9._-]*$")
          gate_fail(rights-token-format "${slug}/${tok}")
        endif()
      endforeach()
    endif()

    # A cleared row needs a resolvable source; resolvability is membership in the sources registry
    # and never the record's own `rights` field, which answers a different question (that field is
    # pinned to `unknown` by schema/source.txt and describes the reference document, not this row).
    json_type(srct "${doc}" source)
    if(srct STREQUAL "NULL")
      if(disposition STREQUAL "cleared")
        gate_fail(rights-source-missing "${slug}")
      endif()
    else()
      string(JSON src GET "${doc}" source)
      if(NOT "${src}" IN_LIST srcids)
        gate_fail(rights-source-unresolved "${slug}/${src}")
      endif()
    endif()
  endforeach()

  # Duplicate ids, then set equality with the inventory in both directions: deleting a row or
  # adding one without its inventory line is red either way.
  list(SORT ids)
  set(prev "")
  foreach(i IN LISTS ids)
    if(i STREQUAL prev)
      gate_fail(rights-duplicate-id "${i}")
    endif()
    set(prev "${i}")
  endforeach()
  set(uniq ${ids})
  list(REMOVE_DUPLICATES uniq)
  read_ids(want "data/expected/rights.ids" rights)
  foreach(i IN LISTS want)
    if(NOT "${i}" IN_LIST uniq)
      gate_fail(rights-id-missing "${i}")
    endif()
  endforeach()
  foreach(i IN LISTS uniq)
    if(NOT "${i}" IN_LIST want)
      gate_fail(rights-id-extra "${i}")
    endif()
  endforeach()

  # The operator page must exist and must carry every row's `item` phrase verbatim. The row is
  # keyed on `item`, never on `id`, so the page never has to spell a refused token to name a row.
  if(EXISTS "${ROOT}/${doc_path}")
    file(READ "${ROOT}/${doc_path}" doctxt)
    list(LENGTH doc_items ndoc)
    if(ndoc GREATER 0)
      math(EXPR dlast "${ndoc} - 1")
      foreach(i RANGE 0 ${dlast})
        list(GET doc_ids ${i} did)
        list(GET doc_items ${i} ditem)
        string(FIND "${doctxt}" "${ditem}" pos)
        if(pos LESS 0)
          gate_fail(rights-doc-row "${did}")
        endif()
      endforeach()
    endif()
  else()
    gate_fail(rights-doc-missing "${doc_path}")
  endif()

  gate_stat(rights "${nsel}")
  gate_stat(rights-cleared "${ncleared}")
endfunction()
