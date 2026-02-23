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
#include "BrowserRendererImpl.hpp"
#include <ttxdecoder/PageId.hpp>

using subttxrend::ttxt::BrowserRendererImpl;

class TestBrowserRendererImpl : public BrowserRendererImpl
{
public:
    TestBrowserRendererImpl()
        : m_setCurrentPageCalls(0), m_lastPageId(0, 0x3F7F)
    {
    }

    int getSetCurrentPageCalls() const { return m_setCurrentPageCalls; }
    ttxdecoder::PageId getLastPageId() const { return m_lastPageId; }

protected:
    void setCurrentPage(const ttxdecoder::PageId& pageId) override
    {
        m_lastPageId = pageId;
        ++m_setCurrentPageCalls;
    }

private:
    int m_setCurrentPageCalls;
    ttxdecoder::PageId m_lastPageId;
};

class BrowserRendererImplTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(BrowserRendererImplTest);
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testDestructor);
    CPPUNIT_TEST(testResetStartPage_SetsDefaultPage);
    CPPUNIT_TEST(testResetStartPage_MultipleCallsIdempotent);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_renderer.reset(new TestBrowserRendererImpl());
    }

    void tearDown() override
    {
        m_renderer.reset();
    }

protected:
    void testConstructor()
    {
        std::unique_ptr<TestBrowserRendererImpl> renderer(new TestBrowserRendererImpl());
        CPPUNIT_ASSERT(renderer.get() != nullptr);
    }

    void testDestructor()
    {
        std::unique_ptr<TestBrowserRendererImpl> renderer(new TestBrowserRendererImpl());
        renderer.reset();
        CPPUNIT_ASSERT(true);
    }

    void testResetStartPage_SetsDefaultPage()
    {
        m_renderer->resetStartPage();
        ttxdecoder::PageId page = m_renderer->getLastPageId();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0100), page.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->getSetCurrentPageCalls());
    }

    void testResetStartPage_MultipleCallsIdempotent()
    {
        m_renderer->resetStartPage();
        m_renderer->resetStartPage();
        ttxdecoder::PageId page = m_renderer->getLastPageId();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0100), page.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(2, m_renderer->getSetCurrentPageCalls());
    }

private:
    std::unique_ptr<TestBrowserRendererImpl> m_renderer;
};

CPPUNIT_TEST_SUITE_REGISTRATION(BrowserRendererImplTest);