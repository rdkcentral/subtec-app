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
