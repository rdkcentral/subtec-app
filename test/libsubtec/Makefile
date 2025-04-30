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

# Compiler and flags
CXX = g++
CXXFLAGS = -fPIC -Wall -DSUBTEC_PACKET_DEBUG=1 -DUBUNTU=1

# Source and object files
SRCS = PacketSender.cpp SubtecChannel.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = libsubtec.so

# Default target
all: $(TARGET)

# Link shared library
$(TARGET): $(OBJS)
	$(CXX) -shared -o $@ $(OBJS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJS) $(TARGET)
