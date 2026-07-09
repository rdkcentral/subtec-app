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
#include "ScteRawBitmap.hpp"
#include "ScteExceptions.hpp"
#include <vector>
#include <cstring>
#include <algorithm>

using namespace subttxrend::scte;

class ScteRawBitmapTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ScteRawBitmapTest );
    CPPUNIT_TEST(testConstructorWithValidCompressedData);
    CPPUNIT_TEST(testConstructorWithValidUncompressedData);
    CPPUNIT_TEST(testConstructorWithNullptr);
    CPPUNIT_TEST(testConstructorWithSizeZero);
    CPPUNIT_TEST(testConstructorWithNullptrAndSizeZero);
    CPPUNIT_TEST(testGetRawDataReturnsCorrectSize);
    CPPUNIT_TEST(testGetRawDataReturnsCorrectContent);
    CPPUNIT_TEST(testGetRawDataAfterConstruction);
    CPPUNIT_TEST(testSetRawDataWithValidPointer);
    CPPUNIT_TEST(testSetRawDataWithNullptr);
    CPPUNIT_TEST(testSetRawDataWithSizeZero);
    CPPUNIT_TEST(testSetRawDataReplacesPreviousData);
    CPPUNIT_TEST(testSetRawDataWithDifferentSizes);
    CPPUNIT_TEST(testSetRawDataMultipleTimes);
    CPPUNIT_TEST(testSetRawDataFromVector);
    CPPUNIT_TEST(testSetRawDataCreatesDeepCopy);
    CPPUNIT_TEST(testSetRawDataWithRvalueReference);
    CPPUNIT_TEST(testSetRawDataFromStdMove);
    CPPUNIT_TEST(testIsCompressedAfterConstructionTrue);
    CPPUNIT_TEST(testIsCompressedAfterConstructionFalse);
    CPPUNIT_TEST(testIsCompressedReflectsSetCompression);
    CPPUNIT_TEST(testSetCompressionToTrue);
    CPPUNIT_TEST(testSetCompressionToFalse);
    CPPUNIT_TEST(testSetCompressionToggleMultipleTimes);
    CPPUNIT_TEST(testLargeDataPayload);
    CPPUNIT_TEST(testSingleByteData);
    CPPUNIT_TEST(testDataWithAllZeros);
    CPPUNIT_TEST(testDataWithAllMaxValues);
    CPPUNIT_TEST(testDefaultConstructor);

CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // Setup code here
    }

    void tearDown()
    {
        // Cleanup code here
    }

