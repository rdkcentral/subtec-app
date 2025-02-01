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


#include "TestCaseData.hpp"
#include "PixelStringWriter.hpp"

namespace DataSetBasic
{
// default bounds where no dds is present
static constexpr Rectangle DEFAULT_GFX_BOUNDS = { 0, 0, 720, 576 };

const std::uint16_t PAGE_ID = 0;
const PageCompositionSegment::Region region = { 1, 100, 100 };
const RegionCompositionSegment::Object object = { 1, 10, 10 };
constexpr std::uint32_t pts = 100 * 90000;

PixelStringWriter createPixelData()
{
    PixelStringWriter pixelData;

    pixelData.start2bitPixelCodeString();
    pixelData.write2bitPixels(0, 16);
    pixelData.end2bitPixelCodeString();

    return pixelData;
}

PixelStringWriter pixelData = createPixelData();

DataDefinitionSegment dds(PAGE_ID,  //        std::uint16_t pageId,
                          0,        //        std::uint8_t versionNumber,
                          false,    //        bool displayWindowFlag,
                          2047,     //        std::uint16_t displayWidth,
                          2047      //        std::uint16_t displayHeight,
                                    //        std::uint16_t horizontalMin = 0,
                                    //        std::uint16_t horizontalMax = 0,
                                    //        std::uint16_t verticalMin = 0,
                                    //        std::uint16_t verticalMax = 0
                          );

PageCompositionSegment pcs(PAGE_ID,     //            std::uint16_t pageId,
        0,                              //            std::uint8_t pageTimeout,
        0,                              //            std::uint8_t versionNumber,
        0x2,                            //            std::uint8_t pageState,     0x2 mode change
        { { region } }                  //            const std::vector<Region>& regions);
        );

RegionCompositionSegment rds(PAGE_ID,   //            std::uint16_t pageId,
        region.m_regionId,              //            std::uint8_t regionId,
        0,                              //            std::uint8_t versionNumber,
        true,                           //            bool fillFlag,
        50,                             //            std::uint16_t regionWidth,
        50,                             //            std::uint16_t regionHeight,
        0x1,                            //            std::uint8_t levelOfCompatibility,
        0x1,                            //            std::uint8_t regionDepth,
        0x0,                            //            std::uint8_t clutId = 0x0,
        0x0,                            //            std::uint8_t region8bPixelCode = 0x0,
        0x0,                            //            std::uint8_t region4bPixelCode = 0x0,
        0x1,                            //            std::uint8_t region2bPixelCode = 0x1,
        { { object } });

RegionCompositionSegment rdsTooBig(PAGE_ID,   //            std::uint16_t pageId,
        region.m_regionId,              //            std::uint8_t regionId,
        0,                              //            std::uint8_t versionNumber,
        true,                           //            bool fillFlag,
        1024,                             //            std::uint16_t regionWidth,
        1024,                             //            std::uint16_t regionHeight,
        0x1,                            //            std::uint8_t levelOfCompatibility,
        0x1,                            //            std::uint8_t regionDepth,
        0x0,                            //            std::uint8_t clutId = 0x0,
        0x0,                            //            std::uint8_t region8bPixelCode = 0x0,
        0x0,                            //            std::uint8_t region4bPixelCode = 0x0,
        0x1,                            //            std::uint8_t region2bPixelCode = 0x1,
        { { object } });

ObjectDataSegment ods(PAGE_ID,          //            std::uint16_t pageId,
        object.m_objectId,              //            std::uint8_t objectId,
        0x1,                            //            std::uint8_t versionNumber,
        0x0,                            //            std::uint8_t codingMethod,
        false,                          //            bool nonModyfingFlag,
        pixelData.size(),               //            std::uint16_t topFieldLength,
        0,                              //            std::uint16_t bottomFieldLength,
        pixelData.data(),               //            std::uint8_t* topFieldPixelData,
        nullptr                         //            std::uint8_t* bottomFieldPixelData) :
        );

ClientCallHistory callsAfterAddPesPacket { };
ClientCallHistory callsAfterProcess { { MethodData::Method::getStc } };
ClientCallHistory callsAfterDraw {
    { MethodData { MethodData::Method::gfxDraw,
        { gfxDrawArgs : { { 50, 50, nullptr, nullptr }, { 0, 0, 50, 50 }, { 100, 100, 150, 150 } } } } },
    { MethodData { MethodData::Method::gfxFinish, {
        gfxFinishArgs : { 100, 100, 150, 150 } } } } };

}   // namespace DataSet1


