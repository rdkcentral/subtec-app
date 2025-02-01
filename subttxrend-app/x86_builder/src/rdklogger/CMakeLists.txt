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

project(rdklogger)

cmake_minimum_required (VERSION 3.2)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#
# Public headers
#
set(RDKLOGGER_PUBLIC_HEADERS
    include/rdk_debug.h
    )

#
# Sources to compile
#
set(RDKLOGGER_SOURCES
    src/logger.cpp
    )

#
# Targets
#
add_library(rdkloggers SHARED ${RDKLOGGER_SOURCES})
set_property(TARGET rdkloggers PROPERTY CXX_STANDARD 11)
set_property(TARGET rdkloggers PROPERTY PUBLIC_HEADER ${RDKLOGGER_PUBLIC_HEADERS})

#
# Install rules
#
install (TARGETS rdkloggers
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         PUBLIC_HEADER DESTINATION include
         )
