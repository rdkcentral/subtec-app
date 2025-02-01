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

#include "ScteSectionAssembler.hpp"

#include <subttxrend/common/Logger.hpp>

#include <utility>
#include <algorithm>

namespace subttxrend
{
namespace scte
{

namespace
{
common::Logger g_logger("Scte", "ScteSectionAssembler");
} // namespace
void SectionAssembler::pushSection(const Section &section)
{
    g_logger.trace("pushing section: %u  %u (isSegmented=%s)",
        section.tableId(),
        section.sectionLength(),
        section.isSegmented() ? "True" : "False");

    if (section.isSegmented())
    {
        auto table_ext = section.segmentationOverlay().table_extension;
        m_input[table_ext].emplace_back(section);
        verifySegmentedMessage(table_ext);
    }
    else
    {
        pushTable(section.data().data(), section.data().size());
    }
}

ScteTablePtr SectionAssembler::provideData()
{
    ScteTablePtr table;

    g_logger.trace("%s output size %zu", __LOGGER_FUNC__, m_output.size());

    if (m_output.size())
    {
        table = std::move(m_output.front());
        m_output.pop();
    }
    return table;
}

void SectionAssembler::pushTable(const uint8_t *data, size_t size)
{
    try
    {
        ScteTablePtr ptr{new ScteTable(TABLE_ID_SCTE_SUB, data, size)};
        g_logger.debug("%s adding table %p (total=%zu)", __LOGGER_FUNC__, ptr.get(), m_output.size());
        m_output.emplace(std::move(ptr));
    }
    catch(const ParseError &error)
    {
        g_logger.error("Error during parsing section: %s", error.what());
    }
}

void SectionAssembler::assembleTable(uint16_t table_extension)
{
    g_logger.debug("%s %u ", __LOGGER_FUNC__, table_extension);

    const auto &secs = m_input[table_extension];
    std::vector<uint8_t> raw_table;
    raw_table.reserve(secs.front().data().size() * secs.size());
    for (unsigned i = 0; i < secs.size(); ++i)
    {
        raw_table.insert(raw_table.end(), secs[i].data().data(), secs[i].data().data() + secs[i].data().size());
    }
    pushTable(raw_table.data(), raw_table.size());
    m_input.erase(table_extension);
}

void SectionAssembler::verifySegmentedMessage(uint16_t table_extension)
{
    sortSections(m_input[table_extension]);

    const auto &secs = m_input[table_extension];
    bool firstOk = secs.front().segmentationOverlay().segment_number == 0;
    bool lastOk = secs.back().segmentationOverlay().segment_number == secs.back().segmentationOverlay().last_segment_number;
    bool countOk = secs.back().segmentationOverlay().last_segment_number + 1u == secs.size();
    bool sizeOk = true;
    auto segment_size = secs.front().data().size();

    for (auto &val : secs)
    {
        //message segments of different size, scrapping sequence
        if(val.data().size() != segment_size)
        {
            g_logger.warning("%s dropping table: %u data size: %zu segment_size %zu count %zu",
                __LOGGER_FUNC__,
                table_extension,
                val.data().size(),
                segment_size,
                secs.size());

            sizeOk = false;
            m_input.erase(table_extension);
        }
    }
    if (firstOk && lastOk && countOk && sizeOk)
    {
        assembleTable(table_extension);
    }
}

void SectionAssembler::sortSections(std::vector<Section> sections)
{
    auto sortFun = [](const Section &l, const Section &r)
    {
        return l.segmentationOverlay().segment_number < r.segmentationOverlay().segment_number;
    };
    std::sort(sections.begin(), sections.end(), sortFun);
}

void SectionAssembler::clear()
{
    m_input.clear();
    decltype(m_output)().swap(m_output);
}

} // namespace scte
} // namespace subttxtrend
