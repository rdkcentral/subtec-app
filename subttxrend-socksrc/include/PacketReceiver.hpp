/*****************************************************************************
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2021 Liberty Global Service B.V.
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
*****************************************************************************/


#ifndef SUBTTXREND_SOCKSRC_PACKETRECEIVER_HPP_
#define SUBTTXREND_SOCKSRC_PACKETRECEIVER_HPP_

#include <subttxrend/common/DataBuffer.hpp>
#include <subttxrend/protocol/Packet.hpp>

namespace subttxrend
{
namespace socksrc
{

/**
 * Interface for receiving packets.
 */
class PacketReceiver
{
public:
    /**
     * Constructor.
     */
    PacketReceiver() = default;

    /**
     * Destructor.
     */
    virtual ~PacketReceiver() = default;

    /**
     * Valid packet received notification.
     *
     * This method is called by source if new valid packet is received.
     *
     * @param packet
     *      Packet received. The packet is only valid within notification.
     */
    virtual void onPacketReceived(const protocol::Packet& packet) = 0;
    virtual void addBuffer(common::DataBufferPtr buffer) = 0;

    /**
     * Stream broken notification.
     *
     * This method is called by source if the packet stream is broken
     * (e.g. invalid packet was received).
     */
    virtual void onStreamBroken() = 0;
};

} // namespace socksrc
} // namespace subttxrend

#endif /*SUBTTXREND_SOCKSRC_PACKETRECEIVER_HPP_*/
