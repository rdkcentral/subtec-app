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
    CPPUNIT_TEST(invalidColorParsing);
    CPPUNIT_TEST(transparentBackgroundColor);
    CPPUNIT_TEST(invalidFontSizeAndLineHeight);
    CPPUNIT_TEST(negativeFontSizeLineHeight);
    CPPUNIT_TEST(outlineWithOnlyThickness);
    CPPUNIT_TEST(outlineWithInvalidColorOrThickness);
    CPPUNIT_TEST(unknownTextAlignAndDisplayAlign);
    CPPUNIT_TEST(toStrOutput);
    CPPUNIT_TEST(operatorEquality);
    CPPUNIT_TEST(setAndGetStyleId);
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

    void invalidColorParsing()
    {
        StyleSet styleSet;
        Attributes attrs = {
            {"color", "notacolor"},
            {"backgroundColor", "#GGHHII"},
        };
        styleSet.merge(attrs);
        CPPUNIT_ASSERT(styleSet.getColor() == subttxrend::gfx::ColorArgb::WHITE);
        CPPUNIT_ASSERT(styleSet.getBackgroundColor() == subttxrend::gfx::ColorArgb::TRANSPARENT);
    }

    void transparentBackgroundColor()
    {
        StyleSet styleSet;
        Attributes attrs = {
            {"backgroundColor", "rgba(0,0,0,0)"}
        };
        styleSet.merge(attrs);
        CPPUNIT_ASSERT(styleSet.getBackgroundColor() == subttxrend::gfx::ColorArgb::BLACK);
    }

    void invalidFontSizeAndLineHeight()
    {
        StyleSet styleSet;
        Attributes attrs = {
            {"fontSize", "abc"},
            {"lineHeight", "100percent"}
        };
        styleSet.merge(attrs);
        CPPUNIT_ASSERT((styleSet.getFontSize() == DomainValue{DomainValue::Type::CELL_HUNDREDTHS, 100}));
        CPPUNIT_ASSERT((styleSet.getLineHeight() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 100*100}));
    }

    void negativeFontSizeLineHeight()
    {
        StyleSet styleSet;
        Attributes attrs = {
            {"fontSize", "-10px"},
            {"lineHeight", "-1px"}
        };
        styleSet.merge(attrs);
        CPPUNIT_ASSERT((styleSet.getFontSize() == DomainValue{DomainValue::Type::CELL_HUNDREDTHS, 100}));
        CPPUNIT_ASSERT((styleSet.getLineHeight() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 100*100}));
    }

    void outlineWithOnlyThickness()
    {
        StyleSet styleSet;
        Attributes attrs = {
            {"textOutline", "2px"}
        };
        styleSet.merge(attrs);
        Outline expected{DomainValue{DomainValue::Type::PIXEL, 2}, subttxrend::gfx::ColorArgb::BLACK};
        CPPUNIT_ASSERT(styleSet.getOutline() == expected);
    }

    void outlineWithInvalidColorOrThickness()
    {
        StyleSet styleSet;
        Attributes attrs = {
            {"textOutline", "notacolor 2px"},
            {"textOutline", "rgba(0,0,0,255) notasize"}
        };
        styleSet.merge(attrs);
        Outline expected{DomainValue{DomainValue::Type::CELL_HUNDREDTHS, 0}, subttxrend::gfx::ColorArgb::BLACK};
        CPPUNIT_ASSERT(styleSet.getOutline() == expected);
    }

    void unknownTextAlignAndDisplayAlign()
    {
        StyleSet styleSet;
        Attributes attrs = {
            {"textAlign", "foo"},
            {"displayAlign", "bar"}
        };
        styleSet.merge(attrs);
        CPPUNIT_ASSERT(styleSet.getTextAlign() == StyleSet::TextAlign::LEFT);
        CPPUNIT_ASSERT(styleSet.getDisplayAlign() == StyleSet::DisplayAlign::BEFORE);
    }

    void toStrOutput()
    {
        StyleSet styleSet;
        styleSet.setStyleId("myStyle");
        std::string str = styleSet.toStr();
        CPPUNIT_ASSERT(str.find("[myStyle]:") != std::string::npos);
        CPPUNIT_ASSERT(str.find("font:") != std::string::npos);
        CPPUNIT_ASSERT(str.find("bg:") != std::string::npos);
    }

    void operatorEquality()
    {
        StyleSet s1, s2;
        CPPUNIT_ASSERT(s1 == s2);
        Attributes attrs = {{"color", "yellow"}};
        s1.merge(attrs);
        CPPUNIT_ASSERT(!(s1 == s2));
        s2.merge(attrs);
        CPPUNIT_ASSERT(s1 == s2);
    }

    void setAndGetStyleId()
    {
        StyleSet styleSet;
        styleSet.setStyleId("style42");
        CPPUNIT_ASSERT(styleSet.getStyleId() == "style42");
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( StyleSetTest );

