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


#include "JsonData.hpp"
#include "Logger.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace subttxrend
{
namespace common
{

namespace
{
common::Logger logger("Common", "JsonData");
}

class JsonData::Impl final
{
  public:
    Impl(std::string const& json)
    {
        auto cutPos = json.find("{");    // json should start with bracket
        if (cutPos == std::string::npos) {
            return;
        }

        try {
            std::istringstream jsonStream{std::string(json, cutPos)};
            read_json(jsonStream, m_data);
            m_valid = true;
        } catch (std::exception const& e) {
            logger.error("%s json parsing error: %s ", __LOGGER_FUNC__, e.what());
        }
    }

    bool isValid() const noexcept
    {
        return m_valid;
    }

    bool check(const std::string& path) const noexcept
    {
        auto pathPresent = true;
        try {
            m_data.get<std::string>(path);
        } catch (std::exception const& e) {
            pathPresent = false;
        }
        return pathPresent;
    }

    std::string get(std::string const& path) const noexcept
    {
        std::string value;
        try {
            value = m_data.get<std::string>(path);
        } catch (std::exception const& e) {
            logger.error("%s couldn't get %s: %s", __LOGGER_FUNC__, path.c_str(), e.what());
        }
        return value;
    }

    std::string getArray(const std::string& path, std::string const& delimiter) const noexcept
    {
        std::string value;
        try {
            for (auto& item : m_data.get_child(path)) {
                value += item.second.data() + delimiter;
            }
            // remove last delimiter
            if (!value.empty()){
                value.pop_back();
            }
        } catch (std::exception const& e) {
            logger.error("%s couldn't get %s: %s", __LOGGER_FUNC__, path.c_str(), e.what());
        }
        return value;
    }

    std::vector<std::string> getArray(std::string const& path, std::string const& delimiter, std::initializer_list<std::string> nodes) const noexcept
    {
        std::vector<std::string> values;
        try {
            for (auto& item : m_data.get_child(path)) {
                std::string value;
                for (auto node : nodes) {
                    try {
                        value += item.second.get<std::string>(node);
                    } catch (std::exception const& e) {
                        //noop
                    }
                    value += delimiter;
                }
                // remove last delimiter
                if (!value.empty()){
                    value.pop_back();
                }
                values.push_back(value);
            }
        } catch (std::exception const& e) {
            logger.error("%s couldn't get %s: %s", __LOGGER_FUNC__, path.c_str(), e.what());
        }

        return values;
    }


  private:
    bool m_valid{};
    boost::property_tree::ptree m_data;
};

JsonData::JsonData(std::string const& json)
    : impl{std::make_unique<Impl>(json)}
{
}

JsonData::~JsonData() = default;
JsonData::JsonData(JsonData&&) noexcept = default;
JsonData& JsonData::operator=(JsonData&&) noexcept = default;


bool JsonData::isValid() const
{
    return impl->isValid();
}

bool JsonData::check(const std::string& path) const
{
    return impl->check(path);
}

std::string JsonData::get(const std::string& path) const
{
    return impl->get(path);
}

std::string JsonData::getArray(const std::string& path, std::string const& delimiter) const noexcept
{
    return impl->getArray(path, delimiter);
}

std::vector<std::string> JsonData::getArray(std::string const& path, std::string const& delimiter, std::initializer_list<std::string> nodes) const noexcept
{
    return impl->getArray(path, delimiter, nodes);
}

} // namespace subttxrend
} // namespace common
