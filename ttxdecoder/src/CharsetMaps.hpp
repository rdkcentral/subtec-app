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


#ifndef TTXDECODER_CHARSET_HPP
#define TTXDECODER_CHARSET_HPP

#include <array>
#include <cassert>

#include "Types.hpp"
#include "DefaultCharsets.hpp"

namespace ttxdecoder
{
namespace // anonymous
{

/**
 * Converts NationalCharset enum to array index.
 *
 * @param nationalCharset
 *      Value to convert.
 *
 * @return
 *      Representation of national code.
 */
inline unsigned int toIndex(NationalCharset nationalCharset)
{
    return static_cast<unsigned int>(nationalCharset);
}

/**
 * Converts Charset enum to array index.
 *
 * @param charset
 *      Value to convert.
 *
 * @return
 *      Representation of Charset code.
 */
inline unsigned int toIndex(Charset charset)
{
    return static_cast<unsigned int>(charset);
}

} // namespace anonymous


/**
 * Charset map provider.
 */
class CharsetMaps
{
public:

    /**
     * Constructor.
     */
    CharsetMaps()
    {
        constexpr auto INIT_CHECK_START = __LINE__ + 1;
        m_maps[toIndex(Charset::G0_LATIN)] = G0_Latin_Default;
        m_maps[toIndex(Charset::G0_ARABIC)] = G0_Arabic;
        m_maps[toIndex(Charset::G0_CYRILLIC1)] = G0_Cyrillic_Option1;
        m_maps[toIndex(Charset::G0_CYRILLIC2)] = G0_Cyrillic_Option2;
        m_maps[toIndex(Charset::G0_CYRILLIC3)] = G0_Cyrillic_Option3;
        m_maps[toIndex(Charset::G0_GREEK)] = G0_Greek;
        m_maps[toIndex(Charset::G0_HEBREW)] = G0_Hebrew;
        m_maps[toIndex(Charset::G0_FARSI)] = G0_Latin_Default;
        m_maps[toIndex(Charset::G1_BLOCK_MOSAIC)] = G1_Mosaics_Block;
        m_maps[toIndex(Charset::G1_BLOCK_MOSAIC_SEPARATED)] = G1_Mosaics_Separated;
        m_maps[toIndex(Charset::G2_LATIN)] = G2_Latin;
        m_maps[toIndex(Charset::G2_ARABIC)] = G0_Latin_Default;
        m_maps[toIndex(Charset::G2_CYRILLIC)] = G0_Latin_Default;
        m_maps[toIndex(Charset::G2_GREEK)] = G0_Latin_Default;
        m_maps[toIndex(Charset::G2_FARSI)] = G0_Latin_Default;
        m_maps[toIndex(Charset::G3_SMOOTH_MOSAIC)] = G3_Mosaics_Smooth;
        m_maps[toIndex(Charset::G3_SMOOTH_MOSAIC_SEPARATED)] = G3_Mosaics_Smooth;
        m_maps[toIndex(Charset::DRCS)] = G0_Latin_Default;
        constexpr auto INIT_CHECK_END = __LINE__;

        static_assert((INIT_CHECK_END - INIT_CHECK_START) == CHARSETS_SIZE, "Not all maps are initialized.");
    }

    /**
     * Charset map getter - const version.
     *
     * @param charset
     *      Charset to get.
     * @return
     *      Charset array.
     */
    const CharsetMappingArray& operator[](Charset charset) const
    {
        return m_maps[toIndex(charset)];
    }

    /**
     * Charset map getter.
     *
     * @param charset
     *      Charset to get.
     * @return
     *      Charset array.
     */
    CharsetMappingArray& operator[](Charset charset)
    {
        unsigned int index = static_cast<int>(charset);
        return m_maps[toIndex(charset)];
    }

    /**
     * Charset map getter by NationalCharset.
     *
     * @param nationalCharset
     *      National charset to get charset for.
     * @return
     *      Charset array.
     */
    const CharsetMappingArray& operator[](NationalCharset nationalCharset) const
    {
        auto nationalIndex = toIndex(nationalCharset);
        assert(nationalIndex < NATIONAL_CHARSET_TO_CHARSET_SIZE);

        Charset charset = NATIONAL_CHARSET_TO_CHARSET[nationalIndex];

        // TODO: trace/verbose national -> nationalIndex -> charset

        return m_maps[toIndex(charset)];
    }

private:

