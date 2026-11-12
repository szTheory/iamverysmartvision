# SPDX-License-Identifier: MIT
# Named-fault control executables (D-18, A-09, D-37); included from tests/CMakeLists.txt only when
# IVS_FAULT_CONTROLS is ON (dev preset). Inventory: every data/expected/faults/*.ids file, each
# with a `# suite=<name>` header naming a suite registered by ivs_fault_suite() in
# tests/CMakeLists.txt, then one line `<FAULT_ID> <must-fail> <allowed-prefixes> <must-pass>` per
# fault. A fault id appears in exactly one line of exactly one file (configure error otherwise).
# Owner TUs are repo-relative (D-24, plan 04-03): the core TUs (IVS_CORE_SOURCES of
# core/CMakeLists.txt, prefixed core/) and the runner library TUs (IVS_RUNNER_SOURCES of
# runner/CMakeLists.txt, prefixed runner/); two owner TUs with the same file stem fail configure
# (object and plain-copy names use the stem). A suite links the plain copies of the modules its
# ivs_fault_suite(MODULES ...) names (default core; core runner for suite runner).
# For a site fault <ID>, exactly one owner TU must hold `IVS_FAULT_<ID>`, in a module of the
# fault's suite; ivs_fault_<ID> is that TU compiled with IVS_FAULT_<ID> defined, the other TUs of
# the suite's modules and the harness from plain (non-sanitized) object copies, and the suite's
# test sources, compiled once per suite into the plain object library ivs_fault_suite_obj_<suite>
# (W7: N faults cost one test-source compile).
# A GEN_ id has no site: it is looked up in the ivs_fault_gen() registry, links the plain core and
# recompiles the suite's test sources against the generated file its generator writes with one
# mutation; with CONSUMER core:<src> the mutated file is consumed by that core TU instead, which
# is rebuilt with the fault directory first on its include path (H2 e, plan 03-04).
# Fault executables link ivs_warnings only, never ivs_san, so no sanitizer abort can pass for a
# detection. They are EXCLUDE_FROM_ALL (aggregate target ivs_faults) and never registered with
# ctest; the c-faults gate step runs them and faults-verdict judges their results.
# Every core and harness copy sees core/include (public ivs/*.h) before core/src, as the real
# core objects do (D-06, plan 03-03). A runner TU copy sees only core/include and runner/, as
# ivs_runner_lib does (it links only the public ivs_core), so a runner TU that reaches into
# core/src fails to compile here too.

