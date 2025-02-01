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

#include "ScteTable.hpp"
#include "ScteExceptions.hpp"

#include <subttxrend/common/Logger.hpp>

namespace subttxrend
{
namespace scte
{

namespace
{
common::Logger g_logger("Scte", "ScteTable");
}

ScteTable::ScteTable(TableID id, const uint8_t *data, std::size_t size)
{
    tableId = id;
    fillFields(data, size);
}


TableID ScteTable::getId() const
{
    return tableId;
}


std::string ScteTable::getLanguageCode() const
{
    return languageCode;
}


bool ScteTable::preClearDisplay() const
{
    return clearDisplay;
}


bool ScteTable::isImmediate() const
{
    return immediate;
}


DisplayStandard ScteTable::getDisplayStandard() const
{
    return displayStandard;
}


uint32_t ScteTable::getPTS() const
{
    return displayInPTS;
}


SubtitleType ScteTable::getSubtitleType() const
{
    return subtitleType;
}


uint16_t ScteTable::getDisplayDuration() const
{
    return displayDuration;
}


const SimpleBitmap& ScteTable::getSimpleBitmap() const
{
    return bitmap;
}

std::uint32_t ScteTable::getEndPts() const
{
    uint32_t frame_duration = 90000/60;

    switch(getDisplayStandard())
    {
      case DisplayStandard::DS_720_480_30:
        frame_duration = 90000/30;
        break;
      case DisplayStandard::DS_720_576_25:
        frame_duration = 90000/25;
        break;
      case DisplayStandard::DS_1280_720_60:
      case DisplayStandard::DS_1920_1080_60:
        frame_duration = 90000/60;
        break;
    }
    return (getPTS() + frame_duration * getDisplayDuration());
}

void ScteTable::setTableId(TableID id)
{
    tableId = id;
}


void ScteTable::setTableData(uint8_t* data, std::size_t size)
{
    fillFields(data, size);
}


void ScteTable::fillFields(const uint8_t* data, std::size_t size)
{
    if (!data || size < 12)
    {
        throw InvalidArgument("invalid data ptr or size");
    }

    languageCode.assign(reinterpret_cast<const char*>(data), 3);
    clearDisplay = (data[3] & 0x80)? true : false;
    immediate = (data[3] & 0x40)? true : false;
    displayStandard = static_cast<DisplayStandard>(data[3] & 0x1F);
    displayInPTS = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];
    subtitleType = static_cast<SubtitleType>(data[8] >> 4);
    displayDuration = ((data[8] & 0x07) << 8) | data[9];

    g_logger.debug("%s lng:%s clearDisplay:%d immediate:%d displayStandard:%d displayInPts:0x%08x subType:%d duration:%u",
        __LOGGER_FUNC__,
        languageCode.c_str(),
        clearDisplay,
        immediate,
        static_cast<int>(displayStandard),
        displayInPTS,
        static_cast<int>(subtitleType),
        displayDuration);


    if(subtitleType == SubtitleType::SIMPLE_BITMAP)
    {
        uint16_t bitmapLen = (data[10] << 8) | data[11];
        if (bitmapLen > size - 12)
        {
            throw ParseError("ScteTable: extracted bitmap length larger than data size");
        }
        if (bitmapLen == 0)
        {
            throw ParseError("ScteTable: extracted bitmap length is zero");
        }
        bitmap.setBitmap(data+12, bitmapLen);
        bitmap.decompress();
    }
}

} // namespace scte
} // namespace subttxtrend