namespace DataSetExtended
{

const PageCompositionSegment::Region region2 = { 2, 400, 400 };

const auto pts2 = DataSetBasic::pts + 5 * 90000;

PageCompositionSegment pcs2(DataSetBasic::PAGE_ID,    //            std::uint16_t pageId,
        0,                              //            std::uint8_t pageTimeout,
        1,                              //            std::uint8_t versionNumber,
        0x0,                            //            std::uint8_t pageState,     0x0 normal case
        { { DataSetBasic::region, region2 } }         //            const std::vector<Region>& regions);
        );

RegionCompositionSegment rds2(DataSetBasic::PAGE_ID, //            std::uint16_t pageId,
        region2.m_regionId,             //            std::uint8_t regionId,
        0,                              //            std::uint8_t versionNumber,
        true,                           //            bool fillFlag,
        75,                             //            std::uint16_t regionWidth,
        75,                             //            std::uint16_t regionHeight,
        0x1,                            //            std::uint8_t levelOfCompatibility,
        0x1,                            //            std::uint8_t regionDepth,
        0x0,                            //            std::uint8_t clutId = 0x0,
        0x0,                            //            std::uint8_t region8bPixelCode = 0x0,
        0x0,                            //            std::uint8_t region4bPixelCode = 0x0,
        0x1,                            //            std::uint8_t region2bPixelCode = 0x1,
        { { DataSetBasic::object } });

ClientCallHistory callsAfterPage2AddPesPacket { };
ClientCallHistory callsAfterPage2Process { { MethodData::Method::getStc } };
ClientCallHistory callsAfterPage2Draw {
    { MethodData { MethodData::Method::gfxDraw,
        { gfxDrawArgs : { { 75, 75, nullptr, nullptr }, { 0, 0, 75, 75 }, { 400, 400, 475, 475 } } } } },
    { MethodData { MethodData::Method::gfxFinish, {
        gfxFinishArgs : { 400, 400, 475, 475 } } } } };

const auto pts3 = pts2 + 5 * 90000;

PageCompositionSegment pcs3(DataSetBasic::PAGE_ID,    //            std::uint16_t pageId,
        0,                              //            std::uint8_t pageTimeout,
        3,                              //            std::uint8_t versionNumber,
        0x0,                            //            std::uint8_t pageState,     0x0 normal case
        { { region2 } }                 //            const std::vector<Region>& regions);
        );

ClientCallHistory callsAfterPage3AddPesPacket { };
ClientCallHistory callsAfterPage3Process { { MethodData::Method::getStc } };
ClientCallHistory callsAfterPage3Draw {
    { MethodData { MethodData::Method::gfxClear, { gfxClearArgs : { 100, 100, 150, 150 } } } },
    { MethodData { MethodData::Method::gfxFinish, { gfxFinishArgs : { 100, 100, 150, 150 } } } } };

const auto pts4 = pts3 + 5 * 90000;

PageCompositionSegment pcs4(DataSetBasic::PAGE_ID,    //            std::uint16_t pageId,
        0,                              //            std::uint8_t pageTimeout,
        0,                              //            std::uint8_t versionNumber,
        0x2,                            //            std::uint8_t pageState,     0x2 mode change
        {}                              //            const std::vector<Region>& regions);
        );

ClientCallHistory callsAfterPage4AddPesPacket { };
ClientCallHistory callsAfterPage4Process { { MethodData::Method::getStc } };
ClientCallHistory callsAfterPage4Draw {
    { MethodData { MethodData::Method::gfxClear, { gfxClearArgs : { 400, 400, 475, 475 } } } },
    { MethodData { MethodData::Method::gfxFinish, { gfxFinishArgs : { 400, 400, 475, 475 } } } } };


}   // namespace DataSet1

