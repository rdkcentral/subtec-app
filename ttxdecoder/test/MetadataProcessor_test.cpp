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
#include <memory>

#include "MetadataProcessor.hpp"
#include "Database.hpp"
#include "PacketHeader.hpp"
#include "PacketTriplets.hpp"
#include "PacketBcastServiceData.hpp"
#include "Page.hpp"
#include "PageId.hpp"

using namespace ttxdecoder;

// Mock Page for testing
class MockPage : public Page
{
public:
    MockPage(PageType type) : m_type(type) {}

    virtual PageType getType() const override
    {
        return m_type;
    }

    virtual bool isValid() const override
    {
        return true;
    }

    virtual void invalidate() override
    {
    }

protected:
    virtual PagePacket* getPagePacket(std::uint8_t packetAddress,
                                      std::int8_t designationCode) override
    {
        return nullptr;
    }

    virtual const PagePacket* getPagePacket(std::uint8_t packetAddress,
                                            std::int8_t designationCode) const override
    {
        return nullptr;
    }

private:
    PageType m_type;
};

class MetadataProcessorTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(MetadataProcessorTest);
    CPPUNIT_TEST(testConstructorInitialized);
    CPPUNIT_TEST(testBsdBufferCode0);
    CPPUNIT_TEST(testBsdBufferCode1);
    CPPUNIT_TEST(testBsdBufferCode2);
    CPPUNIT_TEST(testBsdBufferCode3);
    CPPUNIT_TEST(testBsdBufferInvalidCodeNegative);
    CPPUNIT_TEST(testBsdBufferInvalidCode4);
    CPPUNIT_TEST(testBsdBufferInvalidCode127);
    CPPUNIT_TEST(testBsdBufferWrongMagazine);
    CPPUNIT_TEST(testBsdBufferWrongPacketAddr);
    CPPUNIT_TEST(testMag8Packet29OutOfRange);
    CPPUNIT_TEST(testMag255Packet29OutOfRange);
    CPPUNIT_TEST(testMagPacket28Nullptr);
    CPPUNIT_TEST(testMagPacket30Nullptr);
    CPPUNIT_TEST(testMagPacket31Nullptr);
    CPPUNIT_TEST(testProcessBsdUpdatesDatabase);
    CPPUNIT_TEST(testTripletsValidMagRange);
    CPPUNIT_TEST(testTripletsInvalidMagazine);
    CPPUNIT_TEST(testTripletsWrongPacketAddr);
    CPPUNIT_TEST(testBsdBufferReused);
    CPPUNIT_TEST(testSequentialProcessing);
    CPPUNIT_TEST(testMultipleResetsConsistent);
    CPPUNIT_TEST(testBufferConsistencyAfterReset);
    CPPUNIT_TEST(testDatabaseAfterMultipleBsd);

CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_database = std::make_unique<Database>();
        m_processor = std::make_unique<MetadataProcessor>(*m_database);
    }

    void tearDown() override
    {
        m_processor.reset();
        m_database.reset();
    }

