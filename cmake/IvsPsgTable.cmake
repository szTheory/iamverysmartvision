# SPDX-License-Identifier: MIT
# PSG parameter generator (D-08, D-12, A-06, plan 05-04): reads every data/profiles/*.json in sorted
# order (the same order as the registry of cmake/IvsMemmapTable.cmake, so a profile's index is the
# same in both tables) and writes ivs_psg_params.inc to OUT (build tree only, never committed). The
# only includer is core/src/psg/psg_regs.c. No C JSON parser exists (H14).
# Usage: cmake -DROOT=<tree> -DOUT=<file> -P cmake/IvsPsgTable.cmake
# The output is a pure function of the profile files: no host path, no time.
#
# Profile fields read (every one carries the profile record's tag, cite and conflict):
#   psg.regmap   array of exactly 16 records {addr, name, gi_octal, wmask, rmask}, ascending CPU
#                addresses "$01F0".."$01FF" (hex values are "$XXXX": four uppercase digits);
#                name is lowercase letters and dashes, unique; gi_octal is the data sheet register
#                number in octal ("R0".."R17"), unique; wmask and rmask are at most "$00FF": only the
#                low byte of a bus word is driven (C64). A store keeps value & wmask, a read gives the
#                store & rmask.
#   psg.mirrors  "none" (C74): the only value implemented.
#   input.line_idle        "$XX" (D-02): the low byte a port in input mode reads with no switch closed.
#   input.port_output_read "latch" (C65): the only value implemented; a port in output mode reads its latch.
#   psg.mixer.port_dir_bits  {"A": a, "B": b, "output": o} (D-04, C73): the bit numbers 0..7 of the mixer
#                register that select the direction of port 0 (A) and port 1 (B), and the bit value (0 or 1)
#                that means output; A and B differ.
# Later plans append their psg.* fields to this generator and to ivs_psg_params.
#
# Fail closed (message `IvsPsgTable: <profile>: <field>: <reason>`): a profile without psg.regmap
# or psg.mirrors; a regmap that is not an array of exactly 16 records; a record that is not an
# object, or has a missing or unknown key; an addr that is not the next of $01F0..$01FF in order
# (a gap, a duplicate, a wrong order, or a value outside the block); a name or gi_octal of a wrong
# shape or used twice; a mask that is not a $XXXX word or has a bit above the low byte; a mirrors
# value other than "none"; (plan 05-05) an input.line_idle that is not $XX, an input.port_output_read other
# than "latch", a psg.mixer.port_dir_bits that is not an object with exactly the keys A, B and output,
# a bit number above 7 or non-numeric, A equal to B, or an output value other than 0 or 1.
#
# Output: IVS_PSG_PROFILE_COUNT, ivs_psg_params_table[profile] ({reg_wmask[16]}, {reg_rmask[16]},
# port_idle, port_output_read_latch, port_dir_bit[2], port_dir_output), and _Static_asserts on the counts.
cmake_minimum_required(VERSION 3.25)

foreach(required ROOT OUT)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "IvsPsgTable: missing -D${required}")
  endif()
endforeach()

set(PT_CTX "")
function(pt_fail msg)
  message(FATAL_ERROR "IvsPsgTable: ${PT_CTX}: ${msg}")
endfunction()

# JSON type of a path in a document, or MISSING.
function(pt_type out doc)
  string(JSON t ERROR_VARIABLE e TYPE "${doc}" ${ARGN})
  if(NOT "${e}" STREQUAL "NOTFOUND")
    set(t MISSING)
  endif()
  set(${out} "${t}" PARENT_SCOPE)
endfunction()

# The member keys of an object must be exactly the listed keys (unknown and missing both fail).
function(pt_keys doc what)
  string(JSON n LENGTH "${doc}")
  set(seen "")
  if(n GREATER 0)
    math(EXPR last "${n} - 1")
    foreach(i RANGE ${last})
      string(JSON k MEMBER "${doc}" ${i})
      if(NOT k IN_LIST ARGN)
        pt_fail("unknown ${what} key '${k}'")
      endif()
      list(APPEND seen "${k}")
    endforeach()
  endif()
  foreach(k IN LISTS ARGN)
    if(NOT k IN_LIST seen)
      pt_fail("missing ${what} key '${k}'")
    endif()
  endforeach()
endfunction()

# Required string at a path.
function(pt_str out doc what)
  pt_type(t "${doc}" ${ARGN})
  if(NOT t STREQUAL "STRING")
    pt_fail("${what} must be a string")
  endif()
  string(JSON v GET "${doc}" ${ARGN})
  set(${out} "${v}" PARENT_SCOPE)
