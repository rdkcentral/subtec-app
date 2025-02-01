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


namespace subttxrend
{
namespace cc
{

struct CcData
{
    enum CcType
    {
        CEA_608_FIELD_1,
        CEA_608_FIELD_2,
        DTVCC_CCP_DATA,
        DTVCC_CCP_START
    };

    bool isCcpStart() const
    {
        return (ccValid && (ccType == CcData::CcType::DTVCC_CCP_START));
    };

    bool isCcpData() const
    {
        return (ccValid && (ccType == CcData::CcType::DTVCC_CCP_DATA));
    };

    bool is608Data() const
    {
        return ((ccType == CcData::CcType::CEA_608_FIELD_1) || (ccType == CcData::CcType::CEA_608_FIELD_2));
    };

    bool isPadding() const
    {
        return !ccValid;
    };

    bool            ccValid;
    CcType          ccType;
    std::uint8_t    data1;
    std::uint8_t    data2;
};

class UserData
{
public:
    UserData();
    UserData(const std::uint8_t* data, std::size_t size);
    ~UserData() = default;

    void setUserData(const std::uint8_t* data, std::size_t size);
    const std::vector<std::unique_ptr<CcData>>& getCcData() const;

    bool isValid() const;

private:
    bool                                    processCcData;
    std::vector<std::unique_ptr<CcData>>    ccData;

    common::Logger                          logger;
};

} // namespace cc
} //namespace subttxrend
