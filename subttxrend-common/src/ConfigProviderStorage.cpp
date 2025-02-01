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


#include "ConfigProviderStorage.hpp"

#include <cstring>

namespace subttxrend
{
namespace common
{

void ConfigProviderStorage::clear()
{
    m_values.clear();

    // value strings cannot be cleared as the string pointers must
    // remain valid for the whole lifecycle.
}

void ConfigProviderStorage::addValue(const std::string& key,
                                     const std::string& value)
{
    std::unique_ptr<char[]> valueBuffer(new char[value.length() + 1]);

    auto valueString = valueBuffer.get();

    (void) std::strcpy(valueString, value.c_str());

    m_valueStrings.push_back(std::move(valueBuffer));

    m_values.insert(std::make_pair(key, valueString));
}

const char* ConfigProviderStorage::getValue(const std::string& key) const
{
    auto iter = m_values.find(key);
    if (iter != m_values.end())
    {
        return iter->second;
    }
    else
    {
        return nullptr;
    }
}

} // namespace common
} // namespace subttxrend