endfunction()

# "$XXXX" -> numeric value.
function(pt_hex out s what)
  if(NOT s MATCHES "^[$][0-9A-F][0-9A-F][0-9A-F][0-9A-F]$")
    pt_fail("${what} '${s}' is not a \$XXXX hex word")
  endif()
  string(SUBSTRING "${s}" 1 4 digits)
  math(EXPR v "0x${digits}")
  set(${out} "${v}" PARENT_SCOPE)
endfunction()

# "$XX" -> numeric value.
function(pt_byte out s what)
  if(NOT s MATCHES "^[$][0-9A-F][0-9A-F]$")
    pt_fail("${what} '${s}' is not a \$XX hex byte")
  endif()
  string(SUBSTRING "${s}" 1 2 digits)
  math(EXPR v "0x${digits}")
  set(${out} "${v}" PARENT_SCOPE)
endfunction()

# Four-digit uppercase C literal of a number.
function(pt_lit out v)
  math(EXPR h "${v}" OUTPUT_FORMAT HEXADECIMAL)
  string(SUBSTRING "${h}" 2 -1 h)
  string(TOUPPER "${h}" h)
  string(LENGTH "${h}" n)
  while(n LESS 4)
    set(h "0${h}")
    math(EXPR n "${n} + 1")
  endwhile()
  set(${out} "0x${h}u" PARENT_SCOPE)
endfunction()

file(GLOB pt_profiles RELATIVE "${ROOT}" "${ROOT}/data/profiles/*.json")
list(SORT pt_profiles)
if(NOT pt_profiles)
  set(PT_CTX "data/profiles")
  pt_fail("no profile file")
endif()
list(LENGTH pt_profiles n_profiles)

