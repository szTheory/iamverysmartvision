# SPDX-License-Identifier: MIT
# Controller table generator (D-02, D-03, plan 05-05): reads every data/profiles/*.json in sorted order
# (the same order as the registry of cmake/IvsMemmapTable.cmake, so a profile's index is the same in
# every generated table) and writes ivs_ctrl_lines.inc to OUT (build tree only, never committed). The
# only includer is core/src/machine/machine_input.c. No C JSON parser exists (H14).
# Usage: cmake -DROOT=<tree> -DOUT=<file> [-DMUTATE=<control>] -P cmake/IvsCtrlTable.cmake
# The output is a pure function of the profile files and data/expected/input-bits.ids: no host path,
# no time.
#
# Inputs:
#   data/expected/input-bits.ids   the pinned control names and bit numbers, read with the strict line
#                                  grammar of tests/input-bits-inc.cmake (`IVS_CTRL_<NAME> <bit>`).
#                                  The bits must be exactly 0..n-1 once each (the table index is the
#                                  bit number).
#   input.control.<name>           one field per pinned name (the name in lower case), value
#                                  {"lines": "$XX"}: the line mask of the control, 01..FF, the bits of the
#                                  port byte a closed switch pulls low.
#   input.line_model               "ground-union" (the only model implemented): the byte a read returns is
#                                  FF XOR the OR of the masks of every closed control.
#   input.controllers              a number; a profile with more than 0 must carry the table.
#
# Fail closed (message `IvsCtrlTable: <profile>: <field>: <reason>`): a missing or malformed pin file or
# bit set; a control name of the pin set with no field, or an input.control.* field outside the pin set
# (missing or extra name); a value that is not an object with the single key "lines"; a mask that is
# not $XX with 01..FF; an unknown input.line_model; a profile whose input.controllers is above 0 and
# that has no input.line_model or no table; a -DMUTATE target that is not a control.
#
# -DMUTATE=<control> (named fault GEN_CTRL_MUTATION, tests/faults.cmake): after validation the mask of
# that control (lower case name) has its lowest bit flipped, so the table the core is built against
# differs from the literal in exactly one bit of one control.
#
# Output: IVS_CTRL_PROFILE_COUNT, IVS_CTRL_COUNT, ivs_ctrl_lines[profile][IVS_CTRL_COUNT] (u8, index = bit
# number) and _Static_asserts tying the count to the pin.
cmake_minimum_required(VERSION 3.25)

foreach(required ROOT OUT)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "IvsCtrlTable: missing -D${required}")
  endif()
endforeach()

set(CT_CTX "")
function(ct_fail msg)
  message(FATAL_ERROR "IvsCtrlTable: ${CT_CTX}: ${msg}")
endfunction()

# JSON type of a path in a document, or MISSING.
function(ct_type out doc)
  string(JSON t ERROR_VARIABLE e TYPE "${doc}" ${ARGN})
  if(NOT "${e}" STREQUAL "NOTFOUND")
    set(t MISSING)
  endif()
  set(${out} "${t}" PARENT_SCOPE)
endfunction()

# "$XX" -> numeric value 1..255.
function(ct_mask out s what)
  if(NOT s MATCHES "^[$][0-9A-F][0-9A-F]$")
    ct_fail("${what} '${s}' is not a \$XX hex byte")
  endif()
  string(SUBSTRING "${s}" 1 2 digits)
  math(EXPR v "0x${digits}")
  if(v EQUAL 0)
    ct_fail("${what} '${s}' is zero: a control that grounds no line (mask 01..FF)")
  endif()
  set(${out} "${v}" PARENT_SCOPE)
endfunction()

# Two-digit uppercase C literal of a byte.
function(ct_lit out v)
  math(EXPR h "${v}" OUTPUT_FORMAT HEXADECIMAL)
  string(SUBSTRING "${h}" 2 -1 h)
  string(TOUPPER "${h}" h)
  string(LENGTH "${h}" n)
  if(n LESS 2)
    set(h "0${h}")
  endif()
  set(${out} "0x${h}u" PARENT_SCOPE)
