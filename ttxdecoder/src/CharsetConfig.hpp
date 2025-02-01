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


#ifndef TTXDECODER_CHARSETCONFIG_HPP
#define TTXDECODER_CHARSETCONFIG_HPP

#include <cstdint>

#include "CharsetMaps.hpp"
#include "Types.hpp"

namespace ttxdecoder
{

/**
 * Charset configuration.
 *
 */
class CharsetConfig
{
public:

    /** Charset mode - utf or diactric. */
    enum class Mode
    {
        DIACTRIC, UTF
    };

    /**
     * Destructor.
     */
    virtual ~CharsetConfig()
    {
        // noop
    }

    /**
     * Charset mode getter.
     *
     * @return
     *      Charset mapping operational mode.
     */
    virtual Mode getMode() const = 0;

    /**
     * Primary national charset getter.
     *
     * @return
     *      Primary national charset.
     */
    virtual NationalCharset getPrimaryNationalCharset(std::uint8_t index = 0) const = 0;

    /**
     * Secondary national charset getter.
     *
     * @return
     *      Secondary national charset.
     */
    virtual NationalCharset getSecondaryNationalCharset(std::uint8_t index = 0) const = 0;

    /**
     * National subsets getter.
     *
     * @return
     *      National subsets table.
     */
    virtual const NationalSubsets& getNationalSubsets() const = 0;

    /**
     * Charset maps getter.
     *
     * @return
     *      Charset maps.
     */
    virtual const CharsetMaps& getCharsetMaps() const = 0;
};

} // namespace ttxdecoder

#endif /* TTXDECODER_CHARSETCONFIG_HPP */
