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


#ifndef SUBTTXREND_COMMON_CONFIGPROVIDERSTORAGE_HPP_
#define SUBTTXREND_COMMON_CONFIGPROVIDERSTORAGE_HPP_

#include <map>
#include <string>
#include <vector>
#include <memory>

#include "NonCopyable.hpp"

namespace subttxrend
{
namespace common
{

/**
 * Configuration provider storage.
 */
class ConfigProviderStorage : NonCopyable
{
public:
    /**
     * Constructor.
     */
    ConfigProviderStorage() = default;

    /**
     * Destructor.
     */
    ~ConfigProviderStorage() = default;

    /**
     * Clears all stored key,value pairs.
     */
    void clear();

    /**
     * Adds key,value pair.
     *
     * @param key
     *      Configuration key.
     * @param value
     *      Configuration value.
     */
    void addValue(const std::string& key,
                  const std::string& value);

    /**
     * Returns value for given key.
     *
     * @param key
     *      Configuration key.
     *
     * @return
     *      Value for given key or nullptr if not available.
     */
    const char* getValue(const std::string& key) const;

private:
    /** Value strings. */
    std::vector<std::unique_ptr<char[]>> m_valueStrings;

    /** Key,value pairs. */
    std::map<std::string, const char*> m_values;
};

} // namespace common
} // namespace subttxrend

#endif /*SUBTTXREND_COMMON_CONFIGPROVIDERSTORAGE_HPP_*/