endfunction()

# The pinned names and bit numbers.
set(CT_PIN "${ROOT}/data/expected/input-bits.ids")
set(CT_CTX "data/expected/input-bits.ids")
if(NOT EXISTS "${CT_PIN}")
  ct_fail("missing")
endif()
file(STRINGS "${CT_PIN}" pin_lines)
set(ct_names "")
set(ct_bits "")
set(lineno 0)
foreach(l IN LISTS pin_lines)
  math(EXPR lineno "${lineno} + 1")
  if(l STREQUAL "" OR l MATCHES "^#")
    continue()
  endif()
  if(NOT l MATCHES "^IVS_CTRL_([A-Z0-9_]+) ([0-9]|[12][0-9]|30)$")
    ct_fail("line ${lineno}: unsupported line: ${l}")
  endif()
  string(TOLOWER "${CMAKE_MATCH_1}" lower)
  set(bit "${CMAKE_MATCH_2}")
  if(lower IN_LIST ct_names)
    ct_fail("line ${lineno}: duplicate name ${lower}")
  endif()
  list(APPEND ct_names "${lower}")
  list(APPEND ct_bits "${bit}")
endforeach()
list(LENGTH ct_names n_ctl)
if(n_ctl EQUAL 0)
  ct_fail("no control name")
endif()
# The bits are exactly 0..n-1 once each: the table index is the bit number.
math(EXPR ct_last "${n_ctl} - 1")
foreach(i RANGE 0 ${ct_last})
  set(ct_slot_${i} "")
endforeach()
set(k 0)
foreach(nm IN LISTS ct_names)
  list(GET ct_bits ${k} bit)
  if(bit GREATER_EQUAL n_ctl)
    ct_fail("bit ${bit} of ${nm} is outside 0..${n_ctl}: the bits must be 0..n-1")
  endif()
  if(NOT "${ct_slot_${bit}}" STREQUAL "")
    ct_fail("bit ${bit} is used by ${ct_slot_${bit}} and ${nm}")
  endif()
  set(ct_slot_${bit} "${nm}")
  math(EXPR k "${k} + 1")
endforeach()

if(DEFINED MUTATE AND NOT MUTATE STREQUAL "" AND NOT MUTATE IN_LIST ct_names)
  set(CT_CTX "-DMUTATE")
  ct_fail("target '${MUTATE}' is not a control")
endif()

set(known_models ground-union)

file(GLOB ct_profiles RELATIVE "${ROOT}" "${ROOT}/data/profiles/*.json")
list(SORT ct_profiles)
if(NOT ct_profiles)
  set(CT_CTX "data/profiles")
  ct_fail("no profile file")
endif()
list(LENGTH ct_profiles n_profiles)

