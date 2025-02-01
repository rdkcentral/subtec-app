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

#pragma once

#include "ScteSection.hpp"
#include "ScteTable.hpp"

#include <memory>
#include <queue>
#include <cstdint>
#include <unordered_map>

namespace subttxrend
{
namespace scte
{

class SectionAssembler
{
public:
    SectionAssembler() = default;

    ScteTablePtr provideData();
    void pushSection(const Section &section);
    void clear();

private:
    void verifySegmentedMessage(uint16_t table_extension);
    void sortSections(std::vector<Section> sections);
    void assembleTable(uint16_t table_extension);
    void pushTable(const uint8_t *data, size_t size);

private:
    std::unordered_map<uint16_t, std::vector<Section>> m_input;
    std::queue<ScteTablePtr> m_output;
};

} // namespace scte
} // namespace subttxtrend
