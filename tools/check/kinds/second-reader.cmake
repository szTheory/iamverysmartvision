# SPDX-License-Identifier: MIT
# Hook for kind second-reader (SKELETON.md section 4).
# Failure tokens: second-reader-format: <id>/pdf, second-reader-c17-format: c17_reading.
# Stats: agree=<n>, resolved=<n>.
# The verdict file records the blind second-reader pass of plan 01-10. The schema enforces id parity
# with the family table (inventory), the verdict vocabulary, and a note on every resolved entry.
cmake_minimum_required(VERSION 3.25)

function(hook_record id rec)
  json_type(vt "${rec}" verdict)
  if(vt STREQUAL STRING)
    string(JSON verdict GET "${rec}" verdict)
    if("${verdict}" STREQUAL "agree")
      gate_incr(reader-agree)
    elseif("${verdict}" STREQUAL "resolved")
      gate_incr(reader-resolved)
    endif()
  endif()
  json_type(pt "${rec}" pdf)
  if(pt STREQUAL STRING)
    string(JSON pdf GET "${rec}" pdf)
    if(NOT "${pdf}" MATCHES "^pdf [0-9]+$")
      gate_fail(second-reader-format "${id}/pdf")
    endif()
  endif()
endfunction()

function(hook_finish)
  # c17_reading: `agree: ...` or `resolved: ...` and at least one `pdf <n>` token.
  json_type(ct "${REC_DOC}" c17_reading)
  if(ct STREQUAL STRING)
    string(JSON c17 GET "${REC_DOC}" c17_reading)
    if(NOT "${c17}" MATCHES "^(agree|resolved): " OR NOT "${c17}" MATCHES "pdf [0-9]+")
      gate_fail(second-reader-c17-format "c17_reading")
    endif()
  endif()
  gate_count(a reader-agree)
  gate_count(r reader-resolved)
  gate_stat(agree "${a}")
  gate_stat(resolved "${r}")
endfunction()
