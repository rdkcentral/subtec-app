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


#ifndef SUBTTXREND_COMMON_PREFIXCONFIGPROVIDER_HPP_
#define SUBTTXREND_COMMON_PREFIXCONFIGPROVIDER_HPP_

#include <string>
#include <vector>

#include "NonCopyable.hpp"

namespace subttxrend
{
namespace common
{

/**
 * Configuration provider that prefixes all keys of another
 * provider.
 */
class PrefixConfigProvider : public ConfigProvider
{
public:
    /**
     * Constructor.
     *
     * @param keyPrefix
     *      Prefix to be added to all keys.
     * @param peerProvider
     *      Peer configuration provider.
     */
    PrefixConfigProvider(const std::string& keyPrefix,
                         const ConfigProvider& peerProvider) :
            m_keyPrefix(keyPrefix),
            m_peerProvider(peerProvider)
    {
        // noop
    }

    /**
     * Destructor.
     */
    virtual ~PrefixConfigProvider() = default;

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
    virtual const char* getValue(const std::string& key) const override
    {
        return m_peerProvider.getCstr(m_keyPrefix + key);
    }

    /** Prefix to be added to all keys. */
    const std::string& m_keyPrefix;

    /** Peer configuration provider. */
    const ConfigProvider& m_peerProvider;
};

} // namespace common
} // namespace subttxrend

#endif /*SUBTTXREND_COMMON_PREFIXCONFIGPROVIDER_HPP_*/
