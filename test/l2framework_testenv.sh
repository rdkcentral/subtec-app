#!/bin/bash
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
echo "Installing required packages"
# Define required packages
required_packages=(
    python3-pip
    python3-venv
)

# Function to check if a package is installed
function is_package_installed() {
    dpkg -s "$1" >/dev/null 2>&1
}

# Install packages
sudo apt-get update
for package in "${required_packages[@]}"; do
    if ! is_package_installed "$package"; then
        echo "Installing: $package"
        sudo apt-get --no-install-recommends install -y "$package"
        if [ $? -ne 0 ]; then
            echo "Error: Failed to install package '$package'."
            exit 1
        fi
    else
        echo "$package is already installed."
    fi
done

# ensure $HOME/.local/bin exists for python pip user installed packages and is in PATH
if [ "$EUID" -ne 0 ]; then
    if [ ! -d "$HOME/.local/bin" ]; then
        mkdir -p "$HOME/.local/bin"
        if [ ! -d "$HOME/.local/bin" ]; then
            echo "Error $HOME/.local/bin dir not present."
        fi
    fi

    if [ ! $(echo "$PATH" | grep "$HOME/.local/bin") ]; then
        echo ""
        echo "Add dir to the PATH, with \"export PATH=\$HOME/.local/bin:\$PATH\""
        echo "Or logout and back in to get \$HOME/.local/bin added to PATH"
        echo "then re-run this script."
        exit 0
    fi
fi

# check if l2venv is already in use and if not create it
if [[ "$VIRTUAL_ENV" != "$(pwd)/l2venv" ]]; then
    python3 -m venv l2venv
    echo ""
    echo "Created a Python virtual environment \"l2venv\""
    echo "Activate venv with \"source l2venv/bin/activate\"."
    echo "You will see (l2venv) at beginning of prompt when activated."
    echo "venv can be deactivated with the \"deactivate\" command."
fi
