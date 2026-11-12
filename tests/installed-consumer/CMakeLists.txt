# SPDX-License-Identifier: MIT
# The independent consumer of the installed ivs package (PKG-01, D-36, D-39, plan 04-23).
# Not reachable from the root CMake (no add_subdirectory): the gate step pkg-install copies this
# directory outside the repository and configures it against the installed, relocated prefix with
#   cmake -G Ninja -S <copy> -B <build> -DCMAKE_PREFIX_PATH=<prefix>
# so the only way to the headers and the archive is find_package. It is also the integrator
# example of docs/packaging.md: find_package, one imported target, nothing else.
cmake_minimum_required(VERSION 3.25)
project(ivs_consumer LANGUAGES C)

# No user or system package registry: the prefix given on the command line is the only source.
set(CMAKE_FIND_USE_PACKAGE_REGISTRY OFF)
set(CMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY OFF)
find_package(ivs 0.1 CONFIG REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
# Imported include dirs are SYSTEM by default, which would silence warnings in the package
# headers; the strict flags below must apply to them too.
set(CMAKE_NO_SYSTEM_FROM_IMPORTED ON)

add_executable(consumer main.c)
target_compile_options(consumer PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
# The version tie (D-34): the package version the config file reported, as three integers.
target_compile_definitions(consumer PRIVATE
  IVS_PKG_VERSION_MAJOR=${ivs_VERSION_MAJOR}
  IVS_PKG_VERSION_MINOR=${ivs_VERSION_MINOR}
  IVS_PKG_VERSION_PATCH=${ivs_VERSION_PATCH})
target_link_libraries(consumer PRIVATE ivs::core)
