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

#include <utils.h>

int main(int argc, char *argv[])
{
    debug_log("Test webvtt attributes font style started");
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

        sendCCAttributeWebvtt(FONT_STYLE_EMBEDDED, "input_files/test_fontStyle_embedded.vtt", FONT_STYLE_ARR_POSITION, channel);

        sleep (8);
        sendCCAttributeWebvtt(FONT_STYLE_DEFAULT, "input_files/test_fontStyle_default.vtt", FONT_STYLE_ARR_POSITION, channel);

        sleep (8);
        sendCCAttributeWebvtt(FONT_STYLE_MONOSPACED_SERIF, "input_files/test_fontStyle_mono_serif.vtt", FONT_STYLE_ARR_POSITION, channel);

        sleep (6);
        sendCCAttributeWebvtt(FONT_STYLE_PROPORTIONAL_SERIF, "input_files/test_fontStyle_prop_serif.vtt", FONT_STYLE_ARR_POSITION, channel);

        sleep (6);
        sendCCAttributeWebvtt(FONT_STYLE_MONOSPACED_SANSSERIF, "input_files/test_fontStyle_mono_sanserif.vtt", FONT_STYLE_ARR_POSITION, channel);

        sleep (6);
        sendCCAttributeWebvtt(FONT_STYLE_PROPORTIONAL_SANSSERIF, "input_files/test_fontStyle_prop_sanserif.vtt", FONT_STYLE_ARR_POSITION, channel);

        sleep (6);
        sendCCAttributeWebvtt(FONT_STYLE_CASUAL, "input_files/test_fontStyle_casual.vtt", FONT_STYLE_ARR_POSITION, channel);

        sleep (6);
        sendCCAttributeWebvtt(FONT_STYLE_CURSIVE, "input_files/test_fontStyle_cursive.vtt", FONT_STYLE_ARR_POSITION, channel);

        sleep (6);
        sendCCAttributeWebvtt(FONT_STYLE_SMALL_CAPITALS, "input_files/test_fontStyle_small_caps.vtt", FONT_STYLE_ARR_POSITION, channel);

        sleep (6);
    }
    else
    {
        debug_log("InitComms failed !");
    }
    debug_log("Test ended");
    return 0;
}
