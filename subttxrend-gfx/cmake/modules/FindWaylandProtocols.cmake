#
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
#
# Copyright 2023 Comcast Cable Communications Management, LLC
# Licensed under the Apache License, Version 2.0
#

include(PkgConfigHelper)

pkg_check_modules(WaylandProtocols REQUIRED wayland-protocols)

# Define WaylandProtocols_pkgdatadir as the base directory of the Wayland
# protocols data directory.
pkg_get_variable(WaylandProtocols_pkgdatadir wayland-protocols pkgdatadir)

message("WaylandProtocols_pkgdatadir ${WaylandProtocols_pkgdatadir}")
