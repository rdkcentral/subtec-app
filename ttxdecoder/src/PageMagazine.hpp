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


#ifndef TTXDECODER_PAGEMAGAZINE_HPP_
#define TTXDECODER_PAGEMAGAZINE_HPP_

#include "Page.hpp"
#include "PacketTriplets.hpp"

namespace ttxdecoder
{

/**
 * Magazine page.
 *
 * It is a pseudo-page to hold M/x/x packets.
 */
class PageMagazine : public Page
{
public:
    /**
     * Constructor.
     */
    PageMagazine() = default;

    /**
     * Returns packet M/29/0 if valid.
     *
     * @return
     *      Packet if valid, nullptr otherwise.
     */
    const PacketTriplets* getPacket29_0() const
    {
        return m_packet_29_0.getTypedPacket();
    }

    /**
     * Returns packet M/29/1 if valid.
     *
     * @return
     *      Packet if valid, nullptr otherwise.
     */
    const PacketTriplets* getPacket29_1() const
    {
        return m_packet_29_1.getTypedPacket();
    }

    /**
     * Returns packet M/29/4 if valid.
     *
     * @return
     *      Packet if valid, nullptr otherwise.
     */
    const PacketTriplets* getPacket29_4() const
    {
        return m_packet_29_4.getTypedPacket();
    }

    /** @copydoc Page::getType() */
    virtual PageType getType() const override
    {
        return PageType::MAGAZINE;
    }

    /** @copydoc Page::isValid */
    virtual bool isValid() const override
    {
        // always valid
        return true;
    }

    /** @copydoc Page::invalidate */
    virtual void invalidate() override
    {
        m_packet_29_0.setValid(false);
        m_packet_29_1.setValid(false);
        m_packet_29_4.setValid(false);
    }

protected:
    /** @copydoc Page::getPagePacket */
    virtual PagePacket* getPagePacket(std::uint8_t packetAddress,
                                      std::int8_t designationCode) override
    {
        if (packetAddress == 29)
        {
            switch (designationCode)
            {
            case 0:
                return &m_packet_29_0;
            case 1:
                return &m_packet_29_1;
            case 4:
                return &m_packet_29_4;
            default:
                break;
            }
        }

        return nullptr;
    }

    /** @copydoc Page::getPagePacket */
    virtual const PagePacket* getPagePacket(std::uint8_t packetAddress,
                                            std::int8_t designationCode) const override
    {
        if (packetAddress == 29)
        {
            switch (designationCode)
            {
            case 0:
                return &m_packet_29_0;
            case 1:
                return &m_packet_29_1;
            case 4:
                return &m_packet_29_4;
            default:
                break;
            }
        }

        return nullptr;
    }

private:
    /** Header packet. */
    TypedPagePacket<PacketTriplets> m_packet_29_0;

    /** Header packet. */
    TypedPagePacket<PacketTriplets> m_packet_29_1;

    /** Header packet. */
    TypedPagePacket<PacketTriplets> m_packet_29_4;
};

}
 // namespace ttxdecoder

#endif /*TTXDECODER_PAGEMAGAZINE_HPP_*/