protected:
    // Helper to create test data
    std::vector<uint8_t> createTestData(size_t size, uint8_t fillValue = 0x00)
    {
        return std::vector<uint8_t>(size, fillValue);
    }

    // Helper to create sequential test data
    std::vector<uint8_t> createSequentialData(size_t size)
    {
        std::vector<uint8_t> data(size);
        for (size_t i = 0; i < size; ++i)
        {
            data[i] = static_cast<uint8_t>(i % 256);
        }
        return data;
    }

    // Helper to compare data
    bool dataMatches(const RawBitmap::Data& data, const uint8_t* expected, size_t size)
    {
        if (data.size() != size) return false;
        return std::equal(data.begin(), data.end(), expected);
    }

    void testConstructorWithValidCompressedData()
    {
        std::vector<uint8_t> testData = createSequentialData(10);

        RawBitmap bitmap(true, testData.data(), testData.size());

        CPPUNIT_ASSERT_EQUAL(true, bitmap.isCompressed());
        CPPUNIT_ASSERT_EQUAL(testData.size(), bitmap.getRawData().size());
        CPPUNIT_ASSERT(dataMatches(bitmap.getRawData(), testData.data(), testData.size()));
    }

    void testConstructorWithValidUncompressedData()
    {
        std::vector<uint8_t> testData = createSequentialData(10);

        RawBitmap bitmap(false, testData.data(), testData.size());

        CPPUNIT_ASSERT_EQUAL(false, bitmap.isCompressed());
        CPPUNIT_ASSERT_EQUAL(testData.size(), bitmap.getRawData().size());
        CPPUNIT_ASSERT(dataMatches(bitmap.getRawData(), testData.data(), testData.size()));
    }

    void testConstructorWithNullptr()
    {
        CPPUNIT_ASSERT_THROW(
            RawBitmap bitmap(true, nullptr, 100),
            InvalidArgument
        );
    }

    void testConstructorWithSizeZero()
    {
        std::vector<uint8_t> testData = createTestData(10);

        CPPUNIT_ASSERT_THROW(
            RawBitmap bitmap(true, testData.data(), 0),
            InvalidArgument
        );
    }

    void testConstructorWithNullptrAndSizeZero()
    {
        CPPUNIT_ASSERT_THROW(
            RawBitmap bitmap(false, nullptr, 0),
            InvalidArgument
        );
    }

    void testGetRawDataReturnsCorrectSize()
    {
        std::vector<uint8_t> testData = createSequentialData(25);
        RawBitmap bitmap(true, testData.data(), testData.size());

        const RawBitmap::Data& data = bitmap.getRawData();

        CPPUNIT_ASSERT_EQUAL(testData.size(), data.size());
    }

    void testGetRawDataReturnsCorrectContent()
    {
        std::vector<uint8_t> testData = {0x01, 0x02, 0x03, 0x04, 0x05};
        RawBitmap bitmap(false, testData.data(), testData.size());

        const RawBitmap::Data& data = bitmap.getRawData();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), data.size());
        for (size_t i = 0; i < testData.size(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL(testData[i], data[i]);
        }
    }

    void testGetRawDataAfterConstruction()
    {
        std::vector<uint8_t> testData = createSequentialData(15);
        RawBitmap bitmap(true, testData.data(), testData.size());

        const RawBitmap::Data& data = bitmap.getRawData();

        CPPUNIT_ASSERT(dataMatches(data, testData.data(), testData.size()));
    }

    void testSetRawDataWithValidPointer()
    {
        std::vector<uint8_t> initialData = createSequentialData(10);
        RawBitmap bitmap(true, initialData.data(), initialData.size());

        std::vector<uint8_t> newData = {0xAA, 0xBB, 0xCC, 0xDD};
        bitmap.setRawData(newData.data(), newData.size());

        CPPUNIT_ASSERT_EQUAL(newData.size(), bitmap.getRawData().size());
        CPPUNIT_ASSERT(dataMatches(bitmap.getRawData(), newData.data(), newData.size()));
    }

    void testSetRawDataWithNullptr()
    {
        std::vector<uint8_t> initialData = createSequentialData(10);
        RawBitmap bitmap(false, initialData.data(), initialData.size());

        CPPUNIT_ASSERT_THROW(
            bitmap.setRawData(nullptr, 100),
            InvalidArgument
        );
    }

    void testSetRawDataWithSizeZero()
    {
        std::vector<uint8_t> initialData = createSequentialData(10);
        RawBitmap bitmap(true, initialData.data(), initialData.size());

        std::vector<uint8_t> newData = createTestData(10);
        CPPUNIT_ASSERT_THROW(
            bitmap.setRawData(newData.data(), 0),
            InvalidArgument
        );
    }

    void testSetRawDataReplacesPreviousData()
    {
        std::vector<uint8_t> initialData = {0x01, 0x02, 0x03};
        RawBitmap bitmap(true, initialData.data(), initialData.size());

        std::vector<uint8_t> newData = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
        bitmap.setRawData(newData.data(), newData.size());

        const RawBitmap::Data& data = bitmap.getRawData();
        CPPUNIT_ASSERT_EQUAL(newData.size(), data.size());

        // Verify old data is completely replaced
        for (size_t i = 0; i < newData.size(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL(newData[i], data[i]);
        }
    }

    void testSetRawDataWithDifferentSizes()
    {
        std::vector<uint8_t> initialData = createSequentialData(100);
        RawBitmap bitmap(false, initialData.data(), initialData.size());

        // Replace with smaller data
        std::vector<uint8_t> smallerData = createTestData(5, 0x11);
        bitmap.setRawData(smallerData.data(), smallerData.size());
        CPPUNIT_ASSERT_EQUAL(smallerData.size(), bitmap.getRawData().size());

        // Replace with larger data
        std::vector<uint8_t> largerData = createTestData(200, 0x22);
        bitmap.setRawData(largerData.data(), largerData.size());
        CPPUNIT_ASSERT_EQUAL(largerData.size(), bitmap.getRawData().size());
    }

    void testSetRawDataMultipleTimes()
    {
        std::vector<uint8_t> initialData = createTestData(10, 0x00);
        RawBitmap bitmap(true, initialData.data(), initialData.size());

        for (int i = 0; i < 5; ++i)
        {
            std::vector<uint8_t> newData = createTestData(10 + i, static_cast<uint8_t>(i));
            bitmap.setRawData(newData.data(), newData.size());

            CPPUNIT_ASSERT_EQUAL(newData.size(), bitmap.getRawData().size());
            CPPUNIT_ASSERT(dataMatches(bitmap.getRawData(), newData.data(), newData.size()));
        }
    }

    void testSetRawDataFromVector()
    {
        std::vector<uint8_t> initialData = createSequentialData(10);
        RawBitmap bitmap(true, initialData.data(), initialData.size());

        RawBitmap::Data newData = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
        bitmap.setRawData(newData);

        CPPUNIT_ASSERT_EQUAL(newData.size(), bitmap.getRawData().size());
        CPPUNIT_ASSERT(dataMatches(bitmap.getRawData(), newData.data(), newData.size()));
    }

    void testSetRawDataCreatesDeepCopy()
    {
        std::vector<uint8_t> initialData = createSequentialData(10);
        RawBitmap bitmap(false, initialData.data(), initialData.size());

        RawBitmap::Data sourceData = {0x11, 0x22, 0x33};
        bitmap.setRawData(sourceData);

        // Modify source data after setting
        sourceData[0] = 0xFF;
        sourceData[1] = 0xEE;
        sourceData[2] = 0xDD;

        // Verify bitmap data is unchanged (deep copy)
        const RawBitmap::Data& bitmapData = bitmap.getRawData();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x11), bitmapData[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x22), bitmapData[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x33), bitmapData[2]);
    }

    void testSetRawDataWithRvalueReference()
    {
        std::vector<uint8_t> initialData = createSequentialData(10);
        RawBitmap bitmap(true, initialData.data(), initialData.size());

        RawBitmap::Data newData = {0xAA, 0xBB, 0xCC, 0xDD};
        size_t expectedSize = newData.size();
        uint8_t expectedFirst = newData[0];
        uint8_t expectedLast = newData[3];

        bitmap.setRawData(std::move(newData));

        const RawBitmap::Data& data = bitmap.getRawData();
        CPPUNIT_ASSERT_EQUAL(expectedSize, data.size());
        CPPUNIT_ASSERT_EQUAL(expectedFirst, data[0]);
        CPPUNIT_ASSERT_EQUAL(expectedLast, data[3]);
    }

    void testSetRawDataFromStdMove()
    {
        std::vector<uint8_t> initialData = createSequentialData(10);
        RawBitmap bitmap(false, initialData.data(), initialData.size());

        RawBitmap::Data sourceData = {0x11, 0x22, 0x33, 0x44, 0x55};
        RawBitmap::Data expectedData = sourceData; // Copy for verification

        bitmap.setRawData(std::move(sourceData));

        CPPUNIT_ASSERT_EQUAL(expectedData.size(), bitmap.getRawData().size());
        CPPUNIT_ASSERT(dataMatches(bitmap.getRawData(), expectedData.data(), expectedData.size()));
    }

    void testIsCompressedAfterConstructionTrue()
    {
        std::vector<uint8_t> testData = createTestData(10);
        RawBitmap bitmap(true, testData.data(), testData.size());

        CPPUNIT_ASSERT_EQUAL(true, bitmap.isCompressed());
    }

    void testIsCompressedAfterConstructionFalse()
    {
        std::vector<uint8_t> testData = createTestData(10);
        RawBitmap bitmap(false, testData.data(), testData.size());

        CPPUNIT_ASSERT_EQUAL(false, bitmap.isCompressed());
    }

    void testIsCompressedReflectsSetCompression()
    {
        std::vector<uint8_t> testData = createTestData(10);
        RawBitmap bitmap(true, testData.data(), testData.size());

        CPPUNIT_ASSERT_EQUAL(true, bitmap.isCompressed());

        bitmap.setCompression(false);
        CPPUNIT_ASSERT_EQUAL(false, bitmap.isCompressed());

        bitmap.setCompression(true);
        CPPUNIT_ASSERT_EQUAL(true, bitmap.isCompressed());
    }

    void testSetCompressionToTrue()
    {
        std::vector<uint8_t> testData = createTestData(10);
        RawBitmap bitmap(false, testData.data(), testData.size());

        bitmap.setCompression(true);

        CPPUNIT_ASSERT_EQUAL(true, bitmap.isCompressed());
    }

    void testSetCompressionToFalse()
    {
        std::vector<uint8_t> testData = createTestData(10);
        RawBitmap bitmap(true, testData.data(), testData.size());

        bitmap.setCompression(false);

        CPPUNIT_ASSERT_EQUAL(false, bitmap.isCompressed());
    }

    void testSetCompressionToggleMultipleTimes()
    {
        std::vector<uint8_t> testData = createTestData(10);
        RawBitmap bitmap(true, testData.data(), testData.size());

        for (int i = 0; i < 10; ++i)
        {
            bool expected = (i % 2 == 0);
            bitmap.setCompression(expected);
            CPPUNIT_ASSERT_EQUAL(expected, bitmap.isCompressed());
        }
    }

    void testLargeDataPayload()
    {
        // Test with 1MB of data
        const size_t largeSize = 1024 * 1024;
        std::vector<uint8_t> largeData = createSequentialData(largeSize);

        RawBitmap bitmap(true, largeData.data(), largeData.size());

        CPPUNIT_ASSERT_EQUAL(largeSize, bitmap.getRawData().size());
        CPPUNIT_ASSERT(dataMatches(bitmap.getRawData(), largeData.data(), largeData.size()));
    }

    void testSingleByteData()
    {
        uint8_t singleByte = 0x42;

        RawBitmap bitmap(false, &singleByte, 1);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), bitmap.getRawData().size());
        CPPUNIT_ASSERT_EQUAL(singleByte, bitmap.getRawData()[0]);
    }

    void testDataWithAllZeros()
    {
        std::vector<uint8_t> zeroData = createTestData(100, 0x00);

        RawBitmap bitmap(true, zeroData.data(), zeroData.size());

        const RawBitmap::Data& data = bitmap.getRawData();
        CPPUNIT_ASSERT_EQUAL(zeroData.size(), data.size());

        for (size_t i = 0; i < data.size(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), data[i]);
        }
    }

    void testDataWithAllMaxValues()
    {
        std::vector<uint8_t> maxData = createTestData(100, 0xFF);

        RawBitmap bitmap(false, maxData.data(), maxData.size());

        const RawBitmap::Data& data = bitmap.getRawData();
        CPPUNIT_ASSERT_EQUAL(maxData.size(), data.size());

        for (size_t i = 0; i < data.size(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), data[i]);
        }
    }

    void testDefaultConstructor()
    {
        RawBitmap bitmap;

        // Default constructed bitmap should have empty data
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), bitmap.getRawData().size());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( ScteRawBitmapTest );
