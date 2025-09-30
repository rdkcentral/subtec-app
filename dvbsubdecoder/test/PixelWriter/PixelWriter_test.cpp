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

#include "PixelWriter.hpp"

#include <array>

using dvbsubdecoder::PixelWriter;
using dvbsubdecoder::Pixmap;

class PixelWriterTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PixelWriterTest );
    CPPUNIT_TEST(testInit);
    CPPUNIT_TEST(testBadInit);
    CPPUNIT_TEST(testBadSetPixels);
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testNonModifyingColor);
    CPPUNIT_TEST(testBounds);
    CPPUNIT_TEST(testStartPos);
    CPPUNIT_TEST(testGetDepthAllValues);
    CPPUNIT_TEST(testConstructorAllValidDepths);
    CPPUNIT_TEST(testConstructorInvalidDepthBoundaries);
    CPPUNIT_TEST(testConstructorBoundaryCoordinates);
    CPPUNIT_TEST(testConstructorOutOfBoundsCoordinates);
    CPPUNIT_TEST(testSetPixelsZeroCount);
    CPPUNIT_TEST(testSetPixelsMaxValidCodes);
    CPPUNIT_TEST(testNonModifyingColorEdgeCases);
    CPPUNIT_TEST(testSetPixelsLineBoundary);
    CPPUNIT_TEST(testSetPixelsLargeCount);
    CPPUNIT_TEST(testMultipleEndLineCalls);
    CPPUNIT_TEST(testEndLineAtBottomBounds);
    CPPUNIT_TEST(testMultipleLinesDifferentStartX);
    CPPUNIT_TEST(testDegeneratePixmapDimensions);
    CPPUNIT_TEST(testOddStartYPosition);
    CPPUNIT_TEST(testComplexOperationSequence);
    CPPUNIT_TEST(testExceptionSafety);
    CPPUNIT_TEST(testPixmapBoundaryClipping);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        m_pixmap.init(WIDTH, HEIGHT, m_pixmapBuffer.data());
        m_pixmap.clear(0);
    }

    void tearDown()
    {
        m_pixmap.reset();
    }

    void testInit()
    {
        PixelWriter testWriter(false, 2, m_pixmap, 0, 0);

        CPPUNIT_ASSERT(testWriter.getDepth() == 2);
    }

    void testBadInit()
    {
        CPPUNIT_ASSERT_THROW(
                std::make_shared<PixelWriter>(false, 2, m_pixmap, -1, 0),
                std::logic_error);
        CPPUNIT_ASSERT_THROW(
                std::make_shared<PixelWriter>(false, 2, m_pixmap, 0, -1),
                std::logic_error);
        CPPUNIT_ASSERT_THROW(
                std::make_shared<PixelWriter>(false, 2, m_pixmap, -1, -1),
                std::logic_error);
        CPPUNIT_ASSERT_THROW(
                std::make_shared<PixelWriter>(false, 0, m_pixmap, 0, 0),
                std::logic_error);
        CPPUNIT_ASSERT_THROW(
                std::make_shared<PixelWriter>(false, 10, m_pixmap, 0, 0),
                std::logic_error);
    }

    void testBadSetPixels()
    {
        {
            PixelWriter testWriter2(false, 2, m_pixmap, 0, 0);

            CPPUNIT_ASSERT_NO_THROW(testWriter2.setPixels(1, 1));
            CPPUNIT_ASSERT_THROW(testWriter2.setPixels(4, 1), std::logic_error);
            CPPUNIT_ASSERT_THROW(testWriter2.setPixels(255, 1),
                    std::logic_error);
        }

        {
            PixelWriter testWriter4(false, 4, m_pixmap, 0, 0);

            CPPUNIT_ASSERT_NO_THROW(testWriter4.setPixels(1, 1));
            CPPUNIT_ASSERT_THROW(testWriter4.setPixels(16, 1),
                    std::logic_error);
            CPPUNIT_ASSERT_THROW(testWriter4.setPixels(255, 1),
                    std::logic_error);
        }

        {
            PixelWriter testWriter8(false, 8, m_pixmap, 0, 0);

            CPPUNIT_ASSERT_NO_THROW(testWriter8.setPixels(1, 1));
            CPPUNIT_ASSERT_NO_THROW(testWriter8.setPixels(255, 1));
        }
    }

    void testSimple()
    {
        PixelWriter testWriter(false, 4, m_pixmap, 0, 0);

        testWriter.setPixels(1, 1);
        testWriter.setPixels(2, 2);
        testWriter.setPixels(3, 3);
        testWriter.setPixels(4, 4);
        testWriter.endLine();
        testWriter.setPixels(11, 1);
        testWriter.setPixels(12, 2);
        testWriter.setPixels(13, 3);
        testWriter.setPixels(14, 4);
        testWriter.endLine();

        CPPUNIT_ASSERT(m_pixmap.getLine(0)[0] == 1);
        CPPUNIT_ASSERT(m_pixmap.getLine(0)[1] == 2);
        CPPUNIT_ASSERT(m_pixmap.getLine(0)[2] == 2);
        CPPUNIT_ASSERT(m_pixmap.getLine(0)[3] == 3);
        CPPUNIT_ASSERT(m_pixmap.getLine(0)[4] == 3);
        CPPUNIT_ASSERT(m_pixmap.getLine(0)[5] == 3);
        CPPUNIT_ASSERT(m_pixmap.getLine(0)[6] == 4);
        CPPUNIT_ASSERT(m_pixmap.getLine(0)[7] == 4);
        CPPUNIT_ASSERT(m_pixmap.getLine(0)[8] == 4);
        CPPUNIT_ASSERT(m_pixmap.getLine(0)[9] == 4);
        CPPUNIT_ASSERT(m_pixmap.getLine(0)[10] == 0);

        CPPUNIT_ASSERT(m_pixmap.getLine(2)[0] == 11);
        CPPUNIT_ASSERT(m_pixmap.getLine(2)[1] == 12);
        CPPUNIT_ASSERT(m_pixmap.getLine(2)[2] == 12);
        CPPUNIT_ASSERT(m_pixmap.getLine(2)[3] == 13);
        CPPUNIT_ASSERT(m_pixmap.getLine(2)[4] == 13);
        CPPUNIT_ASSERT(m_pixmap.getLine(2)[5] == 13);
        CPPUNIT_ASSERT(m_pixmap.getLine(2)[6] == 14);
        CPPUNIT_ASSERT(m_pixmap.getLine(2)[7] == 14);
        CPPUNIT_ASSERT(m_pixmap.getLine(2)[8] == 14);
        CPPUNIT_ASSERT(m_pixmap.getLine(2)[9] == 14);
        CPPUNIT_ASSERT(m_pixmap.getLine(2)[10] == 0);

        CPPUNIT_ASSERT(m_pixmap.getLine(4)[0] == 0);
    }

    void testNonModifyingColor()
    {
        {
            m_pixmap.clear(0xFE);

            PixelWriter testWriterSimple(true, 8, m_pixmap, 0, 0);

            testWriterSimple.setPixels(1, 1);
            CPPUNIT_ASSERT(m_pixmap.getLine(0)[0] == 0xFE);
        }

        {
            m_pixmap.clear(0);

            PixelWriter testWriter(true, 8, m_pixmap, 0, 0);

            for (auto i = 0; i < 10; ++i)
            {
                testWriter.setPixels(1, i);
                testWriter.setPixels(10 + i, 10 - i);
                testWriter.endLine();
            }

            for (auto i = 0; i < 10; ++i)
            {
                auto line = m_pixmap.getLine(i * 2);

                for (auto j = 0; j < i; ++j)
                {
                    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(line[j]));
                }

                for (auto j = i; j < 10; ++j)
                {
                    CPPUNIT_ASSERT_EQUAL(10 + i, static_cast<int>(line[j]));
                }

                for (auto j = 10; j < WIDTH; ++j)
                {
                    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(line[j]));
                }
            }

            auto line = m_pixmap.getLine(10 * 2);
            for (auto i = 0; i < WIDTH; ++i)
            {
                CPPUNIT_ASSERT(line[i] == 0);
            }
        }
    }

    void testBounds()
    {
        static const std::int32_t SMALL_WIDTH = 20;
        static const std::int32_t SMALL_HEIGHT = 10;
        const std::int32_t SMALL_PIXMAP_SIZE = SMALL_WIDTH * SMALL_HEIGHT;

        Pixmap smallPixmap;
        smallPixmap.init(SMALL_WIDTH, SMALL_HEIGHT, m_pixmapBuffer.data());
        smallPixmap.clear(0xFF);

        {
            PixelWriter testWriter(false, 8, smallPixmap, 0, 0);
            for (auto i = 0; i < SMALL_HEIGHT * 2; ++i)
            {
                testWriter.setPixels(0x55, SMALL_WIDTH * 2);
                testWriter.setPixels(0x55, SMALL_WIDTH * 2);
                testWriter.endLine();
            }

            for (auto line = 0; line < SMALL_HEIGHT; ++line)
            {
                if ((line % 2) == 0)
                {
                    for (auto i = 0; i < SMALL_HEIGHT; ++i)
                    {
                        CPPUNIT_ASSERT_EQUAL(0x55,
                                static_cast<int>(m_pixmapBuffer[line
                                        * SMALL_WIDTH + i]));
                    }
                }
                else
                {
                    for (auto i = 0; i < SMALL_HEIGHT; ++i)
                    {
                        CPPUNIT_ASSERT_EQUAL(0xFF,
                                static_cast<int>(m_pixmapBuffer[line
                                        * SMALL_WIDTH + i]));
                    }
                }
            }
            for (auto i = SMALL_PIXMAP_SIZE; i < PIXMAP_SIZE; ++i)
            {
                CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(m_pixmapBuffer[i]));
            }
        }

        smallPixmap.clear(0xFE);
        PixelWriter nmTestWriter(true, 8, smallPixmap, 0, 0);

        {
            nmTestWriter.setPixels(1, SMALL_WIDTH * 3);
            for (auto i = 0; i < SMALL_PIXMAP_SIZE; ++i)
            {
                CPPUNIT_ASSERT_EQUAL(0xFE, static_cast<int>(m_pixmapBuffer[i]));
            }
            for (auto i = SMALL_PIXMAP_SIZE; i < PIXMAP_SIZE; ++i)
            {
                CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(m_pixmapBuffer[i]));
            }
        }
    }

    void testStartPos()
    {
        PixelWriter writer1(false, 4, m_pixmap, 2 + 0, 2 + 0);
        PixelWriter writer2(false, 4, m_pixmap, 2 + 1, 2 + 1);

        for (auto i = 0; i < 10; ++i)
        {
            writer1.setPixels(2, 10);
            writer1.endLine();
            writer2.setPixels(3, 10);
            writer2.endLine();
        }

        for (auto y = 0; y < HEIGHT; ++y)
        {
            if ((y >= 2) && (y < 2 + 2 * 10))
            {
                if ((y % 2) == 0)
                {
                    for (auto x = 0; x < WIDTH; ++x)
                    {
                        if ((x >= 2) && (x < 2 + 10))
                        {
                            CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(m_pixmap.getLine(y)[x]));
                        }
                        else
                        {
                            CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(m_pixmap.getLine(y)[x]));
                        }
                    }
                }
                else
                {
                    for (auto x = 0; x < WIDTH; ++x)
                    {
                        if ((x >= 3) && (x < 3 + 10))
                        {
                            CPPUNIT_ASSERT_EQUAL(3, static_cast<int>(m_pixmap.getLine(y)[x]));
                        }
                        else
                        {
                            CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(m_pixmap.getLine(y)[x]));
                        }
                    }
                }
            }
            else
            {
                for (auto x = 0; x < WIDTH; ++x)
                {
                    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(m_pixmap.getLine(y)[x]));
                }
            }
        }
    }

    void testGetDepthAllValues()
    {
        // Test getDepth() with all valid depth values
        PixelWriter writer1(false, 1, m_pixmap, 0, 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(1), writer1.getDepth());
        
        PixelWriter writer2(false, 2, m_pixmap, 0, 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(2), writer2.getDepth());
        
        PixelWriter writer3(false, 3, m_pixmap, 0, 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(3), writer3.getDepth());
        
        PixelWriter writer4(false, 4, m_pixmap, 0, 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(4), writer4.getDepth());
        
        PixelWriter writer5(false, 5, m_pixmap, 0, 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(5), writer5.getDepth());
        
        PixelWriter writer6(false, 6, m_pixmap, 0, 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(6), writer6.getDepth());
        
        PixelWriter writer7(false, 7, m_pixmap, 0, 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(7), writer7.getDepth());
        
        PixelWriter writer8(false, 8, m_pixmap, 0, 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(8), writer8.getDepth());
    }

    void testConstructorAllValidDepths()
    {
        // Test constructor accepts all valid depth values without throwing
        CPPUNIT_ASSERT_NO_THROW(PixelWriter(false, 1, m_pixmap, 0, 0));
        CPPUNIT_ASSERT_NO_THROW(PixelWriter(false, 2, m_pixmap, 0, 0));
        CPPUNIT_ASSERT_NO_THROW(PixelWriter(false, 3, m_pixmap, 0, 0));
        CPPUNIT_ASSERT_NO_THROW(PixelWriter(false, 4, m_pixmap, 0, 0));
        CPPUNIT_ASSERT_NO_THROW(PixelWriter(false, 5, m_pixmap, 0, 0));
        CPPUNIT_ASSERT_NO_THROW(PixelWriter(false, 6, m_pixmap, 0, 0));
        CPPUNIT_ASSERT_NO_THROW(PixelWriter(false, 7, m_pixmap, 0, 0));
        CPPUNIT_ASSERT_NO_THROW(PixelWriter(false, 8, m_pixmap, 0, 0));
    }

    void testConstructorInvalidDepthBoundaries()
    {
        // Test boundary values for invalid depth
        CPPUNIT_ASSERT_THROW(PixelWriter(false, 9, m_pixmap, 0, 0), std::invalid_argument);
        CPPUNIT_ASSERT_THROW(PixelWriter(false, 255, m_pixmap, 0, 0), std::invalid_argument);
    }

    void testConstructorBoundaryCoordinates()
    {
        // Test coordinates at pixmap boundaries
        CPPUNIT_ASSERT_NO_THROW(PixelWriter(false, 4, m_pixmap, WIDTH, 0));
        CPPUNIT_ASSERT_NO_THROW(PixelWriter(false, 4, m_pixmap, 0, HEIGHT));
        CPPUNIT_ASSERT_NO_THROW(PixelWriter(false, 4, m_pixmap, WIDTH, HEIGHT));
    }

    void testConstructorOutOfBoundsCoordinates()
    {
        // Test coordinates beyond pixmap boundaries
        CPPUNIT_ASSERT_NO_THROW(PixelWriter(false, 4, m_pixmap, WIDTH + 10, 0));
        CPPUNIT_ASSERT_NO_THROW(PixelWriter(false, 4, m_pixmap, 0, HEIGHT + 10));
        CPPUNIT_ASSERT_NO_THROW(PixelWriter(false, 4, m_pixmap, WIDTH + 10, HEIGHT + 10));
        
        // Verify that writing is no-op when out of bounds
        PixelWriter outOfBoundsWriter(false, 4, m_pixmap, WIDTH + 10, HEIGHT + 10);
        m_pixmap.clear(0xAA);
        outOfBoundsWriter.setPixels(0x55, 100);
        
        // Verify pixmap unchanged
        for (auto y = 0; y < HEIGHT; ++y)
        {
            for (auto x = 0; x < WIDTH; ++x)
            {
                CPPUNIT_ASSERT_EQUAL(0xAA, static_cast<int>(m_pixmap.getLine(y)[x]));
            }
        }
    }

    void testSetPixelsZeroCount()
    {
        PixelWriter writer(false, 4, m_pixmap, 0, 0);
        m_pixmap.clear(0x42);
        
        // setPixels with count=0 should be no-op
        // Use a valid pixel code for depth=4 (max 0x0F) to avoid validation exception
        CPPUNIT_ASSERT_NO_THROW(writer.setPixels(0x0F, 0));
        
        // Verify pixmap unchanged
        CPPUNIT_ASSERT_EQUAL(0x42, static_cast<int>(m_pixmap.getLine(0)[0]));
        CPPUNIT_ASSERT_EQUAL(0x42, static_cast<int>(m_pixmap.getLine(0)[1]));
    }

    void testSetPixelsMaxValidCodes()
    {
        // Test maximum valid pixel codes for each depth
        {
            PixelWriter writer1(false, 1, m_pixmap, 0, 0);
            CPPUNIT_ASSERT_NO_THROW(writer1.setPixels(1, 1)); // 2^1 - 1 = 1
        }
        
        {
            PixelWriter writer2(false, 2, m_pixmap, 0, 0);
            CPPUNIT_ASSERT_NO_THROW(writer2.setPixels(3, 1)); // 2^2 - 1 = 3
        }
        
        {
            PixelWriter writer3(false, 3, m_pixmap, 0, 0);
            CPPUNIT_ASSERT_NO_THROW(writer3.setPixels(7, 1)); // 2^3 - 1 = 7
        }
        
        {
            PixelWriter writer4(false, 4, m_pixmap, 0, 0);
            CPPUNIT_ASSERT_NO_THROW(writer4.setPixels(15, 1)); // 2^4 - 1 = 15
        }
        
        {
            PixelWriter writer5(false, 5, m_pixmap, 0, 0);
            CPPUNIT_ASSERT_NO_THROW(writer5.setPixels(31, 1)); // 2^5 - 1 = 31
        }
        
        {
            PixelWriter writer6(false, 6, m_pixmap, 0, 0);
            CPPUNIT_ASSERT_NO_THROW(writer6.setPixels(63, 1)); // 2^6 - 1 = 63
        }
        
        {
            PixelWriter writer7(false, 7, m_pixmap, 0, 0);
            CPPUNIT_ASSERT_NO_THROW(writer7.setPixels(127, 1)); // 2^7 - 1 = 127
        }
        
        {
            PixelWriter writer8(false, 8, m_pixmap, 0, 0);
            CPPUNIT_ASSERT_NO_THROW(writer8.setPixels(255, 1)); // 2^8 - 1 = 255
        }
    }

    void testNonModifyingColorEdgeCases()
    {
        m_pixmap.clear(0xCC);
        
        // Test that pixelCode=0 writes normally even with non-modifying flag
        {
            PixelWriter writer(true, 8, m_pixmap, 0, 0);
            writer.setPixels(0, 3);
            
            CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(m_pixmap.getLine(0)[0]));
            CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(m_pixmap.getLine(0)[1]));
            CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(m_pixmap.getLine(0)[2]));
        }
        
        m_pixmap.clear(0xDD);
        
        // Test that pixelCode>1 writes normally even with non-modifying flag
        {
            PixelWriter writer(true, 8, m_pixmap, 0, 0);
            writer.setPixels(42, 3);
            
            CPPUNIT_ASSERT_EQUAL(42, static_cast<int>(m_pixmap.getLine(0)[0]));
            CPPUNIT_ASSERT_EQUAL(42, static_cast<int>(m_pixmap.getLine(0)[1]));
            CPPUNIT_ASSERT_EQUAL(42, static_cast<int>(m_pixmap.getLine(0)[2]));
        }
        
        m_pixmap.clear(0xEE);
        
        // Test that pixelCode=1 is non-modifying with flag=true
        {
            PixelWriter writer(true, 8, m_pixmap, 0, 0);
            writer.setPixels(1, 3);
            
            CPPUNIT_ASSERT_EQUAL(0xEE, static_cast<int>(m_pixmap.getLine(0)[0]));
            CPPUNIT_ASSERT_EQUAL(0xEE, static_cast<int>(m_pixmap.getLine(0)[1]));
            CPPUNIT_ASSERT_EQUAL(0xEE, static_cast<int>(m_pixmap.getLine(0)[2]));
        }
    }

    void testSetPixelsLineBoundary()
    {
        PixelWriter writer(false, 8, m_pixmap, WIDTH - 5, 0);
        m_pixmap.clear(0);
        
        // Write exactly to line boundary
        writer.setPixels(0x77, 5);
        
        // Verify pixels written up to boundary
        for (auto i = 0; i < 5; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(0x77, static_cast<int>(m_pixmap.getLine(0)[WIDTH - 5 + i]));
        }
        
        // Verify no overflow to next line
        CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(m_pixmap.getLine(1)[0]));
    }

    void testSetPixelsLargeCount()
    {
        PixelWriter writer(false, 8, m_pixmap, WIDTH - 3, 0);
        m_pixmap.clear(0);
        
        // Write with count larger than remaining line space
        writer.setPixels(0x88, 10);
        
        // Verify only 3 pixels written (remaining line space)
        CPPUNIT_ASSERT_EQUAL(0x88, static_cast<int>(m_pixmap.getLine(0)[WIDTH - 3]));
        CPPUNIT_ASSERT_EQUAL(0x88, static_cast<int>(m_pixmap.getLine(0)[WIDTH - 2]));
        CPPUNIT_ASSERT_EQUAL(0x88, static_cast<int>(m_pixmap.getLine(0)[WIDTH - 1]));
        
        // Verify no overflow
        CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(m_pixmap.getLine(1)[0]));
    }

    void testMultipleEndLineCalls()
    {
        PixelWriter writer(false, 8, m_pixmap, 5, 4);
        m_pixmap.clear(0);
        
        // Test multiple endLine calls advance Y correctly
        writer.setPixels(1, 1);
        CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(m_pixmap.getLine(4)[5]));
        
        writer.endLine();
        writer.setPixels(2, 1);
        CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(m_pixmap.getLine(6)[5]));
        
        writer.endLine();
        writer.setPixels(3, 1);
        CPPUNIT_ASSERT_EQUAL(3, static_cast<int>(m_pixmap.getLine(8)[5]));
        
        writer.endLine();
        writer.setPixels(4, 1);
        CPPUNIT_ASSERT_EQUAL(4, static_cast<int>(m_pixmap.getLine(10)[5]));
    }

    void testEndLineAtBottomBounds()
    {
        PixelWriter writer(false, 8, m_pixmap, 0, HEIGHT - 2);
        m_pixmap.clear(0xBB);
        
        // Write at bottom boundary
        writer.setPixels(0x11, 3);
        CPPUNIT_ASSERT_EQUAL(0x11, static_cast<int>(m_pixmap.getLine(HEIGHT - 2)[0]));
        
        // End line should go out of bounds
        writer.endLine();
        writer.setPixels(0x22, 3);
        
        // Verify original pixmap unchanged (writes should be no-op)
        for (auto x = 0; x < 3; ++x)
        {
            for (auto y = 0; y < HEIGHT; ++y)
            {
                if (y == HEIGHT - 2 && x < 3)
                {
                    CPPUNIT_ASSERT_EQUAL(0x11, static_cast<int>(m_pixmap.getLine(y)[x]));
                }
                else
                {
                    CPPUNIT_ASSERT_EQUAL(0xBB, static_cast<int>(m_pixmap.getLine(y)[x]));
                }
            }
        }
    }

    void testMultipleLinesDifferentStartX()
    {
        PixelWriter writer1(false, 8, m_pixmap, 10, 0);
        PixelWriter writer2(false, 8, m_pixmap, 20, 0);
        m_pixmap.clear(0);
        
        // Write with different X positions
        writer1.setPixels(0x33, 5);
        writer2.setPixels(0x44, 5);
        
        // Verify correct positioning
        for (auto i = 0; i < 5; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(0x33, static_cast<int>(m_pixmap.getLine(0)[10 + i]));
            CPPUNIT_ASSERT_EQUAL(0x44, static_cast<int>(m_pixmap.getLine(0)[20 + i]));
        }
        
        writer1.endLine();
        writer2.endLine();
        
        writer1.setPixels(0x55, 5);
        writer2.setPixels(0x66, 5);
        
        // Verify second line positioning
        for (auto i = 0; i < 5; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(0x55, static_cast<int>(m_pixmap.getLine(2)[10 + i]));
            CPPUNIT_ASSERT_EQUAL(0x66, static_cast<int>(m_pixmap.getLine(2)[20 + i]));
        }
    }

    void testDegeneratePixmapDimensions()
    {
        std::array<std::uint8_t, 100> smallBuffer;
        Pixmap zeroWidthPixmap;
        
        // Test zero width pixmap
        zeroWidthPixmap.init(0, 10, smallBuffer.data());
        CPPUNIT_ASSERT_NO_THROW(PixelWriter(false, 8, zeroWidthPixmap, 0, 0));
        
        PixelWriter writerZeroWidth(false, 8, zeroWidthPixmap, 0, 0);
        std::fill(smallBuffer.begin(), smallBuffer.end(), 0xAA);
        writerZeroWidth.setPixels(0x55, 10);
        
        // Verify buffer unchanged (all writes should be no-op)
        for (auto val : smallBuffer)
        {
            CPPUNIT_ASSERT_EQUAL(0xAA, static_cast<int>(val));
        }
        
        // Test zero height pixmap
        Pixmap zeroHeightPixmap;
        zeroHeightPixmap.init(10, 0, smallBuffer.data());
        CPPUNIT_ASSERT_NO_THROW(PixelWriter(false, 8, zeroHeightPixmap, 0, 0));
        
        PixelWriter writerZeroHeight(false, 8, zeroHeightPixmap, 0, 0);
        std::fill(smallBuffer.begin(), smallBuffer.end(), 0xBB);
        writerZeroHeight.setPixels(0x77, 10);
        
        // Verify buffer unchanged
        for (auto val : smallBuffer)
        {
            CPPUNIT_ASSERT_EQUAL(0xBB, static_cast<int>(val));
        }
    }

    void testOddStartYPosition()
    {
        PixelWriter writer(false, 8, m_pixmap, 5, 3); // Start at odd Y
        m_pixmap.clear(0);
        
        writer.setPixels(0x99, 3);
        CPPUNIT_ASSERT_EQUAL(0x99, static_cast<int>(m_pixmap.getLine(3)[5]));
        CPPUNIT_ASSERT_EQUAL(0x99, static_cast<int>(m_pixmap.getLine(3)[6]));
        CPPUNIT_ASSERT_EQUAL(0x99, static_cast<int>(m_pixmap.getLine(3)[7]));
        
        writer.endLine();
        writer.setPixels(0xAA, 3);
        
        // Should advance to Y=5 (3+2)
        CPPUNIT_ASSERT_EQUAL(0xAA, static_cast<int>(m_pixmap.getLine(5)[5]));
        CPPUNIT_ASSERT_EQUAL(0xAA, static_cast<int>(m_pixmap.getLine(5)[6]));
        CPPUNIT_ASSERT_EQUAL(0xAA, static_cast<int>(m_pixmap.getLine(5)[7]));
    }

    void testComplexOperationSequence()
    {
        PixelWriter writer(false, 4, m_pixmap, 10, 6);
        m_pixmap.clear(0);
        
        // Complex sequence: setPixels, partial line, setPixels, endLine, repeat
        writer.setPixels(1, 3);
        writer.setPixels(2, 2);
        writer.setPixels(3, 4);
        writer.endLine();
        
        writer.setPixels(4, 1);
        writer.setPixels(5, 5);
        writer.endLine();
        
        writer.setPixels(6, 2);
        
        // Verify first line
        auto line1 = m_pixmap.getLine(6);
        CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(line1[10]));
        CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(line1[11]));
        CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(line1[12]));
        CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(line1[13]));
        CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(line1[14]));
        CPPUNIT_ASSERT_EQUAL(3, static_cast<int>(line1[15]));
        CPPUNIT_ASSERT_EQUAL(3, static_cast<int>(line1[16]));
        CPPUNIT_ASSERT_EQUAL(3, static_cast<int>(line1[17]));
        CPPUNIT_ASSERT_EQUAL(3, static_cast<int>(line1[18]));
        
        // Verify second line
        auto line2 = m_pixmap.getLine(8);
        CPPUNIT_ASSERT_EQUAL(4, static_cast<int>(line2[10]));
        CPPUNIT_ASSERT_EQUAL(5, static_cast<int>(line2[11]));
        CPPUNIT_ASSERT_EQUAL(5, static_cast<int>(line2[12]));
        CPPUNIT_ASSERT_EQUAL(5, static_cast<int>(line2[13]));
        CPPUNIT_ASSERT_EQUAL(5, static_cast<int>(line2[14]));
        CPPUNIT_ASSERT_EQUAL(5, static_cast<int>(line2[15]));
        
        // Verify third line
        auto line3 = m_pixmap.getLine(10);
        CPPUNIT_ASSERT_EQUAL(6, static_cast<int>(line3[10]));
        CPPUNIT_ASSERT_EQUAL(6, static_cast<int>(line3[11]));
    }

    void testExceptionSafety()
    {
        PixelWriter writer(false, 4, m_pixmap, 0, 0);
        m_pixmap.clear(0);
        
        // Write some valid data
        writer.setPixels(5, 3);
        CPPUNIT_ASSERT_EQUAL(5, static_cast<int>(m_pixmap.getLine(0)[0]));
        
        // Cause exception with invalid pixel code
        CPPUNIT_ASSERT_THROW(writer.setPixels(16, 1), std::invalid_argument);
        
        // Verify object remains in valid state - can continue operations
        CPPUNIT_ASSERT_NO_THROW(writer.setPixels(7, 2));
        CPPUNIT_ASSERT_EQUAL(7, static_cast<int>(m_pixmap.getLine(0)[3]));
        CPPUNIT_ASSERT_EQUAL(7, static_cast<int>(m_pixmap.getLine(0)[4]));
        
        // Verify endLine still works
        CPPUNIT_ASSERT_NO_THROW(writer.endLine());
        CPPUNIT_ASSERT_NO_THROW(writer.setPixels(8, 1));
        CPPUNIT_ASSERT_EQUAL(8, static_cast<int>(m_pixmap.getLine(2)[0]));
    }

    void testPixmapBoundaryClipping()
    {
        // Test writing that spans all four pixmap boundaries
        
        // Right boundary clipping
        {
            PixelWriter writer(false, 8, m_pixmap, WIDTH - 3, 0);
            m_pixmap.clear(0);
            writer.setPixels(0x11, 10); // Exceeds right boundary
            
            CPPUNIT_ASSERT_EQUAL(0x11, static_cast<int>(m_pixmap.getLine(0)[WIDTH - 3]));
            CPPUNIT_ASSERT_EQUAL(0x11, static_cast<int>(m_pixmap.getLine(0)[WIDTH - 2]));
            CPPUNIT_ASSERT_EQUAL(0x11, static_cast<int>(m_pixmap.getLine(0)[WIDTH - 1]));
            
            // Verify no overflow to next line
            CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(m_pixmap.getLine(1)[0]));
        }
        
        // Bottom boundary clipping
        {
            PixelWriter writer(false, 8, m_pixmap, 0, HEIGHT - 1);
            m_pixmap.clear(0);
            
            writer.setPixels(0x22, 5);
            CPPUNIT_ASSERT_EQUAL(0x22, static_cast<int>(m_pixmap.getLine(HEIGHT - 1)[0]));
            
            writer.endLine(); // Should go out of bounds
            writer.setPixels(0x33, 5); // Should be no-op
            
            // Verify no writes beyond bottom
            for (auto x = 0; x < 5; ++x)
            {
                for (auto y = 0; y < HEIGHT; ++y)
                {
                    if (y == HEIGHT - 1 && x < 5)
                    {
                        CPPUNIT_ASSERT_EQUAL(0x22, static_cast<int>(m_pixmap.getLine(y)[x]));
                    }
                    else
                    {
                        CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(m_pixmap.getLine(y)[x]));
                    }
                }
            }
        }
        
        // Left boundary (startX beyond left edge - handled by constructor validation)
        // Top boundary (startY beyond top edge - handled by constructor validation)
        
        // Corner case: bottom-right corner
        {
            PixelWriter writer(false, 8, m_pixmap, WIDTH - 1, HEIGHT - 1);
            m_pixmap.clear(0);
            
            writer.setPixels(0x44, 5); // Should write only 1 pixel
            CPPUNIT_ASSERT_EQUAL(0x44, static_cast<int>(m_pixmap.getLine(HEIGHT - 1)[WIDTH - 1]));
            
            // Verify no overflow
            for (auto y = 0; y < HEIGHT - 1; ++y)
            {
                for (auto x = 0; x < WIDTH; ++x)
                {
                    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(m_pixmap.getLine(y)[x]));
                }
            }
            for (auto x = 0; x < WIDTH - 1; ++x)
            {
                CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(m_pixmap.getLine(HEIGHT - 1)[x]));
            }
        }
    }

private:
    static const std::int32_t WIDTH = 100;
    static const std::int32_t HEIGHT = 50;
    static const std::int32_t PIXMAP_SIZE = WIDTH * HEIGHT;

    std::array<std::uint8_t, PIXMAP_SIZE> m_pixmapBuffer;
    Pixmap m_pixmap;
}
;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PixelWriterTest);
