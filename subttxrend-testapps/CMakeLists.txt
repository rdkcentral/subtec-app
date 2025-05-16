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

project(subttxrend-testapps)

cmake_minimum_required (VERSION 3.2)

#
# Directory with modules
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/" "${CMAKE_MODULE_PATH}/websocket-ipplayer2-utils")

#
# Extra compiler / linker options
#
IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall  -Wno-unused-parameter  -Wextra -Werror -Wformat=2 ")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

#
# Packages to use
#
find_package(LibSubTtxRendCommon REQUIRED)
find_package(LibSubTtxRendProtocol REQUIRED)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${LIBSUBTTXRENDCOMMON_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDPROTOCOL_INCLUDE_DIRS})

#
# Sources to compile
#
set(SUBTTXREND_TESTAPPS_COMMON_SOURCES
    src/common/DataPacket.cpp
    
    src/common/DataSource.cpp
    src/common/Ipv4SocketSource.cpp
    src/common/PlainFileSource.cpp
    src/common/RandomPacketSource.cpp
    src/common/SmartFileSource.cpp
    src/common/TtmlFileSource.cpp
    src/common/WebvttFileSource.cpp
    src/common/WebvttFileNoHeaderSource.cpp
    src/common/UnixSocketSource.cpp
    
    src/common/DataSourceFactory.cpp
    
    src/common/DataTarget.cpp
    src/common/ConsoleLogTarget.cpp
    src/common/Ipv4SocketTarget.cpp
    src/common/PlainFileTarget.cpp
    src/common/SmartFileTarget.cpp
    src/common/UnixSocketTarget.cpp

    src/common/DataTargetFactory.cpp
)

set(SUBTTXREND_TESTAPPS_DATAPROXY_SOURCES
    src/dataproxy/main.cpp
    src/dataproxy/DataProxyApp.cpp
)

#
# Targets
#
add_library(subttxrend-testapps-common STATIC ${SUBTTXREND_TESTAPPS_COMMON_SOURCES})
set_property(TARGET subttxrend-testapps-common PROPERTY CXX_STANDARD 14)

add_executable(subttxrend-dataproxy ${SUBTTXREND_TESTAPPS_DATAPROXY_SOURCES})
set_property(TARGET subttxrend-dataproxy PROPERTY CXX_STANDARD 14)
target_link_libraries(subttxrend-dataproxy subttxrend-testapps-common)
target_link_libraries(subttxrend-dataproxy ${LIBSUBTTXRENDPROTOCOL_LIBRARIES})

#
# Install rules
#
install(TARGETS subttxrend-dataproxy
        RUNTIME DESTINATION bin)
