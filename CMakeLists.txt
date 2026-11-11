# SPDX-License-Identifier: MIT
# Root build of the deterministic machine core and its tests (D-19).
# Developer loop: cmake --preset dev && cmake --build --preset dev && ctest --preset dev
cmake_minimum_required(VERSION 3.25)
project(ivs LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(IVS_WERROR "Treat warnings as errors (ON in the presets, OFF for downstream builds)" OFF)
option(IVS_SAN "Build the core and tests with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(IVS_FAULT_CONTROLS "Build the named-fault control executables (dev tree only)" OFF)

include(cmake/IvsWarnings.cmake)

add_subdirectory(core)
add_subdirectory(runner)

enable_testing()
add_subdirectory(tests)
