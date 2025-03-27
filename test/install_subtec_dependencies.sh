#!/usr/bin/env bash
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
function package_exists_lin_fn() {
    dpkg -s "$1" &> /dev/null
    return $?
}

function install_package_fn() {
    if ! package_exists_lin_fn $1 ; then
        echo "Installing $1"
        sudo apt install $1 -y
        if [ $? == 0 ] ; then
            INSTALL_STATUS_ARR+=("$1 was successfully installed.")
        else
            INSTALL_STATUS_ARR+=("The package $1 FAILED to be installed.")
        fi
    else
       echo "$1 is already installed."
       INSTALL_STATUS_ARR+=("$1 is already installed.")
    fi
}

function install_pkgs_linux_fn()
{
    sudo apt update
    install_package_fn cmake
    install_package_fn g++
    install_package_fn libglib2.0-0
    install_package_fn libglib2.0-dev
    install_package_fn libxml2-dev
    install_package_fn pkg-config
    install_package_fn freeglut3-dev
    install_package_fn libboost-all-dev
    install_package_fn libwebsocketpp-dev
    install_package_fn libjansson-dev
    install_package_fn libwayland-dev
    install_package_fn libxkbcommon-dev
    install_package_fn libfontconfig-dev
    install_package_fn libharfbuzz-dev
    install_package_fn wayland-protocols

    VER=$(grep -oP 'VERSION_ID="\K[\d.]+' /etc/os-release)

    if [ ${VER:0:2} -lt 22 ]; then
        echo "Please upgrade your Ubuntu version to at least 22:04 LTS. OS version is $VER"
        return 1
    fi
}

function install_pkgs_fn()
{
  if [[ "$OSTYPE" == "linux"* ]]; then  
      install_pkgs_linux_fn
  fi
}

if [[ -z "${MAKEFLAGS}" ]]; then
    export MAKEFLAGS=-j$(nproc)
fi

# Fail the script should any step fail. To override this behavior use "|| true" on those statements
set -eo pipefail

echo ""
echo "*** Check/Install dependency packages"
install_pkgs_fn 