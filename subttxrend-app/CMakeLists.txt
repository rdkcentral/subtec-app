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

project(subttxrend-app)

cmake_minimum_required (VERSION 3.2)

#
# Directory with modules
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

#
# Configuration variables
#
option(INSTALL_CONFIG_FILE "Install the configuration file" ON)

#
# Extra compiler / linker options
#
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed" )
IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Werror -Wformat=2")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

#
# Packages to use
#
find_package(LibGlib REQUIRED)
find_package(LibSubTtxRendCommon REQUIRED)
find_package(LibSubTtxRendProtocol REQUIRED)
find_package(LibSubTtxRendSockSrc REQUIRED)
find_package(LibSubTtxRendDbus REQUIRED)
find_package(LibSubTtxRendGfx REQUIRED)
find_package(LibSubTtxRendDvbSub REQUIRED)
find_package(LibSubTtxRendScte REQUIRED)
find_package(LibSubTtxRendCc REQUIRED)
find_package(LibSubTtxRendTtxt REQUIRED)
find_package(LibSubTtxRendTtml REQUIRED)
find_package(LibSubTtxRendWebvtt REQUIRED)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${LIBGLIB_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDCOMMON_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDPROTOCOL_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDSOCKSRC_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDDBUS_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDGFX_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDDVBSUB_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDSCTE_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDCC_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDTTXT_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDTTML_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDWEBVTT_INCLUDE_DIRS})

#
# Definitions (flags etc.)
#
add_definitions(${LIBDVBTTXSUBUTILS_DEFINITIONS})

#
# Documentation
#
set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
set(DOXYGEN_INPUT_RECURSIVE "YES")
set(DOXYGEN_INPUT_FOLDERS
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
# Sources to compile
#
set(SUBTTXREND_APP_SOURCES
    src/main.cpp
    src/Configuration.cpp
    src/DvbSubController.cpp
    src/Options.cpp
    src/PesFinder.cpp
    src/TtxController.cpp
    src/TtmlController.cpp
    src/WebvttController.cpp
    src/StcProvider.cpp
    src/ScteSubController.cpp
    src/CcSubController.cpp
    src/Controller.cpp
    src/Application.cpp
)

#
# Targets
#
set(APP_NAME "subttxrend-app")
add_executable(${APP_NAME} ${SUBTTXREND_APP_SOURCES})
set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 14)
target_link_libraries(${APP_NAME} ${LIBGLIB_LIBRARIES})
target_link_libraries(${APP_NAME} ${LIBSUBTTXRENDCOMMON_LIBRARIES})
target_link_libraries(${APP_NAME} ${LIBSUBTTXRENDPROTOCOL_LIBRARIES})
target_link_libraries(${APP_NAME} ${LIBSUBTTXRENDSOCKSRC_LIBRARIES})
target_link_libraries(${APP_NAME} ${LIBSUBTTXRENDDBUS_LIBRARIES})
target_link_libraries(${APP_NAME} ${LIBSUBTTXRENDGFX_LIBRARIES})
target_link_libraries(${APP_NAME} ${LIBSUBTTXRENDDVBSUB_LIBRARIES})
target_link_libraries(${APP_NAME} ${LIBSUBTTXRENDSCTE_LIBRARIES})
target_link_libraries(${APP_NAME} ${LIBSUBTTXRENDCC_LIBRARIES})
target_link_libraries(${APP_NAME} ${LIBSUBTTXRENDTTXT_LIBRARIES})
target_link_libraries(${APP_NAME} ${LIBSUBTTXRENDTTML_LIBRARIES})
target_link_libraries(${APP_NAME} ${LIBSUBTTXRENDWEBVTT_LIBRARIES})
target_link_libraries(${APP_NAME} pthread)
target_link_libraries(${APP_NAME} boost_system)

#
# Install rules
#
install (TARGETS ${APP_NAME}
         RUNTIME DESTINATION bin)

if(INSTALL_CONFIG_FILE)
install(FILES conf/config.ini
        DESTINATION /etc/subttxrend
        COMPONENT config)
endif()
