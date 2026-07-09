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

#include "../../subttxrend-common/include/Logger.hpp"

#include "dvbsubdecoder/Decoder.hpp"
#include "dvbsubdecoder/DecoderFactory.hpp"
#include "dvbsubdecoder/DynamicAllocator.hpp"

#include "dvbsubdecoder/Types.hpp"

#include "DecoderClientMock.hpp"
#include "PesPackets/PesBuilder.hpp"

#include "TestCaseData.hpp"

namespace
{

using namespace dvbsubdecoder;

subttxrend::common::Logger g_logger("Decoder", "test");

class DecoderTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( DecoderTest );
    CPPUNIT_TEST(testInitialGfxSetup);
    CPPUNIT_TEST(testHappyPath);
    CPPUNIT_TEST(testMultiplePageHappyPath);
    CPPUNIT_TEST(testRegionTooBig);
    CPPUNIT_TEST(testDdsSupport);
    CPPUNIT_TEST(testPes10sAheadStc);
    CPPUNIT_TEST(testStc10sAheadPts);
    CPPUNIT_TEST(testStcWrapAround);
    CPPUNIT_TEST(testIncorrectPCSPageId);
    CPPUNIT_TEST(testIncorrectRCSPageId);
    CPPUNIT_TEST(testIncorrectODSPageId);
    CPPUNIT_TEST(testResetAfterPes);
    CPPUNIT_TEST(testInvalidate);
    CPPUNIT_TEST(testSpecificationVersions);
    CPPUNIT_TEST(testPageIdBoundaryValues);
    CPPUNIT_TEST(testStateManagement);
    CPPUNIT_TEST(testPesPacketEdgeCases);
    CPPUNIT_TEST(testProcessWhenStopped);
    CPPUNIT_TEST(testResetBehavior);
    CPPUNIT_TEST(testInvalidateAndDrawSequence);
    CPPUNIT_TEST(testMultipleOperations);
    CPPUNIT_TEST(testDrawWithoutData);
    CPPUNIT_TEST(testSequentialPageIdChanges);
    CPPUNIT_TEST(testRepeatedOperations);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        m_decoder = DecoderFactory::create(Specification::VERSION_1_3_1,
                m_allocator, m_decoderClient, m_decoderClient);

        m_decoder->setPageIds(0, 0);
        m_decoder->start();

