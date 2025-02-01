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


#ifndef TTXDECODER_NATIONALOPTIONS_HPP
#define TTXDECODER_NATIONALOPTIONS_HPP

#include <cassert>
#include <cstdint>

#include "Types.hpp"
#include "DefaultCharsets.hpp"

namespace ttxdecoder
{

/**
 * National options subset container.
 */
class NationalOptions
{
public:

    /**
     * Constructor.
     */
    NationalOptions()
    {
        m_array =
        {{
                NationalCharset::ENGLISH,
                NationalCharset::GERMAN,
                NationalCharset::SWEDISH,
                NationalCharset::ITALIAN,
                NationalCharset::FRENCH,
                NationalCharset::SPANISH,
                NationalCharset::CZECH,
                NationalCharset::ENGLISH
        }};
    }

    /**
     * Sets value in national charset table.
     *
     * @param index
     *      Index in national options table.
     * @param charset
     *      New national charset value.
     */
    void setOption(std::uint8_t index,
            NationalCharset charset)
    {
        assert(index < OPTIONS_COUNT);
        m_array[index] = charset;
    }

    /**
     * Gets value from national charset table.
     *
     * @param index
     *      Index in national options table.
     * @return
     *      Current national charset value.
     */
    NationalCharset getOption(std::uint8_t index) const
    {
        assert(index < OPTIONS_COUNT);
        return m_array[index];
    }

private:

    /** Number of national options. */
    static constexpr std::size_t OPTIONS_COUNT = 8;

    /** Helper class for readability. */
    using OptionsArray = std::array<NationalCharset, OPTIONS_COUNT>;

    /** National charset table. */
    OptionsArray m_array;

};

} // namespace ttxdecoder

#endif /* TTXDECODER_NATIONALOPTIONS_HPP */
