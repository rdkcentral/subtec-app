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


#ifndef SUBTTXREND_TTXT_CHARSET_HANDLER_HPP_
#define SUBTTXREND_TTXT_CHARSET_HANDLER_HPP_

#include <vector>

#include <ttxdecoder/Engine.hpp>
#include <subttxrend/gfx/FontStripMap.hpp>

namespace subttxrend
{
namespace ttxt
{

/**
 * Charset related operations handler.
 */
class CharsetHandler
{
public:
    /**
     * Constructor.
     */
    CharsetHandler();

    /**
     * Initializes the handler.
     *
     * @param engine
     *      Engine to get charset definitions.
     */
    void init(const ttxdecoder::Engine& engine);

    /**
     * Shuts down the handler.
     */
    void shutdown();

    /**
     * Returns mapping to be used by font.
     *
     * @return
     *      Created mapping.
     */
    const gfx::FontStripMap& getMapping() const;

    /**
     * Returns character (code + diacritic) mapped to UTF-16.
     *
     * @param character
     *      Character code (UTF-16).
     * @param diacriticProperty
     *      Diacritic property value.
     *
     * @return
     *      Mapped character.
     */
    static std::uint16_t getDiacriticCharacterCode(std::uint16_t character,
                                                   std::uint16_t diacriticProperty);

private:
    /** Created character to glyph index mapping. */
    gfx::FontStripMap m_mapping;
};

} // namespace ttxt
} // namespace subttxrend

#endif /*SUBTTXREND_TTXT_CHARSET_HANDLER_HPP_*/
