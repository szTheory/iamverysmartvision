# SPDX-License-Identifier: MIT
# Root build of the deterministic machine core and its tests (D-19).
# Developer loop: cmake --preset dev && cmake --build --preset dev && ctest --preset dev
cmake_minimum_required(VERSION 3.25)
# One version source of truth (D-34, plan 04-22): the IVS_API_VERSION_{MAJOR,MINOR,PATCH} macros
# of core/include/ivs/ivs.h, read before project() so the package version file, the header and
# ivs_api_version() cannot disagree. A missing or unparsable macro fails the configure.
set(ivs_version_parts "")
foreach(part MAJOR MINOR PATCH)
  file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/core/include/ivs/ivs.h" ivs_version_line
    REGEX "^#define IVS_API_VERSION_${part} [0-9]+[uU]?$")
  list(LENGTH ivs_version_line ivs_version_n)
  if(NOT ivs_version_n EQUAL 1)
    message(FATAL_ERROR "core/include/ivs/ivs.h: expected one '#define IVS_API_VERSION_${part} <n>' line")
  endif()
  string(REGEX REPLACE "^#define IVS_API_VERSION_${part} ([0-9]+)[uU]?$" "\\1" ivs_version_value "${ivs_version_line}")
  list(APPEND ivs_version_parts "${ivs_version_value}")
endforeach()
list(JOIN ivs_version_parts "." ivs_version_parsed)
project(ivs VERSION ${ivs_version_parsed} LANGUAGES C)
include(GNUInstallDirs)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(IVS_WERROR "Treat warnings as errors (ON in the presets, OFF for downstream builds)" OFF)
option(IVS_SAN "Build the core and tests with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(IVS_FAULT_CONTROLS "Build the named-fault control executables (dev tree only)" OFF)

include(cmake/IvsWarnings.cmake)

add_subdirectory(core)
add_subdirectory(runner)
# Build-tree image tool and images (plan 04-14, D-30): sets IVS_IMAGES_DIR before tests.
add_subdirectory(tools/pack)

enable_testing()
add_subdirectory(tests)
