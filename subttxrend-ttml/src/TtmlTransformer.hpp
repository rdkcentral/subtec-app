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

#include <string>
#include <vector>
#include "Parser/IntermediateDocument.hpp"
#include <subttxrend/common/Properties.hpp>

namespace subttxrend
{
namespace ttmlengine
{

enum class Style
{
    NORMAL,
    BLACK_BACKGROUND,
    OUTLINE
};

enum class TrackType
{
    NORMAL,
    HARD_OF_HEARING,
    ANY
};

struct SubtitleInfo
{
    std::string language;
    TrackType type {TrackType::NORMAL};
};

class TtmlTransformer final
{
public:
    void setProperties(common::Properties const& properties);
    void setSubtitleInfo(const std::string& contentType, const std::string& subsInfo);
    void transform(IntermediateDocument& doc);

private:
    void setRules(std::vector<std::string> const& rules);
    Style getPreferredStyle();

private:
    bool m_applyTransform{};
    std::vector<std::pair<SubtitleInfo, Style>> m_rules;
    SubtitleInfo m_currentSubtitleInfo;
};

} // namespace ttmlengine
} // namespace subttxrend
