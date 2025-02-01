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


#include "DecodedPage.hpp"

#include <stdexcept>

#include <subttxrend/common/Logger.hpp>

namespace
{

subttxrend::common::Logger g_logger("TtxDecoder", "DecodedPage");

} // namespace <anonymous>

namespace ttxdecoder
{

DecodedPage::DecodedPage() :
        m_pageId(),
        m_controlInfo(),
        m_rows()
{
    clear();
}

void DecodedPage::clear()
{
    m_pageId = PageId();

    m_controlInfo = 0;

    for (auto& row : m_rows)
    {
        row.clear();
    }

    m_colourKeyLinks.fill(PageId());
}

PageId DecodedPage::getPageId() const
{
    return m_pageId;
}

void DecodedPage::setPageId(PageId pageId)
{
    m_pageId = pageId;
}

std::uint8_t DecodedPage::getPageControlInfo() const
{
    return m_controlInfo;
}

void DecodedPage::setPageControlInfo(std::uint8_t controlInfo)
{
    m_controlInfo = controlInfo;
}

std::size_t DecodedPage::getRowCount() const
{
    return m_rows.size();
}

DecodedPageRow& DecodedPage::getRow(std::size_t row)
{
    if (row >= m_rows.size())
    {
        throw std::invalid_argument("Invalid row index");
    }

    return m_rows[row];
}

const DecodedPageRow& DecodedPage::getRow(std::size_t row) const
{
    if (row >= m_rows.size())
    {
        throw std::invalid_argument("Invalid row index");
    }

    return m_rows[row];
}

void DecodedPage::setColourKeyLink(Link link,
                                   const PageId& linkValue)
{
    m_colourKeyLinks[static_cast<int>(link)] = linkValue;
}

const PageId& DecodedPage::getColourKeyLink(Link link) const
{
    return m_colourKeyLinks[static_cast<int>(link)];
}

void DecodedPage::dump(bool charsOnly)
{
    static const char HEX_CHARS[] = "0123456789ABCDEF";

    const std::size_t ROW_WIDTH = 40;

    char line[5 * ROW_WIDTH + ROW_WIDTH + 20];

    g_logger.trace("-----PAGE-----");

    // info
    g_logger.trace("ID: %04hX:%04hX - control: %02X",
            m_pageId.getMagazinePage(), m_pageId.getSubpage(), m_controlInfo);

    // links
    if (!charsOnly)
    {
        char* p = line;

        for (std::size_t index = 0; index < m_colourKeyLinks.size(); ++index)
        {
            PageId link = m_colourKeyLinks[index];

            auto mag = link.getMagazinePage();
            auto sub = link.getSubpage();

            uint8_t mag1 = (mag >> 12) & 0x0F;
            uint8_t mag2 = (mag >> 8) & 0x0F;
            uint8_t mag3 = (mag >> 4) & 0x0F;
            uint8_t mag4 = (mag >> 0) & 0x0F;

            uint8_t sub1 = (sub >> 12) & 0x0F;
            uint8_t sub2 = (sub >> 8) & 0x0F;
            uint8_t sub3 = (sub >> 4) & 0x0F;
            uint8_t sub4 = (sub >> 0) & 0x0F;

            *p++ = ' ';

            *p++ = HEX_CHARS[mag1];
            *p++ = HEX_CHARS[mag2];
            *p++ = HEX_CHARS[mag3];
            *p++ = HEX_CHARS[mag4];

            *p++ = ':';

            *p++ = HEX_CHARS[sub1];
            *p++ = HEX_CHARS[sub2];
            *p++ = HEX_CHARS[sub3];
            *p++ = HEX_CHARS[sub4];
        }

        *p = 0;

        g_logger.trace("Links: %s", line);
    }
    g_logger.trace(" ");

    // characters
    for (std::size_t row = 0; row < m_rows.size(); ++row)
    {
        const auto& str = m_rows[row].m_levelOnePageSegment.m_charArray;
        const auto& data = str;
        const auto length = m_rows[row].m_levelOnePageSegment.getWidth();

        char* p = line;

        if (!charsOnly)
        {
            for (std::size_t i = 0; i < length; ++i)
            {
                uint8_t digit1 = (data[i] >> 12) & 0x0F;
                uint8_t digit2 = (data[i] >> 8) & 0x0F;
                uint8_t digit3 = (data[i] >> 4) & 0x0F;
                uint8_t digit4 = (data[i] >> 0) & 0x0F;
                *p++ = HEX_CHARS[digit1];
                *p++ = HEX_CHARS[digit2];
                *p++ = HEX_CHARS[digit3];
                *p++ = HEX_CHARS[digit4];
                *p++ = ' ';
            }
        }

        *p++ = '-';
        *p++ = ' ';

        for (std::size_t i = 0; i < length; ++i)
        {
            if ((str[i] >= 0x20) && (str[i] <= 0x7F))
            {
                *p++ = str[i];
            }
            else
            {
                *p++ = '?';
            }
        }

        *p++ = ' ';
        *p++ = '-';

        *p = 0;

        g_logger.trace("%s", line);
    }
    g_logger.trace(" ");

    // colors
    if (!charsOnly)
    {
        for (std::size_t row = 0; row < m_rows.size(); ++row)
        {
            const auto& bg = m_rows[row].m_levelOnePageSegment.m_bgColorIndexArray;
            const auto& fg = m_rows[row].m_levelOnePageSegment.m_fgColorIndexArray;
            const auto length = m_rows[row].m_levelOnePageSegment.getWidth();

            char* p = line;

            for (std::size_t i = 0; i < length; ++i)
            {
                uint8_t digit1 = (bg[i] >> 4) & 0x0F;
                uint8_t digit2 = (bg[i] >> 0) & 0x0F;
                *p++ = HEX_CHARS[digit1];
                *p++ = HEX_CHARS[digit2];
                *p++ = ' ';
            }

            *p++ = '-';
            *p++ = ' ';

            for (std::size_t i = 0; i < length; ++i)
            {
                uint8_t digit1 = (fg[i] >> 4) & 0x0F;
                uint8_t digit2 = (fg[i] >> 0) & 0x0F;
                *p++ = HEX_CHARS[digit1];
                *p++ = HEX_CHARS[digit2];
                *p++ = ' ';
            }

            *p = 0;

            g_logger.trace("%s", line);
        }
    }

    // properties
    if (!charsOnly)
    {
        for (std::size_t row = 0; row < m_rows.size(); ++row)
        {
            const auto& props = m_rows[row].m_levelOnePageSegment.m_propertiesArray;
            const auto length = m_rows[row].m_levelOnePageSegment.getWidth();

            char* p = line;
            for (std::size_t i = 0; i < length; ++i)
            {
                uint8_t digit1 = (props[i] >> 12) & 0x0F;
                uint8_t digit2 = (props[i] >> 8) & 0x0F;
                uint8_t digit3 = (props[i] >> 4) & 0x0F;
                uint8_t digit4 = (props[i] >> 0) & 0x0F;
                *p++ = HEX_CHARS[digit1];
                *p++ = HEX_CHARS[digit2];
                *p++ = HEX_CHARS[digit3];
                *p++ = HEX_CHARS[digit4];
                *p++ = ' ';
            }
            *p = 0;

            g_logger.trace("%s", line);
        }
    }

    g_logger.trace("-----END-OF-PAGE-----");
}

} // namespace ttxdecoder
