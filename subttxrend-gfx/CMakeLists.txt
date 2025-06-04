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

project(subttxrend-gfx)

cmake_minimum_required (VERSION 3.2)
if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
add_subdirectory(quartzcpp)
else()
add_subdirectory(waylandcpp)
endif()
#
# Directory with modules
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

#
# Configuration variables
#
option(WITH_OPENGL      "Include the OpenGL support" OFF)

#
# Extra compiler / linker options
#
IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Werror -Wformat=2")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

#
# Packages to use
#
find_package(LibGlib REQUIRED)
find_package(LibGio REQUIRED)
find_package(LibSubTtxRendCommon REQUIRED)
find_package(LibXkbCommon REQUIRED)
find_package(LibFontConfig REQUIRED)
find_package(Freetype REQUIRED)
find_package(HarfBuzz REQUIRED)
find_package(LibPng REQUIRED)
if(NOT CMAKE_SYSTEM_NAME STREQUAL Darwin)
find_package(LibWaylandClient REQUIRED)
if(WITH_OPENGL)
find_package(LibWaylandEgl REQUIRED)
find_package(LibEgl REQUIRED)
find_package(LibGlesV2 REQUIRED)
endif(WITH_OPENGL)
endif()
if(WITH_WESTEROS)
find_package(LibWesterosSimpleShellClient REQUIRED)
endif(WITH_WESTEROS)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/quartzcpp/include)
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ftcpp/include)
include_directories(${LIBGLIB_INCLUDE_DIRS})
include_directories(${LIBGIO_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDCOMMON_INCLUDE_DIRS})
include_directories(${LIBWAYLANDCLIENT_INCLUDE_DIRS})
include_directories(${LIBXKBCOMMON_INCLUDE_DIRS})
include_directories(${LIBFONTCONFIG_INCLUDE_DIRS})
include_directories(${FREETYPE_INCLUDE_DIRS})
include_directories(${LIBHARFBUZZ_INCLUDE_DIRS})
include_directories(${LIBPNG_INCLUDE_DIRS})
if(NOT CMAKE_SYSTEM_NAME STREQUAL Darwin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/waylandcpp/include)
if(WITH_OPENGL)
include_directories(${LIBWAYLANDEGL_INCLUDE_DIRS})
include_directories(${LIBEGL_INCLUDE_DIRS})
include_directories(${LIBGLESV2_INCLUDE_DIRS})
endif(WITH_OPENGL)
endif()
if(WITH_WESTEROS)
include_directories(${LIBWESTEROSSIMPLESHELLCLIENT_INCLUDE_DIRS})
endif(WITH_WESTEROS)

#
# Documentation
#
set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
set(DOXYGEN_INPUT_RECURSIVE "YES")
set(DOXYGEN_INPUT_FOLDERS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/waylandcpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ftcpp
)
foreach(arg ${DOXYGEN_INPUT_FOLDERS})
    set(DOXYGEN_INPUT "${DOXYGEN_INPUT} ${arg}")
endforeach(arg ${foo})

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY
    )
    add_custom_target(
        doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)

#
# Public headers
#
set(SUBTTXREND_GFX_PUBLIC_HEADERS
    include/ColorArgb.hpp
    include/DrawContext.hpp
    include/Factory.hpp
    include/FontStrip.hpp
    include/FontStripMap.hpp
    include/Engine.hpp
    include/KeyEvent.hpp
    include/KeyEventListener.hpp
    include/Window.hpp
    include/Types.hpp
    include/PrerenderedFont.hpp
    include/Base64ToPixmap.hpp
)

