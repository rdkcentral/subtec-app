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
#include <TtmlPacket.hpp>
#include <fstream>

#include <utils.h>

int main(int argc, char *argv[])
{
    debug_log("Test ttml attribute font color started");
    bool ret = false;

    TtmlChannel *channel = new TtmlChannel();
    ret = channel->InitComms();
    if (ret)
    {
        channel->SendResetAllPacket();
        channel->SendResetChannelPacket();
        channel->SendSelectionPacket(1920, 1080);
        channel->SendUnmutePacket();

        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_transparent.ttml", channel);
        sleep (6);

        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_black.ttml", channel);
        sleep (6);

        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_silver.ttml", channel);
        sleep (6);

        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_gray.ttml", channel);
        sleep (6);

        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_white.ttml", channel);
        sleep (6);
        
        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_maroon.ttml", channel);
        sleep (6);
        
        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_red.ttml", channel);
        sleep (6);
        
        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_purple.ttml", channel);
        sleep (6);
        
        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_fuchsia.ttml", channel);
        sleep (6);
        
        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_magenta.ttml", channel);
        sleep (6);
        
        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_green.ttml", channel);
        sleep (6);
        
        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_lime.ttml", channel);
        sleep (6);
        
        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_olive.ttml", channel);
        sleep (6);
        
        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_yellow.ttml", channel);
        sleep (6);
        
        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_navy.ttml", channel);
        sleep (6);
        
        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_blue.ttml", channel);
        sleep (6);
        
        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_teal.ttml", channel);
        sleep (6);
        
        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_aqua.ttml", channel);
        sleep (6);
        
        read_file_send_data_packet_ttml("input_files/test_sample_fontColor_cyan.ttml", channel);
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