void TestData::defaultInit()
{
    m_currentDataSet.m_pesBuilder.init(DataSetBasic::pts);
    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::pcs);
    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::rds);
    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::ods);
    m_currentDataSet.m_pesBuilder.addSegment(EndOfDisplaySetSegment(DataSetBasic::PAGE_ID));
    m_currentDataSet.m_pesBuilder.finalize();

    m_currentDataSet.m_stcTime.m_type = StcTimeType::LOW_32;
    m_currentDataSet.m_stcTime.m_time = DataSetBasic::pts;

    m_currentDataSet.m_callsAfterAddPesPacket = DataSetBasic::callsAfterAddPesPacket;
    m_currentDataSet.m_callsAfterProcess = DataSetBasic::callsAfterProcess;
    m_currentDataSet.m_callsAfterDraw = DataSetBasic::callsAfterDraw;
}

TestData::TestData(InitFunction initFunction) :
            m_currentDataSet(),
            m_dataSets(),
            m_currentDataSetIt()
{
    (this->*initFunction)();
    m_currentDataSetIt = m_dataSets.begin();
}

TestData& TestData::getHappyPath1Region1ObjectTest()
{
    static TestData happyPathInstance(&TestData::defaultInit);
    return happyPathInstance;
}

TestData& TestData::getMultiplePageHappyPathTest()
{
    static TestData multiplePageHappyPathInstance(&TestData::initMultiplePageHappyPathTest);
    return multiplePageHappyPathInstance;
}

void TestData::initMultiplePageHappyPathTest()
{
    DataSet dataSet;

    dataSet.m_pesBuilder.init(DataSetBasic::pts);
    dataSet.m_pesBuilder.addSegment(DataSetBasic::pcs);
    dataSet.m_pesBuilder.addSegment(DataSetBasic::rds);
    dataSet.m_pesBuilder.addSegment(DataSetExtended::rds2);
    dataSet.m_pesBuilder.addSegment(DataSetBasic::ods);
    dataSet.m_pesBuilder.addSegment(EndOfDisplaySetSegment(DataSetBasic::PAGE_ID));
    dataSet.m_pesBuilder.finalize();

    dataSet.m_stcTime.m_type = StcTimeType::LOW_32;
    dataSet.m_stcTime.m_time = DataSetBasic::pts;

    dataSet.m_callsAfterAddPesPacket = DataSetBasic::callsAfterAddPesPacket;
    dataSet.m_callsAfterProcess = DataSetBasic::callsAfterProcess;
    dataSet.m_callsAfterDraw = DataSetBasic::callsAfterDraw;

    m_dataSets.push_back(dataSet);

    DataSet dataSet2;

    dataSet2.m_pesBuilder.init(DataSetExtended::pts2);
    dataSet2.m_pesBuilder.addSegment(DataSetExtended::pcs2);
    dataSet2.m_pesBuilder.addSegment(DataSetBasic::rds);
    dataSet2.m_pesBuilder.addSegment(DataSetExtended::rds2);
    dataSet2.m_pesBuilder.addSegment(DataSetBasic::ods);
    dataSet2.m_pesBuilder.addSegment(EndOfDisplaySetSegment(DataSetBasic::PAGE_ID));
    dataSet2.m_pesBuilder.finalize();

    dataSet2.m_stcTime.m_type = StcTimeType::LOW_32;
    dataSet2.m_stcTime.m_time = DataSetExtended::pts2;

    dataSet2.m_callsAfterAddPesPacket = DataSetExtended::callsAfterPage2AddPesPacket;
    dataSet2.m_callsAfterProcess = DataSetExtended::callsAfterPage2Process;
    dataSet2.m_callsAfterDraw = DataSetExtended::callsAfterPage2Draw;

    m_dataSets.push_back(dataSet2);

    DataSet dataSet3;

    dataSet3.m_pesBuilder.init(DataSetExtended::pts3);
    dataSet3.m_pesBuilder.addSegment(DataSetExtended::pcs3);
    dataSet3.m_pesBuilder.addSegment(DataSetExtended::rds2);
    dataSet3.m_pesBuilder.addSegment(DataSetBasic::ods);
    dataSet3.m_pesBuilder.addSegment(EndOfDisplaySetSegment(DataSetBasic::PAGE_ID));
    dataSet3.m_pesBuilder.finalize();

    dataSet3.m_stcTime.m_type = StcTimeType::LOW_32;
    dataSet3.m_stcTime.m_time = DataSetExtended::pts3;

    dataSet3.m_callsAfterAddPesPacket = DataSetExtended::callsAfterPage3AddPesPacket;
    dataSet3.m_callsAfterProcess = DataSetExtended::callsAfterPage3Process;
    dataSet3.m_callsAfterDraw = DataSetExtended::callsAfterPage3Draw;

    m_dataSets.push_back(dataSet3);

    DataSet dataSet4;

    dataSet4.m_pesBuilder.init(DataSetExtended::pts4);
    dataSet4.m_pesBuilder.addSegment(DataSetExtended::pcs4);
    dataSet4.m_pesBuilder.addSegment(EndOfDisplaySetSegment(DataSetBasic::PAGE_ID));
    dataSet4.m_pesBuilder.finalize();

    dataSet4.m_stcTime.m_type = StcTimeType::LOW_32;
    dataSet4.m_stcTime.m_time = DataSetExtended::pts4;

    dataSet4.m_callsAfterAddPesPacket = DataSetExtended::callsAfterPage4AddPesPacket;
    dataSet4.m_callsAfterProcess = DataSetExtended::callsAfterPage4Process;
    dataSet4.m_callsAfterDraw = DataSetExtended::callsAfterPage4Draw;

    m_dataSets.push_back(dataSet4);
}

