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


#ifndef DVBSUBDECODER_CLUT_HPP_
#define DVBSUBDECODER_CLUT_HPP_

#include <cstdint>
#include <array>

namespace dvbsubdecoder
{

/**
 * Colour lookup table.
 */
class Clut
{
public:
    /**
     * Constructor.
     */
    Clut();

    /**
     * Destructor.
     */
    ~Clut() = default;

    /**
     * Resets CLUT to defaults.
     */
    void reset();

    /**
     * Returns CLUT id.
     *
     * @return
     *      CLUT id.
     */
    std::uint8_t getId() const
    {
        return m_id;
    }

    /**
     * Sets CLUT id.
     *
     * @param id
     *      CLUT id.
     */
    void setId(std::uint8_t id)
    {
        m_id = id;
    }

    /**
     * Returns version.
     *
     * @return
     *      Version.
     */
    std::uint8_t getVersion() const
    {
        return m_version;
    }

    /**
     * Sets version.
     *
     * @param version
     *      Version.
     */
    void setVersion(std::uint8_t version)
    {
        m_version = version;
    }

    /**
     * Sets 2-bit color entry.
     *
     * @param index
     *      Color index.
     * @param argb
     *      ARGB value.
     */
    void set2bit(std::uint8_t index,
                 std::uint32_t argb)
    {
        m_clut2bit[index] = argb;
    }

    /**
     * Sets 4-bit color entry.
     *
     * @param index
     *      Color index.
     * @param argb
     *      ARGB value.
     */
    void set4bit(std::uint8_t index,
                 std::uint32_t argb)
    {
        m_clut4bit[index] = argb;
    }

    /**
     * Sets 8-bit color entry.
     *
     * @param index
     *      Color index.
     * @param argb
     *      ARGB value.
     */
    void set8bit(std::uint8_t index,
                 std::uint32_t argb)
    {
        m_clut8bit[index] = argb;
    }

    /**
     * Returns CLUT array (2-bit version).
     *
     * @return
     *      CLUT array.
     */
    const std::uint32_t* getArray2bit() const
    {
        return m_clut2bit.data();
    }

    /**
     * Returns CLUT array (4-bit version).
     *
     * @return
     *      CLUT array.
     */
    const std::uint32_t* getArray4bit() const
    {
        return m_clut4bit.data();
    }

    /**
     * Returns CLUT array (8-bit version).
     *
     * @return
     *      CLUT array.
     */
    const std::uint32_t* getArray8bit() const
    {
        return m_clut8bit.data();
    }

private:
    /** CLUT identifier. */
    std::uint8_t m_id{};

    /** CLUT version. */
    std::uint8_t m_version;

    /** CLUT - 2-bit depth. */
    std::array<std::uint32_t, 1 << 2> m_clut2bit;

    /** CLUT - 4-bit depth. */
    std::array<std::uint32_t, 1 << 4> m_clut4bit;

    /** CLUT - 8-bit depth. */
    std::array<std::uint32_t, 1 << 8> m_clut8bit;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_CLUT_HPP_*/
