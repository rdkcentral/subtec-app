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
cmake_minimum_required(VERSION 3.2) # Or higher, if you prefer

# Project name
project(subtec)

# Compiler and flags
set(CMAKE_CXX_COMPILER "g++") # Explicitly set the C++ compiler, though CMake often detects it
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -DSUBTEC_PACKET_DEBUG=1 -DUBUNTU=1") # General flags

# Source files
set(SOURCE_FILES
    PacketSender.cpp
    SubtecChannel.cpp
)

# Create shared library
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

#Set the target name
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "subtec") # Sets library name, ensuring "lib" prefix is NOT added.
