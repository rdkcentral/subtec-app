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


#include "Configuration.hpp"

#include "Options.hpp"

#include <subttxrend/common/Logger.hpp>
#include <subttxrend/common/StringUtils.hpp>

namespace subttxrend
{
namespace ctrl
{

namespace
{

/**
 * Configuration entry definition.
 */
struct ConfigEntry
{
    /** Ini file key. */
    const std::string m_key;

    /** Default value. */
    const char* const m_defaultValue;

    /**
     * Constructor.
     *
     * @param key
     *      Ini file key.
     * @param defaultValue
     *      Default value.
     */
    ConfigEntry(const std::string& key,
                const char* const defaultValue) :
            m_key(key),
            m_defaultValue(defaultValue)
    {
        // noop
    }
};

const std::string MAIN_CONTEXT_SOCKET_PATH_KEY("MAIN_CONTEXT.SOCKET_PATH");
const std::string TELETEXT_PREFIX("TELETEXT.");
const std::string LOGGER_PREFIX("LOGGER.");
const std::string RDKENV_PREFIX("RDKENV.");
const std::string TTML_PREFIX("TTML.");
const std::string WEBVTT_PREFIX("WEBVTT.");

#ifndef PC_BUILD
const ConfigEntry MAIN_CONTEXT_SOCKET_PATH_ENTRY(MAIN_CONTEXT_SOCKET_PATH_KEY,
        "/var/run/subttx/pes_data_main");
#else // PC_BUILD
const ConfigEntry MAIN_CONTEXT_SOCKET_PATH_ENTRY(MAIN_CONTEXT_SOCKET_PATH_KEY,
        "/tmp/subttx-socket");
#endif // PC_BUILD

const ConfigEntry DEFAULT_ENTRIES[] =
{
// Teletext related settings
// - window size (720p)
{ "RDKENV.GFX.VL.APP.1.WIDTH", "1280" },
{ "RDKENV.GFX.VL.APP.1.HEIGHT", "720" },
// - grid size (height for 720p - borders, width to get good aspect ratio)
{ "RDKENV.FEATURE.TTX.GRID_W", "880" },
{ "RDKENV.FEATURE.TTX.GRID_H", "650" },
// - main font
{ "RDKENV.FEATURE.TTX.FONT.G0.NAME", "Bitstream Vera Sans Mono Bold" },
{ "RDKENV.FEATURE.TTX.FONT.G0.GLYPH_WIDTH", "22" },
{ "RDKENV.FEATURE.TTX.FONT.G0.GLYPH_HEIGHT", "26" },
{ "RDKENV.FEATURE.TTX.FONT.G0.CHAR_WIDTH", "34" },
{ "RDKENV.FEATURE.TTX.FONT.G0.CHAR_HEIGHT", "24" },
// - other settings
{ "RDKENV.FEATURE.TTX.FLASH_PERIOD_MS", "1000" },
{ "RDKENV.FEATURE.TTX.BG_ALPHA", "255" },
{ "RDKENV.FEATURE.TTX.RESET_ON_STOP","TRUE" },
// Subtitles related settings
{ "RDKENV.GFX.VL.APP.2.WIDTH", "0" },
{ "RDKENV.GFX.VL.APP.2.HEIGHT", "0" },
// Logging settings
{ "LOGGER.BACKEND", "rdk" },
{ "LOGGER.BACKEND_RDK_CONFIG_FILE", "/etc/debug.ini" },
{ "LOGGER.LEVELS_DEFAULT", "FATAL ERROR WARNING INFO DEBUG TRACE" } };

common::Logger g_logger("App", "Configuration");

}

Configuration::Configuration(const Options& options) :
        m_options(options),
        m_teletextConfigProvider(TELETEXT_PREFIX, *this),
        m_rdkEnvConfigProvider(RDKENV_PREFIX, *this),
        m_loggerConfigProvider(LOGGER_PREFIX, *this),
        m_ttmlConfigProvider(TTML_PREFIX, *this),
        m_webvttConfigProvider(WEBVTT_PREFIX, *this)
{
    auto configFileName = options.getOptionValue(
            Options::Key::CONFIG_FILE_PATH);
    if (configFileName.size() > 0)
    {
        if (!m_configFile.parse(configFileName))
        {
            g_logger.warning("%s - Config file parsing failed: %s", __func__,
                    configFileName.c_str());
        }
    }
    else
    {
        g_logger.info("%s - Config file not specified", __func__);
    }
}

std::string Configuration::getMainContextSocketPath() const
{
    std::string value = m_options.getOptionValue(
            Options::Key::MAIN_SOCKET_PATH);
    if (value.size() == 0)
    {
        value = m_configFile.get(MAIN_CONTEXT_SOCKET_PATH_ENTRY.m_key,
                MAIN_CONTEXT_SOCKET_PATH_ENTRY.m_defaultValue);
    }

    return common::StringUtils::trim(value);
}

const char* Configuration::getValue(const std::string& key) const
{
    for (const auto& entry : DEFAULT_ENTRIES)
    {
        if (entry.m_key == key)
        {
            return m_configFile.getCstr(key, entry.m_defaultValue);
        }
    }

    return m_configFile.getCstr(key);
}

} // namespace ctrl
}
// namespace subttxrend
