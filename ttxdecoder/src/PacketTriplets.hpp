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


#ifndef TTXDECODER_PACKETTRIPLETS_HPP_
#define TTXDECODER_PACKETTRIPLETS_HPP_

#include "Packet.hpp"

namespace ttxdecoder
{

/**
 * Enhancement packet with triplets.
 */
class PacketTriplets : public Packet
{
public:
    /** Number of triplets. */
    static const std::size_t TRIPLET_COUNT = 13;

    /** @copydoc Packet::getType() */
    virtual PacketType getType() const override
    {
        return PacketType::TRIPLETS;
    }

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
     * Returns triplet value.
     *
     * @param index
     *      Triplet index.
     *
     * @return
     *      Triplet value (decoded).
     *      For invalid indexes returns 0xFFFFFFFF.
     */
    uint32_t getTripletValue(std::size_t index) const
    {
        if (index < TRIPLET_COUNT)
        {
            return m_triplets[index];
        }
        else
        {
            return 0xFFFFFFFF;
        }
    }

    /**
     * Sets triplet value.
     *
     * @param index
     *      Triplet value.
     * @param value
     *      Decoded triplet value to set.
     */
    void setTripletValue(std::size_t index,
                         std::uint32_t value)
    {
        if (index < TRIPLET_COUNT)
        {
            m_triplets[index] = value;
        }
    }

private:
    /** Designation code. */
    std::int8_t m_designationCode;

    /** Triplet values. */
    std::uint32_t m_triplets[TRIPLET_COUNT];
};

} // namespace ttxdecoder

#endif /*TTXDECODER_PACKETTRIPLETS_HPP_*/
