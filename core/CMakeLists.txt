# SPDX-License-Identifier: MIT
# The machine core: an explicit source list (no file(GLOB)), one OBJECT library per source,
# and the STATIC library ivs_core built from those objects (D-19). The repo-relative source
# list is written to the build tree so the c-build step can find a tracked core source that
# is not compiled (token c-build-unlisted-source).
# Include split (D-06, plan 03-03): every core object sees core/include (public headers, ivs/*.h)
# and core/src (internals) PRIVATE; ivs_core exports only core/include, so a consumer that links
# it sees only ivs/*.h. Tests reach core/src internals through the interface target
# ivs_core_internal.

set(IVS_CORE_SOURCES
  src/common/ivs_sha256.c
  src/content/ivsc.c
  src/cpu/cp1610.c
  src/cpu/cp1610_decode.c
  src/machine/machine.c
  src/machine/machine_bound.c
  src/machine/machine_bus.c
  src/machine/machine_digest.c
  src/machine/machine_input.c
  src/machine/machine_inspect.c
  src/machine/machine_load.c
  src/machine/machine_run.c
  src/memmap/memsys.c
  src/psg/psg_env.c
  src/psg/psg_mix.c
  src/psg/psg_noise.c
  src/psg/psg_regs.c
  src/psg/psg_log.c
  src/psg/psg_sample.c
  src/psg/psg_state.c
  src/psg/psg_tone.c
  src/stic/stic_events.c
  src/stic/stic_frame.c
  src/stic/stic_lines.c
  src/stic/stic_params.c
  src/stic/stic_regs.c
  src/stic/stic_state.c
)

