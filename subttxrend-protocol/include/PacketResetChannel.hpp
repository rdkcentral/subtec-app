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


#ifndef SUBTTXREND_PROTOCOL_PACKETRESETCHANNEL_HPP_
#define SUBTTXREND_PROTOCOL_PACKETRESETCHANNEL_HPP_

#include "Packet.hpp"

namespace subttxrend
{
namespace protocol
{

/**
 * Packet - RESET CHANNEL.
 */
class PacketResetChannel final : public PacketChannelSpecific
{
  public:
    /**
     * Constructor.
     */
    PacketResetChannel()
        : PacketChannelSpecific{Type::RESET_CHANNEL}
    {
    }
};

} // namespace protocol
} // namespace subttxrend

#endif /*SUBTTXREND_PROTOCOL_PACKETRESETCHANNEL_HPP_*/
