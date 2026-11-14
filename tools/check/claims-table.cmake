# SPDX-License-Identifier: MIT
# Claim table of the release notes, rendered from the claim ledger (plan 08-02, D-27, D-33).
#
#   cmake -DROOT=<repo> [-DCLAIMS=<file>] -P tools/check/claims-table.cmake
#
# Reads ${CLAIMS} (default ${ROOT}/data/claims.json) and prints to stdout, and writes nothing:
#   | Claim | Tier | Standing | Artifact |
#   |---|---|---|---|
#   | CL-0001 | <tier> | <standing> | <artifact> |      one row per claim, in file order
#   (a blank line)
#   Claims: <N> in total, <P> provisional, <E> established; none at tier measured-hardware.
# No claim statement or note is printed: a statement is a claim and needs its own wording rules, so the
# table cites id, tier, standing and artifact only. CHANGELOG.md holds this exact text between the
# markers claims-table-begin and claims-table-end, and the release-assets gate step fails when the two
# differ, so any change of data/claims.json regenerates the table in the same commit.
# Findings (a line on stdout, then exit status 1):
#   release-claims-measured-hardware: <id>   a claim at tier measured-hardware; no hardware measurement
#                                            was performed (D10), so no claim may stand there
#   release-claims-unreadable: <what>       a missing or unparsable file, a claim without its id, tier,
#                                            standing or artifact, or a standing other than provisional
#                                            or established
cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED ROOT OR "${ROOT}" STREQUAL "")
  message(FATAL_ERROR "claims-table: -DROOT=<repo> is required")
endif()
if(NOT DEFINED CLAIMS OR "${CLAIMS}" STREQUAL "")
  set(CLAIMS "${ROOT}/data/claims.json")
endif()

set(out "")
set(findings "")

function(ct_finish)
  # Findings are printed on stdout like the table, then the script ends with a nonzero status.
  execute_process(COMMAND "${CMAKE_COMMAND}" -E echo_append "${findings}")
  message(FATAL_ERROR "claims-table: failed")
endfunction()

if(NOT EXISTS "${CLAIMS}" OR IS_DIRECTORY "${CLAIMS}")
  get_filename_component(cname "${CLAIMS}" NAME)
  set(findings "release-claims-unreadable: ${cname}\n")
  ct_finish()
endif()
file(READ "${CLAIMS}" doc)
string(JSON ctype ERROR_VARIABLE err TYPE "${doc}" claims)
if(NOT "${err}" STREQUAL "NOTFOUND" OR NOT ctype STREQUAL ARRAY)
  set(findings "release-claims-unreadable: claims array\n")
  ct_finish()
endif()
string(JSON n LENGTH "${doc}" claims)
if(n EQUAL 0)
  set(findings "release-claims-unreadable: no claim\n")
  ct_finish()
endif()

set(out "| Claim | Tier | Standing | Artifact |\n|---|---|---|---|\n")
set(np 0)
set(ne 0)
math(EXPR last "${n} - 1")
foreach(i RANGE 0 ${last})
  string(JSON claim GET "${doc}" claims ${i})
  set(ok TRUE)
  foreach(field id tier standing artifact)
    string(JSON ft ERROR_VARIABLE err TYPE "${claim}" ${field})
    if(NOT "${err}" STREQUAL "NOTFOUND" OR NOT ft STREQUAL STRING)
      string(APPEND findings "release-claims-unreadable: claim ${i} ${field}\n")
      set(ok FALSE)
    else()
      string(JSON v_${field} GET "${claim}" ${field})
      if("${v_${field}}" STREQUAL "")
        string(APPEND findings "release-claims-unreadable: claim ${i} ${field}\n")
        set(ok FALSE)
      endif()
    endif()
  endforeach()
  if(NOT ok)
    continue()
  endif()
  if(v_tier STREQUAL "measured-hardware")
    string(APPEND findings "release-claims-measured-hardware: ${v_id}\n")
  endif()
  if(v_standing STREQUAL "provisional")
    math(EXPR np "${np} + 1")
  elseif(v_standing STREQUAL "established")
    math(EXPR ne "${ne} + 1")
  else()
    string(APPEND findings "release-claims-unreadable: ${v_id} standing\n")
  endif()
  string(APPEND out "| ${v_id} | ${v_tier} | ${v_standing} | ${v_artifact} |\n")
endforeach()

if(NOT "${findings}" STREQUAL "")
  ct_finish()
endif()
string(APPEND out "\nClaims: ${n} in total, ${np} provisional, ${ne} established; none at tier measured-hardware.\n")
execute_process(COMMAND "${CMAKE_COMMAND}" -E echo_append "${out}")
