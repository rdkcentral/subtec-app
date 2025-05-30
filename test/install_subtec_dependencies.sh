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
required_packages_ubuntu=(
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

# Function to check if a package is installed
function is_package_installed_ubuntu() {
    dpkg -s "$1" >/dev/null 2>&1
}

function install_pkgs_ubuntu() {
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

    echo "Installing required packages..."
    sudo apt update

    for package in "${required_packages_ubuntu[@]}"; do
        if ! is_package_installed_ubuntu "$package"; then
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
}

function install_pkgs_darwin() 
{
    # Check if brew package $1 is installed
    # http://stackoverflow.com/a/20802425/1573477
    for PKG in "$@";
    do
        if brew ls --versions $PKG > /dev/null; then
            echo "${PKG} is already installed."
            INSTALL_STATUS_ARR+=("${PKG} is already installed.")
        else
            echo "Installing ${PKG}"
            brew install $PKG
            #update summary
            if brew ls --versions $PKG > /dev/null; then
                #The package is successfully installed
                INSTALL_STATUS_ARR+=("The package was ${PKG} was successfully installed.")

            else
                #The package is failed to be installed
                INSTALL_STATUS_ARR+=("The package ${PKG} FAILED to be installed.")
            fi
        fi

        PKGDIR="`brew --prefix ${PKG}`/lib/pkgconfig:"
        INSTALLED_PKGCONFIG=$PKGDIR$INSTALLED_PKGCONFIG

    done
    echo "${INSTALLED_PKGCONFIG}"
}

function install_pkgs() {
  if [[ "$OSTYPE" == "darwin"* ]]; then

      #Check/Install brew
      which -s brew
      if [[ $? != 0 ]] ; then
          echo "Installing homebrew"
          /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
          # TODO: Need to add to path or have user do it and fail this script
      else
          echo "Updating homebrew"
          brew update
      fi

      install_pkgs_darwin cmake pkg-config websocketpp janssons libxkbcommon fontconfig doxygen json-glib libxml2 "boost@1.85"

      # ORC causes compile errors on x86_64 Mac, but not on ARM64
      if [[ $ARCH == "x86_64" ]]; then
          
          # Workaround for making boost compatible with websocketpp (used for subtec)

          export BOOST_ROOT="/usr/local/opt/boost@1.85"
          export CMAKE_PREFIX_PATH="/usr/local/opt/boost@1.85"
          export LDFLAGS="-L/usr/local/opt/boost@1.85/lib -L/usr/local/lib -lwavpack"
          export CPPFLAGS="-I/usr/local/opt/boost@1.85/include"

          echo "Checking/removing ORC package which cause compile errors with gst-plugins-good"

          # "|| true" prevents the script from exiting if orc is not found, that is not an error
          ORC_FOUND=`brew list | grep -i orc | wc -l` || true
          if [ "${ORC_FOUND}" -gt 0 ]; then
              read -p "Found ORC, remove ORC package (Y/N)" remove_orc
              case $remove_orc in
                [Yy]* ) brew remove -f --ignore-dependencies orc
                    ;;
                * ) echo "Exiting without removal ..."
                    return 1
                    ;;
              esac
          fi
      elif [[ $ARCH == "arm64" ]]; then
          
          # Workaround for making boost compatible with websocketpp (used for subtec)
          export BOOST_ROOT="/opt/homebrew/opt/boost@1.85"
          export CMAKE_PREFIX_PATH="/opt/homebrew/opt/boost@1.85"
          export LDFLAGS="-L/opt/homebrew/opt/boost@1.85/lib -L/opt/homebrew/lib -lwavpack"
          export CPPFLAGS="-I/opt/homebrew/opt/boost@1.85/include"

          install_pkgs_darwin orc
      fi
  elif [[ "$OSTYPE" == "linux"* ]]; then  
      install_pkgs_ubuntu
  fi
}

install_pkgs

echo "Dependency setup complete"
exit 0

