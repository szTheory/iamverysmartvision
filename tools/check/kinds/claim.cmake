# SPDX-License-Identifier: MIT
# Hook for kind claim (SKELETON.md section 4).
# Failure tokens: claim-order: <id>, claim-unknown-profile: <id>, claim-dangling-path: <id>,
#   claim-handoff-artifact: <id>,
#   claim-unknown-field: <id>/<field>, claim-overstated: <id>,
#   claim-measured-without-d10: <id>, claim-type: <id>/depends_on.
# Stats: claims=<n>, provisional=<n>.
# The profile file is read once per profile id and cached in GLOBAL properties.
cmake_minimum_required(VERSION 3.25)

# Load data/profiles/<profile>.json. Sets GLOBAL properties
#   CLAIM_PROFILE_OK:<profile>  TRUE or FALSE
#   CLAIM_PROFILE_FIELDS:<profile>  list of field paths
#   CLAIM_PROFILE_TAG:<profile>:<path>  the tag of that field
function(claim_load_profile profile)
  set(loaded "CLAIM_PROFILE_LOADED:${profile}")
  get_property(done GLOBAL PROPERTY "${loaded}" SET)
  if(done)
    return()
  endif()
  set_property(GLOBAL PROPERTY "${loaded}" "1")
  set_property(GLOBAL PROPERTY "CLAIM_PROFILE_OK:${profile}" "FALSE")
  if(NOT "${profile}" MATCHES "^[a-z0-9][a-z0-9-]*$")
    return()
  endif()
  set(rel "data/profiles/${profile}.json")
  if(NOT EXISTS "${ROOT}/${rel}")
    return()
  endif()
  read_record_file(pdoc "${rel}" claim-profile)
  if("${pdoc}" STREQUAL "")
    return()
  endif()
  json_type(ft "${pdoc}" fields)
  if(NOT ft STREQUAL OBJECT)
    return()
  endif()
  string(JSON fobj GET "${pdoc}" fields)
  json_members(paths "${fobj}")
  set(plist "")
  foreach(p IN LISTS paths)
    list(APPEND plist "${p}")
    set(tag "")
    json_type(tt "${pdoc}" fields "${p}" tag)
    if(tt STREQUAL STRING)
      string(JSON tag GET "${pdoc}" fields "${p}" tag)
    endif()
    set_property(GLOBAL PROPERTY "CLAIM_PROFILE_TAG:${profile}:${p}" "${tag}")
  endforeach()
  set_property(GLOBAL PROPERTY "CLAIM_PROFILE_FIELDS:${profile}" "${plist}")
  set_property(GLOBAL PROPERTY "CLAIM_PROFILE_OK:${profile}" "TRUE")
endfunction()