#
# Sources to compile
#
set(SUBTTXREND_GFX_SOURCES
    src/BackendFactory.cpp
    src/Blitter.cpp
    src/ColorArgb.cpp
    src/EngineImpl.cpp
    src/Factory.cpp
    src/FontStripImpl.cpp
    src/WindowImpl.cpp
    src/PrerenderedFontImpl.cpp
    src/Base64ToPixmap.cpp
    src/PrerenderedFontCache.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
add_definitions(-DBACKEND_TYPE=BACKEND_TYPE_QUARTZ)
list(APPEND SUBTTXREND_GFX_SOURCES src/QuartzBackend.cpp)
else()
list(APPEND SUBTTXREND_GFX_SOURCES src/WaylandBackend.cpp)
list(APPEND SUBTTXREND_GFX_SOURCES src/WaylandBackendLoopGlib.cpp)
if(WITH_OPENGL)
list(APPEND SUBTTXREND_GFX_SOURCES src/WaylandBackendEgl.cpp)
list(APPEND SUBTTXREND_GFX_SOURCES src/GLcpp.cpp)
add_definitions(-DBACKEND_TYPE=BACKEND_TYPE_EGL)
else(WITH_OPENGL)
list(APPEND SUBTTXREND_GFX_SOURCES src/WaylandBackendShm.cpp)
list(APPEND SUBTTXREND_GFX_SOURCES src/WaylandBuffer.cpp)
list(APPEND SUBTTXREND_GFX_SOURCES src/WaylandBufferManager.cpp)
add_definitions(-DBACKEND_TYPE=BACKEND_TYPE_SHM)
endif(WITH_OPENGL)
endif()
if(WITH_WESTEROS)
add_definitions(-DWESTEROS)
endif(WITH_WESTEROS)

#
# Targets
#
set(LIBRARY_NAME "subttxrend-gfx")
add_library(${LIBRARY_NAME} SHARED ${SUBTTXREND_GFX_SOURCES})
set_property(TARGET ${LIBRARY_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${LIBRARY_NAME} PROPERTY VERSION 0.0.0)
set_property(TARGET ${LIBRARY_NAME} PROPERTY SOVERSION 0)
set_property(TARGET ${LIBRARY_NAME} PROPERTY PUBLIC_HEADER ${SUBTTXREND_GFX_PUBLIC_HEADERS})
if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    target_link_libraries(${LIBRARY_NAME} quartzcpp)
endif()
target_link_libraries(${LIBRARY_NAME} ${LIBSUBTTXRENDCOMMON_LIBRARIES})
target_link_libraries(${LIBRARY_NAME} ${LIBXKBCOMMON_LIBRARIES})
target_link_libraries(${LIBRARY_NAME} ${LIBFONTCONFIG_LIBRARIES})
target_link_libraries(${LIBRARY_NAME} ${FREETYPE_LIBRARIES})
target_link_libraries(${LIBRARY_NAME} ${LIBHARFBUZZ_LIBRARIES})
target_link_libraries(${LIBRARY_NAME} ${LIBPNG_LIBRARIES})
if(NOT CMAKE_SYSTEM_NAME STREQUAL Darwin)
target_link_libraries(${LIBRARY_NAME} waylandcpp)
target_link_libraries(${LIBRARY_NAME} ${LIBWAYLANDCLIENT_LIBRARIES})

target_compile_definitions(${LIBRARY_NAME} PRIVATE BACKEND_TYPE_EGL=2 BACKEND_TYPE_SHM=1)
if(WITH_OPENGL)
target_link_libraries(${LIBRARY_NAME} ${LIBWAYLANDEGL_LIBRARIES})
target_link_libraries(${LIBRARY_NAME} ${LIBEGL_LIBRARIES})
target_link_libraries(${LIBRARY_NAME} ${LIBGLESV2_LIBRARIES})
endif(WITH_OPENGL)
endif()
target_link_libraries(${LIBRARY_NAME} ${LIBGIO_LINK_LIBRARIES})
target_link_libraries(${LIBRARY_NAME} ${LIBGLIB_LIBRARIES})
if(WITH_WESTEROS)
target_link_libraries(${LIBRARY_NAME} ${LIBWESTEROSSIMPLESHELLCLIENT_LIBRARIES})
endif(WITH_WESTEROS)

#
# Install rules
#
install (TARGETS ${LIBRARY_NAME}
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         PUBLIC_HEADER DESTINATION include/subttxrend/gfx
)

# Use the pkgconfig
find_package(PkgConfig REQUIRED)

#
# Package config info
#
set(PACKAGE_NAME        "${PROJECT_NAME}")
set(PACKAGE_DESCRIPTION "Subtitles/Teletext Renderer - Gfx Engine")
set(PACKAGE_VERSION     "0.0.0")
set(PACKAGE_REQUIRES    "${LIBSUBTTXRENDCOMMON_PACKAGE_NAME} ${LIBWAYLANDCLIENT_PACKAGE_NAME} ${LIBXKBCOMMON_PACKAGE_NAME} ${LIBFONTCONFIG_PACKAGE_NAME} freetype ${LIBHARFBUZZ_PACKAGE_NAME} ${LIBPNG_PACKAGE_NAME} glib-2.0")
if(WITH_OPENGL)
set(PACKAGE_REQUIRES    "${PACKAGE_REQUIRES} ${LIBWAYLANDEGL_PACKAGE_NAME} ${LIBEGL_PACKAGE_NAME} ${LIBGLESV2_PACKAGE_NAME}")
endif(WITH_OPENGL)
set(PACKAGE_LIB_NAME    "${LIBRARY_NAME}")

if(PKG_CONFIG_FOUND)
    configure_file("${PACKAGE_NAME}.pc.in"
                   "${PACKAGE_NAME}.pc"
                    @ONLY)
    install(FILES       "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}.pc"
            DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")
endif(PKG_CONFIG_FOUND)
