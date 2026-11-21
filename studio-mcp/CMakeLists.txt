# TuxBlox - Linux Compatibility Layer for the Roblox Engine
# Copyright (C) 2026 TuxBlox Developers
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.16)
project(TuxBloxStudioMcp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TUXBLOX_LAUNCHER_DIR "${CMAKE_SOURCE_DIR}/../launcher" CACHE PATH
    "Path to the launcher source tree this shares its path-resolution code with")
if(NOT EXISTS "${TUXBLOX_LAUNCHER_DIR}/src/launch_paths.cpp")
    message(FATAL_ERROR
        "TUXBLOX_LAUNCHER_DIR (${TUXBLOX_LAUNCHER_DIR}) is not a launcher source tree.")
endif()
if(NOT EXISTS "${TUXBLOX_LAUNCHER_DIR}/third_party/json.hpp")
    message(FATAL_ERROR
        "${TUXBLOX_LAUNCHER_DIR}/third_party/json.hpp is missing. "
        "It should be committed in launcher/third_party/.")
endif()

set(TUXBLOX_SHARED_RELEASE "${CMAKE_SOURCE_DIR}/../shared/release/src" CACHE PATH
    "Path to the shared release sources")
if(NOT EXISTS "${TUXBLOX_SHARED_RELEASE}/release_stamp.cpp")
    message(FATAL_ERROR
        "TUXBLOX_SHARED_RELEASE (${TUXBLOX_SHARED_RELEASE}) does not hold release_stamp.cpp.")
endif()

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/generated)
if(NOT "$ENV{TUXBLOX_BUILD_VERSION}" STREQUAL "")
    set(TUXBLOX_MCP_VERSION "$ENV{TUXBLOX_BUILD_VERSION}")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/VERSION")
    file(STRINGS "${CMAKE_SOURCE_DIR}/VERSION" TUXBLOX_MCP_VERSION LIMIT_COUNT 1)
else()
    set(TUXBLOX_MCP_VERSION "0.0.0-dev")
endif()
message(STATUS "TuxBlox Studio MCP version: ${TUXBLOX_MCP_VERSION}")
configure_file(${CMAKE_SOURCE_DIR}/cmake/version.h.in
               ${CMAKE_BINARY_DIR}/generated/version.h @ONLY)

if(NOT "$ENV{TUXBLOX_RELEASE}" STREQUAL "")
    set(TUXBLOX_STAMP_DEFINES "TUXBLOX_RELEASE=\"$ENV{TUXBLOX_RELEASE}\"")
    if(NOT "$ENV{TUXBLOX_CHANNEL}" STREQUAL "")
        list(APPEND TUXBLOX_STAMP_DEFINES "TUXBLOX_CHANNEL=\"$ENV{TUXBLOX_CHANNEL}\"")
    endif()
    set_source_files_properties(${TUXBLOX_SHARED_RELEASE}/release_stamp.cpp PROPERTIES
        COMPILE_DEFINITIONS "${TUXBLOX_STAMP_DEFINES}")
endif()

add_executable(studio-mcp
    src/main.cpp
    src/mcp_locate.cpp
    ${TUXBLOX_LAUNCHER_DIR}/src/install_paths.cpp
    ${TUXBLOX_LAUNCHER_DIR}/src/launch_paths.cpp
    ${TUXBLOX_LAUNCHER_DIR}/src/lnk_resolver.cpp
    ${TUXBLOX_LAUNCHER_DIR}/src/pe_version.cpp
    ${TUXBLOX_LAUNCHER_DIR}/src/prefix_user.cpp
    ${TUXBLOX_LAUNCHER_DIR}/src/versions_manifest.cpp
    ${TUXBLOX_SHARED_RELEASE}/release_stamp.cpp
)

target_link_options(studio-mcp PRIVATE -static-libstdc++ -static-libgcc)
target_include_directories(studio-mcp PRIVATE
    src
    ${TUXBLOX_LAUNCHER_DIR}/src
    ${TUXBLOX_LAUNCHER_DIR}/third_party
    ${TUXBLOX_SHARED_RELEASE}
    ${CMAKE_BINARY_DIR}/generated
)

enable_testing()
add_subdirectory(tests)
