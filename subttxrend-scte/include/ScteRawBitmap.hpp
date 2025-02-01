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

class RawBitmap
{
public:
    RawBitmap() = default;
    RawBitmap(bool compressed, const uint8_t* data, std::size_t size);
    ~RawBitmap() = default;
    RawBitmap(RawBitmap &) = default;
    RawBitmap(RawBitmap &&) = default;
    RawBitmap& operator=(const RawBitmap &) = default;

    using Data = std::vector<uint8_t>;
    const Data& getRawData() const;
    void setRawData(const uint8_t *data, std::size_t size);
    void setRawData(const Data &rawData);
    void setRawData(Data &&rawData);

    bool isCompressed() const;
    void setCompression(bool compressed);

private:
    void fillFields(const uint8_t *data, std::size_t size);

    /* For compressed bitmap, format is defined in SCTE-27 standard
     * in table 5.8.
     *
     * Uncompressed bitmap will have one bit per pixel format
     */
    Data rawData;
    bool compressed;
};

} // namespace scte
} // namespace subttxrend
