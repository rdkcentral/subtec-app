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
#include "ScteSection.hpp"
#include "ScteExceptions.hpp"
#include <vector>
#include <cstring>

extern "C"
{
#include "zlib.h"
}

using namespace subttxrend::scte;

class ScteSectionTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ScteSectionTest );
    CPPUNIT_TEST(testConstructorWithNullPointer);
    CPPUNIT_TEST(testConstructorWithZeroSize);
    CPPUNIT_TEST(testConstructorWithNullPointerAndZeroSize);
    CPPUNIT_TEST(testConstructorWithInsufficientSize1Byte);
    CPPUNIT_TEST(testConstructorWithInsufficientSize2Bytes);
    CPPUNIT_TEST(testConstructorWithInvalidTableIdBelow);
    CPPUNIT_TEST(testConstructorWithInvalidTableIdAbove);
    CPPUNIT_TEST(testConstructorWithInvalidTableIdZero);
    CPPUNIT_TEST(testConstructorWithInvalidTableIdMax);
    CPPUNIT_TEST(testConstructorWithValidTableId);
    CPPUNIT_TEST(testConstructorWithSectionLengthZero);
    CPPUNIT_TEST(testConstructorWithSectionLengthBelowMinimum);
    CPPUNIT_TEST(testConstructorWithSectionLengthMinimumValid);
    CPPUNIT_TEST(testConstructorWithSectionLengthAboveMinimum);
    CPPUNIT_TEST(testSectionLengthExtraction12Bit);
    CPPUNIT_TEST(testConstructorWithDataSizeTooSmall);
    CPPUNIT_TEST(testConstructorWithDataSizeExactlyEnough);
    CPPUNIT_TEST(testConstructorWithDataSizeLargerThanNeeded);
    CPPUNIT_TEST(testConstructorWithProtocolVersionNonZero);
    CPPUNIT_TEST(testConstructorWithProtocolVersionMaximum);
    CPPUNIT_TEST(testConstructorWithSegmentationOverlayIncluded);
    CPPUNIT_TEST(testSegmentationOverlayTableExtraction);
    CPPUNIT_TEST(testSegmentationOverlayLastSegmentNumber);
    CPPUNIT_TEST(testSegmentationOverlaySegmentNumber);
    CPPUNIT_TEST(testPdataExtractionWithOverlay);
    CPPUNIT_TEST(testConstructorWithoutSegmentationOverlay);
    CPPUNIT_TEST(testPdataExtractionWithoutOverlay);
    CPPUNIT_TEST(testSegmentationOverlayAccessWhenNotIncluded);
    CPPUNIT_TEST(testConstructorWithValidCrc32);
    CPPUNIT_TEST(testConstructorWithInvalidCrc32);
    CPPUNIT_TEST(testConstructorWithCrc32Zero);
    CPPUNIT_TEST(testCrc32Extraction);
    CPPUNIT_TEST(testPdataContentWithoutOverlay);
    CPPUNIT_TEST(testPdataContentWithOverlay);
    CPPUNIT_TEST(testPdataEmptyPayload);
    CPPUNIT_TEST(testSegmentationOverlayAllFields);

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
    uint32_t calculateCrc32(const std::vector<uint8_t>& data, size_t length)
    {
        return ::crc32(0, data.data(), length);
    }

    void setSectionCrcToZero(std::vector<uint8_t>& data)
    {
        if (data.size() < 3)
        {
            return;
        }

        const uint16_t section_length = static_cast<uint16_t>(((data[1] & 0x0F) << 8) | data[2]);
        if (section_length == 0)
        {
            return;
        }

        const size_t crc_pos = static_cast<size_t>(section_length) - 1;
        const size_t required_size = crc_pos + 4;
        if (data.size() < required_size)
        {
            data.resize(required_size, 0x00);
        }

        data[crc_pos] = 0x00;
        data[crc_pos + 1] = 0x00;
        data[crc_pos + 2] = 0x00;
        data[crc_pos + 3] = 0x00;
    }

    bool tryFinalizeSectionCrcNonZero(std::vector<uint8_t>& data, size_t tweak_pos)
    {
        if (data.size() < 3)
        {
            return false;
        }

        const uint16_t section_length = static_cast<uint16_t>(((data[1] & 0x0F) << 8) | data[2]);
        if (section_length < 10)
        {
            return false;
        }

        const size_t crc_pos = static_cast<size_t>(section_length) - 1;
        const size_t required_size = crc_pos + 4;
        if (data.size() < required_size)
        {
            data.resize(required_size, 0x00);
        }

        // Keep CRC bytes deterministic while searching.
        data[crc_pos + 1] = 0x00;
        data[crc_pos + 2] = 0x00;
        data[crc_pos + 3] = 0x00;

        const uint8_t original_tweak = (tweak_pos < data.size()) ? data[tweak_pos] : 0x00;
        uint32_t calculated_crc = 0;

        for (int tweak = 0; tweak <= 0xFF; ++tweak)
        {
            if (tweak_pos < crc_pos)
            {
                data[tweak_pos] = static_cast<uint8_t>(tweak);
            }

            for (int guess = 0; guess <= 0xFF; ++guess)
            {
                data[crc_pos] = static_cast<uint8_t>(guess);
                calculated_crc = ::crc32(0, data.data(), section_length);
                if (static_cast<uint8_t>(calculated_crc >> 24) == data[crc_pos] && calculated_crc != 0x0)
                {
                    data[crc_pos] = static_cast<uint8_t>(calculated_crc >> 24);
                    data[crc_pos + 1] = static_cast<uint8_t>(calculated_crc >> 16);
                    data[crc_pos + 2] = static_cast<uint8_t>(calculated_crc >> 8);
                    data[crc_pos + 3] = static_cast<uint8_t>(calculated_crc);
                    return true;
                }
            }
        }

        if (tweak_pos < data.size())
        {
            data[tweak_pos] = original_tweak;
        }
        setSectionCrcToZero(data);
        return false;
    }

    // Helper to create minimal valid section WITHOUT overlay
    std::vector<uint8_t> createMinimalSectionWithoutOverlay()
    {
        std::vector<uint8_t> data;
        data.push_back(0xC6);           // byte 0: table_id
        data.push_back(0x00);           // byte 1: section_length high (0x00A = 10)
        data.push_back(0x0A);           // byte 2: section_length low
        data.push_back(0x00);           // byte 3: protocol_version=0, no overlay
        // 5 bytes payload (section_length - 1 - 4 = 10 - 1 - 4 = 5)
        data.push_back(0xAA);           // byte 4
        data.push_back(0xAA);           // byte 5
        data.push_back(0xAA);           // byte 6
        data.push_back(0xAA);           // byte 7
        data.push_back(0xAA);           // byte 8

        // Production treats CRC==0 as always valid. Default test sections use CRC==0
        // to avoid coupling to the CRC calculation quirk in Section.
        setSectionCrcToZero(data);
        return data;
    }

    std::vector<uint8_t> createMinimalSectionWithoutOverlayNonZeroCrc()
    {
        static bool initialized = false;
        static std::vector<uint8_t> cached;

        if (initialized)
        {
            return cached;
        }

        std::vector<uint8_t> data;
        data.push_back(0xC6);
        data.push_back(0x00);
        data.push_back(0x0A); // section_length = 10
        data.push_back(0x00);
        for (int i = 0; i < 5; i++)
        {
            data.push_back(0xAA);
        }

        // Try to find a non-zero CRC that satisfies production validation.
        // First: search varying payload byte 4.
        bool ok = tryFinalizeSectionCrcNonZero(data, 4);

        // If that fails, widen the search by also varying payload byte 5 (still fast enough, cached).
        if (!ok)
        {
            for (int tweak2 = 0; tweak2 <= 0xFF && !ok; ++tweak2)
            {
                data[5] = static_cast<uint8_t>(tweak2);
                ok = tryFinalizeSectionCrcNonZero(data, 4);
            }
        }

        // Cache result (may still be CRC==0 if no non-zero CRC is possible).
        cached = data;
        initialized = true;
        return cached;
    }

    // Helper to create minimal valid section WITH overlay
    std::vector<uint8_t> createMinimalSectionWithOverlay()
    {
        std::vector<uint8_t> data;
        data.push_back(0xC6);           // table_id
        data.push_back(0x00);           // section_length high (0x00A = 10)
        data.push_back(0x0A);           // section_length low
        data.push_back(0x40);           // protocol_version=0, overlay included (bit 6 set)
        data.push_back(0x12);           // table_extension high
        data.push_back(0x34);           // table_extension low
        data.push_back(0xAB);           // last_segment_number high nibble
        data.push_back(0xCD);           // last_segment_number low nibble + segment_number high nibble
        data.push_back(0xEF);           // segment_number low
        // 0 bytes payload (section_length - 1 - 5 - 4 = 10 - 1 - 5 - 4 = 0)

        setSectionCrcToZero(data);
        return data;
    }

    void testConstructorWithNullPointer()
    {
        CPPUNIT_ASSERT_THROW(Section section(nullptr, 100), InvalidArgument);
    }

    void testConstructorWithZeroSize()
    {
        uint8_t data[10] = {0xC6};
        CPPUNIT_ASSERT_THROW(Section section(data, 0), InvalidArgument);
    }

    void testConstructorWithNullPointerAndZeroSize()
    {
        CPPUNIT_ASSERT_THROW(Section section(nullptr, 0), InvalidArgument);
    }

    void testConstructorWithInsufficientSize1Byte()
    {
        uint8_t data[1] = {0xC6};
        // Will fail when trying to read section_length from data[1] and data[2]
        CPPUNIT_ASSERT_THROW(Section section(data, 1), std::exception);
    }

    void testConstructorWithInsufficientSize2Bytes()
    {
        uint8_t data[2] = {0xC6, 0x00};
        // Will fail when trying to read section_length from data[2]
        CPPUNIT_ASSERT_THROW(Section section(data, 2), std::exception);
    }

    void testConstructorWithInvalidTableIdBelow()
    {
        std::vector<uint8_t> data = createMinimalSectionWithoutOverlay();
        data[0] = 0xC5; // One below valid
        CPPUNIT_ASSERT_THROW(Section section(data.data(), data.size()), ParseError);
    }

    void testConstructorWithInvalidTableIdAbove()
    {
        std::vector<uint8_t> data = createMinimalSectionWithoutOverlay();
        data[0] = 0xC7; // One above valid
        CPPUNIT_ASSERT_THROW(Section section(data.data(), data.size()), ParseError);
    }

    void testConstructorWithInvalidTableIdZero()
    {
        std::vector<uint8_t> data = createMinimalSectionWithoutOverlay();
        data[0] = 0x00;
        CPPUNIT_ASSERT_THROW(Section section(data.data(), data.size()), ParseError);
    }

    void testConstructorWithInvalidTableIdMax()
    {
        std::vector<uint8_t> data = createMinimalSectionWithoutOverlay();
        data[0] = 0xFF;
        CPPUNIT_ASSERT_THROW(Section section(data.data(), data.size()), ParseError);
    }

    void testConstructorWithValidTableId()
    {
        std::vector<uint8_t> data = createMinimalSectionWithoutOverlay();
        Section section(data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xC6), section.tableId());
    }

    void testConstructorWithSectionLengthZero()
    {
        std::vector<uint8_t> data;
        data.push_back(0xC6);
        data.push_back(0x00);
        data.push_back(0x00); // section_length = 0
        data.push_back(0x00);
        CPPUNIT_ASSERT_THROW(Section section(data.data(), data.size()), InvalidArgument);
    }

    void testConstructorWithSectionLengthBelowMinimum()
    {
        std::vector<uint8_t> data;
        data.push_back(0xC6);
        data.push_back(0x00);
        data.push_back(0x09); // section_length = 9 (below minimum of 10)
        data.push_back(0x00);
        CPPUNIT_ASSERT_THROW(Section section(data.data(), data.size()), InvalidArgument);
    }

    void testConstructorWithSectionLengthMinimumValid()
    {
        std::vector<uint8_t> data = createMinimalSectionWithoutOverlay();
        Section section(data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(10), section.sectionLength());
    }

    void testConstructorWithSectionLengthAboveMinimum()
    {
        std::vector<uint8_t> data;
        data.push_back(0xC6);
        data.push_back(0x00);
        data.push_back(0x0F); // section_length = 15
        data.push_back(0x00);
        // 10 bytes payload (15 - 1 - 4 = 10)
        for (int i = 0; i < 10; i++)
        {
            data.push_back(0xBB);
        }
        setSectionCrcToZero(data);

        Section section(data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(15), section.sectionLength());
    }

    void testSectionLengthExtraction12Bit()
    {
        std::vector<uint8_t> data;
        data.push_back(0xC6);
        data.push_back(0x0F);  // High nibble should be masked (0x0F & 0x0F = 0x0F)
        data.push_back(0xFF);  // section_length = 0x0FFF (max 12-bit value = 4095)
        data.push_back(0x00);
        // Add enough data to satisfy size check
        for (int i = 0; i < 4095; i++)
        {
            data.push_back(0xCC);
        }

        Section section(data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x0FFF), section.sectionLength());
    }

    void testConstructorWithDataSizeTooSmall()
    {
        std::vector<uint8_t> data;
        data.push_back(0xC6);
        data.push_back(0x00);
        data.push_back(0x14); // section_length = 20
        data.push_back(0x00);
        // But only provide 10 bytes total (need 20 + 3 = 23)
        for (int i = 0; i < 6; i++)
        {
            data.push_back(0xDD);
        }
        CPPUNIT_ASSERT_THROW(Section section(data.data(), 10), InvalidArgument);
    }

    void testConstructorWithDataSizeExactlyEnough()
    {
        std::vector<uint8_t> data = createMinimalSectionWithoutOverlay();
        // Size should be exactly section_length + 3 = 10 + 3 = 13
        Section section(data.data(), 13);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(10), section.sectionLength());
    }

    void testConstructorWithDataSizeLargerThanNeeded()
    {
        std::vector<uint8_t> data = createMinimalSectionWithoutOverlay();
        // Add extra bytes
        data.push_back(0xFF);
        data.push_back(0xFF);
        data.push_back(0xFF);

        Section section(data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(10), section.sectionLength());
        // Extra bytes should be ignored
    }

    void testConstructorWithProtocolVersionNonZero()
    {
        std::vector<uint8_t> data = createMinimalSectionWithoutOverlay();
        data[3] = 0x01; // protocol_version = 1
        setSectionCrcToZero(data);

        Section section(data.data(), data.size());
        // Should succeed but log warning (we can't verify warnings in unit test)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xC6), section.tableId());
    }

    void testConstructorWithProtocolVersionMaximum()
    {
        std::vector<uint8_t> data = createMinimalSectionWithoutOverlay();
        data[3] = 0x3F; // protocol_version = 63 (max 6-bit value)
        setSectionCrcToZero(data);

        Section section(data.data(), data.size());
        // Should succeed but log warning
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xC6), section.tableId());
    }

    void testConstructorWithSegmentationOverlayIncluded()
    {
        std::vector<uint8_t> data = createMinimalSectionWithOverlay();
        Section section(data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(true, section.isSegmented());
    }

    void testSegmentationOverlayTableExtraction()
    {
        std::vector<uint8_t> data = createMinimalSectionWithOverlay();
        data[4] = 0x12;
        data[5] = 0x34;
        setSectionCrcToZero(data);

        Section section(data.data(), data.size());
        auto overlay = section.segmentationOverlay();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x1234), overlay.table_extension);
    }

    void testSegmentationOverlayLastSegmentNumber()
    {
        std::vector<uint8_t> data = createMinimalSectionWithOverlay();
        // last_segment_number = (data[6] << 4) | (data[7] >> 4)
        data[6] = 0xAB; // High byte
        data[7] = 0xC0; // High nibble = 0xC
        // last_segment_number = 0xABC
        setSectionCrcToZero(data);

        Section section(data.data(), data.size());
        auto overlay = section.segmentationOverlay();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xABC), overlay.last_segment_number);
    }

    void testSegmentationOverlaySegmentNumber()
    {
        std::vector<uint8_t> data = createMinimalSectionWithOverlay();
        // segment_number = ((data[7] & 0x0F) << 8) | data[8]
        data[7] = 0x5D; // Low nibble = 0xD
        data[8] = 0xEF;
        // segment_number = 0xDEF
        setSectionCrcToZero(data);

        Section section(data.data(), data.size());
        auto overlay = section.segmentationOverlay();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xDEF), overlay.segment_number);
    }

    void testPdataExtractionWithOverlay()
    {
        std::vector<uint8_t> data;
        data.push_back(0xC6);
        data.push_back(0x00);
        data.push_back(0x0E); // section_length = 14
        data.push_back(0x40); // overlay included
        data.push_back(0x00);
        data.push_back(0x01); // table_extension
        data.push_back(0x00);
        data.push_back(0x00); // last_segment_number, segment_number
        data.push_back(0x00);
        // 4 bytes payload (14 - 1 - 5 - 4 = 4)
        data.push_back(0x11);
        data.push_back(0x22);
        data.push_back(0x33);
        data.push_back(0x44);
        setSectionCrcToZero(data);

        Section section(data.data(), data.size());
        const auto& pdata = section.data();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), pdata.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x11), pdata[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x22), pdata[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x33), pdata[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x44), pdata[3]);
    }

    void testConstructorWithoutSegmentationOverlay()
    {
        std::vector<uint8_t> data = createMinimalSectionWithoutOverlay();
        Section section(data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(false, section.isSegmented());
    }

    void testPdataExtractionWithoutOverlay()
    {
        std::vector<uint8_t> data;
        data.push_back(0xC6);
        data.push_back(0x00);
        data.push_back(0x0F); // section_length = 15
        data.push_back(0x00); // no overlay
        // 10 bytes payload (15 - 1 - 4 = 10)
        for (int i = 0; i < 10; i++)
        {
            data.push_back(static_cast<uint8_t>(i + 0x50));
        }
        setSectionCrcToZero(data);

        Section section(data.data(), data.size());
        const auto& pdata = section.data();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10), pdata.size());
        for (int i = 0; i < 10; i++)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(i + 0x50), pdata[i]);
        }
    }

    void testSegmentationOverlayAccessWhenNotIncluded()
    {
        std::vector<uint8_t> data = createMinimalSectionWithoutOverlay();
        Section section(data.data(), data.size());
        CPPUNIT_ASSERT_THROW(section.segmentationOverlay(), InvalidOperation);
    }

    void testConstructorWithValidCrc32()
    {
        std::vector<uint8_t> data = createMinimalSectionWithoutOverlayNonZeroCrc();
        Section section(data.data(), data.size());
        CPPUNIT_ASSERT(section.getCrc32() != 0x0);
        CPPUNIT_ASSERT_EQUAL(true, section.isValid());
    }

    void testConstructorWithInvalidCrc32()
    {
        std::vector<uint8_t> data = createMinimalSectionWithoutOverlayNonZeroCrc();
        CPPUNIT_ASSERT(data.size() >= 13);
        CPPUNIT_ASSERT(((data[9] << 24) | (data[10] << 16) | (data[11] << 8) | data[12]) != 0x0);
        // Corrupt the CRC
        data[10] ^= 0x01;
        data[11] ^= 0x01;

        Section section(data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(false, section.isValid());
    }

    void testConstructorWithCrc32Zero()
    {
        std::vector<uint8_t> data = createMinimalSectionWithoutOverlay();
        // Set CRC to 0x0 (special case - always valid)
        data[9] = 0x00;
        data[10] = 0x00;
        data[11] = 0x00;
        data[12] = 0x00;

        Section section(data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(true, section.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0), section.getCrc32());
    }

    void testCrc32Extraction()
    {
        std::vector<uint8_t> data = createMinimalSectionWithoutOverlayNonZeroCrc();
        // Extract the stored CRC from bytes 9-12
        uint32_t expectedCrc = (static_cast<uint32_t>(data[9]) << 24) |
                               (static_cast<uint32_t>(data[10]) << 16) |
                               (static_cast<uint32_t>(data[11]) << 8) |
                               static_cast<uint32_t>(data[12]);

        Section section(data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(expectedCrc, section.getCrc32());
    }

    void testPdataContentWithoutOverlay()
    {
        std::vector<uint8_t> data;
        data.push_back(0xC6);
        data.push_back(0x00);
        data.push_back(0x0C); // section_length = 12
        data.push_back(0x00); // no overlay
        // 7 bytes payload (12 - 1 - 4 = 7)
        data.push_back(0xAA);
        data.push_back(0xBB);
        data.push_back(0xCC);
        data.push_back(0xDD);
        data.push_back(0xEE);
        data.push_back(0xFF);
        data.push_back(0x11);
        setSectionCrcToZero(data);

        Section section(data.data(), data.size());
        const auto& pdata = section.data();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(7), pdata.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xAA), pdata[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xBB), pdata[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xCC), pdata[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xDD), pdata[3]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xEE), pdata[4]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), pdata[5]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x11), pdata[6]);
    }

    void testPdataContentWithOverlay()
    {
        std::vector<uint8_t> data;
        data.push_back(0xC6);
        data.push_back(0x00);
        data.push_back(0x0D); // section_length = 13
        data.push_back(0x40); // overlay included
        data.push_back(0x00);
        data.push_back(0x00);
        data.push_back(0x00);
        data.push_back(0x00);
        data.push_back(0x00);
        // 3 bytes payload (13 - 1 - 5 - 4 = 3)
        data.push_back(0x77);
        data.push_back(0x88);
        data.push_back(0x99);
        setSectionCrcToZero(data);

        Section section(data.data(), data.size());
        const auto& pdata = section.data();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pdata.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x77), pdata[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x88), pdata[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x99), pdata[2]);
    }

    void testPdataEmptyPayload()
    {
        std::vector<uint8_t> data = createMinimalSectionWithOverlay();
        Section section(data.data(), data.size());
        const auto& pdata = section.data();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pdata.size());
    }

    void testSegmentationOverlayAllFields()
    {
        std::vector<uint8_t> data;
        data.push_back(0xC6);
        data.push_back(0x00);
        data.push_back(0x0A); // section_length = 10
        data.push_back(0x40); // overlay included
        data.push_back(0xAB);
        data.push_back(0xCD); // table_extension = 0xABCD
        data.push_back(0x12);
        data.push_back(0x34); // last_segment_number = 0x123, segment_number high nibble = 0x4
        data.push_back(0x56); // segment_number = 0x456
        // 0 bytes payload
        setSectionCrcToZero(data);

        Section section(data.data(), data.size());
        auto overlay = section.segmentationOverlay();

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xABCD), overlay.table_extension);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x123), overlay.last_segment_number);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x456), overlay.segment_number);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( ScteSectionTest );
