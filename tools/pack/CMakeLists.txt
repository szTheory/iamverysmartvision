# SPDX-License-Identifier: MIT
# tools/pack (D-25, D-26, D-30, plan 04-14): ivs_pack, the build-tree image tool of the project's
# hand-assembled listings, and the custom target ivs_images, part of the default build of both
# presets, which writes the images into ${CMAKE_BINARY_DIR}/images (exposed as IVS_IMAGES_DIR) and
# nowhere else. ivs_pack is a development tool: no install rule, never shipped. It links the core
# archive only for the SHA-256 of IVSC trailers (ivs_core_internal gives it common/ivs_sha256.h).
# IVS_PACK_SOURCES (relative to tools/pack/) is written repo-relative to the build tree like the
# core and runner lists, so the c-build step finds a tracked tools/pack/*.c that is not compiled.
# The C include of the images for the in-process suites is generated in tests/CMakeLists.txt.
# Plan 04-16 adds diag-cpu-1.ivsc to ivs_images.

set(IVS_PACK_SOURCES
  pack.c
)

add_executable(ivs_pack ${IVS_PACK_SOURCES})
set_target_properties(ivs_pack PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF LINKER_LANGUAGE C)
target_link_libraries(ivs_pack PRIVATE ivs_core ivs_core_internal ivs_warnings ivs_san)

set(IVS_IMAGES_DIR "${CMAKE_BINARY_DIR}/images")
set(IVS_IMAGES_DIR "${IVS_IMAGES_DIR}" PARENT_SCOPE)
set(IVS_BOOT_STUB_LISTING "${PROJECT_SOURCE_DIR}/fixtures/FX-boot-stub/boot-stub.listing.txt")
set(IVS_BOOT_STUB_EXEC "${IVS_IMAGES_DIR}/boot-stub.exec")
set(IVS_BOOT_STUB_EXEC "${IVS_BOOT_STUB_EXEC}" PARENT_SCOPE)

add_custom_command(
  OUTPUT "${IVS_BOOT_STUB_EXEC}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${IVS_IMAGES_DIR}"
  COMMAND ivs_pack --exec "${IVS_BOOT_STUB_LISTING}" "${IVS_BOOT_STUB_EXEC}"
  DEPENDS ivs_pack "${IVS_BOOT_STUB_LISTING}"
  COMMENT "Packing the project boot stub listing into the build tree"
  VERBATIM)
add_custom_target(ivs_images ALL DEPENDS "${IVS_BOOT_STUB_EXEC}")

set(listed "")
foreach(src IN LISTS IVS_PACK_SOURCES)
  string(APPEND listed "tools/pack/${src}\n")
endforeach()
file(WRITE "${CMAKE_BINARY_DIR}/ivs-pack-sources.txt.tmp" "${listed}")
file(COPY_FILE "${CMAKE_BINARY_DIR}/ivs-pack-sources.txt.tmp" "${CMAKE_BINARY_DIR}/ivs-pack-sources.txt"
  ONLY_IF_DIFFERENT)
