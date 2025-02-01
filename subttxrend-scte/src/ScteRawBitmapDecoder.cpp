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

#include "ScteRawBitmapDecoder.hpp"
#include "ScteBitStream.hpp"
#include "ScteExceptions.hpp"

#include <subttxrend/common/Logger.hpp>


namespace subttxrend
{
namespace scte
{

namespace
{
common::Logger g_logger("Scte", "ScteRawBitmapDecoder");
} // namespace

RawBitmapDecoder::RawBitmapDecoder(RawBitmap &source, uint16_t width, uint16_t height)
    : in(source), out(width, height)
{
    if (!width || !height)
    {
        throw InvalidArgument("RawBitmapDecoder: width and height cennot be zero");
    }
}

void RawBitmapDecoder::decompress()
{
    if (!in.isCompressed()) return;

    try
    {
        decompressInternal();
    }
    catch (InvalidArgument &ex)
    {
        g_logger.error("bitmap finished prematurely during decompression");
    }

    in.setRawData(std::move(out.data));
    in.setCompression(false);
}

unsigned RawBitmapDecoder::fillTillEol()
{
    uint16_t to_fill = out.width - (out.data.size() % out.width);
    return fill(to_fill, false);
}

unsigned RawBitmapDecoder::fill(uint16_t size, bool value)
{
    out.data.insert(out.data.end(), size, value);
    return size;
}

void RawBitmapDecoder::decompressInternal()
{
    BitStream bs{in.getRawData()};

    unsigned bitmap_size = out.width * out.height;
    for (unsigned i = 0; i < bitmap_size;)
    {
        if (bs.data(1))
        {
            //8_on, 32_off
            bs << 1;
            i += fill(bs.zeroAdjustedData(3), true);
            bs << 3;
            i += fill(bs.zeroAdjustedData(5), false);
            bs << 5;
        }
        else if (bs.data(2))
        {
            //64_off
            bs << 2;
            i += fill(bs.zeroAdjustedData(6), false);
            bs << 6;
        }
        else if (bs.data(3))
        {
            //16_on
            bs << 3;
            i += fill(bs.zeroAdjustedData(4), true);
            bs << 4;
        }
        else
        {
            auto op = bs.data(5);
            bs << 5;
            if (op == 0x1)
            {
                // newline, fill remaining bits with zeroes
                i += fillTillEol();
            }
            else if (op != 0x0)
            {
                g_logger.error("invalid steering sequence: 0x%x", op);
            }
        }
    }
}

} // namespace scte
} // namespace subttxrend
