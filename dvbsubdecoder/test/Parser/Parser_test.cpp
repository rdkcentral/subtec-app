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
    CPPUNIT_TEST(testUnknownSegment);
    CPPUNIT_TEST(testEmptyBufferReturnsFalse);
    CPPUNIT_TEST(testNonSubtitlePacketSkipped);
    CPPUNIT_TEST(testPacketWithoutPtsSkipped);
    CPPUNIT_TEST(testInvalidDataIdentifierSkipped);
    CPPUNIT_TEST(testInvalidStreamIdSkipped);
    CPPUNIT_TEST(testTimingWaitClassification);
    CPPUNIT_TEST(testTimingDropClassification);
    CPPUNIT_TEST(testTimingProcessClassification);
    CPPUNIT_TEST(testWaitPacketRetention);
    CPPUNIT_TEST(testDropPacketConsumed);
    CPPUNIT_TEST(testMultipleSegmentsSinglePacket);
    CPPUNIT_TEST(testUnselectedPageIgnored);
    CPPUNIT_TEST(testNonCompositionPagePcsIgnored);
    CPPUNIT_TEST(testNonCompositionPageRcsIgnored);
    CPPUNIT_TEST(testNonCompositionPageDdsIgnored);
    CPPUNIT_TEST(testNonAncillaryPageEdsIgnored);
    CPPUNIT_TEST(testDdsIgnoredOnOlderSpec);
    CPPUNIT_TEST(testBoundaryTimingMinWait);
    CPPUNIT_TEST(testBoundaryTimingMaxWait);
    CPPUNIT_TEST(testBoundaryTimingJustBelowMin);
    CPPUNIT_TEST(testMixedSelectedUnselectedSegments);
    CPPUNIT_TEST(testInvalidSyncByteException);
    CPPUNIT_TEST(testInvalidEndMarkerException);
