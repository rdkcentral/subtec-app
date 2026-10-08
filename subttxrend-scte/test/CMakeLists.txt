##############################################################################
# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2021 Liberty Global Service B.V.
#
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

project(subttxrend-scte-test)

enable_testing()

cmake_minimum_required (VERSION 3.2)

#
# Directory with modules
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/")

#
# Packages to use
#
find_package(LibCppUnit REQUIRED)
find_package(LibSubTtxRendCommon REQUIRED)
find_package(LibSubTtxRendGfx REQUIRED)
find_package(LibSubTtxRendProtocol REQUIRED)
find_package(ZLIB REQUIRED)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${LIBCPPUNIT_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDGFX_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDCOMMON_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDPROTOCOL_INCLUDE_DIRS})

#
# Macros
#
macro (add_cppunit_test _name)
    # invoke built-in add_executable
    add_executable(${ARGV})

    set_property(TARGET ${_name} PROPERTY CXX_STANDARD 14)

    target_link_libraries(${_name} ${LIBSUBTTXRENDGFX_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDCOMMON_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDPROTOCOL_LIBRARIES})
    target_link_libraries(${_name} ${LIBCPPUNIT_LIBRARIES})
    target_link_libraries(${_name} ${ZLIB_LIBRARIES})

    add_test(NAME ${_name} COMMAND ${_name} )
endmacro()

#
# Tests
#
add_cppunit_test(ScteBitStream_Test
                 ScteBitStream_test.cpp
                 TestRunner.cpp
                 ../src/ScteBitStream.cpp
                 )

add_cppunit_test(ScteController_Test
                 ScteController_test.cpp
                 TestRunner.cpp
                 ../src/ScteController.cpp
                 ../src/ScteSectionAssembler.cpp
                 ../src/ScteSection.cpp
                 ../src/ScteTable.cpp
                 ../src/ScteBitStream.cpp
                 ../src/ScteRawBitmapDecoder.cpp
                 ../src/ScteRawBitmap.cpp
                 ../src/ScteSimpleBitmap.cpp
                 ../src/ScteRenderer.cpp
                 ../src/ScteOutliner.cpp
                 )

add_cppunit_test(ScteRenderer_Test
                 ScteRenderer_test.cpp
                 TestRunner.cpp
                 ../src/ScteRenderer.cpp
                 ../src/ScteOutliner.cpp
                 ../src/ScteSimpleBitmap.cpp
                 ../src/ScteRawBitmap.cpp
                 ../src/ScteRawBitmapDecoder.cpp
                 ../src/ScteBitStream.cpp
                 )

add_cppunit_test(ScteRawBitmapDecoder_Test
                 ScteRawBitmapDecoder_test.cpp
                 TestRunner.cpp
                 ../src/ScteRawBitmapDecoder.cpp
                 ../src/ScteRawBitmap.cpp
                 ../src/ScteSimpleBitmap.cpp
                 ../src/ScteBitStream.cpp
                 )

add_cppunit_test(ScteSection_Test
                 ScteSection_test.cpp
                 TestRunner.cpp
                 ../src/ScteSection.cpp
                 ../src/ScteBitStream.cpp
                 )

add_cppunit_test(ScteTable_Test
                 ScteTable_test.cpp
                 TestRunner.cpp
                 ../src/ScteTable.cpp
                 ../src/ScteSection.cpp
                 ../src/ScteBitStream.cpp
                 ../src/ScteRawBitmapDecoder.cpp
                 ../src/ScteRawBitmap.cpp
                 ../src/ScteSimpleBitmap.cpp
                 )

add_cppunit_test(ScteSectionAssembler_Test
                 ScteSectionAssembler_test.cpp
                 TestRunner.cpp
                 ../src/ScteSectionAssembler.cpp
                 ../src/ScteSection.cpp
                 ../src/ScteBitStream.cpp
                 ../src/ScteTable.cpp
                 ../src/ScteSimpleBitmap.cpp
                 ../src/ScteRawBitmap.cpp
                 ../src/ScteRawBitmapDecoder.cpp
                 )

add_cppunit_test(ScteOutliner_Test
                 ScteOutliner_test.cpp
                 TestRunner.cpp
                 ../src/ScteOutliner.cpp
                 )

add_cppunit_test(ScteSimpleBitmap_Test
                 ScteSimpleBitmap_test.cpp
                 TestRunner.cpp
                 ../src/ScteSimpleBitmap.cpp
                 ../src/ScteRawBitmap.cpp
                 ../src/ScteRawBitmapDecoder.cpp
                 ../src/ScteBitStream.cpp
                 )

add_cppunit_test(ScteRawBitmap_Test
                 ScteRawBitmap_test.cpp
                 TestRunner.cpp
                 ../src/ScteRawBitmap.cpp
                 )
