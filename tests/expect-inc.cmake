# SPDX-License-Identifier: MIT
# Turns the assert lines of a predicate file (data/golden/<id>.expect, D-11, plan 04-16) into a C
# include of X-macro lines, so a test reads the expected values from the reviewed file instead of
# retyping them. Build output only, never committed.
# Usage: cmake -DIN=<file.expect> -DOUT=<file.inc> -P expect-inc.cmake
# Accepted body lines (the runner grammar of plan 04-17, the subset the in-process tests need):
#   assert halted == yes|no      -> IVS_EXPECT_HALTED(1|0)
#   assert reg R<0-7> == HHHH    -> IVS_EXPECT_REG(<n>u, 0xHHHHu)
#   assert mem AAAA == HHHH      -> IVS_EXPECT_MEM(0xAAAAu, 0xHHHHu)
# HHHH and AAAA are 4 upper-case hex digits. '#' lines and blank lines are skipped. Any other line
# (another subject, '!=', a range, lower-case hex) stops the build: a clause the test cannot check
# must not be dropped silently. A file without one assert line also stops the build.
if(NOT DEFINED IN OR NOT DEFINED OUT)
  message(FATAL_ERROR "expect-inc: IN and OUT are required")
endif()
file(STRINGS "${IN}" lines)
get_filename_component(in_name "${IN}" NAME)
set(body "/* Generated from data/golden/${in_name} by tests/expect-inc.cmake: build output only, never committed. */\n")
set(n 0)
set(lineno 0)
foreach(l IN LISTS lines)
  math(EXPR lineno "${lineno} + 1")
  if(l STREQUAL "" OR l MATCHES "^#")
    continue()
  endif()
  if(l MATCHES "^assert halted == (yes|no)$")
    if(CMAKE_MATCH_1 STREQUAL "yes")
      string(APPEND body "IVS_EXPECT_HALTED(1)\n")
    else()
      string(APPEND body "IVS_EXPECT_HALTED(0)\n")
    endif()
  elseif(l MATCHES "^assert reg R([0-7]) == ([0-9A-F][0-9A-F][0-9A-F][0-9A-F])$")
    string(APPEND body "IVS_EXPECT_REG(${CMAKE_MATCH_1}u, 0x${CMAKE_MATCH_2}u)\n")
  elseif(l MATCHES "^assert mem ([0-9A-F][0-9A-F][0-9A-F][0-9A-F]) == ([0-9A-F][0-9A-F][0-9A-F][0-9A-F])$")
    string(APPEND body "IVS_EXPECT_MEM(0x${CMAKE_MATCH_1}u, 0x${CMAKE_MATCH_2}u)\n")
  else()
    message(FATAL_ERROR "expect-inc: ${IN}:${lineno}: unsupported line: ${l}")
  endif()
  math(EXPR n "${n} + 1")
endforeach()
if(n EQUAL 0)
  message(FATAL_ERROR "expect-inc: ${IN} has no assert line")
endif()
file(WRITE "${OUT}" "${body}")
