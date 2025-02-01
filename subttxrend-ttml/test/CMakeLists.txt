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
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/")

#
# Packages to use
#
find_package(LibCppUnit REQUIRED)
find_package(LibSubTtxRendCommon REQUIRED)
find_package(LibSubTtxRendGfx REQUIRED)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/stub)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${LIBCPPUNIT_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDGFX_INCLUDE_DIRS})

#
# Macros
#
macro (add_cppunit_test _name)
    # invoke built-in add_executable
    add_executable(${ARGV})

    set_property(TARGET ${_name} PROPERTY CXX_STANDARD 14)

    target_link_libraries(${_name} ${LIBSUBTTXRENDGFX_LIBRARIES})
    target_link_libraries(${_name} ${LIBCPPUNIT_LIBRARIES})

    add_test(NAME ${_name} COMMAND ${_name} )
endmacro()

#
# Tests
#
add_cppunit_test(TtmlTiming_Test
                 Elements_test.cpp
                 TtmlTiming_test.cpp
                 TestRunner.cpp
                 ../src/Parser/AttributeHandlers.cpp
                 )
                 
add_cppunit_test(StyleSet_Test
                 StyleSet_test.cpp
                 ../src/Parser/AttributeHandlers.cpp
                 ../src/Parser/Outline.cpp
                 ../src/Parser/StyleSet.cpp
                 ../src/Parser/Utils.cpp
                 TestRunner.cpp
                 )

add_cppunit_test(DocumentInstance_Test
                 DocumentInstance_test.cpp
                 ../src/Parser/AttributeHandlers.cpp
                 ../src/Parser/Outline.cpp
                 ../src/Parser/StyleSet.cpp
                 ../src/Parser/Utils.cpp
                 TestRunner.cpp
                 )

add_cppunit_test(AttributeHandlers_test
                 AttributeHandlers_test.cpp
                 TestRunner.cpp
                 ../src/Parser/AttributeHandlers.cpp
                 )
