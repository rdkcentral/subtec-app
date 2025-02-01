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


#pragma once

#include <string>
#include <memory>
#include <vector>

// TODO reuse implementation from ipp2::utils. check if subttxrend-dbus/src/JsonHelper.cpp can reuse it as well
namespace subttxrend
{
namespace common
{

class JsonData final
{
  public:
    JsonData(std::string const& json);
    ~JsonData();

    JsonData(JsonData&&) noexcept;
    JsonData& operator=(JsonData&&) noexcept;

    bool isValid() const;
    bool check(std::string const& path) const;
    std::string get(std::string const& path) const;
    std::string getArray(const std::string& path, std::string const& delimiter) const noexcept;
    std::vector<std::string> getArray(std::string const& path, std::string const& delimiter, std::initializer_list<std::string> nodes) const noexcept;

  private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace subttxrend
} // namespace common
