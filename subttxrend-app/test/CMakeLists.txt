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

project(subttxrend-app-test)

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
find_package(LibGlib REQUIRED)
find_package(LibSubTtxRendCtrl REQUIRED)
find_package(LibSubTtxRendCommon REQUIRED)
find_package(LibSubTtxRendProtocol REQUIRED)
find_package(LibSubTtxRendSockSrc REQUIRED)
find_package(LibSubTtxRendDbus REQUIRED)
find_package(LibSubTtxRendGfx REQUIRED)
find_package(LibSubTtxRendDvbSub REQUIRED)
find_package(LibSubTtxRendScte REQUIRED)
find_package(LibSubTtxRendCc REQUIRED)
find_package(LibSubTtxRendTtxt REQUIRED)
find_package(LibSubTtxRendTtml REQUIRED)
find_package(LibSubTtxRendWebvtt REQUIRED)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${LIBCPPUNIT_INCLUDE_DIRS})
include_directories(${LIBGLIB_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDCTRL_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDCOMMON_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDPROTOCOL_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDSOCKSRC_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDDBUS_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDGFX_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDDVBSUB_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDSCTE_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDCC_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDTTXT_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDTTML_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDWEBVTT_INCLUDE_DIRS})

#
# Macros
#
macro (add_cppunit_test _name)
    # invoke built-in add_executable
    add_executable(${ARGV})

    set_property(TARGET ${_name} PROPERTY CXX_STANDARD 14)

    target_link_libraries(${_name} ${LIBCPPUNIT_LIBRARIES})
    target_link_libraries(${_name} ${LIBGLIB_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDCTRL_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDCOMMON_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDPROTOCOL_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDSOCKSRC_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDDBUS_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDGFX_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDDVBSUB_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDSCTE_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDCC_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDTTXT_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDTTML_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDWEBVTT_LIBRARIES})

    add_test(NAME ${_name} COMMAND ${_name} )
endmacro()

#
# Tests
#
add_cppunit_test(Application_Test
                 Application_test.cpp
                 TestRunner.cpp
                 ../src/Application.cpp
                 ../src/Controller.cpp
                 )

add_cppunit_test(Controller_Test
                 Controller_test.cpp
                 TestRunner.cpp
                 ../src/Controller.cpp
                 ../src/Application.cpp
                 )
