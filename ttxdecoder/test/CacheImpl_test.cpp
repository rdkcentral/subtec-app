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

#include "CacheImpl.hpp"

using ttxdecoder::CacheImpl;
using ttxdecoder::PageId;

class CacheImplTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( CacheImplTest );
    CPPUNIT_TEST(testPageReleasedDuringUpdate);
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

    void testPageReleasedDuringUpdate()
    {
        PageId pageId{100, 0};

        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        cache.setCurrentPage(pageId);

        auto clearPage = cache.getClearPage();
        CPPUNIT_ASSERT(clearPage != nullptr);

        auto headerPacket = clearPage->takeHeader();
        CPPUNIT_ASSERT(headerPacket != nullptr);

        headerPacket->setPageInfo(pageId, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);

        clearPage->setLastPacketValid(headerPacket);
        cache.insertPage(clearPage);

        auto mutablePage = cache.getMutablePage(pageId);

        CPPUNIT_ASSERT(clearPage == mutablePage);

        auto readyPage = cache.getPage(pageId);
        cache.releasePage(readyPage);

        CPPUNIT_ASSERT_NO_THROW(cache.insertPage(mutablePage));
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( CacheImplTest );