set(table_c "")
foreach(rel IN LISTS pt_profiles)
  file(READ "${ROOT}/${rel}" doc)
  set(PT_CTX "${rel}: id")
  pt_str(pid "${doc}" "profile id" id)

  # psg.mirrors: only "none".
  set(PT_CTX "${rel}: psg.mirrors")
  pt_type(t "${doc}" fields psg.mirrors value)
  if(t STREQUAL "MISSING")
    pt_fail("missing")
  endif()
  pt_str(mirrors "${doc}" "value" fields psg.mirrors value)
  if(NOT mirrors STREQUAL "none")
    pt_fail("value '${mirrors}' is not 'none' (only an undecoded repeat is implemented, C74)")
  endif()

  # The port parameters (plan 05-05): input.line_idle, input.port_output_read, psg.mixer.port_dir_bits.
  set(PT_CTX "${rel}: input.line_idle")
  pt_type(t "${doc}" fields input.line_idle value)
  if(t STREQUAL "MISSING")
    pt_fail("missing")
  endif()
  pt_str(s "${doc}" "value" fields input.line_idle value)
  pt_byte(port_idle "${s}" "value")
  set(PT_CTX "${rel}: input.port_output_read")
  pt_type(t "${doc}" fields input.port_output_read value)
  if(t STREQUAL "MISSING")
    pt_fail("missing")
  endif()
  pt_str(s "${doc}" "value" fields input.port_output_read value)
  if(NOT s STREQUAL "latch")
    pt_fail("value '${s}' is not 'latch' (only the latch read of an output port is implemented, C65)")
  endif()
  set(PT_CTX "${rel}: psg.mixer.port_dir_bits")
  pt_type(t "${doc}" fields psg.mixer.port_dir_bits value)
  if(t STREQUAL "MISSING")
    pt_fail("missing")
  endif()
  if(NOT t STREQUAL "OBJECT")
    pt_fail("value must be an object {\"A\": n, \"B\": n, \"output\": 0 or 1}")
  endif()
  string(JSON dobj GET "${doc}" fields psg.mixer.port_dir_bits value)
  pt_keys("${dobj}" "value" A B output)
  foreach(dk A B output)
    pt_type(t "${dobj}" ${dk})
    if(NOT t STREQUAL "NUMBER")
      pt_fail("${dk} must be a number")
    endif()
    string(JSON dv GET "${dobj}" ${dk})
    if(NOT dv MATCHES "^[0-9]$")
      pt_fail("${dk} '${dv}' is not a bit number 0..7 (or the output value 0 or 1)")
    endif()
    set(dir_${dk} "${dv}")
  endforeach()
  if(dir_A GREATER 7 OR dir_B GREATER 7)
    pt_fail("a direction bit number is above 7")
  endif()
  if(dir_A EQUAL dir_B)
    pt_fail("A and B name the same bit ${dir_A}")
  endif()
  if(dir_output GREATER 1)
    pt_fail("output '${dir_output}' is not 0 or 1")
  endif()
  pt_lit(idle_l "${port_idle}")
  string(REGEX REPLACE "^0x00" "0x" idle_l "${idle_l}")

  # psg.regmap: sixteen consecutive records.
  set(PT_CTX "${rel}: psg.regmap")
  pt_type(t "${doc}" fields psg.regmap value)
  if(t STREQUAL "MISSING")
    pt_fail("missing")
  endif()
  if(NOT t STREQUAL "ARRAY")
    pt_fail("value must be an array of 16 records")
  endif()
  string(JSON nrec LENGTH "${doc}" fields psg.regmap value)
  if(NOT nrec EQUAL 16)
    pt_fail("has ${nrec} records, want exactly 16 (addresses \$01F0..\$01FF)")
  endif()
  set(wm_c "")
  set(rm_c "")
  set(names "")
  set(gis "")
  foreach(i RANGE 0 15)
    set(PT_CTX "${rel}: psg.regmap[${i}]")
    pt_type(t "${doc}" fields psg.regmap value ${i})
    if(NOT t STREQUAL "OBJECT")
      pt_fail("record must be an object")
    endif()
    string(JSON rec GET "${doc}" fields psg.regmap value ${i})
    pt_keys("${rec}" "record" addr name gi_octal wmask rmask)
    pt_str(s "${rec}" "addr" addr)
    pt_hex(addr "${s}" "addr")
    math(EXPR want "0x01F0 + ${i}")
    if(NOT addr EQUAL want)
      pt_lit(want_l "${want}")
      pt_fail("addr '${s}' is not the next address ${want_l} of the block \$01F0..\$01FF in order")
    endif()
    pt_str(nm "${rec}" "name" name)
    if(NOT nm MATCHES "^[a-z][a-z-]*$")
      pt_fail("name '${nm}' is not lowercase letters and dashes")
    endif()
    if(nm IN_LIST names)
      pt_fail("name '${nm}' is used twice")
    endif()
    list(APPEND names "${nm}")
    pt_str(gi "${rec}" "gi_octal" gi_octal)
    if(NOT gi MATCHES "^R[0-7]+$")
      pt_fail("gi_octal '${gi}' is not R followed by octal digits")
    endif()
    if(gi IN_LIST gis)
      pt_fail("gi_octal '${gi}' is used twice")
    endif()
    list(APPEND gis "${gi}")
    pt_str(s "${rec}" "wmask" wmask)
    pt_hex(wmask "${s}" "wmask")
    pt_str(s "${rec}" "rmask" rmask)
    pt_hex(rmask "${s}" "rmask")
    if(wmask GREATER 255 OR rmask GREATER 255)
      pt_fail("a mask has a bit above the low byte (only bits 7..0 of a bus word are driven, C64)")
    endif()
    pt_lit(wm_l "${wmask}")
    pt_lit(rm_l "${rmask}")
    if(i EQUAL 0)
      set(wm_c "${wm_l}")
      set(rm_c "${rm_l}")
    else()
      string(APPEND wm_c ", ${wm_l}")
      string(APPEND rm_c ", ${rm_l}")
    endif()
  endforeach()
  string(APPEND table_c "  { /* ${pid} */\n    {${wm_c}},\n    {${rm_c}},\n    ${idle_l}, 1u, {${dir_A}u, ${dir_B}u}, ${dir_output}u\n  },\n")
endforeach()

set(out_c "/* Generated by cmake/IvsPsgTable.cmake from the files in data/profiles (A-06). */\n")
string(APPEND out_c "/* Build output only: never committed, never edited. */\n")
string(APPEND out_c "#define IVS_PSG_PROFILE_COUNT ${n_profiles}u\n")
string(APPEND out_c "_Static_assert(IVS_PSG_REG_COUNT == 16u, \"the generator emits 16 register masks\");\n")
string(APPEND out_c "static const ivs_psg_params ivs_psg_params_table[IVS_PSG_PROFILE_COUNT] = {\n${table_c}};\n")
string(APPEND out_c "_Static_assert(sizeof ivs_psg_params_table / sizeof ivs_psg_params_table[0] == IVS_PSG_PROFILE_COUNT, \"one parameter row per profile\");\n")
file(WRITE "${OUT}" "${out_c}")
