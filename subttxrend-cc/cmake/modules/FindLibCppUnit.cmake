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

# - Try to find CppUnit
#
# Once done this will define
#  LIBCPPUNIT_FOUND           - System has the component
#  LIBCPPUNIT_INCLUDE_DIRS    - Component include directories
#  LIBCPPUNIT_LIBRARIES       - Libraries needed to use the component

# Use the pkgconfig
find_package(PkgConfig REQUIRED)

# Find the component information
pkg_check_modules(PC_LIBCPPUNIT QUIET cppunit)

message(STATUS "PC_LIBCPPUNIT_FOUND         = ${PC_LIBCPPUNIT_FOUND}")
message(STATUS "PC_LIBCPPUNIT_LIBRARIES     = ${PC_LIBCPPUNIT_LIBRARIES}")
message(STATUS "PC_LIBCPPUNIT_LIBRARY_DIRS  = ${PC_LIBCPPUNIT_LIBRARY_DIRS}")
message(STATUS "PC_LIBCPPUNIT_LDFLAGS       = ${PC_LIBCPPUNIT_LDFLAGS}")
message(STATUS "PC_LIBCPPUNIT_LDFLAGS_OTHER = ${PC_LIBCPPUNIT_LDFLAGS_OTHER}")
message(STATUS "PC_LIBCPPUNIT_INCLUDE_DIRS  = ${PC_LIBCPPUNIT_INCLUDE_DIRS}")
message(STATUS "PC_LIBCPPUNIT_CFLAGS        = ${PC_LIBCPPUNIT_CFLAGS}")
message(STATUS "PC_LIBCPPUNIT_CFLAGS_OTHER  = ${PC_LIBCPPUNIT_CFLAGS_OTHER}")
message(STATUS "PC_LIBCPPUNIT_VERSION       = ${PC_LIBCPPUNIT_VERSION}")

find_path(LIBCPPUNIT_INCLUDE_DIR
          NAMES cppunit/TestCase.h
          HINTS ${PC_LIBCPPUNIT_INCLUDEDIR} ${PC_LIBCPPUNIT_INCLUDE_DIRS}
          PATH_SUFFIXES cppunit)

find_library(LIBCPPUNIT_LIBRARY
             NAMES cppunit
             HINTS ${PC_LIBCPPUNIT_LIBDIR} ${PC_LIBCPPUNIT_LIBRARY_DIRS})

message(STATUS "LIBCPPUNIT_INCLUDE_DIR = ${LIBCPPUNIT_INCLUDE_DIR}")
message(STATUS "LIBCPPUNIT_LIBRARY     = ${LIBCPPUNIT_LIBRARY}")

# handle the QUIETLY and REQUIRED arguments and set component to TRUE
# if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibCppUnit DEFAULT_MSG
                                  LIBCPPUNIT_LIBRARY LIBCPPUNIT_INCLUDE_DIR)

mark_as_advanced(LIBCPPUNIT_INCLUDE_DIR LIBCPPUNIT_LIBRARY)

set(LIBCPPUNIT_INCLUDE_DIRS ${LIBCPPUNIT_INCLUDE_DIR})
set(LIBCPPUNIT_LIBRARIES ${LIBCPPUNIT_LIBRARY})
