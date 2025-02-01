##############################################################################
# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2021 Liberty Global Service B.V.#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##############################################################################

project(subttxrend-protocol)

cmake_minimum_required (VERSION 3.2)

#
# Directory with modules
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

#
# Extra compiler / linker options
#
IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Werror -Wformat=2")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

#
# Packages to use
#
find_package(LibSubTtxRendCommon REQUIRED)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${LIBSUBTTXRENDCOMMON_INCLUDE_DIRS})

#
# Documentation
#
set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
set(DOXYGEN_INPUT_RECURSIVE "YES")
set(DOXYGEN_INPUT_FOLDERS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
foreach(arg ${DOXYGEN_INPUT_FOLDERS})
    set(DOXYGEN_INPUT "${DOXYGEN_INPUT} ${arg}")
endforeach(arg ${foo})

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY
    )
    add_custom_target(
        doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)

#
# Public headers
#
set(SUBTTXREND_PROTOCOL_PUBLIC_HEADERS
    include/Packet.hpp
    
    include/PacketData.hpp
    include/PacketTimestamp.hpp
    include/PacketPause.hpp
    include/PacketResume.hpp
    include/PacketMute.hpp
    include/PacketUnmute.hpp
    include/PacketResetAll.hpp
    include/PacketResetChannel.hpp
    include/PacketSubtitleSelection.hpp
    include/PacketTeletextSelection.hpp
    include/PacketTtmlSelection.hpp
    include/PacketTtmlTimestamp.hpp
    include/PacketTtmlInfo.hpp
    include/PacketWebvttSelection.hpp
    include/PacketWebvttTimestamp.hpp
    include/PacketSetCCAttributes.hpp
    
    include/PacketParser.hpp
    
    include/StreamValidator.hpp
    )

#
# Sources to compile
#
set(SUBTTXREND_PROTOCOL_SOURCES
    src/Buffer.cpp
    src/BufferReader.cpp
    src/Packet.cpp
    src/PacketInvalid.cpp
    src/PacketData.cpp
    src/PacketSubtitleSelection.cpp
    src/PacketTeletextSelection.cpp
    src/PacketTimestamp.cpp
    src/PacketTtmlSelection.cpp
    src/PacketTtmlTimestamp.cpp
    src/PacketTtmlInfo.cpp
    src/PacketWebvttSelection.cpp
    src/PacketWebvttTimestamp.cpp
    src/PacketSetCCAttributes.cpp

    src/PacketParser.cpp
    src/StreamValidator.cpp
    )

#
# Targets
#
set(LIBRARY_NAME "subttxrend-protocol")
add_library(${LIBRARY_NAME} SHARED ${SUBTTXREND_PROTOCOL_SOURCES})
set_property(TARGET ${LIBRARY_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${LIBRARY_NAME} PROPERTY VERSION 0.0.0)
set_property(TARGET ${LIBRARY_NAME} PROPERTY SOVERSION 0)
set_property(TARGET ${LIBRARY_NAME} PROPERTY PUBLIC_HEADER ${SUBTTXREND_PROTOCOL_PUBLIC_HEADERS})
target_link_libraries(${LIBRARY_NAME} ${LIBSUBTTXRENDCOMMON_LIBRARIES})
target_link_libraries(${LIBRARY_NAME} ${LIBGIO_LIBRARIES})
target_link_libraries(${LIBRARY_NAME} ${LIBGLIB_LIBRARIES})

#
# Install rules
#
install (TARGETS ${LIBRARY_NAME}
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         PUBLIC_HEADER DESTINATION include/subttxrend/protocol
)

# Use the pkgconfig
find_package(PkgConfig REQUIRED)

#
# Package config info
#
set(PACKAGE_NAME        "${PROJECT_NAME}")
set(PACKAGE_DESCRIPTION "Subtitles/Teletext Renderer - Protocol")
set(PACKAGE_VERSION     "0.0.0")
set(PACKAGE_REQUIRES    "")
set(PACKAGE_LIB_NAME    "${LIBRARY_NAME}")

if(PKG_CONFIG_FOUND)
    configure_file("${PACKAGE_NAME}.pc.in"
                   "${PACKAGE_NAME}.pc"
                    @ONLY)
    install(FILES       "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}.pc"
            DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")
endif(PKG_CONFIG_FOUND)