set(table_c "")
foreach(rel IN LISTS ct_profiles)
  file(READ "${ROOT}/${rel}" doc)
  set(CT_CTX "${rel}: id")
  ct_type(t "${doc}" id)
  if(NOT t STREQUAL "STRING")
    ct_fail("profile id must be a string")
  endif()
  string(JSON pid GET "${doc}" id)

  # input.controllers: a number.
  set(CT_CTX "${rel}: input.controllers")
  ct_type(t "${doc}" fields input.controllers value)
  if(NOT t STREQUAL "NUMBER")
    ct_fail("value must be a number")
  endif()
  string(JSON controllers GET "${doc}" fields input.controllers value)

  # Which input.control.* fields the profile has.
  set(have "")
  string(JSON nf LENGTH "${doc}" fields)
  if(nf GREATER 0)
    math(EXPR lastf "${nf} - 1")
    foreach(i RANGE ${lastf})
      string(JSON fk MEMBER "${doc}" fields ${i})
      if(fk MATCHES "^input[.]control[.](.+)$")
        list(APPEND have "${CMAKE_MATCH_1}")
      endif()
    endforeach()
  endif()

  set(row_c "")
  if(controllers GREATER 0 OR have)
    set(CT_CTX "${rel}: input.line_model")
    ct_type(t "${doc}" fields input.line_model value)
    if(t STREQUAL "MISSING")
      ct_fail("missing (a profile with controllers carries the table and its line model)")
    endif()
    if(NOT t STREQUAL "STRING")
      ct_fail("value must be a string")
    endif()
    string(JSON model GET "${doc}" fields input.line_model value)
    if(NOT model IN_LIST known_models)
      ct_fail("value '${model}' is not a known line model (${known_models})")
    endif()
    foreach(h IN LISTS have)
      if(NOT h IN_LIST ct_names)
        set(CT_CTX "${rel}: input.control.${h}")
        ct_fail("extra field: '${h}' is not a name of data/expected/input-bits.ids")
      endif()
    endforeach()
    foreach(i RANGE 0 ${ct_last})
      set(nm "${ct_slot_${i}}")
      set(CT_CTX "${rel}: input.control.${nm}")
      ct_type(t "${doc}" fields input.control.${nm} value)
      if(t STREQUAL "MISSING")
        ct_fail("missing")
      endif()
      if(NOT t STREQUAL "OBJECT")
        ct_fail("value must be an object {\"lines\": \"\$XX\"}")
      endif()
      string(JSON obj GET "${doc}" fields input.control.${nm} value)
      string(JSON nk LENGTH "${obj}")
      if(NOT nk EQUAL 1)
        ct_fail("value has ${nk} keys, want exactly one key 'lines'")
      endif()
      string(JSON k0 MEMBER "${obj}" 0)
      if(NOT k0 STREQUAL "lines")
        ct_fail("unknown key '${k0}' (the only key is 'lines')")
      endif()
      ct_type(t "${obj}" lines)
      if(NOT t STREQUAL "STRING")
        ct_fail("lines must be a string")
      endif()
      string(JSON s GET "${obj}" lines)
      ct_mask(mask "${s}" "lines")
      if(DEFINED MUTATE AND "${MUTATE}" STREQUAL "${nm}")
        math(EXPR mask "${mask} ^ 1")
      endif()
      ct_lit(lit "${mask}")
      if(i EQUAL 0)
        set(row_c "${lit}")
      else()
        string(APPEND row_c ", ${lit}")
      endif()
    endforeach()
  else()
    # No controller and no table: a zero row (the core never derives pins for it).
    foreach(i RANGE 0 ${ct_last})
      if(i EQUAL 0)
        set(row_c "0x00u")
      else()
        string(APPEND row_c ", 0x00u")
      endif()
    endforeach()
  endif()
  string(APPEND table_c "  { /* ${pid} */ ${row_c} },\n")
endforeach()

set(out_c "/* Generated by cmake/IvsCtrlTable.cmake from the files in data/profiles (D-02). */\n")
string(APPEND out_c "/* Build output only: never committed, never edited. */\n")
string(APPEND out_c "#define IVS_CTRL_PROFILE_COUNT ${n_profiles}u\n")
string(APPEND out_c "#define IVS_CTRL_COUNT ${n_ctl}u\n")
string(APPEND out_c "_Static_assert(IVS_CTRL_COUNT == 31u, \"the pinned control set has 31 names (data/expected/input-bits.ids)\");\n")
string(APPEND out_c "static const uint8_t ivs_ctrl_lines[IVS_CTRL_PROFILE_COUNT][IVS_CTRL_COUNT] = {\n${table_c}};\n")
string(APPEND out_c "_Static_assert(sizeof ivs_ctrl_lines / sizeof ivs_ctrl_lines[0] == IVS_CTRL_PROFILE_COUNT, \"one table row per profile\");\n")
file(WRITE "${OUT}" "${out_c}")
