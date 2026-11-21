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

set(JSON_LICENSE_SRC "${CMAKE_SOURCE_DIR}/assets/json-LICENSE.MIT")
set(LGPL21_LICENSE_SRC "${CMAKE_SOURCE_DIR}/licenses/lgpl-2.1.txt")
set(GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
set(JSON_LICENSE_TXT_PATH "${GENERATED_DIR}/json-LICENSE.MIT")
set(LGPL21_LICENSE_TXT_PATH "${GENERATED_DIR}/lgpl-2.1.txt")
set(JSON_LICENSE_HEADER_PATH "${GENERATED_DIR}/json_license_txt.h")
set(LGPL21_LICENSE_HEADER_PATH "${GENERATED_DIR}/lgpl21_license_txt.h")

file(MAKE_DIRECTORY ${GENERATED_DIR})

add_custom_command(
    OUTPUT ${JSON_LICENSE_HEADER_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy ${JSON_LICENSE_SRC} ${JSON_LICENSE_TXT_PATH}
    COMMAND ${CMAKE_COMMAND} -DINPUT=${JSON_LICENSE_TXT_PATH} -DOUTPUT=${JSON_LICENSE_HEADER_PATH}
            -DSYMBOL=kJsonLicenseTxt
            -P ${CMAKE_SOURCE_DIR}/cmake/BinToHeader.cmake
    VERBATIM
)

add_custom_command(
    OUTPUT ${LGPL21_LICENSE_HEADER_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy ${LGPL21_LICENSE_SRC} ${LGPL21_LICENSE_TXT_PATH}
    COMMAND ${CMAKE_COMMAND} -DINPUT=${LGPL21_LICENSE_TXT_PATH} -DOUTPUT=${LGPL21_LICENSE_HEADER_PATH}
            -DSYMBOL=kLgpl21LicenseTxt
            -P ${CMAKE_SOURCE_DIR}/cmake/BinToHeader.cmake
    DEPENDS ${LGPL21_LICENSE_SRC}
    VERBATIM
)

add_custom_target(generate_thirdparty_license_headers DEPENDS
    ${JSON_LICENSE_HEADER_PATH} ${LGPL21_LICENSE_HEADER_PATH})
