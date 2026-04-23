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
#include <map>
#include <climits>

#include "ConfigProvider.hpp"

using subttxrend::common::ConfigProvider;

/**
 * Test ConfigProvider implementation that allows setting values for testing
 */
class TestConfigProvider : public ConfigProvider
{
public:
    void setValue(const std::string& key, const std::string& value)
    {
        m_values[key] = value;
    }

    void setValue(const std::string& key, const char* value)
    {
        if (value)
        {
            m_values[key] = std::string(value);
        }
        else
        {
            m_values.erase(key);
        }
    }

    void removeValue(const std::string& key)
    {
        m_values.erase(key);
    }

    void clear()
    {
        m_values.clear();
    }

protected:
    const char* getValue(const std::string& key) const override
    {
        auto it = m_values.find(key);
        if (it != m_values.end())
        {
            return it->second.c_str();
        }
        return nullptr;
    }

private:
    std::map<std::string, std::string> m_values;
};

class ConfigProviderTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(ConfigProviderTest);
    CPPUNIT_TEST(testHasValueWithExistingKey);
    CPPUNIT_TEST(testHasValueWithMissingKey);
    CPPUNIT_TEST(testHasValueWithEmptyString);
    CPPUNIT_TEST(testGetWithExistingKey);
    CPPUNIT_TEST(testGetWithMissingKey);
    CPPUNIT_TEST(testGetWithEmptyString);
    CPPUNIT_TEST(testGetWithWhitespace);
    CPPUNIT_TEST(testGetWithUnicodeContent);
    CPPUNIT_TEST(testGetWithLongValue);
    CPPUNIT_TEST(testGetIntValidDecimal);
    CPPUNIT_TEST(testGetIntWithWhitespace);
    CPPUNIT_TEST(testGetIntNegative);
    CPPUNIT_TEST(testGetIntZero);
    CPPUNIT_TEST(testGetIntWithPlusSign);
    CPPUNIT_TEST(testGetIntHexAutodetect);
    CPPUNIT_TEST(testGetIntOctalAutodetect);
    CPPUNIT_TEST(testGetIntMissingKey);
    CPPUNIT_TEST(testGetIntInvalidPartial);
    CPPUNIT_TEST(testGetIntNonNumeric);
    CPPUNIT_TEST(testGetIntEmptyString);
    CPPUNIT_TEST(testGetIntWhitespaceOnly);
    CPPUNIT_TEST(testGetIntSignOnly);
    CPPUNIT_TEST(testGetIntOverflow);
    CPPUNIT_TEST(testGetIntUnderflow);
    CPPUNIT_TEST(testGetArrayBasicSplit);
    CPPUNIT_TEST(testGetArraySingleToken);
    CPPUNIT_TEST(testGetArrayEmptyValue);
    CPPUNIT_TEST(testGetArrayMissingKey);
    CPPUNIT_TEST(testGetArrayMultiCharSeparator);
    CPPUNIT_TEST(testGetArrayConsecutiveSeparators);
    CPPUNIT_TEST(testGetArrayLeadingSeparator);
    CPPUNIT_TEST(testGetArrayTrailingSeparator);
    CPPUNIT_TEST(testGetArrayOnlySeparators);
    CPPUNIT_TEST(testGetArraySeparatorNotFound);
    CPPUNIT_TEST(testGetArrayEmptySeparator);
    CPPUNIT_TEST(testGetArrayLargeInput);
    CPPUNIT_TEST(testGetCstrWithExistingKey);
    CPPUNIT_TEST(testGetCstrWithMissingKey);
    CPPUNIT_TEST(testGetCstrWithEmptyString);
    CPPUNIT_TEST(testGetCstrNullDefault);
    CPPUNIT_TEST(testMixedTypesInConfig);
    CPPUNIT_TEST(testBulkRetrieval);
    CPPUNIT_TEST(testHasValueVsGetDefaultConsistency);
    CPPUNIT_TEST(testArrayWithContentContainingSeparator);
    CPPUNIT_TEST(testVeryLongKeyName);
    CPPUNIT_TEST(testSequentialCalls);
    CPPUNIT_TEST(testConstCorrectnessAllMethods);
    CPPUNIT_TEST(testUnicodeKeyAndValue);
    CPPUNIT_TEST(testSpecialCharactersInValues);
    CPPUNIT_TEST(testNullCharacterInValue);