TestData& TestData::getRegionTooBigTest()
{
    static TestData ptsFarAheadStcInstance(&TestData::initRegionTooBigTest);
    return ptsFarAheadStcInstance;

}

void TestData::initRegionTooBigTest()
{
    m_currentDataSet.m_pesBuilder.init(DataSetBasic::pts);
    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::pcs);
    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::rdsTooBig);
    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::ods);
    m_currentDataSet.m_pesBuilder.addSegment(EndOfDisplaySetSegment(DataSetBasic::PAGE_ID));
    m_currentDataSet.m_pesBuilder.finalize();

    m_currentDataSet.m_stcTime.m_type = StcTimeType::LOW_32;
    m_currentDataSet.m_stcTime.m_time = DataSetBasic::pts;

    m_currentDataSet.m_callsAfterAddPesPacket = DataSetBasic::callsAfterAddPesPacket;
    m_currentDataSet.m_callsAfterProcess = DataSetBasic::callsAfterProcess;
    m_currentDataSet.m_callsAfterDraw = {
        { MethodData { MethodData::Method::gfxFinish, { gfxFinishArgs : { 0, 0, 0, 0 } } } } };
}

TestData& TestData::getDdsSupportTest()
{
    static TestData regionTooBigInstance(&TestData::initDdsSupportTest);
    return regionTooBigInstance;
}

void TestData::initDdsSupportTest()
{
    m_currentDataSet.m_pesBuilder.init(DataSetBasic::pts);
    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::dds);
    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::pcs);
    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::rds);
    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::ods);
    m_currentDataSet.m_pesBuilder.addSegment(EndOfDisplaySetSegment(DataSetBasic::PAGE_ID));
    m_currentDataSet.m_pesBuilder.finalize();

    m_currentDataSet.m_stcTime.m_type = StcTimeType::LOW_32;
    m_currentDataSet.m_stcTime.m_time = DataSetBasic::pts;

    m_currentDataSet.m_callsAfterAddPesPacket = DataSetBasic::callsAfterAddPesPacket;
    m_currentDataSet.m_callsAfterProcess = DataSetBasic::callsAfterProcess;
    m_currentDataSet.m_callsAfterDraw = {
            { MethodData { MethodData::Method::gfxFinish,
                { gfxFinishArgs : { 0, 0, 0, 0 } } } } };

}

