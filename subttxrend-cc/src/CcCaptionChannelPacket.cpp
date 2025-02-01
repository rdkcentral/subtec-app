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

#include "CcCaptionChannelPacket.hpp"
#include "CcExceptions.hpp"


namespace subttxrend
{
namespace cc
{

CaptionChannelPacket::CaptionChannelPacket()
    : started(false),
      seqNo(0),
      size(0),
      logger("ClosedCaptions", "CcCaptionChannelPacket")
{

}

CaptionChannelPacket::CaptionChannelPacket(const CcData& ccData)
    : CaptionChannelPacket()
{
    addCcData(ccData);
}

void CaptionChannelPacket::addCcData(const CcData& ccData)
{
    if(!ccData.ccValid)
    {
        throw InvalidOperation("Cannot add invalid data to CCP");
    }

    if(started && (ccData.ccType != CcData::CcType::DTVCC_CCP_DATA))
    {
        throw InvalidOperation("Cannot start already started CCP");
    }

    if(!started && (ccData.ccType != CcData::CcType::DTVCC_CCP_START))
    {
        throw InvalidOperation("Cannot add data to CCP that isn't started");
    }

    switch(ccData.ccType)
    {
        case CcData::CcType::DTVCC_CCP_START:
            seqNo = (ccData.data1 & 0xC0) >> 6;
            size = ccData.data1 & 0x3F;
            size = (size == 0)? CCP_SIZE_MAX  : size * 2;
            started = true;

            logger.debug("%s CCP seqNo %d, size %zu", __func__, seqNo, size);
            break;
        case CcData::CcType::DTVCC_CCP_DATA:
            /* Transmitted size holds the number of bytes in CCP
             * including header byte. Vector stores only data
             * without header. Its size is 1 byte smaller than
             * CPP size.
             */
            if(size < ccpData.size() + 3)
            {
                throw InvalidOperation("Cannot add more data than stated in CCP header");
            }

            ccpData.push_back(ccData.data1);
            break;
        /* Remaining 2 CEA-608 types doesn't have to be handled here
         * as they are rejected in parameters check stage.
         */
        default:
            break;
    }

    ccpData.push_back(ccData.data2);
}

const std::vector<std::uint8_t>& CaptionChannelPacket::getCcpData() const
{
    return ccpData;
}

std::size_t CaptionChannelPacket::getSize() const
{
    return size;
}

bool CaptionChannelPacket::isFull() const
{
    return (size == (ccpData.size() + 1));
}

void CaptionChannelPacket::reset()
{
    ccpData.clear();
    started = false;
    seqNo = 0;
    size = 0;
}

} //namespace cc
} //namespace subttxrend
