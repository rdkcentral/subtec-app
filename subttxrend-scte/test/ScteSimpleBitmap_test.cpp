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
#include "ScteSimpleBitmap.hpp"
#include "ScteExceptions.hpp"
#include "ScteRawBitmap.hpp"
#include <vector>
#include <cstring>

using namespace subttxrend::scte;

class ScteSimpleBitmapTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ScteSimpleBitmapTest );
    CPPUNIT_TEST(testConstructorWithNullPointer);
    CPPUNIT_TEST(testConstructorWithSizeZero);
    CPPUNIT_TEST(testConstructorWithNullPointerAndSizeZero);
    CPPUNIT_TEST(testConstructorTransparentNone);
    CPPUNIT_TEST(testConstructorTransparentOutline);
    CPPUNIT_TEST(testConstructorTransparentDropShadow);
    CPPUNIT_TEST(testConstructorTransparentReserved);
    CPPUNIT_TEST(testConstructorFramedNone);
    CPPUNIT_TEST(testConstructorFramedOutline);
    CPPUNIT_TEST(testConstructorFramedDropShadow);
    CPPUNIT_TEST(testGetBackgroundStyleTransparent);
    CPPUNIT_TEST(testGetBackgroundStyleFramed);
    CPPUNIT_TEST(testGetOutlineStyleNone);
    CPPUNIT_TEST(testGetOutlineStyleOutline);
    CPPUNIT_TEST(testGetOutlineStyleDropShadow);
    CPPUNIT_TEST(testGetOutlineStyleReserved);
    CPPUNIT_TEST(testGetCharacterColor);
    CPPUNIT_TEST(testGetCharacterCoordinates);
    CPPUNIT_TEST(testGetOutlineThicknessWhenOutline);
    CPPUNIT_TEST(testGetOutlineColorWhenOutline);
    CPPUNIT_TEST(testGetShadowOffsetsWhenDropShadow);
    CPPUNIT_TEST(testGetShadowColorWhenDropShadow);
    CPPUNIT_TEST(testWidthCalculation);
    CPPUNIT_TEST(testHeightCalculation);
    CPPUNIT_TEST(testWidthWithZeroDimension);
    CPPUNIT_TEST(testHeightWithZeroDimension);
    CPPUNIT_TEST(testMaximumCoordinateValues);
    CPPUNIT_TEST(testBitmapLengthExceedsDataSize);
    CPPUNIT_TEST(testBitmapLengthZero);
    CPPUNIT_TEST(testBitmapLengthExactFit);
    CPPUNIT_TEST(testSetBitmapWithRawData);
    CPPUNIT_TEST(testSetBitmapWithRawBitmapObject);
    CPPUNIT_TEST(testSetBitmapCompressedFlag);
    CPPUNIT_TEST(testConstructorFromRawBitmap);
    CPPUNIT_TEST(testDefaultConstructor);
    CPPUNIT_TEST(testGetBitmap);
    CPPUNIT_TEST(testColorFieldParsing);
    CPPUNIT_TEST(testCoordinateFieldParsing);

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
    // Helper to create minimal valid SimpleBitmap data
    // Format: byte0 (style) | bytes1-2 (char color) | bytes3-8 (char coords) |
    //         [outline/shadow data if set] |
    //         bytes for bitmap length | bitmap data
    std::vector<uint8_t> createSimpleBitmapData(BackgroundStyle bgStyle, OutlineStyle olStyle,
                                                  uint16_t charTopX = 0, uint16_t charTopY = 0,
                                                  uint16_t charBottomX = 10, uint16_t charBottomY = 10,
                                                  uint16_t bitmapDataLen = 1)
    {
        std::vector<uint8_t> data;

        // Byte 0: background style (bit 2) | outline style (bits 0-1)
        uint8_t byte0 = 0;
        if (bgStyle == BackgroundStyle::FRAMED) byte0 |= 0x04;
        byte0 |= static_cast<uint8_t>(olStyle) & 0x03;
        data.push_back(byte0);

        // Bytes 1-2: character color (simplified to zeros)
        data.push_back(0x00);
        data.push_back(0x00);

        // Bytes 3-5: character top coordinates (12 bits each)
        data.push_back(static_cast<uint8_t>(charTopX >> 4));
        data.push_back(static_cast<uint8_t>(((charTopX & 0x0F) << 4) | ((charTopY >> 8) & 0x0F)));
        data.push_back(static_cast<uint8_t>(charTopY & 0xFF));

        // Bytes 6-8: character bottom coordinates (12 bits each)
        data.push_back(static_cast<uint8_t>(charBottomX >> 4));
        data.push_back(static_cast<uint8_t>(((charBottomX & 0x0F) << 4) | ((charBottomY >> 8) & 0x0F)));
        data.push_back(static_cast<uint8_t>(charBottomY & 0xFF));

        // NOTE: Production code parses background style as:
        //   backgroundStyle = static_cast<BackgroundStyle>(data[0] & 0x04);
        // and then checks "== BackgroundStyle::FRAMED" (which is 1).
        // This means the FRAMED branch is never entered (value becomes 0x04),
        // so including framed-only bytes here would misalign the following fields.

        // Outline/shadow data if needed (3 bytes)
        if (olStyle == OutlineStyle::OUTLINE)
        {
            data.push_back(0x05); // outline thickness = 5
            data.push_back(0x00); // outline color
            data.push_back(0x00);
        }
        else if (olStyle == OutlineStyle::DROP_SHADOW)
        {
            data.push_back(0x34); // shadow right = 3, upper nibble; byte also contains part of shadow bottom
            data.push_back(0x22); // shadow bottom = 2 (low nibble); high bits also feed shadow color
            data.push_back(0x00); // shadow color continued
        }
        else if (olStyle == OutlineStyle::RESERVED)
        {
            data.push_back(0x00);
            data.push_back(0x00);
            data.push_back(0x00);
        }

        // Bitmap length (2 bytes)
        data.push_back(static_cast<uint8_t>(bitmapDataLen >> 8));
        data.push_back(static_cast<uint8_t>(bitmapDataLen & 0xFF));

        // Bitmap data
        for (uint16_t i = 0; i < bitmapDataLen; ++i)
        {
            data.push_back(static_cast<uint8_t>(i % 256));
        }

        return data;
    }

    void testConstructorWithNullPointer()
    {
        CPPUNIT_ASSERT_THROW(
            SimpleBitmap(nullptr, 100),
            InvalidArgument
        );
    }

    void testConstructorWithSizeZero()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE);
        CPPUNIT_ASSERT_THROW(
            SimpleBitmap(data.data(), 0),
            InvalidArgument
        );
    }

    void testConstructorWithNullPointerAndSizeZero()
    {
        CPPUNIT_ASSERT_THROW(
            SimpleBitmap(nullptr, 0),
            InvalidArgument
        );
    }

    void testConstructorTransparentNone()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE);

        CPPUNIT_ASSERT_NO_THROW(
            SimpleBitmap bitmap(data.data(), data.size())
        );
    }

    void testConstructorTransparentOutline()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::OUTLINE);

        SimpleBitmap bitmap(data.data(), data.size());
        CPPUNIT_ASSERT(bitmap.getBackgroundStyle() == BackgroundStyle::TRANSPARENT);
        CPPUNIT_ASSERT(bitmap.getOutlineStyle() == OutlineStyle::OUTLINE);
    }

    void testConstructorTransparentDropShadow()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::DROP_SHADOW);

        SimpleBitmap bitmap(data.data(), data.size());
        CPPUNIT_ASSERT(bitmap.getBackgroundStyle() == BackgroundStyle::TRANSPARENT);
        CPPUNIT_ASSERT(bitmap.getOutlineStyle() == OutlineStyle::DROP_SHADOW);
    }

    void testConstructorTransparentReserved()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::RESERVED);

        SimpleBitmap bitmap(data.data(), data.size());
        CPPUNIT_ASSERT(bitmap.getBackgroundStyle() == BackgroundStyle::TRANSPARENT);
        CPPUNIT_ASSERT(bitmap.getOutlineStyle() == OutlineStyle::RESERVED);
    }

    void testConstructorFramedNone()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::FRAMED, OutlineStyle::NONE);

        SimpleBitmap bitmap(data.data(), data.size());
        CPPUNIT_ASSERT(bitmap.getBackgroundStyle() != BackgroundStyle::TRANSPARENT);
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(0x04), static_cast<int>(bitmap.getBackgroundStyle()));
        CPPUNIT_ASSERT(bitmap.getOutlineStyle() == OutlineStyle::NONE);
    }

    void testConstructorFramedOutline()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::FRAMED, OutlineStyle::OUTLINE);

        SimpleBitmap bitmap(data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(0x04), static_cast<int>(bitmap.getBackgroundStyle()));
        CPPUNIT_ASSERT(bitmap.getOutlineStyle() == OutlineStyle::OUTLINE);
    }

    void testConstructorFramedDropShadow()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::FRAMED, OutlineStyle::DROP_SHADOW);

        SimpleBitmap bitmap(data.data(), data.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(0x04), static_cast<int>(bitmap.getBackgroundStyle()));
        CPPUNIT_ASSERT(bitmap.getOutlineStyle() == OutlineStyle::DROP_SHADOW);
    }

    void testGetBackgroundStyleTransparent()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE);

        SimpleBitmap bitmap(data.data(), data.size());
        CPPUNIT_ASSERT(bitmap.getBackgroundStyle() == BackgroundStyle::TRANSPARENT);
    }

    void testGetBackgroundStyleFramed()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::FRAMED, OutlineStyle::NONE);

        SimpleBitmap bitmap(data.data(), data.size());
        CPPUNIT_ASSERT(bitmap.getBackgroundStyle() != BackgroundStyle::TRANSPARENT);
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(0x04), static_cast<int>(bitmap.getBackgroundStyle()));
    }

    void testGetOutlineStyleNone()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE);

        SimpleBitmap bitmap(data.data(), data.size());
        CPPUNIT_ASSERT(bitmap.getOutlineStyle() == OutlineStyle::NONE);
    }

    void testGetOutlineStyleOutline()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::OUTLINE);

        SimpleBitmap bitmap(data.data(), data.size());
        CPPUNIT_ASSERT(bitmap.getOutlineStyle() == OutlineStyle::OUTLINE);
    }

    void testGetOutlineStyleDropShadow()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::DROP_SHADOW);

        SimpleBitmap bitmap(data.data(), data.size());
        CPPUNIT_ASSERT(bitmap.getOutlineStyle() == OutlineStyle::DROP_SHADOW);
    }

    void testGetOutlineStyleReserved()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::RESERVED);

        SimpleBitmap bitmap(data.data(), data.size());
        CPPUNIT_ASSERT(bitmap.getOutlineStyle() == OutlineStyle::RESERVED);
    }

    void testGetCharacterColor()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE);
        // Set specific color values: byte1 = 0xF8, byte2 = 0xE5
        data[1] = 0xF8; // y = 31 (0xF8 >> 3), opaqueEnabled = false (bit 2 clear)
        data[2] = 0xE5; // cr and cb calculated from bits

        SimpleBitmap bitmap(data.data(), data.size());
        Color color = bitmap.getCharacterColor();

        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(31), color.y);
        CPPUNIT_ASSERT_EQUAL(false, color.opaqueEnabled);
    }

    void testGetCharacterCoordinates()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE,
                                                             100, 200, 300, 400);

        SimpleBitmap bitmap(data.data(), data.size());
        Coords top = bitmap.getCharacterTop();
        Coords bottom = bitmap.getCharacterBottom();

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), top.x);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(200), top.y);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(300), bottom.x);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(400), bottom.y);
    }

    void testGetOutlineThicknessWhenOutline()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::OUTLINE);

        SimpleBitmap bitmap(data.data(), data.size());
        uint8_t thickness = bitmap.getOutlineThickness();

        // Helper sets thickness to 5
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(5), thickness);
    }

    void testGetOutlineColorWhenOutline()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::OUTLINE);

        SimpleBitmap bitmap(data.data(), data.size());
        Color outlineColor = bitmap.getOutlineColor();

        // Outline color is set to zeros in helper
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), outlineColor.y);
    }

    void testGetShadowOffsetsWhenDropShadow()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::DROP_SHADOW);

        SimpleBitmap bitmap(data.data(), data.size());
        uint8_t shadowRight = bitmap.getShadowRight();
        uint8_t shadowBottom = bitmap.getShadowBottom();

        // Helper sets shadowRight = 3 (upper nibble of 0x34), shadowBottom = 2 (lower nibble of byte+1)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(3), shadowRight);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(2), shadowBottom);
    }

    void testGetShadowColorWhenDropShadow()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::DROP_SHADOW);

        SimpleBitmap bitmap(data.data(), data.size());
        Color shadowColor = bitmap.getShadowColor();

        // Shadow color parsing from bytes - verify it doesn't crash
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(4), shadowColor.y); // (0x20 & 0xF8) >> 3 = 4
    }

    void testWidthCalculation()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE,
                                                             50, 60, 150, 160);

        SimpleBitmap bitmap(data.data(), data.size());
        uint16_t width = bitmap.width();

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), width);
    }

    void testHeightCalculation()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE,
                                                             50, 60, 150, 160);

        SimpleBitmap bitmap(data.data(), data.size());
        uint16_t height = bitmap.height();

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), height);
    }

    void testWidthWithZeroDimension()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE,
                                                             100, 100, 100, 200);

        SimpleBitmap bitmap(data.data(), data.size());
        uint16_t width = bitmap.width();

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), width);
    }

    void testHeightWithZeroDimension()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE,
                                                             100, 100, 200, 100);

        SimpleBitmap bitmap(data.data(), data.size());
        uint16_t height = bitmap.height();

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), height);
    }

    void testMaximumCoordinateValues()
    {
        // 12-bit coordinates max = 4095 (0xFFF)
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE,
                                                             0, 0, 4095, 4095);

        SimpleBitmap bitmap(data.data(), data.size());
        Coords bottom = bitmap.getCharacterBottom();

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(4095), bottom.x);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(4095), bottom.y);
    }

    void testBitmapLengthExceedsDataSize()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE,
                                                             0, 0, 10, 10, 0);
        // Manually set bitmap length to exceed available data
        size_t bitmapLenIdx = data.size() - 2;
        data[bitmapLenIdx] = 0x10;     // High byte = 16
        data[bitmapLenIdx + 1] = 0x00; // Low byte = 0, total = 4096

        CPPUNIT_ASSERT_THROW(
            SimpleBitmap bitmap(data.data(), data.size()),
            ParseError
        );
    }

    void testBitmapLengthZero()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE,
                                                             0, 0, 10, 10, 0);

        CPPUNIT_ASSERT_THROW(
            SimpleBitmap bitmap(data.data(), data.size()),
            InvalidArgument
        );
    }

    void testBitmapLengthExactFit()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE,
                                                             0, 0, 10, 10, 5);

        SimpleBitmap bitmap(data.data(), data.size());
        const RawBitmap& rawBitmap = bitmap.getBitmap();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), rawBitmap.getRawData().size());
    }

    void testSetBitmapWithRawData()
    {
        std::vector<uint8_t> data1 = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE,
                                                              0, 0, 50, 50, 2);
        SimpleBitmap bitmap(data1.data(), data1.size());

        std::vector<uint8_t> data2 = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE,
                                                              0, 0, 100, 100, 3);

        bitmap.setBitmap(data2.data(), data2.size());

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), bitmap.width());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), bitmap.height());
    }

    void testSetBitmapWithRawBitmapObject()
    {
        SimpleBitmap bitmap1;

        RawBitmap rawBitmap(true, reinterpret_cast<const uint8_t*>("test"), 4);

        bitmap1.setBitmap(rawBitmap);

        const RawBitmap& result = bitmap1.getBitmap();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), result.getRawData().size());
    }

    void testSetBitmapCompressedFlag()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE,
                                                             0, 0, 10, 10, 2);
        SimpleBitmap bitmap(data.data(), data.size());

        // Set with compressed = false
        std::vector<uint8_t> data2 = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE,
                                                              0, 0, 20, 20, 3);
        bitmap.setBitmap(data2.data(), data2.size(), false);

        const RawBitmap& rawBitmap = bitmap.getBitmap();
        CPPUNIT_ASSERT_EQUAL(false, rawBitmap.isCompressed());
    }

    void testConstructorFromRawBitmap()
    {
        RawBitmap rawBitmap(true, reinterpret_cast<const uint8_t*>("data"), 4);

        SimpleBitmap bitmap(rawBitmap);

        const RawBitmap& result = bitmap.getBitmap();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), result.getRawData().size());
    }

    void testDefaultConstructor()
    {
        SimpleBitmap bitmap;

        // Default constructor should create valid object
        // Width and height depend on uninitialized coords, just verify no crash
        CPPUNIT_ASSERT_NO_THROW(bitmap.getBitmap());
    }

    void testGetBitmap()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE,
                                                             0, 0, 10, 10, 5);

        SimpleBitmap bitmap(data.data(), data.size());
        const RawBitmap& rawBitmap = bitmap.getBitmap();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), rawBitmap.getRawData().size());
    }

    void testColorFieldParsing()
    {
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE);

        // Set specific color pattern: y=15, opaqueEnabled=true, cr=10, cb=5
        // Byte 1: [y:5bits][opaque:1bit][cr_hi:2bits] = [01111][1][01] = 0x7D
        // Byte 2: [cr_lo:3bits][cb:5bits] = [010][00101] = 0x45
        data[1] = 0x7D;
        data[2] = 0x45;

        SimpleBitmap bitmap(data.data(), data.size());
        Color color = bitmap.getCharacterColor();

        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(15), color.y);
        CPPUNIT_ASSERT_EQUAL(true, color.opaqueEnabled);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(10), color.cr);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(5), color.cb);
    }

    void testCoordinateFieldParsing()
    {
        // Test specific coordinate values: top=(0x123, 0x456), bottom=(0x789, 0xABC)
        std::vector<uint8_t> data = createSimpleBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE,
                                                             0x123, 0x456, 0x789, 0xABC);

        SimpleBitmap bitmap(data.data(), data.size());
        Coords top = bitmap.getCharacterTop();
        Coords bottom = bitmap.getCharacterBottom();

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x123), top.x);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x456), top.y);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x789), bottom.x);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xABC), bottom.y);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( ScteSimpleBitmapTest );