CPPUNIT_TEST_SUITE_END();

private:
    TestConfigProvider m_provider;

public:
    void setUp()
    {
        m_provider.clear();
    }

    void tearDown()
    {
        m_provider.clear();
    }

    void testHasValueWithExistingKey()
    {
        m_provider.setValue("testKey", "testValue");
        CPPUNIT_ASSERT(m_provider.hasValue("testKey"));
    }

    void testHasValueWithMissingKey()
    {
        CPPUNIT_ASSERT(!m_provider.hasValue("nonExistentKey"));
    }

    void testHasValueWithEmptyString()
    {
        m_provider.setValue("emptyKey", "");
        CPPUNIT_ASSERT(m_provider.hasValue("emptyKey"));
    }

    void testGetWithExistingKey()
    {
        m_provider.setValue("testKey", "testValue");
        CPPUNIT_ASSERT_EQUAL(std::string("testValue"), m_provider.get("testKey"));
    }

    void testGetWithMissingKey()
    {
        CPPUNIT_ASSERT_EQUAL(std::string("default"), m_provider.get("missing", "default"));
        CPPUNIT_ASSERT_EQUAL(std::string(""), m_provider.get("missing"));
    }

    void testGetWithEmptyString()
    {
        m_provider.setValue("emptyKey", "");
        CPPUNIT_ASSERT_EQUAL(std::string(""), m_provider.get("emptyKey", "default"));
    }

    void testGetWithWhitespace()
    {
        m_provider.setValue("whitespaceKey", "  value with spaces  ");
        CPPUNIT_ASSERT_EQUAL(std::string("  value with spaces  "), m_provider.get("whitespaceKey"));
    }

    void testGetWithUnicodeContent()
    {
        m_provider.setValue("unicodeKey", "テスト文字列");
        CPPUNIT_ASSERT_EQUAL(std::string("テスト文字列"), m_provider.get("unicodeKey"));
    }

    void testGetWithLongValue()
    {
        std::string longValue(10000, 'x');
        m_provider.setValue("longKey", longValue);
        CPPUNIT_ASSERT_EQUAL(longValue, m_provider.get("longKey"));
    }

    void testGetIntValidDecimal()
    {
        m_provider.setValue("intKey", "42");
        CPPUNIT_ASSERT_EQUAL(42, m_provider.getInt("intKey"));
    }

    void testGetIntWithWhitespace()
    {
        m_provider.setValue("intKey", "  42  ");
        CPPUNIT_ASSERT_EQUAL(42, m_provider.getInt("intKey"));
    }

    void testGetIntNegative()
    {
        m_provider.setValue("intKey", "-7");
        CPPUNIT_ASSERT_EQUAL(-7, m_provider.getInt("intKey"));
    }

    void testGetIntZero()
    {
        m_provider.setValue("intKey", "0");
        CPPUNIT_ASSERT_EQUAL(0, m_provider.getInt("intKey"));
    }

    void testGetIntWithPlusSign()
    {
        m_provider.setValue("intKey", "+8");
        CPPUNIT_ASSERT_EQUAL(8, m_provider.getInt("intKey"));
    }

    void testGetIntHexAutodetect()
    {
        m_provider.setValue("intKey", "0x10");
        CPPUNIT_ASSERT_EQUAL(16, m_provider.getInt("intKey"));
    }

    void testGetIntOctalAutodetect()
    {
        // With base=0 std::stoi interprets leading 0 as octal -> value 8
        m_provider.setValue("intKey", "010");
        CPPUNIT_ASSERT_EQUAL(8, m_provider.getInt("intKey"));
    }

    void testGetIntMissingKey()
    {
        CPPUNIT_ASSERT_EQUAL(99, m_provider.getInt("missing", 99));
        CPPUNIT_ASSERT_EQUAL(-1, m_provider.getInt("missing")); // default is -1
    }

    void testGetIntInvalidPartial()
    {
        m_provider.setValue("intKey", "12abc");
        CPPUNIT_ASSERT_EQUAL(99, m_provider.getInt("intKey", 99));
    }

    void testGetIntNonNumeric()
    {
        m_provider.setValue("intKey", "abc");
        CPPUNIT_ASSERT_EQUAL(99, m_provider.getInt("intKey", 99));
    }

    void testGetIntEmptyString()
    {
        m_provider.setValue("intKey", "");
        CPPUNIT_ASSERT_EQUAL(99, m_provider.getInt("intKey", 99));
    }

    void testGetIntWhitespaceOnly()
    {
        m_provider.setValue("intKey", "   ");
        CPPUNIT_ASSERT_EQUAL(99, m_provider.getInt("intKey", 99));
    }

    void testGetIntSignOnly()
    {
        m_provider.setValue("intKey1", "+");
        m_provider.setValue("intKey2", "-");
        CPPUNIT_ASSERT_EQUAL(99, m_provider.getInt("intKey1", 99));
        CPPUNIT_ASSERT_EQUAL(99, m_provider.getInt("intKey2", 99));
    }

    void testGetIntOverflow()
    {
        // Test with a number larger than INT_MAX
        m_provider.setValue("intKey", "99999999999999999999");
        CPPUNIT_ASSERT_EQUAL(99, m_provider.getInt("intKey", 99));
    }

    void testGetIntUnderflow()
    {
        // Test with a number smaller than INT_MIN
        m_provider.setValue("intKey", "-99999999999999999999");
        CPPUNIT_ASSERT_EQUAL(99, m_provider.getInt("intKey", 99));
    }

    void testGetArrayBasicSplit()
    {
        m_provider.setValue("arrayKey", "a,b,c");
        std::vector<std::string> result = m_provider.getArray("arrayKey", ",");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), result.size());
        CPPUNIT_ASSERT_EQUAL(std::string("a"), result[0]);
        CPPUNIT_ASSERT_EQUAL(std::string("b"), result[1]);
        CPPUNIT_ASSERT_EQUAL(std::string("c"), result[2]);
    }

    void testGetArraySingleToken()
    {
        m_provider.setValue("arrayKey", "abc");
        std::vector<std::string> result = m_provider.getArray("arrayKey", ",");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), result.size());
        CPPUNIT_ASSERT_EQUAL(std::string("abc"), result[0]);
    }

    void testGetArrayEmptyValue()
    {
        m_provider.setValue("arrayKey", "");
        std::vector<std::string> result = m_provider.getArray("arrayKey", ",");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), result.size());
    }

    void testGetArrayMissingKey()
    {
        std::vector<std::string> result = m_provider.getArray("missing", ",");
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), result.size());
    }

    void testGetArrayMultiCharSeparator()
    {
        m_provider.setValue("arrayKey", "a::b::::c");
        std::vector<std::string> result = m_provider.getArray("arrayKey", "::");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), result.size());
        CPPUNIT_ASSERT_EQUAL(std::string("a"), result[0]);
        CPPUNIT_ASSERT_EQUAL(std::string("b"), result[1]);
        CPPUNIT_ASSERT_EQUAL(std::string("c"), result[2]);
    }

    void testGetArrayConsecutiveSeparators()
    {
        m_provider.setValue("arrayKey", "a,,b");
        std::vector<std::string> result = m_provider.getArray("arrayKey", ",");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result.size());
        CPPUNIT_ASSERT_EQUAL(std::string("a"), result[0]);
        CPPUNIT_ASSERT_EQUAL(std::string("b"), result[1]);
    }

    void testGetArrayLeadingSeparator()
    {
        m_provider.setValue("arrayKey", ",a,b");
        std::vector<std::string> result = m_provider.getArray("arrayKey", ",");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result.size());
        CPPUNIT_ASSERT_EQUAL(std::string("a"), result[0]);
        CPPUNIT_ASSERT_EQUAL(std::string("b"), result[1]);
    }

    void testGetArrayTrailingSeparator()
    {
        m_provider.setValue("arrayKey", "a,b,");
        std::vector<std::string> result = m_provider.getArray("arrayKey", ",");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result.size());
        CPPUNIT_ASSERT_EQUAL(std::string("a"), result[0]);
        CPPUNIT_ASSERT_EQUAL(std::string("b"), result[1]);
    }

    void testGetArrayOnlySeparators()
    {
        m_provider.setValue("arrayKey", ",,,");
        std::vector<std::string> result = m_provider.getArray("arrayKey", ",");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), result.size());
    }

    void testGetArraySeparatorNotFound()
    {
        m_provider.setValue("arrayKey", "abc");
        std::vector<std::string> result = m_provider.getArray("arrayKey", "|");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), result.size());
        CPPUNIT_ASSERT_EQUAL(std::string("abc"), result[0]);
    }

    void testGetArrayEmptySeparator()
    {
        m_provider.setValue("arrayKey", "a,b,c");
        std::vector<std::string> result = m_provider.getArray("arrayKey", "");

        // Should return empty vector and log warning
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), result.size());
    }

    void testGetArrayLargeInput()
    {
        // Build a large comma-separated string
        std::string largeValue;
        for (int i = 0; i < 1000; ++i)
        {
            if (i > 0) largeValue += ",";
            largeValue += "item" + std::to_string(i);
        }

        m_provider.setValue("arrayKey", largeValue);
        std::vector<std::string> result = m_provider.getArray("arrayKey", ",");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1000), result.size());
        CPPUNIT_ASSERT_EQUAL(std::string("item0"), result[0]);
        CPPUNIT_ASSERT_EQUAL(std::string("item999"), result[999]);
    }

    void testGetCstrWithExistingKey()
    {
        m_provider.setValue("testKey", "testValue");
        const char* result = m_provider.getCstr("testKey");
        CPPUNIT_ASSERT(result != nullptr);
        CPPUNIT_ASSERT_EQUAL(std::string("testValue"), std::string(result));
    }

    void testGetCstrWithMissingKey()
    {
        const char* defaultValue = "default";
        const char* result = m_provider.getCstr("missing", defaultValue);
        CPPUNIT_ASSERT_EQUAL(defaultValue, result);
    }

    void testGetCstrWithEmptyString()
    {
        m_provider.setValue("emptyKey", "");
        const char* result = m_provider.getCstr("emptyKey", "default");
        CPPUNIT_ASSERT(result != nullptr);
        CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(result));
    }

    void testGetCstrNullDefault()
    {
        const char* result = m_provider.getCstr("missing", nullptr);
        CPPUNIT_ASSERT(result == nullptr);
    }

    void testMixedTypesInConfig()
    {
        m_provider.setValue("stringVal", "hello");
        m_provider.setValue("intVal", "42");
        m_provider.setValue("arrayVal", "a,b,c");

        // Test that different getters work correctly with same data
        CPPUNIT_ASSERT_EQUAL(std::string("hello"), m_provider.get("stringVal"));
        CPPUNIT_ASSERT_EQUAL(42, m_provider.getInt("intVal"));

        std::vector<std::string> arrayResult = m_provider.getArray("arrayVal", ",");
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), arrayResult.size());

        // Cross-type access
        CPPUNIT_ASSERT_EQUAL(std::string("42"), m_provider.get("intVal"));
        CPPUNIT_ASSERT_EQUAL(0, m_provider.getInt("stringVal", 0)); // Should fall back
    }

    void testBulkRetrieval()
    {
        // Set up multiple keys
        for (int i = 0; i < 100; ++i)
        {
            m_provider.setValue("key" + std::to_string(i), "value" + std::to_string(i));
        }

        // Retrieve all in sequence - test for performance/stability
        for (int i = 0; i < 100; ++i)
        {
            std::string expected = "value" + std::to_string(i);
            CPPUNIT_ASSERT_EQUAL(expected, m_provider.get("key" + std::to_string(i)));
        }
    }

    void testHasValueVsGetDefaultConsistency()
    {
        m_provider.setValue("emptyKey", "");

        // hasValue should return true for empty string
        CPPUNIT_ASSERT(m_provider.hasValue("emptyKey"));

        // get should return the empty string, not the default
        CPPUNIT_ASSERT_EQUAL(std::string(""), m_provider.get("emptyKey", "default"));

        // Missing key should behave differently
        CPPUNIT_ASSERT(!m_provider.hasValue("missingKey"));
        CPPUNIT_ASSERT_EQUAL(std::string("default"), m_provider.get("missingKey", "default"));
    }

    void testArrayWithContentContainingSeparator()
    {
        // Test when content legitimately contains the separator
        m_provider.setValue("urlArray", "http://example.com:8080,https://test.com:443");
        std::vector<std::string> result = m_provider.getArray("urlArray", ",");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result.size());
        CPPUNIT_ASSERT_EQUAL(std::string("http://example.com:8080"), result[0]);
        CPPUNIT_ASSERT_EQUAL(std::string("https://test.com:443"), result[1]);
    }

    void testVeryLongKeyName()
    {
        std::string longKey(1000, 'k');
        m_provider.setValue(longKey, "value");

        CPPUNIT_ASSERT(m_provider.hasValue(longKey));
        CPPUNIT_ASSERT_EQUAL(std::string("value"), m_provider.get(longKey));
    }

    void testSequentialCalls()
    {
        m_provider.setValue("testKey", "42");

        // Multiple sequential calls should return consistent results
        for (int i = 0; i < 10; ++i)
        {
            CPPUNIT_ASSERT(m_provider.hasValue("testKey"));
            CPPUNIT_ASSERT_EQUAL(std::string("42"), m_provider.get("testKey"));
            CPPUNIT_ASSERT_EQUAL(42, m_provider.getInt("testKey"));
            CPPUNIT_ASSERT_EQUAL(std::string("42"), std::string(m_provider.getCstr("testKey")));
        }
    }

    void testConstCorrectnessAllMethods()
    {
        m_provider.setValue("testKey", "value");

        // Create const reference to test const-correctness
        const ConfigProvider& constProvider = m_provider;

        // All these should compile and work with const reference
        CPPUNIT_ASSERT(constProvider.hasValue("testKey"));
        CPPUNIT_ASSERT_EQUAL(std::string("value"), constProvider.get("testKey"));
        CPPUNIT_ASSERT_EQUAL(0, constProvider.getInt("testKey", 0));

        std::vector<std::string> result = constProvider.getArray("testKey", ",");
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), result.size());

        const char* cstrResult = constProvider.getCstr("testKey");
        CPPUNIT_ASSERT_EQUAL(std::string("value"), std::string(cstrResult));
    }

    void testUnicodeKeyAndValue()
    {
        std::string unicodeKey = "キー";
        std::string unicodeValue = "値";

        m_provider.setValue(unicodeKey, unicodeValue);

        CPPUNIT_ASSERT(m_provider.hasValue(unicodeKey));
        CPPUNIT_ASSERT_EQUAL(unicodeValue, m_provider.get(unicodeKey));
    }

    void testSpecialCharactersInValues()
    {
        m_provider.setValue("specialKey", "value\nwith\ttabs\rand\r\nnewlines");
        std::string result = m_provider.get("specialKey");

        CPPUNIT_ASSERT(result.find('\n') != std::string::npos);
        CPPUNIT_ASSERT(result.find('\t') != std::string::npos);
        CPPUNIT_ASSERT(result.find('\r') != std::string::npos);
    }

    void testNullCharacterInValue()
    {
        std::string valueWithNull = "before";
        valueWithNull += '\0';
        valueWithNull += "after";

        m_provider.setValue("nullKey", valueWithNull);

        // C-string interpretation should stop at first null
        std::string result = m_provider.get("nullKey");
        CPPUNIT_ASSERT_EQUAL(std::string("before"), result);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConfigProviderTest);
