##############################################################################
# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2021 Liberty Global Service B.V.#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##############################################################################

project(dvbsubdecoder)

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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/dvbsubdecoder)
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
set(DVBSUBDECODER_PUBLIC_HEADERS
    include/dvbsubdecoder/Allocator.hpp
    include/dvbsubdecoder/BasicAllocator.hpp
    include/dvbsubdecoder/Decoder.hpp
    include/dvbsubdecoder/DecoderClient.hpp
    include/dvbsubdecoder/DecoderFactory.hpp
    include/dvbsubdecoder/DynamicAllocator.hpp
    include/dvbsubdecoder/TimeProvider.hpp
    include/dvbsubdecoder/Types.hpp
)

#
# Sources to compile
#
set(DVBSUBDECODER_SOURCES
    src/BasicAllocator.cpp
    src/Clut.cpp
    src/ColorCalculator.cpp
    src/Database.cpp
    src/DecoderFactory.cpp
    src/DecoderImpl.cpp
    src/DynamicAllocator.cpp
    src/ObjectParser.cpp
    src/Parser.cpp
    src/ParserCDS.cpp
    src/ParserDDS.cpp
    src/ParserEDS.cpp
    src/ParserODS.cpp
    src/ParserPCS.cpp
    src/ParserRCS.cpp
    src/PesBuffer.cpp
    src/PesPacketReader.cpp
    src/PixelWriter.cpp
    src/Pixmap.cpp
    src/PixmapAllocator.cpp
    src/Presenter.cpp
    src/Region.cpp
    src/Status.cpp
    src/Storage.cpp
)

#
# Targets
#
set(LIBRARY_NAME "dvbsubdecoder")
add_library(${LIBRARY_NAME} SHARED ${DVBSUBDECODER_SOURCES})
set_property(TARGET ${LIBRARY_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${LIBRARY_NAME} PROPERTY VERSION 0.0.0)
set_property(TARGET ${LIBRARY_NAME} PROPERTY SOVERSION 0)
set_property(TARGET ${LIBRARY_NAME} PROPERTY PUBLIC_HEADER ${DVBSUBDECODER_PUBLIC_HEADERS})
target_link_libraries(${LIBRARY_NAME} ${LIBSUBTTXRENDCOMMON_LIBRARIES})

#
# Install rules
#
install(TARGETS ${LIBRARY_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/dvbsubdecoder
)

# Use the pkgconfig
find_package(PkgConfig REQUIRED)

#
# Package config info
#
set(PACKAGE_NAME        "${PROJECT_NAME}")
set(PACKAGE_DESCRIPTION "DVB Subtitles Decoder")
set(PACKAGE_VERSION     "0.0.0")
set(PACKAGE_REQUIRES    "${LIBSUBTTXRENDCOMMON_PACKAGE_NAME}")
set(PACKAGE_LIB_NAME    "${LIBRARY_NAME}")

if(PKG_CONFIG_FOUND)
    configure_file("${PACKAGE_NAME}.pc.in"
                   "${PACKAGE_NAME}.pc"
                    @ONLY)
    install(FILES       "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}.pc"
            DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")
endif(PKG_CONFIG_FOUND)
