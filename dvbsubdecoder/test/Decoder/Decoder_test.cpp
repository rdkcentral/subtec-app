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

private:
    DynamicAllocator m_allocator;
    DecoderClientMock m_decoderClient;
    DecoderPtr m_decoder;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(DecoderTest);

} //namespace dvbsubdecoder
