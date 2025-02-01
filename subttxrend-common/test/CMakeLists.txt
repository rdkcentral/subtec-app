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

project(subttxrend-common-test)

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

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${LIBCPPUNIT_INCLUDE_DIRS})

#
# Macros
#
macro (add_cppunit_test _name)
    # invoke built-in add_executable
    add_executable(${ARGV})
    
    set_property(TARGET ${_name} PROPERTY CXX_STANDARD 11)

    target_link_libraries(${_name} ${LIBCPPUNIT_LIBRARIES})

    add_test(NAME ${_name} COMMAND ${_name} )
endmacro()

#
# Data files
#
configure_file("test1.ini" "test1.ini" COPYONLY)
configure_file("test1a.ini" "test1a.ini" COPYONLY)
configure_file("test2.ini" "test2.ini" COPYONLY)
configure_file("test3.ini" "test3.ini" COPYONLY)

#
# Tests
#
add_cppunit_test(StringUtils_Test
                 StringUtils_test.cpp
                 TestRunner.cpp
                 ../src/StringUtils.cpp)

#
# Tests
#
add_cppunit_test(IniFile_Test
                 IniFile_test.cpp
                 TestRunner.cpp
                 ../src/IniFile.cpp
                 rdk_debug.cpp
                 ../src/ConfigProvider.cpp
                 ../src/ConfigProviderStorage.cpp
                 ../src/Logger.cpp
                 ../src/LoggerBackendRdk.cpp
                 ../src/LoggerBackendStd.cpp
                 ../src/LoggerManagerImpl.cpp
                 ../src/StringUtils.cpp)
