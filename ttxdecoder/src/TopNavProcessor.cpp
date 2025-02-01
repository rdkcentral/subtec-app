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


#include "TopNavProcessor.hpp"

#include <subttxrend/common/Logger.hpp>

#include "DecodedPage.hpp"
#include "PageAll.hpp"
#include "Database.hpp"

namespace ttxdecoder
{

namespace
{

subttxrend::common::Logger g_logger("TtxDecoder", "TopNavProcessor");

} // namespace <anonymous>

TopNavProcessor::TopNavProcessor(Database& database) :
        m_database(database)
{
    // noop
}

void TopNavProcessor::reset()
{
    g_logger.trace("%s", __func__);
}

Page* TopNavProcessor::getPageBuffer(const PageId& pageId)
{
    if (pageId.getMagazinePage() == BTT_PAGE_ID)
    {
        m_pageBtt.invalidate();
        return &m_pageBtt;
    }
    else
    {
        return nullptr;
    }
}

void TopNavProcessor::processPage(const Page& page)
{
    switch (page.getType())
    {
    case PageType::BTT:
        processTypedPage(static_cast<const PageBtt&>(page));
        break;

    default:
        break;
    }
}

void TopNavProcessor::processTypedPage(const PageBtt& collectedPage)
{
    g_logger.trace("%s", __func__);

    if (setTypes(collectedPage))
    {
        fillPrevPage();
        fillNextPage();
        fillNextGroup();
        fillNextBlock();

        g_logger.trace("%s - database ready", __func__);
    }
    else
    {
        m_database.resetTopMetadata();
    }
}

bool TopNavProcessor::setTypes(const PageBtt& collectedPage)
{
    std::size_t pageIndex = 0;

    for (std::uint8_t row = 1; row <= 20; ++row)
    {
        auto packet = collectedPage.getPageTypePacket(row);
        if (!packet)
        {
            g_logger.warning("%s - cannot get row %d buffer", __func__, row);
            return false;
        }

        auto data = packet->getBuffer();

        for (int tens = 0; tens < 4; ++tens)
        {
            for (int units = 0; units < 10; ++units)
            {
                if (*data < 0)
                {
                    return false;
                }

                m_pageTypes[pageIndex++] = static_cast<BttPageType>(*data++);
            }
        }
    }

    if (m_pageTypes[0] == BTT_NO_PAGE)
    {
        m_pageTypes[0] = BTT_BLOCK_M;
    }

    return true;
}

void TopNavProcessor::fillPrevPage()
{
    int prevPage = 0;

    for (int i = 799; i >= 0; --i)
    {
        if (isNavigableType(m_pageTypes[i]))
        {
            prevPage = i;
            break;
        }
    }
    for (int i = 0; i < 800; ++i)
    {
        auto& topMetadata = m_database.getTopMetatadata(indexToHexPage(i));

        topMetadata.m_prevPage = indexToHexPage(prevPage);

        if (isNavigableType(m_pageTypes[i]))
        {
            prevPage = i;
        }
    }
}

void TopNavProcessor::fillNextPage()
{
    int nextPage = 0;

    for (int i = 0; i < 800; ++i)
    {
        if (isNavigableType(m_pageTypes[i]))
        {
            nextPage = i;
            break;
        }
    }
    for (int i = 799; i >= 0; --i)
    {
        auto& topMetadata = m_database.getTopMetatadata(indexToHexPage(i));

        topMetadata.m_nextPage = indexToHexPage(nextPage);

        if (isNavigableType(m_pageTypes[i]))
        {
            nextPage = i;
        }
    }
}

void TopNavProcessor::fillNextGroup()
{
    int nextGroupPage = 0;

    for (int i = 0; i < 800; ++i)
    {
        if (isGroupType(m_pageTypes[i]))
        {
            nextGroupPage = i;
            break;
        }
    }

    for (int i = 799; i >= 0; --i)
    {
        auto& topMetadata = m_database.getTopMetatadata(indexToHexPage(i));

        topMetadata.m_nextGroupPage = indexToHexPage(nextGroupPage);

        if (isGroupType(m_pageTypes[i]))
        {
            nextGroupPage = i;
        }
    }
}

void TopNavProcessor::fillNextBlock()
{
    int nextBlockPage = 0;

    for (int i = 0; i < 800; ++i)
    {
        if (isBlockType(m_pageTypes[i]))
        {
            nextBlockPage = i;
            break;
        }
    }

    for (int i = 799; i >= 0; --i)
    {
        auto& topMetadata = m_database.getTopMetatadata(indexToHexPage(i));

        topMetadata.m_nextBlockPage = indexToHexPage(nextBlockPage);

        if (isBlockType(m_pageTypes[i]))
        {
            nextBlockPage = i;
        }
    }
}

int TopNavProcessor::hexPageToindex(uint16_t hexPage) const
{
    int d0 = (hexPage >> 0) & 0x0F;
    int d1 = (hexPage >> 4) & 0x0F;
    int d2 = (hexPage >> 8) & 0x0F;

    return ((d2 * 100) + (d1 * 10) + d0) - 100;
}

uint16_t TopNavProcessor::indexToHexPage(int index) const
{
    int pageNumber = index + 100;
    int d0 = (pageNumber / 1) % 10;
    int d1 = (pageNumber / 10) % 10;
    int d2 = (pageNumber / 100) % 10;
    return (d2 << 8) | (d1 << 4) | d0;
}

bool TopNavProcessor::isGroupType(BttPageType type)
{
    return (type == BTT_GROUP_M) || (type == BTT_GROUP_S);
}

bool TopNavProcessor::isBlockType(BttPageType type)
{
    return (type == BTT_BLOCK_M) || (type == BTT_BLOCK_S);
}

bool TopNavProcessor::isNavigableType(BttPageType type)
{
    return (type != BTT_NO_PAGE) /*&& (type != BTT_SUBTITLE)*/;
}

} // namespace ttxdecoder
