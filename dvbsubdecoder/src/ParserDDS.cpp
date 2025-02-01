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


#include "ParserDDS.hpp"

#include <subttxrend/common/Logger.hpp>

#include "ParserException.hpp"
#include "PesPacketReader.hpp"
#include "Database.hpp"

namespace dvbsubdecoder
{

namespace
{

subttxrend::common::Logger g_logger("DvbSubDecoder", "ParserDDS");

} // namespace <anonmymous>

void ParserDDS::parseDisplayDefinitionSegment(Database& database,
                                              PesPacketReader& reader)
{
    g_logger.trace("%s", __func__);

    // == parse header ==

    const std::uint8_t versionFlags = reader.readUint8();
    const std::uint8_t versionNumber = (versionFlags >> 4) & 0x0F;
    const bool windowFlag = ((versionFlags & 0x08) != 0);

    g_logger.trace("%s - version: %X", __func__, versionNumber);

    // == check version ==

    auto& display = database.getParsedDisplay();

    if (display.getVersion() == versionNumber)
    {
        g_logger.trace("%s - same version, skipping", __func__);
        return;
    }

    // == parse data ==

    std::uint16_t displayWidth = reader.readUint16be();
    std::uint16_t displayHeight = reader.readUint16be();

    std::uint16_t windowMinimumX = 0;
    std::uint16_t windowMaximumX = displayWidth;
    std::uint16_t windowMinimumY = 0;
    std::uint16_t windowMaximumY = displayHeight;

    if (windowFlag)
    {
        windowMinimumX = reader.readUint16be();
        windowMaximumX = reader.readUint16be();
        windowMinimumY = reader.readUint16be();
        windowMaximumY = reader.readUint16be();
    }

    g_logger.trace("%s - display: %u,%u-%u,%u", __func__, 0, 0, displayWidth,
            displayHeight);
    g_logger.trace("%s - window:  %u,%u-%u,%u", __func__, windowMinimumX,
            windowMinimumY, windowMaximumX, windowMaximumY);

    // == check ==

    static const std::int32_t MAX_WIDTH = 1919;
    static const std::int32_t MAX_HEIGHT = 1079;

    if ((displayWidth > MAX_WIDTH) || (displayHeight > MAX_HEIGHT))
    {
        displayWidth = windowMaximumX - windowMinimumX;
        displayHeight = windowMaximumY - windowMinimumY;
        g_logger.warning("%s - updated display to (%u, %u)", __func__, displayWidth, displayHeight);
    }

    if ((displayWidth > MAX_WIDTH) || (displayHeight > MAX_HEIGHT))
    {
        throw ParserException("Maximum display size exceeded");
    }
    if ((windowMinimumX > windowMaximumX) || (windowMinimumY > windowMaximumY))
    {
        throw ParserException("Invalid window definition");
    }
    if ((windowMinimumX > displayWidth) || (windowMaximumX > displayWidth)
            || (windowMinimumY > displayHeight) || (windowMaximumY > displayHeight))
    {
        throw ParserException("Window outside display");
    }

    // == apply ==

    display.set(versionNumber,
    { 0, 0, displayWidth + 1, displayHeight + 1},
    { windowMinimumX, windowMinimumY, windowMaximumX + 1, windowMaximumY + 1});
}

} // namespace dvbsubdecoder