CPPUNIT_TEST_SUITE_END();

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

    // Empty buffer should return false
    void testEmptyBufferReturnsFalse()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, 12345));
        
        CPPUNIT_ASSERT(!result);
        CPPUNIT_ASSERT(g_ddsCounter == 0);
        CPPUNIT_ASSERT(g_pcsCounter == 0);
        CPPUNIT_ASSERT(g_rcsCounter == 0);
        CPPUNIT_ASSERT(g_cdsCounter == 0);
        CPPUNIT_ASSERT(g_odsCounter == 0);
        CPPUNIT_ASSERT(g_edsCounter == 0);
    }

    // Non-subtitle packet should be skipped
    void testNonSubtitlePacketSkipped()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(0, 0);

        // Create a non-subtitle packet (different stream ID in header)
        BitStreamWriter pesWriter;
        BitStreamWriter pesDataWriter;
        
        startPesData(pesDataWriter);
        BitStreamWriter segmentWriter;
        segmentWriter.clear();
        buildPesSegment(pesDataWriter, dvbsubdecoder::SegmentTypeBits::CLUT_DEFINITION, 0, segmentWriter);
        endPesData(pesDataWriter);

        buildNonSubtitlePacket(pesWriter, 0, pesDataWriter);

        // PesBuffer should reject non-subtitle packets (stream ID != 0xBD)
        CPPUNIT_ASSERT(!pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, 0));
        
        // No packet was added to buffer, so parser should return false
        CPPUNIT_ASSERT(!result);
        CPPUNIT_ASSERT(g_cdsCounter == 0);
    }

    // Packet without PTS should be skipped
    void testPacketWithoutPtsSkipped()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(0, 0);

        BitStreamWriter pesWriter;
        BitStreamWriter pesDataWriter;
        
        startPesData(pesDataWriter);
        BitStreamWriter segmentWriter;
        segmentWriter.clear();
        buildPesSegment(pesDataWriter, dvbsubdecoder::SegmentTypeBits::OBJECT_DATA, 0, segmentWriter);
        endPesData(pesDataWriter);

        buildPacketWithoutPts(pesWriter, pesDataWriter);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, 0));
        
        CPPUNIT_ASSERT(!result);
        CPPUNIT_ASSERT(g_odsCounter == 0);
    }

    // Invalid data identifier should cause early return
    void testInvalidDataIdentifierSkipped()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(0, 0);

        BitStreamWriter pesDataWriter;
        pesDataWriter.clear();
        
        // Write invalid data identifier (should be SUBTITLE_DATA_IDENTIFIER)
        pesDataWriter.write(0xFF, 8);
        pesDataWriter.write(dvbsubdecoder::PesBits::STREAM_ID_VALUE, 8);
        pesDataWriter.write(dvbsubdecoder::PesBits::END_MARKER_VALUE, 8);

        BitStreamWriter pesWriter;
        buildPesPacket(pesWriter, 0, pesDataWriter);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, 0));
        
        // Should return true (packet processed) but no segments parsed
        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(g_ddsCounter == 0);
        CPPUNIT_ASSERT(g_pcsCounter == 0);
        CPPUNIT_ASSERT(g_rcsCounter == 0);
        CPPUNIT_ASSERT(g_cdsCounter == 0);
        CPPUNIT_ASSERT(g_odsCounter == 0);
        CPPUNIT_ASSERT(g_edsCounter == 0);
    }

    // Invalid stream ID should cause early return
    void testInvalidStreamIdSkipped()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(0, 0);

        BitStreamWriter pesDataWriter;
        pesDataWriter.clear();
        
        pesDataWriter.write(dvbsubdecoder::PesBits::SUBTITLE_DATA_IDENTIFIER, 8);
        pesDataWriter.write(0xAB, 8); // Invalid stream ID
        pesDataWriter.write(dvbsubdecoder::PesBits::END_MARKER_VALUE, 8);

        BitStreamWriter pesWriter;
        buildPesPacket(pesWriter, 0, pesDataWriter);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, 0));
        
        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(g_ddsCounter == 0);
        CPPUNIT_ASSERT(g_pcsCounter == 0);
        CPPUNIT_ASSERT(g_rcsCounter == 0);
        CPPUNIT_ASSERT(g_cdsCounter == 0);
        CPPUNIT_ASSERT(g_odsCounter == 0);
        CPPUNIT_ASSERT(g_edsCounter == 0);
    }

    // WAIT timing classification
    void testTimingWaitClassification()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(0, 0);

        const uint32_t stcTime = 1000000;
        const uint32_t ptsTime = stcTime + (dvbsubdecoder::PTS_STC_DIFF_MIN_90KHZ + dvbsubdecoder::PTS_STC_DIFF_MAX_90KHZ) / 2; // In WAIT range

        BitStreamWriter pesWriter;
        buildPesWithEmptySegment(pesWriter, dvbsubdecoder::SegmentTypeBits::CLUT_DEFINITION);
        
        // Modify PTS in the packet
        modifyPacketPts(pesWriter, ptsTime);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, stcTime));
        
        // WAIT should return false and not process segments
        CPPUNIT_ASSERT(!result);
        CPPUNIT_ASSERT(g_cdsCounter == 0);
    }

    // DROP timing classification
    void testTimingDropClassification()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(0, 0);

        const uint32_t stcTime = 1000000;
        const uint32_t ptsTime = stcTime + dvbsubdecoder::PTS_STC_DIFF_MAX_90KHZ + 100000; // Beyond WAIT range

        BitStreamWriter pesWriter;
        buildPesWithEmptySegment(pesWriter, dvbsubdecoder::SegmentTypeBits::OBJECT_DATA);
        
        modifyPacketPts(pesWriter, ptsTime);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, stcTime));
        
        // DROP should return false and not process segments
        CPPUNIT_ASSERT(!result);
        CPPUNIT_ASSERT(g_odsCounter == 0);
    }

    // PROCESS timing classification
    void testTimingProcessClassification()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(0, 0);

        const uint32_t stcTime = 1000000;
        const uint32_t ptsTime = stcTime - 1000; // Just processed

        BitStreamWriter pesWriter;
        buildPesWithEmptySegment(pesWriter, dvbsubdecoder::SegmentTypeBits::REGION_COMPOSITION);
        
        modifyPacketPts(pesWriter, ptsTime);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, stcTime));
        
        // PROCESS should return true and process segments
        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(g_rcsCounter == 1);
    }

    // WAIT packet retention across calls
    void testWaitPacketRetention()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(0, 0);

        const uint32_t stcTime1 = 1000000;
        const uint32_t ptsTime = stcTime1 + dvbsubdecoder::PTS_STC_DIFF_MIN_90KHZ + 1000; // In WAIT range
        const uint32_t stcTime2 = ptsTime; // STC catches up

        BitStreamWriter pesWriter;
        buildPesWithEmptySegment(pesWriter, dvbsubdecoder::SegmentTypeBits::PAGE_COMPOSITION);
        
        modifyPacketPts(pesWriter, ptsTime);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        
        // First call: WAIT
        bool result1 = parser.process(StcTime(StcTimeType::LOW_32, stcTime1));
        CPPUNIT_ASSERT(!result1);
        CPPUNIT_ASSERT(g_pcsCounter == 0);

        // Second call: PROCESS (packet retained)
        bool result2 = parser.process(StcTime(StcTimeType::LOW_32, stcTime2));
        CPPUNIT_ASSERT(result2);
        CPPUNIT_ASSERT(g_pcsCounter == 1);
    }

    // DROP packet is consumed
    void testDropPacketConsumed()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(0, 0);

        const uint32_t stcTime = 1000000;
        const uint32_t ptsTime = stcTime + dvbsubdecoder::PTS_STC_DIFF_MAX_90KHZ + 200000; // DROP range

        BitStreamWriter pesWriter;
        buildPesWithEmptySegment(pesWriter, dvbsubdecoder::SegmentTypeBits::DISPLAY_DEFINITION);
        
        modifyPacketPts(pesWriter, ptsTime);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        
        // First call: DROP
        bool result1 = parser.process(StcTime(StcTimeType::LOW_32, stcTime));
        CPPUNIT_ASSERT(!result1);
        CPPUNIT_ASSERT(g_ddsCounter == 0);

        // Second call: No packets left
        bool result2 = parser.process(StcTime(StcTimeType::LOW_32, stcTime));
        CPPUNIT_ASSERT(!result2);
        CPPUNIT_ASSERT(g_ddsCounter == 0);
    }

    // Multiple segments in single packet
    void testMultipleSegmentsSinglePacket()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(0, 0);

        BitStreamWriter pesDataWriter;
        BitStreamWriter segmentWriter;
        
        startPesData(pesDataWriter);
        
        segmentWriter.clear();
        buildPesSegment(pesDataWriter, dvbsubdecoder::SegmentTypeBits::CLUT_DEFINITION, 0, segmentWriter);
        buildPesSegment(pesDataWriter, dvbsubdecoder::SegmentTypeBits::OBJECT_DATA, 0, segmentWriter);
        buildPesSegment(pesDataWriter, dvbsubdecoder::SegmentTypeBits::END_OF_DISPLAY_SET, 0, segmentWriter);
        
        endPesData(pesDataWriter);

        BitStreamWriter pesWriter;
        buildPesPacket(pesWriter, 0, pesDataWriter);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, 0));
        
        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(g_cdsCounter == 1);
        CPPUNIT_ASSERT(g_odsCounter == 1);
        CPPUNIT_ASSERT(g_edsCounter == 1);
    }

    // Unselected page segments ignored
    void testUnselectedPageIgnored()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(5, 5); // Select page 5

        BitStreamWriter pesWriter;
        BitStreamWriter pesDataWriter;
        BitStreamWriter segmentWriter;
        
        startPesData(pesDataWriter);
        segmentWriter.clear();
        buildPesSegment(pesDataWriter, dvbsubdecoder::SegmentTypeBits::CLUT_DEFINITION, 10, segmentWriter); // Page 10 (unselected)
        endPesData(pesDataWriter);

        buildPesPacket(pesWriter, 0, pesDataWriter);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, 0));
        
        // Should process packet but ignore segment due to page mismatch
        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(g_cdsCounter == 0);
    }

    // Non-composition page PCS ignored
    void testNonCompositionPagePcsIgnored()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(1, 2); // Composition=1, Ancillary=2

        BitStreamWriter pesWriter;
        BitStreamWriter pesDataWriter;
        BitStreamWriter segmentWriter;
        
        startPesData(pesDataWriter);
        segmentWriter.clear();
        buildPesSegment(pesDataWriter, dvbsubdecoder::SegmentTypeBits::PAGE_COMPOSITION, 3, segmentWriter); // Page 3 (not composition)
        endPesData(pesDataWriter);

        buildPesPacket(pesWriter, 0, pesDataWriter);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, 0));
        
        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(g_pcsCounter == 0);
    }

    // Non-composition page RCS ignored
    void testNonCompositionPageRcsIgnored()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(1, 2); // Composition=1, Ancillary=2

        BitStreamWriter pesWriter;
        BitStreamWriter pesDataWriter;
        BitStreamWriter segmentWriter;
        
        startPesData(pesDataWriter);
        segmentWriter.clear();
        buildPesSegment(pesDataWriter, dvbsubdecoder::SegmentTypeBits::REGION_COMPOSITION, 4, segmentWriter); // Page 4 (not composition)
        endPesData(pesDataWriter);

        buildPesPacket(pesWriter, 0, pesDataWriter);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, 0));
        
        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(g_rcsCounter == 0);
    }

    // Non-composition page DDS ignored
    void testNonCompositionPageDdsIgnored()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(1, 2); // Composition=1, Ancillary=2

        BitStreamWriter pesWriter;
        BitStreamWriter pesDataWriter;
        BitStreamWriter segmentWriter;
        
        startPesData(pesDataWriter);
        segmentWriter.clear();
        buildPesSegment(pesDataWriter, dvbsubdecoder::SegmentTypeBits::DISPLAY_DEFINITION, 5, segmentWriter); // Page 5 (not composition)
        endPesData(pesDataWriter);

        buildPesPacket(pesWriter, 0, pesDataWriter);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, 0));
        
        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(g_ddsCounter == 0);
    }

    // Non-ancillary page EDS ignored
    void testNonAncillaryPageEdsIgnored()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(1, 2); // Composition=1, Ancillary=2

        BitStreamWriter pesWriter;
        BitStreamWriter pesDataWriter;
        BitStreamWriter segmentWriter;
        
        startPesData(pesDataWriter);
        segmentWriter.clear();
        buildPesSegment(pesDataWriter, dvbsubdecoder::SegmentTypeBits::END_OF_DISPLAY_SET, 6, segmentWriter); // Page 6 (not ancillary)
        endPesData(pesDataWriter);

        buildPesPacket(pesWriter, 0, pesDataWriter);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, 0));
        
        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(g_edsCounter == 0);
    }

    // DDS ignored on older spec version
    void testDdsIgnoredOnOlderSpec()
    {
        // Create environment with spec 1.2.1
        std::unique_ptr<DecoderClientMock> altClient(new DecoderClientMock());
        altClient->setAllocLimit(PixmapAllocator::BUFFER_SIZE_131);
        std::unique_ptr<PixmapAllocator> altAllocator(new PixmapAllocator(Specification::VERSION_1_2_1, *altClient));
        std::unique_ptr<Database> altDb(new Database(Specification::VERSION_1_2_1, *altAllocator));

        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*altClient, *altDb, pesBuffer);

        altDb->getStatus().setPageIds(0, 0);

        BitStreamWriter pesWriter;
        buildPesWithEmptySegment(pesWriter, dvbsubdecoder::SegmentTypeBits::DISPLAY_DEFINITION);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, 0));
        
        // Should process packet but ignore DDS due to spec version
        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(g_ddsCounter == 0);
    }

    // Boundary timing at dvbsubdecoder::PTS_STC_DIFF_MIN_90KHZ (WAIT)
    void testBoundaryTimingMinWait()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(0, 0);

        const uint32_t stcTime = 2000000;
        const uint32_t ptsTime = stcTime + dvbsubdecoder::PTS_STC_DIFF_MIN_90KHZ; // Exact boundary

        BitStreamWriter pesWriter;
        buildPesWithEmptySegment(pesWriter, dvbsubdecoder::SegmentTypeBits::OBJECT_DATA);
        
        modifyPacketPts(pesWriter, ptsTime);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, stcTime));
        
        // Should be WAIT (boundary inclusive)
        CPPUNIT_ASSERT(!result);
        CPPUNIT_ASSERT(g_odsCounter == 0);
    }

    // Boundary timing at dvbsubdecoder::PTS_STC_DIFF_MAX_90KHZ (WAIT)
    void testBoundaryTimingMaxWait()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(0, 0);

        const uint32_t stcTime = 3000000;
        const uint32_t ptsTime = stcTime + dvbsubdecoder::PTS_STC_DIFF_MAX_90KHZ; // Exact boundary

        BitStreamWriter pesWriter;
        buildPesWithEmptySegment(pesWriter, dvbsubdecoder::SegmentTypeBits::CLUT_DEFINITION);
        
        modifyPacketPts(pesWriter, ptsTime);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, stcTime));
        
        // Should be WAIT (boundary inclusive)
        CPPUNIT_ASSERT(!result);
        CPPUNIT_ASSERT(g_cdsCounter == 0);
    }

    // Just below dvbsubdecoder::PTS_STC_DIFF_MIN_90KHZ (PROCESS)
    void testBoundaryTimingJustBelowMin()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(0, 0);

        const uint32_t stcTime = 4000000;
        const uint32_t ptsTime = stcTime + dvbsubdecoder::PTS_STC_DIFF_MIN_90KHZ - 1; // Just below

        BitStreamWriter pesWriter;
        buildPesWithEmptySegment(pesWriter, dvbsubdecoder::SegmentTypeBits::REGION_COMPOSITION);
        
        modifyPacketPts(pesWriter, ptsTime);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, stcTime));
        
        // Should be PROCESS
        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(g_rcsCounter == 1);
    }

    // Mixed selected and unselected segments
    void testMixedSelectedUnselectedSegments()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(10, 10);

        BitStreamWriter pesDataWriter;
        BitStreamWriter segmentWriter;
        
        startPesData(pesDataWriter);
        segmentWriter.clear();
        
        // Mix of selected (page 10) and unselected (page 99) segments
        buildPesSegment(pesDataWriter, dvbsubdecoder::SegmentTypeBits::OBJECT_DATA, 99, segmentWriter); // Unselected
        buildPesSegment(pesDataWriter, dvbsubdecoder::SegmentTypeBits::OBJECT_DATA, 10, segmentWriter); // Selected
        buildPesSegment(pesDataWriter, dvbsubdecoder::SegmentTypeBits::CLUT_DEFINITION, 10, segmentWriter); // Selected
        buildPesSegment(pesDataWriter, dvbsubdecoder::SegmentTypeBits::CLUT_DEFINITION, 99, segmentWriter); // Unselected
        
        endPesData(pesDataWriter);

        BitStreamWriter pesWriter;
        buildPesPacket(pesWriter, 0, pesDataWriter);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, 0));
        
        CPPUNIT_ASSERT(result);
        // Only selected page segments should be processed
        CPPUNIT_ASSERT(g_odsCounter == 1);
        CPPUNIT_ASSERT(g_cdsCounter == 1);
    }

    // Invalid sync byte causes exception
    void testInvalidSyncByteException()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(0, 0);

        BitStreamWriter pesDataWriter;
        pesDataWriter.clear();
        
        pesDataWriter.write(dvbsubdecoder::PesBits::SUBTITLE_DATA_IDENTIFIER, 8);
        pesDataWriter.write(dvbsubdecoder::PesBits::STREAM_ID_VALUE, 8);
        
        // Invalid sync byte (should be SYNC_BYTE_VALUE)
        pesDataWriter.write(0x0E, 8); // Wrong sync byte
        pesDataWriter.write(dvbsubdecoder::SegmentTypeBits::CLUT_DEFINITION, 8);
        pesDataWriter.write(0, 16); // Page ID
        pesDataWriter.write(0, 16); // Segment length
        
        pesDataWriter.write(dvbsubdecoder::PesBits::END_MARKER_VALUE, 8);

        BitStreamWriter pesWriter;
        buildPesPacket(pesWriter, 0, pesDataWriter);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, 0));
        
        // Exception should be caught, buffer cleared, returns true
        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(g_cdsCounter == 0);
    }

    // Invalid end marker causes exception
    void testInvalidEndMarkerException()
    {
        DynamicAllocator allocator;
        PesBuffer pesBuffer(allocator);
        Parser parser(*m_client, *m_database, pesBuffer);

        m_database->getStatus().setPageIds(0, 0);

        BitStreamWriter pesDataWriter;
        pesDataWriter.clear();
        
        pesDataWriter.write(dvbsubdecoder::PesBits::SUBTITLE_DATA_IDENTIFIER, 8);
        pesDataWriter.write(dvbsubdecoder::PesBits::STREAM_ID_VALUE, 8);
        
        // Valid segment
        pesDataWriter.write(dvbsubdecoder::PesBits::SYNC_BYTE_VALUE, 8);
        pesDataWriter.write(dvbsubdecoder::SegmentTypeBits::CLUT_DEFINITION, 8);
        pesDataWriter.write(0, 16); // Page ID
        pesDataWriter.write(0, 16); // Segment length
        
        // Invalid end marker (should be END_MARKER_VALUE)
        pesDataWriter.write(0x99, 8);

        BitStreamWriter pesWriter;
        buildPesPacket(pesWriter, 0, pesDataWriter);

        CPPUNIT_ASSERT(pesBuffer.addPesPacket(pesWriter.data(), pesWriter.size()));

        clearCounters();
        bool result = parser.process(StcTime(StcTimeType::LOW_32, 0));
        
        // Exception should be caught, epoch reset, returns true
        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(g_cdsCounter == 1); // Segment processed before exception
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

    // Helper methods for new tests
    void buildNonSubtitlePacket(BitStreamWriter& pesWriter,
                               std::uint64_t pts33,
                               const BitStreamWriter& pesDataWriter)
    {
        pesWriter.clear();

        const auto dataPtr = pesDataWriter.data();
        const auto dataSize = pesDataWriter.size();

        // Build packet header with different stream ID to make it non-subtitle
        const std::uint8_t PTS_LENGTH = 5;
        const std::uint8_t HEADER_LENGTH = 3 + PTS_LENGTH;

        // PES start code
        pesWriter.write(0x00, 8);
        pesWriter.write(0x00, 8);
        pesWriter.write(0x01, 8);
        pesWriter.write(0xC0, 8); // Different stream ID (audio instead of 0xBD)

        const std::uint16_t pesSize = HEADER_LENGTH + dataSize;
        pesWriter.write((pesSize >> 8) & 0xFF, 8);
        pesWriter.write((pesSize >> 0) & 0xFF, 8);

        std::uint8_t control1 = 0;
        control1 |= 2 << 6; // "10'

        std::uint8_t control2 = 0;
        control2 |= 2 << 6; // PTS_DTS_flags (PTS only)

        pesWriter.write(control1, 8);
        pesWriter.write(control2, 8);
        pesWriter.write(PTS_LENGTH, 8);

        std::uint8_t ptsBytes[PTS_LENGTH];
        ptsBytes[0] = 0; ptsBytes[1] = 0; ptsBytes[2] = 0; ptsBytes[3] = 0; ptsBytes[4] = 0;

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
            pesWriter.write(ptsBytes[i], 8);
        }

        for (std::size_t i = 0; i < dataSize; ++i)
        {
            pesWriter.write(dataPtr[i], 8);
        }
    }

    void buildPacketWithoutPts(BitStreamWriter& pesWriter,
                              const BitStreamWriter& pesDataWriter)
    {
        pesWriter.clear();

        const auto dataPtr = pesDataWriter.data();
        const auto dataSize = pesDataWriter.size();

        const std::uint8_t HEADER_LENGTH = 3; // No PTS

        // PES start code
        pesWriter.write(0x00, 8);
        pesWriter.write(0x00, 8);
        pesWriter.write(0x01, 8);
        pesWriter.write(0xBD, 8);

        const std::uint16_t pesSize = HEADER_LENGTH + dataSize;
        pesWriter.write((pesSize >> 8) & 0xFF, 8);
        pesWriter.write((pesSize >> 0) & 0xFF, 8);

        std::uint8_t control1 = 0;
        control1 |= 2 << 6; // "10'

        std::uint8_t control2 = 0; // No PTS_DTS_flags

        pesWriter.write(control1, 8);
        pesWriter.write(control2, 8);
        pesWriter.write(0, 8); // Header length = 0

        for (std::size_t i = 0; i < dataSize; ++i)
        {
            pesWriter.write(dataPtr[i], 8);
        }
    }

    void modifyPacketPts(BitStreamWriter& pesWriter, std::uint64_t newPts)
    {
        // Modify PTS in existing packet data
        // PTS starts at byte 9 (after PES header fields)
        auto data = const_cast<std::uint8_t*>(pesWriter.data());
        
        data[9] = 0;
        data[10] = 0;
        data[11] = 0;
        data[12] = 0;
        data[13] = 0;

        data[9] |= 2 << 4; // '0010'
        data[9] |= ((newPts >> 30) & 0x7) << 1; // PTS[32..30]
        data[9] |= 1 << 0; // marker_bit
        data[10] |= ((newPts >> 22) & 0xFF) << 0; // PTS[29..22]
        data[11] |= ((newPts >> 15) & 0x7F) << 1; // PTS[21..15]
        data[11] |= 1 << 0; // marker_bit
        data[12] |= ((newPts >> 7) & 0xFF) << 0; // PTS[14..7]
        data[13] |= ((newPts >> 0) & 0x7F) << 1; // PTS[6..0]
        data[13] |= 1 << 0; // marker_bit
    }

    const Specification SPEC_VERSION = Specification::VERSION_1_3_1;

    std::unique_ptr<DecoderClientMock> m_client;
    std::unique_ptr<PixmapAllocator> m_pixmapAllocator;
    std::unique_ptr<Database> m_database;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ParserTest);
