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


#include "AttributeHandlers.hpp"

namespace subttxrend
{
namespace ttmlengine
{

void mergeAttributes(Attributes& destination, const Attributes& source)
{
    for (const auto& attr : source)
    {
        const auto result = destination.insert({attr.first, attr.second});
        if (!result.second)
        {
            result.first->second = attr.second;
        }
    }
}

std::ostream& operator<<(std::ostream& stream, const Attributes& attrSet)
{
    for (const auto& attr : attrSet)
    {
        stream << "[" << attr.first << ":" << attr.second << "], ";
    }
    return stream;
}

const Attributes& StyleAttributeHandler::getStyleAttributes()
{
    return m_styleAttributes;
}

void StyleAttributeHandler::addAttribute(const std::string& name, const std::string& value)
{
    const auto result = m_styleAttributes.insert({name, value});
    if (!result.second)
    {
        result.first->second = value;
    }
}

void StyleAttributeHandler::set(Attributes attrSet)
{
    m_styleAttributes = std::move(attrSet);
}

} // namespace subttxrend
} // namespace ttmlengine

