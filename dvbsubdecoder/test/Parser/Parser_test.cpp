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

#include "Parser.hpp"

#include "DecoderClientMock.hpp"
#include "BitStreamWriter.hpp"
#include "Misc.hpp"

#include "Parser.hpp"
#include "PixmapAllocator.hpp"
#include "Database.hpp"
#include "PesBuffer.hpp"
#include "PixmapAllocator.hpp"
#include "DynamicAllocator.hpp"

using dvbsubdecoder::Parser;
using dvbsubdecoder::Specification;
using dvbsubdecoder::PixmapAllocator;
using dvbsubdecoder::Database;
using dvbsubdecoder::PesBuffer;
using dvbsubdecoder::DynamicAllocator;
using dvbsubdecoder::StcTime;
using dvbsubdecoder::StcTimeType;

namespace
{

std::uint32_t g_ddsCounter = 0;
std::uint32_t g_pcsCounter = 0;
std::uint32_t g_rcsCounter = 0;
std::uint32_t g_cdsCounter = 0;
std::uint32_t g_odsCounter = 0;
std::uint32_t g_edsCounter = 0;

}

/* STUB method used for testing
 */
namespace dvbsubdecoder
{

void ParserDDS::parseDisplayDefinitionSegment(Database& database,
                                              PesPacketReader& reader)
{
    ++g_ddsCounter;
}

void ParserPCS::parsePageCompositionSegment(Database& database,
                                            PesPacketReader& reader)
{
    ++g_pcsCounter;
}

void ParserRCS::parseRegionCompositionSegment(Database& database,
                                              PesPacketReader& reader)
{
    ++g_rcsCounter;
}

void ParserCDS::parseClutDefinitionSegment(Database& database,
                                           PesPacketReader& reader)
{
    ++g_cdsCounter;
}

void ParserODS::parseObjectDataSegment(Database& database,
                                       PesPacketReader& reader)
{
    ++g_odsCounter;
}

void ParserEDS::parseEndOfDisplaySetSegment(Database& database,
                                            PesPacketReader& reader)
{
    ++g_edsCounter;
}

}

class ParserTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ParserTest );
    CPPUNIT_TEST(testDdsSegment);
    CPPUNIT_TEST(testPcsSegment);
    CPPUNIT_TEST(testRcsSegment);
    CPPUNIT_TEST(testCdsSegment);
    CPPUNIT_TEST(testOdsSegment);
    CPPUNIT_TEST(testEdsSegment);
    CPPUNIT_TEST(testUnknownSegment);CPPUNIT_TEST_SUITE_END()
    ;

public:
    void setUp()
    {
        m_client.reset(new DecoderClientMock());
        m_client->setAllocLimit(PixmapAllocator::BUFFER_SIZE_131);
        m_pixmapAllocator.reset(new PixmapAllocator(SPEC_VERSION, *m_client));
        m_database.reset(new Database(SPEC_VERSION, *m_pixmapAllocator));
    }

    void tearDown()
    {
        m_database.reset();
        m_pixmapAllocator.reset();
        m_client.reset();
    }

    void testDdsSegment()
    {
        testSegmentSimple(dvbsubdecoder::SegmentTypeBits::DISPLAY_DEFINITION,
                &g_ddsCounter);
    }

    void testPcsSegment()
    {
        testSegmentSimple(dvbsubdecoder::SegmentTypeBits::PAGE_COMPOSITION,
                &g_pcsCounter);
    }

    void testRcsSegment()
    {
        testSegmentSimple(dvbsubdecoder::SegmentTypeBits::REGION_COMPOSITION,
                &g_rcsCounter);
    }

    void testCdsSegment()
    {
        testSegmentSimple(dvbsubdecoder::SegmentTypeBits::CLUT_DEFINITION,
                &g_cdsCounter);
    }

    void testOdsSegment()
    {
        testSegmentSimple(dvbsubdecoder::SegmentTypeBits::OBJECT_DATA,
                &g_odsCounter);
    }

    void testEdsSegment()
    {
        testSegmentSimple(dvbsubdecoder::SegmentTypeBits::END_OF_DISPLAY_SET,
                &g_edsCounter);
    }

    void testUnknownSegment()
    {
        BitStreamWriter pesWriter;

        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);

        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(0, 0);

        buildPesWithEmptySegment(pesWriter, 0x72);

        CPPUNIT_ASSERT(
                pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        CPPUNIT_ASSERT(parser.process(StcTime(StcTimeType::HIGH_32, 0)));
        CPPUNIT_ASSERT(g_ddsCounter == 0);
        CPPUNIT_ASSERT(g_pcsCounter == 0);
        CPPUNIT_ASSERT(g_rcsCounter == 0);
        CPPUNIT_ASSERT(g_cdsCounter == 0);
        CPPUNIT_ASSERT(g_odsCounter == 0);
        CPPUNIT_ASSERT(g_edsCounter == 0);
    }

