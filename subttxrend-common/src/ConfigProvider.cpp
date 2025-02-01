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


#include "ConfigProvider.hpp"

#include "StringUtils.hpp"
#include "Logger.hpp"

namespace subttxrend
{
namespace common
{

namespace
{

Logger g_logger("Common", "ConfigProvider");

}

bool ConfigProvider::hasValue(const std::string& key) const
{
    return getValue(key);
}

std::string ConfigProvider::get(const std::string& key,
                                const std::string& defaultValue) const
{
    const char* value = getValue(key);

    return value ? value : defaultValue;
}

int ConfigProvider::getInt(const std::string& key,
                           int defaultValue) const
{
    const char* value = getValue(key);

    if (value)
    {
        std::string valueString = StringUtils::trim(value);

        try
        {
            std::size_t pos = 0;

            int value = std::stoi(valueString, &pos, 0);
            if (pos == valueString.length())
            {
                return value;
            }
        }
        catch (...)
        {
            g_logger.warning("%s - Not an integer value: %s", __func__,
                    valueString.c_str());
        }
    }
    return defaultValue;
}

std::vector<std::string> ConfigProvider::getArray(const std::string& key,
                                                  const std::string& separator) const
{
    std::vector<std::string> values;

    if (separator.length() > 0)
    {
        const char* value = getValue(key);

        if (value)
        {
            std::string valueString(value);

            for (std::string::size_type pos = 0;;)
            {
                std::string itemValue;

                std::string::size_type nextPos = valueString.find(separator,
                        pos);
                if (nextPos == std::string::npos)
                {
                    itemValue = valueString.substr(pos);
                    if (itemValue.length() > 0)
                    {
                        values.push_back(itemValue);
                    }
                    break;
                }
                else
                {
                    itemValue = valueString.substr(pos, nextPos - pos);
                    if (itemValue.length() > 0)
                    {
                        values.push_back(itemValue);
                    }

                    pos = nextPos + separator.length();
                }
            }
        }
    }
    else
    {
        g_logger.warning("%s - Empty separator given, ignoring call", __func__);
    }

    return values;
}

const char* ConfigProvider::getCstr(const std::string& key,
                                    const char* defaultValue) const
{
    const char* value = getValue(key);

    return value ? value : defaultValue;
}

} // namespace common
} // namespace subttxrend
