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

#include "ScteRawBitmap.hpp"
#include "ScteExceptions.hpp"

#include <utility>

namespace subttxrend
{
namespace scte
{

RawBitmap::RawBitmap(bool compressed, const uint8_t *data, std::size_t size)
{
    fillFields(data, size);
    setCompression(compressed);
}


const RawBitmap::Data& RawBitmap::getRawData() const
{
    return rawData;
}


void RawBitmap::setRawData(const uint8_t* data, std::size_t size)
{
    fillFields(data, size);
}


void RawBitmap::setRawData(const Data &rawData)
{
    this->rawData = rawData;
}

void RawBitmap::setRawData(Data &&rawData)
{
    this->rawData = std::move(rawData);
}

bool RawBitmap::isCompressed() const
{
    return compressed;
}


void RawBitmap::setCompression(bool compressed)
{
    this->compressed = compressed;
}


void RawBitmap::fillFields(const uint8_t* data, std::size_t size)
{
    if (!data || !size)
    {
        throw InvalidArgument("invalid data ptr or size");
    }

    rawData.assign(data, data+size);
}

} // namespace scte
} // namespace subttxrend
