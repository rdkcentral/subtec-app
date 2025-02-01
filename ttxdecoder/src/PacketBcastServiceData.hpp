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


#ifndef TTXDECODER_PACKETBSD_HPP_
#define TTXDECODER_PACKETBSD_HPP_

#include "Packet.hpp"
#include "PageId.hpp"

namespace ttxdecoder
{

/**
 * Broadcast service data.
 *
 * The BSD is transmitted via 8/30 packets.
 *
 * Please note that the network/program informations are currently
 * not parsed.
 */
class PacketBcastServiceData : public Packet
{
public:
    /** @copydoc Packet::getType() */
    virtual PacketType getType() const override
    {
        return PacketType::BCAST_SERVICE_DATA;
    }

    /**
     * Data format.
     */
    enum class Format
    {
        /** Format 1. */
        F1,
        /** Format 2. */
        F2,
        /** Unknown format. */
        UNKNOWN
    };

    /**
     * Function.
     */
    enum class Function
    {
        /** Multiplexed function. */
        MULTIPLEXED,
        /** Non-multiplexed function. */
        NON_MULTIPLEXED,
        /** Unknown function. */
        UNKNOWN
    };

    /**
     * Returns designation code.
     *
     * @return
     *      Designation code (negative values means invalid).
     */
    int8_t getDesignationCode() const
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
     * Returns data format.
     *
     * @return
     *      Data format desoded from designation code.
     */
    Format getFormat() const
    {
        switch (m_designationCode)
        {
        case 0:
        case 1:
            return Format::F1;
        case 2:
        case 3:
            return Format::F2;
        default:
            return Format::UNKNOWN;
        }
    }

    /**
     * Returns function.
     *
     * @return
     *      Function decoded from designation code.
     */
    Function getFunction() const
    {
        switch (m_designationCode)
        {
        case 0:
        case 2:
            return Function::MULTIPLEXED;
        case 1:
        case 3:
            return Function::NON_MULTIPLEXED;
        default:
            return Function::UNKNOWN;
        }
    }

    /**
     * Returns initial page id.
     *
     * @return
     *      Initial page id.
     */
    const PageId& getInitialPage() const
    {
        return m_initialPage;
    }

    /**
     * Sets initial page id.
     *
     * @param pageId
     *      Initial page id.
     */
    void setInitialPage(const PageId& pageId)
    {
        m_initialPage = pageId;
    }

    /**
     * Returns status display buffer.
     *
     * @return
     *      Status display buffer.
     */
    std::int8_t* getStatusDisplayBuffer()
    {
        return m_statusDisplay;
    }

    /**
     * Returns status display buffer.
     *
     * @return
     *      Status display buffer.
     */
    const std::int8_t* getStatusDisplayBuffer() const
    {
        return m_statusDisplay;
    }

    /**
     * Returns status display buffer length.
     *
     * @return
     *      Status display buffer length in bytes.
     */
    std::size_t getStatusDisplayBufferLength()
    {
        return STATUS_DISPLAY_BUFFER_LENGTH;
    }

private:
    /** Status display buffer length in bytes. */
    static const std::size_t STATUS_DISPLAY_BUFFER_LENGTH = 20;

    /** Designation code. */
    std::int8_t m_designationCode;

    /** Initial page. */
    PageId m_initialPage;

    /** Status display buffer. */
    std::int8_t m_statusDisplay[STATUS_DISPLAY_BUFFER_LENGTH + 1];
};

}
 // namespace ttxdecoder

#endif /*TTXDECODER_PACKETBSD_HPP_*/
