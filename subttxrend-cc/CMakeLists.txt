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

project(subttxrend-cc)

cmake_minimum_required (VERSION 3.2)

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
find_package(LibSubTtxRendCommon REQUIRED)
find_package(LibSubTtxRendGfx REQUIRED)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${LIBSUBTTXRENDCOMMON_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDGFX_INCLUDE_DIRS})
include_directories(${LIBCOMMON_INCLUDE_DIRS})

#
# Public headers
#
set(SUBTTXREND_CC_PUBLIC_HEADERS
    include/CcController.hpp
    include/CcExceptions.hpp
    include/CcUserData.hpp
    include/CcCaptionChannelPacket.hpp
    include/CcServiceBlock.hpp
    include/CcWindowController.hpp
    include/CcGfx.hpp
    include/CcRenderer.hpp
    include/CcCommand.hpp
    include/CcWindow.hpp
    include/CcTextDrawer.hpp
    include/CcCommandParser.hpp
    include/CcUserSettingsController.hpp
    include/CcCommandProcessor.hpp
)

#
# Sources to compile
#
set(SUBTTXREND_CC_SOURCES
    src/CcController.cpp
    src/CcUserData.cpp
    src/CcCaptionChannelPacket.cpp
    src/CcServiceBlock.cpp
    src/CcWindowController.cpp
    src/CcTextGfxDrawer.cpp
    src/CcWindow.cpp
    src/CcCommandParser.cpp
    src/CcCommand608Parser.cpp
    src/CcCommand.cpp
    src/CcRenderer.cpp
)

#
# Targets
#
set(LIBRARY_NAME "subttxrend-cc")
add_library(${LIBRARY_NAME} SHARED ${SUBTTXREND_CC_SOURCES})
set_property(TARGET ${LIBRARY_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${LIBRARY_NAME} PROPERTY VERSION 0.0.0)
set_property(TARGET ${LIBRARY_NAME} PROPERTY SOVERSION 0)
set_property(TARGET ${LIBRARY_NAME} PROPERTY PUBLIC_HEADER ${SUBTTXREND_CC_PUBLIC_HEADERS})
target_link_libraries(${LIBRARY_NAME} ${LIBSUBTTXRENDCOMMON_LIBRARIES})
target_link_libraries(${LIBRARY_NAME} ${LIBSUBTTXRENDGFX_LIBRARIES})
target_link_libraries(${LIBRARY_NAME} ${LIBDVBSUBDECODER_LIBRARIES})

#
# Install rules
#
install (TARGETS ${LIBRARY_NAME}
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         PUBLIC_HEADER DESTINATION include/subttxrend/cc
)

# Use the pkgconfig
find_package(PkgConfig REQUIRED)

#
# Package config info
#
set(PACKAGE_NAME        "${PROJECT_NAME}")
set(PACKAGE_DESCRIPTION "Subtitles Renderer - Closed Captions Renderer")
set(PACKAGE_VERSION     "0.0.0")
set(PACKAGE_REQUIRES    "")
set(PACKAGE_LIB_NAME    "${LIBRARY_NAME}")

if(PKG_CONFIG_FOUND)
    configure_file("${PACKAGE_NAME}.pc.in"
                   "${PACKAGE_NAME}.pc"
                    @ONLY)
    install(FILES       "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}.pc"
            DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")
endif(PKG_CONFIG_FOUND)
