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

project(subttxrend-ttxt-test)

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
find_package(LibTtxDecoder REQUIRED)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${LIBCPPUNIT_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDCOMMON_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDGFX_INCLUDE_DIRS})
include_directories(${LIBTTXDECODER_INCLUDE_DIRS})

#
# Macros
#
macro (add_cppunit_test _name)
    # invoke built-in add_executable
    add_executable(${ARGV})

    set_property(TARGET ${_name} PROPERTY CXX_STANDARD 14)

    target_link_libraries(${_name} ${LIBCPPUNIT_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDCOMMON_LIBRARIES})
    target_link_libraries(${_name} ${LIBSUBTTXRENDGFX_LIBRARIES})
    target_link_libraries(${_name} ${LIBTTXDECODER_LIBRARIES})

    add_test(NAME ${_name} COMMAND ${_name} )
endmacro()

#
# Tests
#

add_cppunit_test(BrowserRendererImpl_Test
                 BrowserRendererImpl_test.cpp
                 TestRunner.cpp
                 ../src/BrowserRendererImpl.cpp
                 ../src/RendererImpl.cpp
                 ../src/GfxRenderer.cpp
                 ../src/GfxConfig.cpp
                 ../src/GfxTtxGrid.cpp
                 ../src/GfxTtxGridModel.cpp
                 ../src/GfxTtxGridCell.cpp
                 ../src/GfxTtxClut.cpp
                 ../src/CharsetHandler.cpp
                 ../src/GfxMosaicGenerator.cpp)

add_cppunit_test(CharsetHandler_Test
                 CharsetHandler_test.cpp
                 TestRunner.cpp
                 ../src/CharsetHandler.cpp)

add_cppunit_test(GfxConfig_Test
                 GfxConfig_test.cpp
                 TestRunner.cpp
                 ../src/GfxConfig.cpp)

add_cppunit_test(GfxMosaicGenerator_Test
                 GfxMosaicGenerator_test.cpp
                 TestRunner.cpp
                 ../src/GfxMosaicGenerator.cpp)

add_cppunit_test(GfxRenderer_Test
                 GfxRenderer_test.cpp
                 TestRunner.cpp
                 ../src/GfxRenderer.cpp
                 ../src/GfxConfig.cpp
                 ../src/GfxTtxGrid.cpp
                 ../src/GfxTtxGridModel.cpp
                 ../src/GfxTtxGridCell.cpp
                 ../src/GfxTtxClut.cpp
                 ../src/CharsetHandler.cpp
                 ../src/GfxMosaicGenerator.cpp)

add_cppunit_test(GfxTtxClut_Test
                 GfxTtxClut_test.cpp
                 TestRunner.cpp
                 ../src/GfxTtxClut.cpp)

add_cppunit_test(GfxTtxGrid_Test
                 GfxTtxGrid_test.cpp
                 TestRunner.cpp
                 ../src/GfxTtxGrid.cpp
                 ../src/GfxTtxGridModel.cpp
                 ../src/GfxTtxGridCell.cpp
                 ../src/GfxTtxClut.cpp
                 ../src/CharsetHandler.cpp
                 ../src/GfxMosaicGenerator.cpp)

add_cppunit_test(GfxTtxGridCell_Test
                 GfxTtxGridCell_test.cpp
                 TestRunner.cpp
                 ../src/GfxTtxGridCell.cpp)

add_cppunit_test(GfxTtxGridModel_Test
                 GfxTtxGridModel_test.cpp
                 TestRunner.cpp
                 ../src/GfxTtxGridModel.cpp
                 ../src/GfxTtxGridCell.cpp
                 ../src/GfxTtxClut.cpp
                 ../src/CharsetHandler.cpp)

add_cppunit_test(RendererImpl_Test
                 RendererImpl_test.cpp
                 TestRunner.cpp
                 ../src/RendererImpl.cpp
                 ../src/GfxRenderer.cpp
                 ../src/GfxConfig.cpp
                 ../src/GfxTtxGrid.cpp
                 ../src/GfxTtxGridModel.cpp
                 ../src/GfxTtxGridCell.cpp
                 ../src/GfxTtxClut.cpp
                 ../src/CharsetHandler.cpp
                 ../src/GfxMosaicGenerator.cpp)

add_cppunit_test(SubtitlesRendererImpl_Test
                 SubtitlesRendererImpl_test.cpp
                 TestRunner.cpp
                 ../src/SubtitlesRendererImpl.cpp
                 ../src/RendererImpl.cpp
                 ../src/GfxRenderer.cpp
                 ../src/GfxConfig.cpp
                 ../src/GfxTtxGrid.cpp
                 ../src/GfxTtxGridModel.cpp
                 ../src/GfxTtxGridCell.cpp
                 ../src/GfxTtxClut.cpp
                 ../src/CharsetHandler.cpp
                 ../src/GfxMosaicGenerator.cpp)
