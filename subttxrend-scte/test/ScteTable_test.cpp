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
#include "ScteTable.hpp"
#include "ScteExceptions.hpp"
#include <vector>
#include <cstring>

using namespace subttxrend::scte;

class ScteTableTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ScteTableTest );
    CPPUNIT_TEST(testConstructorWithNullPointer);
    CPPUNIT_TEST(testConstructorWithSizeZero);
    CPPUNIT_TEST(testConstructorWithSizeBelowMinimum);
    CPPUNIT_TEST(testConstructorWithSizeExactlyMinimum);
    CPPUNIT_TEST(testGetLanguageCodeStandard);
    CPPUNIT_TEST(testGetLanguageCodeNonAscii);
    CPPUNIT_TEST(testPreClearDisplayWhenBitSet);
    CPPUNIT_TEST(testPreClearDisplayWhenBitNotSet);
    CPPUNIT_TEST(testIsImmediateWhenBitSet);
    CPPUNIT_TEST(testIsImmediateWhenBitNotSet);
    CPPUNIT_TEST(testBothFlagsSet);
    CPPUNIT_TEST(testBothFlagsClear);
    CPPUNIT_TEST(testGetDisplayStandard720x480);
    CPPUNIT_TEST(testGetDisplayStandard720x576);
    CPPUNIT_TEST(testGetDisplayStandard1280x720);
    CPPUNIT_TEST(testGetDisplayStandard1920x1080);
    CPPUNIT_TEST(testGetPTSMinimumValue);
    CPPUNIT_TEST(testGetPTSMaximumValue);
    CPPUNIT_TEST(testGetPTSTypicalValue);
    CPPUNIT_TEST(testGetSubtitleTypeSimpleBitmap);
    CPPUNIT_TEST(testGetDisplayDurationMinimum);
    CPPUNIT_TEST(testGetDisplayDurationMaximum);
    CPPUNIT_TEST(testGetDisplayDurationTypical);
    CPPUNIT_TEST(testGetEndPtsFor720x480);
    CPPUNIT_TEST(testGetEndPtsFor720x576);
    CPPUNIT_TEST(testGetEndPtsFor1280x720);
    CPPUNIT_TEST(testGetEndPtsFor1920x1080);
    CPPUNIT_TEST(testGetEndPtsWithZeroDuration);
    CPPUNIT_TEST(testGetIdReturnsCorrectValue);
    CPPUNIT_TEST(testSetTableIdModifiesId);
    CPPUNIT_TEST(testSetTableDataWithValidData);
    CPPUNIT_TEST(testSetTableDataWithNullPointer);
    CPPUNIT_TEST(testSetTableDataWithInsufficientSize);
    CPPUNIT_TEST(testConstructorWithValidBitmapData);
    CPPUNIT_TEST(testConstructorWithNonBitmapSubtitleType);
    CPPUNIT_TEST(testConstructorWithBitmapLengthExceedsDataSize);
    CPPUNIT_TEST(testConstructorWithZeroBitmapLength);

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
    // Helper to create minimal valid test data (12 bytes minimum)
    std::vector<uint8_t> createMinimalValidData()
    {
        std::vector<uint8_t> data(12, 0x00);
        data[0] = 'e'; // language code
        data[1] = 'n';
        data[2] = 'g';
        data[3] = 0x00; // flags and display standard
        data[4] = 0x00; // PTS byte 0
        data[5] = 0x00; // PTS byte 1
        data[6] = 0x00; // PTS byte 2
        data[7] = 0x00; // PTS byte 3
        data[8] = 0x00; // subtitle type (upper nibble) and duration upper bits
        data[9] = 0x00; // duration lower byte
        data[10] = 0x00; // bitmap length upper byte
        data[11] = 0x00; // bitmap length lower byte
        return data;
    }

    std::vector<uint8_t> createValidDataWithBitmap()
    {
        // Build a minimally valid SCTE table with a SIMPLE_BITMAP payload.
        // ScteTable expects:
        // - bytes 0..11: table header fields (incl. bitmapLen at 10..11)
        // - bytes 12..(12+bitmapLen-1): SimpleBitmap structure
        //
        // SimpleBitmap::fillFields reads:
        // - 9 bytes header (style/color/coords)
        // - 2 bytes internal bitmapLength
        // - bitmapLength bytes raw bitmap data
        // and then decompresses if compressed (default=true). We keep width/height = 1.

        std::vector<uint8_t> data = createMinimalValidData();

        // Mark subtitle type SIMPLE_BITMAP (0x01 in upper nibble).
        data[8] = static_cast<uint8_t>((static_cast<uint8_t>(SubtitleType::SIMPLE_BITMAP) << 4) | (data[8] & 0x0F));

        // SimpleBitmap payload (12 bytes total): 9(header) + 2(internal length) + 1(raw bitmap byte)
        const uint16_t simpleBitmapLen = 12;
        data[10] = static_cast<uint8_t>((simpleBitmapLen >> 8) & 0xFF);
        data[11] = static_cast<uint8_t>(simpleBitmapLen & 0xFF);

        std::vector<uint8_t> simpleBitmap;
        simpleBitmap.reserve(simpleBitmapLen);

        // Byte 0: backgroundStyle/outlineStyle. Keep both NONE/TRANSPARENT.
        simpleBitmap.push_back(0x00);
        // Bytes 1-2: character color fields (arbitrary).
        simpleBitmap.push_back(0x00);
        simpleBitmap.push_back(0x00);
        // Bytes 3-5: charTop (x=0,y=0)
        simpleBitmap.push_back(0x00);
        simpleBitmap.push_back(0x00);
        simpleBitmap.push_back(0x00);
        // Bytes 6-8: charBottom (x=1,y=1) => width=1, height=1
        simpleBitmap.push_back(0x00);
        simpleBitmap.push_back(0x10); // high nibble gives x=1, low nibble gives y high bits=0
        simpleBitmap.push_back(0x01);
        // Bytes 9-10: internal bitmapLength = 1
        simpleBitmap.push_back(0x00);
        simpleBitmap.push_back(0x01);
        // Byte 11: compressed raw bitmap data; start with 1-bit set to ensure decoder makes progress.
        simpleBitmap.push_back(0x80);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(simpleBitmapLen), simpleBitmap.size());
        data.insert(data.end(), simpleBitmap.begin(), simpleBitmap.end());

        return data;
    }

    void testConstructorWithNullPointer()
    {
        CPPUNIT_ASSERT_THROW(
            ScteTable(TABLE_ID_SCTE_SUB, nullptr, 12),
            InvalidArgument
        );
    }

    void testConstructorWithSizeZero()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        CPPUNIT_ASSERT_THROW(
            ScteTable(TABLE_ID_SCTE_SUB, data.data(), 0),
            InvalidArgument
        );
    }

    void testConstructorWithSizeBelowMinimum()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        CPPUNIT_ASSERT_THROW(
            ScteTable(TABLE_ID_SCTE_SUB, data.data(), 11),
            InvalidArgument
        );
    }

    void testConstructorWithSizeExactlyMinimum()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        CPPUNIT_ASSERT_NO_THROW(
            ScteTable(TABLE_ID_SCTE_SUB, data.data(), 12)
        );
    }

    void testGetLanguageCodeStandard()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[0] = 'e';
        data[1] = 'n';
        data[2] = 'g';

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(std::string("eng"), table.getLanguageCode());
    }

    void testGetLanguageCodeNonAscii()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[0] = 0xFF;
        data[1] = 0xFE;
        data[2] = 0xFD;

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        std::string langCode = table.getLanguageCode();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), langCode.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(0xFF), static_cast<unsigned char>(langCode[0]));
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(0xFE), static_cast<unsigned char>(langCode[1]));
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(0xFD), static_cast<unsigned char>(langCode[2]));
    }

    void testPreClearDisplayWhenBitSet()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[3] = 0x80; // Set bit 7

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(true, table.preClearDisplay());
    }

    void testPreClearDisplayWhenBitNotSet()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[3] = 0x00; // Clear bit 7

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(false, table.preClearDisplay());
    }

    void testIsImmediateWhenBitSet()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[3] = 0x40; // Set bit 6

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(true, table.isImmediate());
    }

    void testIsImmediateWhenBitNotSet()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[3] = 0x00; // Clear bit 6

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(false, table.isImmediate());
    }

    void testBothFlagsSet()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[3] = 0xC0; // Set both bits 7 and 6

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(true, table.preClearDisplay());
        CPPUNIT_ASSERT_EQUAL(true, table.isImmediate());
    }

    void testBothFlagsClear()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[3] = 0x00; // Clear all bits

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(false, table.preClearDisplay());
        CPPUNIT_ASSERT_EQUAL(false, table.isImmediate());
    }

    void testGetDisplayStandard720x480()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[3] = 0x00; // DS_720_480_30

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT(table.getDisplayStandard() == DisplayStandard::DS_720_480_30);
    }

    void testGetDisplayStandard720x576()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[3] = 0x01; // DS_720_576_25

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT(table.getDisplayStandard() == DisplayStandard::DS_720_576_25);
    }

    void testGetDisplayStandard1280x720()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[3] = 0x02; // DS_1280_720_60

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT(table.getDisplayStandard() == DisplayStandard::DS_1280_720_60);
    }

    void testGetDisplayStandard1920x1080()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[3] = 0x03; // DS_1920_1080_60

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT(table.getDisplayStandard() == DisplayStandard::DS_1920_1080_60);
    }

    void testGetPTSMinimumValue()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[4] = 0x00;
        data[5] = 0x00;
        data[6] = 0x00;
        data[7] = 0x00;

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0), table.getPTS());
    }

    void testGetPTSMaximumValue()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[4] = 0xFF;
        data[5] = 0xFF;
        data[6] = 0xFF;
        data[7] = 0xFF;

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0xFFFFFFFF), table.getPTS());
    }

    void testGetPTSTypicalValue()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[4] = 0x12;
        data[5] = 0x34;
        data[6] = 0x56;
        data[7] = 0x78;

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0x12345678), table.getPTS());
    }

    void testGetSubtitleTypeSimpleBitmap()
    {
        // Use a valid SIMPLE_BITMAP payload so construction doesn't fail on bitmap parsing.
        std::vector<uint8_t> data = createValidDataWithBitmap();
        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT(table.getSubtitleType() == SubtitleType::SIMPLE_BITMAP);
    }

    void testGetDisplayDurationMinimum()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[8] = 0x00; // Upper 3 bits clear
        data[9] = 0x00; // Lower 8 bits clear

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), table.getDisplayDuration());
    }

    void testGetDisplayDurationMaximum()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[8] = 0x07; // Upper 3 bits set (0x07)
        data[9] = 0xFF; // Lower 8 bits set

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x7FF), table.getDisplayDuration());
    }

    void testGetDisplayDurationTypical()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[8] = 0x01; // 0x001 in bits 10-8
        data[9] = 0xE0; // 0xE0 in bits 7-0

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x1E0), table.getDisplayDuration());
    }

    void testGetEndPtsFor720x480()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[3] = 0x00; // DS_720_480_30
        data[4] = 0x00;
        data[5] = 0x00;
        data[6] = 0x00;
        data[7] = 0x00; // PTS = 0
        data[8] = 0x00;
        data[9] = 0x3C; // Duration = 60 frames

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        uint32_t expectedEndPts = 0 + (90000 / 30) * 60; // 180000
        CPPUNIT_ASSERT_EQUAL(expectedEndPts, table.getEndPts());
    }

    void testGetEndPtsFor720x576()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[3] = 0x01; // DS_720_576_25
        data[4] = 0x00;
        data[5] = 0x00;
        data[6] = 0x00;
        data[7] = 0x00; // PTS = 0
        data[8] = 0x00;
        data[9] = 0x19; // Duration = 25 frames

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        uint32_t expectedEndPts = 0 + (90000 / 25) * 25; // 90000
        CPPUNIT_ASSERT_EQUAL(expectedEndPts, table.getEndPts());
    }

    void testGetEndPtsFor1280x720()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[3] = 0x02; // DS_1280_720_60
        data[4] = 0x00;
        data[5] = 0x00;
        data[6] = 0x00;
        data[7] = 0x00; // PTS = 0
        data[8] = 0x00;
        data[9] = 0x3C; // Duration = 60 frames

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        uint32_t expectedEndPts = 0 + (90000 / 60) * 60; // 90000
        CPPUNIT_ASSERT_EQUAL(expectedEndPts, table.getEndPts());
    }

    void testGetEndPtsFor1920x1080()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[3] = 0x03; // DS_1920_1080_60
        data[4] = 0x00;
        data[5] = 0x01;
        data[6] = 0x5F;
        data[7] = 0x90; // PTS = 90000
        data[8] = 0x00;
        data[9] = 0x78; // Duration = 120 frames

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        uint32_t expectedEndPts = 90000 + (90000 / 60) * 120; // 90000 + 180000 = 270000
        CPPUNIT_ASSERT_EQUAL(expectedEndPts, table.getEndPts());
    }

    void testGetEndPtsWithZeroDuration()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        data[3] = 0x00; // DS_720_480_30
        data[4] = 0x00;
        data[5] = 0x00;
        data[6] = 0x27;
        data[7] = 0x10; // PTS = 10000
        data[8] = 0x00;
        data[9] = 0x00; // Duration = 0

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(10000), table.getEndPts());
    }

    void testGetIdReturnsCorrectValue()
    {
        std::vector<uint8_t> data = createMinimalValidData();

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(TABLE_ID_SCTE_SUB, table.getId());
    }

    void testSetTableIdModifiesId()
    {
        std::vector<uint8_t> data = createMinimalValidData();

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(TABLE_ID_SCTE_SUB, table.getId());

        table.setTableId(TABLE_ID_SCTE_SUB);
        CPPUNIT_ASSERT_EQUAL(TABLE_ID_SCTE_SUB, table.getId());
    }

    void testSetTableDataWithValidData()
    {
        std::vector<uint8_t> data1 = createMinimalValidData();
        data1[0] = 'e';
        data1[1] = 'n';
        data1[2] = 'g';

        ScteTable table(TABLE_ID_SCTE_SUB, data1.data(), data1.size());
        CPPUNIT_ASSERT_EQUAL(std::string("eng"), table.getLanguageCode());

        std::vector<uint8_t> data2 = createMinimalValidData();
        data2[0] = 'f';
        data2[1] = 'r';
        data2[2] = 'a';

        table.setTableData(data2.data(), data2.size());
        CPPUNIT_ASSERT_EQUAL(std::string("fra"), table.getLanguageCode());
    }

    void testSetTableDataWithNullPointer()
    {
        std::vector<uint8_t> data = createMinimalValidData();

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());

        CPPUNIT_ASSERT_THROW(
            table.setTableData(nullptr, 12),
            InvalidArgument
        );
    }

    void testSetTableDataWithInsufficientSize()
    {
        std::vector<uint8_t> data = createMinimalValidData();

        ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size());

        CPPUNIT_ASSERT_THROW(
            table.setTableData(data.data(), 11),
            InvalidArgument
        );
    }

    void testConstructorWithValidBitmapData()
    {
        std::vector<uint8_t> data = createValidDataWithBitmap();

        CPPUNIT_ASSERT_NO_THROW(
            ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size())
        );
    }

    void testConstructorWithNonBitmapSubtitleType()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        // Set subtitle type to something other than SIMPLE_BITMAP (0x00 in upper nibble)
        data[8] = 0x00;
        // Set non-zero bitmap length
        data[10] = 0x00;
        data[11] = 0x05;
        // Don't add bitmap data - should not throw since type is not SIMPLE_BITMAP

        CPPUNIT_ASSERT_NO_THROW(
            ScteTable table(TABLE_ID_SCTE_SUB, data.data(), data.size())
        );
    }

    void testConstructorWithBitmapLengthExceedsDataSize()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        // Set subtitle type to SIMPLE_BITMAP
        data[8] = 0x10;
        // Set bitmap length to 100 bytes but don't provide that much data
        data[10] = 0x00;
        data[11] = 0x64; // 100 bytes
        // Only 12 bytes total, so bitmap length (100) > size - 12 (0)

        CPPUNIT_ASSERT_THROW(
            ScteTable(TABLE_ID_SCTE_SUB, data.data(), data.size()),
            ParseError
        );
    }

    void testConstructorWithZeroBitmapLength()
    {
        std::vector<uint8_t> data = createMinimalValidData();
        // Set subtitle type to SIMPLE_BITMAP
        data[8] = 0x10;
        // Set bitmap length to 0
        data[10] = 0x00;
        data[11] = 0x00;

        CPPUNIT_ASSERT_THROW(
            ScteTable(TABLE_ID_SCTE_SUB, data.data(), data.size()),
            ParseError
        );
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( ScteTableTest );