function(hook_record id rec)
  gate_incr(claim-total)
  # Ids are CL-NNNN with four digits, so bytewise comparison equals numeric comparison.
  if("${id}" MATCHES "^CL-[0-9][0-9][0-9][0-9]$")
    string(SUBSTRING "${id}" 3 -1 num)
    get_property(have GLOBAL PROPERTY CLAIM_LAST_NUM SET)
    if(have)
      get_property(prev GLOBAL PROPERTY CLAIM_LAST_NUM)
      if(NOT "${num}" STRGREATER "${prev}")
        gate_fail(claim-order "${id}")
      endif()
    endif()
    set_property(GLOBAL PROPERTY CLAIM_LAST_NUM "${num}")
  endif()

  set(profile "")
  json_type(pt "${rec}" profile)
  if(pt STREQUAL STRING)
    string(JSON profile GET "${rec}" profile)
  endif()
  set(artifact "")
  json_type(at "${rec}" artifact)
  if(at STREQUAL STRING)
    string(JSON artifact GET "${rec}" artifact)
  endif()
  set(standing "")
  json_type(st "${rec}" standing)
  if(st STREQUAL STRING)
    string(JSON standing GET "${rec}" standing)
  endif()
  set(tier "")
  json_type(tt "${rec}" tier)
  if(tt STREQUAL STRING)
    string(JSON tier GET "${rec}" tier)
  endif()

  if(standing STREQUAL "provisional")
    gate_incr(claim-provisional)
  endif()
  if(tier STREQUAL "measured-hardware")
    gate_fail(claim-measured-without-d10 "${id}")
  endif()

  # The artifact must be a repo-relative file under ROOT.
  if(NOT "${artifact}" STREQUAL "")
    if(NOT "${artifact}" MATCHES "^[A-Za-z0-9._/-]+$" OR "${artifact}" MATCHES "^/" OR "${artifact}" MATCHES "[.][.]"
       OR NOT EXISTS "${ROOT}/${artifact}" OR IS_DIRECTORY "${ROOT}/${artifact}")
      gate_fail(claim-dangling-path "${id}")
    endif()
  endif()

  # A claim never cites a handoff-shaped artifact (EV-05, GUARDRAILS 2.8): a build product, an
  # operator render, a timing record or the handoff ledger itself is never evidence for a claim,
  # so each of the five classes is refused by its own token rather than by claim-dangling-path,
  # which only fires while the file is absent. The any-`.tsv` suffix is used instead of a
  # `data/bench/` prefix: one predicate instead of a path prefix to get wrong, and it covers the
  # hosted timing record `.planning/ci-runs.tsv` as well.
  #
  # The test is made on the NORMALISED path, never on the raw spelling. file(REAL_PATH) resolves a
  # leading `./`, a doubled `/`, and a symlink indirection to the real target, so `./build/x` and
  # `data/evidence/x` (a link to build) are the same file as `build/x`; the path is then made
  # relative to ROOT and lower-cased, because this project's primary host is case-insensitive, so
  # `Build/x` resolves there and `out/x.WAV` is the file `out/x.wav`. Without that normalisation a
  # claim satisfied every clause by spelling alone: only the file had to exist, and during a gate
  # run `build/gate/` does. Residual, named rather than papered over: a symlink whose target lies
  # OUTSIDE the repository normalises to a `../`-relative path, which no clause below matches; such
  # a claim is still refused by claim-dangling-path only if the link is broken, so a link to an
  # external build directory is not refused here and is recorded as an open residual.
  # ROOT is resolved too: a tree reached through a symlink (a worktree under /tmp, which is a link
  # on Darwin) would otherwise make every relative path a `../` path and silently disarm the two
  # prefix clauses while the suffix clauses kept working.
  if(NOT "${artifact}" STREQUAL "" AND EXISTS "${ROOT}/${artifact}")
    file(REAL_PATH "${ROOT}" handoff_root)
    file(REAL_PATH "${ROOT}/${artifact}" handoff_real)
    file(RELATIVE_PATH handoff_norm "${handoff_root}" "${handoff_real}")
    string(TOLOWER "${handoff_norm}" handoff_low)
    if("${handoff_low}" MATCHES "^build/" OR "${handoff_low}" MATCHES "[.]wav$"
       OR "${handoff_low}" MATCHES "[.]ppm$" OR "${handoff_low}" MATCHES "[.]tsv$"
       OR "${handoff_low}" STREQUAL "data/expected/handoffs.ids")
      gate_fail(claim-handoff-artifact "${id}")
    endif()
  endif()

  # The profile must exist; depended-on fields must be members of it.
  if("${profile}" STREQUAL "")
    return()
  endif()
  claim_load_profile("${profile}")
  get_property(pok GLOBAL PROPERTY "CLAIM_PROFILE_OK:${profile}")
  if(NOT pok)
    gate_fail(claim-unknown-profile "${id}")
    return()
  endif()
  get_property(known GLOBAL PROPERTY "CLAIM_PROFILE_FIELDS:${profile}")
  json_type(dt "${rec}" depends_on)
  if(NOT dt STREQUAL ARRAY)
    return()
  endif()
  string(JSON dn LENGTH "${rec}" depends_on)
  if(dn EQUAL 0)
    return()
  endif()
  math(EXPR dlast "${dn} - 1")
  set(weak FALSE)
  foreach(i RANGE 0 ${dlast})
    json_type(et "${rec}" depends_on ${i})
    if(NOT et STREQUAL STRING)
      gate_fail(claim-type "${id}/depends_on")
      continue()
    endif()
    string(JSON field GET "${rec}" depends_on ${i})
    if(NOT "${field}" IN_LIST known)
      gate_fail(claim-unknown-field "${id}/${field}")
      continue()
    endif()
    get_property(ftag GLOBAL PROPERTY "CLAIM_PROFILE_TAG:${profile}:${field}")
    if("${ftag}" MATCHES "^(tier4-community|tier5-emulator-hint|inference|unknown-default-chosen)$")
      set(weak TRUE)
    endif()
  endforeach()
  # Weakest-field rule: an established claim may not rest on a weak field.
  if(weak AND standing STREQUAL "established")
    gate_fail(claim-overstated "${id}")
  endif()
endfunction()

function(hook_finish)
  gate_count(nc claim-total)
  gate_count(np claim-provisional)
  gate_stat(claims "${nc}")
  gate_stat(provisional "${np}")
endfunction()
