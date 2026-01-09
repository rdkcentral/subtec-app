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

project(subttxrend-ctrl-test)

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
add_cppunit_test(CcSubController_Test
                 CcSubController_test.cpp
                 TestRunner.cpp
                 ../src/CcSubController.cpp
                 ../src/StcProvider.cpp)

add_cppunit_test(Configuration_Test
                 Configuration_test.cpp
                 TestRunner.cpp
                 ../src/Configuration.cpp
                 ../src/Options.cpp)

add_cppunit_test(DvbSubController_Test
                 DvbSubController_test.cpp
                 TestRunner.cpp
                 ../src/DvbSubController.cpp
                 ../src/StcProvider.cpp
                 ../src/PesFinder.cpp)

add_cppunit_test(Options_Test
                 Options_test.cpp
                 TestRunner.cpp
                 ../src/Options.cpp)

add_cppunit_test(PesFinder_Test
                 PesFinder_test.cpp
                 TestRunner.cpp
                 ../src/PesFinder.cpp)

add_cppunit_test(ScteSubController_Test
                 ScteSubController_test.cpp
                 TestRunner.cpp
                 ../src/ScteSubController.cpp
                 ../src/StcProvider.cpp)

add_cppunit_test(StcProvider_Test
                 StcProvider_test.cpp
                 TestRunner.cpp
                 ../src/StcProvider.cpp)

add_cppunit_test(TtmlController_Test
                 TtmlController_test.cpp
                 TestRunner.cpp
                 ../src/TtmlController.cpp
                 ../src/StcProvider.cpp)

add_cppunit_test(TtxController_Test
                 TtxController_test.cpp
                 TestRunner.cpp
                 ../src/TtxController.cpp
                 ../src/StcProvider.cpp
                 ../src/PesFinder.cpp)

add_cppunit_test(WebvttController_Test
                 WebvttController_test.cpp
                 TestRunner.cpp
                 ../src/WebvttController.cpp
                 ../src/StcProvider.cpp)
