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
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <map>

#include "Properties.hpp"

using subttxrend::common::Properties;
using subttxrend::common::paramsToProperties;

// Test enum for scoped enum functionality
enum class TestEnum : int
{
    VALUE_A = 1,
    VALUE_B = 2,
    VALUE_C = 3
};

class PropertiesTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(PropertiesTest);
    CPPUNIT_TEST(testDefaultConstructor);
    CPPUNIT_TEST(testSetGetString);
    CPPUNIT_TEST(testSetGetInt);
    CPPUNIT_TEST(testSetGetBool);
    CPPUNIT_TEST(testSetGetDouble);
    CPPUNIT_TEST(testOverwriteExistingKey);
    CPPUNIT_TEST(testGetStringMissingKeyThrows);
    CPPUNIT_TEST(testGetWithDefaultValue);
    CPPUNIT_TEST(testGetWithDefaultValueMissingKey);
    CPPUNIT_TEST(testSetGetEnumValueOnly);
    CPPUNIT_TEST(testHasKeyString);
    CPPUNIT_TEST(testHasKeyNonExistent);
    CPPUNIT_TEST(testForEachEmpty);
    CPPUNIT_TEST(testForEachMultipleEntries);
    CPPUNIT_TEST(testForEachOrder);
    CPPUNIT_TEST(testToStringEmpty);
    CPPUNIT_TEST(testToStringSingleEntry);
    CPPUNIT_TEST(testToStringMultipleEntries);
    CPPUNIT_TEST(testToStringFormat);
    CPPUNIT_TEST(testClear);
    CPPUNIT_TEST(testClearEmpty);
    CPPUNIT_TEST(testParamsToPropertiesNormal);
    CPPUNIT_TEST(testParamsToPropertiesEmpty);
    CPPUNIT_TEST(testParamsToPropertiesTrailingSemicolon);
    CPPUNIT_TEST(testParamsToPropertiesValueWithColon);
    CPPUNIT_TEST(testParamsToPropertiesDuplicateKeys);
    CPPUNIT_TEST(testParamsToPropertiesEmptySegments);
    CPPUNIT_TEST(testParamsToPropertiesNoColon);
    CPPUNIT_TEST(testParamsToPropertiesMultipleColons);
    CPPUNIT_TEST(testLargeNumberOfEntries);
    CPPUNIT_TEST(testSpecialCharactersInKeys);
    CPPUNIT_TEST(testSpecialCharactersInValues);
    CPPUNIT_TEST(testEmptyKeyAndValue);
    CPPUNIT_TEST(testStreamOperator);
    CPPUNIT_TEST(testTtmlLikeIntegration);
    CPPUNIT_TEST(testComplexParsingAndIteration);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // Setup any common test data if needed
    }

    void tearDown()
    {
        // Cleanup after tests
    }

    void testDefaultConstructor()
    {
        Properties props;
        CPPUNIT_ASSERT(!props.hasKey("any"));
        CPPUNIT_ASSERT_EQUAL(std::string(""), props.toString());
    }
    
    void testSetGetString()
    {
        Properties props;
        props.setString("string_key", "string_value");
        
        CPPUNIT_ASSERT_EQUAL(std::string("string_value"), props.get<std::string>("string_key"));
        CPPUNIT_ASSERT_EQUAL(std::string("string_value"), props.getString("string_key"));
    }
    
    void testSetGetInt()
    {
        Properties props;
        props.setString("int_key", "42");
        
        CPPUNIT_ASSERT_EQUAL(42, props.get<int>("int_key"));
        CPPUNIT_ASSERT_EQUAL(std::string("42"), props.getString("int_key"));
    }
    
    void testSetGetBool()
    {
        Properties props;
        props.setString("bool_key", "true");
        
        CPPUNIT_ASSERT_EQUAL(true, props.get<bool>("bool_key"));
        CPPUNIT_ASSERT_EQUAL(std::string("true"), props.getString("bool_key"));
        
        props.setString("bool_key_false", "false");
        CPPUNIT_ASSERT_EQUAL(false, props.get<bool>("bool_key_false"));
        CPPUNIT_ASSERT_EQUAL(std::string("false"), props.getString("bool_key_false"));
    }
    
    void testSetGetDouble()
    {
        Properties props;
        props.setString("double_key", "3.14");
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(3.14, props.get<double>("double_key"), 0.001);
        CPPUNIT_ASSERT_EQUAL(std::string("3.14"), props.getString("double_key"));
    }
    
    void testOverwriteExistingKey()
    {
        Properties props;
        props.setString("key", "original");
        CPPUNIT_ASSERT_EQUAL(std::string("original"), props.getString("key"));
        
        props.setString("key", "overwritten");
        CPPUNIT_ASSERT_EQUAL(std::string("overwritten"), props.getString("key"));
    }

    void testGetStringMissingKeyThrows()
    {
        Properties props;
        props.setString("existing", "value");
        
        CPPUNIT_ASSERT_THROW(props.getString("missing"), std::logic_error);
        
        try {
            props.getString("missing");
            CPPUNIT_FAIL("Expected std::logic_error");
        } catch (const std::logic_error& e) {
            std::string message = e.what();
            CPPUNIT_ASSERT(message.find("missing") != std::string::npos);
        }
    }
    
    void testGetWithDefaultValue()
    {
        Properties props;
        props.setString("existing", "42");
        
        CPPUNIT_ASSERT_EQUAL(42, props.get("existing", 99));
        CPPUNIT_ASSERT_EQUAL(std::string("42"), props.getString("existing"));
    }
    
    void testGetWithDefaultValueMissingKey()
    {
        Properties props;
        
        CPPUNIT_ASSERT_EQUAL(99, props.get("missing", 99));
        CPPUNIT_ASSERT_EQUAL(std::string("default"), props.get("missing", std::string("default")));
    }

    void testSetGetEnumValueOnly()
    {
        Properties props;
        // Store enum value under a string key
        props.setString("enum_key", "2");
        CPPUNIT_ASSERT(props.hasKey("enum_key"));
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(TestEnum::VALUE_B), props.get<int>("enum_key"));
        CPPUNIT_ASSERT_EQUAL(std::string("2"), props.getString("enum_key"));
    }

    void testHasKeyString()
    {
        Properties props;
        props.setString("existing", "value");
        
        CPPUNIT_ASSERT(props.hasKey("existing"));
        CPPUNIT_ASSERT(!props.hasKey("missing"));
    }

    void testHasKeyNonExistent()
    {
        Properties props;
        CPPUNIT_ASSERT(!props.hasKey("any"));
        // Verify container is still empty
        CPPUNIT_ASSERT_EQUAL(std::string(""), props.toString());
        // Repeated queries shouldn't mutate state
        CPPUNIT_ASSERT(!props.hasKey("any"));
        CPPUNIT_ASSERT(!props.hasKey("another"));
        // Default value path works for missing key
        CPPUNIT_ASSERT_EQUAL(123, props.get("missing_int", 123));
        // getString on missing still throws
        CPPUNIT_ASSERT_THROW(props.getString("missing_str"), std::logic_error);
        // forEach should not invoke callback
        int iterations = 0;
        props.forEach([&](const std::string&, const std::string&){ ++iterations; });

        CPPUNIT_ASSERT_EQUAL(0, iterations);
    }

    void testForEachEmpty()
    {
        Properties props;
        int count = 0;
        
        props.forEach([&count](const std::string& k, const std::string& v) {
            count++;
        });
        
        CPPUNIT_ASSERT_EQUAL(0, count);
    }
    
    void testForEachMultipleEntries()
    {
        Properties props;
        props.setString("key1", "value1");
        props.setString("key2", "value2");
        props.setString("key3", "value3");
        
        std::vector<std::pair<std::string, std::string>> collected;
        props.forEach([&collected](const std::string& k, const std::string& v) {
            collected.emplace_back(k, v);
        });
        
        CPPUNIT_ASSERT_EQUAL(size_t(3), collected.size());
        
        // Properties uses std::map, so entries should be in lexicographical order
        CPPUNIT_ASSERT_EQUAL(std::string("key1"), collected[0].first);
        CPPUNIT_ASSERT_EQUAL(std::string("value1"), collected[0].second);
        CPPUNIT_ASSERT_EQUAL(std::string("key2"), collected[1].first);
        CPPUNIT_ASSERT_EQUAL(std::string("value2"), collected[1].second);
        CPPUNIT_ASSERT_EQUAL(std::string("key3"), collected[2].first);
        CPPUNIT_ASSERT_EQUAL(std::string("value3"), collected[2].second);
    }
    
    void testForEachOrder()
    {
        Properties props;
        props.setString("zebra", "z");
        props.setString("alpha", "a");
        props.setString("beta", "b");
        
        std::vector<std::string> keys;
        props.forEach([&keys](const std::string& k, const std::string& v) {
            keys.push_back(k);
        });
        
        CPPUNIT_ASSERT_EQUAL(size_t(3), keys.size());
        CPPUNIT_ASSERT_EQUAL(std::string("alpha"), keys[0]);
        CPPUNIT_ASSERT_EQUAL(std::string("beta"), keys[1]);
        CPPUNIT_ASSERT_EQUAL(std::string("zebra"), keys[2]);
    }

    void testToStringEmpty()
    {
        Properties props;
        CPPUNIT_ASSERT_EQUAL(std::string(""), props.toString());
    }
    
    void testToStringSingleEntry()
    {
        Properties props;
        props.setString("key", "value");
        
        std::string result = props.toString();
        CPPUNIT_ASSERT_EQUAL(std::string("key: value, "), result);
    }
    
    void testToStringMultipleEntries()
    {
        Properties props;
        props.setString("key1", "value1");
        props.setString("key2", "value2");
        
        std::string result = props.toString();
        CPPUNIT_ASSERT_EQUAL(std::string("key1: value1, key2: value2, "), result);
    }
    
    void testToStringFormat()
    {
        Properties props;
        props.setString("number", "42");
        props.setString("bool", "true");
        
        std::string result = props.toString();
        // Should contain key: value, format with trailing ", "
        CPPUNIT_ASSERT(result.find("bool: true, ") != std::string::npos);
        CPPUNIT_ASSERT(result.find("number: 42, ") != std::string::npos);
        CPPUNIT_ASSERT(result.substr(result.length() - 2) == ", ");
    }

    void testClear()
    {
        Properties props;
        props.setString("key1", "value1");
        props.setString("key2", "value2");
        
        CPPUNIT_ASSERT(props.hasKey("key1"));
        CPPUNIT_ASSERT(props.hasKey("key2"));
        
        props.clear();
        
        CPPUNIT_ASSERT(!props.hasKey("key1"));
        CPPUNIT_ASSERT(!props.hasKey("key2"));
        CPPUNIT_ASSERT_EQUAL(std::string(""), props.toString());
    }
    
    void testClearEmpty()
    {
        Properties props;
        props.clear(); // Should not crash
        CPPUNIT_ASSERT_EQUAL(std::string(""), props.toString());
    }

    void testParamsToPropertiesNormal()
    {
        Properties props;
        paramsToProperties(props, "key1:value1;key2:value2");
        
        CPPUNIT_ASSERT(props.hasKey("key1"));
        CPPUNIT_ASSERT(props.hasKey("key2"));
        CPPUNIT_ASSERT_EQUAL(std::string("value1"), props.getString("key1"));
        CPPUNIT_ASSERT_EQUAL(std::string("value2"), props.getString("key2"));
    }
    
    void testParamsToPropertiesEmpty()
    {
        Properties props;
        paramsToProperties(props, "");
        
        CPPUNIT_ASSERT_EQUAL(std::string(""), props.toString());
    }
    
    void testParamsToPropertiesTrailingSemicolon()
    {
        Properties props;
        paramsToProperties(props, "key1:value1;");
        
        CPPUNIT_ASSERT(props.hasKey("key1"));
        CPPUNIT_ASSERT_EQUAL(std::string("value1"), props.getString("key1"));
    }
    
    void testParamsToPropertiesValueWithColon()
    {
        Properties props;
        paramsToProperties(props, "url:http://example.com:8080");
        
        CPPUNIT_ASSERT(props.hasKey("url"));
        CPPUNIT_ASSERT_EQUAL(std::string("http://example.com:8080"), props.getString("url"));
    }
    
    void testParamsToPropertiesDuplicateKeys()
    {
        Properties props;
        paramsToProperties(props, "key1:first;key1:second");
        
        CPPUNIT_ASSERT(props.hasKey("key1"));
        CPPUNIT_ASSERT_EQUAL(std::string("second"), props.getString("key1"));
    }
    
    void testParamsToPropertiesEmptySegments()
    {
        Properties props;
        paramsToProperties(props, "key1:value1;;key2:value2");
        
        CPPUNIT_ASSERT(props.hasKey("key1"));
        CPPUNIT_ASSERT(props.hasKey("key2"));
        CPPUNIT_ASSERT_EQUAL(std::string("value1"), props.getString("key1"));
        CPPUNIT_ASSERT_EQUAL(std::string("value2"), props.getString("key2"));
    }
    
    void testParamsToPropertiesNoColon()
    {
        Properties props;
        paramsToProperties(props, "key1;key2:value2");
        
        // key1 without colon should be ignored (no value set)
        CPPUNIT_ASSERT(!props.hasKey("key1"));
        CPPUNIT_ASSERT(props.hasKey("key2"));
        CPPUNIT_ASSERT_EQUAL(std::string("value2"), props.getString("key2"));
    }
    
    void testParamsToPropertiesMultipleColons()
    {
        Properties props;
        paramsToProperties(props, "time:12:34:56;url:http://test:8080/path");
        
        CPPUNIT_ASSERT(props.hasKey("time"));
        CPPUNIT_ASSERT(props.hasKey("url"));
        CPPUNIT_ASSERT_EQUAL(std::string("12:34:56"), props.getString("time"));
        CPPUNIT_ASSERT_EQUAL(std::string("http://test:8080/path"), props.getString("url"));
    }

    void testLargeNumberOfEntries()
    {
        Properties props;
        const int count = 1000;
        
        for (int i = 0; i < count; ++i) {
            props.setString("key" + std::to_string(i), "value" + std::to_string(i));
        }
        
        for (int i = 0; i < count; ++i) {
            CPPUNIT_ASSERT(props.hasKey("key" + std::to_string(i)));
            CPPUNIT_ASSERT_EQUAL(std::string("value" + std::to_string(i)), 
                                props.getString("key" + std::to_string(i)));
        }
    }
    
    void testSpecialCharactersInKeys()
    {
        Properties props;
        props.setString("key with spaces", "value1");
        props.setString("key-with-dashes", "value2");
        props.setString("key_with_underscores", "value3");
        props.setString("key.with.dots", "value4");
        
        CPPUNIT_ASSERT(props.hasKey("key with spaces"));
        CPPUNIT_ASSERT(props.hasKey("key-with-dashes"));
        CPPUNIT_ASSERT(props.hasKey("key_with_underscores"));
        CPPUNIT_ASSERT(props.hasKey("key.with.dots"));
    }
    
    void testSpecialCharactersInValues()
    {
        Properties props;
        props.setString("newlines", "line1\nline2");
        props.setString("tabs", "col1\tcol2");
        props.setString("quotes", "He said \"Hello\"");
        props.setString("unicode", "héllo wörld 🌟");
        
        CPPUNIT_ASSERT_EQUAL(std::string("line1\nline2"), props.getString("newlines"));
        CPPUNIT_ASSERT_EQUAL(std::string("col1\tcol2"), props.getString("tabs"));
        CPPUNIT_ASSERT_EQUAL(std::string("He said \"Hello\""), props.getString("quotes"));
        CPPUNIT_ASSERT_EQUAL(std::string("héllo wörld 🌟"), props.getString("unicode"));
    }
    
    void testEmptyKeyAndValue()
    {
        Properties props;
        props.setString("", "empty_key");
        props.setString("empty_value", "");
        
        CPPUNIT_ASSERT(props.hasKey(""));
        CPPUNIT_ASSERT(props.hasKey("empty_value"));
        CPPUNIT_ASSERT_EQUAL(std::string("empty_key"), props.getString(""));
        CPPUNIT_ASSERT_EQUAL(std::string(""), props.getString("empty_value"));
    }

    void testStreamOperator()
    {
        Properties props;
        props.setString("key1", "value1");
        props.setString("key2", "value2");
        
        std::ostringstream oss;
        oss << props;
        
        std::string result = oss.str();
        CPPUNIT_ASSERT_EQUAL(props.toString(), result);
    }

    void testTtmlLikeIntegration()
    {
        // Mimic the TTML usage pattern: paramsToProperties + forEach
        Properties styleProperties;
        std::string styling = "color:red;font-size:12px;font-weight:bold";
        
        paramsToProperties(styleProperties, styling);
        
        // Verify parsing worked
        CPPUNIT_ASSERT(styleProperties.hasKey("color"));
        CPPUNIT_ASSERT(styleProperties.hasKey("font-size"));
        CPPUNIT_ASSERT(styleProperties.hasKey("font-weight"));
        
        // Simulate building attributes map like in TTML
        std::map<std::string, std::string> attributes;
        styleProperties.forEach([&attributes](const std::string& k, const std::string& v) {
            attributes.emplace(k, v);
        });
        
        CPPUNIT_ASSERT_EQUAL(size_t(3), attributes.size());
        CPPUNIT_ASSERT_EQUAL(std::string("red"), attributes["color"]);
        CPPUNIT_ASSERT_EQUAL(std::string("12px"), attributes["font-size"]);
        CPPUNIT_ASSERT_EQUAL(std::string("bold"), attributes["font-weight"]);
    }
    
    void testComplexParsingAndIteration()
    {
        Properties props;
        
        // Complex real-world-like parameter string
        std::string params = "server:localhost:8080;timeout:30;ssl:true;path:/api/v1;query:param=value&other=data";
        paramsToProperties(props, params);
        
        // Verify all properties were parsed correctly
        CPPUNIT_ASSERT_EQUAL(std::string("localhost:8080"), props.getString("server"));
        CPPUNIT_ASSERT_EQUAL(30, props.get<int>("timeout"));
        CPPUNIT_ASSERT_EQUAL(true, props.get<bool>("ssl"));
        CPPUNIT_ASSERT_EQUAL(std::string("/api/v1"), props.getString("path"));
        CPPUNIT_ASSERT_EQUAL(std::string("param=value&other=data"), props.getString("query"));
        
        // Test iteration and accumulation
        std::ostringstream combined;
        props.forEach([&combined](const std::string& k, const std::string& v) {
            combined << k << "=" << v << "&";
        });
        
        std::string result = combined.str();
        CPPUNIT_ASSERT(!result.empty());
        CPPUNIT_ASSERT(result.find("server=localhost:8080&") != std::string::npos);
        CPPUNIT_ASSERT(result.find("ssl=true&") != std::string::npos);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PropertiesTest);
