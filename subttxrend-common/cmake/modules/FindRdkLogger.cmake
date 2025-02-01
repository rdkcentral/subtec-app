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

# - Try to find RDK Logger
#
# Once done this will define
#  LIBRDKLOGGER_FOUND           - System has the component
#  LIBRDKLOGGER_INCLUDE_DIRS    - Component include directories
#  LIBRDKLOGGER_LIBRARIES       - Libraries needed to use the component

find_path(LIBRDKLOGGER_INCLUDE_DIR
          NAMES rdk_debug.h
          )

find_library(LIBRDKLOGGER_LIBRARY
             NAMES rdkloggers
             )

message(STATUS "LIBRDKLOGGER_INCLUDE_DIR = ${LIBRDKLOGGER_INCLUDE_DIR}")
message(STATUS "LIBRDKLOGGER_LIBRARY     = ${LIBRDKLOGGER_LIBRARY}")

# handle the QUIETLY and REQUIRED arguments and set component to TRUE
# if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibRdkLogger DEFAULT_MSG
                                  LIBRDKLOGGER_LIBRARY
                                  LIBRDKLOGGER_INCLUDE_DIR)

mark_as_advanced(LIBRDKLOGGER_INCLUDE_DIR LIBRDKLOGGER_LIBRARY)

set(LIBRDKLOGGER_INCLUDE_DIRS ${LIBRDKLOGGER_INCLUDE_DIR})
set(LIBRDKLOGGER_LIBRARIES ${LIBRDKLOGGER_LIBRARY})

set(LIBRDKLOGGER_PKG_EXTRA_LIBS "-lrdkloggers")
