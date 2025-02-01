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


#ifndef SUBTTXREND_COMMON_CONFIGPROVIDER_HPP_
#define SUBTTXREND_COMMON_CONFIGPROVIDER_HPP_

#include <string>
#include <vector>

#include "NonCopyable.hpp"

namespace subttxrend
{
namespace common
{

/**
 * Configuration provider.
 */
class ConfigProvider : NonCopyable
{
public:
    /**
     * Constructor.
     *
     * Creates configuration provider no contents.
     */
    ConfigProvider() = default;

    /**
     * Destructor.
     */
    virtual ~ConfigProvider() = default;

    /**
     * Checks if value for given key exists.
     *
     * @param key
     *      Configuration key.
     *
     * @retval true
     *      Configuration value exists.
     * @retval false
     *      Configuration value is not set.
     */
    bool hasValue(const std::string& key) const;

    /**
     * Returns value for the given key.
     *
     * @param key
     *      Configuration key.
     * @param defaultValue
     *      Default value to return if mapping is not available in config.
     *
     * @return
     *      Value for given key or defaultValue if not available.
     */
    std::string get(const std::string& key,
                    const std::string& defaultValue = std::string()) const;

    /**
     * Returns value for the given key.
     *
     * @param key
     *      Configuration key.
     * @param defaultValue
     *      Default value to return if mapping is not available in config.
     *
     * @return
     *      Value for given key as an array or empty array if not available.
     */
    int getInt(const std::string& key,
               int defaultValue = -1) const;

    /**
     * Returns value for the given key.
     *
     * @param key
     *      Configuration key.
     * @param separator
     *      Separator between the items.
     *
     * @return
     *      Value for given key as an array or empty array if not available.
     */
    std::vector<std::string> getArray(const std::string& key,
                                      const std::string& separator) const;

    /**
     * Returns value for the given key.
     *
     * @param key
     *      Configuration key.
     * @param defaultValue
     *      Default value to return if mapping is not available in config.
     *
     * @return
     *      Value for given key or defaultValue if not available.
     */
    const char* getCstr(const std::string& key,
                        const char* defaultValue = nullptr) const;

protected:
    /**
     * Returns value for given key.
     *
     * @param key
     *      Configuration key.
     *
     * @return
     *      Value for given key or nullptr if not available.
     */
    virtual const char* getValue(const std::string& key) const = 0;
};

} // namespace common
} // namespace subttxrend

#endif /*SUBTTXREND_COMMON_CONFIGPROVIDER_HPP_*/
