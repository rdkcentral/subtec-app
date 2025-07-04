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
#include <utils.h>

int main(int argc, char *argv[])
{
    debug_log("Test timestamp packet started");
    bool ret = false;

    WebVttChannel *channel = new WebVttChannel();

    ret = channel->InitComms();
    if (ret)
    {
        channel->SendResetAllPacket();
        channel->SendResetChannelPacket();
        channel->SendSelectionPacket(1920, 1080);
    	channel->SendUnmutePacket();
        channel->SendTimestampPacket(0);
    }
    else
    {
        debug_log("InitComms failed !");
    }
    /* Sleep is added here for runWorkerTask thread in PacketSender to be created and in a 
    stable state, so that there is no crash when ~PacketSender checks joinable() 
    and tries to join the thread */
    sleep(5);
    debug_log("Test ended");
    return 0;
}
