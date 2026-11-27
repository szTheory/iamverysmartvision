# SPDX-License-Identifier: MIT
# tools/inspect (Phase 18, plan 18-02, TOOL-02, TOOL-09): the target ivs_inspect and the executable
# ivs-inspect (the underscore target / hyphen executable convention ivs_pack, ivs_asm, ivs_bench and
# ivs_flatwrap follow; the source list below is the hyphenated ivs-inspect-sources.txt).
# It is a development instrument: no install rule anywhere in this file, never shipped, built in both
# presets like the other tools, so the installed prefix keeps its eleven paths. It links
# ivs_runner_lib for the scenario parser and the report (a second report writer would be a second
# truth) and ivs_core_internal for the per-unit access-trace seam and the core's own input floor
# (core/src/machine/machine.h), the two things the public API does not expose; ivs_core for the rest.
# IVS_INSPECT_TREE_ROOT is the configure-time work-tree root the output rule compares against,
# resolved physically at run time. IVS_INSPECT_SOURCES (relative to tools/inspect/) is written
# repo-relative to the build tree (ivs-inspect-sources.txt) like the lists of the other tools, so the
# c-build step finds a tracked tools/inspect/*.c that is not compiled.
#
# Two file stems and no third file: object and plain-copy names use the stem, so a stem that collides
# with core/src or runner/ is a configure-time fault (runner/CMakeLists.txt states the rule), and the
# tool's own two-file split is pinned by plan 18-02, so the one cross-file prototype is declared at
# both ends (tools/inspect/inspect.c and tools/inspect/inspect_trace.c) rather than in a header.

set(IVS_INSPECT_SOURCES
  inspect.c
  inspect_trace.c
)

add_executable(ivs_inspect ${IVS_INSPECT_SOURCES})
set_target_properties(ivs_inspect PROPERTIES OUTPUT_NAME ivs-inspect INTERPROCEDURAL_OPTIMIZATION OFF
  LINKER_LANGUAGE C)
target_link_libraries(ivs_inspect PRIVATE ivs_runner_lib ivs_core ivs_core_internal ivs_warnings ivs_san)
target_compile_definitions(ivs_inspect PRIVATE "IVS_INSPECT_TREE_ROOT=\"${PROJECT_SOURCE_DIR}\"")

set(listed "")
foreach(src IN LISTS IVS_INSPECT_SOURCES)
  string(APPEND listed "tools/inspect/${src}\n")
endforeach()
file(WRITE "${CMAKE_BINARY_DIR}/ivs-inspect-sources.txt.tmp" "${listed}")
file(COPY_FILE "${CMAKE_BINARY_DIR}/ivs-inspect-sources.txt.tmp" "${CMAKE_BINARY_DIR}/ivs-inspect-sources.txt"
  ONLY_IF_DIFFERENT)
