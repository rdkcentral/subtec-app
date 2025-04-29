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
#include <PacketSender.hpp>
#include <ClosedCaptionsPacket.hpp>
#include <SubtecConnector.h>
#include <fstream>

#include <utils.h>
#include <sstream>

int main(int argc, char *argv[])
{
    debug_log("Test CC 708 attribute opacity started");
    bool ret = false;

    ClosedCaptionsChannel *channel = new ClosedCaptionsChannel();

    ret = channel->InitComms();
    if (ret)
    {
        channel->SendResetAllPacket();
        channel->SendResetChannelPacket();
        channel->SendActiveTypePacket(ClosedCaptionsActiveTypePacket::CEA::type_708, 1);
        channel->SendUnmutePacket();

        std::ifstream inFile("input_files/capture708.hex", std::ios::in);
        if (inFile.fail())
        {
            debug_log("Input file can not be opened/does not exist. Aborting !");
            abort();
        }

        // Read the entire file into a string
        std::stringstream buffer;
        buffer << inFile.rdbuf();
        std::string hexData = buffer.str();

        // Remove any non-hexadecimal characters (e.g., spaces, newlines)
        hexData.erase(std::remove_if(hexData.begin(), hexData.end(),
            [](unsigned char c) { return !std::isxdigit(c); }), hexData.end());

        inFile.close();

        debug_log("Set all CC Attributes to defaults");
        resetAllCCAttributes(channel);

        int counter = 0;
        size_t offset = 0;
            // Extract dataLength and localPts
        unsigned dataLength;
        long long localPts;
        while (offset + 24 <= hexData.length()) {
            if (counter == 150) {
                break;
            }
            if (counter == 0) {
                debug_log ("-------------------------");
                debug_log ("Window color : Blue");
                debug_log ("Font Color : Red");
                debug_log ("Font size : Extra Large");
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_SIZE, GSW_CC_FONT_SIZE_EXTRALARGE, channel);
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_COLOR, (int)0x00FF0000, channel); // CCColor_RED
                sendCCAttributePacket(GSW_CC_ATTRIB_WIN_COLOR, (int)0x000000FF, channel); // CCColor_BLUE

                debug_log ("Font opacity : Transparent");
                debug_log ("Background opacity : Transparent");
                debug_log ("Window opacity : Solid");
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_OPACITY, GSW_CC_OPACITY_TRANSPARENT, channel);
                sendCCAttributePacket(GSW_CC_ATTRIB_BACKGROUND_OPACITY, GSW_CC_OPACITY_TRANSPARENT, channel);
                sendCCAttributePacket(GSW_CC_ATTRIB_WIN_OPACITY, GSW_CC_OPACITY_SOLID, channel);
            }
            if (counter == 40) {
                debug_log ("-------------------------");
                debug_log ("Window color : Red");
                sendCCAttributePacket(GSW_CC_ATTRIB_WIN_COLOR, (int)0x00FF0000, channel); // CCColor_RED

                debug_log ("Font opacity : Translucent");
                debug_log ("Background opacity : Translucent");
                debug_log ("Window opacity : Translucent");
                // Font, background and window opacity
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_OPACITY, GSW_CC_OPACITY_TRANSLUCENT, channel);
                sendCCAttributePacket(GSW_CC_ATTRIB_BACKGROUND_OPACITY, GSW_CC_OPACITY_TRANSLUCENT, channel);
                sendCCAttributePacket(GSW_CC_ATTRIB_WIN_OPACITY, GSW_CC_OPACITY_TRANSLUCENT, channel);
            }
            if (counter == 112) {
                debug_log ("-------------------------");
                debug_log ("Window color : Green");
                sendCCAttributePacket(GSW_CC_ATTRIB_WIN_COLOR, (int)0x0000FF00, channel); // CCColor_GREEN

                debug_log ("Font opacity : Solid");
                debug_log ("Background opacity : Solid");
                debug_log ("Window opacity : Transparent");
                // Font, background and window opacity
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_OPACITY, GSW_CC_OPACITY_SOLID, channel);
                sendCCAttributePacket(GSW_CC_ATTRIB_BACKGROUND_OPACITY, GSW_CC_OPACITY_SOLID, channel);
                sendCCAttributePacket(GSW_CC_ATTRIB_WIN_OPACITY, GSW_CC_OPACITY_TRANSPARENT, channel);
            }

            dataLength = std::stoul(hexData.substr(offset, 8), nullptr, 16);
            localPts = std::stoll(hexData.substr(offset + 8, 16), nullptr, 16);

            // Extract ccData
            std::string ccDataHex = hexData.substr(offset + 24, dataLength * 2);

            // Ensure the length of ccDataHex is even
            if (ccDataHex.length() % 2 != 0) {
                std::cerr << "Hex data for ccData has an odd length\n";
                return 1;
            }

            // Convert hex string to bytes
            std::vector<uint8_t> ccData;
            for (size_t i = 0; i < ccDataHex.length(); i += 2) {
                uint8_t byte = std::stoi(ccDataHex.substr(i, 2), nullptr, 16);
                ccData.push_back(byte);
            }

            // Pass the data to the function
            channel->SendDataPacketWithPTS(static_cast<uint32_t>(localPts), ccData.data(), dataLength);

            // Update offset for the next entry
            offset += 24 + dataLength * 2;
            counter++;
            usleep(250000); // microseconds
        }
    }
    else
    {
        debug_log("InitComms failed !");
    }
    /* Sleep is added here for runWorkerTask thread in PacketSender to be created and in a 
    stable state, so that there is no crash when ~PacketSender checks joinable() 
    and tries to join the thread */
    sleep(10);
    debug_log("Test ended");
    return 0;
}
