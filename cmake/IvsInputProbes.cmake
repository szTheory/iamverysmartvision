# SPDX-License-Identifier: MIT
# Controller probe table generator (D-02, D-03, plan 05-05): reads ONLY the sourced controller literal
# data/input/*.literal.txt and the pinned name set data/expected/input-bits.ids (never data/profiles,
# the generated ivs_ctrl_lines table or any core header) and writes ivs_input_probes.inc to OUT (build
# tree only, never committed) for suite input (tests/input). The expected byte of every test of that
# suite comes from this table or from the four statements of the wiki prose, never from a table the
# core was built from.
# Usage: cmake -DROOT=<repo> -DOUT=<build>/tests/ivs_input_probes.inc -P cmake/IvsInputProbes.cmake
# Input row grammar (the header of the literal file documents it):
#   <NAME> pins=<n,n[,n]> cleared=<HH> wiki=<HH> byte=<HH> src=<SRC-id>@<locator> tag=<tag> conf=C<n>
# Output: one record per control, sorted by bit number (index = bit): name (lower case), bit, cleared,
# byte (FF XOR cleared) and pins (a 16-bit set, bit n for connector pin n), as ivs_ip_ctls[IVS_IP_COUNT]
# of ivs_ip_ctl (tests/input/input_cases.h), and ivs_ip_sources[] with the SHA-256 of the literal file
# (the TAP `# source` line that the tests-inventory step re-hashes).
# Fail closed (message `IvsInputProbes: <file>[:<line>]: <reason>`): no literal file, or more than one;
# a row that does not match the grammar; a name outside the pinned set, or used twice, or missing; a
# pin outside 1..9; wiki different from cleared, or byte different from FF XOR cleared.
cmake_minimum_required(VERSION 3.25)

foreach(required ROOT OUT)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "IvsInputProbes: missing -D${required}")
  endif()
endforeach()

function(ip_fail where msg)
  message(FATAL_ERROR "IvsInputProbes: ${where}: ${msg}")
endfunction()

# Two-digit uppercase C literal of a byte.
function(ip_lit out v)
  math(EXPR h "${v}" OUTPUT_FORMAT HEXADECIMAL)
  string(SUBSTRING "${h}" 2 -1 h)
  string(TOUPPER "${h}" h)
  string(LENGTH "${h}" n)
  if(n LESS 2)
    set(h "0${h}")
  endif()
  set(${out} "0x${h}u" PARENT_SCOPE)
endfunction()

# The pinned names and bits.
set(pin_ids "${ROOT}/data/expected/input-bits.ids")
if(NOT EXISTS "${pin_ids}")
  ip_fail("data/expected/input-bits.ids" "missing")
endif()
file(STRINGS "${pin_ids}" pin_lines)
set(pin_names "")
foreach(l IN LISTS pin_lines)
  if(l STREQUAL "" OR l MATCHES "^#")
    continue()
  endif()
  if(NOT l MATCHES "^IVS_CTRL_([A-Z0-9_]+) ([0-9]|[12][0-9]|30)$")
    ip_fail("data/expected/input-bits.ids" "unsupported line: ${l}")
  endif()
  list(APPEND pin_names "${CMAKE_MATCH_1}")
  set(bit_of_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}")
endforeach()
list(LENGTH pin_names n_ctl)
if(n_ctl EQUAL 0)
  ip_fail("data/expected/input-bits.ids" "no control name")
endif()

file(GLOB ip_files LIST_DIRECTORIES false "${ROOT}/data/input/*.literal.txt")
list(SORT ip_files)
list(LENGTH ip_files n_files)
if(n_files EQUAL 0)
  ip_fail("data/input" "no *.literal.txt file")
endif()
if(n_files GREATER 1)
  ip_fail("data/input" "${n_files} literal files, the suite reads exactly one")
endif()
list(GET ip_files 0 path)
file(RELATIVE_PATH rel "${ROOT}" "${path}")
file(SHA256 "${path}" digest)

file(STRINGS "${path}" lines)
set(seen "")
set(lineno 0)
foreach(l IN LISTS lines)
  math(EXPR lineno "${lineno} + 1")
  if(l STREQUAL "" OR l MATCHES "^#")
    continue()
  endif()
  set(at "${rel}:${lineno}")
  if(NOT l MATCHES "^([A-Z0-9_]+) pins=([0-9](,[0-9])*) cleared=([0-9A-F][0-9A-F]) wiki=([0-9A-F][0-9A-F]) byte=([0-9A-F][0-9A-F]) src=(SRC-[a-z0-9-]+)@([A-Za-z0-9._,-]+) tag=([a-z0-9-]+) conf=(C[0-9]+)$")
    ip_fail("${at}" "row does not match the grammar: ${l}")
  endif()
  set(nm "${CMAKE_MATCH_1}")
  set(pins "${CMAKE_MATCH_2}")
  set(cleared "${CMAKE_MATCH_4}")
  set(wiki "${CMAKE_MATCH_5}")
  set(byte "${CMAKE_MATCH_6}")
  if(NOT nm IN_LIST pin_names)
    ip_fail("${at}" "name '${nm}' is not a name of data/expected/input-bits.ids")
  endif()
  if(nm IN_LIST seen)
    ip_fail("${at}" "name '${nm}' is used twice")
  endif()
  list(APPEND seen "${nm}")
  if(NOT cleared STREQUAL wiki)
    ip_fail("${at}" "${nm}: cleared ${cleared} differs from wiki ${wiki}")
  endif()
  math(EXPR want "255 ^ 0x${cleared}")
  math(EXPR got "0x${byte}")
  if(NOT want EQUAL got)
    ip_fail("${at}" "${nm}: byte ${byte} is not FF XOR ${cleared}")
  endif()
  set(pinset 0)
  string(REPLACE "," ";" pin_list "${pins}")
  foreach(pin IN LISTS pin_list)
    if(pin LESS 1)
      ip_fail("${at}" "${nm}: pin ${pin} is not a connector pin 1..9")
    endif()
    math(EXPR pinset "${pinset} | (1 << ${pin})")
  endforeach()
  string(TOLOWER "${nm}" lower)
  set(rec_${bit_of_${nm}} "{\"${lower}\", ${bit_of_${nm}}u, 0x${cleared}u, 0x${byte}u, ${pinset}u}")
endforeach()
foreach(nm IN LISTS pin_names)
  if(NOT nm IN_LIST seen)
    ip_fail("${rel}" "control ${nm} has no row")
  endif()
endforeach()

# The bits of the pinned set must be exactly 0..n-1 once each (the record index is the bit).
math(EXPR last "${n_ctl} - 1")
set(recs "")
foreach(b RANGE 0 ${last})
  if(NOT DEFINED rec_${b})
    ip_fail("data/expected/input-bits.ids" "no control has bit ${b} (the bits must be 0..${last})")
  endif()
  string(APPEND recs "  ${rec_${b}},\n")
endforeach()

set(out_c "/* Generated by cmake/IvsInputProbes.cmake from the controller literal (D-02). */\n")
string(APPEND out_c "/* Build output only: never committed, never edited. */\n")
string(APPEND out_c "#define IVS_IP_COUNT ${n_ctl}u\n")
string(APPEND out_c "const size_t ivs_ip_count = IVS_IP_COUNT;\n")
string(APPEND out_c "const ivs_ip_ctl ivs_ip_ctls[IVS_IP_COUNT] = {\n${recs}};\n")
string(APPEND out_c "static const ivs_test_source ivs_ip_sources[] = {\n  {\"${rel}\", \"${digest}\"},\n};\n")
file(WRITE "${OUT}" "${out_c}")
