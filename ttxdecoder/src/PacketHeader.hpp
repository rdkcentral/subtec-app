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


#ifndef TTXDECODER_PACKETHEADER_HPP_
#define TTXDECODER_PACKETHEADER_HPP_

#include <cstddef>
#include "Packet.hpp"
#include "PageId.hpp"

namespace ttxdecoder
{

/**
 * Header (X/0) packet.
 */
class PacketHeader : public Packet
{
public:
    /** @copydoc Packet::getType() */
    virtual PacketType getType() const override
    {
        return PacketType::HEADER;
    }

    /**
     * Returns page identifier.
     *
     * @return
     *      Page identifier.
     */
    const PageId& getPageId() const
    {
        return m_pageId;
    }

    /**
     * Returns control information value.
     *
     * @return
     *      Control information value.
     */
    std::uint8_t getControlInfo() const
    {
        return m_controlInfo;
    }

    /**
     * Returns national option value.
     *
     * @return
     *      National option value.
     */
    std::uint8_t getNationalOption() const
    {
        return m_nationalOption;
    }

    /**
     * Sets the page information.
     *
     * @param pageId
     *      Collected page identifier.
     * @param controlInfo
     *      Control information value.
     * @param nationalOption
     *      National option value.
     */
    void setPageInfo(const PageId& pageId,
                     const std::uint8_t controlInfo,
                     const std::uint8_t nationalOption)
    {
        m_pageId = pageId;
        m_controlInfo = controlInfo;
        m_nationalOption = nationalOption;
    }

    /**
     * Returns data buffer length.
     *
     * @return
     *      Data buffer length in bytes.
     */
    std::size_t getBufferLength() const
    {
        return BUFFER_LENGTH;
    }

    /**
     * Returns data buffer.
     *
     * @return
     *      Data buffer.
     */
    std::int8_t* getBuffer()
    {
        return m_buffer;
    }

    /**
     * Returns data buffer.
     *
     * @return
     *      Data buffer.
     */
    const std::int8_t* getBuffer() const
    {
        return m_buffer;
    }

private:
    /** Number of the data bytes. */
    static const std::size_t BUFFER_LENGTH = 32;

    /** Page identifier. */
    PageId m_pageId;

    /** Control information value. */
    std::uint8_t m_controlInfo;

    /** National option value. */
    std::uint8_t m_nationalOption;

    /** Data buffer. */
    std::int8_t m_buffer[BUFFER_LENGTH];
};

} // namespace ttxdecoder

#endif /*TTXDECODER_PACKETHEADER_HPP_*/
