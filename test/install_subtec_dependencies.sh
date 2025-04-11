#!/usr/bin/env bash
#
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
#

# Set -eo pipefail to exit immediately if a pipeline fails
set -eo pipefail

# Set MAKEFLAGS to use the number of processors
if [[ -z "${MAKEFLAGS}" ]]; then
    NPROC=$(nproc)
    export MAKEFLAGS="-j${NPROC}"
fi

echo "Check/Install dependency packages"

# Define required packages
required_packages=(
    cmake
    g++
    libglib2.0-0
    libglib2.0-dev
    libxml2-dev
    pkg-config
    freeglut3-dev
    libboost-all-dev
    libwebsocketpp-dev
    libjansson-dev
    libwayland-dev
    libxkbcommon-dev
    libfontconfig-dev
    libharfbuzz-dev
    wayland-protocols
)

echo "Checking Ubuntu version..."
# Check Ubuntu version
if [ -f "/etc/os-release" ]; then
    UBUNTU_VERSION=$(grep '^VERSION_ID=' /etc/os-release | sed -E 's/VERSION_ID="?([^"]+)"?/\1/')
else
    echo "Error: Could not determine OS version."
fi

EXPECTED_VERSION="22.04"

if [[ "$UBUNTU_VERSION" != "$EXPECTED_VERSION" ]]; then
    echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    echo "Warning: This script has been designed for Ubuntu ${EXPECTED_VERSION}."
    echo "Your Ubuntu version is: ${UBUNTU_VERSION}. Please check !"
    echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
fi

# Function to check if a package is installed
function is_package_installed() {
    dpkg -s "$1" >/dev/null 2>&1
}

echo "Installing required packages..."
sudo apt update

for package in "${required_packages[@]}"; do
    if ! is_package_installed "$package"; then
        echo "Installing: $package"
        sudo apt install -y "$package"
        if [ $? -ne 0 ]; then
            echo "Error: Failed to install package '$package'."
            exit 1
        fi
    else
        echo "$package is already installed."
    fi
done

echo "Dependency setup complete"
exit 0

