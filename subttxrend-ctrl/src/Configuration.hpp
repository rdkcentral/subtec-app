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


#ifndef SUBTTXREND_APP_CONFIGURATION_HPP_
#define SUBTTXREND_APP_CONFIGURATION_HPP_

#include <subttxrend/common/IniFile.hpp>
#include <subttxrend/common/ConfigProvider.hpp>
#include <subttxrend/common/PrefixConfigProvider.hpp>

namespace subttxrend
{
namespace app
{

class Options;

/**
 * Application configuration.
 */
class Configuration : private common::ConfigProvider
{
public:
    /**
     * Constructor.
     *
     * @param options
     *      Command line options.
     */
    Configuration(const Options& options);

    /**
     * Destructor.
     */
    ~Configuration() = default;

    /**
     * Returns main context socket path.
     *
     * @return
     *      Main context socket path.
     */
    std::string getMainContextSocketPath() const;

    /**
     * Returns teletext configuration.
     *
     * @return
     *      Configuration provider.
     */
    const common::ConfigProvider& getTeletextConfig() const
    {
        return m_teletextConfigProvider;
    }

    /**
     * Returns RDK environment configuration.
     *
     * @return
     *      Configuration provider.
     */
    const common::ConfigProvider& getRdkEnvConfig() const
    {
        return m_rdkEnvConfigProvider;
    }

    /**
     * Returns logger configuration.
     *
     * @return
     *      Configuration provider.
     */
    const common::ConfigProvider& getLoggerConfig() const
    {
        return m_loggerConfigProvider;
    }

    /**
     * Returns TTML configuration.
     *
     * @return
     *      Configuration provider.
     */
    const common::ConfigProvider& getTtmlConfig() const
    {
        return m_ttmlConfigProvider;
    }
    
    /**
     * Returns WebVTT configuration.
     *
     * @return
     *      Configuration provider.
     */
    const common::ConfigProvider& getWebvttConfig() const
    {
        return m_webvttConfigProvider;
    }

private:
    /**
     * Returns value for given key.
     *
     * @param key
     *      Configuration key.
     *
     * @return
     *      Value for given key or nullptr if not available.
     */
    virtual const char* getValue(const std::string& key) const override;

private:
    /** Command line options. */
    const Options& m_options;

    /** Configuration ini file contents. */
    common::IniFile m_configFile;

    /** Teletext config provider. */
    common::PrefixConfigProvider m_teletextConfigProvider;

    /** RDK environment config provider. */
    common::PrefixConfigProvider m_rdkEnvConfigProvider;

    /** Logger config provider. */
    common::PrefixConfigProvider m_loggerConfigProvider;

    /** Logger config provider. */
    common::PrefixConfigProvider m_ttmlConfigProvider;

    /** Logger config provider. */
    common::PrefixConfigProvider m_webvttConfigProvider;
};

} // namespace app
} // namespace subttxrend

#endif /*SUBTTXREND_APP_CONFIGURATION_HPP_*/
