#############################################################################
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
##############################################################################

find_library(LIBWESTEROSSIMPLESHELLCLIENT_LIBRARY NAMES westeros_simpleshell_client REQUIRED)

find_path(LIBWESTEROSSIMPLESHELLCLIENT_INCLUDE_DIR NAMES westeros-simpleshell.h REQUIRED)

find_path(LIBWESTEROSSIMPLESHELLCLIENT_PROTOCOL_INCLUDE_DIR NAMES simpleshell-client-protocol.h)

message(STATUS "LIBWESTEROSSIMPLESHELLCLIENT_LIBRARY              = ${LIBWESTEROSSIMPLESHELLCLIENT_LIBRARY}")
message(STATUS "LIBWESTEROSSIMPLESHELLCLIENT_PROTOCOL_INCLUDE_DIR = ${LIBWESTEROSSIMPLESHELLCLIENT_PROTOCOL_INCLUDE_DIR}")

set(LIBWESTEROSSIMPLESHELLCLIENT_INCLUDE_DIRS ${LIBWESTEROSSIMPLESHELLCLIENT_PROTOCOL_INCLUDE_DIR})
set(LIBWESTEROSSIMPLESHELLCLIENT_LIBRARIES ${LIBWESTEROSSIMPLESHELLCLIENT_LIBRARY})
