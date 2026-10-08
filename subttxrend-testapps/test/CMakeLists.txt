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

project(subttxrend-testapps-test)

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
find_package(LibSubTtxRendProtocol REQUIRED)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/dataproxy)
include_directories(${LIBCPPUNIT_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDCOMMON_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDPROTOCOL_INCLUDE_DIRS})

#
# Macros
#
macro (add_cppunit_test _name)
    # invoke built-in add_executable
    add_executable(${ARGV})

    set_property(TARGET ${_name} PROPERTY CXX_STANDARD 14)

    target_link_libraries(${_name} ${LIBCPPUNIT_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDCOMMON_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDPROTOCOL_LIBRARIES})

    add_test(NAME ${_name} COMMAND ${_name} )
endmacro()

#
# Tests
#
add_cppunit_test(DataSourceFactory_Test
                 DataSourceFactory_test.cpp
                 ../src/common/DataSourceFactory.cpp
                 ../src/common/DataSource.cpp
                 ../src/common/Ipv4SocketSource.cpp
                 ../src/common/UnixSocketSource.cpp
                 ../src/common/PlainFileSource.cpp
                 ../src/common/SmartFileSource.cpp
                 ../src/common/RandomPacketSource.cpp
                 ../src/common/TtmlFileSource.cpp
                 ../src/common/WebvttFileSource.cpp
                 ../src/common/WebvttFileNoHeaderSource.cpp
                 ../src/common/DataPacket.cpp
                 TestRunner.cpp
)

add_cppunit_test(DataTargetFactory_Test
                 DataTargetFactory_test.cpp
                 ../src/common/DataTargetFactory.cpp
                 ../src/common/DataTarget.cpp
                 ../src/common/ConsoleLogTarget.cpp
                 ../src/common/Ipv4SocketTarget.cpp
                 ../src/common/PlainFileTarget.cpp
                 ../src/common/SmartFileTarget.cpp
                 ../src/common/UnixSocketTarget.cpp
                 ../src/common/DataPacket.cpp
                 TestRunner.cpp
)

add_cppunit_test(DataPacket_Test
                 DataPacket_test.cpp
                 ../src/common/DataPacket.cpp
                 TestRunner.cpp
)

add_cppunit_test(DataProxyApp_Test
                 DataProxyApp_test.cpp
                 ../src/dataproxy/DataProxyApp.cpp
                 ../src/common/DataSourceFactory.cpp
                 ../src/common/DataTargetFactory.cpp
                 ../src/common/DataSource.cpp
                 ../src/common/DataTarget.cpp
                 ../src/common/Ipv4SocketSource.cpp
                 ../src/common/Ipv4SocketTarget.cpp
                 ../src/common/UnixSocketSource.cpp
                 ../src/common/UnixSocketTarget.cpp
                 ../src/common/PlainFileSource.cpp
                 ../src/common/PlainFileTarget.cpp
                 ../src/common/SmartFileSource.cpp
                 ../src/common/SmartFileTarget.cpp
                 ../src/common/RandomPacketSource.cpp
                 ../src/common/TtmlFileSource.cpp
                 ../src/common/WebvttFileSource.cpp
                 ../src/common/WebvttFileNoHeaderSource.cpp
                 ../src/common/ConsoleLogTarget.cpp
                 ../src/common/DataPacket.cpp
                 TestRunner.cpp
)

add_cppunit_test(SmartFileSource_Test
                 SmartFileSource_test.cpp
                 ../src/common/SmartFileSource.cpp
                 ../src/common/PlainFileSource.cpp
                 ../src/common/DataSource.cpp
                 ../src/common/DataPacket.cpp
                 TestRunner.cpp
)

add_cppunit_test(SmartFileTarget_Test
                 SmartFileTarget_test.cpp
                 ../src/common/SmartFileTarget.cpp
                 ../src/common/PlainFileTarget.cpp
                 ../src/common/DataTarget.cpp
                 ../src/common/DataPacket.cpp
                 TestRunner.cpp
)

add_cppunit_test(TtmlFileSource_Test
                 TtmlFileSource_test.cpp
                 ../src/common/TtmlFileSource.cpp
                 ../src/common/PlainFileSource.cpp
                 ../src/common/DataSource.cpp
                 ../src/common/DataPacket.cpp
                 TestRunner.cpp
)

add_cppunit_test(WebvttFileSource_Test
                 WebvttFileSource_test.cpp
                 ../src/common/WebvttFileSource.cpp
                 ../src/common/PlainFileSource.cpp
                 ../src/common/DataSource.cpp
                 ../src/common/DataPacket.cpp
                 TestRunner.cpp
)

add_cppunit_test(Ipv4SocketTarget_Test
                 Ipv4SocketTarget_test.cpp
                 ../src/common/Ipv4SocketTarget.cpp
                 ../src/common/DataTarget.cpp
                 ../src/common/DataPacket.cpp
                 TestRunner.cpp
)

add_cppunit_test(Ipv4SocketSource_Test
                 Ipv4SocketSource_test.cpp
                 ../src/common/Ipv4SocketSource.cpp
                 ../src/common/DataSource.cpp
                 ../src/common/DataPacket.cpp
                 TestRunner.cpp
)

add_cppunit_test(UnixSocketSource_Test
                 UnixSocketSource_test.cpp
                 ../src/common/UnixSocketSource.cpp
                 ../src/common/DataSource.cpp
                 ../src/common/DataPacket.cpp
                 TestRunner.cpp
)

add_cppunit_test(UnixSocketTarget_Test
                 UnixSocketTarget_test.cpp
                 ../src/common/UnixSocketTarget.cpp
                 ../src/common/DataTarget.cpp
                 ../src/common/DataPacket.cpp
                 TestRunner.cpp
)
