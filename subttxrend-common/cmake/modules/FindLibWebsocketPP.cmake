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

# - Try to find websockets++ library
#
# Once done this will define
#  LIBWEBSOCKETPP_FOUND           - System has the component
#  LIBWEBSOCKETPP_INCLUDE_DIRS    - Component include directories

find_path(LIBWEBSOCKETPP_INCLUDE_DIR
          NAMES websocketpp/server.hpp)

message(STATUS "LIBWEBSOCKETPP_INCLUDE_DIR           = ${LIBWEBSOCKETPP_INCLUDE_DIR}")

# handle the QUIETLY and REQUIRED arguments and set component to TRUE
# if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibWebsocketPP DEFAULT_MSG
                                  LIBWEBSOCKETPP_INCLUDE_DIR)

mark_as_advanced(LIBWEBSOCKETPP_INCLUDE_DIR)

set(LIBWEBSOCKETPP_INCLUDE_DIRS ${LIBWEBSOCKETPP_INCLUDE_DIR})
