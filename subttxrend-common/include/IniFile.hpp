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


#ifndef SUBTTXREND_COMMON_INIFILE_HPP_
#define SUBTTXREND_COMMON_INIFILE_HPP_

#include <memory>

#include "ConfigProvider.hpp"

namespace subttxrend
{
namespace common
{

class ConfigProviderStorage;

/**
 * Simple INI files parser.
 */
class IniFile : public ConfigProvider
{
public:
    /**
     * Constructor.
     *
     * Creates ini file with no keys.
     */
    IniFile();

    /**
     * Destructor.
     */
    virtual ~IniFile();

    /**
     * Clears all stored contents.
     */
    void clear();

    /**
     * Parses the given file.
     *
     * All previously added contents are removed before parsing the file.
     * The key,value pairs are parsed from the file and stored.
     *
     * @param fileName
     *      Name of the file to parse.
     *
     * @retval true
     *      File parsed sucessfully.
     * @retval false
     *      File parsing failed.
     *      All keys parsed until the error will be stored!
     */
    bool parse(const std::string& fileName);

    /**
     * Parses the given file and appends to existing configuration.
     *
     * The key,value pairs are parsed from the file and stored.
     * Previously added contents may be overwritten if found in the new file.
     *
     * @param fileName
     *      Name of the file to parse.
     *
     * @retval true
     *      File parsed sucessfully.
     * @retval false
     *      File parsing failed.
     *      All keys parsed until the error will be stored!
     */
    bool parseAppend(const std::string& fileName);

protected:
    /** @copydoc ConfigProvider::getValue */
    virtual const char* getValue(const std::string& key) const override;

private:
    /** Data storage. */
    std::unique_ptr<ConfigProviderStorage> m_storage;
};

} // namespace common
} // namespace subttxrend

#endif /*SUBTTXREND_COMMON_INIFILE_HPP_*/
