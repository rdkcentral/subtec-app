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

void sendccattribute(Opacity value, std::string filename, AttribPosInArray attribute = FONT_OPACITY_ARR_POSITION )
{
    std::uint32_t attributesMask = 0;
    attributesType attributesValues = {0};
    int ccType = 0;            /* Value not used by WebVTT */

    channel->SendTimestampPacket(0);

    attributesMask |= (1 << attribute);
    attributesValues[attribute] = value;

    attributesMask |= (1 << FONT_COLOR_ARR_POSITION);
    attributesValues[FONT_COLOR_ARR_POSITION] = COLOR_RED;

    attributesMask |= (1 << FONT_SIZE_ARR_POSITION);
    attributesValues[FONT_SIZE_ARR_POSITION] = FONT_SIZE_EXTRALARGE;

    if (attributesMask)
    {
        channel->SendCCSetAttributePacket(ccType, attributesMask, attributesValues);
    }
    read_file_send_data_packet_webvtt(filename, channel);
}

int main(int argc, char *argv[])
{
    debug_log("Test webvtt attributes font opacity started");
    bool ret = false;

    ret = channel->InitComms();
    if (ret)
    {
        channel->SendResetAllPacket();
        channel->SendResetChannelPacket();
        channel->SendSelectionPacket(1920, 1080);
        channel->SendUnmutePacket();

        /* TODO : Resetting attributes here causes test case failure
        as font opacity seems to be dependent on background opacity
        when running on ubuntu, so setting background opacity to 
        default of OPACITY_EMBEDDED causes font to be opaque too.
        */

        sendccattribute(OPACITY_EMBEDDED, "input_files/test_fontOpacity_embedded.vtt");

        sleep (8);
        sendccattribute(OPACITY_SOLID, "input_files/test_fontOpacity_solid.vtt");

        sleep (8);
        sendccattribute(OPACITY_FLASHING, "input_files/test_fontOpacity_flashing.vtt");

        sleep (6);
        sendccattribute(OPACITY_TRANSLUCENT, "input_files/test_fontOpacity_translucent.vtt");

        sleep (6);
        sendccattribute(OPACITY_TRANSPARENT, "input_files/test_fontOpacity_transparent.vtt");

        sleep (6);
    }
    else
    {
        debug_log("InitComms failed !");
    }
    debug_log("Test ended");
    return 0;
}
