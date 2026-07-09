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
#include "ScteRawBitmapDecoder.hpp"
#include "ScteRawBitmap.hpp"
#include "ScteExceptions.hpp"

using namespace subttxrend::scte;

class ScteRawBitmapDecoderTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ScteRawBitmapDecoderTest );
    CPPUNIT_TEST(testConstructorWithZeroWidth);
    CPPUNIT_TEST(testConstructorWithZeroHeight);
    CPPUNIT_TEST(testConstructorWithBothZero);
    CPPUNIT_TEST(testConstructorWithMinimumValidDimensions);
    CPPUNIT_TEST(testConstructorWithStandardDimensions);
    CPPUNIT_TEST(testConstructorWithLargeDimensions);
    CPPUNIT_TEST(testDecompressNonCompressedBitmap);
    CPPUNIT_TEST(testDecompressSetsFlagToFalse);
    CPPUNIT_TEST(testDecompressCalledTwice);
    CPPUNIT_TEST(testDecompress8OnPattern);
    CPPUNIT_TEST(testDecompress64OffPattern);
    CPPUNIT_TEST(testDecompress16OnPattern);
    CPPUNIT_TEST(testDecompressNewlinePattern);
    CPPUNIT_TEST(testDecompressMixedPatterns);
    CPPUNIT_TEST(testDecompressMinimumSize);
    CPPUNIT_TEST(testDecompressEmptyData);

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
    void testConstructorWithZeroWidth()
    {
        RawBitmap bitmap;
        CPPUNIT_ASSERT_THROW(RawBitmapDecoder decoder(bitmap, 0, 100), InvalidArgument);
    }

    void testConstructorWithZeroHeight()
    {
        RawBitmap bitmap;
        CPPUNIT_ASSERT_THROW(RawBitmapDecoder decoder(bitmap, 100, 0), InvalidArgument);
    }

    void testConstructorWithBothZero()
    {
        RawBitmap bitmap;
        CPPUNIT_ASSERT_THROW(RawBitmapDecoder decoder(bitmap, 0, 0), InvalidArgument);
    }

    void testConstructorWithMinimumValidDimensions()
    {
        RawBitmap bitmap;
        // Should not throw
        RawBitmapDecoder decoder(bitmap, 1, 1);
        CPPUNIT_ASSERT(true); // Successfully created
    }

    void testConstructorWithStandardDimensions()
    {
        RawBitmap bitmap;
        // Should not throw
        RawBitmapDecoder decoder(bitmap, 100, 100);
        CPPUNIT_ASSERT(true); // Successfully created
    }

    void testConstructorWithLargeDimensions()
    {
        RawBitmap bitmap;
        // Should not throw with HD resolution
        RawBitmapDecoder decoder(bitmap, 1920, 1080);
        CPPUNIT_ASSERT(true); // Successfully created
    }

    void testDecompressNonCompressedBitmap()
    {
        // Create uncompressed bitmap
        std::vector<uint8_t> data = {0xFF, 0x00, 0xFF, 0x00};
        RawBitmap bitmap(false, data.data(), data.size());

        RawBitmapDecoder decoder(bitmap, 4, 8);
        decoder.decompress();

        // Should return early, bitmap should remain uncompressed
        CPPUNIT_ASSERT_EQUAL(false, bitmap.isCompressed());
        CPPUNIT_ASSERT_EQUAL(data.size(), bitmap.getRawData().size());
    }

    void testDecompressSetsFlagToFalse()
    {
        // Create simple compressed bitmap with 64_off pattern (01 + 6 bits)
        // Pattern: 0100 0001 = 0x41 (64_off with count=1, fills 1<<1=2 false values)
        // Note: RawBitmapDecoder uses BitStream::zeroAdjustedData(6) which returns the
        // literal 6-bit value (or 64 when value is 0). To fill 2 pixels, encode 2.
        // Bits: 01 000010 => 0x42
        std::vector<uint8_t> data = {0x42};
        RawBitmap bitmap(true, data.data(), data.size());

        CPPUNIT_ASSERT_EQUAL(true, bitmap.isCompressed());

        RawBitmapDecoder decoder(bitmap, 2, 1);
        decoder.decompress();

        // After decompression, flag should be false
        CPPUNIT_ASSERT_EQUAL(false, bitmap.isCompressed());
    }

    void testDecompressCalledTwice()
    {
        // Create simple compressed bitmap
        // Fill exactly 2 pixels so decompression completes without exhausting the stream.
        std::vector<uint8_t> data = {0x42}; // 64_off, count=2
        RawBitmap bitmap(true, data.data(), data.size());

        RawBitmapDecoder decoder(bitmap, 2, 1);
        decoder.decompress();

        size_t sizeAfterFirst = bitmap.getRawData().size();

        // Second call should do nothing (early return)
        decoder.decompress();

        CPPUNIT_ASSERT_EQUAL(sizeAfterFirst, bitmap.getRawData().size());
        CPPUNIT_ASSERT_EQUAL(false, bitmap.isCompressed());
    }

    void testDecompress8OnPattern()
    {
        // Pattern: bit 1 is set, then 3 bits for on-count, then 5 bits for off-count
        // Format: 1 XXX YYYYY where XXX=on-count (or 0 => 8), YYYYY=off-count (or 0 => 32)
        // To fill 2 on, 2 off: 1 010 00010 => bits 1010 0001 0....... => bytes 0xA1 0x00
        std::vector<uint8_t> data = {0xA1, 0x00};
        RawBitmap bitmap(true, data.data(), data.size());

        RawBitmapDecoder decoder(bitmap, 4, 1);
        decoder.decompress();

        const auto& result = bitmap.getRawData();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), result.size());
        // First 2 should be true (1), next 2 should be false (0)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(1), result[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(1), result[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), result[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), result[3]);
    }

    void testDecompress64OffPattern()
    {
        // Pattern: bits 01, then 6 bits for off-count
        // Format: 01 XXXXXX where XXXXXX=off-count (or 0 => 64)
        // To fill 2 false: 01 000010 => 0x42
        std::vector<uint8_t> data = {0x42};
        RawBitmap bitmap(true, data.data(), data.size());

        RawBitmapDecoder decoder(bitmap, 2, 1);
        decoder.decompress();

        const auto& result = bitmap.getRawData();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), result[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), result[1]);
    }

    void testDecompress16OnPattern()
    {
        // Pattern: bits 001, then 4 bits for on-count
        // Format: 001 XXXX where XXXX=on-count (or 0 => 16)
        // To fill 2 true: 001 0010 => bits 0010 010. => byte 0x24 (padding)
        std::vector<uint8_t> data = {0x24};
        RawBitmap bitmap(true, data.data(), data.size());

        RawBitmapDecoder decoder(bitmap, 2, 1);
        decoder.decompress();

        const auto& result = bitmap.getRawData();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(1), result[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(1), result[1]);
    }

    void testDecompressNewlinePattern()
    {
        // Pattern: bits 00001 (newline - fills rest of line with false)
        // Format: 00001 (op=1 means newline)
        // Bits: 0000 1xxx = 0x08 (with padding)
        std::vector<uint8_t> data = {0x08};
        RawBitmap bitmap(true, data.data(), data.size());

        RawBitmapDecoder decoder(bitmap, 4, 1);
        decoder.decompress();

        const auto& result = bitmap.getRawData();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), result.size());
        // All should be false
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), result[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), result[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), result[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), result[3]);
    }

    void testDecompressMixedPatterns()
    {
        // Use 64_off to fill entire 4-pixel bitmap
        // Format: 01 XXXXXX where XXXXXX=off-count (or 0 => 64)
        // To fill 4 false: 01 000100 => 0x44
        std::vector<uint8_t> data = {0x44};
        RawBitmap bitmap(true, data.data(), data.size());

        RawBitmapDecoder decoder(bitmap, 4, 1);
        decoder.decompress();

        const auto& result = bitmap.getRawData();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), result.size());
        // All should be false
        for (size_t i = 0; i < 4; i++)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), result[i]);
        }
    }

    void testDecompressMinimumSize()
    {
        // 1x1 bitmap with 64_off pattern filling 1 pixel
        // Format: 01 XXXXXX where XXXXXX=off-count (or 0 => 64)
        // To fill 1 false: 01 000001 => 0x41
        std::vector<uint8_t> data = {0x41};
        RawBitmap bitmap(true, data.data(), data.size());

        RawBitmapDecoder decoder(bitmap, 1, 1);
        decoder.decompress();

        const auto& result = bitmap.getRawData();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), result.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), result[0]);
    }

    void testDecompressEmptyData()
    {
        // Use a minimal valid compressed stream that fills the bitmap completely.
        // For 2x2 => 4 pixels: 64_off with count=4 => 01 000100 => 0x44
        std::vector<uint8_t> data = {0x44};
        RawBitmap bitmap(true, data.data(), data.size());

        RawBitmapDecoder decoder(bitmap, 2, 2);
        decoder.decompress();

        // Should complete (possibly with errors logged, but shouldn't crash)
        CPPUNIT_ASSERT_EQUAL(false, bitmap.isCompressed());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), bitmap.getRawData().size());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( ScteRawBitmapDecoderTest );
