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

#include <array>

#include "Array.hpp"

class ArrayTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ArrayTest );

    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testConstructorDefaultState);
    CPPUNIT_TEST(testGetDataNonConst);
    CPPUNIT_TEST(testGetSizeVariousSizes);
    CPPUNIT_TEST(testDifferentDataTypes);
    CPPUNIT_TEST(testPolymorphicUsage);
    CPPUNIT_TEST(testConstCorrectness);
    CPPUNIT_TEST(testZeroSizedArray);
    CPPUNIT_TEST(testSingleElementArray);
    CPPUNIT_TEST(testLargeArrays);
    CPPUNIT_TEST(testComplexDataTypes);
    CPPUNIT_TEST(testDataIntegrity);
    CPPUNIT_TEST(testMemoryLayout);
    CPPUNIT_TEST(testMultipleInstances);
    CPPUNIT_TEST(testVirtualMethodDispatch);
    CPPUNIT_TEST(testTemplateEdgeCases);
    CPPUNIT_TEST(testPointerTypes);
    CPPUNIT_TEST(testFloatingPointTypes);
    CPPUNIT_TEST(testBooleanType);
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

    void testSimple()
    {
        const std::size_t ARRAY_SIZE = 32;

        dvbsubdecoder::SizedArray<std::uint16_t, ARRAY_SIZE> array;
        dvbsubdecoder::Array<std::uint16_t>& baseArray = array;
        const dvbsubdecoder::SizedArray<std::uint16_t, ARRAY_SIZE>& constArray =
                array;
        const dvbsubdecoder::Array<std::uint16_t>& constBaseArray = array;

        CPPUNIT_ASSERT(array.getData());
        CPPUNIT_ASSERT(array.getData() == baseArray.getData());
        CPPUNIT_ASSERT(array.getData() == constArray.getData());
        CPPUNIT_ASSERT(array.getData() == constBaseArray.getData());

        CPPUNIT_ASSERT(array.getSize() == ARRAY_SIZE);
        CPPUNIT_ASSERT(baseArray.getSize() == ARRAY_SIZE);
        CPPUNIT_ASSERT(constArray.getSize() == ARRAY_SIZE);
        CPPUNIT_ASSERT(constBaseArray.getSize() == ARRAY_SIZE);
    }

    // Basic API coverage tests
    void testConstructorDefaultState()
    {
        // Test default constructor with different template parameters
        dvbsubdecoder::SizedArray<int, 10> intArray;
        dvbsubdecoder::SizedArray<float, 5> floatArray;
        dvbsubdecoder::SizedArray<char, 1> charArray;
        
        // Verify initial state - arrays should be properly constructed
        CPPUNIT_ASSERT(intArray.getData() != nullptr);
        CPPUNIT_ASSERT(floatArray.getData() != nullptr);
        CPPUNIT_ASSERT(charArray.getData() != nullptr);
        
        CPPUNIT_ASSERT(intArray.getSize() == 10);
        CPPUNIT_ASSERT(floatArray.getSize() == 5);
        CPPUNIT_ASSERT(charArray.getSize() == 1);
    }

    void testGetDataNonConst()
    {
        dvbsubdecoder::SizedArray<int, 8> array;
        
        // Test non-const getData() method
        int* data = array.getData();
        CPPUNIT_ASSERT(data != nullptr);
        
        // Verify we can write through the pointer
        data[0] = 42;
        data[7] = 99;
        
        // Verify the data was written correctly
        CPPUNIT_ASSERT(array.getData()[0] == 42);
        CPPUNIT_ASSERT(array.getData()[7] == 99);
    }

    void testGetSizeVariousSizes()
    {
        // Test various array sizes
        dvbsubdecoder::SizedArray<char, 1> size1;
        dvbsubdecoder::SizedArray<int, 16> size16;
        dvbsubdecoder::SizedArray<double, 100> size100;
        dvbsubdecoder::SizedArray<long, 1024> size1024;
        
        CPPUNIT_ASSERT(size1.getSize() == 1);
        CPPUNIT_ASSERT(size16.getSize() == 16);
        CPPUNIT_ASSERT(size100.getSize() == 100);
        CPPUNIT_ASSERT(size1024.getSize() == 1024);
    }

    void testDifferentDataTypes()
    {
        // Test various data types
        dvbsubdecoder::SizedArray<std::int8_t, 4> int8Array;
        dvbsubdecoder::SizedArray<std::int32_t, 4> int32Array;
        dvbsubdecoder::SizedArray<std::int64_t, 4> int64Array;
        dvbsubdecoder::SizedArray<float, 4> floatArray;
        dvbsubdecoder::SizedArray<double, 4> doubleArray;
        
        // Verify all types work correctly
        CPPUNIT_ASSERT(int8Array.getData() != nullptr);
        CPPUNIT_ASSERT(int32Array.getData() != nullptr);
        CPPUNIT_ASSERT(int64Array.getData() != nullptr);
        CPPUNIT_ASSERT(floatArray.getData() != nullptr);
        CPPUNIT_ASSERT(doubleArray.getData() != nullptr);
        
        CPPUNIT_ASSERT(int8Array.getSize() == 4);
        CPPUNIT_ASSERT(int32Array.getSize() == 4);
        CPPUNIT_ASSERT(int64Array.getSize() == 4);
        CPPUNIT_ASSERT(floatArray.getSize() == 4);
        CPPUNIT_ASSERT(doubleArray.getSize() == 4);
    }

    void testPolymorphicUsage()
    {
        dvbsubdecoder::SizedArray<int, 12> concreteArray;
        dvbsubdecoder::Array<int>& baseRef = concreteArray;
        dvbsubdecoder::Array<int>* basePtr = &concreteArray;
        
        // Test polymorphic access through base class interface
        CPPUNIT_ASSERT(baseRef.getData() == concreteArray.getData());
        CPPUNIT_ASSERT(baseRef.getSize() == concreteArray.getSize());
        
        CPPUNIT_ASSERT(basePtr->getData() == concreteArray.getData());
        CPPUNIT_ASSERT(basePtr->getSize() == concreteArray.getSize());
        
        // Test const polymorphic access
        const dvbsubdecoder::Array<int>& constBaseRef = concreteArray;
        CPPUNIT_ASSERT(constBaseRef.getData() == concreteArray.getData());
        CPPUNIT_ASSERT(constBaseRef.getSize() == concreteArray.getSize());
    }

    void testConstCorrectness()
    {
        dvbsubdecoder::SizedArray<int, 3> array;
        const dvbsubdecoder::SizedArray<int, 3>& constArray = array;
        
        // Write through non-const interface
        int* nonConstData = array.getData();
        nonConstData[0] = 111;
        nonConstData[1] = 222;
        nonConstData[2] = 333;
        
        // Read through const interface
        const int* constData = constArray.getData();
        CPPUNIT_ASSERT(constData[0] == 111);
        CPPUNIT_ASSERT(constData[1] == 222);
        CPPUNIT_ASSERT(constData[2] == 333);
        
        // Verify const methods return correct size
        CPPUNIT_ASSERT(constArray.getSize() == 3);
        
        // Verify polymorphic const access
        const dvbsubdecoder::Array<int>& constBase = array;
        CPPUNIT_ASSERT(constBase.getSize() == 3);
        CPPUNIT_ASSERT(constBase.getData() == constData);
    }

    // Edge cases and error handling tests
    void testZeroSizedArray()
    {
        // Test zero-sized array edge case
        dvbsubdecoder::SizedArray<int, 0> zeroArray;
        
        CPPUNIT_ASSERT(zeroArray.getSize() == 0);
        // Just verify it's callable.
        int* data = zeroArray.getData();
        (void)data; // Suppress unused variable warning
        
        // Test polymorphic access with zero size
        dvbsubdecoder::Array<int>& baseRef = zeroArray;
        CPPUNIT_ASSERT(baseRef.getSize() == 0);
        CPPUNIT_ASSERT(baseRef.getData() == zeroArray.getData());
    }

    void testSingleElementArray()
    {
        // Test single element array
        dvbsubdecoder::SizedArray<double, 1> singleArray;
        
        CPPUNIT_ASSERT(singleArray.getSize() == 1);
        CPPUNIT_ASSERT(singleArray.getData() != nullptr);
        
        // Test data access with single element
        double* data = singleArray.getData();
        data[0] = 3.14159;
        CPPUNIT_ASSERT(data[0] == 3.14159);
        
        // Test const access
        const dvbsubdecoder::SizedArray<double, 1>& constRef = singleArray;
        CPPUNIT_ASSERT(constRef.getData()[0] == 3.14159);
    }

    void testLargeArrays()
    {
        // Test large array sizes
        dvbsubdecoder::SizedArray<char, 4096> largeArray;
        dvbsubdecoder::SizedArray<int, 1000> mediumArray;
        
        CPPUNIT_ASSERT(largeArray.getSize() == 4096);
        CPPUNIT_ASSERT(mediumArray.getSize() == 1000);
        
        CPPUNIT_ASSERT(largeArray.getData() != nullptr);
        CPPUNIT_ASSERT(mediumArray.getData() != nullptr);
        
        // Test writing to boundary elements
        char* largeData = largeArray.getData();
        largeData[0] = 'A';
        largeData[4095] = 'Z';
        
        int* mediumData = mediumArray.getData();
        mediumData[0] = 1;
        mediumData[999] = 999;
        
        // Verify boundary writes
        CPPUNIT_ASSERT(largeData[0] == 'A');
        CPPUNIT_ASSERT(largeData[4095] == 'Z');
        CPPUNIT_ASSERT(mediumData[0] == 1);
        CPPUNIT_ASSERT(mediumData[999] == 999);
    }

    void testComplexDataTypes()
    {
        // Test with struct as data type
        struct TestStruct {
            int x;
            float y;
            char z;
            
            TestStruct() : x(0), y(0.0f), z(0) {}
            TestStruct(int a, float b, char c) : x(a), y(b), z(c) {}
            
            bool operator==(const TestStruct& other) const {
                return x == other.x && y == other.y && z == other.z;
            }
        };
        
        dvbsubdecoder::SizedArray<TestStruct, 3> structArray;
        
        CPPUNIT_ASSERT(structArray.getSize() == 3);
        CPPUNIT_ASSERT(structArray.getData() != nullptr);
        
        // Test writing complex data
        TestStruct* data = structArray.getData();
        data[0] = TestStruct(10, 1.5f, 'A');
        data[1] = TestStruct(20, 2.5f, 'B');
        data[2] = TestStruct(30, 3.5f, 'C');
        
        // Verify complex data
        CPPUNIT_ASSERT(data[0] == TestStruct(10, 1.5f, 'A'));
        CPPUNIT_ASSERT(data[1] == TestStruct(20, 2.5f, 'B'));
        CPPUNIT_ASSERT(data[2] == TestStruct(30, 3.5f, 'C'));
    }

    void testDataIntegrity()
    {
        dvbsubdecoder::SizedArray<long, 8> array;
        
        // Fill array with test pattern
        long* data = array.getData();
        for (std::size_t i = 0; i < array.getSize(); ++i) {
            data[i] = static_cast<long>(i * i + 100);
        }
        
        // Verify data integrity through different access methods
        dvbsubdecoder::Array<long>& baseRef = array;
        const dvbsubdecoder::SizedArray<long, 8>& constRef = array;
        
        for (std::size_t i = 0; i < 8; ++i) {
            long expected = static_cast<long>(i * i + 100);
            CPPUNIT_ASSERT(array.getData()[i] == expected);
            CPPUNIT_ASSERT(baseRef.getData()[i] == expected);
            CPPUNIT_ASSERT(constRef.getData()[i] == expected);
        }
    }

    void testMemoryLayout()
    {
        dvbsubdecoder::SizedArray<int, 4> array;
        
        int* data = array.getData();
        
        // Test that elements are contiguous in memory
        data[0] = 100;
        data[1] = 200;
        data[2] = 300;
        data[3] = 400;
        
        // Verify pointer arithmetic works as expected for contiguous memory
        CPPUNIT_ASSERT(*(data + 0) == 100);
        CPPUNIT_ASSERT(*(data + 1) == 200);
        CPPUNIT_ASSERT(*(data + 2) == 300);
        CPPUNIT_ASSERT(*(data + 3) == 400);
        
        // Test address differences
        CPPUNIT_ASSERT((data + 1) - data == 1);
        CPPUNIT_ASSERT((data + 3) - data == 3);
    }

    void testMultipleInstances()
    {
        // Test independence of multiple array instances
        dvbsubdecoder::SizedArray<int, 4> array1;
        dvbsubdecoder::SizedArray<int, 4> array2;
        dvbsubdecoder::SizedArray<int, 4> array3;
        
        // Verify different instances have different data pointers
        CPPUNIT_ASSERT(array1.getData() != array2.getData());
        CPPUNIT_ASSERT(array2.getData() != array3.getData());
        CPPUNIT_ASSERT(array1.getData() != array3.getData());
        
        // Verify instance independence
        array1.getData()[0] = 111;
        array2.getData()[0] = 222;
        array3.getData()[0] = 333;
        
        CPPUNIT_ASSERT(array1.getData()[0] == 111);
        CPPUNIT_ASSERT(array2.getData()[0] == 222);
        CPPUNIT_ASSERT(array3.getData()[0] == 333);
        
        // Verify sizes are consistent
        CPPUNIT_ASSERT(array1.getSize() == 4);
        CPPUNIT_ASSERT(array2.getSize() == 4);
        CPPUNIT_ASSERT(array3.getSize() == 4);
    }

    void testVirtualMethodDispatch()
    {
        dvbsubdecoder::SizedArray<short, 6> concreteArray;
        
        // Test virtual method dispatch through base class pointer
        dvbsubdecoder::Array<short>* basePtr = &concreteArray;
        
        // Verify virtual methods work correctly
        CPPUNIT_ASSERT(basePtr->getSize() == 6);
        CPPUNIT_ASSERT(basePtr->getData() == concreteArray.getData());
        
        // Test data access through virtual interface
        short* directData = concreteArray.getData();
        short* virtualData = basePtr->getData();
        
        CPPUNIT_ASSERT(directData == virtualData);
        
        // Test writing through virtual interface
        virtualData[0] = 777;
        virtualData[5] = 888;
        
        CPPUNIT_ASSERT(directData[0] == 777);
        CPPUNIT_ASSERT(directData[5] == 888);
        
        // Test const virtual methods
        const dvbsubdecoder::Array<short>* constBasePtr = &concreteArray;
        CPPUNIT_ASSERT(constBasePtr->getSize() == 6);
        CPPUNIT_ASSERT(constBasePtr->getData()[0] == 777);
        CPPUNIT_ASSERT(constBasePtr->getData()[5] == 888);
    }

    void testTemplateEdgeCases()
    {
        // Test various template parameter combinations
        dvbsubdecoder::SizedArray<unsigned char, 255> maxCharArray;
        dvbsubdecoder::SizedArray<std::int64_t, 2> int64Array;
        dvbsubdecoder::SizedArray<void*, 8> pointerArray;
        
        CPPUNIT_ASSERT(maxCharArray.getSize() == 255);
        CPPUNIT_ASSERT(int64Array.getSize() == 2);
        CPPUNIT_ASSERT(pointerArray.getSize() == 8);
        
        CPPUNIT_ASSERT(maxCharArray.getData() != nullptr);
        CPPUNIT_ASSERT(int64Array.getData() != nullptr);
        CPPUNIT_ASSERT(pointerArray.getData() != nullptr);
        
        // Test boundary element access
        maxCharArray.getData()[0] = 1;
        maxCharArray.getData()[254] = 254;
        
        int64Array.getData()[0] = INT64_MIN;
        int64Array.getData()[1] = INT64_MAX;
        
        // Verify boundary values
        CPPUNIT_ASSERT(maxCharArray.getData()[0] == 1);
        CPPUNIT_ASSERT(maxCharArray.getData()[254] == 254);
        CPPUNIT_ASSERT(int64Array.getData()[0] == INT64_MIN);
        CPPUNIT_ASSERT(int64Array.getData()[1] == INT64_MAX);
    }

    void testPointerTypes()
    {
        // Test array of pointers
        dvbsubdecoder::SizedArray<int*, 4> ptrArray;
        
        CPPUNIT_ASSERT(ptrArray.getSize() == 4);
        CPPUNIT_ASSERT(ptrArray.getData() != nullptr);
        
        // Test storing and retrieving pointers
        int value1 = 100, value2 = 200, value3 = 300;
        int** ptrData = ptrArray.getData();
        
        ptrData[0] = &value1;
        ptrData[1] = &value2;
        ptrData[2] = &value3;
        ptrData[3] = nullptr;
        
        CPPUNIT_ASSERT(*ptrData[0] == 100);
        CPPUNIT_ASSERT(*ptrData[1] == 200);
        CPPUNIT_ASSERT(*ptrData[2] == 300);
        CPPUNIT_ASSERT(ptrData[3] == nullptr);
    }

    void testFloatingPointTypes()
    {
        // Test floating point precision
        dvbsubdecoder::SizedArray<float, 3> floatArray;
        dvbsubdecoder::SizedArray<double, 3> doubleArray;
        
        float* floatData = floatArray.getData();
        double* doubleData = doubleArray.getData();
        
        // Test precision values
        floatData[0] = 3.14159f;
        floatData[1] = -2.71828f;
        floatData[2] = 1.41421f;
        
        doubleData[0] = 3.141592653589793;
        doubleData[1] = -2.718281828459045;
        doubleData[2] = 1.414213562373095;
        
        // Verify floating point values (with appropriate tolerance)
        CPPUNIT_ASSERT(std::abs(floatData[0] - 3.14159f) < 0.00001f);
        CPPUNIT_ASSERT(std::abs(floatData[1] - (-2.71828f)) < 0.00001f);
        CPPUNIT_ASSERT(std::abs(floatData[2] - 1.41421f) < 0.00001f);
        
        CPPUNIT_ASSERT(std::abs(doubleData[0] - 3.141592653589793) < 0.000000000000001);
        CPPUNIT_ASSERT(std::abs(doubleData[1] - (-2.718281828459045)) < 0.000000000000001);
        CPPUNIT_ASSERT(std::abs(doubleData[2] - 1.414213562373095) < 0.000000000000001);
    }

    void testBooleanType()
    {
        // Test boolean array
        dvbsubdecoder::SizedArray<bool, 8> boolArray;
        
        CPPUNIT_ASSERT(boolArray.getSize() == 8);
        CPPUNIT_ASSERT(boolArray.getData() != nullptr);
        
        bool* boolData = boolArray.getData();
        
        // Test boolean values
        boolData[0] = true;
        boolData[1] = false;
        boolData[2] = true;
        boolData[3] = false;
        boolData[4] = true;
        boolData[5] = true;
        boolData[6] = false;
        boolData[7] = false;
        
        // Verify boolean values
        CPPUNIT_ASSERT(boolData[0] == true);
        CPPUNIT_ASSERT(boolData[1] == false);
        CPPUNIT_ASSERT(boolData[2] == true);
        CPPUNIT_ASSERT(boolData[3] == false);
        CPPUNIT_ASSERT(boolData[4] == true);
        CPPUNIT_ASSERT(boolData[5] == true);
        CPPUNIT_ASSERT(boolData[6] == false);
        CPPUNIT_ASSERT(boolData[7] == false);
        
        // Test const access
        const dvbsubdecoder::SizedArray<bool, 8>& constBoolArray = boolArray;
        CPPUNIT_ASSERT(constBoolArray.getData()[0] == true);
        CPPUNIT_ASSERT(constBoolArray.getData()[7] == false);
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ArrayTest);
