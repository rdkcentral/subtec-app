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

project(waylandcpp)

cmake_minimum_required (VERSION 3.2)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#
# Directory with modules
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/")

#
# Packages to use
#
find_package(LibSubTtxRendCommon REQUIRED)
find_package(LibWaylandClient REQUIRED)
find_package(LibXkbCommon REQUIRED)
find_package(WaylandProtocols REQUIRED)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/waylandcpp-client)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/waylandcpp-utils)
include_directories(${CMAKE_BINARY_DIR})
include_directories(${LIBSUBTTXRENDCOMMON_INCLUDE_DIRS})
include_directories(${LIBWAYLANDCLIENT_INCLUDE_DIRS})
include_directories(${LIBXKBCOMMON_INCLUDE_DIRS})

#
# Generate the XDG shell protocol source
#
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/xdg-shell-protocol.c
    COMMAND wayland-scanner private-code ${WaylandProtocols_pkgdatadir}/stable/xdg-shell/xdg-shell.xml ${CMAKE_BINARY_DIR}/xdg-shell-protocol.c
)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.h
    COMMAND wayland-scanner client-header ${WaylandProtocols_pkgdatadir}/stable/xdg-shell/xdg-shell.xml ${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.h
)

add_custom_target(genhdr DEPENDS ${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.h)

#
# Sources to compile
#
set(DVBGRAPHICSENGINE_SOURCES
    src/waylandcpp-client/Buffer.cpp
    src/waylandcpp-client/Client.cpp
    src/waylandcpp-client/Compositor.cpp
    src/waylandcpp-client/Display.cpp
    src/waylandcpp-client/File.cpp
    src/waylandcpp-client/Keyboard.cpp
    src/waylandcpp-client/ObjectFactory.cpp
    src/waylandcpp-client/Output.cpp
    src/waylandcpp-client/PixelFormat.cpp
    src/waylandcpp-client/Registry.cpp
    src/waylandcpp-client/Seat.cpp
    src/waylandcpp-client/Shell.cpp
    src/waylandcpp-client/ShellSurface.cpp
    src/waylandcpp-client/Shm.cpp
    src/waylandcpp-client/ShmPool.cpp
    src/waylandcpp-client/Surface.cpp
    src/waylandcpp-client/XdgSurface.cpp
    src/waylandcpp-client/XdgToplevel.cpp
    src/waylandcpp-client/XdgWmBase.cpp

    src/waylandcpp-utils/Epoll.cpp
    src/waylandcpp-utils/EpollDisplayHandler.cpp
    src/waylandcpp-utils/EpollEventHandler.cpp
    src/waylandcpp-utils/EpollDisplayLoop.cpp
    src/waylandcpp-utils/KeymapFactory.cpp
    src/waylandcpp-utils/KeymapXkbV1.cpp

    ${CMAKE_BINARY_DIR}/xdg-shell-protocol.c
    )

if(WITH_WESTEROS)
list(APPEND DVBGRAPHICSENGINE_SOURCES src/waylandcpp-client/SimpleShell.cpp)
add_definitions(-DWESTEROS)
endif(WITH_WESTEROS)

#
# Targets
#
add_library(waylandcpp STATIC ${DVBGRAPHICSENGINE_SOURCES})
set_property(TARGET waylandcpp PROPERTY CXX_STANDARD 14)
target_link_libraries(waylandcpp ${LIBSUBTTXRENDCOMMON_LIBRARIES})
target_link_libraries(waylandcpp ${LIBWAYLANDCLIENT_LIBRARIES})
target_link_libraries(waylandcpp ${LIBXKBCOMMON_LIBRARIES})
add_dependencies(waylandcpp genhdr)
