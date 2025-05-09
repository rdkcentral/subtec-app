/*****************************************************************************
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2021 Liberty Global Service B.V.#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*****************************************************************************/

#pragma once

#include <string>
#include <vector>

namespace subttxrend
{
namespace common {
    class ConfigProvider;
}

namespace ttmlengine
{
/**
 * Data dumper - used for debugging purposes.
 */
class DataDumper
{
public:
    DataDumper() = default;

    void setup(const common::ConfigProvider* provider);
    void dump(const std::uint8_t* buffer, std::size_t bufferSize) const;
    void toFile(const std::string filename,
                const std::uint8_t* buffer,
                std::size_t bufferLen) const;
    bool imageDumpEnabled() const;
    std::vector<std::uint8_t> readTtmlFromFile(const std::string& path);

private:
    void dumpXml(const std::uint8_t* buffer, std::size_t bufferSize) const;
    void toLog(const std::uint8_t* buffer, std::size_t bufferSize) const;

private:
    bool dumpTolog{false};
    bool dumpToFile{false};
    std::string dirPath;

};

} // namespace ttmlengine
} // namespace subttxrend

