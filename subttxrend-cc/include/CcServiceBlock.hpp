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

#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <subttxrend/common/Logger.hpp>
#include "CcCaptionChannelPacket.hpp"


namespace subttxrend
{
namespace cc
{

class ServiceBlock
{
public:
    ServiceBlock();
    ServiceBlock(const std::vector<std::uint8_t>& ccpData, std::size_t& offset);
    ServiceBlock(const ServiceBlock &) = default;
    ServiceBlock(ServiceBlock &&) = default;
    ServiceBlock & operator=(const ServiceBlock&) = default;
    ~ServiceBlock() = default;

    void setCcpData(const std::vector<std::uint8_t>& ccpData, std::size_t& offset);
    // setRawData is used by unit tests
    void setRawData(const std::vector<std::uint8_t>& data);
    const std::vector<std::uint8_t>& getSbData() const;
    void eraseTo(size_t pos);

    /* Returns SB size without header byte(s).
     */
    std::size_t getBlockSize() const;
    bool isExtended() const;
    std::uint32_t getServiceNumber() const;

private:
    std::uint32_t               serviceNo;
    std::size_t                 blockSize;
    bool                        extended;
    std::vector<std::uint8_t>   sbData;
};

} // namespace cc
} //namespace subttxrend
