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


#ifndef TTXDECODER_PAGE_HPP_
#define TTXDECODER_PAGE_HPP_

#include "PagePacket.hpp"
#include "Packet.hpp"
#include "PacketHeader.hpp"

#include <subttxrend/common/NonCopyable.hpp>

namespace ttxdecoder
{

/**
 * Page type.
 */
enum class PageType
{
    /** Displayable page. */
    DISPLAYABLE,

    /** BTT page. */
    BTT,

    /** Magazine page. */
    MAGAZINE,
};

/**
 * Base class for collected pages.
 */
class Page : private subttxrend::common::NonCopyable
{
public:
    /**
     * Constructor.
     */
    Page() :
            m_lastTakenPacket(nullptr)
    {
        // noop
    }

    /**
     * Destructor.
     */
    virtual ~Page() = default;

    /**
     * Returns page type.
     *
     * @return
     *      Page type.
     */
    virtual PageType getType() const = 0;


    /**
     * Returns page identifier.
     *
     * @return
     *      Page identifier. Invalid if page does not have header or
     *      header is not valid.
     */
    const PageId& getPageId() const
    {
        const auto header = getHeader();
        if (header)
        {
            return header->getPageId();
        }
        else
        {
            static PageId INVALID;
            return INVALID;
        }
    }

    /**
     * Returns header packet.
     *
     * Only returns packet if it is valid.
     *
     * @return
     *      Packet if available, null pointer otherwise.
     */
    const PacketHeader* getHeader() const
    {
        auto packet = getPacket(0, 0);
        if (packet)
        {
            if (packet->getType() == PacketType::HEADER)
            {
                return static_cast<const PacketHeader*>(packet);
            }
        }
        return nullptr;
    }

    /**
     * Returns header packet.
     *
     * Marks the packet as invalid.
     *
     * @return
     *      Packet if available, null pointer otherwise.
     */
    PacketHeader* takeHeader()
    {
        auto packet = takePacket(0, 0);
        if (packet)
        {
            if (packet->getType() == PacketType::HEADER)
            {
                return static_cast<PacketHeader*>(packet);
            }

            m_lastTakenPacket = nullptr;
        }
        return nullptr;
    }

    /**
     * Checks if page is valid.
     *
     * @retval true
     *      Page is valid.
     * @retval false
     *      Page is not valid.
     */
    virtual bool isValid() const = 0;

    /**
     * Invalidates the page.
     */
    virtual void invalidate() = 0;

    /**
     * Returns packet for given packet address.
     *
     * Marks the packet as invalid.
     *
     * @param packetAddress
     *      Packet address (0-31).
     * @param designationCode
     *      Designation code. Only valid when defined by packet type.
     *      Hamming-8-4 decoded, but not verified (will be negative if
     *      value is not valid for hamming encoding).
     *
     * @return
     *      Packet if available, null pointer otherwise.
     */
    Packet* takePacket(std::uint8_t packetAddress,
                       std::int8_t designationCode)
    {
        auto packet = getPagePacket(packetAddress, designationCode);
        if (packet)
        {
            m_lastTakenPacket = packet;
            return packet->takePacket();
        }
        else
        {
            return nullptr;
        }
    }

    /**
     * Returns packet for given packet address.
     *
     * Only returns packet if it is valid.
     *
     * @param packetAddress
     *      Packet address (0-31).
     * @param designationCode
     *      Designation code. Only valid when defined by packet type.
     *      Hamming-8-4 decoded, but not verified (will be negative if
     *      value is not valid for hamming encoding).
     *
     * @return
     *      Packet if available, null pointer otherwise.
     */
    const Packet* getPacket(std::uint8_t packetAddress,
                            std::int8_t designationCode) const
    {
        auto packet = getPagePacket(packetAddress, designationCode);
        if (packet)
        {
            return packet->getPacket();
        }
        else
        {
            return nullptr;
        }
    }

    /**
     * Marks last taken packet at valid.
     *
     * @param packet
     *      Packet pointer - to be used to verify if it is really
     *      the last packet.
     */
    void setLastPacketValid(const Packet* packet)
    {
        if (m_lastTakenPacket)
        {
            if (m_lastTakenPacket->isPacket(packet))
            {
                m_lastTakenPacket->setValid(true);
                m_lastTakenPacket = nullptr;
            }
        }
    }

protected:
    /**
     * Returns packet for given packet address.
     *
     * Marks the packet as invalid.
     *
     * @param packetAddress
     *      Packet address (0-31).
     * @param designationCode
     *      Designation code. Only valid when defined by packet type.
     *      Hamming-8-4 decoded, but not verified (will be negative if
     *      value is not valid for hamming encoding).
     *
     * @return
     *      Packet if available, null pointer otherwise.
     */
    virtual PagePacket* getPagePacket(std::uint8_t packetAddress,
                                      std::int8_t designationCode) = 0;

    /**
     * Returns packet for given packet address.
     *
     * Marks the packet as invalid.
     *
     * @param packetAddress
     *      Packet address (0-31).
     * @param designationCode
     *      Designation code. Only valid when defined by packet type.
     *      Hamming-8-4 decoded, but not verified (will be negative if
     *      value is not valid for hamming encoding).
     *
     * @return
     *      Packet if available, null pointer otherwise.
     */
    virtual const PagePacket* getPagePacket(std::uint8_t packetAddress,
                                            std::int8_t designationCode) const = 0;

private:
    /** Last taken packet. */
    PagePacket* m_lastTakenPacket;
};

}
 // namespace ttxdecoder

#endif /*TTXDECODER_PAGE_HPP_*/
