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
#include "CcUserData.hpp"


namespace subttxrend
{
namespace cc
{

class CaptionChannelPacket
{
public:
    CaptionChannelPacket();
    CaptionChannelPacket(const CcData& ccData);
    ~CaptionChannelPacket() = default;

    void addCcData(const CcData& ccData);
    const std::vector<std::uint8_t>& getCcpData() const;

    // Returns CCP size including header byte. Data size is 1 byte smaller
    std::size_t getSize() const;
    bool isFull() const;
    void reset();

private:
    bool                        started;
    std::uint32_t               seqNo;
    std::size_t                 size;
    std::vector<std::uint8_t>   ccpData;

    common::Logger              logger;

    static constexpr auto       CCP_SIZE_MAX = 128;
};

} // namespace cc
} //namespace subttxrend
