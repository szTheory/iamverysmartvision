# SPDX-License-Identifier: MIT
# Turns the pinned controller input names (data/expected/input-bits.ids, plan 04-13) into a C
# include of X-macro lines IVS_INPUT_BIT(<NAME>, <bit>), NAME without the IVS_CTRL_ prefix, so
# suite runner (plan 04-17, runner.parse.input-names-complete) checks the scenario parser's name
# table against the pin file itself: a newly pinned name without a parser entry fails the test.
# Build output only, never committed.
# Usage: cmake -DIN=<input-bits.ids> -DOUT=<file.inc> -P input-bits-inc.cmake
# Accepted lines: `IVS_CTRL_<NAME> <bit>` with NAME [A-Z0-9_]+ and bit 0..30; '#' lines and blank
# lines are skipped; any other line, and a file without one name, stop the build.
cmake_minimum_required(VERSION 3.25)
if(NOT DEFINED IN OR NOT DEFINED OUT)
  message(FATAL_ERROR "input-bits-inc: IN and OUT are required")
endif()
file(STRINGS "${IN}" lines)
set(body "/* Generated from data/expected/input-bits.ids by tests/input-bits-inc.cmake: build output only, never committed. */\n")
set(n 0)
set(lineno 0)
foreach(l IN LISTS lines)
  math(EXPR lineno "${lineno} + 1")
  if(l STREQUAL "" OR l MATCHES "^#")
    continue()
  endif()
  if(NOT l MATCHES "^IVS_CTRL_([A-Z0-9_]+) ([0-9]|[12][0-9]|30)$")
    message(FATAL_ERROR "input-bits-inc: ${IN}:${lineno}: unsupported line: ${l}")
  endif()
  string(APPEND body "IVS_INPUT_BIT(${CMAKE_MATCH_1}, ${CMAKE_MATCH_2})\n")
  math(EXPR n "${n} + 1")
endforeach()
if(n EQUAL 0)
  message(FATAL_ERROR "input-bits-inc: ${IN} has no name")
endif()
file(WRITE "${OUT}" "${body}")
