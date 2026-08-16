# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2021 RDK Management
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

project(subttxrend-gfx-test)

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
find_package(LibGlib REQUIRED)
find_package(LibGio REQUIRED)
find_package(LibXkbCommon REQUIRED)
find_package(LibFontConfig REQUIRED)
find_package(Freetype REQUIRED)
find_package(HarfBuzz REQUIRED)
find_package(LibPng REQUIRED)

#
# Include directories
#
# Avoid leaking global includes into subprojects; set per-target below.

#
# Macros
#
macro (add_cppunit_test _name)
    # invoke built-in add_executable
    add_executable(${ARGV})

    set_property(TARGET ${_name} PROPERTY CXX_STANDARD 14)

    # Per-target include directories to avoid polluting subdirectories
    target_include_directories(${_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_SOURCE_DIR}/../ftcpp/include
        ${LIBCPPUNIT_INCLUDE_DIRS}
        ${LIBSUBTTXRENDCOMMON_INCLUDE_DIRS}
        ${LIBGLIB_INCLUDE_DIRS}
        ${LIBGIO_INCLUDE_DIRS}
        ${LIBXKBCOMMON_INCLUDE_DIRS}
        ${LIBFONTCONFIG_INCLUDE_DIRS}
        ${FREETYPE_INCLUDE_DIRS}
        ${LIBHARFBUZZ_INCLUDE_DIRS}
        ${LIBPNG_INCLUDE_DIRS}
    )

    target_link_libraries(${_name} ${LIBCPPUNIT_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDCOMMON_LIBRARIES})
    target_link_libraries(${_name} ${LIBXKBCOMMON_LIBRARIES})
    target_link_libraries(${_name} ${LIBFONTCONFIG_LIBRARIES})
    target_link_libraries(${_name} ${FREETYPE_LIBRARIES})
    target_link_libraries(${_name} ${LIBHARFBUZZ_LIBRARIES})
    target_link_libraries(${_name} ${LIBPNG_LIBRARIES})
    target_link_libraries(${_name} ${LIBGIO_LINK_LIBRARIES})
    target_link_libraries(${_name} ${LIBGLIB_LIBRARIES})

    target_compile_definitions(${_name} PRIVATE BACKEND_TYPE_EGL=2 BACKEND_TYPE_SHM=1)
    if(WITH_OPENGL)
    add_definitions(-DBACKEND_TYPE=BACKEND_TYPE_EGL)
    else()
    add_definitions(-DBACKEND_TYPE=BACKEND_TYPE_SHM)
    endif()

    add_test(NAME ${_name} COMMAND ${_name} )
endmacro()

#
# Tests
#
add_cppunit_test(EngineImpl_Test
                 EngineImpl_test.cpp
                 TestRunner.cpp
                 Logger.cpp
                 ../src/EngineImpl.cpp
                 ../src/WindowImpl.cpp
                 ../src/FontStripImpl.cpp
                 ../src/Blitter.cpp
                 ../src/ColorArgb.cpp
                 ../src/Factory.cpp
                 ../src/PrerenderedFontImpl.cpp
                 ../src/PrerenderedFontCache.cpp
                 ../src/Base64ToPixmap.cpp)

add_cppunit_test(WindowImpl_Test
                 WindowImpl_test.cpp
                 TestRunner.cpp
                 Logger.cpp
                 ../src/WindowImpl.cpp
                 ../src/FontStripImpl.cpp
                 ../src/Blitter.cpp
                 ../src/ColorArgb.cpp
                 ../src/PrerenderedFontImpl.cpp
                 ../src/PrerenderedFontCache.cpp
                 ../src/Base64ToPixmap.cpp)

add_cppunit_test(FontStripImpl_Test
                 FontStripImpl_test.cpp
                 TestRunner.cpp
                 Logger.cpp
                 ../src/FontStripImpl.cpp
                 ../src/Blitter.cpp
                 ../src/ColorArgb.cpp)

add_cppunit_test(Base64ToPixmap_Test
                 Base64ToPixmap_test.cpp
                 TestRunner.cpp
                 Logger.cpp
                 ../src/Base64ToPixmap.cpp)

add_cppunit_test(ColorArgb_Test
                 ColorArgb_test.cpp
                 TestRunner.cpp
                 Logger.cpp
                 ../src/ColorArgb.cpp)

add_cppunit_test(PrerenderedFontImpl_Test
                 PrerenderedFontImpl_test.cpp
                 TestRunner.cpp
                 Logger.cpp
                 ../src/PrerenderedFontImpl.cpp
                 ../src/FontStripImpl.cpp
                 ../src/Base64ToPixmap.cpp
                 ../src/Blitter.cpp
                 ../src/ColorArgb.cpp)

add_cppunit_test(PrerenderedFontCache_Test
                 PrerenderedFontCache_test.cpp
                 TestRunner.cpp
                 Logger.cpp
                 ../src/PrerenderedFontCache.cpp
                 ../src/PrerenderedFontImpl.cpp
                 ../src/FontStripImpl.cpp
                 ../src/Base64ToPixmap.cpp
                 ../src/Blitter.cpp
                 ../src/ColorArgb.cpp)
