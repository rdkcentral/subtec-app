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


#ifndef TTXDECODER_PAGEBTT_HPP_
#define TTXDECODER_PAGEBTT_HPP_

#include "Page.hpp"
#include "PacketBttPageType.hpp"

namespace ttxdecoder
{

/**
 * BTT page.
 */
class PageBtt : public Page
{
public:
    /**
     * Constructor.
     */
    PageBtt() = default;

    /** @copydoc Page::getType() */
    virtual PageType getType() const override
    {
        return PageType::BTT;
    }

    /**
     * Returns page type row packet.
     *
     * Only returns packet if it is valid.
     *
     * @return
     *      Packet if available, null pointer otherwise.
     */
    const PacketBttPageType* getPageTypePacket(std::uint8_t packetAddress) const
    {
        if ((packetAddress >= 1) && (packetAddress <= PAGE_TYPE_ROWS))
        {
            return m_pageTypeRows[packetAddress - 1].getTypedPacket();
        }

        return nullptr;
    }

    /** @copydoc Page::isValid */
    virtual bool isValid() const override
    {
        if (!m_header.isValid())
        {
            return false;
        }

        for (std::size_t i = 0; i < PAGE_TYPE_ROWS; ++i)
        {
            if (!m_pageTypeRows[i].isValid())
            {
                return false;
            }
        }

        return true;
    }

    /** @copydoc Page::invalidate */
    virtual void invalidate() override
    {
        m_header.setValid(false);
        for (std::size_t i = 0; i < PAGE_TYPE_ROWS; ++i)
        {
            m_pageTypeRows[i].setValid(false);
        }
    }

protected:
    /** @copydoc Page::getPagePacket */
    virtual PagePacket* getPagePacket(std::uint8_t packetAddress,
                                      std::int8_t designationCode) override
    {
        if (packetAddress == 0)
        {
            return &m_header;
        }
        else if ((packetAddress >= 1) && (packetAddress <= PAGE_TYPE_ROWS))
        {
            return &m_pageTypeRows[packetAddress - 1];
        }
        else
        {
            return nullptr;
        }
    }

    /** @copydoc Page::getPagePacket */
    virtual const PagePacket* getPagePacket(std::uint8_t packetAddress,
                                            std::int8_t designationCode) const override
    {
        if (packetAddress == 0)
        {
            return &m_header;
        }
        else if ((packetAddress >= 1) && (packetAddress <= PAGE_TYPE_ROWS))
        {
            return &m_pageTypeRows[packetAddress - 1];
        }
        else
        {
            return nullptr;
        }
    }

private:
    /** Number of page type rows (1..20). */
    static const std::size_t PAGE_TYPE_ROWS = 20;

    /** Header packet. */
    TypedPagePacket<PacketHeader> m_header;

    /** Page type rows packets. */
    TypedPagePacket<PacketBttPageType> m_pageTypeRows[PAGE_TYPE_ROWS];
};

}
 // namespace ttxdecoder

#endif /*TTXDECODER_PAGEBTT_HPP_*/
