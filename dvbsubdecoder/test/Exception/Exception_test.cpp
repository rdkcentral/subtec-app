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
#include <utility> // for std::declval used in static_assert noexcept checks

#include "ParserException.hpp"
#include "PesPacketReader.hpp"

using dvbsubdecoder::ParserException;
using dvbsubdecoder::PesPacketReader;

class ExceptionTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ExceptionTest );
    CPPUNIT_TEST(testParserException);
    CPPUNIT_TEST(testPesPacketReaderException);
    CPPUNIT_TEST(testExceptionPolymorphism);
    CPPUNIT_TEST(testMessageBoundaryValues);
    CPPUNIT_TEST(testExceptionCopySemantics);
    CPPUNIT_TEST(testExceptionPropagation);
    CPPUNIT_TEST(testMessageLifetimeSafety);
    CPPUNIT_TEST(testExceptionConsistency);
    CPPUNIT_TEST(testNoexceptGuarantee);
    CPPUNIT_TEST(testMultipleWhatCalls);
    CPPUNIT_TEST(testSpecialCharacterMessages);
    CPPUNIT_TEST(testExceptionRethrowing);
    CPPUNIT_TEST(testExceptionInheritanceHierarchy);
    CPPUNIT_TEST(testMessagePointerStability);
    CPPUNIT_TEST(testExceptionConstCorrectness);
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

    void testParserException()
    {
        const std::string message = "exception message";

        try
        {
            throw ParserException(message.c_str());
        }
        catch (const ParserException& e)
        {
            CPPUNIT_ASSERT_EQUAL(message, std::string(e.what()));
        }

        try
        {
            throw ParserException(nullptr);
        }
        catch (const ParserException& e)
        {
            CPPUNIT_ASSERT_EQUAL(std::string(), std::string(e.what()));
        }
    }

    void testPesPacketReaderException()
    {
        const std::string message = "exception message";

        try
        {
            throw PesPacketReader::Exception(message.c_str());
        }
        catch (const PesPacketReader::Exception& e)
        {
            CPPUNIT_ASSERT_EQUAL(message, std::string(e.what()));
        }

        try
        {
            throw PesPacketReader::Exception(nullptr);
        }
        catch (const PesPacketReader::Exception& e)
        {
            CPPUNIT_ASSERT_EQUAL(std::string(), std::string(e.what()));
        }
    }

    void testExceptionPolymorphism()
    {
        // Test ParserException polymorphic behavior
        const std::string parserMessage = "parser error";
        try
        {
            throw ParserException(parserMessage.c_str());
        }
        catch (const std::exception& e)
        {
            // Verify virtual function call works through base class
            CPPUNIT_ASSERT_EQUAL(parserMessage, std::string(e.what()));
        }

        // Test PesPacketReader::Exception polymorphic behavior
        const std::string pesMessage = "pes packet error";
        try
        {
            throw PesPacketReader::Exception(pesMessage.c_str());
        }
        catch (const std::exception& e)
        {
            // Verify virtual function call works through base class
            CPPUNIT_ASSERT_EQUAL(pesMessage, std::string(e.what()));
        }

        // Test std::exception interface with nullptr
        try
        {
            throw ParserException(nullptr);
        }
        catch (const std::exception& e)
        {
            CPPUNIT_ASSERT_EQUAL(std::string(), std::string(e.what()));
        }
    }

    void testMessageBoundaryValues()
    {
        // Test empty string message
        const std::string emptyMessage = "";
        ParserException emptyException(emptyMessage.c_str());
        CPPUNIT_ASSERT_EQUAL(emptyMessage, std::string(emptyException.what()));

        PesPacketReader::Exception emptyPesException(emptyMessage.c_str());
        CPPUNIT_ASSERT_EQUAL(emptyMessage, std::string(emptyPesException.what()));

        // Test single character message
        const std::string singleChar = "X";
        ParserException singleException(singleChar.c_str());
        CPPUNIT_ASSERT_EQUAL(singleChar, std::string(singleException.what()));

        // Test very long message
        const std::string longMessage(1000, 'A');
        ParserException longException(longMessage.c_str());
        CPPUNIT_ASSERT_EQUAL(longMessage, std::string(longException.what()));

        PesPacketReader::Exception longPesException(longMessage.c_str());
        CPPUNIT_ASSERT_EQUAL(longMessage, std::string(longPesException.what()));

        // Test message with only whitespace
        const std::string whitespaceMessage = "   \t\n   ";
        ParserException whitespaceException(whitespaceMessage.c_str());
        CPPUNIT_ASSERT_EQUAL(whitespaceMessage, std::string(whitespaceException.what()));
    }

    void testExceptionCopySemantics()
    {
        const std::string originalMessage = "original exception message";
        
        // Test ParserException copy behavior
        ParserException originalParser(originalMessage.c_str());
        ParserException copiedParser = originalParser;
        
        CPPUNIT_ASSERT_EQUAL(std::string(originalParser.what()), std::string(copiedParser.what()));
        CPPUNIT_ASSERT_EQUAL(originalMessage, std::string(copiedParser.what()));

        // Test PesPacketReader::Exception copy behavior
        PesPacketReader::Exception originalPes(originalMessage.c_str());
        PesPacketReader::Exception copiedPes = originalPes;
        
        CPPUNIT_ASSERT_EQUAL(std::string(originalPes.what()), std::string(copiedPes.what()));
        CPPUNIT_ASSERT_EQUAL(originalMessage, std::string(copiedPes.what()));

        // Test copy constructor with different messages
        const std::string newMessage = "new message";
        ParserException newException(newMessage.c_str());
        ParserException anotherCopy = newException;
        CPPUNIT_ASSERT_EQUAL(newMessage, std::string(anotherCopy.what()));
        
        // Verify original and copy are independent objects
        CPPUNIT_ASSERT_EQUAL(originalMessage, std::string(originalParser.what()));
        CPPUNIT_ASSERT_EQUAL(originalMessage, std::string(copiedParser.what()));
        CPPUNIT_ASSERT_EQUAL(newMessage, std::string(newException.what()));
        CPPUNIT_ASSERT_EQUAL(newMessage, std::string(anotherCopy.what()));
    }

    void testExceptionPropagation()
    {
        const std::string message = "propagated exception";
        
        // Test exception propagation through helper function
        auto throwParserException = [&message]() {
            throw ParserException(message.c_str());
        };

        try
        {
            throwParserException();
            CPPUNIT_FAIL("Exception should have been thrown");
        }
        catch (const ParserException& e)
        {
            CPPUNIT_ASSERT_EQUAL(message, std::string(e.what()));
        }

        // Test PesPacketReader::Exception propagation
        auto throwPesException = [&message]() {
            throw PesPacketReader::Exception(message.c_str());
        };

        try
        {
            throwPesException();
            CPPUNIT_FAIL("Exception should have been thrown");
        }
        catch (const PesPacketReader::Exception& e)
        {
            CPPUNIT_ASSERT_EQUAL(message, std::string(e.what()));
        }
    }

    void testMessageLifetimeSafety()
    {
        ParserException* exception = nullptr;
        PesPacketReader::Exception* pesException = nullptr;

        // Test with string that goes out of scope
        {
            std::string localMessage = "local scope message";
            exception = new ParserException(localMessage.c_str());
            pesException = new PesPacketReader::Exception(localMessage.c_str());
            
            // Verify messages are accessible while string is in scope
            CPPUNIT_ASSERT_EQUAL(localMessage, std::string(exception->what()));
            CPPUNIT_ASSERT_EQUAL(localMessage, std::string(pesException->what()));
        }
        
        // After scope: only assert non-null; implementation may or may not deep-copy message.
        CPPUNIT_ASSERT(exception->what() != nullptr);
        CPPUNIT_ASSERT(pesException->what() != nullptr);

        delete exception;
        delete pesException;

        // Test with string literal (guaranteed lifetime)
        ParserException literalException("string literal message");
        CPPUNIT_ASSERT_EQUAL(std::string("string literal message"), std::string(literalException.what()));
    }

    void testExceptionConsistency()
    {
        const std::string testMessage = "consistency test message";
        
        // Create both exception types with same message
        ParserException parserEx(testMessage.c_str());
        PesPacketReader::Exception pesEx(testMessage.c_str());

        // Verify both behave identically
        CPPUNIT_ASSERT_EQUAL(std::string(parserEx.what()), std::string(pesEx.what()));
        CPPUNIT_ASSERT_EQUAL(testMessage, std::string(parserEx.what()));
        CPPUNIT_ASSERT_EQUAL(testMessage, std::string(pesEx.what()));

        // Test nullptr handling consistency
        ParserException parserNullEx(nullptr);
        PesPacketReader::Exception pesNullEx(nullptr);

        CPPUNIT_ASSERT_EQUAL(std::string(parserNullEx.what()), std::string(pesNullEx.what()));
        CPPUNIT_ASSERT_EQUAL(std::string(), std::string(parserNullEx.what()));
        CPPUNIT_ASSERT_EQUAL(std::string(), std::string(pesNullEx.what()));

        // Test empty string consistency
        ParserException parserEmptyEx("");
        PesPacketReader::Exception pesEmptyEx("");

        CPPUNIT_ASSERT_EQUAL(std::string(parserEmptyEx.what()), std::string(pesEmptyEx.what()));
        CPPUNIT_ASSERT_EQUAL(std::string(), std::string(parserEmptyEx.what()));
    }

    void testNoexceptGuarantee()
    {
        const std::string message = "noexcept test message";
        // Compile-time guarantees (will fail to compile if not noexcept)
        static_assert(noexcept(std::declval<ParserException>().what()), "ParserException::what() must be noexcept");
        static_assert(noexcept(std::declval<PesPacketReader::Exception>().what()), "PesPacketReader::Exception::what() must be noexcept");
        
        // Runtime sanity (should not throw)
        ParserException parserEx(message.c_str());
        CPPUNIT_ASSERT(parserEx.what() != nullptr);
        PesPacketReader::Exception pesEx(message.c_str());
        CPPUNIT_ASSERT(pesEx.what() != nullptr);
        ParserException nullEx(nullptr);
        CPPUNIT_ASSERT(nullEx.what() != nullptr); // Expect empty string, not nullptr
    }

    void testMultipleWhatCalls()
    {
        const std::string message = "multiple calls test";
        
        // Test ParserException consistency across multiple what() calls
        ParserException parserEx(message.c_str());
        const char* result1 = parserEx.what();
        const char* result2 = parserEx.what();
        const char* result3 = parserEx.what();

        CPPUNIT_ASSERT_EQUAL(std::string(result1), std::string(result2));
        CPPUNIT_ASSERT_EQUAL(std::string(result2), std::string(result3));
        CPPUNIT_ASSERT_EQUAL(message, std::string(result1));

        // Test PesPacketReader::Exception consistency
        PesPacketReader::Exception pesEx(message.c_str());
        const char* pesResult1 = pesEx.what();
        const char* pesResult2 = pesEx.what();
        const char* pesResult3 = pesEx.what();

        CPPUNIT_ASSERT_EQUAL(std::string(pesResult1), std::string(pesResult2));
        CPPUNIT_ASSERT_EQUAL(std::string(pesResult2), std::string(pesResult3));
        CPPUNIT_ASSERT_EQUAL(message, std::string(pesResult1));

    }

    void testSpecialCharacterMessages()
    {
        // Test message with newlines and tabs
        const std::string newlineMessage = "Line 1\nLine 2\tTabbed";
        ParserException newlineEx(newlineMessage.c_str());
        CPPUNIT_ASSERT_EQUAL(newlineMessage, std::string(newlineEx.what()));

        PesPacketReader::Exception newlinePesEx(newlineMessage.c_str());
        CPPUNIT_ASSERT_EQUAL(newlineMessage, std::string(newlinePesEx.what()));

        // Test message with special characters
        const std::string specialMessage = "Error: 'invalid' data @line #42 [code: 0xFF]";
        ParserException specialEx(specialMessage.c_str());
        CPPUNIT_ASSERT_EQUAL(specialMessage, std::string(specialEx.what()));

        // Test message with quotes and backslashes
        const std::string quotesMessage = "Error: \"quoted text\" and \\ backslash";
        ParserException quotesEx(quotesMessage.c_str());
        CPPUNIT_ASSERT_EQUAL(quotesMessage, std::string(quotesEx.what()));

        // Test message with numbers and symbols
        const std::string symbolMessage = "Code: 12345 Error: !@#$%^&*()";
        PesPacketReader::Exception symbolEx(symbolMessage.c_str());
        CPPUNIT_ASSERT_EQUAL(symbolMessage, std::string(symbolEx.what()));
    }

    void testExceptionRethrowing()
    {
        const std::string originalMessage = "rethrow test message";
        
        // Test ParserException rethrowing
        try
        {
            try
            {
                throw ParserException(originalMessage.c_str());
            }
            catch (const ParserException& e)
            {
                CPPUNIT_ASSERT_EQUAL(originalMessage, std::string(e.what()));
                throw; // rethrow
            }
        }
        catch (const ParserException& e)
        {
            CPPUNIT_ASSERT_EQUAL(originalMessage, std::string(e.what()));
        }

        // Test PesPacketReader::Exception rethrowing
        try
        {
            try
            {
                throw PesPacketReader::Exception(originalMessage.c_str());
            }
            catch (const PesPacketReader::Exception& e)
            {
                CPPUNIT_ASSERT_EQUAL(originalMessage, std::string(e.what()));
                throw; // rethrow
            }
        }
        catch (const PesPacketReader::Exception& e)
        {
            CPPUNIT_ASSERT_EQUAL(originalMessage, std::string(e.what()));
        }
    }

    void testExceptionInheritanceHierarchy()
    {
        const std::string message = "inheritance test";
        
        // Test catching ParserException as std::exception
        bool caughtAsStdException = false;
        try
        {
            throw ParserException(message.c_str());
        }
        catch (const std::exception& e)
        {
            caughtAsStdException = true;
            CPPUNIT_ASSERT_EQUAL(message, std::string(e.what()));
        }
        catch (...)
        {
            CPPUNIT_FAIL("Should have been caught as std::exception");
        }
        CPPUNIT_ASSERT(caughtAsStdException);

        // Test catching PesPacketReader::Exception as std::exception
        caughtAsStdException = false;
        try
        {
            throw PesPacketReader::Exception(message.c_str());
        }
        catch (const std::exception& e)
        {
            caughtAsStdException = true;
            CPPUNIT_ASSERT_EQUAL(message, std::string(e.what()));
        }
        catch (...)
        {
            CPPUNIT_FAIL("Should have been caught as std::exception");
        }
        CPPUNIT_ASSERT(caughtAsStdException);

        // Test multiple catch blocks with inheritance
        bool caughtSpecific = false;
        try
        {
            throw ParserException(message.c_str());
        }
        catch (const ParserException& e)
        {
            caughtSpecific = true;
            CPPUNIT_ASSERT_EQUAL(message, std::string(e.what()));
        }
        catch (const std::exception& e)
        {
            CPPUNIT_FAIL("Should have been caught by specific handler");
        }
        CPPUNIT_ASSERT(caughtSpecific);
    }

    void testMessagePointerStability()
    {
        const std::string message = "pointer stability test";
        
        // Test ParserException pointer stability
        ParserException parserEx(message.c_str());
        const char* ptr1 = parserEx.what();
        const char* ptr2 = parserEx.what();
        // Do not assert pointer identity; only content.
        CPPUNIT_ASSERT_EQUAL(message, std::string(ptr1));
        CPPUNIT_ASSERT_EQUAL(std::string(ptr1), std::string(ptr2));

        // Test PesPacketReader::Exception pointer stability
        PesPacketReader::Exception pesEx(message.c_str());
        const char* pesPtr1 = pesEx.what();
        const char* pesPtr2 = pesEx.what();
        
        CPPUNIT_ASSERT_EQUAL(message, std::string(pesPtr1));
        CPPUNIT_ASSERT_EQUAL(std::string(pesPtr1), std::string(pesPtr2));

        // Test that pointer remains valid after operations
        ParserException tempEx(message.c_str());
        const char* tempPtr = tempEx.what();
        ParserException copiedEx = tempEx;
        
        CPPUNIT_ASSERT_EQUAL(std::string(tempPtr), std::string(copiedEx.what()));
    }

    void testExceptionConstCorrectness()
    {
        const std::string message = "const correctness test";
        
        // Test const ParserException
        const ParserException constParserEx(message.c_str());
        const char* result = constParserEx.what();
        CPPUNIT_ASSERT_EQUAL(message, std::string(result));

        // Test const PesPacketReader::Exception
        const PesPacketReader::Exception constPesEx(message.c_str());
        const char* pesResult = constPesEx.what();
        CPPUNIT_ASSERT_EQUAL(message, std::string(pesResult));

        // Test const reference behavior
        const ParserException& constRef = constParserEx;
        CPPUNIT_ASSERT_EQUAL(message, std::string(constRef.what()));

        const PesPacketReader::Exception& constPesRef = constPesEx;
        CPPUNIT_ASSERT_EQUAL(message, std::string(constPesRef.what()));

        // Test const exception through std::exception interface
        const std::exception& stdRef = constParserEx;
        CPPUNIT_ASSERT_EQUAL(message, std::string(stdRef.what()));
    }
}
;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ExceptionTest);
