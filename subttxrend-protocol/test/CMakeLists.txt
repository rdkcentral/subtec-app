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

project(subttxrend-protocol-test)

enable_testing()

cmake_minimum_required (VERSION 3.2)

#
# Directory with modules
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/" "${CMAKE_MODULE_PATH}/websocket-ipplayer2-utils")

#
# Packages to use
#
find_package(LibCppUnit REQUIRED)
find_package(LibSubTtxRendCommon REQUIRED)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${LIBCPPUNIT_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDCOMMON_INCLUDE_DIRS})

#
# Macros
#
macro (add_cppunit_test _name)
    # invoke built-in add_executable
    add_executable(${ARGV})

    set_property(TARGET ${_name} PROPERTY CXX_STANDARD 14)

    target_link_libraries(${_name} ${LIBCPPUNIT_LIBRARIES})


    add_test(NAME ${_name} COMMAND ${_name} )
endmacro()

#
# Tests
#
add_cppunit_test(PacketData_Test
                 PacketData_test.cpp
                 TestRunner.cpp
                 Logger.cpp
                 ../src/PacketData.cpp
                 ../src/Packet.cpp
                 ../src/Buffer.cpp
                 ../src/BufferReader.cpp)

add_cppunit_test(PacketTimestamp_Test
                 PacketTimestamp_test.cpp
                 TestRunner.cpp
                 Logger.cpp
                 ../src/PacketTimestamp.cpp
                 ../src/Packet.cpp
                 ../src/Buffer.cpp
                 ../src/BufferReader.cpp)

add_cppunit_test(PacketTtmlInfo_Test
                 PacketTtmlInfo_test.cpp
                 TestRunner.cpp
                 Logger.cpp
                 ../src/PacketTtmlInfo.cpp
                 ../src/Packet.cpp
                 ../src/Buffer.cpp
                 ../src/BufferReader.cpp)

add_cppunit_test(PacketResetAll_Test
                 PacketResetAll_test.cpp
                 TestRunner.cpp
                 Logger.cpp
                 ../src/Packet.cpp
                 ../src/Buffer.cpp
                 ../src/BufferReader.cpp)

add_cppunit_test(PacketResetChannel_Test
                 PacketResetChannel_test.cpp
                 TestRunner.cpp
                 Logger.cpp
                 ../src/Packet.cpp
                 ../src/Buffer.cpp
                 ../src/BufferReader.cpp)

add_cppunit_test(PacketSubtitleSelection_Test
                 PacketSubtitleSelection_test.cpp
                 TestRunner.cpp
                 Logger.cpp
                 ../src/PacketSubtitleSelection.cpp
                 ../src/Packet.cpp
                 ../src/Buffer.cpp
                 ../src/BufferReader.cpp)

add_cppunit_test(PacketTeletextSelection_Test
                 PacketTeletextSelection_test.cpp
                 TestRunner.cpp
                 Logger.cpp
                 ../src/PacketTeletextSelection.cpp
                 ../src/Packet.cpp
                 ../src/Buffer.cpp
                 ../src/BufferReader.cpp)

add_cppunit_test(StreamValidator_Test
                 StreamValidator_test.cpp
                 TestRunner.cpp
                 Logger.cpp
                 ../src/StreamValidator.cpp
                 ../src/PacketTimestamp.cpp
                 ../src/Packet.cpp
                 ../src/Buffer.cpp
                 ../src/BufferReader.cpp)
