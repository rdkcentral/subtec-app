/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm> // For std::find_if and std::reverse_iterator
#include <fstream>

#include <ClosedCaptionsPacket.hpp>
#include <SubtecConnector.h>

#include <WebVttPacket.hpp>
#include <TextStyleAttributes.h>

#include <TtmlPacket.hpp>

#define debug_log(message) \
    do { \
    std::string infoStr = "[" + getFilename(__FILE__) + ":" + std::to_string(__LINE__) + "] "; \
    std::cout << std::left << std::setw(50) << infoStr << " " << message << std::endl; \
    } while (0);

std::string getFilename(const std::string& filePath);

void sendCCAttributePacket (short type, int value, ClosedCaptionsChannel *channel);
void resetAllCCAttributes(ClosedCaptionsChannel *channel);

void sendCCAttributeWebvtt(int value, std::string filename, AttribPosInArray attribute, WebVttChannel *channel );
void sendWebVttAttributePacket (int value, std::string filename, AttribPosInArray attribute, WebVttChannel *channel);
void resetAllWebVttAttributes(WebVttChannel *channel);
void read_file_send_data_packet_webvtt(std::string filename, WebVttChannel *channel);

void read_file_send_data_packet_ttml(std::string filename, TtmlChannel *channel);
