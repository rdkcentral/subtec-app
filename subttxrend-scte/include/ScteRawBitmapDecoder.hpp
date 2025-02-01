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

#include "ScteRawBitmap.hpp"

#include <cstdint>
#include <cstddef>

namespace subttxrend
{
namespace scte
{

class RawBitmapDecoder
{
public:
    RawBitmapDecoder(RawBitmap &source, uint16_t width, uint16_t height);
    void decompress();

private:
    struct OutputData
    {
        RawBitmap::Data data;
        const uint16_t width;
        const uint16_t height;

        OutputData(uint16_t width, uint16_t height)
            : width(width), height(height)
        {
            data.reserve(width * height);
        }
    };

    void decompressInternal();
    unsigned fillTillEol();
    unsigned fill(uint16_t size, bool value);

private:
    RawBitmap &in;
    OutputData out;

};

} // namespace scte
} // namespace subttxrend