# Generated core include (D-16, H2 a, plan 03-04): the memory map, device records and profile
# registry, written from every data/profiles/*.json by cmake/IvsMemmapTable.cmake into
# gen/ivs_profiles.inc in this build directory. memsys.c is its only includer and is listed in
# data/expected/core-generated.ids with its generator, so c-purity regenerates and checks it.
# tests/faults.cmake reads IVS_CORE_GEN_DIR and IVS_CORE_GEN_TARGET to compile the plain and
# owner copies of every core TU against the same directory.
set(IVS_CORE_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen")
set(IVS_PROFILES_INC "${IVS_CORE_GEN_DIR}/ivs_profiles.inc")
set(IVS_MEMMAP_GENERATOR "${PROJECT_SOURCE_DIR}/cmake/IvsMemmapTable.cmake")
file(GLOB IVS_PROFILE_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/data/profiles/*.json")
list(SORT IVS_PROFILE_FILES)
add_custom_command(
  OUTPUT "${IVS_PROFILES_INC}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${IVS_CORE_GEN_DIR}"
  COMMAND "${CMAKE_COMMAND}" "-DROOT=${PROJECT_SOURCE_DIR}" "-DOUT=${IVS_PROFILES_INC}"
          -P "${IVS_MEMMAP_GENERATOR}"
  DEPENDS ${IVS_PROFILE_FILES} "${IVS_MEMMAP_GENERATOR}"
  COMMENT "Generating the memory map and profile registry"
  VERBATIM)
# Second generated include (A-06, plan 05-04): the PSG parameters (register masks from psg.regmap)
# written by cmake/IvsPsgTable.cmake into gen/ivs_psg_params.inc. psg_regs.c is its only includer
# and data/expected/core-generated.ids lists it with its generator. ivs_core_gen waits for both.
set(IVS_PSG_PARAMS_INC "${IVS_CORE_GEN_DIR}/ivs_psg_params.inc")
set(IVS_PSG_GENERATOR "${PROJECT_SOURCE_DIR}/cmake/IvsPsgTable.cmake")
add_custom_command(
  OUTPUT "${IVS_PSG_PARAMS_INC}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${IVS_CORE_GEN_DIR}"
  COMMAND "${CMAKE_COMMAND}" "-DROOT=${PROJECT_SOURCE_DIR}" "-DOUT=${IVS_PSG_PARAMS_INC}"
          -P "${IVS_PSG_GENERATOR}"
  DEPENDS ${IVS_PROFILE_FILES} "${IVS_PSG_GENERATOR}"
  COMMENT "Generating the PSG parameter table"
  VERBATIM)
# Third generated include (plan 05-05, D-02): the control-to-pin table ivs_ctrl_lines[profile][31]
# from the profile fields input.control.* by cmake/IvsCtrlTable.cmake (it also reads
# data/expected/input-bits.ids, the pinned name set). machine_input.c is its only includer; the
# named fault GEN_CTRL_MUTATION rebuilds that TU against a one-bit mutation of it.
set(IVS_CTRL_LINES_INC "${IVS_CORE_GEN_DIR}/ivs_ctrl_lines.inc")
set(IVS_CTRL_GENERATOR "${PROJECT_SOURCE_DIR}/cmake/IvsCtrlTable.cmake")
set(IVS_INPUT_BITS_IDS "${PROJECT_SOURCE_DIR}/data/expected/input-bits.ids")
add_custom_command(
  OUTPUT "${IVS_CTRL_LINES_INC}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${IVS_CORE_GEN_DIR}"
  COMMAND "${CMAKE_COMMAND}" "-DROOT=${PROJECT_SOURCE_DIR}" "-DOUT=${IVS_CTRL_LINES_INC}"
          -P "${IVS_CTRL_GENERATOR}"
  DEPENDS ${IVS_PROFILE_FILES} "${IVS_CTRL_GENERATOR}" "${IVS_INPUT_BITS_IDS}"
  COMMENT "Generating the controller line table"
  VERBATIM)
# Fourth generated include (plan 06-04, A-11): the STIC parameter table (windows, bus windows, the per-frame
# event table, the register masks, the derived video geometry) from the profile fields stic.*, video.* and
# bus.line_sample_lag_cpu_cycles by cmake/IvsSticTable.cmake, fail closed. stic_params.c is its only includer
# and data/expected/core-generated.ids lists it with its generator.
set(IVS_STIC_PARAMS_INC "${IVS_CORE_GEN_DIR}/ivs_stic_params.inc")
set(IVS_STIC_GENERATOR "${PROJECT_SOURCE_DIR}/cmake/IvsSticTable.cmake")
add_custom_command(
  OUTPUT "${IVS_STIC_PARAMS_INC}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${IVS_CORE_GEN_DIR}"
  COMMAND "${CMAKE_COMMAND}" "-DROOT=${PROJECT_SOURCE_DIR}" "-DOUT=${IVS_STIC_PARAMS_INC}"
          -P "${IVS_STIC_GENERATOR}"
  DEPENDS ${IVS_PROFILE_FILES} "${IVS_STIC_GENERATOR}"
  COMMENT "Generating the STIC parameter table"
  VERBATIM)
add_custom_target(ivs_core_gen DEPENDS "${IVS_PROFILES_INC}" "${IVS_PSG_PARAMS_INC}" "${IVS_CTRL_LINES_INC}"
  "${IVS_STIC_PARAMS_INC}")
set(IVS_CORE_GEN_TARGET ivs_core_gen)

set(ivs_core_objects "")
foreach(src IN LISTS IVS_CORE_SOURCES)
  get_filename_component(stem "${src}" NAME_WE)
  set(obj "ivs_core_obj_${stem}")
  add_library(${obj} OBJECT "${src}")
  target_include_directories(${obj} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/src")
  target_include_directories(${obj} PRIVATE "${IVS_CORE_GEN_DIR}")
  add_dependencies(${obj} ivs_core_gen)
  target_link_libraries(${obj} PRIVATE ivs_warnings ivs_san)
  # PIC (D-34, plan 04-22): a host may link the archive into a shared plugin.
  set_target_properties(${obj} PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF POSITION_INDEPENDENT_CODE ON)
  list(APPEND ivs_core_objects "$<TARGET_OBJECTS:${obj}>")
endforeach()

# Static only (D-35): STATIC is explicit, so BUILD_SHARED_LIBS is ignored and IVS_API stays empty.
add_library(ivs_core STATIC ${ivs_core_objects})
set_target_properties(ivs_core PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF LINKER_LANGUAGE C
  EXPORT_NAME core)
# The in-tree name a FetchContent or add_subdirectory user writes, the same as the installed one.
add_library(ivs::core ALIAS ivs_core)
# No absolute path may reach the export (D-33): the build tree sees core/include, the installed
# package its own include/ directory.
target_include_directories(ivs_core PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

add_library(ivs_core_internal INTERFACE)
target_include_directories(ivs_core_internal INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(listed "")
foreach(src IN LISTS IVS_CORE_SOURCES)
  string(APPEND listed "core/${src}\n")
endforeach()
file(WRITE "${CMAKE_BINARY_DIR}/ivs-core-sources.txt.tmp" "${listed}")
file(COPY_FILE "${CMAKE_BINARY_DIR}/ivs-core-sources.txt.tmp" "${CMAKE_BINARY_DIR}/ivs-core-sources.txt"
  ONLY_IF_DIFFERENT)

# Install and export (D-33, D-34, D-35, plan 04-22). The package is the static archive, the
# public headers ivs/*.h, the CMake package files and the licence; nothing else (no runner, no
# internals, no images; the gate step pkg-install compares the prefix with
# data/expected/pkg-install-files.ids). The package config is hand-written (no paths to
# resolve), the version file comes from project(VERSION), parsed from ivs.h in the root.
# No package-registry export and no pkg-config file: a consumer must never find this build tree.
# A sanitized tree refuses to install: its archive needs -fsanitize at the consumer's link.
if(IVS_SAN)
  install(CODE "message(FATAL_ERROR \"ivs: refusing to install a sanitized (IVS_SAN) tree; install from the release preset\")")
endif()
install(TARGETS ivs_core EXPORT ivsTargets ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(DIRECTORY include/ivs DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" FILES_MATCHING PATTERN "*.h")
install(EXPORT ivsTargets NAMESPACE ivs:: DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/ivs")
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/ivsConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}" COMPATIBILITY SameMinorVersion ARCH_INDEPENDENT)
install(FILES "${PROJECT_SOURCE_DIR}/cmake/ivsConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/ivsConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/ivs")
install(FILES "${PROJECT_SOURCE_DIR}/LICENSE" DESTINATION "${CMAKE_INSTALL_DOCDIR}")
