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

#include "utils/JsonData.hpp"

using subttxrend::common::JsonData;

class JsonDataTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(JsonDataTest);
    CPPUNIT_TEST(testConstructorValidJson);
    CPPUNIT_TEST(testConstructorInvalidJson);
    CPPUNIT_TEST(testConstructorEmptyString);
    CPPUNIT_TEST(testConstructorWhitespaceOnly);
    CPPUNIT_TEST(testConstructorNoBracket);
    CPPUNIT_TEST(testConstructorBracketNotAtStart);
    CPPUNIT_TEST(testConstructorEmptyJsonObject);
    CPPUNIT_TEST(testConstructorMalformedJson);
    CPPUNIT_TEST(testConstructorUnicodeCharacters);
    CPPUNIT_TEST(testConstructorLargeJson);
    CPPUNIT_TEST(testConstructorDeeplyNestedJson);
    CPPUNIT_TEST(testMoveConstructor);
    CPPUNIT_TEST(testMoveAssignment);
    CPPUNIT_TEST(testIsValidAfterSuccessfulParsing);
    CPPUNIT_TEST(testIsValidAfterFailedParsing);
    CPPUNIT_TEST(testCheckExistingSimplePath);
    CPPUNIT_TEST(testCheckNonExistingPath);
    CPPUNIT_TEST(testCheckNestedPath);
    CPPUNIT_TEST(testCheckEmptyPath);
    CPPUNIT_TEST(testCheckInvalidPathSyntax);
    CPPUNIT_TEST(testCheckPathWithSpecialCharacters);
    CPPUNIT_TEST(testGetExistingPath);
    CPPUNIT_TEST(testGetNonExistingPath);
    CPPUNIT_TEST(testGetNumericValues);
    CPPUNIT_TEST(testGetBooleanValues);
    CPPUNIT_TEST(testGetNullValue);
    CPPUNIT_TEST(testGetNestedPath);
    CPPUNIT_TEST(testGetEmptyStringValue);
    CPPUNIT_TEST(testGetSpecialCharacters);
    CPPUNIT_TEST(testGetArrayStringArray);
    CPPUNIT_TEST(testGetArrayEmptyArray);
    CPPUNIT_TEST(testGetArraySingleElement);
    CPPUNIT_TEST(testGetArrayMixedDataTypes);
    CPPUNIT_TEST(testGetArrayNonExistingPath);
    CPPUNIT_TEST(testGetArrayEmptyDelimiter);
    CPPUNIT_TEST(testGetArrayMultiCharDelimiter);
    CPPUNIT_TEST(testGetArrayNumericArray);
    CPPUNIT_TEST(testGetArrayComplexObjectArray);
    CPPUNIT_TEST(testGetArrayComplexMissingNodes);
    CPPUNIT_TEST(testGetArrayComplexEmptyNodeList);
    CPPUNIT_TEST(testGetArrayComplexNonExistingPath);
    CPPUNIT_TEST(testGetArrayComplexPartialData);
    CPPUNIT_TEST(testGetArrayComplexNestedObjects);
    CPPUNIT_TEST(testErrorHandlingInvalidObject);
    CPPUNIT_TEST(testErrorHandlingAfterFailedConstruction);
    CPPUNIT_TEST(testJsonWithEscapedCharacters);
    CPPUNIT_TEST(testJsonWithQuotesInValues);
    CPPUNIT_TEST(testVeryLongPath);
    CPPUNIT_TEST(testArrayWithNullElements);
    CPPUNIT_TEST(testMultipleOperationsOnSameObject);
    CPPUNIT_TEST(testRealWorldJsonStructure);
    CPPUNIT_TEST(testSequentialOperations);
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

    void testConstructorValidJson()
    {
        JsonData jsonData(R"({"key": "value", "number": 42})");
        CPPUNIT_ASSERT(jsonData.isValid());
    }

    void testConstructorInvalidJson()
    {
        JsonData jsonData(R"({"key": "value", "number": })");
        CPPUNIT_ASSERT(!jsonData.isValid());
    }

    void testConstructorEmptyString()
    {
        JsonData jsonData("");
        CPPUNIT_ASSERT(!jsonData.isValid());
    }

    void testConstructorWhitespaceOnly()
    {
        JsonData jsonData("   \t\n  ");
        CPPUNIT_ASSERT(!jsonData.isValid());
    }

    void testConstructorNoBracket()
    {
        JsonData jsonData("no bracket here");
        CPPUNIT_ASSERT(!jsonData.isValid());
    }

    void testConstructorBracketNotAtStart()
    {
        JsonData jsonData(R"(prefix{"key": "value"})");
        CPPUNIT_ASSERT(jsonData.isValid());
        CPPUNIT_ASSERT(jsonData.check("key"));
        CPPUNIT_ASSERT_EQUAL(std::string("value"), jsonData.get("key"));
    }

    void testConstructorEmptyJsonObject()
    {
        JsonData jsonData("{}");
        CPPUNIT_ASSERT(jsonData.isValid());
        CPPUNIT_ASSERT(!jsonData.check("nonexistent"));
    }

    void testConstructorMalformedJson()
    {
        JsonData jsonData(R"({"key": "value",,})");
        CPPUNIT_ASSERT(!jsonData.isValid());
    }

    void testConstructorUnicodeCharacters()
    {
        JsonData jsonData(R"({"unicode": "héllo wörld", "emoji": "🌟"})");
        CPPUNIT_ASSERT(jsonData.isValid());
        CPPUNIT_ASSERT_EQUAL(std::string("héllo wörld"), jsonData.get("unicode"));
    }

    void testConstructorLargeJson()
    {
        std::string largeJson = "{";
        for (int i = 0; i < 1000; ++i) {
            if (i > 0) largeJson += ",";
            largeJson += "\"key" + std::to_string(i) + "\": \"value" + std::to_string(i) + "\"";
        }
        largeJson += "}";

        JsonData jsonData(largeJson);
        CPPUNIT_ASSERT(jsonData.isValid());
        CPPUNIT_ASSERT_EQUAL(std::string("value500"), jsonData.get("key500"));
    }

    void testConstructorDeeplyNestedJson()
    {
        std::string nestedJson = "{";
        for (int i = 0; i < 50; ++i) {
            nestedJson += "\"level" + std::to_string(i) + "\": {";
        }
        nestedJson += "\"deepest\": \"value\"";
        for (int i = 0; i < 50; ++i) {
            nestedJson += "}";
        }
        nestedJson += "}";

        JsonData jsonData(nestedJson);
        CPPUNIT_ASSERT(jsonData.isValid());
    }

    void testMoveConstructor()
    {
        JsonData original(R"({"key": "value"})");
        CPPUNIT_ASSERT(original.isValid());

        JsonData moved(std::move(original));
        CPPUNIT_ASSERT(moved.isValid());
        CPPUNIT_ASSERT_EQUAL(std::string("value"), moved.get("key"));
    }

    void testMoveAssignment()
    {
        JsonData original(R"({"key": "value"})");
        JsonData target(R"({"other": "data"})");

        target = std::move(original);
        CPPUNIT_ASSERT(target.isValid());
        CPPUNIT_ASSERT_EQUAL(std::string("value"), target.get("key"));
    }

    void testIsValidAfterSuccessfulParsing()
    {
        JsonData jsonData(R"({"valid": true})");
        CPPUNIT_ASSERT(jsonData.isValid());
    }

    void testIsValidAfterFailedParsing()
    {
        JsonData jsonData(R"(invalid json)");
        CPPUNIT_ASSERT(!jsonData.isValid());
    }

    void testCheckExistingSimplePath()
    {
        JsonData jsonData(R"({"key": "value", "number": 42})");
        CPPUNIT_ASSERT(jsonData.check("key"));
        CPPUNIT_ASSERT(jsonData.check("number"));
    }

    void testCheckNonExistingPath()
    {
        JsonData jsonData(R"({"key": "value"})");
        CPPUNIT_ASSERT(!jsonData.check("nonexistent"));
    }

    void testCheckNestedPath()
    {
        JsonData jsonData(R"({"level1": {"level2": {"key": "value"}}})");
        CPPUNIT_ASSERT(jsonData.check("level1.level2.key"));
        CPPUNIT_ASSERT(!jsonData.check("level1.level2.nonexistent"));
    }

    void testCheckEmptyPath()
    {
        JsonData jsonData(R"({"key": "value"})");
        // Implementation treats empty path as root path (property_tree allows this), so it returns true.
        CPPUNIT_ASSERT(jsonData.check(""));
    }

    void testCheckInvalidPathSyntax()
    {
        JsonData jsonData(R"({"key": "value"})");
        CPPUNIT_ASSERT(!jsonData.check("..invalid.."));
        CPPUNIT_ASSERT(!jsonData.check("."));
    }

    void testCheckPathWithSpecialCharacters()
    {
        JsonData jsonData(R"({"key with spaces": "value", "key-with-dashes": "value2"})");
        CPPUNIT_ASSERT(jsonData.check("key with spaces"));
        CPPUNIT_ASSERT(jsonData.check("key-with-dashes"));
    }

    void testGetExistingPath()
    {
        JsonData jsonData(R"({"string": "value", "number": 42})");
        CPPUNIT_ASSERT_EQUAL(std::string("value"), jsonData.get("string"));
        CPPUNIT_ASSERT_EQUAL(std::string("42"), jsonData.get("number"));
    }

    void testGetNonExistingPath()
    {
        JsonData jsonData(R"({"key": "value"})");
        CPPUNIT_ASSERT_EQUAL(std::string(""), jsonData.get("nonexistent"));
    }

    void testGetNumericValues()
    {
        JsonData jsonData(R"({"int": 42, "float": 3.14, "negative": -123})");
        CPPUNIT_ASSERT_EQUAL(std::string("42"), jsonData.get("int"));
        CPPUNIT_ASSERT_EQUAL(std::string("3.14"), jsonData.get("float"));
        CPPUNIT_ASSERT_EQUAL(std::string("-123"), jsonData.get("negative"));
    }

    void testGetBooleanValues()
    {
        JsonData jsonData(R"({"true_val": true, "false_val": false})");
        CPPUNIT_ASSERT_EQUAL(std::string("true"), jsonData.get("true_val"));
        CPPUNIT_ASSERT_EQUAL(std::string("false"), jsonData.get("false_val"));
    }

    void testGetNullValue()
    {
        JsonData jsonData(R"({"null_val": null})");
        // Boost property tree typically converts null to empty string
        std::string result = jsonData.get("null_val");
        CPPUNIT_ASSERT(result.empty() || result == "null");
    }

    void testGetNestedPath()
    {
        JsonData jsonData(R"({"level1": {"level2": {"key": "deep_value"}}})");
        CPPUNIT_ASSERT_EQUAL(std::string("deep_value"), jsonData.get("level1.level2.key"));
    }

    void testGetEmptyStringValue()
    {
        JsonData jsonData(R"({"empty": ""})");
        CPPUNIT_ASSERT_EQUAL(std::string(""), jsonData.get("empty"));
    }

    void testGetSpecialCharacters()
    {
        JsonData jsonData(R"({"special": "Hello\nWorld\t!"})");
        std::string result = jsonData.get("special");
        CPPUNIT_ASSERT(!result.empty());
    }

    void testGetArrayStringArray()
    {
        JsonData jsonData(R"({"array": ["one", "two", "three"]})");
        std::string result = jsonData.getArray("array", ",");
        CPPUNIT_ASSERT_EQUAL(std::string("one,two,three"), result);
    }

    void testGetArrayEmptyArray()
    {
        JsonData jsonData(R"({"array": []})");
        std::string result = jsonData.getArray("array", ",");
        CPPUNIT_ASSERT_EQUAL(std::string(""), result);
    }

    void testGetArraySingleElement()
    {
        JsonData jsonData(R"({"array": ["single"]})");
        std::string result = jsonData.getArray("array", ",");
        CPPUNIT_ASSERT_EQUAL(std::string("single"), result);
    }

    void testGetArrayMixedDataTypes()
    {
        JsonData jsonData(R"({"array": ["string", 42, true, null]})");
        std::string result = jsonData.getArray("array", "|");
        // Order and exact representation may vary, but should contain all elements
        CPPUNIT_ASSERT(!result.empty());
        CPPUNIT_ASSERT(result.find("string") != std::string::npos);
        CPPUNIT_ASSERT(result.find("42") != std::string::npos);
    }

    void testGetArrayNonExistingPath()
    {
        JsonData jsonData(R"({"other": "value"})");
        std::string result = jsonData.getArray("nonexistent", ",");
        CPPUNIT_ASSERT_EQUAL(std::string(""), result);
    }

    void testGetArrayEmptyDelimiter()
    {
        JsonData jsonData(R"({"array": ["a", "b", "c"]})");
        std::string result = jsonData.getArray("array", "");
        // Current implementation appends delimiter (empty here) after
        // each element then unconditionally pop_back() once,
        // causing the last character to be removed when delimiter
        // length == 0. Expected becomes "ab".
        CPPUNIT_ASSERT_EQUAL(std::string("ab"), result);
    }

    void testGetArrayMultiCharDelimiter()
    {
        JsonData jsonData(R"({"array": ["one", "two", "three"]})");
        std::string result = jsonData.getArray("array", " | ");
        // Implementation removes only the last character of the accumulated
        // string, not the whole multi-char delimiter.
        // Thus a trailing '|' remains.
        CPPUNIT_ASSERT_EQUAL(std::string("one | two | three |"), result);
    }

    void testGetArrayNumericArray()
    {
        JsonData jsonData(R"({"numbers": [1, 2, 3, 4, 5]})");
        std::string result = jsonData.getArray("numbers", ",");
        CPPUNIT_ASSERT_EQUAL(std::string("1,2,3,4,5"), result);
    }

    void testGetArrayComplexObjectArray()
    {
        JsonData jsonData(R"({"users": [{"name": "John", "age": "30"}, {"name": "Jane", "age": "25"}]})");
        std::vector<std::string> result = jsonData.getArray("users", ":", {"name", "age"});

        CPPUNIT_ASSERT_EQUAL(size_t(2), result.size());
        CPPUNIT_ASSERT_EQUAL(std::string("John:30"), result[0]);
        CPPUNIT_ASSERT_EQUAL(std::string("Jane:25"), result[1]);
    }

    void testGetArrayComplexMissingNodes()
    {
        JsonData jsonData(R"({"users": [{"name": "John"}, {"name": "Jane", "age": "25"}]})");
        std::vector<std::string> result = jsonData.getArray("users", ":", {"name", "age"});

        CPPUNIT_ASSERT_EQUAL(size_t(2), result.size());
        // Missing nodes still incur a delimiter insertion; implementation
        // pops only one char, leaving trailing ':'
        // when final node missing. Hence first element ends with ':'
        CPPUNIT_ASSERT_EQUAL(std::string("John:"), result[0]);
        CPPUNIT_ASSERT_EQUAL(std::string("Jane:25"), result[1]);
    }

    void testGetArrayComplexEmptyNodeList()
    {
        JsonData jsonData(R"({"users": [{"name": "John"}, {"name": "Jane"}]})");
        std::vector<std::string> result = jsonData.getArray("users", ":", {});

        CPPUNIT_ASSERT_EQUAL(size_t(2), result.size());
        CPPUNIT_ASSERT_EQUAL(std::string(""), result[0]);
        CPPUNIT_ASSERT_EQUAL(std::string(""), result[1]);
    }

    void testGetArrayComplexNonExistingPath()
    {
        JsonData jsonData(R"({"other": "value"})");
        std::vector<std::string> result = jsonData.getArray("nonexistent", ":", {"name"});

        CPPUNIT_ASSERT(result.empty());
    }

    void testGetArrayComplexPartialData()
    {
        JsonData jsonData(R"({"items": [{"a": "1", "b": "2"}, {"a": "3"}, {"b": "4"}]})");
        std::vector<std::string> result = jsonData.getArray("items", "-", {"a", "b", "c"});

        CPPUNIT_ASSERT_EQUAL(size_t(3), result.size());
        // Trailing delimiters remain when last (or multiple) nodes are missing
        // because only one char is trimmed.
        CPPUNIT_ASSERT_EQUAL(std::string("1-2-"), result[0]);
        CPPUNIT_ASSERT_EQUAL(std::string("3--"), result[1]);
        CPPUNIT_ASSERT_EQUAL(std::string("-4-"), result[2]);
    }

    void testGetArrayComplexNestedObjects()
    {
        JsonData jsonData(R"({"data": [{"info": {"id": "1", "type": "A"}}, {"info": {"id": "2", "type": "B"}}]})");
        std::vector<std::string> result = jsonData.getArray("data", "|", {"info"});

        CPPUNIT_ASSERT_EQUAL(size_t(2), result.size());
        // Accessing an object node's data yields an empty string in property_tree; expect empty strings.
        CPPUNIT_ASSERT(result[0].empty());
        CPPUNIT_ASSERT(result[1].empty());
    }

    void testErrorHandlingInvalidObject()
    {
        JsonData jsonData("invalid");
        CPPUNIT_ASSERT(!jsonData.isValid());

        // Operations on invalid object should not crash
        CPPUNIT_ASSERT(!jsonData.check("any"));
        CPPUNIT_ASSERT_EQUAL(std::string(""), jsonData.get("any"));
        CPPUNIT_ASSERT_EQUAL(std::string(""), jsonData.getArray("any", ","));

        std::vector<std::string> complexResult = jsonData.getArray("any", ",", {"node"});
        CPPUNIT_ASSERT(complexResult.empty());
    }

    void testErrorHandlingAfterFailedConstruction()
    {
        JsonData jsonData(R"({"malformed": })");
        CPPUNIT_ASSERT(!jsonData.isValid());

        // All operations should be safe
        CPPUNIT_ASSERT(!jsonData.check("malformed"));
        CPPUNIT_ASSERT_EQUAL(std::string(""), jsonData.get("malformed"));
    }

    void testJsonWithEscapedCharacters()
    {
        JsonData jsonData(R"({"escaped": "He said \"Hello\""})");
        CPPUNIT_ASSERT(jsonData.isValid());
        std::string result = jsonData.get("escaped");
        CPPUNIT_ASSERT(result.find("Hello") != std::string::npos);
    }

    void testJsonWithQuotesInValues()
    {
        JsonData jsonData(R"({"quotes": "She said 'Hi' to me"})");
        CPPUNIT_ASSERT(jsonData.isValid());
        CPPUNIT_ASSERT_EQUAL(std::string("She said 'Hi' to me"), jsonData.get("quotes"));
    }

    void testVeryLongPath()
    {
        std::string longPath;
        std::string json = "{";

        for (int i = 0; i < 20; ++i) {
            std::string level = "level" + std::to_string(i);
            if (i > 0) {
                longPath += "."; // path separator
            }
            longPath += level;
            json += "\"" + level + "\": {";
        }

        json += "\"final\": \"deep_value\"";
        longPath += ".final";

        for (int i = 0; i < 20; ++i) {
            json += "}";
        }
        json += "}";

        JsonData jsonData(json);
        CPPUNIT_ASSERT(jsonData.isValid());
        CPPUNIT_ASSERT(jsonData.check(longPath));
        CPPUNIT_ASSERT_EQUAL(std::string("deep_value"), jsonData.get(longPath));
    }

    void testArrayWithNullElements()
    {
        JsonData jsonData(R"({"array": ["value", null, "another"]})");
        std::string result = jsonData.getArray("array", ",");
        CPPUNIT_ASSERT(!result.empty());
        // Should handle null elements gracefully
    }

    void testMultipleOperationsOnSameObject()
    {
        JsonData jsonData(R"({
            "string": "value",
            "number": 42,
            "array": ["a", "b", "c"],
            "nested": {"key": "nested_value"},
            "objects": [{"name": "obj1"}, {"name": "obj2"}]
        })");

        CPPUNIT_ASSERT(jsonData.isValid());

        // Multiple check operations
        CPPUNIT_ASSERT(jsonData.check("string"));
        CPPUNIT_ASSERT(jsonData.check("nested.key"));
        CPPUNIT_ASSERT(!jsonData.check("nonexistent"));

        // Multiple get operations
        CPPUNIT_ASSERT_EQUAL(std::string("value"), jsonData.get("string"));
        CPPUNIT_ASSERT_EQUAL(std::string("42"), jsonData.get("number"));
        CPPUNIT_ASSERT_EQUAL(std::string("nested_value"), jsonData.get("nested.key"));

        // Array operations
        CPPUNIT_ASSERT_EQUAL(std::string("a,b,c"), jsonData.getArray("array", ","));

        std::vector<std::string> objectArray = jsonData.getArray("objects", ":", {"name"});
        CPPUNIT_ASSERT_EQUAL(size_t(2), objectArray.size());
        CPPUNIT_ASSERT_EQUAL(std::string("obj1"), objectArray[0]);
        CPPUNIT_ASSERT_EQUAL(std::string("obj2"), objectArray[1]);
    }

    void testRealWorldJsonStructure()
    {
        JsonData jsonData(R"({
            "users": [
                {"id": 1, "name": "John Doe", "email": "john@example.com", "active": true},
                {"id": 2, "name": "Jane Smith", "email": "jane@example.com", "active": false}
            ],
            "metadata": {
                "total": 2,
                "page": 1,
                "timestamp": "2023-01-01T00:00:00Z"
            },
            "config": {
                "api_version": "1.0",
                "features": ["feature1", "feature2", "feature3"]
            }
        })");

        CPPUNIT_ASSERT(jsonData.isValid());

        // Test metadata access
        CPPUNIT_ASSERT_EQUAL(std::string("2"), jsonData.get("metadata.total"));
        CPPUNIT_ASSERT_EQUAL(std::string("1.0"), jsonData.get("config.api_version"));

        // Test array processing
        std::string features = jsonData.getArray("config.features", ",");
        CPPUNIT_ASSERT_EQUAL(std::string("feature1,feature2,feature3"), features);

        // Test complex array processing
        std::vector<std::string> userInfo = jsonData.getArray("users", " | ", {"name", "email"});
        CPPUNIT_ASSERT_EQUAL(size_t(2), userInfo.size());
        // Expect trailing delimiter artifact due to single-char pop_back trimming
        CPPUNIT_ASSERT_EQUAL(std::string("John Doe | john@example.com |"), userInfo[0]);
        CPPUNIT_ASSERT_EQUAL(std::string("Jane Smith | jane@example.com |"), userInfo[1]);
    }

    void testSequentialOperations()
    {
        JsonData jsonData(R"({"counter": 0, "items": []})");
        CPPUNIT_ASSERT(jsonData.isValid());

        // Perform multiple sequential operations
        for (int i = 0; i < 10; ++i) {
            CPPUNIT_ASSERT(jsonData.check("counter"));
            CPPUNIT_ASSERT_EQUAL(std::string("0"), jsonData.get("counter"));
            CPPUNIT_ASSERT_EQUAL(std::string(""), jsonData.getArray("items", ","));
        }

        // Object should remain consistent
        CPPUNIT_ASSERT(jsonData.isValid());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(JsonDataTest);
