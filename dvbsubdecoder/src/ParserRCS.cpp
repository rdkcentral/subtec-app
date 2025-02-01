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


#include "ParserRCS.hpp"

#include <subttxrend/common/Logger.hpp>

#include "ParserException.hpp"
#include "PesPacketReader.hpp"
#include "Database.hpp"
#include "Consts.hpp"

namespace dvbsubdecoder
{

namespace
{

subttxrend::common::Logger g_logger("DvbSubDecoder", "ParserRCS");

} // namespace <anonmymous>

void ParserRCS::parseRegionCompositionSegment(Database& database,
                                              PesPacketReader& reader)
{
    g_logger.trace("%s", __func__);

    // == initial check ==

    if (database.getPage().getState() != Page::State::INCOMPLETE)
    {
        g_logger.trace("%s - page not currently being parsed", __func__);
        return;
    }

    // == parse header ==

    const std::uint8_t regionId = reader.readUint8();
    const std::uint8_t versionFlags = reader.readUint8();
    const std::uint8_t versionNumber = (versionFlags >> 4) & 0x0F;
    const bool fillFlag = ((versionFlags & 0x08) != 0);

    g_logger.trace("%s - id: %02X", __func__, regionId);
    g_logger.trace("%s - version: %X", __func__, versionNumber);
    g_logger.trace("%s - fill: %X", __func__, fillFlag);

    // == check version ==

    auto region = database.getRegionById(regionId);
    if (region)
    {
        if (region->getVersion() == versionNumber)
        {
            g_logger.trace("%s - same version, skipping", __func__);
            return;
        }
    }
    else
    {
        if (!database.canAddRegion())
        {
            g_logger.warning("Cannot add region to database, skipping");
            return;
        }
    }

    // == parse data (part 1) ==

    const std::uint16_t width = reader.readUint16be();
    const std::uint16_t height = reader.readUint16be();

    g_logger.trace("%s - size: %u x %u", __func__, width, height);

    const std::uint8_t regionFlags = reader.readUint8();
    const std::uint8_t compatibilityLevel = (regionFlags >> 5) & 0x07;
    const std::uint8_t depth = (regionFlags >> 2) & 0x03;

    g_logger.trace("%s - compatibility: %X  depth: %X", __func__,
            compatibilityLevel, depth);

    const std::uint8_t clutId = reader.readUint8();

    g_logger.trace("%s - clut: %02X", __func__, clutId);

    // == add/verify ==

    if (!region)
    {
        // check if compatibilityLevel is valid
        if ((compatibilityLevel != RegionDepthBits::DEPTH_2BIT)
                && (compatibilityLevel != RegionDepthBits::DEPTH_4BIT)
                && (compatibilityLevel != RegionDepthBits::DEPTH_8BIT))
        {
            throw ParserException("invalid compatibility level");
        }

        // check if depth is valid
        if ((depth != RegionDepthBits::DEPTH_2BIT)
                && (depth != RegionDepthBits::DEPTH_4BIT)
                && (depth != RegionDepthBits::DEPTH_8BIT))
        {
            throw ParserException("invalid compatibility level");
        }

        region = database.addRegionAndClut(regionId, width, height,
                compatibilityLevel, depth, clutId);
        if (!region)
        {
            g_logger.warning("%s - cannot add region, skipping", __func__);
            return;
        }
    }
    else
    {
        if ((region->getWidth() != width) || (region->getHeight() != height)
                || (region->getDepth() != depth)
                || (region->getCompatibilityLevel() != compatibilityLevel)
                || (region->getClutId() != clutId))
        {
            throw ParserException("Region parameters changed");
        }
    }

    // == parse data ==

    const std::uint8_t bgPixel8 = reader.readUint8();
    const std::uint8_t bgPixelFlags = reader.readUint8();
    const std::uint8_t bgPixel4 = (bgPixelFlags >> 4) & 0x0F;
    const std::uint8_t bgPixel2 = (bgPixelFlags >> 2) & 0x03;

    g_logger.trace("%s - bgpixel: 8bit=%02X 4bit=%02X 2bit=%02X", __func__,
            bgPixel8, bgPixel4, bgPixel2);

    if (depth == RegionDepthBits::DEPTH_2BIT)
    {
        region->setBackgroundIndex(bgPixel2);
    }
    else if (depth == RegionDepthBits::DEPTH_4BIT)
    {
        region->setBackgroundIndex(bgPixel4);
    }
    else if (depth == RegionDepthBits::DEPTH_8BIT)
    {
        region->setBackgroundIndex(bgPixel8);
    }
    else
    {
        throw ParserException("unknown depth");
    }

    // == parse object references
    database.removeRegionObjects(region);

    while (reader.getBytesLeft() > 0)
    {
        const std::uint16_t objectId = reader.readUint16be();
        const std::uint16_t objectData1 = reader.readUint16be();
        const std::uint16_t objectData2 = reader.readUint16be();

        const std::uint8_t objectType = (objectData1 >> 14) & 0x03;
        const std::uint8_t objectProviderFlag = (objectData1 >> 12) & 0x03;
        const std::uint16_t objectPosX = objectData1 & 0x0FFF;
        const std::uint16_t objectPosY = objectData2 & 0x0FFF;

        g_logger.trace("%s - object: id=%04X type=%X prflag=%X", __func__,
                objectId, objectType, objectProviderFlag);
        g_logger.trace("%s - object: position=%u,%u", __func__, objectPosX,
                objectPosY);

        std::uint8_t fgPixelCode = 0;
        std::uint8_t bgPixelCode = 0;

        if ((objectType == RegionObjectTypeBits::BASIC_CHARACTER)
                || (objectType == RegionObjectTypeBits::COMPOSITE_STRING))
        {
            fgPixelCode = reader.readUint8();
            bgPixelCode = reader.readUint8();
        }

        g_logger.trace("%s - object: colors fg=%02X bg=%02X", __func__,
                fgPixelCode, bgPixelCode);

        if (objectType != RegionObjectTypeBits::BASIC_BITMAP)
        {
            throw ParserException("Unsupported object type");
        }

        if (objectProviderFlag != RegionObjectProviderBits::SUBTITLING_STREAM)
        {
            throw ParserException("Unsupported object provider");
        }

        if (!database.addRegionObject(region, objectId, objectPosX, objectPosY))
        {
            throw ParserException("Too many region objects in stream");
        }

        g_logger.trace("%s - object id=%04X added", __func__, objectId);
    }

    // == fill if requested ==

    if (fillFlag)
    {
        auto& pixmap = region->getPixmap();
        pixmap.clear(region->getBackgroundIndex());
    }

    // == finish ==

    region->setVersion(versionNumber);
}

} // namespace dvbsubdecoder
