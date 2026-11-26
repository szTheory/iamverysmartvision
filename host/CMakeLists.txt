# SPDX-License-Identifier: MIT
# The build-only host shim (plan 15-01, TOOL-08, D-48, D-35): libivs_retro, a SHARED object over the
# unchanged static core. It is a NON-MEMBER by construction: this file carries no install rule and none
# may be added anywhere, so the package stays eleven files and the release ships no shared object.
# Three things this file is the only place for:
#   - IVS_API, defined for this target alone as default visibility: every project target carries
#     -fvisibility=hidden (cmake/IvsWarnings.cmake), and the installed headers keep IVS_API empty, so
#     D-35's static-only package stands;
#   - the linker's export list, GENERATED from data/expected/host-symbols.ids, so the artifact and the
#     pin are one contract: Apple -exported_symbols_list, GNU --version-script with `local: *`;
#   - the palette, reached by compiling runner/frame_export.c IN rather than copying it (one palette in
#     the tree, never in core/), and the one PCM converter, linked as ivs_refpcm (never a second one).
# ivs_san is not optional: the dev preset builds the core archive instrumented, and an instrumented
# object loaded into a plain process fails with "ASan runtime does not come first" (research Pitfall 3).
# Scoping's "no sanitizer lane for the shim" means no EXTRA lane, not no sanitizer.

set(IVS_HOST_SYMBOLS_FILE "${PROJECT_SOURCE_DIR}/data/expected/host-symbols.ids")
# The reader cannot see a comment: every comment line begins with '#' and so cannot match this regex.
file(STRINGS "${IVS_HOST_SYMBOLS_FILE}" IVS_HOST_SYMBOLS REGEX "^[A-Za-z_][A-Za-z0-9_]*$")

add_library(ivs_retro_shim SHARED ivs_retro_shim.c "${PROJECT_SOURCE_DIR}/runner/frame_export.c")
target_include_directories(ivs_retro_shim PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/runner" "${PROJECT_SOURCE_DIR}/core/include")
# The only place in the tree where IVS_API is defined (D-35). The escaped quotes are required: the flag
# must reach the compiler with its parentheses intact.
target_compile_definitions(ivs_retro_shim PRIVATE "IVS_API=__attribute__((visibility(\"default\")))")
target_link_libraries(ivs_retro_shim PRIVATE ivs_core ivs_refpcm ivs_warnings ivs_san)
set_target_properties(ivs_retro_shim PROPERTIES OUTPUT_NAME ivs_retro PREFIX "lib")

# The export list, one contract file read by the linker here and by tools/check/host-symbols.sh there.
# Apple's ld adds the leading underscore itself, so the generated list carries it and the check's norm()
# recipe strips it again; GNU's version script names the symbol as written.
if(APPLE)
  set(ivs_host_exports "")
  foreach(sym IN LISTS IVS_HOST_SYMBOLS)
    string(APPEND ivs_host_exports "_${sym}\n")
  endforeach()
  file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/host-exports.txt" CONTENT "${ivs_host_exports}")
  target_link_options(ivs_retro_shim PRIVATE
    "LINKER:-exported_symbols_list,${CMAKE_CURRENT_BINARY_DIR}/host-exports.txt")
else()
  set(ivs_host_exports "{ global:\n")
  foreach(sym IN LISTS IVS_HOST_SYMBOLS)
    string(APPEND ivs_host_exports "  ${sym};\n")
  endforeach()
  string(APPEND ivs_host_exports "  local: *;\n};\n")
  file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/host-exports.map" CONTENT "${ivs_host_exports}")
  target_link_options(ivs_retro_shim PRIVATE
    "LINKER:--version-script,${CMAKE_CURRENT_BINARY_DIR}/host-exports.map")
endif()

# NO INSTALL RULE HERE, EVER: the object is build-only (D-35). Plan 15-02 proves the absence from the
# installed package's inventory and from tools/check/binary-ext.txt.
