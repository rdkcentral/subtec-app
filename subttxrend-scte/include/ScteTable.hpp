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

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include "ScteSimpleBitmap.hpp"


namespace subttxrend
{
namespace scte
{

enum class DisplayStandard
{
    DS_720_480_30,
    DS_720_576_25,
    DS_1280_720_60,
    DS_1920_1080_60
};

enum TableID
{
    TABLE_ID_SCTE_SUB = 0xC6
};

enum class SubtitleType
{
    SIMPLE_BITMAP = 0x01
};

class ScteTable
{
public:
    ScteTable() = delete;
    ScteTable(TableID id, const uint8_t* data, std::size_t size);
    ~ScteTable() = default;

    TableID getId() const;
    std::string getLanguageCode() const;
    bool preClearDisplay() const;
    bool isImmediate() const;
    DisplayStandard getDisplayStandard() const;
    uint32_t getPTS() const;
    SubtitleType getSubtitleType() const;
    uint16_t getDisplayDuration() const;
    const SimpleBitmap& getSimpleBitmap() const;
    uint32_t getEndPts() const;

    void setTableId(TableID id);
    void setTableData(uint8_t* data, std::size_t size);

private:
    void fillFields(const uint8_t *data, std::size_t size);

    TableID         tableId;
    std::string     languageCode;
    bool            clearDisplay;
    bool            immediate;
    DisplayStandard displayStandard;
    uint32_t        displayInPTS;
    SubtitleType    subtitleType;
    uint16_t        displayDuration;
    SimpleBitmap    bitmap;
};

using ScteTablePtr = std::unique_ptr<ScteTable>;

} // namespace scte
} // namespace subttxtrend
