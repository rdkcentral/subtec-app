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


#ifndef TTXDECODER_PACKETEDITORIALLINKS_HPP_
#define TTXDECODER_PACKETEDITORIALLINKS_HPP_

#include "Packet.hpp"
#include "PageId.hpp"

namespace ttxdecoder
{

/**
 * Editorial links (X/27/0-3) packet.
 */
class PacketEditorialLinks : public Packet
{
public:
    /** @copydoc Packet::getType() */
    virtual PacketType getType() const override
    {
        return PacketType::EDITORIAL_LINKS;
    }

    /**
     * Returns designation code.
     *
     * @return
     *      Designation code.
     */
    std::int8_t getDesignationCode() const
    {
        return m_designationCode;
    }

    /**
     * Sets designation code.
     *
     * @param designationCode
     *      Designation code.
     */
    void setDesignationCode(std::int8_t designationCode)
    {
        m_designationCode = designationCode;
    }

    /**
     * Returns link.
     *
     * @param index
     *      Link index.
     *
     * @return
     *      Link at given index. Invalid page id if index is invalid.
     */
    const PageId& getLink(std::size_t index) const
    {
        static PageId INVALID;

        if (index < LINK_COUNT)
        {
            return m_links[index];
        }
        else
        {
            return INVALID;
        }
    }

    /**
     * Sets link.
     *
     * @param index
     *      Link index.
     * @param link
     *      Link data.
     */
    void setLink(std::size_t index,
                 const PageId& link)
    {
        if (index < LINK_COUNT)
        {
            m_links[index] = link;
        }
    }

    /**
     * Returns link control.
     *
     * @return
     *      Link control.
     */
    std::int8_t getLinkControl() const
    {
        return m_linkControl;
    }

    /**
     * Sets link control.
     *
     * @param linkControl
     *      Link control.
     */
    void setLinkControl(std::int8_t linkControl)
    {
        m_linkControl = linkControl;
    }

    /**
     * Returns CRC.
     *
     * @return
     *      CRC value.
     */
    std::uint16_t getCrc() const
    {
        return m_crc;
    }

    /**
     * Sets CRC.
     *
     * @param crc
     *      CRC value.
     */
    void setCrc(std::uint16_t crc)
    {
        m_crc = crc;
    }

private:
    /** Number of links. */
    static const std::size_t LINK_COUNT = 6;

    /** Designation code. */
    std::int8_t m_designationCode;

    /** Links. */
    PageId m_links[LINK_COUNT];

    /** Link Control. */
    std::int8_t m_linkControl;

    /** CRC. */
    std::uint16_t m_crc;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_PACKETEDITORIALLINKS_HPP_*/
