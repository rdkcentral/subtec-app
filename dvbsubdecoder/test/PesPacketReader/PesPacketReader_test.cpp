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

class PesPacketReaderTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PesPacketReaderTest );
    CPPUNIT_TEST(testEmpty);
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testSubReader);
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
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PesPacketReaderTest);
