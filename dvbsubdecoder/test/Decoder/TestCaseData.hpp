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


#ifndef DVBSUBDECODER_TEST_PESPACKETS_TESTCASEDATA_HPP
#define DVBSUBDECODER_TEST_PESPACKETS_TESTCASEDATA_HPP

#include <array>
#include <map>
#include <vector>
#include <memory>

#include "DecoderClientMock.hpp"
#include "PesPackets/PesBuilder.hpp"

class TestData
{
public:

    struct DataSet
    {
        PesBuilder m_pesBuilder;
        dvbsubdecoder::StcTime m_stcTime;

        ClientCallHistory m_callsAfterAddPesPacket;
        ClientCallHistory m_callsAfterProcess;
        ClientCallHistory m_callsAfterDraw;

        const ClientCallHistory& getCallsAfterAddPesPacket() const
        {
            return m_callsAfterAddPesPacket;
        }

        const ClientCallHistory& getCallsAfterDraw() const
        {
            return m_callsAfterDraw;
        }

        const ClientCallHistory& getCallsAfterProcess() const
        {
            return m_callsAfterProcess;
        }

        const PesBuilder& getPesPacket() const
        {
            return m_pesBuilder;
        }

        dvbsubdecoder::StcTime getStc() const
        {
            return m_stcTime;
        }
    };

    static TestData& getHappyPath1Region1ObjectTest();
    static TestData& getMultiplePageHappyPathTest();
    static TestData& getRegionTooBigTest();
    static TestData& getDdsSupportTest();
    static TestData& getPts10sAheadStcTest();
    static TestData& getStc10sAheadPtsTest();
    static TestData& getStcWrapAroundTest();
    static TestData& getIncorrectPCSPageTest();
    static TestData& getIncorrectRCSPageTest();
    static TestData& getIncorrectODSPageTest();
    static TestData& getResetAfterPesTest();
    static TestData& getInvalidateTest();

    const TestData::DataSet& getNextDataSet()
    {
        if (m_currentDataSetIt != m_dataSets.end())
        {
            m_currentDataSet = *m_currentDataSetIt;
            m_currentDataSetIt++;
        }
        return m_currentDataSet;
    }

private:

    using InitFunction = void (TestData::*)(void);

    TestData(InitFunction initFunction = &TestData::defaultInit);

    void defaultInit();

    void initMultiplePageHappyPathTest();
    void initRegionTooBigTest();
    void initDdsSupportTest();
    void initPtsFarAheadStcTest();
    void initStc10sAheadPtsTest();
    void initStcWrapAroundTest();
    void initIncorrectPCSPageTest();
    void initIncorrectRCSPageTest();
    void initIncorrectODSPageTest();
    void initResetAfterPesTest();
    void initInvalidateTest();

    DataSet m_currentDataSet;

    std::vector<DataSet> m_dataSets;
    std::vector<DataSet>::iterator m_currentDataSetIt;
};

#endif /* DVBSUBDECODER_TEST_PESPACKETS_TESTCASEDATA_HPP */
