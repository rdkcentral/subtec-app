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

int main(int argc, char *argv[])
{
    debug_log("Test webvtt attributes font color started");
    bool ret = false;

    WebVttChannel *channel = new WebVttChannel();
    ret = channel->InitComms();

    if (ret)
    {
        channel->SendResetAllPacket();
        channel->SendResetChannelPacket();
        channel->SendSelectionPacket(1920, 1080);
        channel->SendUnmutePacket();

        debug_log("Set all webvtt Attributes to defaults");
        resetAllWebVttAttributes(channel);

        sendCCAttributeWebvtt(COLOR_EMBEDDED, "input_files/test_fontColor_embedded.vtt", FONT_COLOR_ARR_POSITION, channel);

        sleep (6);
        sendCCAttributeWebvtt(COLOR_BLACK, "input_files/test_fontColor_black.vtt", FONT_COLOR_ARR_POSITION, channel);

        sleep (6);
        sendCCAttributeWebvtt(COLOR_WHITE, "input_files/test_fontColor_white.vtt", FONT_COLOR_ARR_POSITION, channel);

        sleep (6);
        sendCCAttributeWebvtt(COLOR_RED, "input_files/test_fontColor_red.vtt", FONT_COLOR_ARR_POSITION, channel);

        sleep (6);
        sendCCAttributeWebvtt(COLOR_GREEN, "input_files/test_fontColor_green.vtt", FONT_COLOR_ARR_POSITION, channel);

        sleep (6);
        sendCCAttributeWebvtt(COLOR_BLUE, "input_files/test_fontColor_blue.vtt", FONT_COLOR_ARR_POSITION, channel);

        sleep (6);
        sendCCAttributeWebvtt(COLOR_YELLOW, "input_files/test_fontColor_yellow.vtt", FONT_COLOR_ARR_POSITION, channel);

        sleep (6);
        sendCCAttributeWebvtt(COLOR_MAGENTA, "input_files/test_fontColor_magenta.vtt", FONT_COLOR_ARR_POSITION, channel);

        sleep (6);
        sendCCAttributeWebvtt(COLOR_CYAN, "input_files/test_fontColor_cyan.vtt", FONT_COLOR_ARR_POSITION, channel);

        sleep (6);
    }
    else
    {
        debug_log("InitComms failed !");
    }
    debug_log("Test ended");
    return 0;
}
