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

project(subttxrend-dbus)

cmake_minimum_required(VERSION 3.2)

#
# Directory with modules
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

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
find_package(LibGioUnix)
find_package(LibGObject REQUIRED)
find_package(LibJansson REQUIRED)
find_package(LibSubTtxRendCommon REQUIRED)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${LIBGLIB_INCLUDE_DIRS})
include_directories(${LIBGIO_INCLUDE_DIRS})
include_directories(${LIBJANSSON_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDCOMMON_INCLUDE_DIRS})
if(${LIBGIOUNIX_FOUND})
include_directories(${LIBGIOUNIX_INCLUDE_DIRS})
endif(${LIBGIOUNIX_FOUND})


add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/SubtitleDbusInterface.h ${CMAKE_CURRENT_BINARY_DIR}/SubtitleDbusInterface.c
                    COMMAND gdbus-codegen --interface-prefix com.libertyglobal.rdk --generate-c-code SubtitleDbusInterface ${CMAKE_CURRENT_SOURCE_DIR}/api/dbus/SubtitleDbusInterface.xml
                    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/api/dbus/SubtitleDbusInterface.xml)

add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/TeletextDbusInterface.h ${CMAKE_CURRENT_BINARY_DIR}/TeletextDbusInterface.c
                    COMMAND gdbus-codegen --interface-prefix com.libertyglobal.rdk --generate-c-code TeletextDbusInterface ${CMAKE_CURRENT_SOURCE_DIR}/api/dbus/TeletextDbusInterface.xml
                    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/api/dbus/TeletextDbusInterface.xml)

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
set(SUBTTXREND_DBUS_PUBLIC_HEADERS
    include/DbusServer.hpp
    include/Executor.hpp
    include/SubtitleStatus.hpp
    include/TeletextStatus.hpp
    )

#
# Sources to compile
#
set(SUBTTXREND_DBUS_SOURCES
    src/DbusServer.cpp
    src/DbusServerImpl.cpp
    src/DbusHandler.cpp
    src/DbusHandlerSubtitles.cpp
    src/DbusHandlerTeletext.cpp
    src/JsonHelper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/SubtitleDbusInterface.c
    ${CMAKE_CURRENT_BINARY_DIR}/TeletextDbusInterface.c
)

#
# Targets
#
set(LIBRARY_NAME "subttxrend-dbus")
add_library(${LIBRARY_NAME} SHARED ${SUBTTXREND_DBUS_SOURCES})
set_property(TARGET ${LIBRARY_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${LIBRARY_NAME} PROPERTY VERSION 0.0.0)
set_property(TARGET ${LIBRARY_NAME} PROPERTY SOVERSION 0)
set_property(TARGET ${LIBRARY_NAME} PROPERTY PUBLIC_HEADER ${SUBTTXREND_DBUS_PUBLIC_HEADERS})
target_link_libraries(${LIBRARY_NAME} ${LIBSUBTTXRENDCOMMON_LIBRARIES})
target_link_libraries(${LIBRARY_NAME} ${LIBJANSSON_LIBRARIES})
if(${LIBGIOUNIX_FOUND})
target_link_libraries(${LIBRARY_NAME} ${LIBGIOUNIX_LIBRARIES})
endif(${LIBGIOUNIX_FOUND})
target_link_libraries(${LIBRARY_NAME} ${LIBGOBJECT_LIBRARIES})
target_link_libraries(${LIBRARY_NAME} ${LIBGIO_LIBRARIES})
target_link_libraries(${LIBRARY_NAME} ${LIBGLIB_LIBRARIES})

#
# Install rules
#
install (TARGETS ${LIBRARY_NAME}
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         PUBLIC_HEADER DESTINATION include/subttxrend/dbus
)

# Use the pkgconfig
find_package(PkgConfig REQUIRED)

#
# Package config info
#
set(PACKAGE_NAME        "${PROJECT_NAME}")
set(PACKAGE_DESCRIPTION "Subtitles/Teletext Renderer - DBus API")
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
