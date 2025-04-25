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
    debug_log("Test CC 708 attribute font size and style started");
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
            if (counter == 225) {
                break;
            }
            if (counter == 0) {
                debug_log ("-------------------------");
                debug_log ("Font underline : True");
                debug_log ("Font Italic : True");
                debug_log ("Font size : Small");
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_UNDERLINE, GSW_CC_TEXT_STYLE_TRUE, channel);
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_ITALIC, GSW_CC_TEXT_STYLE_TRUE, channel);

                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_SIZE, GSW_CC_FONT_SIZE_SMALL, channel);
            }
            if (counter == 12) {
                debug_log ("-------------------------");
                debug_log ("Font underline : False");
                debug_log ("Font Italic : False");
                debug_log ("Font size : Standard");
                debug_log ("Font Style : MONOSPACED_SERIF");
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_UNDERLINE, GSW_CC_TEXT_STYLE_FALSE, channel);
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_ITALIC, GSW_CC_TEXT_STYLE_FALSE, channel);
                
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_SIZE, GSW_CC_FONT_SIZE_STANDARD, channel);
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_STYLE, 1, channel); // GSW_CC_FONT_STYLE_MONOSPACED_SERIF
            }
            if (counter == 40) {
                debug_log ("-------------------------");
                debug_log ("Font size : Large");
                debug_log ("Font Style : PROPORTIONAL_SERIF");
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_SIZE, GSW_CC_FONT_SIZE_LARGE, channel);
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_STYLE, 2, channel); // GSW_CC_FONT_STYLE_PROPORTIONAL_SERIF
            }
            if (counter == 112) {
                debug_log ("-------------------------");
                debug_log ("Font size : Extra Large");
                debug_log ("Font Style : MONOSPACED_SANSSERIF");
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_SIZE, GSW_CC_FONT_SIZE_EXTRALARGE, channel);
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_STYLE, 3, channel); // GSW_CC_FONT_STYLE_MONOSPACED_SANSSERIF
            }
            if (counter == 136) {
                debug_log ("-------------------------");
                debug_log ("Font Style : PROPORTIONAL_SANSSERIF");
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_STYLE, 4, channel); // GSW_CC_FONT_STYLE_PROPORTIONAL_SANSSERIF
            }
            if (counter == 160) {
                debug_log ("-------------------------");
                debug_log ("Font Style : CASUAL");
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_STYLE, 5, channel); // GSW_CC_FONT_STYLE_CASUAL
            }
            if (counter == 176) {
                debug_log ("-------------------------");
                debug_log ("Font Style : CURSIVE");
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_STYLE, 6, channel); // GSW_CC_FONT_STYLE_CURSIVE
            }
            if (counter == 190) {
                debug_log ("-------------------------");
                debug_log ("Font Style : SMALL_CAPITALS");
                sendCCAttributePacket(GSW_CC_ATTRIB_FONT_STYLE, 7, channel); // GSW_CC_FONT_STYLE_SMALL_CAPITALS
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
