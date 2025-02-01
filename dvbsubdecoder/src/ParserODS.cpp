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


#include "ParserODS.hpp"

#include <subttxrend/common/Logger.hpp>

#include "ParserException.hpp"
#include "PesPacketReader.hpp"
#include "Database.hpp"
#include "ObjectParser.hpp"
#include "PixelWriter.hpp"
#include "Consts.hpp"

namespace dvbsubdecoder
{

namespace
{

subttxrend::common::Logger g_logger("DvbSubDecoder", "ParserODS");

} // namespace <anonmymous>

void ParserODS::parseObjectDataSegment(Database& database,
                                       PesPacketReader& reader)
{
    g_logger.trace("%s", __func__);

    if (database.getPage().getState() != Page::State::INCOMPLETE)
    {
        g_logger.trace("%s - page not currently being parsed", __func__);
        return;
    }

    const std::uint16_t objectId = reader.readUint16be();
    const std::uint8_t versionFlags = reader.readUint8();
    const std::uint8_t versionNumber = (versionFlags >> 4) & 0x0F;
    const std::uint8_t codingMethod = (versionFlags >> 2) & 0x03;
    const bool nonModifyingColourFlag = ((versionFlags & 0x02) != 0);

    g_logger.trace("%s - id: %04X - version: %X", __func__, objectId,
            versionNumber);
    g_logger.trace("%s - coding: %X - nmcf: %X", __func__, codingMethod,
            nonModifyingColourFlag);

    // no version checking as it is not stored

    if (codingMethod != ObjectCodingMethodBits::PIXELS)
    {
        g_logger.trace("%s - unsupported coding method, skipping", __func__);
        return;
    }

    auto regionCount = database.getRegionCount();
    for (std::size_t regionIndex = 0; regionIndex < regionCount; ++regionIndex)
    {
        auto region = database.getRegionByIndex(regionIndex);

        g_logger.trace("%s - looking for match in region %zu/%zu (id=%02u)",
                __func__, regionIndex + 1, regionCount, region->getId());

        for (auto objectRef = region->getFirstObject(); objectRef; objectRef =
                region->getNextObject(objectRef))
        {
            const auto& object = *objectRef;

            g_logger.trace("%s - region id=%02u, object=%04X", __func__,
                    region->getId(), object.m_objectId);

            if (object.m_objectId == objectId)
            {
                // passing copy of reader to share data between objects
                PesPacketReader pixelsReader(reader);

                parseObjectPixels(nonModifyingColourFlag, *region, object,
                        pixelsReader);
            }
        }
    }

    g_logger.trace("%s - finished", __func__);
}

void ParserODS::parseObjectPixels(bool nonModifyingColourFlag,
                                  Region& region,
                                  const ObjectInstance& object,
                                  PesPacketReader& reader)
{
    g_logger.trace("%s - nmcf=%X region=%u object=%u", __func__,
            nonModifyingColourFlag, region.getId(), object.m_objectId);

    auto topLength = reader.readUint16be();
    auto bottomLength = reader.readUint16be();

    // prepare readers for top & bottom data
    PesPacketReader topDataReader = PesPacketReader(reader, topLength);
    reader.skip(topLength);

    PesPacketReader bottomDataReader;

    if (bottomLength > 0)
    {
        bottomDataReader = PesPacketReader(reader, bottomLength);
        reader.skip(bottomLength);
    }
    else
    {
        bottomDataReader = topDataReader;
    }

    std::uint8_t depth = 0;
    switch (region.getDepth())
    {
    case RegionDepthBits::DEPTH_2BIT:
        depth = 2;
        break;
    case RegionDepthBits::DEPTH_4BIT:
        depth = 4;
        break;
    case RegionDepthBits::DEPTH_8BIT:
        depth = 8;
        break;
    }
    if (depth == 0)
    {
        throw ParserException("Unsupported region depth");
    }

    // prepare writers for top & bottom data
    PixelWriter topPixelWriter(nonModifyingColourFlag, depth,
            region.getPixmap(), object.m_positionX, object.m_positionY);
    PixelWriter bottomPixelWriter(nonModifyingColourFlag, depth,
            region.getPixmap(), object.m_positionX, object.m_positionY + 1);

    ObjectParser topParser(topDataReader, topPixelWriter);
    ObjectParser bottomParser(bottomDataReader, bottomPixelWriter);

    topParser.parse();
    bottomParser.parse();
}

} // namespace dvbsubdecoder
