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
#include "Parser/AttributeHandlers.hpp"

using namespace subttxrend::ttmlengine;

class AttributeHandlersTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( AttributeHandlersTest );
    CPPUNIT_TEST(merging);
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

    void merging()
    {
        const auto someFontName_01 = "SomeFontName_01";

        Attributes dest = {
        };

        Attributes src_01 = {
                {"fontFamily", someFontName_01},
        };

        mergeAttributes(dest, src_01);
        CPPUNIT_ASSERT(dest.size() == 1);
        CPPUNIT_ASSERT(dest.count("fontFamily") == 1);
        auto search_01 = dest.find("fontFamily");
        CPPUNIT_ASSERT(search_01 != dest.end());
        CPPUNIT_ASSERT(search_01->second == someFontName_01);

        const auto someFontName_02 = "SomeFontName_02";
        Attributes src_02 = {
                {"fontFamily", someFontName_02},
        };

        mergeAttributes(dest, src_02);
        CPPUNIT_ASSERT(dest.size() == 1);
        CPPUNIT_ASSERT(dest.count("fontFamily") == 1);
        auto search_02 = dest.find("fontFamily");
        CPPUNIT_ASSERT(search_02 != dest.end());
        CPPUNIT_ASSERT(search_02->second == someFontName_02);

        const auto someFontName_03 = "SomeFontName_03";
        Attributes src_03 = {
                {"fontFamily", someFontName_03},
                {"color", "rgba(12,34,56,78)"},
                {"backgroundColor", "#ABCDEF12"},
                {"fontSize", "67.12%"},
        };

        mergeAttributes(dest, src_03);
        CPPUNIT_ASSERT(dest.size() == 4);
        CPPUNIT_ASSERT(dest.count("fontFamily") == 1);
        auto search_03 = dest.find("fontFamily");
        CPPUNIT_ASSERT(search_03 != dest.end());
        CPPUNIT_ASSERT(search_03->second == someFontName_03);


        StyleAttributeHandler styleAttrHndl;
        const auto someFontName_04 = "SomeFontName_04";
        styleAttrHndl.addAttribute("fontFamily", someFontName_01);
        styleAttrHndl.addAttribute("fontFamily", someFontName_02);
        styleAttrHndl.addAttribute("fontFamily", someFontName_03);
        styleAttrHndl.addAttribute("fontFamily", someFontName_04);


        mergeAttributes(dest, styleAttrHndl.getStyleAttributes());
        CPPUNIT_ASSERT(dest.size() == 4);
        CPPUNIT_ASSERT(dest.count("fontFamily") == 1);
        auto search_04 = dest.find("fontFamily");
        CPPUNIT_ASSERT(search_04 != dest.end());
        CPPUNIT_ASSERT(search_04->second == someFontName_04);
    }

};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( AttributeHandlersTest );

