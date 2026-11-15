# SPDX-License-Identifier: MIT
# Turns data/expected/state-cuts.ids (the 18 named hard-point cuts and their pinned ticks) into a C include of X-macro lines, so the tests of
# suite state read every cut id, program, tick and family from the reviewed data file and never retype a tick (plan 08-14, D-16, D-17,
# threat T-08-35; the precedent is tests/ivsn-literals-inc.cmake). Pure: no host path, no time, no core input. Build output only, never
# committed.
# Usage: cmake -DIN=<file> -DOUT=<file.inc> -P state-cuts-inc.cmake
# Accepted lines: blank lines and `#` comment lines (skipped) and
#   <cut-id> <program> <pinned-tick> <family>
# with a cut id HP-<upper case letters, digits, dashes>, a program of letters, digits, underscores and dashes, the pinned tick a decimal
# number or the token pending-08-14, and the family cpu-points, stic-points or device-points. Any other line stops the build: a cut the test
# cannot read must not be dropped silently. The output is one line IVSN_SOURCE("<repo path>", "<sha256 of the file>") (the `# source` line of
# the result file, re-hashed by the gate step tests-inventory) followed by STATE_CUT("<cut>", "<program>", <tick>, "<family>") per line,
# where a pending tick is written as the largest 64-bit value (the test treats it as not pinned yet).
cmake_minimum_required(VERSION 3.25)
if(NOT DEFINED IN OR NOT DEFINED OUT)
  message(FATAL_ERROR "state-cuts-inc: IN and OUT are required")
endif()
if(NOT EXISTS "${IN}")
  message(FATAL_ERROR "state-cuts-inc: ${IN} does not exist")
endif()
get_filename_component(in_name "${IN}" NAME)
file(SHA256 "${IN}" in_sha)
file(STRINGS "${IN}" lines)
set(body "/* Generated from data/expected/${in_name} by tests/state-cuts-inc.cmake: build output only, never committed. */\n")
string(APPEND body "IVSN_SOURCE(\"data/expected/${in_name}\", \"${in_sha}\")\n")
set(n 0)
set(lineno 0)
set(seen "")
foreach(l IN LISTS lines)
  math(EXPR lineno "${lineno} + 1")
  if(l STREQUAL "" OR l MATCHES "^#")
    continue()
  endif()
  if(NOT l MATCHES "^(HP-[A-Z0-9-]+) ([A-Za-z0-9_-]+) (pending-08-14|[0-9]+) (cpu-points|stic-points|device-points)$")
    message(FATAL_ERROR "state-cuts-inc: ${IN}:${lineno}: unsupported line: ${l}")
  endif()
  set(cut "${CMAKE_MATCH_1}")
  set(prog "${CMAKE_MATCH_2}")
  set(tick "${CMAKE_MATCH_3}")
  set(family "${CMAKE_MATCH_4}")
  if(cut IN_LIST seen)
    message(FATAL_ERROR "state-cuts-inc: ${IN}:${lineno}: duplicate cut ${cut}")
  endif()
  list(APPEND seen "${cut}")
  if(tick STREQUAL "pending-08-14")
    set(tick "UINT64_C(18446744073709551615)")
  else()
    set(tick "UINT64_C(${tick})")
  endif()
  string(APPEND body "STATE_CUT(\"${cut}\", \"${prog}\", ${tick}, \"${family}\")\n")
  math(EXPR n "${n} + 1")
endforeach()
if(n EQUAL 0)
  message(FATAL_ERROR "state-cuts-inc: ${IN} has no cut line")
endif()
file(WRITE "${OUT}" "${body}")
