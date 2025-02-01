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


#ifndef TTXDECODER_CHARSETMANAGER_HPP
#define TTXDECODER_CHARSETMANAGER_HPP

#include <algorithm>
#include <cstdint>

#include <subttxrend/common/NonCopyable.hpp>

#include "CharsetConfig.hpp"
#include "CharsetMaps.hpp"
#include "NationalOptions.hpp"
#include "Types.hpp"

namespace ttxdecoder
{

/**
 * Manages charset related settings:
 * - replace default charsets
 * - choose utf or diactric mode
 * - setting default primary and secondary languages.
 */
class CharsetManager : private subttxrend::common::NonCopyable,
                       public CharsetConfig
{
public:

    /**
     * Constructor.
     */
    CharsetManager() :
            m_mode(CharsetConfig::Mode::DIACTRIC),
            m_charsetMaps(),
            m_nationalSubsets(),
            m_primaryNationalOptions(),
            m_secondaryNationalOptions()
    {
        // noop
    }

    /** @copydoc Engine::getCharsetMapping */
    const CharsetMappingArray& getCharsetMapping(Charset charset) const
    {
        return m_charsetMaps[charset];
    }

    /** @copydoc Engine::setCharsetMapping */
    void setCharsetMapping(Charset charset,
                           const CharsetMappingArray& mappingArray)
    {
        m_charsetMaps[charset] = mappingArray;
    }

    /** @copydoc Engine::setDefaultPrimaryNationalCharset */
    void setDefaultPrimaryNationalCharset(std::uint8_t index,
                                          NationalCharset charset)
    {
        m_primaryNationalOptions.setOption(index, charset);
    }

    /** @copydoc Engine::setDefaultSecondaryNationalCharset */
    void setDefaultSecondaryNationalCharset(std::uint8_t index,
                                            NationalCharset charset)
    {
        m_secondaryNationalOptions.setOption(index, charset);
    }

    /**
     * Sets operation mode - utf or diactric.
     *
     * @param mode
     *      Sets operational mode.
     */
    void setMode(CharsetConfig::Mode mode)
    {
        m_mode = mode;
    }

    /** @copydoc CharsetConfig::getMode */
    virtual CharsetConfig::Mode getMode() const override
    {
        return m_mode;
    }

    /** @copydoc CharsetConfig::getPrimaryNationalCharset */
    virtual NationalCharset getPrimaryNationalCharset(std::uint8_t index = 0) const override
    {
        return m_primaryNationalOptions.getOption(index);
    }

    /** @copydoc CharsetConfig::getSecondaryNationalCharset */
    virtual NationalCharset getSecondaryNationalCharset(std::uint8_t index = 0) const override
    {
        return m_secondaryNationalOptions.getOption(index);
    }

    /** @copydoc CharsetConfig::getNationalSubsets */
    virtual const NationalSubsets& getNationalSubsets() const override
    {
        return m_nationalSubsets;
    }

    /** @copydoc CharsetConfig::getCharsetMaps */
    virtual const CharsetMaps& getCharsetMaps() const override
    {
        return m_charsetMaps;
    }

private:

    /** Charset mode - utf od diactric. */
    CharsetConfig::Mode m_mode;

    /** Charset maps. */
    CharsetMaps m_charsetMaps;

    /** National option subsets. */
    NationalSubsets m_nationalSubsets;

    /** Primary set of national options. */
    NationalOptions m_primaryNationalOptions;

    /** Secondary set of national options. */
    NationalOptions m_secondaryNationalOptions;
};

} // namespace ttxdecoder

#endif /* TTXDECODER_CHARSETMANAGER_HPP */
