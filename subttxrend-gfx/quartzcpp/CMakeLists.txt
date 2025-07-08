##############################################################################
#
# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright 2022 RDK Management
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
#
##############################################################################

project(quartzcpp)

cmake_minimum_required (VERSION 3.2)

#
# Directory with modules
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/")

#
# Packages to use
#
find_package(LibGlib REQUIRED)
find_library(APPKIT_LIB AppKit)
find_library(GLUT_LIB GLUT)
find_library(OPENGL_LIB OpenGl)
#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${LIBGLIB_INCLUDE_DIRS})

#
# Sources to compile
#
set(DVBGRAPHICSENGINE_SOURCES
    src/cocoa_window.mm
    )

#
# Targets
#
add_library(quartzcpp STATIC ${DVBGRAPHICSENGINE_SOURCES})
set_property(TARGET quartzcpp PROPERTY CXX_STANDARD 14)
target_link_libraries(quartzcpp PRIVATE ${APPKIT_LIB})
target_link_libraries(quartzcpp PRIVATE ${GLUT_LIB})
target_link_libraries(quartzcpp PRIVATE ${OPENGL_LIB})
install(FILES "assets/RDKLogo.png" DESTINATION assets)
