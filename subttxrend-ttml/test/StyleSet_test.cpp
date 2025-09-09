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
#include "Parser/StyleSet.hpp"
#include "Parser/Outline.hpp"
#include <subttxrend/gfx/ColorArgb.hpp>

using namespace subttxrend::ttmlengine;

class StyleSetTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( StyleSetTest );
    CPPUNIT_TEST(checkDefaults);
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

    void checkDefaults()
    {
        StyleSet styleSet{};

        CPPUNIT_ASSERT(styleSet.getColor() == subttxrend::gfx::ColorArgb::WHITE);
        CPPUNIT_ASSERT(styleSet.getBackgroundColor() == subttxrend::gfx::ColorArgb::TRANSPARENT);
        CPPUNIT_ASSERT(styleSet.getFontFamily() == "default");
        CPPUNIT_ASSERT((styleSet.getFontSize() == DomainValue{DomainValue::Type::CELL_HUNDREDTHS, 100}));
        CPPUNIT_ASSERT((styleSet.getLineHeight() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 100*100}));
        CPPUNIT_ASSERT(styleSet.getTextAlign() == StyleSet::TextAlign::CENTER);
        CPPUNIT_ASSERT(styleSet.getDisplayAlign() == StyleSet::DisplayAlign::BEFORE);
        auto outline = Outline{DomainValue{DomainValue::Type::CELL_HUNDREDTHS, 0}, subttxrend::gfx::ColorArgb{subttxrend::gfx::ColorArgb::BLACK}};
        CPPUNIT_ASSERT((styleSet.getOutline() == outline));
    }

    void merging()
    {
        StyleSet styleSet{};

        const auto someFontName = "SomeFontName";

        Attributes oneAttribute = {
                {"fontFamily", someFontName},
        };

        styleSet.merge(oneAttribute);

        CPPUNIT_ASSERT(styleSet.getColor() == subttxrend::gfx::ColorArgb::WHITE);
        CPPUNIT_ASSERT(styleSet.getBackgroundColor() == subttxrend::gfx::ColorArgb::TRANSPARENT);
        CPPUNIT_ASSERT(styleSet.getFontFamily() == std::string{someFontName});
        CPPUNIT_ASSERT((styleSet.getFontSize() == DomainValue{DomainValue::Type::CELL_HUNDREDTHS, 100}));
        CPPUNIT_ASSERT((styleSet.getLineHeight() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 100*100}));
        CPPUNIT_ASSERT(styleSet.getTextAlign() == StyleSet::TextAlign::CENTER);
        CPPUNIT_ASSERT(styleSet.getDisplayAlign() == StyleSet::DisplayAlign::BEFORE);
        auto outline = Outline{DomainValue{DomainValue::Type::CELL_HUNDREDTHS, 0}, subttxrend::gfx::ColorArgb{subttxrend::gfx::ColorArgb::BLACK}};
        CPPUNIT_ASSERT((styleSet.getOutline() == outline));

        const auto totalyDifferentFontName = "totalyDifferentFontName";

        Attributes styleAttributes = {
                {"color", "rgba(12,34,56,78)"},
                {"backgroundColor", "#ABCDEF12"},
                {"fontFamily", totalyDifferentFontName},
                {"fontSize", "67.12%"},
                {"lineHeight", "90.98%"},
                {"textAlign", "end"},
                {"displayAlign", "after"},
                {"textOutline", "rgba(12,34,56,78) 67.12%"}
        };

        styleSet.merge(styleAttributes);

        CPPUNIT_ASSERT((styleSet.getColor() == subttxrend::gfx::ColorArgb{0x4E, 0x0C, 0x22, 0x38}));
        CPPUNIT_ASSERT((styleSet.getBackgroundColor() == subttxrend::gfx::ColorArgb{0x12, 0xAB, 0xCD, 0xEF}));
        CPPUNIT_ASSERT(styleSet.getFontFamily() == std::string{totalyDifferentFontName});
        CPPUNIT_ASSERT((styleSet.getFontSize() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 6712}));
        CPPUNIT_ASSERT((styleSet.getLineHeight() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 9098}));
        CPPUNIT_ASSERT(styleSet.getTextAlign() == StyleSet::TextAlign::RIGHT);
        CPPUNIT_ASSERT(styleSet.getDisplayAlign() == StyleSet::DisplayAlign::AFTER);
        outline = Outline{DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 6712}, subttxrend::gfx::ColorArgb{0x4E, 0x0C, 0x22, 0x38}};
        CPPUNIT_ASSERT((styleSet.getOutline() == outline));

        Attributes colorOnlyAttribute = {
                {"color", "rgba(255,255,255,255)"},
        };
        styleSet.merge(colorOnlyAttribute);

        CPPUNIT_ASSERT((styleSet.getColor() == subttxrend::gfx::ColorArgb{0xFF, 0xFF, 0xFF, 0xFF}));
        CPPUNIT_ASSERT((styleSet.getBackgroundColor() == subttxrend::gfx::ColorArgb{0x12, 0xAB, 0xCD, 0xEF}));
        CPPUNIT_ASSERT(styleSet.getFontFamily() == std::string{totalyDifferentFontName});
        CPPUNIT_ASSERT((styleSet.getFontSize() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 6712}));
        CPPUNIT_ASSERT((styleSet.getLineHeight() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 9098}));
        CPPUNIT_ASSERT(styleSet.getTextAlign() == StyleSet::TextAlign::RIGHT);
        CPPUNIT_ASSERT(styleSet.getDisplayAlign() == StyleSet::DisplayAlign::AFTER);
        CPPUNIT_ASSERT((styleSet.getOutline() == outline));

        Attributes yellowColorOnlyAttribute = {
                {"color", "YeLloW"},
        };
        styleSet.merge(yellowColorOnlyAttribute);

        CPPUNIT_ASSERT((styleSet.getColor() == subttxrend::gfx::ColorArgb::YELLOW));
        CPPUNIT_ASSERT((styleSet.getBackgroundColor() == subttxrend::gfx::ColorArgb{0x12, 0xAB, 0xCD, 0xEF}));
        CPPUNIT_ASSERT(styleSet.getFontFamily() == std::string{totalyDifferentFontName});
        CPPUNIT_ASSERT((styleSet.getFontSize() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 6712}));
        CPPUNIT_ASSERT((styleSet.getLineHeight() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 9098}));
        CPPUNIT_ASSERT(styleSet.getTextAlign() == StyleSet::TextAlign::RIGHT);
        CPPUNIT_ASSERT(styleSet.getDisplayAlign() == StyleSet::DisplayAlign::AFTER);
        CPPUNIT_ASSERT((styleSet.getOutline() == outline));

        Attributes greenBgColorOnlyAttribute = {
                {"backgroundColor", "rgba( 0 , 128 , 0 , 255 )"},
        };
        styleSet.merge(greenBgColorOnlyAttribute);

        CPPUNIT_ASSERT((styleSet.getColor() == subttxrend::gfx::ColorArgb::YELLOW));
        CPPUNIT_ASSERT((styleSet.getBackgroundColor() == subttxrend::gfx::ColorArgb::GREEN));
        CPPUNIT_ASSERT(styleSet.getFontFamily() == std::string{totalyDifferentFontName});
        CPPUNIT_ASSERT((styleSet.getFontSize() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 6712}));
        CPPUNIT_ASSERT((styleSet.getLineHeight() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 9098}));
        CPPUNIT_ASSERT(styleSet.getTextAlign() == StyleSet::TextAlign::RIGHT);
        CPPUNIT_ASSERT(styleSet.getDisplayAlign() == StyleSet::DisplayAlign::AFTER);
        CPPUNIT_ASSERT((styleSet.getOutline() == outline));

        Attributes invalidAttribute = {
                {"something", "something else"},
        };
        styleSet.merge(invalidAttribute);

        CPPUNIT_ASSERT((styleSet.getColor() == subttxrend::gfx::ColorArgb::YELLOW));
        CPPUNIT_ASSERT((styleSet.getBackgroundColor() == subttxrend::gfx::ColorArgb::GREEN));
        CPPUNIT_ASSERT(styleSet.getFontFamily() == std::string{totalyDifferentFontName});
        CPPUNIT_ASSERT((styleSet.getFontSize() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 6712}));
        CPPUNIT_ASSERT((styleSet.getLineHeight() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 9098}));
        CPPUNIT_ASSERT(styleSet.getTextAlign() == StyleSet::TextAlign::RIGHT);
        CPPUNIT_ASSERT(styleSet.getDisplayAlign() == StyleSet::DisplayAlign::AFTER);
        CPPUNIT_ASSERT((styleSet.getOutline() == outline));
    }

};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( StyleSetTest );

