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

#include "ScteBitStream.hpp"
#include "ScteExceptions.hpp"

namespace subttxrend
{
namespace scte
{

BitStream::BitStream(const std::vector<uint8_t>& stream)
{
    setData(stream);
}


void BitStream::shift(int shift)
{
    if(((bitPos + shift) <= length()) && (((int) bitPos + shift) >= 0))
    {
        setOffset(bitPos + shift);
    }
}


std::vector<uint8_t>& BitStream::data()
{
    return bits;
}

// 0, 0b1, 0b11, 0b111 ...
const uint32_t masks[33] = {
    0, 
    0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,
    0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff,
    0x1ffff, 0x3ffff, 0x7ffff, 0xfffff, 0x1fffff, 0x3fffff, 0x7fffff, 0xffffff,
    0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
};

uint32_t BitStream::data(std::size_t size) const
{
    if((size > 32) || (size > (length() - bitPos)) || bits.size() == 0)
    {
        throw InvalidArgument("Invalid number of bits requested");
    }

    uint64_t t = 0;
    for (int i = 0; i < 5; ++i){
        if (bytePos + i < bits.size())
        {
            t += bits[bytePos+i];
        }
        t <<= 8; // we will get extra 8 bits shift
    }
    auto start = (bitPos % 8);
    t >>= (40 + 8) - (start + size); // 8(bits in bytes) * 5(bytes) + (extra shift)

    return t & masks[size];
}


uint32_t BitStream::zeroAdjustedData(std::size_t size) const
{
    auto val = data(size);
    if (val == 0)
    {
        val = 0x1 << size;
    }
    return val;
}

std::size_t BitStream::size() const
{
    return bits.size();
}


std::size_t BitStream::length() const
{
    return size() * 8;
}

std::size_t BitStream::remainingBits() const
{
    return length() - bitPos;
}


void BitStream::reset()
{
    bitPos = 0;
    bytePos = 0;
}


void BitStream::setOffset(uint32_t offset)
{
    if(offset <= length())
    {
        bitPos = offset;
        bytePos = bitPos / 8;
    }
}


void BitStream::setData(const std::vector<uint8_t> stream)
{
    bits = stream;
    setOffset(0);
}

} // namespace scte
} // namespace subttxtrend