file(GLOB ivs_fault_files CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/data/expected/faults/*.ids")
list(SORT ivs_fault_files)
if(NOT ivs_fault_files)
  message(FATAL_ERROR "faults: no data/expected/faults/*.ids file")
endif()
# Owner list: repo-relative paths of every module that may hold fault sites.
set(ivs_fault_modules core runner)
get_directory_property(ivs_fault_core_rel DIRECTORY "${PROJECT_SOURCE_DIR}/core"
  DEFINITION IVS_CORE_SOURCES)
get_directory_property(ivs_fault_runner_rel DIRECTORY "${PROJECT_SOURCE_DIR}/runner"
  DEFINITION IVS_RUNNER_SOURCES)
if(NOT ivs_fault_core_rel)
  message(FATAL_ERROR "faults: IVS_CORE_SOURCES is empty")
endif()
if(NOT ivs_fault_runner_rel)
  message(FATAL_ERROR "faults: IVS_RUNNER_SOURCES is empty")
endif()
set(ivs_fault_owner_sources "")
foreach(m IN LISTS ivs_fault_modules)
  set(ivs_fault_module_${m}_sources "")
  foreach(s IN LISTS ivs_fault_${m}_rel)
    list(APPEND ivs_fault_module_${m}_sources "${m}/${s}")
    list(APPEND ivs_fault_owner_sources "${m}/${s}")
  endforeach()
endforeach()
# File stems name the object and plain-copy targets: they must be unique across all modules.
foreach(src IN LISTS ivs_fault_owner_sources)
  get_filename_component(stem "${src}" NAME_WE)
  if(DEFINED ivs_fault_stem_${stem})
    message(FATAL_ERROR "faults: file stem ${stem} is used by ${ivs_fault_stem_${stem}} and ${src}")
  endif()
  set(ivs_fault_stem_${stem} "${src}")
endforeach()
# Generated core includes (H2 b, plan 03-04): every plain and owner copy of a core TU sees the
# core's generated directory and waits for its generator target.
get_directory_property(ivs_fault_core_gen_dir DIRECTORY "${PROJECT_SOURCE_DIR}/core"
  DEFINITION IVS_CORE_GEN_DIR)
get_directory_property(ivs_fault_core_gen_target DIRECTORY "${PROJECT_SOURCE_DIR}/core"
  DEFINITION IVS_CORE_GEN_TARGET)
if(NOT ivs_fault_core_gen_dir OR NOT ivs_fault_core_gen_target)
  message(FATAL_ERROR "faults: IVS_CORE_GEN_DIR or IVS_CORE_GEN_TARGET is not set by core/CMakeLists.txt")
endif()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${ivs_fault_files})

add_custom_target(ivs_faults)

# ivs_fault_tu_setup(<object target> <repo-relative owner TU>): the include path and generator
# dependency of a plain or owner copy, by module: a core TU sees core/include, core/src and the
# core's generated directory; a runner TU sees only core/include and runner/ (and, since plan 05-10, the header directory tools/pcm; its
# converter TU is compiled into the suite's own objects).
function(ivs_fault_tu_setup target src)
  if(src MATCHES "^core/")
    target_include_directories(${target} PRIVATE
      "${PROJECT_SOURCE_DIR}/core/include" "${PROJECT_SOURCE_DIR}/core/src" "${ivs_fault_core_gen_dir}")
    add_dependencies(${target} ${ivs_fault_core_gen_target})
  elseif(src MATCHES "^runner/")
    # plus tools/pcm: the runner includes the reference converter's header (plan 05-10, D-21)
    target_include_directories(${target} PRIVATE
      "${PROJECT_SOURCE_DIR}/core/include" "${PROJECT_SOURCE_DIR}/runner" "${PROJECT_SOURCE_DIR}/tools/pcm")
  else()
    message(FATAL_ERROR "faults: ${src} is not under core/ or runner/")
  endif()
  target_link_libraries(${target} PRIVATE ivs_warnings)
  set_target_properties(${target} PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF)
endfunction()

# Plain object copies of every owner TU and of the harness library.
foreach(src IN LISTS ivs_fault_owner_sources)
  get_filename_component(stem "${src}" NAME_WE)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/${src}")
  add_library(ivs_fault_plain_${stem} OBJECT EXCLUDE_FROM_ALL "${PROJECT_SOURCE_DIR}/${src}")
  ivs_fault_tu_setup(ivs_fault_plain_${stem} "${src}")
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
# The repo-relative TUs of the modules a suite names.
function(ivs_fault_suite_tus suite out)
  set(tus "")
  foreach(m IN LISTS IVS_FAULT_SUITE_${suite}_MODULES)
    if(NOT m IN_LIST ivs_fault_modules)
      message(FATAL_ERROR "faults: suite ${suite} names module ${m} (known: ${ivs_fault_modules})")
    endif()
    list(APPEND tus ${ivs_fault_module_${m}_sources})
  endforeach()
  set(${out} "${tus}" PARENT_SCOPE)
endfunction()

function(ivs_fault_exe id suite owner gen_dir gen_file)
  set(objs "$<TARGET_OBJECTS:ivs_fault_plain_harness>")
  ivs_fault_suite_tus(${suite} tus)
  if(NOT owner STREQUAL "" AND NOT owner IN_LIST tus)
    message(FATAL_ERROR "fault ${id}: owner ${owner} is not in a module of suite ${suite} (${IVS_FAULT_SUITE_${suite}_MODULES})")
  endif()
  foreach(src IN LISTS tus)
    get_filename_component(stem "${src}" NAME_WE)
    if(src STREQUAL owner)
      add_library(ivs_fault_obj_${id} OBJECT EXCLUDE_FROM_ALL "${PROJECT_SOURCE_DIR}/${src}")
      ivs_fault_tu_setup(ivs_fault_obj_${id} "${src}")
      target_compile_definitions(ivs_fault_obj_${id} PRIVATE "IVS_FAULT_${id}")
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

# ivs_fault_exe_core_gen(<GEN_ID> <suite> <consumer core src> <generated dir> <generated file>)
# A GEN_ fault whose generated file a core TU includes (H2 e): that TU is the owner copy, built
# with the fault directory before the real generated directory; the suite's test sources come
# from the shared suite objects, the other TUs of the suite's modules from plain copies.
# <consumer> is repo-relative (core/<core source>).
function(ivs_fault_exe_core_gen id suite consumer gen_dir gen_file)
  set(objs "$<TARGET_OBJECTS:ivs_fault_plain_harness>")
  ivs_fault_suite_tus(${suite} tus)
  foreach(src IN LISTS tus)
    get_filename_component(stem "${src}" NAME_WE)
    if(src STREQUAL consumer)
      add_library(ivs_fault_obj_${id} OBJECT EXCLUDE_FROM_ALL "${PROJECT_SOURCE_DIR}/${src}" "${gen_file}")
      target_include_directories(ivs_fault_obj_${id} PRIVATE "${gen_dir}"
        "${PROJECT_SOURCE_DIR}/core/include" "${PROJECT_SOURCE_DIR}/core/src" "${ivs_fault_core_gen_dir}")
      target_link_libraries(ivs_fault_obj_${id} PRIVATE ivs_warnings)
      set_target_properties(ivs_fault_obj_${id} PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF)
      list(APPEND objs "$<TARGET_OBJECTS:ivs_fault_obj_${id}>")
    else()
      list(APPEND objs "$<TARGET_OBJECTS:ivs_fault_plain_${stem}>")
    endif()
  endforeach()
  ivs_fault_suite_obj(${suite})
  add_executable(ivs_fault_${id} EXCLUDE_FROM_ALL "$<TARGET_OBJECTS:ivs_fault_suite_obj_${suite}>" ${objs})
  set_target_properties(ivs_fault_${id} PROPERTIES LINKER_LANGUAGE C INTERPROCEDURAL_OPTIMIZATION OFF)
  target_link_libraries(ivs_fault_${id} PRIVATE ivs_warnings)
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
      set(mutate_arg "-DMUTATE=${must_fail}")
      foreach(a IN LISTS IVS_FAULT_GEN_${id}_ARGS)
        if(a MATCHES "^-DMUTATE=")
          set(mutate_arg "")
        endif()
      endforeach()
      add_custom_command(
        OUTPUT "${out}"
        COMMAND "${CMAKE_COMMAND}" ${IVS_FAULT_GEN_${id}_ARGS} "-DOUT=${out}" ${mutate_arg}
                -P "${IVS_FAULT_GEN_${id}_GENERATOR}"
        DEPENDS ${IVS_FAULT_GEN_${id}_DEPENDS}
        COMMENT "Generating the mutated ${IVS_FAULT_GEN_${id}_OUTPUT} for fault ${id}"
        VERBATIM)
      set(consumer "${IVS_FAULT_GEN_${id}_CONSUMER}")
      if(consumer STREQUAL "")
        ivs_fault_exe(${id} ${suite} "" "${dir}" "${out}")
      elseif(consumer MATCHES "^core:(.+)$")
        set(csrc "core/${CMAKE_MATCH_1}")
        if(NOT csrc IN_LIST ivs_fault_module_core_sources)
          message(FATAL_ERROR "fault ${id}: CONSUMER ${consumer} is not a core source")
        endif()
        ivs_fault_exe_core_gen(${id} ${suite} "${csrc}" "${dir}" "${out}")
      else()
        message(FATAL_ERROR "fault ${id}: CONSUMER '${consumer}' is not core:<core source>")
      endif()
      continue()
    endif()
    set(owner "")
    foreach(src IN LISTS ivs_fault_owner_sources)
      file(READ "${PROJECT_SOURCE_DIR}/${src}" text)
      if(text MATCHES "IVS_FAULT_${id}[^A-Za-z0-9_]")
        list(APPEND owner "${src}")
      endif()
    endforeach()
    list(LENGTH owner n_owner)
    if(NOT n_owner EQUAL 1)
      message(FATAL_ERROR "fault ${id}: IVS_FAULT_${id} must appear in exactly one owner TU under core/ or runner/ (found ${n_owner}: ${owner})")
    endif()
    ivs_fault_exe(${id} ${suite} "${owner}" "" "")
  endforeach()
endforeach()
