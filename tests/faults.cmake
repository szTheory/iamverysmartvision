# SPDX-License-Identifier: MIT
# Named-fault control executables (D-18, A-09, D-37); included from tests/CMakeLists.txt only when
# IVS_FAULT_CONTROLS is ON (dev preset). Inventory: every data/expected/faults/*.ids file, each
# with a `# suite=<name>` header naming a suite registered by ivs_fault_suite() in
# tests/CMakeLists.txt, then one line `<FAULT_ID> <must-fail> <allowed-prefixes> <must-pass>` per
# fault. A fault id appears in exactly one line of exactly one file (configure error otherwise).
# For a site fault <ID>, exactly one core TU must hold `IVS_FAULT_<ID>`; ivs_fault_<ID> is that
# TU compiled with IVS_FAULT_<ID> defined, the other core TUs and the harness from plain
# (non-sanitized) object copies, and the suite's test sources, compiled once per suite into the
# plain object library ivs_fault_suite_obj_<suite> (W7: N faults cost one test-source compile).
# A GEN_ id has no site: it is looked up in the ivs_fault_gen() registry, links the plain core and
# recompiles the suite's test sources against the generated file its generator writes with one
# mutation.
# Fault executables link ivs_warnings only, never ivs_san, so no sanitizer abort can pass for a
# detection. They are EXCLUDE_FROM_ALL (aggregate target ivs_faults) and never registered with
# ctest; the c-faults gate step runs them and faults-verdict judges their results.
# Every core and harness copy sees core/include (public ivs/*.h) before core/src, as the real
# core objects do (D-06, plan 03-03).

