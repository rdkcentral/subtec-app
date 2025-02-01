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


#include <cppunit/extensions/HelperMacros.h>

#include "BitStream.hpp"

using dvbsubdecoder::BitStream;
using dvbsubdecoder::PesPacketReader;

class BitStreamTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( BitStreamTest );
    CPPUNIT_TEST(testReadSimple);
    CPPUNIT_TEST(testReadRandom);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // noop
    }

    void tearDown()
    {
        // noop
    }

    template<std::uint32_t ITEM_SIZE>
    void testReadSimple(std::uint32_t dataValue)
    {
        // build bytes table
        std::vector<std::uint8_t> data;
        data.push_back((dataValue >> 24) & 0xFF);
        data.push_back((dataValue >> 16) & 0xFF);
        data.push_back((dataValue >> 8) & 0xFF);
        data.push_back((dataValue >> 0) & 0xFF);

        // build expected bit chunks table
        std::vector<std::uint8_t> expected;
        const std::uint32_t bitMask = (1 << ITEM_SIZE) - 1;
        for (int bitIndex = 32 - ITEM_SIZE; bitIndex >= 0; bitIndex -=
                ITEM_SIZE)
        {
            expected.push_back((dataValue >> bitIndex) & bitMask);
        }

        // build read bit chunks table
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        std::vector<std::uint8_t> read;

        for (std::size_t i = 0; i < data.size() * 8 / ITEM_SIZE; ++i)
        {
            read.push_back(bitStream.read<ITEM_SIZE>());
        }

        CPPUNIT_ASSERT_EQUAL(expected.size(), read.size());
        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL(expected[i], read[i]);
        }
    }

    void testReadSimple()
    {
        const std::uint32_t values[] =
        { 0x12345678, 0xFE12EF21, 0x11223221, 0xFFFFFFFF, 0x00000000 };
        for (auto iter = std::begin(values); iter != std::end(values); ++iter)
        {
            testReadSimple<1>(*iter);
            testReadSimple<2>(*iter);
            testReadSimple<3>(*iter);
            testReadSimple<4>(*iter);
            testReadSimple<5>(*iter);
            testReadSimple<6>(*iter);
            testReadSimple<7>(*iter);
            testReadSimple<8>(*iter);
        }
    }

    void testReadRandom()
    {
        struct TestItem
        {
            constexpr TestItem(std::uint32_t value,
                               int size) :
                    m_value(value),
                    m_size(size)
            {
                // noop
            }

            std::uint32_t m_value;
            int m_size;
        };

        constexpr TestItem items[] =
        {
        { 1, 1 },
        { 2, 3 },
        { 3, 2 },
        { 0, 1 },
        { 255, 8 },
        { 6, 3 },
        { 14, 5 },
        { 1, 1 },
        { 0, 2 },
        { 1, 1 } };

        std::vector<std::uint8_t> expected;

        std::uint32_t dataValue = 0;
        int totalSize = 0;
        for (auto iter = std::begin(items); iter != std::end(items); ++iter)
        {
            CPPUNIT_ASSERT(iter->m_size >= 1);
            CPPUNIT_ASSERT(iter->m_size <= 8);
            CPPUNIT_ASSERT(iter->m_value < (1 << iter->m_size));

            dataValue <<= iter->m_size;
            dataValue |= iter->m_value;

            expected.push_back(iter->m_value);

            totalSize += iter->m_size;
        }
        CPPUNIT_ASSERT(totalSize <= 32);

        dataValue <<= (32 - totalSize);

        // build bytes table
        std::vector<std::uint8_t> data;
        data.push_back((dataValue >> 24) & 0xFF);
        data.push_back((dataValue >> 16) & 0xFF);
        data.push_back((dataValue >> 8) & 0xFF);
        data.push_back((dataValue >> 0) & 0xFF);

        // build read bit chunks table
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        std::vector<std::uint8_t> read;
        read.push_back(bitStream.read<items[0].m_size>());
        read.push_back(bitStream.read<items[1].m_size>());
        read.push_back(bitStream.read<items[2].m_size>());
        read.push_back(bitStream.read<items[3].m_size>());
        read.push_back(bitStream.read<items[4].m_size>());
        read.push_back(bitStream.read<items[5].m_size>());
        read.push_back(bitStream.read<items[6].m_size>());
        read.push_back(bitStream.read<items[7].m_size>());
        read.push_back(bitStream.read<items[8].m_size>());
        read.push_back(bitStream.read<items[9].m_size>());

        CPPUNIT_ASSERT_EQUAL(expected.size(), read.size());
        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL((int )expected[i], (int )read[i]);
        }
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(BitStreamTest);
