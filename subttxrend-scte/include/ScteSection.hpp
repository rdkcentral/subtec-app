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
#include <vector>
#include "ScteExceptions.hpp"


namespace subttxrend
{
namespace scte
{

class Section
{
    struct SegmentationOverlay
    {
        uint16_t table_extension;
        uint16_t last_segment_number;
        uint16_t segment_number;
    } segmentation_overlay;


public:
    Section(const uint8_t *data, uint32_t size);
    uint8_t tableId() const noexcept { return table_id; }
    uint16_t sectionLength() const noexcept { return section_length; }
    bool isSegmented() const noexcept { return segmentation_overlay_included; }
    const std::vector<uint8_t>& data() const noexcept { return pdata; }
    SegmentationOverlay segmentationOverlay() const;
    bool isValid() const noexcept { return is_valid; }
    uint32_t getCrc32() const noexcept { return crc32; }

private:
    uint8_t table_id;
    uint16_t section_length;
    bool segmentation_overlay_included;
    std::vector<uint8_t> pdata;
    uint32_t crc32;
    bool is_valid;
};

} // namespace scte
} // namespace subttxrend