    /** Charset maps size. */
    static constexpr std::size_t CHARSETS_SIZE = (static_cast<int>(Charset::LAST) + 1);

    /** Charset maps. */
    CharsetMappingArray m_maps[CHARSETS_SIZE];
};

/**
 * Represent national characters subset.
 */
class NationalSubsets
{
public:

    /**
     * Constructor.
     */
    NationalSubsets()
    {
        std::copy(std::begin(NATIONAL_SUBSETS), std::end(NATIONAL_SUBSETS), std::begin(m_nationalSubsets));
    }

    /**
     * National characters array getter - const version.
     *
     * @param nationalCharset
     *      National charset to get.
     * @return
     *      Array of NationalCharacters.
     */
    const NationalCharactersArray& operator[](NationalCharset nationalCharset) const
    {
        unsigned int index = static_cast<int>(nationalCharset);
        assert(index < NATIONAL_SUBSETS_COUNT);
        return m_nationalSubsets[index];
    }

    /**
     * National characters array getter.
     *
     * @param nationalCharset
     *      National charset to get.
     * @return
     *      Array of NationalCharacters.
     */
    const NationalCharactersArray& operator[](NationalCharset nationalCharset)
    {
        unsigned int index = static_cast<int>(nationalCharset);
        assert(index < NATIONAL_SUBSETS_COUNT);
        return m_nationalSubsets[index];
    }

private:

    /** NationalCharacters arrays for every national subset. */
    NationalCharactersArray m_nationalSubsets[NATIONAL_OPTIONS];
};

// basic sanity checks if charset enums were not modified...
static_assert(static_cast<int>(Charset::G0_LATIN) == 0, "Charset changed, verify default maps.");
static_assert(static_cast<int>(Charset::G0_ARABIC) == 1, "Charset changed, verify default maps.");
static_assert(static_cast<int>(Charset::G0_CYRILLIC1) == 2, "Charset changed, verify default maps.");
static_assert(static_cast<int>(Charset::G0_CYRILLIC2) == 3, "Charset changed, verify default maps.");
static_assert(static_cast<int>(Charset::G0_CYRILLIC3) == 4, "Charset changed, verify default maps.");
static_assert(static_cast<int>(Charset::G0_GREEK) == 5, "Charset changed, verify default maps.");
static_assert(static_cast<int>(Charset::G0_HEBREW) == 6, "Charset changed, verify default maps.");
static_assert(static_cast<int>(Charset::G0_FARSI) == 7, "Charset changed, verify default maps.");
static_assert(static_cast<int>(Charset::G1_BLOCK_MOSAIC) == 8, "Charset changed, verify default maps.");
static_assert(static_cast<int>(Charset::G1_BLOCK_MOSAIC_SEPARATED) == 9, "Charset changed, verify default maps.");
static_assert(static_cast<int>(Charset::G2_LATIN) == 10, "Charset changed, verify default maps.");
static_assert(static_cast<int>(Charset::G2_ARABIC) == 11, "Charset changed, verify default maps.");
static_assert(static_cast<int>(Charset::G2_CYRILLIC) == 12, "Charset changed, verify default maps.");
static_assert(static_cast<int>(Charset::G2_GREEK) == 13, "Charset changed, verify default maps.");
static_assert(static_cast<int>(Charset::G2_FARSI) == 14, "Charset changed, verify default maps.");
static_assert(static_cast<int>(Charset::G3_SMOOTH_MOSAIC) == 15, "Charset changed, verify default maps.");
static_assert(static_cast<int>(Charset::G3_SMOOTH_MOSAIC_SEPARATED) == 16, "Charset changed, verify default maps.");
static_assert(static_cast<int>(Charset::DRCS) == 17, "Charset changed, verify default maps.");
static_assert(static_cast<int>(Charset::LAST) == 17, "Charset changed, verify default maps.");

} // namespace ttxdecoder

#endif /* TTXDECODER_CHARSET_HPP */
