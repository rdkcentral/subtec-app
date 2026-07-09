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

#include <sstream>

#include <cppunit/extensions/HelperMacros.h>
#include "Parser/AttributeHandlers.hpp"

using namespace subttxrend::ttmlengine;

class AttributeHandlersTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( AttributeHandlersTest );
    CPPUNIT_TEST(mergeAttributes_emptySource);
    CPPUNIT_TEST(mergeAttributes_overwrite);
    CPPUNIT_TEST(operatorStream_empty);
    CPPUNIT_TEST(styleAttributeHandler_addAndGet);
    CPPUNIT_TEST(styleAttributeHandler_set);
    CPPUNIT_TEST(testStyleAttributeHandlerSetReplacesAll);
    CPPUNIT_TEST(testStyleAttributeHandlerGetStyleAttributesReturnsReference);
    CPPUNIT_TEST(testMergeAttributesEmptyDestination);
    CPPUNIT_TEST(testOperatorStreamSingleAttribute);
    CPPUNIT_TEST(testOperatorStreamSpecialChars);
    CPPUNIT_TEST(testStyleAttributeHandlerEmptyKeyValue);
    CPPUNIT_TEST(testStyleAttributeHandlerSpecialChars);
    CPPUNIT_TEST(testStyleAttributeHandlerSetEmpty);
CPPUNIT_TEST_SUITE_END();

    class TestableStyleAttributeHandler : public StyleAttributeHandler
    {
    public:
        const Attributes& exposeStyleAttributes() const
        {
            return m_styleAttributes;
        }
    };

public:
    void setUp()
    {
        // noop
    }

    void tearDown()
    {
        // noop
    }

    void mergeAttributes_emptySource()
    {
        Attributes dest = { {"a", "1"}, {"b", "2"} };
        Attributes src = {};
        mergeAttributes(dest, src);
        CPPUNIT_ASSERT(dest.size() == 2);
        CPPUNIT_ASSERT(dest.at("a") == "1");
        CPPUNIT_ASSERT(dest.at("b") == "2");
    }

    void mergeAttributes_overwrite()
    {
        Attributes dest = { {"a", "1"}, {"b", "2"} };
        Attributes src = { {"a", "3"}, {"c", "4"} };
        mergeAttributes(dest, src);
        CPPUNIT_ASSERT(dest.size() == 3);
        CPPUNIT_ASSERT(dest.at("a") == "3");
        CPPUNIT_ASSERT(dest.at("b") == "2");
        CPPUNIT_ASSERT(dest.at("c") == "4");
    }

    void operatorStream_empty()
    {
        Attributes attrSet = {};
        std::ostringstream oss;
        oss << attrSet;
        CPPUNIT_ASSERT(oss.str() == "");
    }

    void styleAttributeHandler_addAndGet()
    {
        StyleAttributeHandler handler;
        handler.addAttribute("foo", "bar");
        handler.addAttribute("foo", "baz");
        const Attributes& attrs = handler.getStyleAttributes();
        CPPUNIT_ASSERT(attrs.size() == 1);
        CPPUNIT_ASSERT(attrs.at("foo") == "baz");
    }

    void styleAttributeHandler_set()
    {
        StyleAttributeHandler handler;
        Attributes attrs = { {"a", "1"}, {"b", "2"} };
        handler.set(attrs);
        const Attributes& got = handler.getStyleAttributes();
        CPPUNIT_ASSERT(got.size() == 2);
        CPPUNIT_ASSERT(got.at("a") == "1");
        CPPUNIT_ASSERT(got.at("b") == "2");
    }

    void testStyleAttributeHandlerSetReplacesAll()
    {
        StyleAttributeHandler handler;
        handler.addAttribute("color", "red");
        Attributes newAttrs = {{"font", "Arial"}};
        handler.set(newAttrs);
        CPPUNIT_ASSERT(handler.getStyleAttributes().size() == 1);
        CPPUNIT_ASSERT(handler.getStyleAttributes().at("font") == "Arial");
    }

    void testStyleAttributeHandlerGetStyleAttributesReturnsReference()
    {
        TestableStyleAttributeHandler handler;
        handler.addAttribute("color", "red");

        const Attributes& ref = handler.getStyleAttributes();
        const Attributes& internal = handler.exposeStyleAttributes();

        CPPUNIT_ASSERT(&ref == &internal);
        CPPUNIT_ASSERT(ref.at("color") == "red");
    }

    void testMergeAttributesEmptyDestination()
    {
        Attributes dest;
        Attributes src = { {"key1", "value1"}, {"key2", "value2"} };
        mergeAttributes(dest, src);
        CPPUNIT_ASSERT(dest.size() == 2);
        CPPUNIT_ASSERT(dest.at("key1") == "value1");
        CPPUNIT_ASSERT(dest.at("key2") == "value2");
    }

    void testOperatorStreamSingleAttribute()
    {
        Attributes attrSet = { {"single", "value"} };
        std::ostringstream oss;
        oss << attrSet;
        CPPUNIT_ASSERT(oss.str() == "[single:value], ");
    }

    void testOperatorStreamSpecialChars()
    {
        Attributes attrSet = { {"key:with:colon", "value,with,comma"}, {"key[bracket]", "value]bracket["} };
        std::ostringstream oss;
        oss << attrSet;
        CPPUNIT_ASSERT(oss.str() == "[key:with:colon:value,with,comma], [key[bracket]:value]bracket[], ");
    }

    void testStyleAttributeHandlerEmptyKeyValue()
    {
        StyleAttributeHandler handler;
        handler.addAttribute("", "value");
        handler.addAttribute("key", "");
        const Attributes& attrs = handler.getStyleAttributes();
        CPPUNIT_ASSERT(attrs.at("") == "value");
        CPPUNIT_ASSERT(attrs.at("key") == "");
    }

    void testStyleAttributeHandlerSpecialChars()
    {
        StyleAttributeHandler handler;
        handler.addAttribute("key:with:colon", "value,with,comma");
        handler.addAttribute("key[bracket]", "value]bracket[");
        const Attributes& attrs = handler.getStyleAttributes();
        CPPUNIT_ASSERT(attrs.at("key:with:colon") == "value,with,comma");
        CPPUNIT_ASSERT(attrs.at("key[bracket]") == "value]bracket[");
    }

    void testStyleAttributeHandlerSetEmpty()
    {
        StyleAttributeHandler handler;
        handler.addAttribute("key", "value");
        Attributes emptyAttrs;
        handler.set(emptyAttrs);
        CPPUNIT_ASSERT(handler.getStyleAttributes().empty());
    }

};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( AttributeHandlersTest );

