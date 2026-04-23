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

#include "PesPacketReader.hpp"

using dvbsubdecoder::PesPacketReader;
using dvbsubdecoder::PesPacketHeader;

class PesPacketReaderTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PesPacketReaderTest );
    CPPUNIT_TEST(testEmpty);
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testSubReader);
    CPPUNIT_TEST(testPesPacketHeaderIsSubtitlesPacket);
    CPPUNIT_TEST(testPesPacketHeaderGetTotalSize);
    CPPUNIT_TEST(testCopyAssignment);
    CPPUNIT_TEST(testSingleByteChunks);
    CPPUNIT_TEST(testZeroLengthFirstChunk);
    CPPUNIT_TEST(testZeroLengthSecondChunk);
    CPPUNIT_TEST(testBothChunksZeroLength);
    CPPUNIT_TEST(testReadUint16AcrossBoundary);
    CPPUNIT_TEST(testSubReaderExactCount);
    CPPUNIT_TEST(testSubReaderZeroCount);
    CPPUNIT_TEST(testMultiplePeekOperations);
    CPPUNIT_TEST(testMixedPeekAndRead);
    CPPUNIT_TEST(testExceptionMessages);
    CPPUNIT_TEST(testStateAfterExceptions);
    CPPUNIT_TEST(testGetBytesLeftAccuracy);
    CPPUNIT_TEST(testUint16BeSpecificPatterns);
    CPPUNIT_TEST(testBoundaryByteValues);
    CPPUNIT_TEST(testLargeSkipOperations);
    CPPUNIT_TEST(testSubReaderFromConsumedReader);
    CPPUNIT_TEST(testComplexOperationSequences);
    CPPUNIT_TEST(testSubReaderMinimalCases);
    CPPUNIT_TEST(testSkipZeroBytes);
    CPPUNIT_TEST(testEmptyReaderOperations);
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

    void testEmpty()
    {
        PesPacketReader reader;

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0),
                reader.getBytesLeft());
        CPPUNIT_ASSERT_THROW(reader.peekUint8(), PesPacketReader::Exception);
        CPPUNIT_ASSERT_THROW(reader.readUint8(), PesPacketReader::Exception);
        CPPUNIT_ASSERT_THROW(reader.readUint16be(), PesPacketReader::Exception);
        CPPUNIT_ASSERT_THROW(reader.skip(1), PesPacketReader::Exception);
    }

    void testSimple()
    {
        std::vector<std::uint8_t> chunk1 =
        { 0x10, 0x11, 0x12 };
        std::vector<std::uint8_t> chunk2 =
        { 0x20, 0x21, 0x30, 0x40 };

        PesPacketReader reader(chunk1.data(), chunk1.size(), chunk2.data(),
                chunk2.size());

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(7),
                reader.getBytesLeft());
        CPPUNIT_ASSERT_EQUAL(0x10, static_cast<int>(reader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0x11, static_cast<int>(reader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0x12, static_cast<int>(reader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0x2021, static_cast<int>(reader.readUint16be()));
        CPPUNIT_ASSERT_NO_THROW(reader.skip(1));
        CPPUNIT_ASSERT_EQUAL(0x40, static_cast<int>(reader.readUint8()));

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0),
                reader.getBytesLeft());
        CPPUNIT_ASSERT_THROW(reader.peekUint8(), PesPacketReader::Exception);
        CPPUNIT_ASSERT_THROW(reader.readUint8(), PesPacketReader::Exception);
        CPPUNIT_ASSERT_THROW(reader.readUint16be(), PesPacketReader::Exception);
        CPPUNIT_ASSERT_THROW(reader.skip(1), PesPacketReader::Exception);
    }

    void testSubReader()
    {
        std::vector<std::uint8_t> chunk1 =
        { 0x10, 0x11, 0x12 };
        std::vector<std::uint8_t> chunk2 =
        { 0x20, 0x21, 0x30, 0x40 };

        PesPacketReader originalReader(chunk1.data(), chunk1.size(),
                chunk2.data(), chunk2.size());

        originalReader.skip(2);

        PesPacketReader reader(originalReader, 4);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(4),
                reader.getBytesLeft());
        CPPUNIT_ASSERT_EQUAL(0x12, static_cast<int>(reader.peekUint8()));
        CPPUNIT_ASSERT_EQUAL(0x12, static_cast<int>(reader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0x2021, static_cast<int>(reader.readUint16be()));
        CPPUNIT_ASSERT_EQUAL(0x30, static_cast<int>(reader.peekUint8()));
        CPPUNIT_ASSERT_EQUAL(0x30, static_cast<int>(reader.readUint8()));

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0),
                reader.getBytesLeft());
        CPPUNIT_ASSERT_THROW(reader.peekUint8(), PesPacketReader::Exception);
        CPPUNIT_ASSERT_THROW(reader.readUint8(), PesPacketReader::Exception);
        CPPUNIT_ASSERT_THROW(reader.readUint16be(), PesPacketReader::Exception);
        CPPUNIT_ASSERT_THROW(reader.skip(1), PesPacketReader::Exception);

        CPPUNIT_ASSERT_THROW(PesPacketReader readerBad(originalReader, 10),
                PesPacketReader::Exception);
    }

    void testPesPacketHeaderIsSubtitlesPacket()
    {
        PesPacketHeader header;
        
        // Test subtitle stream ID (0xBD)
        header.m_streamId = 0xBD;
        CPPUNIT_ASSERT(header.isSubtitlesPacket());
        
        // Test non-subtitle stream IDs
        header.m_streamId = 0xBC;
        CPPUNIT_ASSERT(!header.isSubtitlesPacket());
        
        header.m_streamId = 0xBE;
        CPPUNIT_ASSERT(!header.isSubtitlesPacket());
        
        header.m_streamId = 0x00;
        CPPUNIT_ASSERT(!header.isSubtitlesPacket());
        
        header.m_streamId = 0xFF;
        CPPUNIT_ASSERT(!header.isSubtitlesPacket());
    }

    void testPesPacketHeaderGetTotalSize()
    {
        PesPacketHeader header;
        
        // Test zero length (unknown size)
        header.m_pesPacketLength = 0;
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), header.getTotalSize());
        
        // Test non-zero length (length + 6 header bytes)
        header.m_pesPacketLength = 100;
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(106), header.getTotalSize());
        
        header.m_pesPacketLength = 1;
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(7), header.getTotalSize());
        
        header.m_pesPacketLength = 65535;
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(65541), header.getTotalSize());
    }

    void testCopyAssignment()
    {
        std::vector<std::uint8_t> data1 = {0x01, 0x02};
        std::vector<std::uint8_t> data2 = {0x03, 0x04};
        
        PesPacketReader reader1(data1.data(), data1.size(), data2.data(), data2.size());
        PesPacketReader reader2;
        
        // Test assignment
        reader2 = reader1;
        
        // Verify both readers work identically
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(4), reader2.getBytesLeft());
        CPPUNIT_ASSERT_EQUAL(0x01, static_cast<int>(reader2.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0x02, static_cast<int>(reader2.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0x03, static_cast<int>(reader2.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0x04, static_cast<int>(reader2.readUint8()));
    }

    void testSingleByteChunks()
    {
        std::vector<std::uint8_t> chunk1 = {0xAA};
        std::vector<std::uint8_t> chunk2 = {0xBB};
        
        PesPacketReader reader(chunk1.data(), chunk1.size(), chunk2.data(), chunk2.size());
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), reader.getBytesLeft());
        CPPUNIT_ASSERT_EQUAL(0xAA, static_cast<int>(reader.peekUint8()));
        CPPUNIT_ASSERT_EQUAL(0xAA, static_cast<int>(reader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), reader.getBytesLeft());
        CPPUNIT_ASSERT_EQUAL(0xBB, static_cast<int>(reader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), reader.getBytesLeft());
    }

    void testZeroLengthFirstChunk()
    {
        std::vector<std::uint8_t> chunk2 = {0x10, 0x20, 0x30};
        
        PesPacketReader reader(nullptr, 0, chunk2.data(), chunk2.size());
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(3), reader.getBytesLeft());
        CPPUNIT_ASSERT_EQUAL(0x10, static_cast<int>(reader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0x20, static_cast<int>(reader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0x30, static_cast<int>(reader.readUint8()));
    }

    void testZeroLengthSecondChunk()
    {
        std::vector<std::uint8_t> chunk1 = {0x40, 0x50, 0x60};
        
        PesPacketReader reader(chunk1.data(), chunk1.size(), nullptr, 0);
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(3), reader.getBytesLeft());
        CPPUNIT_ASSERT_EQUAL(0x40, static_cast<int>(reader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0x50, static_cast<int>(reader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0x60, static_cast<int>(reader.readUint8()));
    }

    void testBothChunksZeroLength()
    {
        PesPacketReader reader(nullptr, 0, nullptr, 0);
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), reader.getBytesLeft());
        CPPUNIT_ASSERT_THROW(reader.peekUint8(), PesPacketReader::Exception);
        CPPUNIT_ASSERT_THROW(reader.readUint8(), PesPacketReader::Exception);
        CPPUNIT_ASSERT_THROW(reader.readUint16be(), PesPacketReader::Exception);
        CPPUNIT_ASSERT_THROW(reader.skip(1), PesPacketReader::Exception);
    }

    void testReadUint16AcrossBoundary()
    {
        std::vector<std::uint8_t> chunk1 = {0x12};
        std::vector<std::uint8_t> chunk2 = {0x34, 0x56};
        
        PesPacketReader reader(chunk1.data(), chunk1.size(), chunk2.data(), chunk2.size());
        
        // Read uint16 that spans across chunk boundary
        CPPUNIT_ASSERT_EQUAL(0x1234, static_cast<int>(reader.readUint16be()));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), reader.getBytesLeft());
        CPPUNIT_ASSERT_EQUAL(0x56, static_cast<int>(reader.readUint8()));
    }

    void testSubReaderExactCount()
    {
        std::vector<std::uint8_t> chunk1 = {0x10, 0x20};
        std::vector<std::uint8_t> chunk2 = {0x30, 0x40};
        
        PesPacketReader originalReader(chunk1.data(), chunk1.size(), chunk2.data(), chunk2.size());
        
        // Create sub-reader with exact available count
        PesPacketReader subReader(originalReader, 4);
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(4), subReader.getBytesLeft());
        CPPUNIT_ASSERT_EQUAL(0x10, static_cast<int>(subReader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0x20, static_cast<int>(subReader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0x30, static_cast<int>(subReader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0x40, static_cast<int>(subReader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), subReader.getBytesLeft());
    }

    void testSubReaderZeroCount()
    {
        std::vector<std::uint8_t> chunk1 = {0x10, 0x20};
        
        PesPacketReader originalReader(chunk1.data(), chunk1.size(), nullptr, 0);
        
        // Create sub-reader with zero count
        PesPacketReader subReader(originalReader, 0);
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), subReader.getBytesLeft());
        CPPUNIT_ASSERT_THROW(subReader.readUint8(), PesPacketReader::Exception);
    }

    void testMultiplePeekOperations()
    {
        std::vector<std::uint8_t> chunk1 = {0x77, 0x88};
        
        PesPacketReader reader(chunk1.data(), chunk1.size(), nullptr, 0);
        
        // Multiple peeks should return same value
        CPPUNIT_ASSERT_EQUAL(0x77, static_cast<int>(reader.peekUint8()));
        CPPUNIT_ASSERT_EQUAL(0x77, static_cast<int>(reader.peekUint8()));
        CPPUNIT_ASSERT_EQUAL(0x77, static_cast<int>(reader.peekUint8()));
        
        // State should not change
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), reader.getBytesLeft());
        
        // Read should return same value as peek
        CPPUNIT_ASSERT_EQUAL(0x77, static_cast<int>(reader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), reader.getBytesLeft());
    }

    void testMixedPeekAndRead()
    {
        std::vector<std::uint8_t> chunk1 = {0x11, 0x22, 0x33};
        
        PesPacketReader reader(chunk1.data(), chunk1.size(), nullptr, 0);
        
        // Peek, read, peek, read pattern
        CPPUNIT_ASSERT_EQUAL(0x11, static_cast<int>(reader.peekUint8()));
        CPPUNIT_ASSERT_EQUAL(0x11, static_cast<int>(reader.readUint8()));
        
        CPPUNIT_ASSERT_EQUAL(0x22, static_cast<int>(reader.peekUint8()));
        CPPUNIT_ASSERT_EQUAL(0x22, static_cast<int>(reader.readUint8()));
        
        CPPUNIT_ASSERT_EQUAL(0x33, static_cast<int>(reader.peekUint8()));
        CPPUNIT_ASSERT_EQUAL(0x33, static_cast<int>(reader.readUint8()));
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), reader.getBytesLeft());
    }

    void testExceptionMessages()
    {
        PesPacketReader emptyReader;
        
        try {
            emptyReader.readUint8();
            CPPUNIT_FAIL("Expected exception");
        } catch (const PesPacketReader::Exception& e) {
            CPPUNIT_ASSERT(std::string(e.what()) == "No more bytes available");
        }
        
        try {
            emptyReader.skip(1);
            CPPUNIT_FAIL("Expected exception");
        } catch (const PesPacketReader::Exception& e) {
            CPPUNIT_ASSERT(std::string(e.what()) == "Cannot skip requested number of bytes");
        }
        
        std::vector<std::uint8_t> data = {0x01};
        PesPacketReader reader(data.data(), data.size(), nullptr, 0);
        
        try {
            PesPacketReader subReader(reader, 5);
            CPPUNIT_FAIL("Expected exception");
        } catch (const PesPacketReader::Exception& e) {
            CPPUNIT_ASSERT(std::string(e.what()) == "More bytes requested than available in original reader");
        }
    }

    void testStateAfterExceptions()
    {
        std::vector<std::uint8_t> data = {0x99};
        PesPacketReader reader(data.data(), data.size(), nullptr, 0);
        
        // Read the only byte
        CPPUNIT_ASSERT_EQUAL(0x99, static_cast<int>(reader.readUint8()));
        
        // Try operations that should throw
        CPPUNIT_ASSERT_THROW(reader.readUint8(), PesPacketReader::Exception);
        CPPUNIT_ASSERT_THROW(reader.peekUint8(), PesPacketReader::Exception);
        CPPUNIT_ASSERT_THROW(reader.skip(1), PesPacketReader::Exception);
        
        // State should remain consistent
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), reader.getBytesLeft());
    }

    void testGetBytesLeftAccuracy()
    {
        std::vector<std::uint8_t> chunk1 = {0x01, 0x02, 0x03};
        std::vector<std::uint8_t> chunk2 = {0x04, 0x05};
        
        PesPacketReader reader(chunk1.data(), chunk1.size(), chunk2.data(), chunk2.size());
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(5), reader.getBytesLeft());
        
        reader.readUint8();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(4), reader.getBytesLeft());
        
        reader.skip(2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), reader.getBytesLeft());
        
        reader.readUint16be();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), reader.getBytesLeft());
    }

    void testUint16BeSpecificPatterns()
    {
        std::vector<std::uint8_t> data = {0x00, 0x00, 0xFF, 0xFF, 0x00, 0x01, 0x01, 0x00};
        
        PesPacketReader reader(data.data(), data.size(), nullptr, 0);
        
        // Test specific patterns
        CPPUNIT_ASSERT_EQUAL(0x0000, static_cast<int>(reader.readUint16be()));
        CPPUNIT_ASSERT_EQUAL(0xFFFF, static_cast<int>(reader.readUint16be()));
        CPPUNIT_ASSERT_EQUAL(0x0001, static_cast<int>(reader.readUint16be()));
        CPPUNIT_ASSERT_EQUAL(0x0100, static_cast<int>(reader.readUint16be()));
    }

    void testBoundaryByteValues()
    {
        std::vector<std::uint8_t> data = {0x00, 0xFF, 0x01, 0xFE};
        
        PesPacketReader reader(data.data(), data.size(), nullptr, 0);
        
        CPPUNIT_ASSERT_EQUAL(0x00, static_cast<int>(reader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0xFF, static_cast<int>(reader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0x01, static_cast<int>(reader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0xFE, static_cast<int>(reader.readUint8()));
    }

    void testLargeSkipOperations()
    {
        std::vector<std::uint8_t> chunk1(1000, 0xAA);
        std::vector<std::uint8_t> chunk2(500, 0xBB);
        
        PesPacketReader reader(chunk1.data(), chunk1.size(), chunk2.data(), chunk2.size());
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1500), reader.getBytesLeft());
        
        // Skip large amount within first chunk
        reader.skip(800);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(700), reader.getBytesLeft());
        
        // Skip across chunk boundary
        reader.skip(300);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(400), reader.getBytesLeft());
        
        // Verify we're in second chunk
        CPPUNIT_ASSERT_EQUAL(0xBB, static_cast<int>(reader.readUint8()));
    }

    void testSubReaderFromConsumedReader()
    {
        std::vector<std::uint8_t> data = {0x10, 0x20, 0x30, 0x40, 0x50};
        
        PesPacketReader originalReader(data.data(), data.size(), nullptr, 0);
        
        // Consume some data
        originalReader.skip(2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(3), originalReader.getBytesLeft());
        
        // Create sub-reader from consumed reader
        PesPacketReader subReader(originalReader, 2);
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), subReader.getBytesLeft());
        CPPUNIT_ASSERT_EQUAL(0x30, static_cast<int>(subReader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0x40, static_cast<int>(subReader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), subReader.getBytesLeft());
    }

    void testComplexOperationSequences()
    {
        std::vector<std::uint8_t> chunk1 = {0x01, 0x02, 0x03};
        std::vector<std::uint8_t> chunk2 = {0x04, 0x05, 0x06, 0x07};
        
        PesPacketReader reader(chunk1.data(), chunk1.size(), chunk2.data(), chunk2.size());
        
        // Complex sequence: read-skip-peek-read-uint16
        CPPUNIT_ASSERT_EQUAL(0x01, static_cast<int>(reader.readUint8()));
        reader.skip(1);
        CPPUNIT_ASSERT_EQUAL(0x03, static_cast<int>(reader.peekUint8()));
        CPPUNIT_ASSERT_EQUAL(0x03, static_cast<int>(reader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(0x0405, static_cast<int>(reader.readUint16be()));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), reader.getBytesLeft());
    }

    void testSubReaderMinimalCases()
    {
        std::vector<std::uint8_t> data = {0xAA, 0xBB, 0xCC};
        
        PesPacketReader originalReader(data.data(), data.size(), nullptr, 0);
        originalReader.skip(1);
        
        // Create sub-reader with count = 1
        PesPacketReader subReader(originalReader, 1);
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), subReader.getBytesLeft());
        CPPUNIT_ASSERT_EQUAL(0xBB, static_cast<int>(subReader.peekUint8()));
        CPPUNIT_ASSERT_EQUAL(0xBB, static_cast<int>(subReader.readUint8()));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), subReader.getBytesLeft());
    }

    void testSkipZeroBytes()
    {
        std::vector<std::uint8_t> data = {0x11, 0x22};
        
        PesPacketReader reader(data.data(), data.size(), nullptr, 0);
        
        // Skip zero bytes should be no-op
        CPPUNIT_ASSERT_NO_THROW(reader.skip(0));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), reader.getBytesLeft());
        CPPUNIT_ASSERT_EQUAL(0x11, static_cast<int>(reader.readUint8()));
    }

    void testEmptyReaderOperations()
    {
        PesPacketReader emptyReader;
        
        // Test all operations on empty reader
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), emptyReader.getBytesLeft());
        CPPUNIT_ASSERT_THROW(emptyReader.peekUint8(), PesPacketReader::Exception);
        CPPUNIT_ASSERT_THROW(emptyReader.readUint8(), PesPacketReader::Exception);
        CPPUNIT_ASSERT_THROW(emptyReader.readUint16be(), PesPacketReader::Exception);
        CPPUNIT_ASSERT_THROW(emptyReader.skip(1), PesPacketReader::Exception);
        
        // Skip zero should work
        CPPUNIT_ASSERT_NO_THROW(emptyReader.skip(0));
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PesPacketReaderTest);
