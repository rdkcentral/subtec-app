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


#ifndef SUBTTXREND_PROTOCOL_PACKETTELETEXTSELECTION_HPP_
#define SUBTTXREND_PROTOCOL_PACKETTELETEXTSELECTION_HPP_

#include "Packet.hpp"

namespace subttxrend
{
namespace protocol
{

/**
 * Packet - TELETEXT SELECTION.
 */
class PacketTeletextSelection final : public PacketChannelSpecific
{
public:
    /**
     * Constructor.
     */
    PacketTeletextSelection();

    /**
     * Returns starting magazine number.
     *
     * @return
     *      Magazine number.
     */
    std::uint32_t getInitialMagazine() const;

    /**
     * Returns starting page number.
     *
     * @return
     *      Page number.
     */
    std::uint32_t getInitialPage() const;

protected:
    /** @copydoc Packet::parseData */
    bool parseDataHeader(BufferReader& bufferReader) override;

private:
    /** Initial magazine number. */
    std::uint32_t m_intitialMagazine;

    /** Initial page number. */
    std::uint32_t m_initialPage;

    /** Logger. */
    static common::Logger m_logger;
};

} // namespace protocol
} // namespace subttxrend

#endif /*SUBTTXREND_PROTOCOL_PACKETTELETEXTSELECTION_HPP_*/