protected:
    void assertPageIdEquals(const PageId& expected, const PageId& actual)
    {
        CPPUNIT_ASSERT_EQUAL(expected.getMagazinePage(), actual.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(expected.getSubpage(), actual.getSubpage());
    }

    PacketTriplets* getTripletsBuffer(std::uint8_t magazine,
                                      std::int8_t designationCode)
    {
        Packet* buffer = m_processor->getPacketBuffer(magazine, 29, designationCode);
        CPPUNIT_ASSERT(buffer != nullptr);
        CPPUNIT_ASSERT_EQUAL(PacketType::TRIPLETS, buffer->getType());

        PacketTriplets* triplets = static_cast<PacketTriplets*>(buffer);
        triplets->setMagazineNumber(magazine);
        triplets->setPacketAddress(29);
        triplets->setDesignationCode(designationCode);
        return triplets;
    }

    void testConstructorInitialized()
    {
        // Verify constructor succeeds and processor is ready to use
        Database testDb;
        MetadataProcessor processor(testDb);

        // Verify processor can immediately provide packet buffers
        Packet* buffer = processor.getPacketBuffer(0, 30, 0);
        CPPUNIT_ASSERT(buffer != nullptr);
        CPPUNIT_ASSERT_EQUAL(PacketType::BCAST_SERVICE_DATA, buffer->getType());
    }

    void testBsdBufferCode0()
    {
        Packet* buffer = m_processor->getPacketBuffer(0, 30, 0);
        CPPUNIT_ASSERT(buffer != nullptr);
        CPPUNIT_ASSERT_EQUAL(PacketType::BCAST_SERVICE_DATA, buffer->getType());
    }

    void testBsdBufferCode1()
    {
        Packet* buffer = m_processor->getPacketBuffer(0, 30, 1);
        CPPUNIT_ASSERT(buffer != nullptr);
        CPPUNIT_ASSERT_EQUAL(PacketType::BCAST_SERVICE_DATA, buffer->getType());
    }

    void testBsdBufferCode2()
    {
        Packet* buffer = m_processor->getPacketBuffer(0, 30, 2);
        CPPUNIT_ASSERT(buffer != nullptr);
        CPPUNIT_ASSERT_EQUAL(PacketType::BCAST_SERVICE_DATA, buffer->getType());
    }

    void testBsdBufferCode3()
    {
        Packet* buffer = m_processor->getPacketBuffer(0, 30, 3);
        CPPUNIT_ASSERT(buffer != nullptr);
        CPPUNIT_ASSERT_EQUAL(PacketType::BCAST_SERVICE_DATA, buffer->getType());
    }

    void testBsdBufferInvalidCodeNegative()
    {
        Packet* buffer = m_processor->getPacketBuffer(0, 30, -1);
        CPPUNIT_ASSERT(buffer == nullptr);
    }

    void testBsdBufferInvalidCode4()
    {
        Packet* buffer = m_processor->getPacketBuffer(0, 30, 4);
        CPPUNIT_ASSERT(buffer == nullptr);
    }

    void testBsdBufferInvalidCode127()
    {
        Packet* buffer = m_processor->getPacketBuffer(0, 30, 127);
        CPPUNIT_ASSERT(buffer == nullptr);
    }

    void testBsdBufferWrongMagazine()
    {
        // Test all magazines except 0 for packet 30
        for (std::uint8_t mag = 1; mag < 8; ++mag)
        {
            Packet* buffer = m_processor->getPacketBuffer(mag, 30, 0);
            CPPUNIT_ASSERT(buffer == nullptr);
        }
    }

    void testBsdBufferWrongPacketAddr()
    {
        // Magazine 0 with packet address 31 should return nullptr (not 29 or 30)
        Packet* buffer28 = m_processor->getPacketBuffer(0, 28, 0);
        Packet* buffer31 = m_processor->getPacketBuffer(0, 31, 0);
        CPPUNIT_ASSERT(buffer28 == nullptr);
        CPPUNIT_ASSERT(buffer31 == nullptr);

        // Note: Magazine 0, packet 29 is valid (magazine packet, not BSD)
        // and returns a packet from magazinePage.takePacket()
    }



    void testMag8Packet29OutOfRange()
    {
        Packet* buffer = m_processor->getPacketBuffer(8, 29, 0);
        CPPUNIT_ASSERT(buffer == nullptr);
    }

    void testMag255Packet29OutOfRange()
    {
        Packet* buffer = m_processor->getPacketBuffer(255, 29, 0);
        CPPUNIT_ASSERT(buffer == nullptr);
    }

    void testMagPacket28Nullptr()
    {
        Packet* buffer = m_processor->getPacketBuffer(1, 28, 0);
        CPPUNIT_ASSERT(buffer == nullptr);
    }

    void testMagPacket30Nullptr()
    {
        // Magazine 1 with packet 30 should return nullptr (only magazine 0 works for packet 30)
        Packet* buffer = m_processor->getPacketBuffer(1, 30, 0);
        CPPUNIT_ASSERT(buffer == nullptr);
    }

    void testMagPacket31Nullptr()
    {
        Packet* buffer = m_processor->getPacketBuffer(1, 31, 0);
        CPPUNIT_ASSERT(buffer == nullptr);
    }

    void testProcessBsdUpdatesDatabase()
    {
        PacketBcastServiceData bsdPacket;
        bsdPacket.setMagazineNumber(0);
        bsdPacket.setPacketAddress(30);
        PageId expectedPage(0x234, 0x0001);
        bsdPacket.setInitialPage(expectedPage);

        m_processor->processPacket(bsdPacket);
        assertPageIdEquals(expectedPage, m_database->getIndexPageP830());
    }

    void testTripletsValidMagRange()
    {
        for (std::uint8_t mag = 0; mag < Database::MAGAZINE_COUNT; ++mag)
        {
            PacketTriplets* triplets = getTripletsBuffer(mag, 0);
            m_processor->processPacket(*triplets);

            const PacketTriplets* storedPacket = m_database->getMagazinePage(mag).getPacket29_0();
            CPPUNIT_ASSERT_EQUAL(static_cast<const PacketTriplets*>(triplets), storedPacket);
        }
    }

    void testTripletsInvalidMagazine()
    {
        PacketTriplets* validTriplets = getTripletsBuffer(1, 0);
        m_processor->processPacket(*validTriplets);

        PacketTriplets triplets;
        triplets.setMagazineNumber(8);  // >= MAGAZINE_COUNT
        triplets.setPacketAddress(29);
        triplets.setDesignationCode(0);

        m_processor->processPacket(triplets);

        const PacketTriplets* storedPacket = m_database->getMagazinePage(1).getPacket29_0();
        CPPUNIT_ASSERT_EQUAL(static_cast<const PacketTriplets*>(validTriplets), storedPacket);
    }

    void testTripletsWrongPacketAddr()
    {
        PacketTriplets* validTriplets = getTripletsBuffer(1, 0);
        m_processor->processPacket(*validTriplets);

        PacketTriplets triplets1;
        triplets1.setMagazineNumber(1);
        triplets1.setPacketAddress(28);
        triplets1.setDesignationCode(0);

        PacketTriplets triplets2;
        triplets2.setMagazineNumber(1);
        triplets2.setPacketAddress(30);
        triplets2.setDesignationCode(0);

        m_processor->processPacket(triplets1);
        m_processor->processPacket(triplets2);

        const PacketTriplets* storedPacket = m_database->getMagazinePage(1).getPacket29_0();
        CPPUNIT_ASSERT_EQUAL(static_cast<const PacketTriplets*>(validTriplets), storedPacket);
    }

    void testBsdBufferReused()
    {
        // Verify same buffer is returned for different BSD designation codes
        Packet* buffer1 = m_processor->getPacketBuffer(0, 30, 0);
        Packet* buffer2 = m_processor->getPacketBuffer(0, 30, 1);
        Packet* buffer3 = m_processor->getPacketBuffer(0, 30, 2);

        CPPUNIT_ASSERT(buffer1 != nullptr);
        CPPUNIT_ASSERT(buffer2 != nullptr);
        CPPUNIT_ASSERT(buffer3 != nullptr);

        // All should point to the same BSD packet buffer
        CPPUNIT_ASSERT_EQUAL(buffer1, buffer2);
        CPPUNIT_ASSERT_EQUAL(buffer2, buffer3);
    }

    void testSequentialProcessing()
    {
        MockPage page(PageType::DISPLAYABLE);
        PacketTriplets* triplets = getTripletsBuffer(1, 0);
        PacketBcastServiceData* bsdBuffer =
            static_cast<PacketBcastServiceData*>(m_processor->getPacketBuffer(0, 30, 0));
        CPPUNIT_ASSERT(bsdBuffer != nullptr);

        PageId expectedPage(0x345, 0x0002);
        bsdBuffer->setInitialPage(expectedPage);

        m_processor->processPage(page);
        m_processor->processPacket(*triplets);

        m_processor->processPage(page);
        m_processor->processPacket(*bsdBuffer);

        CPPUNIT_ASSERT_EQUAL(static_cast<const PacketTriplets*>(triplets),
                             m_database->getMagazinePage(1).getPacket29_0());
        assertPageIdEquals(expectedPage, m_database->getIndexPageP830());
    }

    void testMultipleResetsConsistent()
    {
        for (int i = 0; i < 3; ++i)
        {
            m_processor->reset();

            // After each reset, verify functionality
            Packet* buffer = m_processor->getPacketBuffer(0, 30, 0);
            CPPUNIT_ASSERT(buffer != nullptr);
            CPPUNIT_ASSERT_EQUAL(PacketType::BCAST_SERVICE_DATA, buffer->getType());
        }
    }

    void testBufferConsistencyAfterReset()
    {
        // Get buffer before reset
        Packet* bufferBefore = m_processor->getPacketBuffer(0, 30, 0);
        CPPUNIT_ASSERT(bufferBefore != nullptr);

        m_processor->reset();

        // Get buffer after reset - should still be same object
        Packet* bufferAfter = m_processor->getPacketBuffer(0, 30, 0);
        CPPUNIT_ASSERT(bufferAfter != nullptr);
        CPPUNIT_ASSERT_EQUAL(bufferBefore, bufferAfter);
    }

    void testDatabaseAfterMultipleBsd()
    {
        PacketBcastServiceData bsd1, bsd2, bsd3;
        bsd1.setMagazineNumber(0);
        bsd1.setPacketAddress(30);
        bsd1.setInitialPage(PageId(0x123, 0x0001));
        bsd2.setMagazineNumber(0);
        bsd2.setPacketAddress(30);
        bsd2.setInitialPage(PageId(0x456, 0x0002));
        bsd3.setMagazineNumber(0);
        bsd3.setPacketAddress(30);
        bsd3.setInitialPage(PageId(0x789, 0x0003));

        m_processor->processPacket(bsd1);
        assertPageIdEquals(bsd1.getInitialPage(), m_database->getIndexPageP830());

        m_processor->processPacket(bsd2);
        assertPageIdEquals(bsd2.getInitialPage(), m_database->getIndexPageP830());

        m_processor->processPacket(bsd3);
        assertPageIdEquals(bsd3.getInitialPage(), m_database->getIndexPageP830());
    }

private:
    std::unique_ptr<Database> m_database;
    std::unique_ptr<MetadataProcessor> m_processor;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MetadataProcessorTest);
