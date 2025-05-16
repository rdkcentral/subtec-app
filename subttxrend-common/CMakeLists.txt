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

project(subttxrend-common)

cmake_minimum_required (VERSION 3.2)

#
# Directory with modules
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/" "${CMAKE_MODULE_PATH}/websocket-ipplayer2-utils")

#
# Extra compiler / linker options
#
IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Werror -Wformat=2")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

#
# Packages to use
#
find_package(RdkLogger REQUIRED)
find_package(Ipp2Utils REQUIRED CONFIG)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${LIBRDKLOGGER_INCLUDE_DIRS})
include_directories(${LIBWEBSOCKETPP_INCLUDE_DIRS})


#
# Documentation
#
set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
set(DOXYGEN_INPUT_RECURSIVE "YES")
set(DOXYGEN_INPUT_FOLDERS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
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
set(SUBTTXREND_COMMON_PUBLIC_HEADERS
    include/ConfigProvider.hpp
    include/IniFile.hpp
    include/StcProvider.hpp
    include/Logger.hpp
    include/LoggerLevel.hpp
    include/LoggerManager.hpp
    include/NonCopyable.hpp
    include/PrefixConfigProvider.hpp
    include/StringUtils.hpp
    include/DataBuffer.hpp
    include/TtmlAsClient.hpp
    )

#
# Sources to compile
#
set(SUBTTXREND_COMMON_SOURCES
    src/ConfigProvider.cpp
    src/ConfigProviderStorage.cpp
    src/IniFile.cpp
    src/Logger.cpp
    src/LoggerBackendRdk.cpp
    src/LoggerBackendStd.cpp
    src/LoggerManagerImpl.cpp
    src/StringUtils.cpp
    )
    
if(BUILD_RDK_REFERENCE)
    set(SUBTTXREND_COMMON_SOURCES ${SUBTTXREND_COMMON_SOURCES} src/clients-generic/TtmlAsClient.cpp)
else(BUILD_RDK_REFERENCE)
     set(SUBTTXREND_COMMON_SOURCES ${SUBTTXREND_COMMON_SOURCES} src/clients-lgi/TtmlAsClient.cpp)
     set(SUBTTXREND_COMMON_SOURCES ${SUBTTXREND_COMMON_SOURCES} src/clients-lgi/AsHelpers.cpp)
endif(BUILD_RDK_REFERENCE)

#
# Targets
#
set(LIBRARY_NAME "subttxrend-common")
add_library(${LIBRARY_NAME} SHARED ${SUBTTXREND_COMMON_SOURCES})
set_property(TARGET ${LIBRARY_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${LIBRARY_NAME} PROPERTY VERSION 0.0.0)
set_property(TARGET ${LIBRARY_NAME} PROPERTY SOVERSION 0)
set_property(TARGET ${LIBRARY_NAME} PROPERTY PUBLIC_HEADER ${SUBTTXREND_COMMON_PUBLIC_HEADERS})
target_link_libraries(${LIBRARY_NAME} ${LIBRDKLOGGER_LIBRARIES})
target_link_libraries(${LIBRARY_NAME} boost_system)
target_link_libraries(${LIBRARY_NAME} pthread)
target_link_libraries(${LIBRARY_NAME} Ipp2Utils::ipp2utils)

#
# Install rules
#
install (TARGETS ${LIBRARY_NAME}
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         PUBLIC_HEADER DESTINATION include/subttxrend/common
)

# Use the pkgconfig
find_package(PkgConfig REQUIRED)

#
# Package config info
#
set(PACKAGE_NAME        "${PROJECT_NAME}")
set(PACKAGE_DESCRIPTION "Subtitles/Teletext Renderer - Common")
set(PACKAGE_VERSION     "0.0.0")
set(PACKAGE_REQUIRES    "")
set(PACKAGE_LIB_NAME    "${LIBRARY_NAME}")
set(PACKAGE_EXTRA_LIBS  "${LIBRDKLOGGER_PKG_EXTRA_LIBS}")

if(PKG_CONFIG_FOUND)
    configure_file("${PACKAGE_NAME}.pc.in"
                   "${PACKAGE_NAME}.pc"
                    @ONLY)
    install(FILES       "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}.pc"
            DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")
endif(PKG_CONFIG_FOUND)