private:
    void testSegmentSimple(std::uint8_t segmentType,
                           const std::uint32_t* counterPtr)
    {
        BitStreamWriter pesWriter;

        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);

        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(0, 0);

        buildPesWithEmptySegment(pesWriter, segmentType);

        CPPUNIT_ASSERT(
                pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        CPPUNIT_ASSERT(parser.process(StcTime(StcTimeType::HIGH_32, 0)));

        std::uint32_t ddsExpected = (counterPtr == &g_ddsCounter) ? 1 : 0;
        std::uint32_t pcsExpected = (counterPtr == &g_pcsCounter) ? 1 : 0;
        std::uint32_t rcsExpected = (counterPtr == &g_rcsCounter) ? 1 : 0;
        std::uint32_t cdsExpected = (counterPtr == &g_cdsCounter) ? 1 : 0;
        std::uint32_t odsExpected = (counterPtr == &g_odsCounter) ? 1 : 0;
        std::uint32_t edsExpected = (counterPtr == &g_edsCounter) ? 1 : 0;

        CPPUNIT_ASSERT(g_ddsCounter == ddsExpected);
        CPPUNIT_ASSERT(g_pcsCounter == pcsExpected);
        CPPUNIT_ASSERT(g_rcsCounter == rcsExpected);
        CPPUNIT_ASSERT(g_cdsCounter == cdsExpected);
        CPPUNIT_ASSERT(g_odsCounter == odsExpected);
        CPPUNIT_ASSERT(g_edsCounter == edsExpected);
    }

    void clearCounters()
    {
        g_ddsCounter = 0;
        g_pcsCounter = 0;
        g_rcsCounter = 0;
        g_cdsCounter = 0;
        g_odsCounter = 0;
        g_edsCounter = 0;
    }

    void buildPesWithEmptySegment(BitStreamWriter& pesWriter,
                                  std::uint8_t segmentType)
    {
        BitStreamWriter segmentWriter;
        BitStreamWriter pesDataWriter;

        segmentWriter.clear();

        startPesData(pesDataWriter);
        buildPesSegment(pesDataWriter, segmentType, 0, segmentWriter);
        endPesData(pesDataWriter);

        buildPesPacket(pesWriter, 0, pesDataWriter);
    }

    void buildPesSegment(BitStreamWriter& dataWriter,
                         std::uint8_t segmentType,
                         std::uint16_t pageId,
                         const BitStreamWriter& segmentWriter)
    {
        dataWriter.write(dvbsubdecoder::PesBits::SYNC_BYTE_VALUE, 8);
        dataWriter.write(segmentType, 8);
        dataWriter.write(pageId, 16);

        const auto segmentPtr = segmentWriter.data();
        const auto segmentSize = segmentWriter.size();

        dataWriter.write(segmentSize, 16);

        for (std::size_t i = 0; i < segmentSize; ++i)
        {
            dataWriter.write(segmentPtr[i], 8);
        }
    }

    void startPesData(BitStreamWriter& dataWriter)
    {
        dataWriter.clear();

        dataWriter.write(dvbsubdecoder::PesBits::SUBTITLE_DATA_IDENTIFIER, 8);
        dataWriter.write(dvbsubdecoder::PesBits::STREAM_ID_VALUE, 8);
    }

    void endPesData(BitStreamWriter& dataWriter)
    {
        dataWriter.write(dvbsubdecoder::PesBits::END_MARKER_VALUE, 8);
    }

    void buildPesPacket(BitStreamWriter& pesWriter,
                        std::uint64_t pts33,
                        const BitStreamWriter& pesDataWriter)
    {
        pesWriter.clear();

        const auto dataPtr = pesDataWriter.data();
        const auto dataSize = pesDataWriter.size();

        buildPacketHeader(pesWriter, dataSize, pts33);
        for (std::size_t i = 0; i < dataSize; ++i)
        {
            pesWriter.write(dataPtr[i], 8);
        }
    }

    void buildPacketHeader(BitStreamWriter& writer,
                           std::uint16_t dataSize,
                           std::uint64_t pts33)
    {
        const std::uint8_t PTS_LENGTH = 5;
        const std::uint8_t HEADER_LENGTH = 3 + PTS_LENGTH;

        // build PES packet
        writer.write(0x00, 8);
        writer.write(0x00, 8);
        writer.write(0x01, 8);
        writer.write(0xBD, 8);

        const std::uint16_t pesSize = HEADER_LENGTH + dataSize;

        writer.write((pesSize >> 8) & 0xFF, 8);
        writer.write((pesSize >> 0) & 0xFF, 8);

        std::uint8_t control1 = 0;
        control1 |= 2 << 6; // "10'

        std::uint8_t control2 = 0;
        control2 |= 2 << 6; // PTS_DTS_flags (PTS only)

        writer.write(control1, 8);
        writer.write(control2, 8);
        writer.write(PTS_LENGTH, 8);

        std::uint8_t ptsBytes[PTS_LENGTH];

        ptsBytes[0] = 0;
        ptsBytes[1] = 0;
        ptsBytes[2] = 0;
        ptsBytes[3] = 0;
        ptsBytes[4] = 0;

        ptsBytes[0] |= 2 << 4; // '0010'
        ptsBytes[0] |= ((pts33 >> 30) & 0x7) << 1; // PTS[32..30]
        ptsBytes[0] |= 1 << 0; // marker_bit
        ptsBytes[1] |= ((pts33 >> 22) & 0xFF) << 0; // PTS[29..22]
        ptsBytes[2] |= ((pts33 >> 15) & 0x7F) << 1; // PTS[21..15]
        ptsBytes[2] |= 1 << 0; // marker_bit
        ptsBytes[3] |= ((pts33 >> 7) & 0xFF) << 0; // PTS[14..7]
        ptsBytes[4] |= ((pts33 >> 0) & 0x7F) << 1; // PTS[6..0]
        ptsBytes[4] |= 1 << 0; // marker_bit

        for (int i = 0; i < PTS_LENGTH; ++i)
        {
            writer.write(ptsBytes[i], 8);
        }
    }

    const Specification SPEC_VERSION = Specification::VERSION_1_3_1;

    std::unique_ptr<DecoderClientMock> m_client;
    std::unique_ptr<PixmapAllocator> m_pixmapAllocator;
    std::unique_ptr<Database> m_database;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ParserTest);
