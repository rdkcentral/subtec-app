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

#include "ValueConverter.hpp"

using namespace subttxrend::ttmlengine;

class ValueConverterTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(ValueConverterTest);
    CPPUNIT_TEST(testSetSourceSizeWithinBounds);
    CPPUNIT_TEST(testSetSourceSizeAtMax);
    CPPUNIT_TEST(testSetSourceSizeClampWidth);
    CPPUNIT_TEST(testSetSourceSizeClampHeight);
    CPPUNIT_TEST(testSetSourceSizeZeroWidth);
    CPPUNIT_TEST(testSetSourceSizeZeroHeight);
    CPPUNIT_TEST(testSetCellResolution);
    CPPUNIT_TEST(testToTargetRectanglePercentRounding);
    CPPUNIT_TEST(testToTargetRectangleNegativePercent);
    CPPUNIT_TEST(testToTargetRectangleRegion);
    CPPUNIT_TEST(testToTargetRectangleDefaultRegion);
    CPPUNIT_TEST(testToTargetRectanglePixelRegion);
    CPPUNIT_TEST(testToTargetRectangleCellUnits);
    CPPUNIT_TEST(testSizeToPixelsPercent);
    CPPUNIT_TEST(testSizeToPixelsPixel);
    CPPUNIT_TEST(testSizeToPixelsCell);
    CPPUNIT_TEST(testSizeToPixelsClampAboveThreshold);
    CPPUNIT_TEST(testSizeToPixelsAtThreshold);
    CPPUNIT_TEST(testSizeToPixelsNegativePercent);
    CPPUNIT_TEST(testSizeToPixelsInvalidType);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
    }

    void tearDown() override
    {
    }

