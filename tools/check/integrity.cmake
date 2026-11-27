# SPDX-License-Identifier: MIT
# Encoding, path and licence-line guard over a path list (SKELETON.md section 6).
# Inputs (passed with -D):
#   ROOT      the tree under test (required)
#   FILELIST  file with one repo-relative path per line (required)
# Failure tokens: integrity-filelist-empty, integrity-path-charset, integrity-empty-file,
#   integrity-crlf, integrity-bom, integrity-non-ascii, integrity-missing-spdx, integrity-ext-list-empty.
# NUL bytes (integrity-nul-byte) are reported by steps/integrity.sh through binscan.sh.
# Success line: "integrity pass files=<n>".
cmake_minimum_required(VERSION 3.25)

foreach(required ROOT FILELIST)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "integrity.cmake is missing ${required}")
  endif()
endforeach()
include("${CMAKE_CURRENT_LIST_DIR}/lib.cmake")

file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/binary-ext.txt" SKIP_EXT REGEX "^[a-z0-9]+$")
if("${SKIP_EXT}" STREQUAL "")
  gate_fail(integrity-ext-list-empty "tools/check/binary-ext.txt")
endif()
file(READ "${FILELIST}" list_text)
gate_lines(paths "${list_text}")
set(nfiles 0)
foreach(p IN LISTS paths)
  gate_unprotect(p "${p}")
  if(p STREQUAL "")
    continue()
  endif()
  math(EXPR nfiles "${nfiles} + 1")
  if(NOT p MATCHES "^[A-Za-z0-9._/-]+$")
    gate_fail(integrity-path-charset "${p}")
    continue()
  endif()
  set(f "${ROOT}/${p}")
  if(NOT EXISTS "${f}")
    continue()
  endif()
  get_filename_component(ext "${p}" LAST_EXT)
  string(TOLOWER "${ext}" ext)
  string(REGEX REPLACE "^[.]" "" ext "${ext}")
  if("${ext}" IN_LIST SKIP_EXT)
    continue()
  endif()
  file(SIZE "${f}" size)
  if(size EQUAL 0)
    gate_fail(integrity-empty-file "${p}")
    continue()
  endif()
  file(READ "${f}" head LIMIT 3 HEX)
  if(head STREQUAL "efbbbf")
    gate_fail(integrity-bom "${p}")
  endif()
  file(READ "${f}" content)
  string(LENGTH "${content}" len)
  if(NOT len EQUAL size OR content MATCHES "\r")
    gate_fail(integrity-crlf "${p}")
  endif()
  # ASCII rule: every text file under these top-level directories (runner/ since plan 04-04, A-07; host/
  # since plan 15-01, TOOL-08: host/ is a new top-level directory, and both this rule and the SPDX rule
  # below are directory-anchored, so without the widening the phase's own sources would escape them).
  if(p MATCHES "^(data|fixtures|tools|[.]github|core|include|tests|cmake|runner|host)/" OR p MATCHES "^CMakeLists[.]txt$")
    if(content MATCHES "[^\t\n -~]")
      gate_fail(integrity-non-ascii "${p}")
    endif()
  endif()
  # SPDX rule (A-07, plan 04-04): runner/ sources, scripts and CMake files, and the C tool
  # directories tools/pack/, tools/cart/ and, since plan 05-10 (D-21, A-07: the reference PCM
  # converter sat outside the rule), tools/pcm/, since plan 07-03 (D-17) tools/asm/, since plan 08-18 (D-21)
  # tools/bench/ and since plan 14-01 (D-47, 2026-11-26) tools/flatwrap/. The C pattern is anchored to those six directories on purpose: tools/check/controls/denylist-hit/overlay/src/x.c and denylist-nul-hit/overlay/src/x.c
  # are SPDX-less denylist control inputs and must stay outside the rule. tools/bench joined the alternation
  # in plan 08-18 without a fourth control directory of its own, and tools/flatwrap joins it the same way in
  # plan 14-01: the rule stays proved three times (integrity-missing-spdx-tools-c, -asm, -pcm), and the control
  # that plan 14-01 does add is the c-build unlisted-source guard for the new directory.
  # tools/inspect/ joins the alternation in plan 18-02 (TOOL-02) the same way: without it the new directory would be
# silently outside this scan, and a source with no licence line would pass the gate.
# host/ joins the C/H and CMakeLists alternations in plan 15-01 (TOOL-08) the same way: tests/host/ was already
  # covered by the tests/ alternative, host/ was not, and a scope widening is not a new check - the mechanisms
  # (integrity-missing-spdx, integrity-non-ascii) stay proved by the eight committed controls.
  if(p MATCHES "^tools/.*[.](sh|cmake)$" OR p MATCHES "^[.]github/workflows/[^/]*[.]yml$"
     OR p MATCHES "^(core|include|tests|host)/.*[.](c|h|sh|cmake)$" OR p MATCHES "^cmake/.*[.]cmake$"
     OR p MATCHES "^CMakeLists[.]txt$" OR p MATCHES "^(core|include|tests|runner|host)/(.*/)?CMakeLists[.]txt$"
     OR p MATCHES "^runner/.*[.](c|h|sh|cmake)$" OR p MATCHES "^tools/(pack|cart|pcm|asm|bench|flatwrap|inspect)/.*[.](c|h)$")
    string(FIND "${content}" "SPDX-License-Identifier: MIT" pos)
    if(pos EQUAL -1)
      gate_fail(integrity-missing-spdx "${p}")
    endif()
  endif()
endforeach()
if(nfiles EQUAL 0)
  gate_fail(integrity-filelist-empty "FILELIST")
endif()
gate_stat(files "${nfiles}")
gate_finish(integrity)
