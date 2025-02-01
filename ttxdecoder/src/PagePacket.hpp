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


#ifndef TTXDECODER_PAGEPACKET_HPP_
#define TTXDECODER_PAGEPACKET_HPP_

namespace ttxdecoder
{

class Packet;

/**
 * Page packet.
 */
class PagePacket
{
public:
    /**
     * Constructor.
     *
     * @param packetPtr
     *      Pointer to packet data.
     *      The pointer must remain valid for the whole lifecycle.
     *      Normally it would be a member of derived class.
     */
    PagePacket(Packet* packetPtr) :
            m_valid(false),
            m_packetPtr(packetPtr)
    {
        // noop
    }

    /**
     * Sets packet validity.
     *
     * @param valid
     *      True to set packet as valid, false as invalid.
     */
    void setValid(bool valid)
    {
        m_valid = valid;
    }

    /**
     * Checks if packet is valid.
     *
     * @retval true
     *      Packet is valid.
     * @retval false
     *      Packet is not valid.
     */
    bool isValid() const
    {
        return m_valid;
    }

    /**
     * Takes packet.
     *
     * Marks packet as invalid and returns its pointer.
     *
     * @return
     *      Pointer to packet.
     */
    virtual Packet* takePacket()
    {
        setValid(false);
        return m_packetPtr;
    }

    /**
     * Returns packet if valid.
     *
     * @return
     *      Packet address is packet is valid, null pointer otherwise.
     */
    const Packet* getPacket() const
    {
        if (isValid())
        {
            return m_packetPtr;
        }
        else
        {
            return nullptr;
        }
    }

    /**
     * Checks if the internal packet pointer matches the given one.
     *
     * @param packet
     *      Packet to check.
     *
     * @return
     *      True if pointers match, false otherwise.
     */
    bool isPacket(const Packet* packet) const
    {
        return m_packetPtr == packet;
    }

private:
    /** Validity flag. */
    bool m_valid;

    /** Packet pointer. */
    Packet* const m_packetPtr;
};

/**
 * Typed page packet.
 */
template<class PacketType>
class TypedPagePacket : public PagePacket
{
public:
    /**
     * Constructor.
     */
    TypedPagePacket() :
            PagePacket(&m_packet),
            m_packet()
    {
        // noop
    }

    /**
     * Returns packet if valid.
     *
     * @return
     *      Packet address is packet is valid, null pointer otherwise.
     */
    const PacketType* getTypedPacket() const
    {
        if (isValid())
        {
            return &m_packet;
        }
        else
        {
            return nullptr;
        }
    }

private:
    /** Packet data. */
    PacketType m_packet;
};

}
 // namespace ttxdecoder

#endif /*TTXDECODER_PAGEPACKET_HPP_*/