TestData& TestData::getPts10sAheadStcTest()
{
    static TestData ptsFarAheadStcInstance(&TestData::initPtsFarAheadStcTest);
    return ptsFarAheadStcInstance;
}

void TestData::initPtsFarAheadStcTest()
{
    defaultInit();
    m_currentDataSet.m_stcTime.m_time -= 10 * 90000;
}

TestData& TestData::getStc10sAheadPtsTest()
{
    static TestData stcFarAheadPtsInstance(&TestData::initStc10sAheadPtsTest);
    return stcFarAheadPtsInstance;
}

void TestData::initStc10sAheadPtsTest()
{
    defaultInit();
    m_currentDataSet.m_stcTime.m_time += 10 * 90000;
}

TestData& TestData::getStcWrapAroundTest()
{
    static TestData stcWrapAroundInstance(&TestData::initStcWrapAroundTest);
    return stcWrapAroundInstance;
}

void TestData::initStcWrapAroundTest()
{
    const std::uint32_t ptsAfterWrap = UINT32_MAX + (10 * 90); // 10 ms after wrap
    const std::uint32_t stcBeforeWrap = UINT32_MAX - (20 * 90);

    m_currentDataSet.m_pesBuilder.init(ptsAfterWrap);
    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::pcs);
    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::rds);
    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::ods);
    m_currentDataSet.m_pesBuilder.addSegment(EndOfDisplaySetSegment(DataSetBasic::PAGE_ID));
    m_currentDataSet.m_pesBuilder.finalize();

    m_currentDataSet.m_stcTime.m_type = StcTimeType::LOW_32;
    m_currentDataSet.m_stcTime.m_time = stcBeforeWrap;

    m_currentDataSet.m_callsAfterAddPesPacket = DataSetBasic::callsAfterAddPesPacket;
    m_currentDataSet.m_callsAfterProcess = DataSetBasic::callsAfterProcess;
    m_currentDataSet.m_callsAfterDraw = DataSetBasic::callsAfterDraw;
}

TestData& TestData::getIncorrectPCSPageTest()
{
    static TestData incorrectPCSPageTest(&TestData::initIncorrectPCSPageTest);
    return incorrectPCSPageTest;
}

void TestData::initIncorrectPCSPageTest()
{
    auto incorrectPageId = DataSetBasic::PAGE_ID + 1;

    PageCompositionSegment invalidPagePcs(incorrectPageId, 0, 0, 0x2, { { DataSetBasic::region } });

    m_currentDataSet.m_pesBuilder.init(DataSetBasic::pts);

    m_currentDataSet.m_pesBuilder.addSegment(invalidPagePcs);
    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::rds);
    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::ods);
    m_currentDataSet.m_pesBuilder.addSegment(EndOfDisplaySetSegment(DataSetBasic::PAGE_ID));
    m_currentDataSet.m_pesBuilder.finalize();

    m_currentDataSet.m_stcTime.m_type = StcTimeType::LOW_32;
    m_currentDataSet.m_stcTime.m_time = DataSetBasic::pts;

    m_currentDataSet.m_callsAfterAddPesPacket = {};
    m_currentDataSet.m_callsAfterProcess = { { MethodData::Method::getStc} };
    m_currentDataSet.m_callsAfterDraw = { MethodData { MethodData::Method::gfxFinish,{ gfxFinishArgs : { 0, 0, 0, 0}}}};
}

TestData& TestData::getIncorrectRCSPageTest()
{
    static TestData incorrectRCSPageTest(&TestData::initIncorrectRCSPageTest);
    return incorrectRCSPageTest;
}