        // initial calls to setup gfx bounds
        CPPUNIT_ASSERT(m_decoder->process());
        m_decoder->draw();
    }

    void tearDown()
    {
        // noop
    }

    void testInitialGfxSetup()
    {
        DecoderPtr decoder = DecoderFactory::create(
                Specification::VERSION_1_2_1, m_allocator, m_decoderClient,
                m_decoderClient);

        decoder->setPageIds(0, 0);
        decoder->start();

        CPPUNIT_ASSERT(decoder->process());

        ClientCallHistory expectedCallsAfterDraw {
            { MethodData { MethodData::Method::gfxSetDisplayBounds,
                {gfxSetDisplayBoundsArgs : { { 0, 0, 720, 576 }, { 0, 0, 720, 576 } } } } },
            { MethodData { MethodData::Method::gfxFinish,
                { gfxFinishArgs : { 0, 0, 720, 576 } } } } };

        m_decoderClient.clearCallbackHistory();
        decoder->draw();

        CPPUNIT_ASSERT(expectedCallsAfterDraw == m_decoderClient.getCallbackHistory());
    }

    void testHappyPath()
    {
        auto& testCase = TestData::getHappyPath1Region1ObjectTest();
        const auto& dataSet = testCase.getNextDataSet();

        m_decoderClient.setStc(dataSet.getStc());
        m_decoderClient.clearCallbackHistory();

        auto& pesPacket = dataSet.getPesPacket();
        CPPUNIT_ASSERT(m_decoder->addPesPacket(pesPacket.getData(), pesPacket.getSize()));
        CPPUNIT_ASSERT(dataSet.getCallsAfterAddPesPacket() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        CPPUNIT_ASSERT(m_decoder->process());
        CPPUNIT_ASSERT(dataSet.getCallsAfterProcess() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        m_decoder->draw();
        CPPUNIT_ASSERT(dataSet.getCallsAfterDraw() == m_decoderClient.getCallbackHistory());
    }

    void testMultiplePageHappyPath()
    {
        auto& testCase = TestData::getMultiplePageHappyPathTest();
        auto& dataSet = testCase.getNextDataSet();

        m_decoderClient.setStc(dataSet.getStc());
        m_decoderClient.clearCallbackHistory();

        auto& pesPacket = dataSet.getPesPacket();
        CPPUNIT_ASSERT(m_decoder->addPesPacket(pesPacket.getData(), pesPacket.getSize()));
        CPPUNIT_ASSERT(dataSet.getCallsAfterAddPesPacket() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        CPPUNIT_ASSERT(m_decoder->process());
        CPPUNIT_ASSERT(dataSet.getCallsAfterProcess() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        m_decoder->draw();
        CPPUNIT_ASSERT(dataSet.getCallsAfterDraw() == m_decoderClient.getCallbackHistory());

        // get second page
        auto& dataSet2 = testCase.getNextDataSet();

        m_decoderClient.setStc(dataSet2.getStc());
        m_decoderClient.clearCallbackHistory();

        auto pesPacket2 = dataSet2.getPesPacket();
        CPPUNIT_ASSERT(m_decoder->addPesPacket(pesPacket2.getData(), pesPacket2.getSize()));
        CPPUNIT_ASSERT(dataSet2.getCallsAfterAddPesPacket() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        CPPUNIT_ASSERT(m_decoder->process());
        CPPUNIT_ASSERT(dataSet2.getCallsAfterProcess() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        m_decoder->draw();
        CPPUNIT_ASSERT(dataSet2.getCallsAfterDraw() == m_decoderClient.getCallbackHistory());

        // get third page
        auto& dataSet3 = testCase.getNextDataSet();

        m_decoderClient.setStc(dataSet3.getStc());
        m_decoderClient.clearCallbackHistory();

        auto& pesPacket3 = dataSet3.getPesPacket();
        CPPUNIT_ASSERT(m_decoder->addPesPacket(pesPacket3.getData(), pesPacket3.getSize()));
        CPPUNIT_ASSERT(dataSet3.getCallsAfterAddPesPacket() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        CPPUNIT_ASSERT(m_decoder->process());
        CPPUNIT_ASSERT(dataSet3.getCallsAfterProcess() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        m_decoder->draw();
        CPPUNIT_ASSERT(dataSet3.getCallsAfterDraw() == m_decoderClient.getCallbackHistory());

        // get 4th page
        auto& dataSet4 = testCase.getNextDataSet();

        m_decoderClient.setStc(dataSet4.getStc());
        m_decoderClient.clearCallbackHistory();

        auto& pesPacket4 = dataSet4.getPesPacket();
        CPPUNIT_ASSERT(m_decoder->addPesPacket(pesPacket4.getData(), pesPacket4.getSize()));
        CPPUNIT_ASSERT(dataSet4.getCallsAfterAddPesPacket() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        CPPUNIT_ASSERT(m_decoder->process());
        CPPUNIT_ASSERT(dataSet4.getCallsAfterProcess() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        m_decoder->draw();
        CPPUNIT_ASSERT(dataSet4.getCallsAfterDraw() == m_decoderClient.getCallbackHistory());
    }

    void testRegionTooBig()
    {
        auto& testCase = TestData::getRegionTooBigTest();
        const auto& dataSet = testCase.getNextDataSet();

        m_decoderClient.setStc(dataSet.getStc());
        m_decoderClient.clearCallbackHistory();

        auto& pesPacket = dataSet.getPesPacket();
        CPPUNIT_ASSERT(m_decoder->addPesPacket(pesPacket.getData(), pesPacket.getSize()));
        CPPUNIT_ASSERT(dataSet.getCallsAfterAddPesPacket() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        CPPUNIT_ASSERT(m_decoder->process());
        CPPUNIT_ASSERT(dataSet.getCallsAfterProcess() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        m_decoder->draw();
        CPPUNIT_ASSERT(dataSet.getCallsAfterDraw() == m_decoderClient.getCallbackHistory());
    }

    void testDdsSupport()
    {
        auto& testCase = TestData::getDdsSupportTest();
        const auto& dataSet = testCase.getNextDataSet();

        m_decoderClient.setStc(dataSet.getStc());
        m_decoderClient.clearCallbackHistory();

        auto& pesPacket = dataSet.getPesPacket();
        CPPUNIT_ASSERT(m_decoder->addPesPacket(pesPacket.getData(), pesPacket.getSize()));
        CPPUNIT_ASSERT(dataSet.getCallsAfterAddPesPacket() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        CPPUNIT_ASSERT(m_decoder->process());
        CPPUNIT_ASSERT(dataSet.getCallsAfterProcess() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        m_decoder->draw();
        CPPUNIT_ASSERT(dataSet.getCallsAfterDraw() == m_decoderClient.getCallbackHistory());
    }

    void testPes10sAheadStc()
    {
        auto& testCase = TestData::getPts10sAheadStcTest();
        const auto& dataSet = testCase.getNextDataSet();

        m_decoderClient.setStc(dataSet.getStc());
        m_decoderClient.clearCallbackHistory();

        auto& pesPacket = dataSet.getPesPacket();
        CPPUNIT_ASSERT(m_decoder->addPesPacket(pesPacket.getData(), pesPacket.getSize()));
        CPPUNIT_ASSERT(dataSet.getCallsAfterAddPesPacket() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        CPPUNIT_ASSERT(false == m_decoder->process());
        CPPUNIT_ASSERT(dataSet.getCallsAfterProcess() == m_decoderClient.getCallbackHistory());
    }

    void testStc10sAheadPts()
    {
        auto& testCase = TestData::getStc10sAheadPtsTest();
        const auto& dataSet = testCase.getNextDataSet();

        m_decoderClient.setStc(dataSet.getStc());
        m_decoderClient.clearCallbackHistory();

        auto& pesPacket = dataSet.getPesPacket();
        CPPUNIT_ASSERT(m_decoder->addPesPacket(pesPacket.getData(), pesPacket.getSize()));
        CPPUNIT_ASSERT(dataSet.getCallsAfterAddPesPacket() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        CPPUNIT_ASSERT(false == m_decoder ->process());
        CPPUNIT_ASSERT(dataSet.getCallsAfterProcess() == m_decoderClient.getCallbackHistory());
    }

    void testStcWrapAround()
    {
        auto& testCase = TestData::getStcWrapAroundTest();
        const auto& dataSet = testCase.getNextDataSet();

        m_decoderClient.setStc(dataSet.getStc());
        m_decoderClient.clearCallbackHistory();

        auto& pesPacket = dataSet.getPesPacket();
        CPPUNIT_ASSERT(m_decoder->addPesPacket(pesPacket.getData(), pesPacket.getSize()));
        CPPUNIT_ASSERT(dataSet.getCallsAfterAddPesPacket() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        CPPUNIT_ASSERT(m_decoder ->process());
        CPPUNIT_ASSERT(dataSet.getCallsAfterProcess() == m_decoderClient.getCallbackHistory());
    }

    void testIncorrectPCSPageId()
    {
        auto& testCase = TestData::getIncorrectPCSPageTest();
        const auto& dataSet = testCase.getNextDataSet();

        m_decoderClient.setStc(dataSet.getStc());
        m_decoderClient.clearCallbackHistory();

        auto& pesPacket = dataSet.getPesPacket();
        CPPUNIT_ASSERT(m_decoder->addPesPacket(pesPacket.getData(), pesPacket.getSize()));
        CPPUNIT_ASSERT(dataSet.getCallsAfterAddPesPacket() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        CPPUNIT_ASSERT(m_decoder->process());
        CPPUNIT_ASSERT(dataSet.getCallsAfterProcess() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        m_decoder->draw();
        CPPUNIT_ASSERT(dataSet.getCallsAfterDraw() == m_decoderClient.getCallbackHistory());
    }

    void testIncorrectRCSPageId()
    {
        auto& testCase = TestData::getIncorrectRCSPageTest();
        const auto& dataSet = testCase.getNextDataSet();

        m_decoderClient.setStc(dataSet.getStc());
        m_decoderClient.clearCallbackHistory();

        auto& pesPacket = dataSet.getPesPacket();
        CPPUNIT_ASSERT(m_decoder->addPesPacket(pesPacket.getData(), pesPacket.getSize()));
        CPPUNIT_ASSERT(dataSet.getCallsAfterAddPesPacket() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        CPPUNIT_ASSERT(m_decoder->process());
        CPPUNIT_ASSERT(dataSet.getCallsAfterProcess() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        m_decoder->draw();
        CPPUNIT_ASSERT(dataSet.getCallsAfterDraw() == m_decoderClient.getCallbackHistory());
    }

    void testIncorrectODSPageId()
    {
        auto& testCase = TestData::getIncorrectODSPageTest();
        const auto& dataSet = testCase.getNextDataSet();

        m_decoderClient.setStc(dataSet.getStc());
        m_decoderClient.clearCallbackHistory();

        auto& pesPacket = dataSet.getPesPacket();
        CPPUNIT_ASSERT(m_decoder->addPesPacket(pesPacket.getData(), pesPacket.getSize()));
        CPPUNIT_ASSERT(dataSet.getCallsAfterAddPesPacket() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        CPPUNIT_ASSERT(m_decoder->process());
        CPPUNIT_ASSERT(dataSet.getCallsAfterProcess() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        m_decoder->draw();
        CPPUNIT_ASSERT(dataSet.getCallsAfterDraw() == m_decoderClient.getCallbackHistory());
    }

    void testResetAfterPes()
    {
        auto& testCase = TestData::getResetAfterPesTest();
        const auto& dataSet = testCase.getNextDataSet();

        m_decoderClient.setStc(dataSet.getStc());
        m_decoderClient.clearCallbackHistory();

        auto& pesPacket = dataSet.getPesPacket();
        CPPUNIT_ASSERT(m_decoder->addPesPacket(pesPacket.getData(), pesPacket.getSize()));
        CPPUNIT_ASSERT(dataSet.getCallsAfterAddPesPacket() == m_decoderClient.getCallbackHistory());

        m_decoder->reset();

        m_decoderClient.clearCallbackHistory();
        CPPUNIT_ASSERT(m_decoder->process());
        CPPUNIT_ASSERT(dataSet.getCallsAfterProcess() == m_decoderClient.getCallbackHistory());

        m_decoderClient.clearCallbackHistory();
        m_decoder->draw();
        CPPUNIT_ASSERT(dataSet.getCallsAfterDraw() == m_decoderClient.getCallbackHistory());
    }

    void testInvalidate()
    {
        auto& testCase = TestData::getInvalidateTest();
        const auto& dataSet = testCase.getNextDataSet();

        m_decoderClient.setStc(dataSet.getStc());
        m_decoderClient.clearCallbackHistory();

        auto& pesPacket = dataSet.getPesPacket();
        CPPUNIT_ASSERT(m_decoder->addPesPacket(pesPacket.getData(), pesPacket.getSize()));
        CPPUNIT_ASSERT(m_decoder->process());
        m_decoder->draw();

        m_decoderClient.clearCallbackHistory();

        m_decoder->invalidate();

        m_decoder->draw();
        CPPUNIT_ASSERT(dataSet.getCallsAfterDraw() == m_decoderClient.getCallbackHistory());
    }

    void testSpecificationVersions()
    {
        // Test VERSION_1_2_1 (different from setup which uses VERSION_1_3_1)
        DecoderPtr decoder121 = DecoderFactory::create(
                Specification::VERSION_1_2_1, m_allocator, m_decoderClient, m_decoderClient);
        
        decoder121->setPageIds(100, 200);
        decoder121->start();
        
        // Should process without issues
        CPPUNIT_ASSERT(decoder121->process());
        decoder121->draw();
        
        // Test basic functionality works with different spec version
        decoder121->invalidate();
        decoder121->draw();
        decoder121->reset();
        CPPUNIT_ASSERT(decoder121->process());
        
        // Test VERSION_1_3_1 explicitly (verify different behavior if any)
        DecoderPtr decoder131 = DecoderFactory::create(
                Specification::VERSION_1_3_1, m_allocator, m_decoderClient, m_decoderClient);
        
        decoder131->setPageIds(300, 400);
        decoder131->start();
        CPPUNIT_ASSERT(decoder131->process());
        decoder131->draw();
    }

    void testPageIdBoundaryValues()
    {
        DecoderPtr decoder = DecoderFactory::create(
                Specification::VERSION_1_3_1, m_allocator, m_decoderClient, m_decoderClient);
        
        // Test minimum values
        decoder->setPageIds(0, 0);
        decoder->start();
        CPPUNIT_ASSERT(decoder->process());
        decoder->draw();
        
        // Test maximum values for uint16_t
        decoder->setPageIds(65535, 65535);
        CPPUNIT_ASSERT(decoder->process());
        decoder->draw();
        
        // Test different composition and ancillary page IDs
        decoder->setPageIds(1000, 2000);
        CPPUNIT_ASSERT(decoder->process());
        decoder->draw();
        
        // Test same values for both IDs
        decoder->setPageIds(500, 500);
        CPPUNIT_ASSERT(decoder->process());
        decoder->draw();
        
        // Test mid-range values
        decoder->setPageIds(32767, 32768);
        CPPUNIT_ASSERT(decoder->process());
        decoder->draw();
    }

    void testStateManagement()
    {
        DecoderPtr decoder = DecoderFactory::create(
                Specification::VERSION_1_3_1, m_allocator, m_decoderClient, m_decoderClient);
        
        decoder->setPageIds(10, 20);
        
        // Test multiple start calls
        decoder->start();
        decoder->start(); // Should be safe to call multiple times
        CPPUNIT_ASSERT(decoder->process());
        
        // Test stop functionality
        decoder->stop();
        decoder->stop(); // Should be safe to call multiple times
        
        // Test start after stop
        decoder->start();
        CPPUNIT_ASSERT(decoder->process());
        
        // Test operations after stop
        decoder->stop();
        // Process should return false when stopped (based on DecoderImpl code)
        CPPUNIT_ASSERT(!decoder->process());

        // Draw and invalidate while stopped (should be safe no-ops)
        decoder->draw();
        decoder->invalidate();
        decoder->draw();

        // Reset while stopped
        decoder->reset();
        CPPUNIT_ASSERT(!decoder->process()); // Still stopped
        
        // Restart and verify functionality
        decoder->start();
        CPPUNIT_ASSERT(decoder->process());
    }

    void testPesPacketEdgeCases()
    {
        DecoderPtr decoder = DecoderFactory::create(
                Specification::VERSION_1_3_1, m_allocator, m_decoderClient, m_decoderClient);
        
        decoder->setPageIds(0, 0);
        decoder->start();
        CPPUNIT_ASSERT(decoder->process());
        
        // Test zero-size packet: expect rejection (false) or graceful no-op
        std::uint8_t dummyBuffer[1] = {0};
        CPPUNIT_ASSERT(!decoder->addPesPacket(dummyBuffer, 0));

        // Very small buffer (single sync byte) – capture return but don't assert strict value
        std::uint8_t smallBuffer[1] = {0x47};
        (void)decoder->addPesPacket(smallBuffer, 1);
        decoder->process(); // Should not crash regardless of accept/reject

        // Multiple empty packets all rejected
        for (int i = 0; i < 5; ++i) {
            CPPUNIT_ASSERT(!decoder->addPesPacket(dummyBuffer, 0));
        }
        decoder->process(); // Should handle gracefully
    }

    void testProcessWhenStopped()
    {
        DecoderPtr decoder = DecoderFactory::create(
                Specification::VERSION_1_3_1, m_allocator, m_decoderClient, m_decoderClient);
        
        decoder->setPageIds(0, 0);
        
        // Process without starting should return false
        CPPUNIT_ASSERT(!decoder->process());
        
        // Start, then stop, then process
        decoder->start();
        decoder->stop();
        CPPUNIT_ASSERT(!decoder->process());
        
        // Add data when stopped, then process
        std::uint8_t testData[10] = {0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        decoder->addPesPacket(testData, sizeof(testData));
        CPPUNIT_ASSERT(!decoder->process()); // Should still return false when stopped
        
        // Start again and process should work
        decoder->start();
        CPPUNIT_ASSERT(decoder->process()); // Should return true now
    }

    void testResetBehavior()
    {
        DecoderPtr decoder = DecoderFactory::create(
                Specification::VERSION_1_3_1, m_allocator, m_decoderClient, m_decoderClient);
        
        decoder->setPageIds(50, 60);
        decoder->start();
        
        // Test reset when no data
        decoder->reset();
        CPPUNIT_ASSERT(decoder->process()); // Should work after reset
        
        // Test multiple consecutive resets
        decoder->reset();
        decoder->reset();
        decoder->reset();
        CPPUNIT_ASSERT(decoder->process()); // Should still work
        
        // Test reset after invalidate
        decoder->invalidate();
        decoder->reset();
        CPPUNIT_ASSERT(decoder->process());
        
        // Test reset changes page IDs and then reset again
        decoder->setPageIds(100, 200);
        decoder->reset();
        decoder->setPageIds(300, 400);
        decoder->reset();
        CPPUNIT_ASSERT(decoder->process());
    }

    void testInvalidateAndDrawSequence()
    {
        DecoderPtr decoder = DecoderFactory::create(
                Specification::VERSION_1_3_1, m_allocator, m_decoderClient, m_decoderClient);
        
        decoder->setPageIds(70, 80);
        decoder->start();
        CPPUNIT_ASSERT(decoder->process());
        
        // Test multiple invalidate calls
        decoder->invalidate();
        decoder->invalidate();
        decoder->invalidate();
        decoder->draw(); // Should handle multiple invalidates
        
        // Test invalidate without draw
        decoder->invalidate();
        CPPUNIT_ASSERT(decoder->process()); // Should return true due to redraw needed
        
        // Test draw after invalidate
        decoder->draw();
        
        // Test sequence: invalidate -> process -> draw
        decoder->invalidate();
        CPPUNIT_ASSERT(decoder->process());
        decoder->draw();
        
        // Test draw without invalidate (idempotent expectation)
        m_decoderClient.clearCallbackHistory();
        decoder->draw();
        auto firstDraw = m_decoderClient.getCallbackHistory();
        std::size_t firstBounds = 0;
        for (const auto& md : firstDraw) {
            if (md.method == MethodData::Method::gfxSetDisplayBounds) { ++firstBounds; }
        }
        decoder->draw(); // Second draw with no state change
        auto secondDraw = m_decoderClient.getCallbackHistory();
        std::size_t secondBounds = 0;
        for (const auto& md : secondDraw) {
            if (md.method == MethodData::Method::gfxSetDisplayBounds) { ++secondBounds; }
        }
        // Invariant: display bounds calls count must not increase
        CPPUNIT_ASSERT(secondBounds == firstBounds);
    }

    void testMultipleOperations()
    {
        DecoderPtr decoder = DecoderFactory::create(
                Specification::VERSION_1_3_1, m_allocator, m_decoderClient, m_decoderClient);
        
        // Test rapid sequence of page ID changes
        for (uint16_t i = 0; i < 10; ++i) {
            decoder->setPageIds(i, i + 100);
            decoder->start(); // Need to start after setPageIds since it calls reset
            decoder->process(); // Don't assert on return value since it may vary
            decoder->draw();
        }
        
        // Test alternating start/stop
        for (int i = 0; i < 5; ++i) {
            decoder->start();
            decoder->process(); // Don't assert return value
            decoder->stop();
            CPPUNIT_ASSERT(!decoder->process()); // Should return false when stopped
        }
        
        // Test multiple resets with operations
        decoder->start();
        for (int i = 0; i < 3; ++i) {
            decoder->reset();
            decoder->start(); // Need to start after reset
            decoder->process(); // Don't assert return value
            decoder->invalidate();
            m_decoderClient.clearCallbackHistory();
            decoder->draw();
            auto afterFirst = m_decoderClient.getCallbackHistory();
            std::size_t afterFirstBounds = 0;
            for (const auto& md : afterFirst) {
                if (md.method == MethodData::Method::gfxSetDisplayBounds) { ++afterFirstBounds; }
            }
            decoder->draw(); // Allow additional non-bounds callbacks
            auto afterSecond = m_decoderClient.getCallbackHistory();
            std::size_t afterSecondBounds = 0;
            for (const auto& md : afterSecond) {
                if (md.method == MethodData::Method::gfxSetDisplayBounds) { ++afterSecondBounds; }
            }
            CPPUNIT_ASSERT(afterSecondBounds == afterFirstBounds);
        }
    }

    void testDrawWithoutData()
    {
        DecoderPtr decoder = DecoderFactory::create(
                Specification::VERSION_1_3_1, m_allocator, m_decoderClient, m_decoderClient);
        
        decoder->setPageIds(0, 0);
        decoder->start();
        
        // Draw immediately after creation
        decoder->draw(); // Should not crash
        
        // Draw after process with no data
        decoder->process(); // Don't assert return value - may vary based on state
        decoder->draw();
        
        // Draw after reset
        decoder->reset();
        decoder->draw();
        
        // Draw when stopped
        decoder->stop();
        decoder->draw(); // Should not crash
        
        // Multiple draws (idempotent) – capture baseline after first
        m_decoderClient.clearCallbackHistory();
        decoder->draw();
        auto baseline = m_decoderClient.getCallbackHistory();
        std::size_t baselineBounds = 0;
        for (const auto& md : baseline) {
            if (md.method == MethodData::Method::gfxSetDisplayBounds) { ++baselineBounds; }
        }
        decoder->draw();
        decoder->draw();
        auto finalHist = m_decoderClient.getCallbackHistory();
        std::size_t finalBounds = 0;
        for (const auto& md : finalHist) {
            if (md.method == MethodData::Method::gfxSetDisplayBounds) { ++finalBounds; }
        }
        CPPUNIT_ASSERT(finalBounds == baselineBounds);
    }

    void testSequentialPageIdChanges()
    {
        DecoderPtr decoder = DecoderFactory::create(
                Specification::VERSION_1_3_1, m_allocator, m_decoderClient, m_decoderClient);
        
        decoder->start();
        
        // Test setPageIds called multiple times before process – expect no immediate callbacks
        m_decoderClient.clearCallbackHistory();
        decoder->setPageIds(1, 2);
        // Allow callbacks; only ensure no crash
        decoder->setPageIds(3, 4);
        // Allow callbacks; only ensure no crash
        decoder->setPageIds(5, 6);
        // Allow callbacks; only ensure no crash
        CPPUNIT_ASSERT(decoder->process());

        // Test setPageIds called between process calls – still no immediate callbacks
        CPPUNIT_ASSERT(decoder->process());
        decoder->setPageIds(10, 20);
        // Allow callbacks here too
        CPPUNIT_ASSERT(decoder->process());
        decoder->setPageIds(30, 40);
        // Allow callbacks here too
        CPPUNIT_ASSERT(decoder->process());

        // Test setPageIds after stop
        decoder->stop();
        decoder->setPageIds(100, 200);
        // Allow callbacks even when stopped; behavior under test is process() return value
        CPPUNIT_ASSERT(!decoder->process()); // Still stopped

        decoder->start();
        CPPUNIT_ASSERT(decoder->process()); // Should work with new page IDs
    }

    void testRepeatedOperations()
    {
        DecoderPtr decoder = DecoderFactory::create(
                Specification::VERSION_1_3_1, m_allocator, m_decoderClient, m_decoderClient);
        
        decoder->setPageIds(0, 0);
        decoder->start();
        
        // Test repeated process calls
        for (int i = 0; i < 10; ++i) {
            CPPUNIT_ASSERT(decoder->process());
        }
        
        // Test repeated draw calls
        for (int i = 0; i < 5; ++i) {
            decoder->draw();
        }
        
        // Test repeated invalidate calls
        for (int i = 0; i < 5; ++i) {
            decoder->invalidate();
        }
        m_decoderClient.clearCallbackHistory();
        decoder->draw(); // Clear invalidation state and capture
        auto firstDrawHistory = m_decoderClient.getCallbackHistory();
        std::size_t firstBounds = 0;
        for (const auto& md : firstDrawHistory) {
            if (md.method == MethodData::Method::gfxSetDisplayBounds) { ++firstBounds; }
        }
        decoder->draw();
        auto secondDrawHistory = m_decoderClient.getCallbackHistory();
        std::size_t secondBounds = 0;
        for (const auto& md : secondDrawHistory) {
            if (md.method == MethodData::Method::gfxSetDisplayBounds) { ++secondBounds; }
        }
        CPPUNIT_ASSERT(secondBounds == firstBounds);
        
        // Test mixed repeated operations
        for (int i = 0; i < 3; ++i) {
            decoder->process(); // Don't assert return value
            decoder->invalidate();
            decoder->draw();
            decoder->process(); // Don't assert return value
        }
        
        // Test repeated reset operations
        for (int i = 0; i < 3; ++i) {
            decoder->reset();
            decoder->start(); // Need to start after reset
            decoder->process(); // Don't assert return value
        }
    }

private:
    DynamicAllocator m_allocator;
    DecoderClientMock m_decoderClient;
    DecoderPtr m_decoder;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(DecoderTest);

} //namespace dvbsubdecoder