file(GLOB ivs_fault_files CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/data/expected/faults/*.ids")
list(SORT ivs_fault_files)
if(NOT ivs_fault_files)
  message(FATAL_ERROR "faults: no data/expected/faults/*.ids file")
endif()
get_directory_property(ivs_fault_core_sources DIRECTORY "${PROJECT_SOURCE_DIR}/core"
  DEFINITION IVS_CORE_SOURCES)
if(NOT ivs_fault_core_sources)
  message(FATAL_ERROR "faults: IVS_CORE_SOURCES is empty")
endif()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${ivs_fault_files})

add_custom_target(ivs_faults)

# Plain object copies of every core TU and of the harness library.
foreach(src IN LISTS ivs_fault_core_sources)
  get_filename_component(stem "${src}" NAME_WE)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/core/${src}")
  add_library(ivs_fault_plain_${stem} OBJECT EXCLUDE_FROM_ALL "${PROJECT_SOURCE_DIR}/core/${src}")
  target_include_directories(ivs_fault_plain_${stem} PRIVATE
    "${PROJECT_SOURCE_DIR}/core/include" "${PROJECT_SOURCE_DIR}/core/src")
  target_link_libraries(ivs_fault_plain_${stem} PRIVATE ivs_warnings)
  set_target_properties(ivs_fault_plain_${stem} PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF)
endforeach()
add_library(ivs_fault_plain_harness OBJECT EXCLUDE_FROM_ALL harness/ivs_test.c harness/ivs_testbus.c)
target_include_directories(ivs_fault_plain_harness PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/harness" "${PROJECT_SOURCE_DIR}/core/include" "${PROJECT_SOURCE_DIR}/core/src")
target_link_libraries(ivs_fault_plain_harness PRIVATE ivs_warnings)

# One plain object library per suite that a faults file names (created on first use).
function(ivs_fault_suite_obj suite)
  if(TARGET ivs_fault_suite_obj_${suite})
    return()
  endif()
  add_library(ivs_fault_suite_obj_${suite} OBJECT EXCLUDE_FROM_ALL
    ${IVS_FAULT_SUITE_${suite}_SOURCES} ${IVS_FAULT_SUITE_${suite}_GENERATED})
  target_include_directories(ivs_fault_suite_obj_${suite} PRIVATE
    ${IVS_FAULT_SUITE_${suite}_INCLUDE_DIRS}
    "${CMAKE_CURRENT_SOURCE_DIR}/harness" "${PROJECT_SOURCE_DIR}/core/include" "${PROJECT_SOURCE_DIR}/core/src")
  target_link_libraries(ivs_fault_suite_obj_${suite} PRIVATE ivs_warnings)
  set_target_properties(ivs_fault_suite_obj_${suite} PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF)
endfunction()

# ivs_fault_exe(<ID> <suite> <owner-src or empty> <generated dir or empty> <generated file or empty>)
# A site fault (owner set) links the suite's shared objects; a GEN_ fault compiles the suite's
# test sources itself with <generated dir> first on the include path.
function(ivs_fault_exe id suite owner gen_dir gen_file)
  set(objs "$<TARGET_OBJECTS:ivs_fault_plain_harness>")
  foreach(src IN LISTS ivs_fault_core_sources)
    get_filename_component(stem "${src}" NAME_WE)
    if(src STREQUAL owner)
      add_library(ivs_fault_obj_${id} OBJECT EXCLUDE_FROM_ALL "${PROJECT_SOURCE_DIR}/core/${src}")
      target_include_directories(ivs_fault_obj_${id} PRIVATE
        "${PROJECT_SOURCE_DIR}/core/include" "${PROJECT_SOURCE_DIR}/core/src")
      target_compile_definitions(ivs_fault_obj_${id} PRIVATE "IVS_FAULT_${id}")
      target_link_libraries(ivs_fault_obj_${id} PRIVATE ivs_warnings)
      set_target_properties(ivs_fault_obj_${id} PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF)
      list(APPEND objs "$<TARGET_OBJECTS:ivs_fault_obj_${id}>")
    else()
      list(APPEND objs "$<TARGET_OBJECTS:ivs_fault_plain_${stem}>")
    endif()
  endforeach()
  if(gen_dir STREQUAL "")
    ivs_fault_suite_obj(${suite})
    add_executable(ivs_fault_${id} EXCLUDE_FROM_ALL
      "$<TARGET_OBJECTS:ivs_fault_suite_obj_${suite}>" ${objs})
    set_target_properties(ivs_fault_${id} PROPERTIES LINKER_LANGUAGE C)
  else()
    get_filename_component(gen_name "${gen_file}" NAME)
    set(gen_srcs "${gen_file}")
    foreach(g IN LISTS IVS_FAULT_SUITE_${suite}_GENERATED)
      get_filename_component(g_name "${g}" NAME)
      if(NOT g_name STREQUAL gen_name)
        list(APPEND gen_srcs "${g}")
      endif()
    endforeach()
    add_executable(ivs_fault_${id} EXCLUDE_FROM_ALL
      ${IVS_FAULT_SUITE_${suite}_SOURCES} ${gen_srcs} ${objs})
    target_include_directories(ivs_fault_${id} PRIVATE "${gen_dir}"
      ${IVS_FAULT_SUITE_${suite}_INCLUDE_DIRS}
      "${CMAKE_CURRENT_SOURCE_DIR}/harness" "${PROJECT_SOURCE_DIR}/core/include" "${PROJECT_SOURCE_DIR}/core/src")
  endif()
  target_link_libraries(ivs_fault_${id} PRIVATE ivs_warnings)
  set_target_properties(ivs_fault_${id} PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF)
  add_dependencies(ivs_faults ivs_fault_${id})
endfunction()

set(ivs_fault_seen "")
foreach(ids_file IN LISTS ivs_fault_files)
  file(RELATIVE_PATH ids_rel "${PROJECT_SOURCE_DIR}" "${ids_file}")
  file(STRINGS "${ids_file}" ivs_fault_lines)
  set(suite "")
  foreach(line IN LISTS ivs_fault_lines)
    if(line MATCHES "^#[ \t]*suite=([a-z0-9][a-z0-9-]*)")
      set(suite "${CMAKE_MATCH_1}")
      break()
    endif()
  endforeach()
  if(suite STREQUAL "")
    message(FATAL_ERROR "faults: ${ids_rel}: no suite header")
  endif()
  if(NOT suite IN_LIST IVS_FAULT_SUITES)
    message(FATAL_ERROR "faults: suite ${suite} is not registered (${ids_rel}; see ivs_fault_suite in tests/CMakeLists.txt)")
  endif()
  foreach(line IN LISTS ivs_fault_lines)
    string(STRIP "${line}" line)
    if(line STREQUAL "" OR line MATCHES "^#")
      continue()
    endif()
    if(NOT line MATCHES "^([A-Z0-9_]+)[ \t]+([^ \t]+)[ \t]+([^ \t]+)[ \t]+([^ \t]+)$")
      message(FATAL_ERROR "faults: bad line in ${ids_rel}: '${line}'")
    endif()
    set(id "${CMAKE_MATCH_1}")
    set(must_fail "${CMAKE_MATCH_2}")
    if(id IN_LIST ivs_fault_seen)
      message(FATAL_ERROR "fault ${id}: listed in ${ivs_fault_file_${id}} and ${ids_rel}")
    endif()
    list(APPEND ivs_fault_seen "${id}")
    set(ivs_fault_file_${id} "${ids_rel}")
    if(id MATCHES "^GEN_")
      if(NOT id IN_LIST IVS_FAULT_GENS)
        message(FATAL_ERROR "fault ${id}: no generator control of that name")
      endif()
      if(NOT IVS_FAULT_GEN_${id}_SUITE STREQUAL suite)
        message(FATAL_ERROR "fault ${id}: registered for suite ${IVS_FAULT_GEN_${id}_SUITE}, listed under suite ${suite} in ${ids_rel}")
      endif()
      set(dir "${CMAKE_CURRENT_BINARY_DIR}/fault-${id}")
      set(out "${dir}/${IVS_FAULT_GEN_${id}_OUTPUT}")
      add_custom_command(
        OUTPUT "${out}"
        COMMAND "${CMAKE_COMMAND}" ${IVS_FAULT_GEN_${id}_ARGS} "-DOUT=${out}" "-DMUTATE=${must_fail}"
                -P "${IVS_FAULT_GEN_${id}_GENERATOR}"
        DEPENDS ${IVS_FAULT_GEN_${id}_DEPENDS}
        COMMENT "Generating the mutated ${IVS_FAULT_GEN_${id}_OUTPUT} for fault ${id}"
        VERBATIM)
      ivs_fault_exe(${id} ${suite} "" "${dir}" "${out}")
      continue()
    endif()
    set(owner "")
    foreach(src IN LISTS ivs_fault_core_sources)
      file(READ "${PROJECT_SOURCE_DIR}/core/${src}" text)
      if(text MATCHES "IVS_FAULT_${id}[^A-Za-z0-9_]")
        list(APPEND owner "${src}")
      endif()
    endforeach()
    list(LENGTH owner n_owner)
    if(NOT n_owner EQUAL 1)
      message(FATAL_ERROR "fault ${id}: IVS_FAULT_${id} must appear in exactly one core TU (found ${n_owner}: ${owner})")
    endif()
    ivs_fault_exe(${id} ${suite} "${owner}" "" "")
  endforeach()
endforeach()
