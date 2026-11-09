# SPDX-License-Identifier: MIT
# Hook for kind profile (SKELETON.md section 4).
# Failure tokens: profile-d3-uncited, profile-tier5-only: <field>,
#   profile-measured-without-d10: <field>.
# Stats: fields=<n>, unknown-fields=<n of unknown-default-chosen>.
cmake_minimum_required(VERSION 3.25)

function(hook_record id rec)
  gate_incr(profile-fields)
  json_type(tt "${rec}" tag)
  if(NOT tt STREQUAL STRING)
    return()
  endif()
  string(JSON tag GET "${rec}" tag)
  if(tag STREQUAL "unknown-default-chosen")
    gate_incr(profile-unknown)
  elseif(tag STREQUAL "tier5-emulator-hint")
    gate_fail(profile-tier5-only "${id}")
  elseif(tag STREQUAL "measured")
    gate_fail(profile-measured-without-d10 "${id}")
  endif()
endfunction()

function(hook_finish)
  set(sel "")
  json_type(st "${REC_DOC}" selected_by)
  if(st STREQUAL STRING)
    string(JSON sel GET "${REC_DOC}" selected_by)
  endif()
  string(FIND "${sel}" "D3" pos)
  if(pos EQUAL -1)
    gate_fail(profile-d3-uncited "selected_by")
  endif()
  gate_count(nf profile-fields)
  gate_count(nu profile-unknown)
  gate_stat(fields "${nf}")
  gate_stat(unknown-fields "${nu}")
endfunction()
