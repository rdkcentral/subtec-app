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

#include <vector>

#include "PesBuffer.hpp"
#include "DynamicAllocator.hpp"
#include "Consts.hpp"

using dvbsubdecoder::PesBuffer;
using dvbsubdecoder::PesPacketHeader;
using dvbsubdecoder::PesPacketReader;
using dvbsubdecoder::DynamicAllocator;
using dvbsubdecoder::StcTime;
using dvbsubdecoder::StcTimeType;

class PesBufferTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PesBufferTest );
    CPPUNIT_TEST(testFill);
    CPPUNIT_TEST(testAddAndProcess);
    CPPUNIT_TEST(testPts);
    CPPUNIT_TEST(testBadData);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        buildPacket(PES_PACKET_LENGTH);
    }

    void tearDown()
    {
        m_pesPacket.clear();
    }

    void testFill()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        // push single
        CPPUNIT_ASSERT(
                buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));

        // push until full
        while (buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()))
        {
            // noop
        }

        // consume one
        PesPacketHeader header;
        PesPacketReader dataReader;

        CPPUNIT_ASSERT(
                buffer.getNextPacket(StcTimeType::HIGH_32, header, dataReader));
        buffer.markPacketConsumed(header);

        CPPUNIT_ASSERT(
                buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));

        // clear all
        buffer.clear();
    }

    void testAddAndProcess()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        const int REPEAT_COUNT = 100;

        for (int i = 0; i < REPEAT_COUNT; ++i)
        {
            // push single
            CPPUNIT_ASSERT(
                    buffer.addPesPacket(m_pesPacket.data(),
                            m_pesPacket.size()));

            PesPacketHeader header;
            PesPacketReader dataReader;

            CPPUNIT_ASSERT(
                    buffer.getNextPacket(StcTimeType::HIGH_32, header,
                            dataReader));
            buffer.markPacketConsumed(header);
        }
    }

    void testPts()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        const int REPEAT_COUNT = 100;

        // high32
        for (int i = 0; i < REPEAT_COUNT; ++i)
        {
            std::uint64_t pts = (1LLU << 32) | (1LLU << 31) | i;
            std::uint32_t ptsHigh = pts >> 1;

            setPacketPts(pts);

            // push single
            CPPUNIT_ASSERT(
                    buffer.addPesPacket(m_pesPacket.data(),
                            m_pesPacket.size()));

            PesPacketHeader header;
            PesPacketReader dataReader;

            CPPUNIT_ASSERT(
                    buffer.getNextPacket(StcTimeType::HIGH_32, header,
                            dataReader));

            CPPUNIT_ASSERT(header.m_streamId == 0xBD);
            CPPUNIT_ASSERT(header.m_pesPacketLength == PES_PACKET_LENGTH);
            CPPUNIT_ASSERT(header.m_hasPts);
            CPPUNIT_ASSERT(header.m_pts.m_type == StcTimeType::HIGH_32);
            CPPUNIT_ASSERT(header.m_pts.m_time == ptsHigh);

            buffer.markPacketConsumed(header);
        }

        // low32
        for (int i = 0; i < REPEAT_COUNT; ++i)
        {
            std::uint64_t pts = (1LLU << 32) | (1LLU << 31) | i;
            std::uint32_t ptsLow = pts & 0xFFFFFFFF;

            setPacketPts(pts);

            // push single
            CPPUNIT_ASSERT(
                    buffer.addPesPacket(m_pesPacket.data(),
                            m_pesPacket.size()));

            PesPacketHeader header;
            PesPacketReader dataReader;

            CPPUNIT_ASSERT(
                    buffer.getNextPacket(StcTimeType::LOW_32, header,
                            dataReader));

            CPPUNIT_ASSERT(header.m_streamId == 0xBD);
            CPPUNIT_ASSERT(header.m_pesPacketLength == PES_PACKET_LENGTH);
            CPPUNIT_ASSERT(header.m_hasPts);
            CPPUNIT_ASSERT(header.m_pts.m_type == StcTimeType::LOW_32);
            CPPUNIT_ASSERT(header.m_pts.m_time == ptsLow);

            buffer.markPacketConsumed(header);
        }
    }

    void testBadData()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        // too small
        buildPacket(3);
        m_pesPacket.resize(5);

        CPPUNIT_ASSERT(
                !buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));

        // broken start code
        buildPacket(3);
        m_pesPacket[2] = 0xFF;

        CPPUNIT_ASSERT(
                !buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));

        // zero size (unlimited)
        buildPacket(0);

        CPPUNIT_ASSERT(
                !buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));

        // bad size
        buildPacket(1);
        m_pesPacket.push_back(0);

        CPPUNIT_ASSERT(
                !buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
    }

private:
    void buildPacket(std::uint16_t size)
    {
        m_pesPacket.clear();

        // build PES packet
        m_pesPacket.push_back(0x00);
        m_pesPacket.push_back(0x00);
        m_pesPacket.push_back(0x01);
        m_pesPacket.push_back(0xBD);

        m_pesPacket.push_back((size >> 8) & 0xFF);
        m_pesPacket.push_back((size >> 0) & 0xFF);

        for (auto i = 0; i < size; ++i)
        {
            m_pesPacket.push_back(0);
        }
    }

    void setPacketPts(std::uint64_t pts33)
    {
        std::uint8_t control1 = 0;
        control1 |= 2 << 6; // "10'

        std::uint8_t control2 = 0;
        control2 |= 2 << 6; // PTS_DTS_flags (PTS only)

        std::uint8_t headerLength = 5;

        const int START_OFFSET = 6;

        m_pesPacket[START_OFFSET + 0] = control1;
        m_pesPacket[START_OFFSET + 1] = control2;
        m_pesPacket[START_OFFSET + 2] = headerLength;

        m_pesPacket[START_OFFSET + 3] = 0;
        m_pesPacket[START_OFFSET + 4] = 0;
        m_pesPacket[START_OFFSET + 5] = 0;
        m_pesPacket[START_OFFSET + 6] = 0;
        m_pesPacket[START_OFFSET + 7] = 0;

        m_pesPacket[START_OFFSET + 3] |= 2 << 4; // '0010'
        m_pesPacket[START_OFFSET + 3] |= ((pts33 >> 30) & 0x7) << 1; // PTS[32..30]
        m_pesPacket[START_OFFSET + 3] |= 1 << 0; // marker_bit
        m_pesPacket[START_OFFSET + 4] |= ((pts33 >> 22) & 0xFF) << 0; // PTS[29..22]
        m_pesPacket[START_OFFSET + 5] |= ((pts33 >> 15) & 0x7F) << 1; // PTS[21..15]
        m_pesPacket[START_OFFSET + 5] |= 1 << 0; // marker_bit
        m_pesPacket[START_OFFSET + 6] |= ((pts33 >> 7) & 0xFF) << 0; // PTS[14..7]
        m_pesPacket[START_OFFSET + 7] |= ((pts33 >> 0) & 0x7F) << 1; // PTS[6..0]
        m_pesPacket[START_OFFSET + 7] |= 1 << 0; // marker_bit
    }

    static const auto PES_PACKET_LENGTH = 800;

    std::vector<std::uint8_t> m_pesPacket;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PesBufferTest);
