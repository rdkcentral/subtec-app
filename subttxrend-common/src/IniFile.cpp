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


#include "IniFile.hpp"

#include <fstream>

#include "StringUtils.hpp"
#include "Logger.hpp"

#include "ConfigProviderStorage.hpp"

namespace subttxrend
{
namespace common
{

namespace
{

Logger g_logger("Common", "IniFile");

}

IniFile::IniFile() :
        m_storage(new ConfigProviderStorage())
{
    // noop
}

IniFile::~IniFile()
{
    // noop
}

void IniFile::clear()
{
    m_storage->clear();
}

bool IniFile::parse(const std::string& fileName)
{
    clear();

    return parseAppend(fileName);
}

bool IniFile::parseAppend(const std::string& fileName)
{
    std::ifstream file(fileName.c_str(), std::ifstream::in);

    if (file.is_open())
    {
        std::string line;

        while (file.peek(), file.good() && std::getline(file, line))
        {
            line = StringUtils::trim(line);

            g_logger.trace("%s - Ini file line: %s", __func__, line.c_str());

            if ((line.length() == 0) || (line[0] == '#'))
            {
                continue;
            }

            auto sep = line.find('=');
            if (sep == std::string::npos)
            {
                g_logger.warning("%s - Separator missing: %s", __func__,
                        line.c_str());
                break;
            }

            std::string key = StringUtils::trim(line.substr(0, sep));
            std::string value = StringUtils::trim(line.substr(sep + 1));

            if (key.length() == 0)
            {
                g_logger.warning("%s - Key missing: %s", __func__,
                        line.c_str());
                break;
            }

            m_storage->addValue(key, value);
        }

        if (file && file.eof())
        {
            g_logger.trace("%s - Read succeeded: file=%s", __func__,
                    fileName.c_str());
            return true;
        }
        else
        {
            g_logger.warning("%s - Read failed: file=%s fail=%d eof=%d",
                    __func__, fileName.c_str(), file.fail(), file.eof());
        }
    }
    else
    {
        g_logger.warning("%s - Cannot open file: %s", __func__,
                fileName.c_str());
    }

    return false;
}

const char* IniFile::getValue(const std::string& key) const
{
    return m_storage->getValue(key);
}

} // namespace common
} // namespace subttxrend
