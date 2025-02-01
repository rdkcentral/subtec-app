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

#include "CcServiceBlock.hpp"
#include "CcExceptions.hpp"


namespace subttxrend
{
namespace cc
{

ServiceBlock::ServiceBlock()
    : serviceNo(0),
      blockSize(0),
      extended(false)
{

}

ServiceBlock::ServiceBlock(const std::vector<std::uint8_t>& ccpData, std::size_t &offset)
    : ServiceBlock()
{
    setCcpData(ccpData, offset);
}

void ServiceBlock::setCcpData(const std::vector<std::uint8_t>& ccpData, std::size_t &offset)
{
    serviceNo = (ccpData[offset] & 0xE0) >> 5;
    blockSize = ccpData[offset] & 0x1F;
    std::size_t headerSize = 1;

    if (serviceNo == 7)
    {
        headerSize = 2;
        extended = true;
    }

    // make sure we don't go out of band
    if (offset + headerSize + blockSize > ccpData.size())
    {
        serviceNo = 0;
        blockSize = 0;
        return;
    }

    if (extended)
    {
        serviceNo = ccpData[offset+1] & 0x3F;
    }

    sbData.assign(&ccpData[offset+headerSize], &ccpData[offset+headerSize+blockSize]);
    offset += (blockSize + headerSize); // advance offset to point after this service block
}

void ServiceBlock::setRawData(const std::vector<uint8_t> &data)
{
    serviceNo = 1;
    blockSize = data.size();
    sbData = data;
}

const std::vector<std::uint8_t>& ServiceBlock::getSbData() const
{
    return sbData;
}

void ServiceBlock::eraseTo(size_t pos)
{
    sbData.erase(sbData.begin(), sbData.begin() + pos);
    blockSize = sbData.size();
}

std::size_t ServiceBlock::getBlockSize() const
{
    return blockSize;
}

bool ServiceBlock::isExtended() const
{
    return extended;
}

std::uint32_t ServiceBlock::getServiceNumber() const
{
    return serviceNo;
}

} //namespace cc
} //namespace subttxrend
