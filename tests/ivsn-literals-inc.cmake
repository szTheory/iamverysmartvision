# SPDX-License-Identifier: MIT
# Turns the hand-derived literals of the IVSN v1 snapshot container into C includes of X-macro lines, so the tests of suite state read
# every expected number and text from the reviewed data files instead of retyping them (plan 08-11, D-16; the precedent is
# tests/expect-inc.cmake). Pure: no host path, no time, no core input. Build output only, never committed.
# Usage: cmake -DIN=<file> -DOUT=<file.inc> [-DFORM=literals|schema] -P ivsn-literals-inc.cmake
#
# FORM=literals (the default) reads data/expected/ivsn-literals.ids. Accepted lines: blank lines and `#` comment lines (skipped) and
#   <key> <value>        key of lower-case letters, digits, dots and dashes; value of letters and digits only (a decimal number or
#                        a hex string); the key may occur once
# and writes one line IVSN_SOURCE("<repo path>", "<sha256 of the file>") followed by IVSN_LIT("<key>", "<value>") per line. Any other
# line stops the build: a literal the test cannot read must not be dropped silently. A file without a literal line stops it too.
# FORM=schema reads data/state/schema.literal.txt (the canonical descriptor text of the 88 table rows, the preimage of the schema
# digest) and writes IVSN_SOURCE("<repo path>", "<sha256>") then IVSN_SCHEMA_LINE("<line>") per line, the line without its newline; a
# line may hold only letters, digits, dots, underscores and single blanks, so nothing needs C escaping. The first line must be
# `IVSSCHEMA 1`, so a file that is not the schema stops the build.
# The SHA-256 of the file travels with it so that the result file of the suite can carry a `# source` line that the gate step
# tests-inventory re-hashes: the oracle that the test read is the oracle that is in the tree.
cmake_minimum_required(VERSION 3.25)
if(NOT DEFINED IN OR NOT DEFINED OUT)
  message(FATAL_ERROR "ivsn-literals-inc: IN and OUT are required")
endif()
if(NOT DEFINED FORM)
  set(FORM literals)
endif()
if(NOT FORM STREQUAL "literals" AND NOT FORM STREQUAL "schema")
  message(FATAL_ERROR "ivsn-literals-inc: FORM must be literals or schema (got ${FORM})")
endif()
if(NOT EXISTS "${IN}")
  message(FATAL_ERROR "ivsn-literals-inc: ${IN} does not exist")
endif()
get_filename_component(in_name "${IN}" NAME)
if(FORM STREQUAL "literals")
  set(repo_path "data/expected/${in_name}")
else()
  set(repo_path "data/state/${in_name}")
endif()
file(SHA256 "${IN}" in_sha)
file(STRINGS "${IN}" lines)
set(body "/* Generated from ${repo_path} by tests/ivsn-literals-inc.cmake (FORM=${FORM}): build output only, never committed. */\n")
string(APPEND body "IVSN_SOURCE(\"${repo_path}\", \"${in_sha}\")\n")
set(n 0)
set(lineno 0)
set(seen "")
foreach(l IN LISTS lines)
  math(EXPR lineno "${lineno} + 1")
  if(FORM STREQUAL "literals")
    if(l STREQUAL "" OR l MATCHES "^#")
      continue()
    endif()
    if(NOT l MATCHES "^([a-z0-9][a-z0-9.-]*) ([0-9A-Za-z]+)$")
      message(FATAL_ERROR "ivsn-literals-inc: ${IN}:${lineno}: unsupported line: ${l}")
    endif()
    set(key "${CMAKE_MATCH_1}")
    set(val "${CMAKE_MATCH_2}")
    if(key IN_LIST seen)
      message(FATAL_ERROR "ivsn-literals-inc: ${IN}:${lineno}: duplicate key ${key}")
    endif()
    list(APPEND seen "${key}")
    string(APPEND body "IVSN_LIT(\"${key}\", \"${val}\")\n")
  else()
    if(lineno EQUAL 1 AND NOT l STREQUAL "IVSSCHEMA 1")
      message(FATAL_ERROR "ivsn-literals-inc: ${IN}:1: the first line must be 'IVSSCHEMA 1' (got: ${l})")
    endif()
    if(NOT l MATCHES "^[A-Za-z0-9._]+( [A-Za-z0-9._]+)*$")
      message(FATAL_ERROR "ivsn-literals-inc: ${IN}:${lineno}: unsupported line: ${l}")
    endif()
    string(APPEND body "IVSN_SCHEMA_LINE(\"${l}\")\n")
  endif()
  math(EXPR n "${n} + 1")
endforeach()
if(n EQUAL 0)
  message(FATAL_ERROR "ivsn-literals-inc: ${IN} has no literal line")
endif()
file(WRITE "${OUT}" "${body}")
