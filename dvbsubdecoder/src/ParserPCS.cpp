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


#include "ParserPCS.hpp"

#include <subttxrend/common/Logger.hpp>

#include "ParserException.hpp"
#include "PesPacketReader.hpp"
#include "Database.hpp"
#include "Consts.hpp"

namespace dvbsubdecoder
{

namespace
{

subttxrend::common::Logger g_logger("DvbSubDecoder", "ParserPCS");

} // namespace <anonmymous>

void ParserPCS::parsePageCompositionSegment(Database& database,
                                            PesPacketReader& reader)
{
    g_logger.trace("%s", __func__);

    // == parse header ==

    const std::uint8_t timeout = reader.readUint8();
    const std::uint8_t versionFlags = reader.readUint8();
    const std::uint8_t versionNumber = (versionFlags >> 4) & 0x0F;
    const std::uint8_t pageState = (versionFlags >> 2) & 0x03;

    g_logger.trace("%s - version: %X", __func__, versionNumber);
    g_logger.trace("%s - timeout: %u, state: %X", __func__, timeout, pageState);

    // == check version ==

    auto& page = database.getPage();

    if (page.getVersion() == versionNumber)
    {
        g_logger.trace("%s - same version, skipping", __func__);
        return;
    }

    // == initial processing ==

    switch(pageState)
    {
    case PageStateBits::MODE_CHANGE:
        // if state is mode change - simply reset everything
        // as it contains all the data needed
        database.epochReset();
        break;

    case PageStateBits::ACQUISITION_POINT:
        if ((page.getState() == Page::State::COMPLETE) ||
                (page.getState() == Page::State::TIMEDOUT))
        {
            // if previous page was complete then this one could
            // be thread as a normal case, nothing special to do

            // NOTE: Dvb specification expects nothing to be done
            // in this case. The following line is a workaround for
            // content providers that always broadcast page composition
            // segments as acquisition points with some pages being
            // empty.
            database.epochReset();
        }
        else
        {
            // if previous state was not complete or already timeouted
            // then epoch reset is a safe way to handle the page
            // as all data needed are transimitted in acquisition points
            database.epochReset();
        }
        break;

    case PageStateBits::NORMAL_CASE:
        if ((page.getState() == Page::State::COMPLETE) ||
                (page.getState() == Page::State::TIMEDOUT))
        {
            // normal case, nothing special to do
        }
        else
        {
            // no complete page was catched before, so this cannot simply
            // update the current page
            g_logger.trace(
                    "%s - complete display set needed, skipping (state=%u)",
                    __func__, pageState);
            return;
        }
        break;

    case PageStateBits::RESERVED:
    default:
        throw ParserException("Invalid page state");
    }

    // == parse data ==

    page.startParsing(versionNumber, database.getStatus().getLastPts(),
            timeout);

    while (reader.getBytesLeft() > 0)
    {
        const std::uint8_t regionId = reader.readUint8();

        reader.skip(1); // reserved

        const std::uint16_t regionAddressX = reader.readUint16be();
        const std::uint16_t regionAddressY = reader.readUint16be();

        g_logger.trace("%s - region: id=%02X pos=%u,%u", __func__, regionId,
                regionAddressX, regionAddressY);

        if (!page.addRegion(regionId, regionAddressX, regionAddressY))
        {
            g_logger.info("%s - not enough space to store region", __func__);
        }
    }
}

} // namespace dvbsubdecoder
