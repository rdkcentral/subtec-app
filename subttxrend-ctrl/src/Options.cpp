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


#include "Options.hpp"

#include <iostream>

namespace subttxrend
{
namespace ctrl
{

Options::Options(int argc,
                 char* argv[]) :
        m_optionsValid(false),
        m_separate()
{
    addEntry(
            Entry(Key::HELP, "--help", "-h",
                    "Shows usage information and exits.", Mode::SEPARATE));
    addEntry(
            Entry(Key::MAIN_SOCKET_PATH, "--main-socket-path", "-msp",
                    "Main context unix socket path.", Mode::OPTIONAL, ""));
    addEntry(
            Entry(Key::CONFIG_FILE_PATH, "--config-file-path", "-cfp",
                    "Configuration file path.", Mode::OPTIONAL,
                    "/etc/subttxrend/config.ini"));

    m_optionsValid = parseOptions(argc, argv);
    if (m_optionsValid)
    {
        m_optionsValid = validateOptions();
    }

    if (!m_optionsValid)
    {
        std::cerr << "Options parsing failed." << std::endl;
    }
    else
    {
        setSeparateOption();
        setDefaultValues();
    }
}

bool Options::isValid() const
{
    return m_optionsValid;
}

bool Options::hasSeparate() const
{
    return m_separate.m_defined;
}

Options::Key Options::getSeparate() const
{
    return m_separate.m_key;
}

std::string Options::getOptionValue(Key key) const
{
    const auto& iter = m_values.find(key);
    if (iter != m_values.end())
    {
        return iter->second;
    }
    else
    {
        return std::string();
    }
}

void Options::printUsage() const
{
    std::cout << std::endl;
    std::cout << "Usage information:" << std::endl;
    std::cout << std::endl;

    /* print separate options */

    std::cout << "Options to be used separately:" << std::endl;
    for (const auto& item : m_entries)
    {
        if (item.m_mode == Mode::SEPARATE)
        {
            std::cout << item.m_longName << " | " << item.m_shortName
                    << std::endl;
            std::cout << "    " << item.m_description << std::endl;
        }
    }
    std::cout << std::endl;

    /* print options */

    std::cout << "Configuration options:" << std::endl;
    for (const auto& item : m_entries)
    {
        if (item.m_mode == Mode::SEPARATE)
        {
            continue;
        }

        std::cout << item.m_longName << " | " << item.m_shortName << std::endl;
        std::cout << "    " << item.m_description << std::endl;
        if (item.m_mode == Mode::REQUIRED)
        {
            std::cout << "        Required." << std::endl;
        }
        if (!item.m_defaultValue.empty())
        {
            std::cout << "        Default:  " << item.m_defaultValue
                    << std::endl;
        }
    }
    std::cout << std::endl;
}

void Options::addEntry(const Entry& entry)
{
    m_entries.push_back(entry);
}

Options::EntryCollection::const_iterator Options::findEntry(const std::string& name) const
{
    auto current = m_entries.begin();

    for (; current != m_entries.end(); ++current)
    {
        if ((current->m_longName == name) || (current->m_shortName == name))
        {
            break;
        }
    }

    return current;
}

bool Options::parseOptions(int argc,
                           char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i])
        {
            if (!parseOption(argv[i]))
            {
                std::cerr << "Invalid option: '" << argv[i] << "'."
                        << std::endl;
                return false;
            }
        }
    }

    return true;
}

bool Options::validateOptions() const
{
    return validateSeparateOptions() && validateRequiredOptions();
}

bool Options::validateSeparateOptions() const
{
    bool valid = true;

    for (const auto& item : m_entries)
    {
        if (item.m_mode == Mode::SEPARATE)
        {
            if (hasValue(item))
            {
                if (getValuesCount() != 1)
                {
                    valid = false;

                    std::cerr << "Option " << item.m_longName
                            << " must be specified separately." << std::endl;
                }
            }
        }
    }

    return valid;
}

bool Options::validateRequiredOptions() const
{
    bool valid = true;

    for (const auto& item : m_entries)
    {
        if (item.m_mode == Mode::REQUIRED)
        {
            if (!hasValue(item))
            {
                valid = false;

                std::cerr << "Option " << item.m_longName << " requires value."
                        << std::endl;
            }
        }
    }

    return valid;
}

void Options::setSeparateOption()
{
    for (const auto& item : m_entries)
    {
        if (item.m_mode == Mode::SEPARATE)
        {
            if (hasValue(item))
            {
                m_separate.m_defined = true;
                m_separate.m_key = item.m_key;
                break;
            }
        }
    }
}

void Options::setDefaultValues()
{
    for (const auto& item : m_entries)
    {
        if (!hasValue(item))
        {
            setValue(item, item.m_defaultValue);
        }
    }
}

bool Options::parseOption(const std::string& cmdValue)
{
    std::string value;
    EntryCollection::const_iterator entryIter = m_entries.end();

    auto index = cmdValue.find('=');
    if (index == std::string::npos)
    {
        std::string key = cmdValue;

        entryIter = findEntry(key);
        if ((entryIter != m_entries.end())
                && (entryIter->m_mode != Mode::SEPARATE))
        {
            std::cerr << "Given option requires a value." << std::endl;
            return false;
        }
    }
    else
    {
        std::string key = cmdValue.substr(0, index);
        value = cmdValue.substr(index + 1);

        entryIter = findEntry(key);
        if ((entryIter != m_entries.end())
                && (entryIter->m_mode == Mode::SEPARATE))
        {
            std::cerr << "Given option does not accept values." << std::endl;
            return false;
        }
    }

    if (entryIter == m_entries.end())
    {
        std::cerr << "Unknown option." << std::endl;
        return false;
    }

    if (hasValue(*entryIter))
    {
        std::cerr << "Option specified twice." << std::endl;
        return false;
    }

    setValue(*entryIter, value);

    return true;
}

std::size_t Options::getValuesCount() const
{
    return m_values.size();
}

bool Options::hasValue(const Entry& entry) const
{
    return m_values.find(entry.m_key) != m_values.end();
}

std::string Options::getValue(const Entry& entry) const
{
    const auto& iter = m_values.find(entry.m_key);
    if (iter != m_values.end())
    {
        return iter->second;
    }
    else
    {
        return std::string();
    }
}

void Options::setValue(const Entry& entry,
                       const std::string& value)
{
    m_values[entry.m_key] = value;
}

} // namespace ctrl
} // namespace subttxrend
