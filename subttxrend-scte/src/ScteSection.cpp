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

#include <string>
#include "ScteSection.hpp"

#include <subttxrend/common/Logger.hpp>

extern "C"
{
#include "zlib.h"
}


namespace subttxrend
{
namespace scte
{

common::Logger g_logger("Scte", "Section");


Section::Section(const uint8_t *data, uint32_t size)
{
    if (!data || !size)
    {
        throw InvalidArgument("invalid data ptr or size");
    }

    table_id = data[0];

    if (table_id != 0xC6)
    {
        throw ParseError("Invalid section_id: " + std::to_string(data[0]));
    }

    section_length = ((data[1] & 0x0F) << 8) | data[2];

    if (static_cast<uint32_t>(section_length) + 3 > size)
    {
        throw InvalidArgument("data size too small");
    }
    segmentation_overlay_included = data[3] & 0x40;

    auto protocol_version = data[3] & 0x3F;

    if (protocol_version != 0)
    {
        g_logger.warning("%s unsupported protocol_version: %d", __LOGGER_FUNC__, protocol_version);
    }

    unsigned first_pos = 0;
    uint16_t segment_size = 0;
    if (segmentation_overlay_included)
    {
        segmentation_overlay.table_extension = (data[4] << 8) | data[5];
        segmentation_overlay.last_segment_number = (data[6] << 4) | (data[7] >> 4);
        segmentation_overlay.segment_number =  ((data[7] & 0x0F) << 8) | data[8];
        first_pos = 9;
        segment_size = section_length - 1 - 5 - 4;
    }
    else
    {
        first_pos = 4;
        segment_size = section_length - 1 - 4;
    }

    if (section_length == 0 || section_length < 10)
    {
        throw InvalidArgument("section length is invalid");
    }
    pdata.insert(pdata.end(), &data[first_pos], &data[first_pos] + segment_size);

    const uint8_t *crc_ptr = &data[first_pos + segment_size];
    crc32 = (crc_ptr[0] << 24) | (crc_ptr[1] << 16) | (crc_ptr[2] << 8) | crc_ptr[3];

    uint32_t calculated_crc = ::crc32(0, reinterpret_cast<const unsigned char*>(data), section_length);

    is_valid = crc32 == calculated_crc || crc32 == 0x0;
}

Section::SegmentationOverlay Section::segmentationOverlay() const
{
    if (!segmentation_overlay_included)
    {
        throw InvalidOperation("Segmentation overlay requested for message with no overlay");
    }
    return segmentation_overlay;
}

} // namespace scte
} // namespace subttxrend
