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


namespace subttxrend
{
namespace scte
{

class BitStream
{
public:
    BitStream() = delete;
    BitStream(const std::vector<uint8_t>& stream);
    ~BitStream() = default;

    void shift(int shift);
    void operator<<(int shift) { this->shift(shift); }

    std::vector<uint8_t>& data();
    uint32_t data(std::size_t size) const;
    // returns max value if data() == 0
    uint32_t zeroAdjustedData(std::size_t size) const;
    std::size_t size() const;
    std::size_t length() const;
    std::size_t remainingBits() const;

    void reset();
    void setOffset(uint32_t offset);
    void setData(const std::vector<uint8_t> stream);

private:
    std::vector<uint8_t> bits;
    std::size_t bitPos;
    std::size_t bytePos;
};

} // namespace scte
} // namespace subttxtrend
