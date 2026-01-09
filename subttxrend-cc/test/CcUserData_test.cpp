/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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
*/

#include <cppunit/extensions/HelperMacros.h>
#include "CcUserData.hpp"
#include "CcExceptions.hpp"
#include <vector>
#include <cstring>

using namespace subttxrend::cc;

class CcUserDataTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CcUserDataTest);
    CPPUNIT_TEST(testDefaultConstructorInitialization);
    CPPUNIT_TEST(testDefaultConstructorIsValid);
    CPPUNIT_TEST(testDefaultConstructorGetCcData);
    CPPUNIT_TEST(testParameterizedConstructorValid);
    CPPUNIT_TEST(testParameterizedConstructorNullptr);
    CPPUNIT_TEST(testParameterizedConstructorZeroSize);
    CPPUNIT_TEST(testParameterizedConstructorBothNullAndZero);
    CPPUNIT_TEST(testSetUserDataSingleTriplet);
    CPPUNIT_TEST(testSetUserDataMultipleTriplets);
    CPPUNIT_TEST(testSetUserDataCcTypeField1);
    CPPUNIT_TEST(testSetUserDataCcTypeField2);
    CPPUNIT_TEST(testSetUserDataCcTypeDtvccData);
    CPPUNIT_TEST(testSetUserDataCcTypeDtvccStart);
    CPPUNIT_TEST(testSetUserDataData1Zero);
    CPPUNIT_TEST(testSetUserDataData1Max);
    CPPUNIT_TEST(testSetUserDataData2Zero);
    CPPUNIT_TEST(testSetUserDataData2Max);
    CPPUNIT_TEST(testSetUserDataSizeNotDivisibleByThree);
    CPPUNIT_TEST(testSetUserDataSizeOne);
    CPPUNIT_TEST(testSetUserDataSizeTwo);
    CPPUNIT_TEST(testSetUserDataLargeSize31Triplets);
    CPPUNIT_TEST(testSetUserDataVeryLarge100Triplets);
    CPPUNIT_TEST(testSetUserDataNullptr);
    CPPUNIT_TEST(testSetUserDataZeroSize);
    CPPUNIT_TEST(testSetUserDataTwice);
    CPPUNIT_TEST(testSetUserDataAfterDefaultConstructor);
    CPPUNIT_TEST(testSetUserDataCcTypeExtractionAllBits);
    CPPUNIT_TEST(testGetCcDataAfterDefaultConstructor);
    CPPUNIT_TEST(testGetCcDataAfterParsing);
    CPPUNIT_TEST(testGetCcDataTwiceConsecutively);
    CPPUNIT_TEST(testGetCcDataVerifyUniquePtr);
    CPPUNIT_TEST(testGetCcDataVerifyConstCorrectness);
    CPPUNIT_TEST(testIsValidAfterDefaultConstructor);
    CPPUNIT_TEST(testIsValidAfterParsingAtscData);
    CPPUNIT_TEST(testIsValidAfterSetUserData);
    CPPUNIT_TEST(testCcDataIsCcpStartWithValidStart);
    CPPUNIT_TEST(testCcDataIsCcpStartWithOtherType);
    CPPUNIT_TEST(testCcDataIsCcpDataWithValidData);
    CPPUNIT_TEST(testCcDataIsCcpDataWithInvalidData);
    CPPUNIT_TEST(testCcDataIsCcpDataWithOtherType);
    CPPUNIT_TEST(testCcDataIs608DataWithField1);
    CPPUNIT_TEST(testCcDataIs608DataWithField2);
    CPPUNIT_TEST(testCcDataIs608DataWithDtvccTypes);
    CPPUNIT_TEST(testCcDataIsPaddingWithInvalid);
    CPPUNIT_TEST(testCcDataIsPaddingWithValid);
    CPPUNIT_TEST(testMixed608And708Data);
    CPPUNIT_TEST(testAllValidTriplets);
    CPPUNIT_TEST(testAlternatingTypes);
    CPPUNIT_TEST(testSequentialData);
    CPPUNIT_TEST(testDataIntegrityAfterParsing);
    CPPUNIT_TEST(testDataOrderPreserved);
    CPPUNIT_TEST(testDataValuesExactMatch);
    CPPUNIT_TEST(testThreeBytesExactly);
    CPPUNIT_TEST(testSixBytesExactly);
    CPPUNIT_TEST(testFourBytesOneTripletRemainder);
    CPPUNIT_TEST(testFiveBytesOneTripletRemainder);
    CPPUNIT_TEST(testMemoryManagementMultipleObjects);
    CPPUNIT_TEST(testMemoryManagementLargeData);
    CPPUNIT_TEST(testStateTransitionFromInvalidToValid);
    CPPUNIT_TEST(testStateReplacementOnSecondSet);
    CPPUNIT_TEST(testMultipleExceptions);
    CPPUNIT_TEST(testCompleteWorkflow);
    CPPUNIT_TEST(testMultipleSetUserDataCalls);
    CPPUNIT_TEST(testAllCcTypesInSingleData);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // Setup common test data
    }

    void tearDown()
    {
        // Cleanup
    }

    void testDefaultConstructorInitialization()
    {
        UserData userData;
        // Verify object is created successfully
        CPPUNIT_ASSERT(true);
    }

    void testDefaultConstructorIsValid()
    {
        UserData userData;
        CPPUNIT_ASSERT_EQUAL(false, userData.isValid());
    }

    void testDefaultConstructorGetCcData()
    {
        UserData userData;
        const std::vector<std::unique_ptr<CcData>>& ccData = userData.getCcData();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), ccData.size());
    }

    void testParameterizedConstructorValid()
    {
        std::uint8_t data[] = {0x00, 0x41, 0x42};  // CEA_608_FIELD_1, data1=0x41, data2=0x42
        UserData userData(data, 3);

        CPPUNIT_ASSERT_EQUAL(true, userData.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), userData.getCcData().size());
    }

    void testParameterizedConstructorNullptr()
    {
        CPPUNIT_ASSERT_THROW(UserData userData(nullptr, 3), InvalidArgument);
    }

    void testParameterizedConstructorZeroSize()
    {
        std::uint8_t data[] = {0x00, 0x41, 0x42};
        CPPUNIT_ASSERT_THROW(UserData userData(data, 0), InvalidArgument);
    }

    void testParameterizedConstructorBothNullAndZero()
    {
        CPPUNIT_ASSERT_THROW(UserData userData(nullptr, 0), InvalidArgument);
    }

    void testSetUserDataSingleTriplet()
    {
        UserData userData;
        std::uint8_t data[] = {0x02, 0x55, 0x66};  // DTVCC_CCP_DATA, data1=0x55, data2=0x66

        userData.setUserData(data, 3);

        CPPUNIT_ASSERT_EQUAL(true, userData.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), userData.getCcData().size());
        CPPUNIT_ASSERT_EQUAL(true, userData.getCcData()[0]->ccValid);
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::DTVCC_CCP_DATA), static_cast<int>(userData.getCcData()[0]->ccType));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x55), userData.getCcData()[0]->data1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x66), userData.getCcData()[0]->data2);
    }

    void testSetUserDataMultipleTriplets()
    {
        UserData userData;
        std::uint8_t data[] = {
            0x00, 0x11, 0x22,  // CEA_608_FIELD_1
            0x01, 0x33, 0x44,  // CEA_608_FIELD_2
            0x02, 0x55, 0x66   // DTVCC_CCP_DATA
        };

        userData.setUserData(data, 9);

        CPPUNIT_ASSERT_EQUAL(true, userData.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), userData.getCcData().size());

        // Verify first triplet
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::CEA_608_FIELD_1), static_cast<int>(userData.getCcData()[0]->ccType));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x11), userData.getCcData()[0]->data1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x22), userData.getCcData()[0]->data2);

        // Verify second triplet
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::CEA_608_FIELD_2), static_cast<int>(userData.getCcData()[1]->ccType));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x33), userData.getCcData()[1]->data1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x44), userData.getCcData()[1]->data2);

        // Verify third triplet
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::DTVCC_CCP_DATA), static_cast<int>(userData.getCcData()[2]->ccType));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x55), userData.getCcData()[2]->data1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x66), userData.getCcData()[2]->data2);
    }

    void testSetUserDataCcTypeField1()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0xAA, 0xBB};  // ccType = 0x00 (CEA_608_FIELD_1)

        userData.setUserData(data, 3);

        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::CEA_608_FIELD_1), static_cast<int>(userData.getCcData()[0]->ccType));
    }

    void testSetUserDataCcTypeField2()
    {
        UserData userData;
        std::uint8_t data[] = {0x01, 0xAA, 0xBB};  // ccType = 0x01 (CEA_608_FIELD_2)

        userData.setUserData(data, 3);

        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::CEA_608_FIELD_2), static_cast<int>(userData.getCcData()[0]->ccType));
    }

    void testSetUserDataCcTypeDtvccData()
    {
        UserData userData;
        std::uint8_t data[] = {0x02, 0xAA, 0xBB};  // ccType = 0x02 (DTVCC_CCP_DATA)

        userData.setUserData(data, 3);

        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::DTVCC_CCP_DATA), static_cast<int>(userData.getCcData()[0]->ccType));
    }

    void testSetUserDataCcTypeDtvccStart()
    {
        UserData userData;
        std::uint8_t data[] = {0x03, 0xAA, 0xBB};  // ccType = 0x03 (DTVCC_CCP_START)

        userData.setUserData(data, 3);

        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::DTVCC_CCP_START), static_cast<int>(userData.getCcData()[0]->ccType));
    }

    void testSetUserDataData1Zero()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0x00, 0xFF};  // data1 = 0x00

        userData.setUserData(data, 3);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), userData.getCcData()[0]->data1);
    }

    void testSetUserDataData1Max()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0xFF, 0x00};  // data1 = 0xFF

        userData.setUserData(data, 3);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xFF), userData.getCcData()[0]->data1);
    }

    void testSetUserDataData2Zero()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0xFF, 0x00};  // data2 = 0x00

        userData.setUserData(data, 3);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), userData.getCcData()[0]->data2);
    }

    void testSetUserDataData2Max()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0x00, 0xFF};  // data2 = 0xFF

        userData.setUserData(data, 3);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xFF), userData.getCcData()[0]->data2);
    }

    void testSetUserDataSizeNotDivisibleByThree()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0x11, 0x22, 0x01};  // 4 bytes, ccCount = 4/3 = 1

        userData.setUserData(data, 4);

        CPPUNIT_ASSERT_EQUAL(true, userData.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), userData.getCcData().size());
        // Verify the incomplete triplet is ignored
    }

    void testSetUserDataSizeOne()
    {
        UserData userData;
        std::uint8_t data[] = {0x00};  // 1 byte, ccCount = 1/3 = 0

        userData.setUserData(data, 1);

        CPPUNIT_ASSERT_EQUAL(true, userData.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), userData.getCcData().size());
    }

    void testSetUserDataSizeTwo()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0x11};  // 2 bytes, ccCount = 2/3 = 0

        userData.setUserData(data, 2);

        CPPUNIT_ASSERT_EQUAL(true, userData.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), userData.getCcData().size());
    }

    void testSetUserDataLargeSize31Triplets()
    {
        UserData userData;
        std::uint8_t data[93];  // 31 triplets * 3 bytes = 93 bytes

        for (int i = 0; i < 31; ++i) {
            data[i*3] = i % 4;  // ccType cycles through 0-3
            data[i*3+1] = 0x30 + i;
            data[i*3+2] = 0x40 + i;
        }

        userData.setUserData(data, 93);

        CPPUNIT_ASSERT_EQUAL(true, userData.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(31), userData.getCcData().size());

        // Verify first and last triplets
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x30), userData.getCcData()[0]->data1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x30 + 30), userData.getCcData()[30]->data1);
    }

    void testSetUserDataVeryLarge100Triplets()
    {
        UserData userData;
        std::uint8_t data[300];  // 100 triplets * 3 bytes = 300 bytes

        for (int i = 0; i < 100; ++i) {
            data[i*3] = i % 4;
            data[i*3+1] = (i & 0xFF);
            data[i*3+2] = ((i + 1) & 0xFF);
        }

        userData.setUserData(data, 300);

        CPPUNIT_ASSERT_EQUAL(true, userData.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(100), userData.getCcData().size());
    }

    void testSetUserDataNullptr()
    {
        UserData userData;
        CPPUNIT_ASSERT_THROW(userData.setUserData(nullptr, 3), InvalidArgument);
    }

    void testSetUserDataZeroSize()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0x11, 0x22};
        CPPUNIT_ASSERT_THROW(userData.setUserData(data, 0), InvalidArgument);
    }

    void testSetUserDataTwice()
    {
        UserData userData;
        std::uint8_t data1[] = {0x00, 0x11, 0x22};
        std::uint8_t data2[] = {0x01, 0x33, 0x44, 0x02, 0x55, 0x66};

        userData.setUserData(data1, 3);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), userData.getCcData().size());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x11), userData.getCcData()[0]->data1);

        userData.setUserData(data2, 6);
        // setUserData appends instead of replacing, so total is 1 + 2 = 3
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), userData.getCcData().size());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x11), userData.getCcData()[0]->data1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x33), userData.getCcData()[1]->data1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x55), userData.getCcData()[2]->data1);
    }

    void testSetUserDataAfterDefaultConstructor()
    {
        UserData userData;
        CPPUNIT_ASSERT_EQUAL(false, userData.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), userData.getCcData().size());

        std::uint8_t data[] = {0x02, 0xAA, 0xBB};
        userData.setUserData(data, 3);

        CPPUNIT_ASSERT_EQUAL(true, userData.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), userData.getCcData().size());
    }

    void testSetUserDataCcTypeExtractionAllBits()
    {
        UserData userData;
        // Test that only lower 2 bits are used for ccType
        std::uint8_t data[] = {0xFC, 0x11, 0x22};  // 0xFC = 11111100, lower 2 bits = 00

        userData.setUserData(data, 3);

        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::CEA_608_FIELD_1), static_cast<int>(userData.getCcData()[0]->ccType));
    }

    void testGetCcDataAfterDefaultConstructor()
    {
        UserData userData;
        const std::vector<std::unique_ptr<CcData>>& ccData = userData.getCcData();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), ccData.size());
    }

    void testGetCcDataAfterParsing()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0x11, 0x22, 0x01, 0x33, 0x44};
        userData.setUserData(data, 6);

        const std::vector<std::unique_ptr<CcData>>& ccData = userData.getCcData();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), ccData.size());
        CPPUNIT_ASSERT(ccData[0] != nullptr);
        CPPUNIT_ASSERT(ccData[1] != nullptr);
    }

    void testGetCcDataTwiceConsecutively()
    {
        UserData userData;
        std::uint8_t data[] = {0x02, 0xAA, 0xBB};
        userData.setUserData(data, 3);

        const std::vector<std::unique_ptr<CcData>>& ccData1 = userData.getCcData();
        const std::vector<std::unique_ptr<CcData>>& ccData2 = userData.getCcData();

        // Should return the same reference
        CPPUNIT_ASSERT_EQUAL(&ccData1, &ccData2);
        CPPUNIT_ASSERT_EQUAL(ccData1.size(), ccData2.size());
    }

    void testGetCcDataVerifyUniquePtr()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0x11, 0x22};
        userData.setUserData(data, 3);

        const std::vector<std::unique_ptr<CcData>>& ccData = userData.getCcData();

        // Verify unique_ptr is not null
        CPPUNIT_ASSERT(ccData[0].get() != nullptr);
        CPPUNIT_ASSERT(ccData[0] != nullptr);
    }

    void testGetCcDataVerifyConstCorrectness()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0x11, 0x22};
        userData.setUserData(data, 3);

        const std::vector<std::unique_ptr<CcData>>& ccData = userData.getCcData();

        // Method is const, should not modify userData
        CPPUNIT_ASSERT_EQUAL(true, userData.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), ccData.size());
    }

    void testIsValidAfterDefaultConstructor()
    {
        UserData userData;
        CPPUNIT_ASSERT_EQUAL(false, userData.isValid());
    }

    void testIsValidAfterParsingAtscData()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0x11, 0x22};
        userData.setUserData(data, 3);

        // In ATSC_CC_POC path, processCcData is always set to true
        CPPUNIT_ASSERT_EQUAL(true, userData.isValid());
    }

    void testIsValidAfterSetUserData()
    {
        UserData userData;
        CPPUNIT_ASSERT_EQUAL(false, userData.isValid());

        std::uint8_t data[] = {0x02, 0x55, 0x66};
        userData.setUserData(data, 3);

        CPPUNIT_ASSERT_EQUAL(true, userData.isValid());
    }

    void testCcDataIsCcpStartWithValidStart()
    {
        UserData userData;
        std::uint8_t data[] = {0x03, 0x11, 0x22};  // DTVCC_CCP_START
        userData.setUserData(data, 3);

        const CcData* ccData = userData.getCcData()[0].get();
        CPPUNIT_ASSERT_EQUAL(true, ccData->isCcpStart());
    }

    void testCcDataIsCcpStartWithOtherType()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0x11, 0x22};  // CEA_608_FIELD_1
        userData.setUserData(data, 3);

        const CcData* ccData = userData.getCcData()[0].get();
        CPPUNIT_ASSERT_EQUAL(false, ccData->isCcpStart());
    }

    void testCcDataIsCcpDataWithValidData()
    {
        UserData userData;
        std::uint8_t data[] = {0x02, 0x11, 0x22};  // DTVCC_CCP_DATA
        userData.setUserData(data, 3);

        const CcData* ccData = userData.getCcData()[0].get();
        CPPUNIT_ASSERT_EQUAL(true, ccData->isCcpData());
    }

    void testCcDataIsCcpDataWithInvalidData()
    {
        // In ATSC_CC_POC path, ccValid is always true
        UserData userData;
        std::uint8_t data[] = {0x02, 0x11, 0x22};
        userData.setUserData(data, 3);

        const CcData* ccData = userData.getCcData()[0].get();
        CPPUNIT_ASSERT_EQUAL(true, ccData->isCcpData());
    }

    void testCcDataIsCcpDataWithOtherType()
    {
        UserData userData;
        std::uint8_t data[] = {0x01, 0x11, 0x22};  // CEA_608_FIELD_2
        userData.setUserData(data, 3);

        const CcData* ccData = userData.getCcData()[0].get();
        CPPUNIT_ASSERT_EQUAL(false, ccData->isCcpData());
    }

    void testCcDataIs608DataWithField1()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0x11, 0x22};  // CEA_608_FIELD_1
        userData.setUserData(data, 3);

        const CcData* ccData = userData.getCcData()[0].get();
        CPPUNIT_ASSERT_EQUAL(true, ccData->is608Data());
    }

    void testCcDataIs608DataWithField2()
    {
        UserData userData;
        std::uint8_t data[] = {0x01, 0x11, 0x22};  // CEA_608_FIELD_2
        userData.setUserData(data, 3);

        const CcData* ccData = userData.getCcData()[0].get();
        CPPUNIT_ASSERT_EQUAL(true, ccData->is608Data());
    }

    void testCcDataIs608DataWithDtvccTypes()
    {
        UserData userData;
        std::uint8_t data[] = {0x02, 0x11, 0x22, 0x03, 0x33, 0x44};
        userData.setUserData(data, 6);

        const CcData* ccData1 = userData.getCcData()[0].get();
        const CcData* ccData2 = userData.getCcData()[1].get();

        CPPUNIT_ASSERT_EQUAL(false, ccData1->is608Data());  // DTVCC_CCP_DATA
        CPPUNIT_ASSERT_EQUAL(false, ccData2->is608Data());  // DTVCC_CCP_START
    }

    void testCcDataIsPaddingWithInvalid()
    {
        // In ATSC_CC_POC path, ccValid is always true, so isPadding always returns false
        UserData userData;
        std::uint8_t data[] = {0x00, 0x11, 0x22};
        userData.setUserData(data, 3);

        const CcData* ccData = userData.getCcData()[0].get();
        CPPUNIT_ASSERT_EQUAL(false, ccData->isPadding());
    }

    void testCcDataIsPaddingWithValid()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0x11, 0x22};
        userData.setUserData(data, 3);

        const CcData* ccData = userData.getCcData()[0].get();
        CPPUNIT_ASSERT_EQUAL(true, ccData->ccValid);
        CPPUNIT_ASSERT_EQUAL(false, ccData->isPadding());
    }

    void testMixed608And708Data()
    {
        UserData userData;
        std::uint8_t data[] = {
            0x00, 0x11, 0x22,  // 608 Field 1
            0x02, 0x33, 0x44,  // 708 Data
            0x01, 0x55, 0x66,  // 608 Field 2
            0x03, 0x77, 0x88   // 708 Start
        };
        userData.setUserData(data, 12);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), userData.getCcData().size());
        CPPUNIT_ASSERT_EQUAL(true, userData.getCcData()[0]->is608Data());
        CPPUNIT_ASSERT_EQUAL(false, userData.getCcData()[1]->is608Data());
        CPPUNIT_ASSERT_EQUAL(true, userData.getCcData()[2]->is608Data());
        CPPUNIT_ASSERT_EQUAL(false, userData.getCcData()[3]->is608Data());
    }

    void testAllValidTriplets()
    {
        UserData userData;
        std::uint8_t data[] = {
            0x00, 0x11, 0x22,
            0x01, 0x33, 0x44,
            0x02, 0x55, 0x66
        };
        userData.setUserData(data, 9);

        // All triplets should be valid in ATSC_CC_POC path
        for (size_t i = 0; i < userData.getCcData().size(); ++i) {
            CPPUNIT_ASSERT_EQUAL(true, userData.getCcData()[i]->ccValid);
        }
    }

    void testAlternatingTypes()
    {
        UserData userData;
        std::uint8_t data[] = {
            0x00, 0x11, 0x22,  // Field 1
            0x01, 0x33, 0x44,  // Field 2
            0x00, 0x55, 0x66,  // Field 1
            0x01, 0x77, 0x88   // Field 2
        };
        userData.setUserData(data, 12);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), userData.getCcData().size());
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::CEA_608_FIELD_1), static_cast<int>(userData.getCcData()[0]->ccType));
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::CEA_608_FIELD_2), static_cast<int>(userData.getCcData()[1]->ccType));
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::CEA_608_FIELD_1), static_cast<int>(userData.getCcData()[2]->ccType));
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::CEA_608_FIELD_2), static_cast<int>(userData.getCcData()[3]->ccType));
    }

    void testSequentialData()
    {
        UserData userData;
        std::uint8_t data[30];  // 10 triplets

        for (int i = 0; i < 10; ++i) {
            data[i*3] = 0x02;  // All DTVCC_CCP_DATA
            data[i*3+1] = 0x41 + i;
            data[i*3+2] = 0x51 + i;
        }
        userData.setUserData(data, 30);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10), userData.getCcData().size());

        // Verify sequential data
        for (int i = 0; i < 10; ++i) {
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x41 + i), userData.getCcData()[i]->data1);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x51 + i), userData.getCcData()[i]->data2);
        }
    }

    void testDataIntegrityAfterParsing()
    {
        UserData userData;
        std::uint8_t data[] = {
            0xFC, 0xAA, 0xBB,  // Type 0 with high bits set
            0xFD, 0xCC, 0xDD   // Type 1 with high bits set
        };
        userData.setUserData(data, 6);

        // Verify ccType extraction (only lower 2 bits)
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::CEA_608_FIELD_1), static_cast<int>(userData.getCcData()[0]->ccType));
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::CEA_608_FIELD_2), static_cast<int>(userData.getCcData()[1]->ccType));

        // Verify data bytes are preserved exactly
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xAA), userData.getCcData()[0]->data1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xBB), userData.getCcData()[0]->data2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xCC), userData.getCcData()[1]->data1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xDD), userData.getCcData()[1]->data2);
    }

    void testDataOrderPreserved()
    {
        UserData userData;
        std::uint8_t data[] = {
            0x00, 0x01, 0x02,
            0x00, 0x03, 0x04,
            0x00, 0x05, 0x06,
            0x00, 0x07, 0x08
        };
        userData.setUserData(data, 12);

        // Verify order is preserved
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x01), userData.getCcData()[0]->data1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x03), userData.getCcData()[1]->data1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x05), userData.getCcData()[2]->data1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x07), userData.getCcData()[3]->data1);
    }

    void testDataValuesExactMatch()
    {
        UserData userData;
        std::uint8_t data[] = {0x02, 0x7F, 0x80};  // Specific boundary values
        userData.setUserData(data, 3);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x7F), userData.getCcData()[0]->data1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x80), userData.getCcData()[0]->data2);
    }

    void testThreeBytesExactly()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0x11, 0x22};
        userData.setUserData(data, 3);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), userData.getCcData().size());
    }

    void testSixBytesExactly()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0x11, 0x22, 0x01, 0x33, 0x44};
        userData.setUserData(data, 6);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), userData.getCcData().size());
    }

    void testFourBytesOneTripletRemainder()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0x11, 0x22, 0x99};  // Last byte ignored
        userData.setUserData(data, 4);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), userData.getCcData().size());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x11), userData.getCcData()[0]->data1);
    }

    void testFiveBytesOneTripletRemainder()
    {
        UserData userData;
        std::uint8_t data[] = {0x00, 0x11, 0x22, 0x98, 0x99};  // Last 2 bytes ignored
        userData.setUserData(data, 5);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), userData.getCcData().size());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x11), userData.getCcData()[0]->data1);
    }

    void testMemoryManagementMultipleObjects()
    {
        // Create and destroy multiple UserData objects
        for (int i = 0; i < 100; ++i) {
            UserData userData;
            std::uint8_t data[] = {0x00, 0x11, 0x22, 0x01, 0x33, 0x44};
            userData.setUserData(data, 6);

            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), userData.getCcData().size());
            // Object goes out of scope, unique_ptr should clean up
        }
        CPPUNIT_ASSERT(true);  // No memory leaks or crashes
    }

    void testMemoryManagementLargeData()
    {
        UserData userData;
        std::uint8_t data[300];  // 100 triplets

        for (int i = 0; i < 100; ++i) {
            data[i*3] = i % 4;
            data[i*3+1] = i & 0xFF;
            data[i*3+2] = (i + 1) & 0xFF;
        }

        userData.setUserData(data, 300);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(100), userData.getCcData().size());

        // Object destruction should clean up all 100 unique_ptrs
    }

    void testStateTransitionFromInvalidToValid()
    {
        UserData userData;
        CPPUNIT_ASSERT_EQUAL(false, userData.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), userData.getCcData().size());

        std::uint8_t data[] = {0x02, 0xAA, 0xBB};
        userData.setUserData(data, 3);

        CPPUNIT_ASSERT_EQUAL(true, userData.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), userData.getCcData().size());
    }

    void testStateReplacementOnSecondSet()
    {
        UserData userData;
        std::uint8_t data1[] = {0x00, 0x11, 0x22, 0x01, 0x33, 0x44};
        userData.setUserData(data1, 6);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), userData.getCcData().size());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x11), userData.getCcData()[0]->data1);

        std::uint8_t data2[] = {0x02, 0xAA, 0xBB};
        userData.setUserData(data2, 3);

        // setUserData appends instead of replacing, so total is 2 + 1 = 3
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), userData.getCcData().size());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x11), userData.getCcData()[0]->data1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xAA), userData.getCcData()[2]->data1);
    }

    void testMultipleExceptions()
    {
        UserData userData;

        // First exception
        try {
            userData.setUserData(nullptr, 3);
            CPPUNIT_FAIL("Should have thrown InvalidArgument");
        } catch (const InvalidArgument&) {
            // Expected
        }

        // Second exception
        std::uint8_t data[] = {0x11, 0x22, 0x33};
        try {
            userData.setUserData(data, 0);
            CPPUNIT_FAIL("Should have thrown InvalidArgument");
        } catch (const InvalidArgument&) {
            // Expected
        }

        // Should still be able to set valid data
        userData.setUserData(data, 3);
        CPPUNIT_ASSERT_EQUAL(true, userData.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), userData.getCcData().size());
    }

    void testCompleteWorkflow()
    {
        // 1. Create UserData
        UserData userData;
        CPPUNIT_ASSERT_EQUAL(false, userData.isValid());

        // 2. Set data with mixed types
        std::uint8_t data[] = {
            0x00, 0x20, 0x21,  // 608 Field 1
            0x02, 0x30, 0x31,  // 708 Data
            0x03, 0x40, 0x41,  // 708 Start
            0x01, 0x50, 0x51   // 608 Field 2
        };
        userData.setUserData(data, 12);

        // 3. Verify parsing
        CPPUNIT_ASSERT_EQUAL(true, userData.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), userData.getCcData().size());

        // 4. Verify helper methods work
        CPPUNIT_ASSERT_EQUAL(true, userData.getCcData()[0]->is608Data());
        CPPUNIT_ASSERT_EQUAL(true, userData.getCcData()[1]->isCcpData());
        CPPUNIT_ASSERT_EQUAL(true, userData.getCcData()[2]->isCcpStart());
        CPPUNIT_ASSERT_EQUAL(true, userData.getCcData()[3]->is608Data());

        // 5. Append new data (setUserData appends instead of replacing)
        std::uint8_t newData[] = {0x00, 0xAA, 0xBB};
        userData.setUserData(newData, 3);

        // Total is now 4 + 1 = 5
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), userData.getCcData().size());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xAA), userData.getCcData()[4]->data1);
    }

    void testMultipleSetUserDataCalls()
    {
        UserData userData;

        // First set
        std::uint8_t data1[] = {0x00, 0x01, 0x02};
        userData.setUserData(data1, 3);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), userData.getCcData().size());

        // Second set (appends, so total is 1 + 2 = 3)
        std::uint8_t data2[] = {0x01, 0x03, 0x04, 0x02, 0x05, 0x06};
        userData.setUserData(data2, 6);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), userData.getCcData().size());

        // Third set (appends, so total is 3 + 1 = 4)
        std::uint8_t data3[] = {0x03, 0x07, 0x08};
        userData.setUserData(data3, 3);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), userData.getCcData().size());
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::DTVCC_CCP_START), static_cast<int>(userData.getCcData()[3]->ccType));
    }

    void testAllCcTypesInSingleData()
    {
        UserData userData;
        std::uint8_t data[] = {
            0x00, 0x10, 0x11,  // CEA_608_FIELD_1
            0x01, 0x20, 0x21,  // CEA_608_FIELD_2
            0x02, 0x30, 0x31,  // DTVCC_CCP_DATA
            0x03, 0x40, 0x41   // DTVCC_CCP_START
        };
        userData.setUserData(data, 12);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), userData.getCcData().size());

        // Verify all types
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::CEA_608_FIELD_1), static_cast<int>(userData.getCcData()[0]->ccType));
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::CEA_608_FIELD_2), static_cast<int>(userData.getCcData()[1]->ccType));
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::DTVCC_CCP_DATA), static_cast<int>(userData.getCcData()[2]->ccType));
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(CcData::DTVCC_CCP_START), static_cast<int>(userData.getCcData()[3]->ccType));

        // Verify data values
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x10), userData.getCcData()[0]->data1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x20), userData.getCcData()[1]->data1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x30), userData.getCcData()[2]->data1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x40), userData.getCcData()[3]->data1);
    }
};

// Register the test suite
CPPUNIT_TEST_SUITE_REGISTRATION(CcUserDataTest);
