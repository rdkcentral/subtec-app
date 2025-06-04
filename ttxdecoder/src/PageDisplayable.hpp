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

#pragma once

#include "ControlInfo.hpp"
#include "Page.hpp"
#include "PacketLopData.hpp"
#include "PacketEditorialLinks.hpp"
#include "PacketTriplets.hpp"

namespace ttxdecoder
{

/**
 * Displayable page (LOP + some extensions).
 */
class PageDisplayable : public Page
{
public:
    /** Maximum number of rows (1..25). */
    static const std::size_t DISPLAYABLE_ROWS = 25;

    /** Number of supported X_26 packets - X/26/0-15. */
    static const std::size_t X_26_PACKET_COUNT = 16;

    /**
     * Constructor.
     */
    PageDisplayable()
    {
        internalInvalidate();
    }

    /**
     * Checks if page content can change. If it does, page has to be redrawn
     * every time it is decoded.
     *
     * @return
     *      True if page content can change, false otherwise.
     */
    bool isVolatile() const
    {
        auto pageHeader = getHeader();
        return pageHeader
                && ((pageHeader->getControlInfo() & ControlInfo::ERASE_PAGE) == 0);
    }

    /** @copydoc Page::getType() */
    virtual PageType getType() const override
    {
        return PageType::DISPLAYABLE;
    }

    /**
     * Returns displayable row packet.
     *
     * Only returns packet if it is valid.
     *
     * @param packetAddress
     *      Packet address.
     *
     * @return
     *      Packet if available, null pointer otherwise.
     */
    const PacketLopData* getLopData(std::uint8_t packetAddress) const
    {
        if ((packetAddress >= 1) && (packetAddress <= DISPLAYABLE_ROWS))
        {
            return m_displayableRows[packetAddress - 1].getTypedPacket();
        }

        return nullptr;
    }

    /**
     * Returns X/28/N packet.
     *
     * Only returns packet if it is valid.
     *
     * @param designationCode
     *      Designation code.
     *
     * @return
     *      Packet if available, null pointer otherwise.
     */
    const PacketTriplets* getPacketX28(std::int8_t designationCode) const
    {
        if ((designationCode >= 0)
                && (static_cast<std::size_t>(designationCode)
                        < X_28_PACKET_COUNT))
        {
            return m_packetsX28[designationCode].getTypedPacket();
        }

        return nullptr;
    }

    /**
     * Returns X/26/N packet.
     *
     * Only returns packet if it is valid.
     *
     * @param designationCode
     *      Designation code.
     *
     * @return
     *      Packet if available, null pointer otherwise.
     */
    const PacketTriplets* getPacketX26(std::int8_t designationCode) const
    {
        if ((designationCode >= 0)
                && (static_cast<std::size_t>(designationCode)
                        < X_26_PACKET_COUNT))
        {
            return m_packetsX26[designationCode].getTypedPacket();
        }

        return nullptr;
    }

    /**
     * Returns editorial links packet.
     *
     * Only returns packet if it is valid.
     *
     * @return
     *      Packet if available, null pointer otherwise.
     */
    const PacketEditorialLinks* getEditorialLinks() const
    {
        return m_editorialLinks.getTypedPacket();
    }

    /** @copydoc Page::isValid */
    virtual bool isValid() const override
    {
        if (!m_header.isValid())
        {
            return false;
        }

        // displayable rows or editorial links are optional

        return true;
    }

    /** @copydoc Page::invalidate */
    virtual void invalidate() override
    {
        internalInvalidate();
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
        else if ((packetAddress >= 1) && (packetAddress <= DISPLAYABLE_ROWS))
        {
            return &m_displayableRows[packetAddress - 1];
        }
        else if (packetAddress == 26)
        {
            if ((designationCode >= 0)
                    && (static_cast<std::size_t>(designationCode)
                            < X_26_PACKET_COUNT))
            {
                return &m_packetsX26[designationCode];
            }
        }
        else if ((packetAddress == 27) && (designationCode == 0))
        {
            return &m_editorialLinks;
        }
        else if (packetAddress == 28)
        {
            if ((designationCode >= 0)
                    && (static_cast<std::size_t>(designationCode)
                            < X_28_PACKET_COUNT))
            {
                return &m_packetsX28[designationCode];
            }
        }

        return nullptr;
    }

    /** @copydoc Page::getPagePacket */
    virtual const PagePacket* getPagePacket(std::uint8_t packetAddress,
                                            std::int8_t designationCode) const override
    {
        if (packetAddress == 0)
        {
            return &m_header;
        }
        else if ((packetAddress >= 1) && (packetAddress <= DISPLAYABLE_ROWS))
        {
            return &m_displayableRows[packetAddress - 1];
        }
        else if (packetAddress == 26)
        {
            if ((designationCode >= 0)
                    && (static_cast<std::size_t>(designationCode)
                            < X_26_PACKET_COUNT))
            {
                return &m_packetsX26[designationCode];
            }
        }
        else if ((packetAddress == 27) && (designationCode == 0))
        {
            return &m_editorialLinks;
        }
        else if (packetAddress == 28)
        {
            if ((designationCode >= 0)
                    && (static_cast<std::size_t>(designationCode)
                            < X_28_PACKET_COUNT))
            {
                return &m_packetsX28[designationCode];
            }
        }

        return nullptr;
    }

private:
    void internalInvalidate()
    {
        m_header.setValid(false);
        for (auto& displayableRow : m_displayableRows)
        {
            displayableRow.setValid(false);
        }
        m_editorialLinks.setValid(false);
        for (auto& x26Packet : m_packetsX26)
        {
            x26Packet.setValid(false);
        }
        for (auto& x28Packet : m_packetsX28)
        {
            x28Packet.setValid(false);
        }
    }

private:
    /** Number of supported X_28 packets - X/28/0-4. */
    static const std::size_t X_28_PACKET_COUNT = 5;

    /** Header packet. */
    TypedPagePacket<PacketHeader> m_header;

    /** Header packet. */
    TypedPagePacket<PacketLopData> m_displayableRows[DISPLAYABLE_ROWS];

    /** Editorial links packet. */
    TypedPagePacket<PacketEditorialLinks> m_editorialLinks;

    /** Enhancement packets X/26. */
    TypedPagePacket<PacketTriplets> m_packetsX26[X_26_PACKET_COUNT];

    /** Enhancement packets X/28. */
    TypedPagePacket<PacketTriplets> m_packetsX28[X_28_PACKET_COUNT];
};

}
// namespace ttxdecoder

