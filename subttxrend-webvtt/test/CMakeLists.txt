# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2018 RDK Management
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

project(subttxrend-webvtt-tests)

enable_testing()

cmake_minimum_required (VERSION 3.2)

#
# Directory with modules
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/")

#
# Packages to use
#
find_package(LibCppUnit)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/Parser/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/)
include_directories(${LIBCPPUNIT_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/Parser/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

if(CMAKE_TRACE_DEBUG)
    message("Setting TRACE_DEBUG")
    set(TEST_DEFINES "${TEST_DEFINES} -DTRACE_DEBUG")
else()
    message("No TRACE_DEBUG")
endif()

if(CMAKE_NO_DEBUG)
    message("Setting no debug")
    set(TEST_DEFINES "${TEST_DEFINES} -DNODEBUG")
endif()

#
# Macros
#
macro (add_cppunit_test _name)
    # invoke built-in add_executable
    add_executable(${ARGV} "${WEBVTT_SOURCES}")

    set_property(TARGET ${_name} PROPERTY CXX_STANDARD 14)
if(PCDATA_LIBCPPUNIT_FOUND)
    target_link_libraries(${_name} ${LIBCPPUNIT_LIBRARIES})
else()
    target_link_libraries(${_name} cppunit)
endif()
    set_target_properties(${_name} PROPERTIES COMPILE_FLAGS "${TEST_DEFINES}")
    target_include_directories(${_name} PRIVATE ${INCLUDE_DIRS})

    add_test(NAME ${_name} COMMAND ${_name} )
endmacro()

set(CMAKE_BUILD_STANDALONE 1)

#
# Tests
#
if(CMAKE_BUILD_STANDALONE)
    add_cppunit_test(WebVTTCue_test
                    WebVTTCue_test.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Parser/WebVTTStyle.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Parser/WebVTTCue.cpp
                    TestRunner.cpp
                    )

    add_cppunit_test(WebVTTDocument_test
                    WebVTTDocument_test.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Parser/WebVTTCue.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Parser/WebVTTStyle.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Parser/WebVTTDocument.cpp
                    TestRunner.cpp
                    )

    add_cppunit_test(WebVTTConverter_test
                    WebVTTConverter_test.cpp
                    TestRunner.cpp
                    )

    add_cppunit_test(LineBuilder_test
                    ${CMAKE_CURRENT_SOURCE_DIR}/../src/LineBuilder.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Parser/WebVTTStyle.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Parser/WebVTTCue.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Parser/WebVTTDocument.cpp
                    LineBuilder_test.cpp
                    TestRunner.cpp
                    )

    add_cppunit_test(WebVTTAttributes_test
                    WebVTTAttributes_test.cpp
                    TestRunner.cpp
                    )
else()
    add_cppunit_test(All_test
                    WebvttParser_test.cpp
                    WebVTTParse_test.cpp
                    WebVTTCue_test.cpp
                    TestRunner.cpp
                    )
endif()

