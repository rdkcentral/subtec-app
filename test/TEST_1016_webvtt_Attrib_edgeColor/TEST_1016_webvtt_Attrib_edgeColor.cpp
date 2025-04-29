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
#include <WebVttPacket.hpp>
#include <TextStyleAttributes.h>
#include <fstream>

#include <utils.h>

WebVttChannel *channel = new WebVttChannel();

void sendccattribute(SupportedColors color, std::string filename, AttribPosInArray attribute = EDGE_COLOR_ARR_POSITION )
{
    std::uint32_t attributesMask = 0;
    attributesType attributesValues = {0};
    int ccType = 0;            /* Value not used by WebVTT */

    channel->SendTimestampPacket(0);

    attributesMask |= (1 << attribute);
    attributesValues[attribute] = color;

    attributesMask |= (1 << EDGE_TYPE_ARR_POSITION);
    attributesValues[EDGE_TYPE_ARR_POSITION] = EDGE_TYPE_SHADOW_LEFT;

    attributesMask |= (1 << FONT_SIZE_ARR_POSITION);
    attributesValues[FONT_SIZE_ARR_POSITION] = FONT_SIZE_LARGE;

    if (attributesMask)
    {
        channel->SendCCSetAttributePacket(ccType, attributesMask, attributesValues);
    }
    read_file_send_data_packet_webvtt(filename, channel);
}

int main(int argc, char *argv[])
{
    debug_log("Test webvtt attributes edge color started");
    bool ret = false;

    ret = channel->InitComms();
    if (ret)
    {
        channel->SendResetAllPacket();
        channel->SendResetChannelPacket();
        channel->SendSelectionPacket(1920, 1080);
        channel->SendUnmutePacket();

        debug_log("Set all webvtt Attributes to defaults");
        resetAllWebVttAttributes(channel);

        sendccattribute(COLOR_EMBEDDED, "input_files/test_edgeColor_embedded.vtt");

        sleep (8);
        sendccattribute(COLOR_BLACK, "input_files/test_edgeColor_black.vtt");

        sleep (8);
        sendccattribute(COLOR_WHITE, "input_files/test_edgeColor_white.vtt");

        sleep (6);
        sendccattribute(COLOR_RED, "input_files/test_edgeColor_red.vtt");

        sleep (6);
        sendccattribute(COLOR_GREEN, "input_files/test_edgeColor_green.vtt");

        sleep (6);
        sendccattribute(COLOR_BLUE, "input_files/test_edgeColor_blue.vtt");

        sleep (6);
        sendccattribute(COLOR_YELLOW, "input_files/test_edgeColor_yellow.vtt");

        sleep (6);
        sendccattribute(COLOR_MAGENTA, "input_files/test_edgeColor_magenta.vtt");

        sleep (6);
        sendccattribute(COLOR_CYAN, "input_files/test_edgeColor_cyan.vtt");

        sleep (6);
    }
    else
    {
        debug_log("InitComms failed !");
    }
    debug_log("Test ended");
    return 0;
}
