# SPDX-License-Identifier: MIT
# Notes-to-log consistency for discrepancy C17 (FND-06, W1, D-21).
# Input (passed with -D): ROOT, the tree under test (required).
# The status of the C17 entry in data/discrepancies.json must equal the value on the single
# `C17 status: ` line of docs/cp1610/reference-notes.md. This checks agreement between the
# image-derived notes and the log; it never requires a particular status.
# Failure tokens: cp1610-c17-missing: <what>, cp1610-c17-inconsistent: log=<status> notes=<status>.
# Stats: c17=<status>.
cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED ROOT OR "${ROOT}" STREQUAL "")
  message(FATAL_ERROR "c17-consistency.cmake is missing ROOT")
endif()
include("${CMAKE_CURRENT_LIST_DIR}/lib.cmake")

# Log side: the entry with id C17.
set(log_status "")
set(have_log FALSE)
read_record_file(DOC "data/discrepancies.json" cp1610-c17)
if(NOT "${DOC}" STREQUAL "")
  json_type(dt "${DOC}" discrepancies)
  if(dt STREQUAL ARRAY)
    string(JSON n LENGTH "${DOC}" discrepancies)
    if(n GREATER 0)
      math(EXPR last "${n} - 1")
      foreach(i RANGE 0 ${last})
        string(JSON rec GET "${DOC}" discrepancies ${i})
        json_type(it "${rec}" id)
        if(NOT it STREQUAL STRING)
          continue()
        endif()
        string(JSON id GET "${rec}" id)
        if(NOT "${id}" STREQUAL "C17")
          continue()
        endif()
        json_type(st "${rec}" status)
        if(st STREQUAL STRING)
          string(JSON log_status GET "${rec}" status)
          set(have_log TRUE)
        endif()
      endforeach()
    endif()
  endif()
endif()
if(NOT have_log)
  gate_fail(cp1610-c17-missing "data/discrepancies.json entry C17")
endif()

# Notes side: exactly one line `C17 status: <value>`.
set(notes_status "")
set(have_notes FALSE)
set(notes "${ROOT}/docs/cp1610/reference-notes.md")
if(NOT EXISTS "${notes}")
  gate_fail(cp1610-c17-missing "docs/cp1610/reference-notes.md")
else()
  file(STRINGS "${notes}" status_lines REGEX "^C17 status: ")
  list(LENGTH status_lines nlines)
  if(nlines EQUAL 1)
    string(REGEX REPLACE "^C17 status:[ \t]*" "" notes_status "${status_lines}")
    string(STRIP "${notes_status}" notes_status)
    set(have_notes TRUE)
  else()
    gate_fail(cp1610-c17-missing "C17 status line in docs/cp1610/reference-notes.md (found ${nlines})")
  endif()
endif()

if(have_log AND have_notes)
  if("${log_status}" STREQUAL "${notes_status}")
    gate_stat(c17 "${log_status}")
  else()
    gate_fail(cp1610-c17-inconsistent "log=${log_status} notes=${notes_status}")
  endif()
endif()
gate_finish(c17-consistency)
