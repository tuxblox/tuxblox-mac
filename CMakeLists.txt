# TuxBlox Mac - Linux launcher for Roblox's macOS Player
# Copyright (C) 2026 TuxBlox Developers
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

cmake_minimum_required(VERSION 3.16)
project(TuxBloxMac VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(ADWAITA REQUIRED IMPORTED_TARGET libadwaita-1)
pkg_check_modules(LIBCURL REQUIRED IMPORTED_TARGET libcurl)
pkg_check_modules(LIBARCHIVE REQUIRED IMPORTED_TARGET libarchive)
pkg_check_modules(OPENSSL REQUIRED IMPORTED_TARGET openssl)
find_package(Threads REQUIRED)

set(APPLE_ROOT_CA_SRC "${CMAKE_CURRENT_SOURCE_DIR}/resources/apple-root-ca.cer")
set(APPLE_ROOT_CA_HEADER "${CMAKE_BINARY_DIR}/generated/apple_root_ca.h")
add_custom_command(
    OUTPUT ${APPLE_ROOT_CA_HEADER}
    COMMAND ${CMAKE_COMMAND} -DINPUT=${APPLE_ROOT_CA_SRC} -DOUTPUT=${APPLE_ROOT_CA_HEADER}
            -DSYMBOL=kAppleRootCaDer -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/BinToHeader.cmake
    DEPENDS ${APPLE_ROOT_CA_SRC} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/BinToHeader.cmake
    COMMENT "Embedding the pinned Apple root certificate"
    VERBATIM)
add_custom_target(generate_apple_root_ca_header DEPENDS ${APPLE_ROOT_CA_HEADER})

add_executable(tuxblox-mac
    src/main.cpp
    src/backend/checksum.cpp
    src/backend/downloader.cpp
    src/backend/mac_client.cpp
    src/backend/mac_signature.cpp
    src/backend/roblox_deploy.cpp
    src/backend/tar_extract.cpp
)
add_dependencies(tuxblox-mac generate_apple_root_ca_header)
target_include_directories(tuxblox-mac PRIVATE
    src/backend
    ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(tuxblox-mac PRIVATE
    PkgConfig::GTK4
    PkgConfig::ADWAITA
    PkgConfig::LIBCURL
    PkgConfig::LIBARCHIVE
    PkgConfig::OPENSSL
    Threads::Threads)

install(TARGETS tuxblox-mac RUNTIME DESTINATION bin)