protected:
    void testSetSourceSizeWithinBounds()
    {
        ValueConverter converter;

        const auto drawingSize = converter.setSourceSize({200, 300});

        assertSize(drawingSize, 200, 300);
        assertRectangle(converter.toTargetRectangle(pixel(10), pixel(20), pixel(30), pixel(40)),
                        10,
                        20,
                        30,
                        40);
    }

    void testSetSourceSizeAtMax()
    {
        ValueConverter converter;

        const auto drawingSize = converter.setSourceSize(ValueConverter::MAX_SURFACE_SIZE);

        assertSize(drawingSize, 1280, 720);
        assertRectangle(converter.toTargetRectangle(pixel(100), pixel(50), pixel(200), pixel(100)),
                        100,
                        50,
                        200,
                        100);
    }

    void testSetSourceSizeClampWidth()
    {
        ValueConverter converter;

        const auto drawingSize = converter.setSourceSize({1920, 700});

        assertSize(drawingSize, 1280, 720);
        assertRectangle(converter.toTargetRectangle(pixel(150), pixel(100), pixel(300), pixel(200)),
                        100,
                        103,
                        200,
                        206);
    }

    void testSetSourceSizeClampHeight()
    {
        ValueConverter converter;

        const auto drawingSize = converter.setSourceSize({1000, 1080});

        assertSize(drawingSize, 1280, 720);
        assertRectangle(converter.toTargetRectangle(pixel(50), pixel(120), pixel(100), pixel(300)),
                        64,
                        80,
                        128,
                        200);
    }

    void testSetSourceSizeZeroWidth()
    {
        ValueConverter converter;

        const auto drawingSize = converter.setSourceSize({0, 300});

        assertSize(drawingSize, 0, 300);
        assertRectangle(converter.toTargetRectangle(pixel(10), pixel(20), pixel(30), pixel(40)),
                        10,
                        20,
                        30,
                        40);
    }

    void testSetSourceSizeZeroHeight()
    {
        ValueConverter converter;

        const auto drawingSize = converter.setSourceSize({200, 0});

        assertSize(drawingSize, 200, 0);
        assertRectangle(converter.toTargetRectangle(pixel(10), pixel(20), pixel(30), pixel(40)),
                        10,
                        20,
                        30,
                        40);
    }

    void testSetCellResolution()
    {
        ValueConverter converter;
        converter.setSourceSize({200, 300});

        converter.setCellResolution({32, 15});

        CPPUNIT_ASSERT_EQUAL(20, converter.getCellHeight());
    }

    void testToTargetRectanglePercentRounding()
    {
        ValueConverter converter;
        converter.setSourceSize({200, 300});

        const auto rect = converter.toTargetRectangle(percent(3333), percent(1667), percent(2500), percent(6667));

        assertRectangle(rect, 67, 50, 50, 200);
    }

    void testToTargetRectangleNegativePercent()
    {
        ValueConverter converter;
        converter.setSourceSize({200, 300});

        const auto rect = converter.toTargetRectangle(percent(-500), percent(-1000), percent(2500), percent(5000));

        assertRectangle(rect, -10, -30, 50, 150);
    }

    void testToTargetRectangleRegion()
    {
        ValueConverter converter;
        converter.setSourceSize({200, 300});
        RegionElement region(1000, 2500, 5000, 2500);

        const auto rect = converter.toTargetRectangle(region);

        assertRectangle(rect, 20, 75, 100, 75);
    }

    void testToTargetRectangleDefaultRegion()
    {
        ValueConverter converter;
        converter.setSourceSize({200, 300});
        RegionElement region;

        const auto rect = converter.toTargetRectangle(region);

        assertRectangle(rect, 0, 0, 200, 300);
    }

    void testToTargetRectanglePixelRegion()
    {
        ValueConverter converter;
        converter.setSourceSize({200, 300});
        RegionElement region;
        region.parseAttribute("tts", "origin", "10px 20px");
        region.parseAttribute("tts", "extent", "30px 40px");

        const auto rect = converter.toTargetRectangle(region);

        assertRectangle(rect, 10, 20, 30, 40);
    }

    void testToTargetRectangleCellUnits()
    {
        ValueConverter converter;
        converter.setSourceSize({200, 300});

        const auto rect = converter.toTargetRectangle(cell(100), cell(100), cell(100), cell(100));

        assertRectangle(rect, 0, 0, 0, 0);
    }

    void testSizeToPixelsPercent()
    {
        ValueConverter converter;
        converter.setSourceSize({200, 300});

        const auto pixelSize = converter.sizeToPixels(percent(2500), 80);

        CPPUNIT_ASSERT_EQUAL(20, pixelSize);
    }

    void testSizeToPixelsPixel()
    {
        ValueConverter converter;
        converter.setSourceSize({1920, 700});

        const auto pixelSize = converter.sizeToPixels(pixel(50), 999);

        CPPUNIT_ASSERT_EQUAL(51, pixelSize);
    }

    void testSizeToPixelsCell()
    {
        ValueConverter converter;
        converter.setSourceSize({200, 300});
        converter.setCellResolution({32, 15});

        const auto pixelSize = converter.sizeToPixels(cell(125), 999);

        CPPUNIT_ASSERT_EQUAL(25, pixelSize);
    }

    void testSizeToPixelsClampAboveThreshold()
    {
        ValueConverter converter;
        converter.setSourceSize({200, 300});

        const auto pixelSize = converter.sizeToPixels(percent(5000), 100);

        CPPUNIT_ASSERT_EQUAL(20, pixelSize);
    }

    void testSizeToPixelsAtThreshold()
    {
        ValueConverter converter;
        converter.setSourceSize({200, 300});

        const auto pixelSize = converter.sizeToPixels(percent(3000), 100);

        CPPUNIT_ASSERT_EQUAL(30, pixelSize);
    }

    void testSizeToPixelsNegativePercent()
    {
        ValueConverter converter;
        converter.setSourceSize({200, 300});

        const auto pixelSize = converter.sizeToPixels(percent(-2500), 80);

        CPPUNIT_ASSERT_EQUAL(-20, pixelSize);
    }

    void testSizeToPixelsInvalidType()
    {
        ValueConverter converter;
        converter.setSourceSize({1280, 720});

        const auto pixelSize = converter.sizeToPixels(DomainValue(static_cast<DomainValue::Type>(999), 1234), 80);

        CPPUNIT_ASSERT_EQUAL(32, pixelSize);
    }

    DomainValue percent(int value) const
    {
        return DomainValue(DomainValue::Type::PERCENTAGE_HUNDREDTHS, value);
    }

    DomainValue pixel(int value) const
    {
        return DomainValue(DomainValue::Type::PIXEL, value);
    }

    DomainValue cell(int value) const
    {
        return DomainValue(DomainValue::Type::CELL_HUNDREDTHS, value);
    }

    void assertSize(const subttxrend::gfx::Size& size,
                    int width,
                    int height) const
    {
        CPPUNIT_ASSERT_EQUAL(width, size.m_w);
        CPPUNIT_ASSERT_EQUAL(height, size.m_h);
    }

    void assertRectangle(const subttxrend::gfx::Rectangle& rectangle,
                         int x,
                         int y,
                         int width,
                         int height) const
    {
        CPPUNIT_ASSERT_EQUAL(x, rectangle.m_x);
        CPPUNIT_ASSERT_EQUAL(y, rectangle.m_y);
        CPPUNIT_ASSERT_EQUAL(width, rectangle.m_w);
        CPPUNIT_ASSERT_EQUAL(height, rectangle.m_h);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ValueConverterTest);