void TestData::initIncorrectRCSPageTest()
{
    auto incorrectPageId = DataSetBasic::PAGE_ID + 1;

    RegionCompositionSegment incorrectPageRds(incorrectPageId, DataSetBasic::region.m_regionId, 0, true, 50, 50, 0x1, 0x1,
            0x0, 0x0, 0x0, 0x1, { { DataSetBasic::object } });

    m_currentDataSet.m_pesBuilder.init(DataSetBasic::pts);

    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::pcs);
    m_currentDataSet.m_pesBuilder.addSegment(incorrectPageRds);
    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::ods);
    m_currentDataSet.m_pesBuilder.addSegment(EndOfDisplaySetSegment(DataSetBasic::PAGE_ID));
    m_currentDataSet.m_pesBuilder.finalize();

    m_currentDataSet.m_stcTime.m_type = StcTimeType::LOW_32;
    m_currentDataSet.m_stcTime.m_time = DataSetBasic::pts;

    m_currentDataSet.m_callsAfterAddPesPacket = {};
    m_currentDataSet.m_callsAfterProcess = {{MethodData::Method::getStc}};
    m_currentDataSet.m_callsAfterDraw = {MethodData { MethodData::Method::gfxFinish, { gfxFinishArgs : {   0, 0, 0, 0 }}}};
}

TestData& TestData::getIncorrectODSPageTest()
{
    static TestData incorrectODSPageTestInstance(&TestData::initIncorrectODSPageTest);
    return incorrectODSPageTestInstance;
}

void TestData::initIncorrectODSPageTest()
{
    auto incorrectPageId = DataSetBasic::PAGE_ID + 1;

    ObjectDataSegment incorrectPageOds(incorrectPageId, DataSetBasic::object.m_objectId, 0x1, 0x0, false,
                                       DataSetBasic::pixelData.size(), 0, DataSetBasic::pixelData.data(), nullptr);

    m_currentDataSet.m_pesBuilder.init(DataSetBasic::pts);

    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::pcs);
    m_currentDataSet.m_pesBuilder.addSegment(DataSetBasic::rds);
    m_currentDataSet.m_pesBuilder.addSegment(incorrectPageOds);
    m_currentDataSet.m_pesBuilder.addSegment(EndOfDisplaySetSegment(DataSetBasic::PAGE_ID));
    m_currentDataSet.m_pesBuilder.finalize();

    m_currentDataSet.m_stcTime.m_type = StcTimeType::LOW_32;
    m_currentDataSet.m_stcTime.m_time = DataSetBasic::pts;

    m_currentDataSet.m_callsAfterAddPesPacket = DataSetBasic::callsAfterAddPesPacket;
    m_currentDataSet.m_callsAfterProcess = DataSetBasic::callsAfterProcess;
    m_currentDataSet.m_callsAfterDraw = DataSetBasic::callsAfterDraw;
}

TestData& TestData::getResetAfterPesTest()
{
    static TestData resetAfterPesTest(&TestData::initResetAfterPesTest);
    return resetAfterPesTest;
}

void TestData::initResetAfterPesTest()
{
    defaultInit();
    m_currentDataSet.m_callsAfterDraw = {
            { MethodData { MethodData::Method::gfxSetDisplayBounds,
                { gfxSetDisplayBoundsArgs : { DataSetBasic::DEFAULT_GFX_BOUNDS , DataSetBasic::DEFAULT_GFX_BOUNDS } } } },
            { MethodData { MethodData::Method::gfxFinish,
                { gfxFinishArgs :  { DataSetBasic::DEFAULT_GFX_BOUNDS } } } } };
}

TestData& TestData::getInvalidateTest()
{
    static TestData invalidateTest(&TestData::initInvalidateTest);
    return invalidateTest;
}

void TestData::initInvalidateTest()
{
    defaultInit();
    m_currentDataSet.m_callsAfterDraw = {
            { MethodData { MethodData::Method::gfxSetDisplayBounds,
                { gfxSetDisplayBoundsArgs : { DataSetBasic::DEFAULT_GFX_BOUNDS , DataSetBasic::DEFAULT_GFX_BOUNDS } } } },
            { MethodData { MethodData::Method::gfxDraw,
                { gfxDrawArgs : { { 50, 50, nullptr, nullptr }, { 0, 0, 50, 50 }, { 100, 100, 150, 150 } } } } },
            { MethodData { MethodData::Method::gfxFinish,
                { gfxFinishArgs :  { DataSetBasic::DEFAULT_GFX_BOUNDS } } } } };
}

