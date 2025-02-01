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

#include "Misc.hpp"

#include "Status.hpp"

using dvbsubdecoder::Status;
using dvbsubdecoder::StcTime;
using dvbsubdecoder::StcTimeType;
using dvbsubdecoder::Specification;

class StatusTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( StatusTest );
    CPPUNIT_TEST(testDefault);
    CPPUNIT_TEST(testStc);
    CPPUNIT_TEST(testPageIds);CPPUNIT_TEST_SUITE_END()
    ;

public:
    void setUp()
    {
        // noop
    }

    void tearDown()
    {
        // noop
    }

    void testDefault()
    {
        Status status121(Specification::VERSION_1_2_1);
        CPPUNIT_ASSERT(
                status121.getSpecVersion() == Specification::VERSION_1_2_1);

        Status status131(Specification::VERSION_1_3_1);
        CPPUNIT_ASSERT(
                status131.getSpecVersion() == Specification::VERSION_1_3_1);

        Status status(Specification::VERSION_1_3_1);
        CPPUNIT_ASSERT(!status.isAncillaryPage(0));
        CPPUNIT_ASSERT(!status.isCompositionPage(0));
        CPPUNIT_ASSERT(!status.isSelectedPage(0));

        CPPUNIT_ASSERT(status.getLastPts() == StcTime());
    }

    void testStc()
    {
        Status status(Specification::VERSION_1_3_1);

        StcTime timeHigh(StcTimeType::HIGH_32, 1);
        StcTime timeLow(StcTimeType::LOW_32, 2);

        status.setLastPts(timeHigh);
        CPPUNIT_ASSERT(status.getLastPts() == timeHigh);

        status.setLastPts(timeLow);
        CPPUNIT_ASSERT(status.getLastPts() == timeLow);
    }

    void testPageIds()
    {
        Status statusDiff(Specification::VERSION_1_3_1);

        statusDiff.setPageIds(1, 2);

        CPPUNIT_ASSERT(!statusDiff.isSelectedPage(0));
        CPPUNIT_ASSERT(statusDiff.isSelectedPage(1));
        CPPUNIT_ASSERT(statusDiff.isSelectedPage(2));
        CPPUNIT_ASSERT(!statusDiff.isSelectedPage(3));

        CPPUNIT_ASSERT(!statusDiff.isCompositionPage(0));
        CPPUNIT_ASSERT(statusDiff.isCompositionPage(1));
        CPPUNIT_ASSERT(!statusDiff.isCompositionPage(2));
        CPPUNIT_ASSERT(!statusDiff.isCompositionPage(3));

        CPPUNIT_ASSERT(!statusDiff.isAncillaryPage(0));
        CPPUNIT_ASSERT(!statusDiff.isAncillaryPage(1));
        CPPUNIT_ASSERT(statusDiff.isAncillaryPage(2));
        CPPUNIT_ASSERT(!statusDiff.isAncillaryPage(3));

        Status statusSame(Specification::VERSION_1_3_1);

        statusSame.setPageIds(1, 1);

        CPPUNIT_ASSERT(!statusSame.isSelectedPage(0));
        CPPUNIT_ASSERT(statusSame.isSelectedPage(1));
        CPPUNIT_ASSERT(!statusSame.isSelectedPage(2));
        CPPUNIT_ASSERT(!statusSame.isSelectedPage(3));

        CPPUNIT_ASSERT(!statusSame.isCompositionPage(0));
        CPPUNIT_ASSERT(statusSame.isCompositionPage(1));
        CPPUNIT_ASSERT(!statusSame.isCompositionPage(2));
        CPPUNIT_ASSERT(!statusSame.isCompositionPage(3));

        CPPUNIT_ASSERT(!statusSame.isAncillaryPage(0));
        CPPUNIT_ASSERT(statusSame.isAncillaryPage(1));
        CPPUNIT_ASSERT(!statusSame.isAncillaryPage(2));
        CPPUNIT_ASSERT(!statusSame.isAncillaryPage(3));
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(StatusTest);
