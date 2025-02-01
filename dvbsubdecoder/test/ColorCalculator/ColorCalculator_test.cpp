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

#include "ColorCalculator.hpp"

using dvbsubdecoder::ColorCalculator;
using dvbsubdecoder::ColorYCrCbT;
using dvbsubdecoder::ColorARGB;

class ColorCalculatorTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ColorCalculatorTest );
    CPPUNIT_TEST(testSimple);CPPUNIT_TEST_SUITE_END()
    ;

public:
    void setUp()
    {
        // noop
    }

    void tearDown()
    {
        // noop
    }

    void testSimple()
    {
        struct TestCase
        {
            ColorYCrCbT m_ycrcbt;
            ColorARGB m_argb;
        };

        TestCase entries[] =
        {
        {
        { 0x00, 0x00, 0x00, 0xFF },
        { 0x00, 0x00, 0x9A, 0x00 } },

        {
        { 0x10, 0x80, 0x80, 0x00 },
        { 0xFF, 0x00, 0x00, 0x00 } },

        {
        { 0x2B, 0x79, 0x82, 0x00 },
        { 0xFF, 0x14, 0x24, 0x23 }, },

        {
        { 0x46, 0x72, 0x85, 0x00 },
        { 0xFF, 0x28, 0x48, 0x48 }, },

        {
        { 0x62, 0x6B, 0x87, 0x00 },
        { 0xFF, 0x3D, 0x6D, 0x6D } },

        {
        { 0x7D, 0x64, 0x89, 0x00 },
        { 0xFF, 0x52, 0x92, 0x91 } },

        {
        { 0x98, 0x5E, 0x8C, 0x00 },
        { 0xFF, 0x67, 0xB5, 0xB6 } },

        {
        { 0xB4, 0x56, 0x8E, 0x00 },
        { 0xFF, 0x7B, 0xDB, 0xDB } },

        {
        { 0xCF, 0x50, 0x90, 0x00 },
        { 0xFF, 0x91, 0xFF, 0xFE }, },

        {
        { 0x10, 0x80, 0x80, 0x11 },
        { 0xEE, 0x00, 0x00, 0x00 } },

        {
        { 0x10, 0x80, 0x80, 0x22 },
        { 0xDD, 0x00, 0x00, 0x00 } },

        {
        { 0x10, 0x80, 0x80, 0x33 },
        { 0xCC, 0x00, 0x00, 0x00 } },

        {
        { 0x10, 0x80, 0x80, 0x3C },
        { 0xC3, 0x00, 0x00, 0x00 } },

        };

        for (auto iter = std::begin(entries); iter != std::end(entries); ++iter)
        {
            auto result = ColorCalculator().toARGB(iter->m_ycrcbt);

            std::cout << "Inp:" << ' ' << static_cast<int>(iter->m_ycrcbt.m_y)
                    << ' ' << static_cast<int>(iter->m_ycrcbt.m_cr) << ' '
                    << static_cast<int>(iter->m_ycrcbt.m_cb) << ' '
                    << static_cast<int>(iter->m_ycrcbt.m_t) << std::endl;

            std::cout << "Exp:" << ' ' << static_cast<int>(iter->m_argb.m_a)
                    << ' ' << static_cast<int>(iter->m_argb.m_r) << ' '
                    << static_cast<int>(iter->m_argb.m_g) << ' '
                    << static_cast<int>(iter->m_argb.m_b) << std::endl;

            std::cout << "Res:" << ' ' << static_cast<int>(result.m_a) << ' '
                    << static_cast<int>(result.m_r) << ' '
                    << static_cast<int>(result.m_g) << ' '
                    << static_cast<int>(result.m_b) << std::endl;

            auto diffA = std::abs(
                    static_cast<int>(iter->m_argb.m_a)
                            - static_cast<int>(result.m_a));
            auto diffR = std::abs(
                    static_cast<int>(iter->m_argb.m_r)
                            - static_cast<int>(result.m_r));
            auto diffG = std::abs(
                    static_cast<int>(iter->m_argb.m_g)
                            - static_cast<int>(result.m_g));
            auto diffB = std::abs(
                    static_cast<int>(iter->m_argb.m_b)
                            - static_cast<int>(result.m_b));

            // allow minimal difference due to roundings

            CPPUNIT_ASSERT(diffA <= 1);
            if (iter->m_argb.m_a > 0)
            {
                // only makes sense for non-fully-transparent colors
                CPPUNIT_ASSERT(diffR <= 1);
                CPPUNIT_ASSERT(diffG <= 1);
                CPPUNIT_ASSERT(diffB <= 1);
            }
        }
    }

};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ColorCalculatorTest);
