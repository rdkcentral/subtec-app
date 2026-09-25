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
#include "ValueConverter.hpp"

using namespace subttxrend::ttmlengine;

class ValueConverterTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ValueConverterTest );
    CPPUNIT_TEST(testPixelSizeClampedToMax);
    CPPUNIT_TEST(testPixelSizeBelowMaxUnclamped);
    CPPUNIT_TEST(testPixelSizeAtMaxUnclamped);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // noop
    }

    void tearDown()
    {
        // noop
    }

    // With drawingSize.m_h == 320 and DEFAULT_CELL_RESOLUTION.m_h == 15,
    // PixelDefault == 320 / 15 == 21, so the max allowed size is
    // lround(21 * 1.5) == 32.
    void testPixelSizeClampedToMax()
    {
        ValueConverter converter;
        gfx::Size size{640, 320};
        converter.setSourceSize(size);

        // Requesting a pixel size well above the max must be trimmed to the max,
        // not to PixelDefault as the pre-fix code incorrectly did.
        int result = converter.sizeToPixels(DomainValue{DomainValue::Type::PIXEL, 40}, 0);

        CPPUNIT_ASSERT_EQUAL(32, result);
    }

    void testPixelSizeBelowMaxUnclamped()
    {
        ValueConverter converter;
        gfx::Size size{640, 320};
        converter.setSourceSize(size);

        int result = converter.sizeToPixels(DomainValue{DomainValue::Type::PIXEL, 25}, 0);

        CPPUNIT_ASSERT_EQUAL(25, result);
    }

    void testPixelSizeAtMaxUnclamped()
    {
        ValueConverter converter;
        gfx::Size size{640, 320};
        converter.setSourceSize(size);

        int result = converter.sizeToPixels(DomainValue{DomainValue::Type::PIXEL, 32}, 0);

        CPPUNIT_ASSERT_EQUAL(32, result);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( ValueConverterTest );
