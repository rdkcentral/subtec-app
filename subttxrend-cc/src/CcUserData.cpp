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
*
* Copyright 2023 Comcast Cable Communications Management, LLC
* Licensed under the Apache License, Version 2.0
*****************************************************************************/

#include "CcUserData.hpp"
#include "CcExceptions.hpp"
#include <iostream>
#include <iomanip>

#define ATSC_CC_POC

namespace subttxrend
{
namespace cc
{

UserData::UserData()
    : processCcData(false),
      logger("ClosedCaptions", "CcData")
{

}

UserData::UserData(const std::uint8_t* data, std::size_t size)
    : UserData()
{
    setUserData(data, size);
}

#ifdef ATSC_CC_POC

void UserData::setUserData(const std::uint8_t* data, std::size_t size)
{
    if((data == nullptr) || (size == 0))
    {
        throw InvalidArgument("Invalid data pointer or data size");
    }

    processCcData = true; // hmm, why not ? :)
    std::uint32_t ccCount  = size/3;

    logger.trace("%s raw data:", __func__);

    for(std::uint32_t i=0; i<ccCount; ++i)
    {
        CcData* pair = new CcData();

        pair->ccValid = true; //invalid cc triplets are skipped in CC HAL
        pair->ccType = static_cast<CcData::CcType>(data[i*3] & 0x03);
        pair->data1 = data[i*3+1];
        pair->data2 = data[i*3+2];

        logger.trace("0x%02x 0x%02x 0x%02x", data[i*3], data[i*3+1], data[i*3+2]);

        ccData.emplace_back(std::move(pair));
    }
}

#else

void UserData::setUserData(const std::uint8_t* data, std::size_t size)
{
    std::uint32_t start = 0;
    std::uint32_t zeros = 0;
    std::uint32_t offset;
    std::uint32_t ccCount = 0;

    if((data == nullptr) || (size == 0))
    {
        throw InvalidArgument("Invalid data pointer or data size");
    }

    /* NOTE: These offsets are Broadcom specific and DO NOT
     * correspond to MPEG-2 start code nor ITU-T T.35 SEI
     * formats.
     */
    for(offset=0; offset<size; offset++)
    {
        if(data[offset] == 0x00)
        {
            zeros++;
        }
        else if((data[offset] == 0x01) && (zeros >= 2))
        {
            break;
        }
        else
        {
            zeros = 0;
        }
    }

    if(offset == size)
    {
        // PES user data start code not found
        return;
    }

    // MPEG-2 start code && user data type 0x03
    if((data[offset+1] == 0xB2) && (data[offset+6] == 0x03))
    {
        start = offset + 7;
        logger.trace("%s MPEG2", __func__);
    }
    // H.264 SEI & payload type & ATSC1 user data
    else if((data[offset+1] == 0x06) && (data[offset+2] == 0x04) && (data[offset+11] == 0x03))
    {
        start = offset + 12;
        logger.trace("%s H.264", __func__);
    }
    // H.265 SEI & payload type & ATSC1 user data
    else if((data[offset+1] == 0x4E) && (data[offset+2] == 0x04) && (data[offset+11] == 0x03))
    {
        start = offset + 12;
        logger.trace("%s H.265", __func__);
    }
    else
    {
        logger.trace("%s Unknown format", __func__);
        return;
    }

    processCcData = (data[start] & 0x40)? true : false;
    ccCount = data[start] & 0x1F;
    start += 2;

    logger.trace("%s raw data:", __func__);

    for(std::uint32_t i=0; i<ccCount; i++)
    {
        CcData* pair = new CcData;

        pair->ccValid = (data[start+i*3] & 0x04)? true : false;
        pair->ccType = static_cast<CcData::CcType>(data[start+i*3] & 0x03);
        pair->data1 = data[start+i*3+1];
        pair->data2 = data[start+i*3+2];

        logger.trace("0x%02x 0x%02x 0x%02x", data[start+i*3], data[start+i*3+1], data[start+i*3+2]);

        ccData.emplace_back(std::move(pair));
    }
}
#endif

const std::vector<std::unique_ptr<CcData>>& UserData::getCcData() const
{
    return ccData;
}

bool UserData::isValid() const
{
    return processCcData;
}

} //namespace cc
} //namespace subttxrend




